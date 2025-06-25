// IOCPort.cpp: implementation of the CIOCPort class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IOCPort.h"
#include "IOCPSocket2.h"
#include "Define.h"
#include <algorithm>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

CRITICAL_SECTION g_critical;

DWORD WINAPI AcceptThread(LPVOID lp);
DWORD WINAPI ReceiveWorkerThread(LPVOID lp);
DWORD WINAPI ClientWorkerThread(LPVOID lp);
DWORD WINAPI SendWorkerThread(LPVOID lp);

DWORD WINAPI AcceptThread(LPVOID lp)
{
	CIOCPort* pIocport = (CIOCPort*) lp;

	WSANETWORKEVENTS	network_event;
	DWORD				wait_return;
	int					sid;
	CIOCPSocket2*		pSocket = nullptr;
	char				logstr[1024] = {};

	sockaddr_in			addr;
	int					len;

	while (1)
	{
		wait_return = WaitForSingleObject(pIocport->m_hListenEvent, INFINITE);
		if (wait_return == WAIT_FAILED)
		{
			TRACE(_T("Wait failed Error %d\n"), GetLastError());

			TCHAR logstr[1024] = {};
			_stprintf(logstr, _T("Wait failed Error %d\r\n"), GetLastError());
			LogFileWrite(logstr);
			return 1;
		}

		WSAEnumNetworkEvents(pIocport->m_ListenSocket, pIocport->m_hListenEvent, &network_event);

		if (network_event.lNetworkEvents & FD_ACCEPT)
		{
			if (network_event.iErrorCode[FD_ACCEPT_BIT] == 0)
			{
				EnterCriticalSection(&g_critical);
				sid = pIocport->GetNewSid();
				LeaveCriticalSection(&g_critical);
				if (sid < 0)
				{
					TRACE(_T("Accepting User Socket Fail - New Uid is -1\n"));

					TCHAR logstr[1024] = {};
					_stprintf(logstr,_T( "Accepting User Socket Fail - New Uid is -1\r\n"));
					LogFileWrite(logstr);
					continue;
				}

				pSocket = pIocport->GetIOCPSocket(sid);
				if (pSocket == nullptr)
				{
					TRACE(_T("Socket Array has Broken...\n"));

					TCHAR logstr[1024] = {};
					_stprintf(logstr, _T("Socket Array has Broken...\r\n"));
					LogFileWrite(logstr);
//					pIocport->PutOldSid( sid );				// Invalid sid must forbidden to use
					continue;
				}

				len = sizeof(addr);
				if (!pSocket->Accept(pIocport->m_ListenSocket, (sockaddr*) &addr, &len))
				{
					TRACE(_T("Accept Fail %d\n"), sid);

					TCHAR logstr[1024] = {};
					_stprintf(logstr, _T("Accept Fail %d\r\n"), sid);
					LogFileWrite(logstr);

					EnterCriticalSection(&g_critical);
					pIocport->RidIOCPSocket(sid, pSocket);
					pIocport->PutOldSid(sid);
					LeaveCriticalSection(&g_critical);
					continue;
				}

				pSocket->InitSocket(pIocport);

				if (!pIocport->Associate(pSocket, pIocport->m_hServerIOCPort))
				{
					TRACE(_T("Socket Associate Fail\n"));

					TCHAR logstr[1024] = {};
					_stprintf(logstr, _T("Socket Associate Fail\r\n"));
					LogFileWrite(logstr);

					EnterCriticalSection(&g_critical);
					pSocket->CloseProcess();
					pIocport->RidIOCPSocket(sid, pSocket);
					pIocport->PutOldSid(sid);
					LeaveCriticalSection(&g_critical);
					continue;
				}

				// Crytion
				//pSocket->SendCryptionKey();	// 암호화
				// ~
				pSocket->Receive();

				TRACE(_T("Success Accepting...%d\n"), sid);
			}
		}
	}

	return 1;
}

DWORD WINAPI ReceiveWorkerThread(LPVOID lp)
{
	CIOCPort* pIocport = (CIOCPort*) lp;

	DWORD			WorkIndex;
	BOOL			b;
	LPOVERLAPPED	pOvl;
	DWORD			nbytes;
	DWORD			dwFlag = 0;
	CIOCPSocket2*	pSocket = nullptr;

	while (1)
	{
		b = GetQueuedCompletionStatus(
				pIocport->m_hServerIOCPort,
				&nbytes,
				&WorkIndex,
				&pOvl,
				INFINITE);
		if (b
			|| pOvl != nullptr)
		{
			if (b)
			{
				if (WorkIndex >= (DWORD) pIocport->m_SocketArraySize)
					continue;

				pSocket = pIocport->m_SockArray[WorkIndex];
				if (pSocket == nullptr)
					continue;

				switch (pOvl->Offset)
				{
					case OVL_RECEIVE:
						EnterCriticalSection(&g_critical);
						if (nbytes == 0)
						{
							TRACE(_T("User Closed By 0 byte Notify...%d\n"), WorkIndex);
							pSocket->CloseProcess();
							pIocport->RidIOCPSocket(pSocket->GetSocketID(), pSocket);
							pIocport->PutOldSid(pSocket->GetSocketID());
							LeaveCriticalSection(&g_critical);
							break;
						}

						pSocket->m_nPending = 0;
						pSocket->m_nWouldblock = 0;

						pSocket->ReceivedData((int) nbytes);
						pSocket->Receive();
						LeaveCriticalSection(&g_critical);
						break;

					case OVL_SEND:
						pSocket->m_nPending = 0;
						pSocket->m_nWouldblock = 0;
						break;

					case OVL_CLOSE:
						EnterCriticalSection(&g_critical);
						TRACE(_T("User Closed By Close()...%d\n"), WorkIndex);

						pSocket->CloseProcess();
						pIocport->RidIOCPSocket(pSocket->GetSocketID(), pSocket);
						pIocport->PutOldSid(pSocket->GetSocketID());

						LeaveCriticalSection(&g_critical);
						break;
				}
			}
			else
			{
				if (WorkIndex >= (DWORD) pIocport->m_SocketArraySize)
					continue;

				pSocket = pIocport->m_SockArray[WorkIndex];
				if (pSocket == nullptr)
					continue;

				EnterCriticalSection(&g_critical);

				pSocket->CloseProcess();
				pIocport->RidIOCPSocket(pSocket->GetSocketID(), pSocket);
				pIocport->PutOldSid(pSocket->GetSocketID());

				LeaveCriticalSection(&g_critical);

				if (pOvl != nullptr)
				{
					TRACE(_T("User Closed By Abnormal Termination...%d\n"), WorkIndex);
				}
				else
				{
					DWORD ioError = GetLastError();
					TRACE(_T("User Closed By IOCP Error[%d] - %d \n"), ioError, WorkIndex);
				}
			}
		}
	}

	return 1;
}

DWORD WINAPI ClientWorkerThread(LPVOID lp)
{
	CIOCPort* pIocport = (CIOCPort*) lp;

	DWORD			WorkIndex;
	BOOL			b;
	LPOVERLAPPED	pOvl;
	DWORD			nbytes;
	DWORD			dwFlag = 0;
	CIOCPSocket2*	pSocket = nullptr;

	while (1)
	{
		b = GetQueuedCompletionStatus(
				pIocport->m_hClientIOCPort,
				&nbytes,
				&WorkIndex,
				&pOvl,
				INFINITE);
		if (b
			|| pOvl != nullptr)
		{
			if (b)
			{
				if (WorkIndex > (DWORD) pIocport->m_ClientSockSize)
					continue;

				pSocket = pIocport->m_ClientSockArray[WorkIndex];
				if (pSocket == nullptr)
					continue;

				switch (pOvl->Offset)
				{
					case OVL_RECEIVE:
						EnterCriticalSection(&g_critical);
						if (nbytes == 0)
						{
							TRACE(_T("AISocket Closed By 0 Byte Notify\n"));
							pSocket->CloseProcess();
							pIocport->RidIOCPSocket(pSocket->GetSocketID(), pSocket);
	//						pIocport->PutOldSid( pSocket->GetSocketID() );		// 클라이언트 소켓은 Sid 관리하지 않음
							LeaveCriticalSection(&g_critical);
							break;
						}

						pSocket->m_nPending = 0;
						pSocket->m_nWouldblock = 0;

						pSocket->ReceivedData((int) nbytes);
						pSocket->Receive();

						LeaveCriticalSection(&g_critical);
						break;

					case OVL_SEND:
						pSocket->m_nPending = 0;
						pSocket->m_nWouldblock = 0;
						break;

					case OVL_CLOSE:
						EnterCriticalSection(&g_critical);

						TRACE(_T("AISocket Closed By Close()\n"));
						pSocket->CloseProcess();
						pIocport->RidIOCPSocket(pSocket->GetSocketID(), pSocket);
	//					pIocport->PutOldSid( pSocket->GetSocketID() );

						LeaveCriticalSection(&g_critical);
						break;
				}
			}
			else
			{
				if (pOvl != nullptr)
				{
					if (WorkIndex > (DWORD) pIocport->m_ClientSockSize)
						continue;

					pSocket = pIocport->m_ClientSockArray[WorkIndex];
					if (pSocket == nullptr)
						continue;

					EnterCriticalSection(&g_critical);

					TRACE(_T("AISocket Closed By Abnormal Termination\n"));
					pSocket->CloseProcess();
					pIocport->RidIOCPSocket(pSocket->GetSocketID(), pSocket);

					LeaveCriticalSection(&g_critical);
				}
			}
		}
	}

	return 1;
}

DWORD WINAPI SendWorkerThread(LPVOID lp)
{
	CIOCPort* pIocport = (CIOCPort*) lp;

	DWORD			WorkIndex;
	BOOL			b;
	LPOVERLAPPED	pOvl;
	DWORD			nbytes;
	DWORD			dwFlag = 0;
	CIOCPSocket2*	pSocket = nullptr;
	char			pBuff[REGION_BUFF_SIZE];

	while (1)
	{
		b = GetQueuedCompletionStatus(
			pIocport->m_hSendIOCPort,
			&nbytes,
			&WorkIndex,
			&pOvl,
			INFINITE);
		if (b
			|| pOvl != nullptr)
		{
			if (b)
			{
				switch (pOvl->Offset)
				{
					case OVL_SEND:
						for (int i = 0; i < MAX_USER; i++)
						{
							pSocket = pIocport->m_SockArray[i];
							if (pSocket)
							{
								if (pSocket->m_pRegionBuffer->iLength == 0)
									continue;

								int len = 0;
								memset(pBuff, 0x00, REGION_BUFF_SIZE);
								pSocket->RegioinPacketClear(pBuff, len);
								if (len < 500)
								{
									pSocket->Send(pBuff, len);
								}
								else
								{
									pSocket->SendCompressingPacket(pBuff, len);
	//								TRACE(_T("Region Packet %d Bytes\n"), len);
								}
							}
						}
						break;
				}
			}
		}
	}

	return 1;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIOCPort::CIOCPort()
{
	m_ListenSocket = INVALID_SOCKET;
	m_hListenEvent = nullptr;
	m_hServerIOCPort = nullptr;
	m_hClientIOCPort = nullptr;
	m_hSendIOCPort = nullptr;
	m_hAcceptThread = nullptr;

	m_SockArray = nullptr;
	m_SockArrayInActive = nullptr;
	m_ClientSockArray = nullptr;

	m_SocketArraySize = 0;
	m_ClientSockSize = 0;

	m_dwNumberOfWorkers = 0;
	m_dwConcurrency = 1;

	memset(&m_PostOverlapped, 0, sizeof(m_PostOverlapped));

	WSADATA wsaData = {};
	WORD wVersionRequested = MAKEWORD(2, 2);
	(void) WSAStartup(wVersionRequested, &wsaData);

	InitializeCriticalSection(&g_critical);
}

CIOCPort::~CIOCPort()
{
	DeleteCriticalSection(&g_critical);
	DeleteAllArray();

	WSACleanup();
}

void CIOCPort::DeleteAllArray()
{
	for (int i = 0; i < m_SocketArraySize; i++)
	{
		delete m_SockArray[i];
		m_SockArray[i] = nullptr;
	}
	delete[] m_SockArray;

	for (int i = 0; i < m_SocketArraySize; i++)
	{
		delete m_SockArrayInActive[i];
		m_SockArrayInActive[i] = nullptr;
	}
	delete[] m_SockArrayInActive;

	for (int i = 0; i < m_ClientSockSize; i++)
	{
		delete m_ClientSockArray[i];
		m_ClientSockArray[i] = nullptr;
	}
	delete[] m_ClientSockArray;

	while (!m_SidList.empty())
		m_SidList.pop_back();
}

void CIOCPort::Init(int serversocksize, int clientsocksize, int workernum)
{
	m_SocketArraySize = serversocksize;
	m_ClientSockSize = clientsocksize;

	m_SockArray = new CIOCPSocket2* [serversocksize];
	for (int i = 0; i < serversocksize; i++)
		m_SockArray[i] = nullptr;

	m_SockArrayInActive = new CIOCPSocket2* [serversocksize];
	for (int i = 0; i < serversocksize; i++)
		m_SockArrayInActive[i] = nullptr;

	m_ClientSockArray = new CIOCPSocket2* [clientsocksize];		// 해당 서버가 클라이언트로서 다른 컴터에 붙는 소켓수
	for (int i = 0; i < clientsocksize; i++)
		m_ClientSockArray[i] = nullptr;

	for (int i = 0; i < serversocksize; i++)
		m_SidList.push_back(i);

	CreateReceiveWorkerThread(workernum);
	CreateClientWorkerThread();
	CreateSendWorkerThread();

	m_PostOverlapped.hEvent = nullptr;
}

BOOL CIOCPort::Listen(int port)
{
	int opt;
	sockaddr_in addr;
	linger lingerOpt;

	// Open a TCP socket (an Internet stream socket).
	//
	m_ListenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_ListenSocket < 0)
	{
		TRACE(_T("Can't open stream socket\n"));
		return FALSE;
	}

	// Bind our local address so that the client can send to us. 
	//
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	// added in an attempt to allow rebinding to the port 
	//
	opt = 1;
	setsockopt(m_ListenSocket, SOL_SOCKET, SO_REUSEADDR, (char*) &opt, sizeof(opt));

	opt = 1;
	setsockopt(m_ListenSocket, SOL_SOCKET, SO_KEEPALIVE, (char*) &opt, sizeof(opt));

	// Linger off -> close socket immediately regardless of existance of data 
	//
	lingerOpt.l_onoff = 1;
	lingerOpt.l_linger = 0;

	setsockopt(m_ListenSocket, SOL_SOCKET, SO_LINGER, (char*) &lingerOpt, sizeof(lingerOpt));

	if (bind(m_ListenSocket, (sockaddr*) &addr, sizeof(addr)) < 0)
	{
		TRACE(_T("Can't bind local address\n"));
		return FALSE;
	}

	int socklen, len, err;

	socklen = SOCKET_BUFF_SIZE * 4;
	setsockopt(m_ListenSocket, SOL_SOCKET, SO_RCVBUF, (char*) &socklen, sizeof(socklen));

	len = sizeof(socklen);
	err = getsockopt(m_ListenSocket, SOL_SOCKET, SO_RCVBUF, (char*) &socklen, &len);
	if (err == SOCKET_ERROR)
	{
		TRACE(_T("FAIL : Set Socket RecvBuf of port(%d) as %d\n"), port, socklen);
		return FALSE;
	}

	socklen = SOCKET_BUFF_SIZE * 4;
	setsockopt(m_ListenSocket, SOL_SOCKET, SO_SNDBUF, (char*) &socklen, sizeof(socklen));
	len = sizeof(socklen);
	err = getsockopt(m_ListenSocket, SOL_SOCKET, SO_SNDBUF, (char*) &socklen, &len);

	if (err == SOCKET_ERROR)
	{
		TRACE(_T("FAIL: Set Socket SendBuf of port(%d) as %d\n"), port, socklen);
		return FALSE;
	}

	listen(m_ListenSocket, 5);

	m_hListenEvent = WSACreateEvent();
	if (m_hListenEvent == WSA_INVALID_EVENT)
	{
		err = WSAGetLastError();
		TRACE(_T("Listen Event Create Fail!! %d \n"), err);
		return FALSE;
	}

	WSAEventSelect(m_ListenSocket, m_hListenEvent, FD_ACCEPT);

	TRACE(_T("Port (%05d) initialzed\n"), port);

	CreateAcceptThread();

	return TRUE;
}

BOOL CIOCPort::Associate(CIOCPSocket2* pIocpSock, HANDLE hPort)
{
	if (hPort == nullptr)
	{
		TRACE(_T("ERROR : No Completion Port\n"));
		return FALSE;
	}

	HANDLE hTemp = CreateIoCompletionPort(
		pIocpSock->GetSocketHandle(), hPort, pIocpSock->GetSocketID(), m_dwConcurrency);
	return (hTemp == hPort);
}

int CIOCPort::GetNewSid()
{
	if (m_SidList.empty())
	{
		TRACE(_T("SID List Is Empty !!\n"));
		return -1;
	}

	int ret = m_SidList.front();
	m_SidList.pop_front();

	return ret;
}

void CIOCPort::PutOldSid(int sid)
{
	if (sid < 0
		|| sid >= m_SocketArraySize)
	{
		TRACE(_T("recycle sid invalid value : %d\n"), sid);
		return;
	}

	auto Iter = std::find(m_SidList.begin(), m_SidList.end(), sid);
	if (Iter != m_SidList.end())
		return;

	m_SidList.push_back(sid);
}

void CIOCPort::CreateAcceptThread()
{
	DWORD id;

	m_hAcceptThread = ::CreateThread(nullptr, 0, AcceptThread, this, CREATE_SUSPENDED, &id);

	::SetThreadPriority(m_hAcceptThread, THREAD_PRIORITY_ABOVE_NORMAL);
}

void CIOCPort::CreateReceiveWorkerThread(int workernum)
{
	SYSTEM_INFO SystemInfo;

	//
	// try to get timing more accurate... Avoid context
	// switch that could occur when threads are released
	//
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

	//
	// Figure out how many processors we have to size the minimum
	// number of worker threads and concurrency
	//
	GetSystemInfo(&SystemInfo);

	if (workernum == 0)
		m_dwNumberOfWorkers = 2 * SystemInfo.dwNumberOfProcessors;
	else
		m_dwNumberOfWorkers = workernum;
	m_dwConcurrency = SystemInfo.dwNumberOfProcessors;

	m_hServerIOCPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 10);

	for (int i = 0; i < (int) m_dwNumberOfWorkers; i++)
	{
		HANDLE hWorkerThread;
		DWORD WorkerId;

		hWorkerThread = ::CreateThread(
			nullptr,
			0,
			ReceiveWorkerThread,
			this,
			0,
			&WorkerId);
		if (hWorkerThread != nullptr)
			CloseHandle(hWorkerThread);
	}
}

void CIOCPort::CreateClientWorkerThread()
{
	m_hClientIOCPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 10);

	for (int i = 0; i < (int) m_dwConcurrency; i++)
	{
		HANDLE hWorkerThread;
		DWORD WorkerId;

		hWorkerThread = ::CreateThread(
			nullptr,
			0,
			ClientWorkerThread,
			this,
			0,
			&WorkerId);
		if (hWorkerThread != nullptr)
			CloseHandle(hWorkerThread);
	}
}

void CIOCPort::CreateSendWorkerThread()
{
	SYSTEM_INFO		SystemInfo;
	DWORD			dwNumberOfWorkers = 0;

	GetSystemInfo(&SystemInfo);
	dwNumberOfWorkers = 2 * SystemInfo.dwNumberOfProcessors;

	m_hSendIOCPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 10);

	for (int i = 0; i < (int) dwNumberOfWorkers; i++)
	{
		HANDLE hWorkerThread;
		DWORD WorkerId;

		hWorkerThread = ::CreateThread(
			nullptr,
			0,
			SendWorkerThread,
			this,
			0,
			&WorkerId);
	}
}

CIOCPSocket2* CIOCPort::GetIOCPSocket(int index)
{
	if (index >= m_SocketArraySize)
	{
		TRACE(_T("InActiveSocket Array Overflow[%d]\n"), index);
		return nullptr;
	}

	CIOCPSocket2* pIOCPSock = m_SockArrayInActive[index];
	if (pIOCPSock == nullptr)
	{
		TRACE(_T("InActiveSocket Array Invalid[%d]\n"), index);
		return nullptr;
	}

	m_SockArray[index] = pIOCPSock;
	m_SockArrayInActive[index] = nullptr;

	pIOCPSock->SetSocketID(index);

	return pIOCPSock;
}

void CIOCPort::RidIOCPSocket(int index, CIOCPSocket2* pSock)
{
	if (index < 0
		|| (pSock->GetSockType() == TYPE_ACCEPT && index >= m_SocketArraySize)
		|| (pSock->GetSockType() == TYPE_CONNECT && index >= m_ClientSockSize))
	{
		TRACE(_T("Invalid Sock index - RidIOCPSocket\n"));
		return;
	}

	if (pSock->GetSockType() == TYPE_ACCEPT)
	{
		m_SockArray[index] = nullptr;
		m_SockArrayInActive[index] = pSock;
	}
	else if (pSock->GetSockType() == TYPE_CONNECT)
	{
		m_ClientSockArray[index] = nullptr;
	}
}

int CIOCPort::GetClientSid()
{
	for (int i = 0; i < m_ClientSockSize; i++)
	{
		if (m_ClientSockArray[i] == nullptr)
			return i;
	}

	return -1;
}

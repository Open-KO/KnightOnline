// IOCPSocket2.cpp: implementation of the CIOCPSocket2 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IOCPSocket2.h"
#include <shared/CircularBuffer.h>

#include <spdlog/spdlog.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIOCPSocket2::CIOCPSocket2()
{
	m_pBuffer = new CCircularBuffer(SOCKET_BUFF_SIZE);
	m_Socket = INVALID_SOCKET;

	m_pIOCPort = nullptr;
}

CIOCPSocket2::~CIOCPSocket2()
{
	delete m_pBuffer;
}

int CIOCPSocket2::Send(char* pBuf, long length, int dwFlag)
{
	int ret_value = 0;
	WSABUF out;
	DWORD sent = 0;
	OVERLAPPED* pOvl;
	HANDLE hComport = nullptr;

	if (length > MAX_PACKET_SIZE)
		return 0;

	char pTBuf[MAX_PACKET_SIZE];
	memset(pTBuf, 0x00, MAX_PACKET_SIZE);
	int index = 0;

	pTBuf[index++] = (uint8_t) PACKET_START1;
	pTBuf[index++] = (uint8_t) PACKET_START2;
	memcpy(pTBuf + index, &length, 2);
	index += 2;
	memcpy(pTBuf + index, pBuf, length);
	index += length;
	pTBuf[index++] = (uint8_t) PACKET_END1;
	pTBuf[index++] = (uint8_t) PACKET_END2;

	out.buf = pTBuf;
	out.len = index;

	pOvl = &m_SendOverlapped;
	pOvl->Offset = OVL_SEND;
	pOvl->OffsetHigh = out.len;

	ret_value = WSASend(m_Socket, &out, 1, &sent, dwFlag, pOvl, nullptr);
	//if( sent > 100 )
	//	TRACE(_T("Send %d BYtes\n"), sent);

	if (ret_value == SOCKET_ERROR)
	{
		int last_err;
		last_err = WSAGetLastError();

		if (last_err == WSA_IO_PENDING)
		{
			spdlog::debug("IOCPSocket2::Send: socketId={} IO_PENDING", m_Sid);
			m_nPending++;
#ifdef __SAMMA
			if (m_nPending > 3)
				goto close_routine;
#endif
			sent = length;
		}
		else if (last_err == WSAEWOULDBLOCK)
		{
			spdlog::debug("IOCPSocket2::Send: socketId={} WOULDBLOCK", m_Sid);

			m_nWouldblock++;
			if (m_nWouldblock > 3)
				goto close_routine;
			return 0;
		}
		else
		{
			spdlog::error("IOCPSocket2::Send: socketId={} winsock error={}",
				m_Sid, last_err);
			m_nSocketErr++;
			goto close_routine;
		}
	}
	else if (ret_value == 0)
	{
		m_nPending = 0;
		m_nWouldblock = 0;
		m_nSocketErr = 0;
	}

	return sent;

close_routine:
	pOvl = &m_RecvOverlapped;
	pOvl->Offset = OVL_CLOSE;

	hComport = m_pIOCPort->m_hServerIOCPort;
	PostQueuedCompletionStatus(hComport, 0, m_Sid, pOvl);

	return -1;
}

int CIOCPSocket2::Receive()
{
	int RetValue;
	WSABUF in;
	DWORD insize, dwFlag = 0;
	OVERLAPPED* pOvl;
	HANDLE	hComport = nullptr;

	memset(m_pRecvBuff, 0, sizeof(m_pRecvBuff));
	in.len = MAX_PACKET_SIZE;
	in.buf = m_pRecvBuff;

	pOvl = &m_RecvOverlapped;
	pOvl->Offset = OVL_RECEIVE;

	RetValue = WSARecv(m_Socket, &in, 1, &insize, &dwFlag, pOvl, nullptr);

	if (RetValue == SOCKET_ERROR)
	{
		int last_err;
		last_err = WSAGetLastError();

		if (last_err == WSA_IO_PENDING)
		{
//			TRACE(_T("RECV : IO_PENDING[SID=%d]\n"), m_Sid);
//			m_nPending++;
//			if( m_nPending > 3 )
//				goto close_routine;
			return 0;
		}
		else if (last_err == WSAEWOULDBLOCK)
		{
			spdlog::debug("IOCPSocket2::Receive: socketId={} WOULDBLOCK", m_Sid);

			m_nWouldblock++;
			if (m_nWouldblock > 3)
				goto close_routine;
			return 0;
		}
		else
		{
			spdlog::error("IOCPSocket2::Receive: socketId={} winsock error={}",
				m_Sid, last_err);

			m_nSocketErr++;
			if (m_nSocketErr == 2)
				goto close_routine;
			return -1;
		}
	}

	return (int) insize;

close_routine:
	pOvl = &m_RecvOverlapped;
	pOvl->Offset = OVL_CLOSE;

	hComport = m_pIOCPort->m_hServerIOCPort;
	PostQueuedCompletionStatus(hComport, 0, m_Sid, pOvl);

	return -1;
}

void CIOCPSocket2::ReceivedData(int length)
{
	if (length <= 0)
		return;

	int len = 0;
	m_pBuffer->PutData(m_pRecvBuff, length);		// 받은 Data를 버퍼에 넣는다

	char* pData = nullptr;
	char* pDecData = nullptr;

	while (PullOutCore(pData, len))
	{
		if (pData != nullptr)
		{
			Parsing(len, pData); // 실제 파싱 함수...

			delete[] pData;
			pData = nullptr;
		}
	}
}

bool CIOCPSocket2::PullOutCore(char*& data, int& length)
{
	uint8_t*	pTmp;
	int			len;
	bool		foundCore;
	MYSHORT		slen;
	uint32_t		wSerial = 0;

	len = m_pBuffer->GetValidCount();

	if (len <= 0)
		return false;

	pTmp = new uint8_t[len];

	m_pBuffer->GetData((char*) pTmp, len);

	foundCore = false;

	int	sPos = 0, ePos = 0;

	for (int i = 0; i < len && !foundCore; i++)
	{
		if (i + 2 >= len)
			break;

		if (pTmp[i] == PACKET_START1
			&& pTmp[i + 1] == PACKET_START2)
		{
//			if (m_wPacketSerial >= wSerial)
//				goto cancelRoutine;

			sPos = i + 2;

			slen.b[0] = pTmp[sPos];
			slen.b[1] = pTmp[sPos + 1];

			length = slen.i;

			if (length < 0)
				goto cancelRoutine;

			if (length > len)
				goto cancelRoutine;

			ePos = sPos + length + 2;

			if ((ePos + 2) > len)
				goto cancelRoutine;

//			ASSERT(ePos+2 <= len);

			if (pTmp[ePos] == PACKET_END1
				&& pTmp[ePos + 1] == PACKET_END2)
			{
				data = new char[length + 1];
				CopyMemory(data, (pTmp + sPos + 2), length);
				data[length] = 0;
				foundCore = true;
				int head = m_pBuffer->GetHeadPos(), tail = m_pBuffer->GetTailPos();
//				TRACE(_T("data : %hs, len : %d\n"), data, length);
//				TRACE(_T("head : %d, tail : %d\n"), head, tail );
				break;
			}
			else
			{
				m_pBuffer->HeadIncrease(3);
				break;
			}
		}
	}

	if (foundCore)
		m_pBuffer->HeadIncrease(6 + length); //6: header 2+ end 2+ length 2

cancelRoutine:
	delete[] pTmp;
	return foundCore;
}

void CIOCPSocket2::Close()
{
	if (m_pIOCPort == nullptr)
		return;

	HANDLE hComport = nullptr;
	OVERLAPPED* pOvl;
	pOvl = &m_RecvOverlapped;
	pOvl->Offset = OVL_CLOSE;

	hComport = m_pIOCPort->m_hServerIOCPort;

	int retValue = PostQueuedCompletionStatus(hComport, 0, m_Sid, pOvl);
	if (retValue == 0)
	{
		int errValue = GetLastError();
		spdlog::error("IOCPSocket2::Close: socketId={} PostQueuedCompletionStatus error={}",
			m_Sid, errValue);
	}
}

void CIOCPSocket2::CloseProcess()
{
	m_State = STATE_DISCONNECTED;

	if (m_Socket != INVALID_SOCKET)
		closesocket(m_Socket);
}

void CIOCPSocket2::InitSocket(CIOCPort* pIOCPort)
{
	m_pIOCPort = pIOCPort;
	m_RecvOverlapped.hEvent = nullptr;
	m_SendOverlapped.hEvent = nullptr;
	m_pBuffer->SetEmpty();
	m_nSocketErr = 0;
	m_nPending = 0;
	m_nWouldblock = 0;

	Initialize();
}

bool CIOCPSocket2::Accept(SOCKET listensocket, sockaddr* addr, int* len)
{
	m_Socket = accept(listensocket, addr, len);
	if (m_Socket == INVALID_SOCKET)
	{
		int err = WSAGetLastError();
		spdlog::error("IOCPSocket2::Accept: socketId={} winsock error={}",
			m_Sid, err);
		return false;
	}

//	int flag = 1;
//	setsockopt(m_Socket, SOL_SOCKET, SO_DONTLINGER, (char *)&flag, sizeof(flag));

//	int lensize, socklen=0;

//	getsockopt( m_Socket, SOL_SOCKET, SO_RCVBUF, (char*)&socklen, &lensize);
//	TRACE(_T("getsockopt : %d\n"), socklen);

//	struct linger lingerOpt;

//	lingerOpt.l_onoff = 1;
//	lingerOpt.l_linger = 0;

//	setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (char *)&lingerOpt, sizeof(lingerOpt));

	return true;
}

void CIOCPSocket2::Parsing(int length, char* pData)
{
}

void CIOCPSocket2::Initialize()
{
}

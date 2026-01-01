// APISocket.cpp: implementation of the CAPISocket class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "APISocket.h"
#include "ClientResourceFormatter.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//
static WSAData s_WSData;
int			CAPISocket::s_nInstanceCount = 0;


#ifdef _CRYPTION
BOOL		CAPISocket::s_bCryptionFlag = FALSE;			//0 : 비암호화 , 1 : 암호화
CJvCryption	CAPISocket::s_JvCrypt;
uint32_t	CAPISocket::s_wSendVal = 0;
uint32_t	CAPISocket::s_wRcvVal = 0;
#endif

const uint16_t PACKET_HEADER = 0XAA55;
const uint16_t PACKET_TAIL = 0X55AA;

#ifdef _N3GAME
#include <N3Base/LogWriter.h>
#endif

CAPISocket::CAPISocket()
{
	m_hSocket = (void *)INVALID_SOCKET;
	m_hWndTarget = nullptr;
	m_szIP.clear();
	m_dwPort = 0;

	if (s_nInstanceCount++ == 0)
		WSAStartup(0x0101, &s_WSData);

	m_iSendByteCount = 0;
	m_bConnected = FALSE;
	m_bEnableSend = TRUE; // 보내기 가능..?
}

CAPISocket::~CAPISocket()
{
	Release();

	s_nInstanceCount--;
	if (s_nInstanceCount==0)
	{
		WSACleanup();
	}
}

void CAPISocket::Release()
{
	this->Disconnect();

	while (!m_qRecvPkt.empty())
	{
		auto pkt = m_qRecvPkt.front();
		delete pkt;
		m_qRecvPkt.pop();
	}

	m_iSendByteCount = 0;
		
#ifdef _DEBUG
	memset(m_Statistics_Send_Sum, 0, sizeof(m_Statistics_Send_Sum));
	memset(m_Statistics_Recv_Sum, 0, sizeof(m_Statistics_Recv_Sum));
#endif
}

void CAPISocket::Disconnect()
{
	if ((SOCKET)m_hSocket != INVALID_SOCKET)
		closesocket((SOCKET)m_hSocket);
	
	m_hSocket = (void *)INVALID_SOCKET;
	m_hWndTarget = nullptr;
	m_szIP.clear();
	m_dwPort = 0;

	m_bConnected = FALSE;
	m_bEnableSend = TRUE; // 보내기 가능..?

#ifdef _CRYPTION
	InitCrypt(0); // 암호화 해제..
#endif // #ifdef _CRYPTION
}

int CAPISocket::Connect(HWND hWnd, const char* pszIP, uint32_t dwPort)
{	
	if (pszIP == nullptr || dwPort == 0)
		return -1;

	if ((SOCKET) m_hSocket != INVALID_SOCKET)
		this->Disconnect();

	//
	struct sockaddr_in far server;
	struct hostent far* hp;
  
	if ((pszIP[0] >= '0') && (pszIP[0] <= '9'))
	{
		memset(&server, 0, sizeof(server));
		server.sin_family      = AF_INET;
		server.sin_addr.s_addr = inet_addr(pszIP);
		server.sin_port        = htons((u_short) dwPort);
	}
	else
	{
		hp = gethostbyname(pszIP);
		if (hp == nullptr)
		{
#ifdef _DEBUG
			std::string msg = fmt::format("Error: Connecting to {}.", pszIP);
			MessageBoxA(hWnd, msg.c_str(), "socket error", MB_OK | MB_ICONSTOP);
#endif
			return -1;
		}

		memset(&server, 0, sizeof(server));
		memcpy(&server.sin_addr, hp->h_addr, hp->h_length);
		server.sin_family = hp->h_addrtype;
		server.sin_port = htons((u_short)dwPort);  
	}// else 

	// create socket 
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) 
	{
		int iErrCode = ::WSAGetLastError();
#ifdef _DEBUG
		char msg[] = "Error opening stream socket";
		MessageBoxA(hWnd, msg, "socket error", MB_OK | MB_ICONSTOP);
#endif
		return iErrCode;
	}

	m_hSocket = (void *)sock;

	// 소켓 옵션
	int iRecvBufferLen = RECEIVE_BUF_SIZE;
	int iErr = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&iRecvBufferLen, 4);
  
	if (connect(sock, (struct sockaddr far *)&server, sizeof(server)) != 0)
	{
		int iErrCode = ::WSAGetLastError();

		closesocket(sock);
		m_hSocket = (void *)INVALID_SOCKET;

		return iErrCode;
	}

	WSAAsyncSelect(sock, hWnd, WM_SOCKETMSG, FD_CONNECT | FD_READ | FD_CLOSE);

	m_hWndTarget = hWnd;
	m_szIP = pszIP;
	m_dwPort = dwPort;
	m_bConnected = TRUE;
	
#ifdef _DEBUG
	memset(m_Statistics_Send_Sum, 0, sizeof(m_Statistics_Send_Sum));
	memset(m_Statistics_Recv_Sum, 0, sizeof(m_Statistics_Recv_Sum));
#endif
	
	return 0;
}

int	CAPISocket::ReConnect()
{
	return Connect(m_hWndTarget, m_szIP.c_str(), m_dwPort);
}

void CAPISocket::Receive()
{
	if (INVALID_SOCKET == (SOCKET)m_hSocket || FALSE == m_bConnected)
		return;

	u_long	dwPktSize = 0;
	u_long	dwRead = 0;
	int		count = 0;

	ioctlsocket((SOCKET) m_hSocket, FIONREAD, &dwPktSize);
	while(dwRead < dwPktSize)
	{
		count = recv((SOCKET)m_hSocket, (char*)m_RecvBuf, RECEIVE_BUF_SIZE, 0);
		if (count == SOCKET_ERROR)
		{
			__ASSERT(0,"socket receive error!");
#ifdef _N3GAME
			int iErr = ::GetLastError();
			CLogWriter::Write("socket receive error! : {}", iErr);
			//TRACE("socket receive error! : %d\n", iErr);
#endif
			break;
		}
		if (count)
		{
			dwRead += count;
			m_CB.PutData(m_RecvBuf, count);
		}
	}

	// packet analysis.
	while(ReceiveProcess());
}

BOOL CAPISocket::ReceiveProcess()
{
	int iCount = m_CB.GetValidCount();
	BOOL bFoundTail = FALSE;
	if (iCount >=7 )
	{
		uint8_t *pData = new uint8_t[iCount];
		m_CB.GetData(pData, iCount);
		int head_inc_size = 0;

		if ( PACKET_HEADER == ntohs(*((uint16_t*)pData)) )
		{
			int16_t siCore = *((int16_t*)(pData+2));
			if ( siCore <= iCount )
			{
				if ( PACKET_TAIL == ntohs(*((uint16_t*)(pData+iCount-2))) ) // 패킷 꼬리 부분 검사..
				{
					Packet * pkt = new Packet();
					if (s_bCryptionFlag)
					{
						static uint8_t pTBuf[RECEIVE_BUF_SIZE];
						s_JvCrypt.JvDecryptionFast(siCore, pData + 4, pTBuf);

						uint16_t sig = *(uint16_t*)pTBuf;

						if (sig != 0x1EFC)
						{
							__ASSERT(0, "Crypt Error");
						}
						else
						{
							uint16_t sequence = *(uint16_t*)&pTBuf[2];
							uint8_t  empty = pTBuf[4];
							uint8_t* payload = &pTBuf[5];

							pkt->append(payload, siCore - 5);
						}
					}
					else
					{
						pkt->append(pData + 4, siCore);
					}

					m_qRecvPkt.push(pkt);
					m_CB.HeadIncrease(siCore + 6); // 환형 버퍼 인덱스 증가 시키기..
					bFoundTail = TRUE;
#ifdef _DEBUG
					uint8_t byCmd = pData[4];
					m_Statistics_Recv_Sum[byCmd].dwTime++;
					m_Statistics_Recv_Sum[byCmd].iSize += siCore;
#endif
				}
			}
		}
		else
		{
			// 패킷이 깨졌다??
			__ASSERT(0, "broken packet header.. skip!");
			m_CB.HeadIncrease(iCount); // 환형 버퍼 인덱스 증가 시키기..
		}

		delete[] pData, pData = nullptr;
	}

	return bFoundTail;
}

void CAPISocket::Send(uint8_t* pData, int nSize)
{
	if(!m_bEnableSend) return; // 보내기 가능..?
	if (INVALID_SOCKET == (SOCKET)m_hSocket || FALSE == m_bConnected)
		return;

#ifdef _CRYPTION
	DataPack DP;

	if (s_bCryptionFlag)
	{
		static uint8_t pTBuf[RECEIVE_BUF_SIZE];

		++s_wSendVal;

		memcpy(pTBuf, &s_wSendVal, sizeof(uint32_t));
		memcpy((pTBuf + 4), pData, nSize);

		*((uint32_t*)(pTBuf + (nSize + 4))) = crc32(pTBuf, (nSize + 4), -1);

		s_JvCrypt.JvEncryptionFast((nSize + 4 + 4), pTBuf, pTBuf);

		DP.m_Size = (nSize + 4 + 4);
		DP.m_pData = new uint8_t[DP.m_Size];
		memcpy(DP.m_pData, pTBuf, DP.m_Size);

		nSize = DP.m_Size;
		pData = DP.m_pData;
	}
#endif
	
	int nTotalSize = nSize+6;
	uint8_t *pSendData = m_RecvBuf;
	*((uint16_t*)pSendData) = htons(PACKET_HEADER);	pSendData+=2;
	*((uint16_t*)pSendData) = nSize;				pSendData+=2;
	memcpy(pSendData, pData, nSize);			pSendData += nSize;
	*((uint16_t*)pSendData) = htons(PACKET_TAIL);	pSendData+=2;

	int nSent = 0;
	int count = 0;
	while(nSent < nTotalSize)
	{
		count = send((SOCKET)m_hSocket, (char*)m_RecvBuf, nTotalSize, 0);
		if (count == SOCKET_ERROR)
		{
			__ASSERT(0,"socket send error!");
#ifdef _N3GAME
			int iErr = ::GetLastError();
			CLogWriter::Write("socket send error! : {}", iErr);
			//TRACE("socket send error! : %d\n", iErr);
			PostQuitMessage(-1);
#endif
			break;
		}
		if (count)
		{
			nSent += count;
		}
	}

#ifdef _DEBUG
	uint8_t byCmd = pData[0]; // 통계 넣기..

	m_Statistics_Send_Sum[byCmd].dwTime++;
	m_Statistics_Send_Sum[byCmd].iSize += nSize;
#endif

	m_iSendByteCount += nTotalSize;
}

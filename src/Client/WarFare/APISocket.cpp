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
int CAPISocket::s_nInstanceCount = 0;

#ifdef _CRYPTION
BOOL CAPISocket::s_bCryptionFlag = FALSE; //0 : 비암호화 , 1 : 암호화
CJvCryption CAPISocket::s_JvCrypt;
uint32_t CAPISocket::s_wSendVal = 0;
uint32_t CAPISocket::s_wRcvVal  = 0;
#endif

const uint16_t PACKET_HEADER = 0XAA55;
const uint16_t PACKET_TAIL   = 0X55AA;

#ifdef _N3GAME
#include <N3Base/LogWriter.h>
#endif

CAPISocket::CAPISocket() : m_SendBuf(SEND_BUF_SIZE), m_RecvBuf(RECV_BUF_SIZE), m_CB(RECV_BUF_SIZE)
{
	m_hSocket    = INVALID_SOCKET;
	m_hWndTarget = nullptr;
	m_dwPort     = 0;

	if (s_nInstanceCount++ == 0)
		(void) WSAStartup(0x0101, &s_WSData);

	m_iSendByteCount = 0;
	m_bConnected     = FALSE;
	m_bEnableSend    = TRUE; // 보내기 가능..?

	memset(m_SendBuf.data(), 0, m_SendBuf.size());
	memset(m_RecvBuf.data(), 0, m_RecvBuf.size());
}

CAPISocket::~CAPISocket()
{
	Release();

	if (--s_nInstanceCount == 0)
		WSACleanup();
}

void CAPISocket::Release()
{
	Disconnect();

	while (!m_qRecvPkt.empty())
	{
		delete m_qRecvPkt.front();
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
	if (m_hSocket != INVALID_SOCKET)
		closesocket(m_hSocket);

	m_hSocket    = INVALID_SOCKET;
	m_hWndTarget = nullptr;
	m_szIP.clear();
	m_dwPort      = 0;

	m_bConnected  = FALSE;
	m_bEnableSend = TRUE; // 보내기 가능..?

#ifdef _CRYPTION
	InitCrypt(0);         // 암호화 해제..
#endif                    // #ifdef _CRYPTION
}

int CAPISocket::Connect(HWND hWnd, const std::string& szIP, uint32_t dwPort)
{
	if (szIP.empty() || dwPort == 0)
		return -1;

	if (m_hSocket != INVALID_SOCKET)
		Disconnect();

	//
	struct sockaddr_in server {};
	struct hostent* hp = nullptr;

	if ((szIP[0] >= '0') && (szIP[0] <= '9'))
	{
		server.sin_family      = AF_INET;
		server.sin_addr.s_addr = inet_addr(szIP.c_str());
		server.sin_port        = htons((u_short) dwPort);
	}
	else
	{
		hp = gethostbyname(szIP.c_str());
		if (hp == nullptr)
		{
#ifdef _DEBUG
			std::string msg = fmt::format("Error: Connecting to {}.", szIP);
			MessageBoxA(hWnd, msg.c_str(), "socket error", MB_OK | MB_ICONSTOP);
#endif
			return -1;
		}

		memcpy(&server.sin_addr, hp->h_addr, hp->h_length);
		server.sin_family = hp->h_addrtype;
		server.sin_port   = htons((u_short) dwPort);
	} // else

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

	m_hSocket          = sock;

	// 소켓 옵션
	int iRecvBufferLen = RECV_BUF_SIZE;
	setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*) &iRecvBufferLen, 4);

	if (connect(sock, (struct sockaddr*) &server, sizeof(server)) != 0)
	{
		int iErrCode = ::WSAGetLastError();

		closesocket(sock);
		m_hSocket = INVALID_SOCKET;

		return iErrCode;
	}

	WSAAsyncSelect(sock, hWnd, WM_SOCKETMSG, FD_CONNECT | FD_READ | FD_CLOSE);

	m_hWndTarget = hWnd;
	m_szIP       = szIP;
	m_dwPort     = dwPort;
	m_bConnected = TRUE;

#ifdef _DEBUG
	memset(m_Statistics_Send_Sum, 0, sizeof(m_Statistics_Send_Sum));
	memset(m_Statistics_Recv_Sum, 0, sizeof(m_Statistics_Recv_Sum));
#endif

	return 0;
}

int CAPISocket::ReConnect()
{
	return Connect(m_hWndTarget, m_szIP, m_dwPort);
}

void CAPISocket::Receive()
{
	if (INVALID_SOCKET == (SOCKET) m_hSocket || FALSE == m_bConnected)
		return;

	u_long dwPktSize = 0;
	u_long dwRead    = 0;
	int count        = 0;

	ioctlsocket((SOCKET) m_hSocket, FIONREAD, &dwPktSize);
	while (dwRead < dwPktSize)
	{
		count = recv((SOCKET) m_hSocket, m_RecvBuf.data(), static_cast<int>(m_RecvBuf.size()), 0);
		if (count == SOCKET_ERROR)
		{
			__ASSERT(0, "socket receive error!");
#ifdef _N3GAME
			int iErr = ::GetLastError();
			CLogWriter::Write("socket receive error! : {}", iErr);
#endif
			break;
		}
		if (count)
		{
			dwRead += count;
			m_CB.PutData(m_RecvBuf.data(), count);
		}
	}

	// packet analysis.
	while (ReceiveProcess())
		;
}

BOOL CAPISocket::ReceiveProcess()
{
	int iCount      = m_CB.GetValidCount();
	BOOL bFoundTail = FALSE;
	if (iCount >= 7)
	{
		std::vector<uint8_t> data(iCount);
		m_CB.GetData(reinterpret_cast<char*>(data.data()), iCount);

		if (PACKET_HEADER == ntohs(*reinterpret_cast<uint16_t*>(&data[0])))
		{
			int16_t siCore = *reinterpret_cast<int16_t*>(&data[2]);
			if (siCore <= iCount)
			{
				// 패킷 꼬리 부분 검사..
				if (PACKET_TAIL == ntohs(*reinterpret_cast<uint16_t*>(&data[iCount - 2])))
				{
					Packet* pkt = new Packet();
					if (s_bCryptionFlag)
					{
						// NOTE: Decrypts in-place
						s_JvCrypt.JvDecryptionFast(siCore, &data[4], &data[4]);

						uint16_t sig = *reinterpret_cast<uint16_t*>(&data[4]);

						if (sig != 0x1EFC)
						{
							delete pkt;
							__ASSERT(0, "Crypt Error");
							return FALSE;
						}

						// uint16_t sequence = *(uint16_t*) &&data[6];
						// uint8_t empty     = &data[8];
						pkt->append(&data[9], siCore - 5);
					}
					else
					{
						pkt->append(&data[4], siCore);
					}

					m_qRecvPkt.push(pkt);
					m_CB.HeadIncrease(siCore + 6); // 환형 버퍼 인덱스 증가 시키기..
					bFoundTail = TRUE;
#ifdef _DEBUG
					uint8_t byCmd = data[4];
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
	}

	return bFoundTail;
}

void CAPISocket::Send(uint8_t* pData, int nSize)
{
	if (!m_bEnableSend)
		return; // 보내기 가능..?
	if (INVALID_SOCKET == (SOCKET) m_hSocket || FALSE == m_bConnected)
		return;

#ifdef _CRYPTION
	DataPack DP;

	if (s_bCryptionFlag)
	{
		static uint8_t pTBuf[SEND_BUF_SIZE];

		++s_wSendVal;

		memcpy(pTBuf, &s_wSendVal, sizeof(uint32_t));
		memcpy((pTBuf + 4), pData, nSize);

		*((uint32_t*) (pTBuf + (nSize + 4))) = crc32(pTBuf, (nSize + 4), -1);

		s_JvCrypt.JvEncryptionFast((nSize + 4 + 4), pTBuf, pTBuf);

		DP.m_Size  = (nSize + 4 + 4);
		DP.m_pData = new uint8_t[DP.m_Size];
		memcpy(DP.m_pData, pTBuf, DP.m_Size);

		nSize = DP.m_Size;
		pData = DP.m_pData;
	}
#endif

	int nTotalSize            = nSize + 6;
	char* pSendData           = m_SendBuf.data();
	*((uint16_t*) pSendData)  = htons(PACKET_HEADER);
	pSendData                += 2;
	*((uint16_t*) pSendData)  = nSize;
	pSendData                += 2;
	memcpy(pSendData, pData, nSize);
	pSendData                += nSize;
	*((uint16_t*) pSendData)  = htons(PACKET_TAIL);
	// pSendData             += 2;

	int nSent                 = 0;
	int count                 = 0;
	while (nSent < nTotalSize)
	{
		count = send((SOCKET) m_hSocket, m_SendBuf.data(), nTotalSize, 0);
		if (count == SOCKET_ERROR)
		{
			__ASSERT(0, "socket send error!");
#ifdef _N3GAME
			int iErr = ::GetLastError();
			CLogWriter::Write("socket send error! : {}", iErr);
			PostQuitMessage(-1);
#endif
			break;
		}

		if (count > 0)
			nSent += count;
	}

#ifdef _DEBUG
	uint8_t byCmd = pData[0]; // 통계 넣기..

	m_Statistics_Send_Sum[byCmd].dwTime++;
	m_Statistics_Send_Sum[byCmd].iSize += nSize;
#endif

	m_iSendByteCount += nTotalSize;
}

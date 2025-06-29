﻿// IOCPSocket2.cpp: implementation of the CIOCPSocket2 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IOCPSocket2.h"
#include "Define.h"

#include <shared/lzf.h>
#include <shared/CircularBuffer.h>
#include <shared/packets.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// nop function
void bb()
{
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIOCPSocket2::CIOCPSocket2()
{
	m_pBuffer = new CCircularBuffer(SOCKET_BUFF_SIZE);
	m_pRegionBuffer = new _REGION_BUFFER();
	m_Socket = INVALID_SOCKET;

	m_pIOCPort = nullptr;
	m_Type = TYPE_ACCEPT;

	// Cryption
	m_CryptionFlag = 0;
}

CIOCPSocket2::~CIOCPSocket2()
{
	delete m_pBuffer;
	delete m_pRegionBuffer;
}

BOOL CIOCPSocket2::Create(UINT nSocketPort, int nSocketType, long lEvent, const char* lpszSocketAddress)
{
	int ret;
	linger lingerOpt;

	m_Socket = socket(AF_INET, nSocketType/*SOCK_STREAM*/, 0);
	if (m_Socket == INVALID_SOCKET)
	{
		ret = WSAGetLastError();
		TRACE(_T("Socket Create Fail! - %d\n"), ret);
		return FALSE;
	}

	m_hSockEvent = WSACreateEvent();
	if (m_hSockEvent == WSA_INVALID_EVENT)
	{
		ret = WSAGetLastError();
		TRACE(_T("Event Create Fail! - %d\n"), ret);
		return FALSE;
	}

	// Linger off -> close socket immediately regardless of existance of data 
	//
	lingerOpt.l_onoff = 0;
	lingerOpt.l_linger = 0;

	setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (char*) &lingerOpt, sizeof(lingerOpt));

	int socklen;

	socklen = SOCKET_BUFF_SIZE * 4;
	setsockopt(m_Socket, SOL_SOCKET, SO_RCVBUF, (char*) &socklen, sizeof(socklen));

	socklen = SOCKET_BUFF_SIZE * 4;
	setsockopt(m_Socket, SOL_SOCKET, SO_SNDBUF, (char*) &socklen, sizeof(socklen));

	return TRUE;
}

BOOL CIOCPSocket2::Connect(CIOCPort* pIocp, const char* lpszHostAddress, UINT nHostPort)
{
	sockaddr_in addr;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(lpszHostAddress);
	addr.sin_port = htons(nHostPort);

	int result = connect(m_Socket, (sockaddr*) &addr, sizeof(addr));
	if (result == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
//		TRACE(_T("CONNECT FAIL : %d\n"), err);
		closesocket(m_Socket);
		return FALSE;
	}

	ASSERT(pIocp);

	InitSocket(pIocp);

	m_Sid = m_pIOCPort->GetClientSid();
	if (m_Sid < 0)
		return FALSE;

	m_pIOCPort->m_ClientSockArray[m_Sid] = this;

	if (!m_pIOCPort->Associate(this, m_pIOCPort->m_hClientIOCPort))
	{
		TRACE(_T("Socket Connecting Fail - Associate\n"));
		return FALSE;
	}

	m_ConnectAddress = lpszHostAddress;
	m_State = STATE_CONNECTED;
	m_Type = TYPE_CONNECT;

	Receive();

	return TRUE;
}

int CIOCPSocket2::Send(char* pBuf, long length, int dwFlag)
{
	int ret_value = 0;
	WSABUF out;
	DWORD sent = 0;
	OVERLAPPED* pOvl;
	HANDLE	hComport = nullptr;

	if (length > MAX_PACKET_SIZE)
		return 0;

	BYTE pTBuf[MAX_PACKET_SIZE] = {},
		pTIBuf[MAX_PACKET_SIZE] = {},
		pTOutBuf[MAX_PACKET_SIZE] = {};

	int index = 0;

	if (m_CryptionFlag)
	{
		unsigned short len = length + sizeof(WORD) + 2 + 1;

		m_Sen_val++;
		m_Sen_val &= 0x00ffffff;

		pTIBuf[0] = 0xfc; // 암호가 정확한지
		pTIBuf[1] = 0x1e;
		memcpy(pTIBuf + 2, &m_Sen_val, sizeof(WORD) + 1);
		memcpy(pTIBuf + 5, pBuf, length);
		jct.JvEncryptionFast(len, pTIBuf, pTOutBuf);

		pTBuf[index++] = (BYTE) PACKET_START1;
		pTBuf[index++] = (BYTE) PACKET_START2;
		memcpy(pTBuf + index, &len, 2);
		index += 2;
		memcpy(pTBuf + index, pTOutBuf, len);
		index += len;
		pTBuf[index++] = (BYTE) PACKET_END1;
		pTBuf[index++] = (BYTE) PACKET_END2;
	}
	else
	{
		pTBuf[index++] = (BYTE) PACKET_START1;
		pTBuf[index++] = (BYTE) PACKET_START2;
		memcpy(pTBuf + index, &length, 2);
		index += 2;
		memcpy(pTBuf + index, pBuf, length);
		index += length;
		pTBuf[index++] = (BYTE) PACKET_END1;
		pTBuf[index++] = (BYTE) PACKET_END2;
	}

	out.buf = (char*) pTBuf;
	out.len = index;

	pOvl = &m_SendOverlapped;
	pOvl->Offset = OVL_SEND;
	pOvl->OffsetHigh = out.len;

	ret_value = WSASend(m_Socket, &out, 1, &sent, dwFlag, pOvl, nullptr);

	if (ret_value == SOCKET_ERROR)
	{
		int last_err = WSAGetLastError();
		if (last_err == WSA_IO_PENDING)
		{
			TRACE(_T("SEND : IO_PENDING[SID=%d]\n"), m_Sid);
			m_nPending++;
			if (m_nPending > 3)
				goto close_routine;

			sent = length;
		}
		else if (last_err == WSAEWOULDBLOCK)
		{
			TRACE(_T("SEND : WOULDBLOCK[SID=%d]\n"), m_Sid);

			m_nWouldblock++;
			if (m_nWouldblock > 3)
				goto close_routine;

			return 0;
		}
		else
		{
			TRACE(_T("SEND : ERROR [SID=%d] - %d\n"), m_Sid, last_err);
//			char logstr[1024] = {};
//			sprintf( logstr, "SEND : ERROR [SID=%d] - %d\r\n", m_Sid, last_err);
//			LogFileWrite( logstr );
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

	if (m_Type == TYPE_ACCEPT)
		hComport = m_pIOCPort->m_hServerIOCPort;
	else
		hComport = m_pIOCPort->m_hClientIOCPort;

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
		int last_err = WSAGetLastError();
		if (last_err == WSA_IO_PENDING)
		{
//			TRACE(_T("RECV : IO_PENDING[SID=%d]\n"), m_Sid);

//			m_nPending++;
//			if (m_nPending > 3)
//				goto close_routine;

			return 0;
		}
		else if (last_err == WSAEWOULDBLOCK)
		{
			TRACE(_T("RECV : WOULDBLOCK[SID=%d]\n"), m_Sid);

			m_nWouldblock++;
			if (m_nWouldblock > 3)
				goto close_routine;

			return 0;
		}
		else
		{
			TRACE(_T("RECV : ERROR [SID=%d] - %d\n"), m_Sid, last_err);

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

	if (m_Type == TYPE_ACCEPT)
		hComport = m_pIOCPort->m_hServerIOCPort;
	else
		hComport = m_pIOCPort->m_hClientIOCPort;

	PostQueuedCompletionStatus(hComport, 0, m_Sid, pOvl);

	return -1;
}

void CIOCPSocket2::ReceivedData(int length)
{
	if (length == 0)
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

BOOL CIOCPSocket2::PullOutCore(char*& data, int& length)
{
	int buffer_len = m_pBuffer->GetValidCount();

	// We expect at least 7 bytes (header, length, data [at least 1 byte], tail)
	if (buffer_len < 7)
		return FALSE; // wait for more data

	std::vector<BYTE> tmp_buffer(buffer_len);
	m_pBuffer->GetData((char*) &tmp_buffer[0], buffer_len);

	if (tmp_buffer[0] != PACKET_START1
		&& tmp_buffer[1] != PACKET_START2)
	{
		TRACE(
			_T("%d: PullOutCore() - failed to detect header (%02X, %02X)\n"),
			m_Sid,
			tmp_buffer[0],
			tmp_buffer[1]);
			
		Close();
		return FALSE;
	}

	// Find the packet's start position - this is in front of the 2 byte header.
	int sPos = 2;

	// Build the length (2 bytes, network order)
	MYSHORT slen;
	slen.b[0] = tmp_buffer[sPos];
	slen.b[1] = tmp_buffer[sPos + 1];

	length = slen.w;

	int original_length = length;

	if (length < 0)
	{
		TRACE(
			_T("%d: PullOutCore() - invalid length (%d)\n"),
			m_Sid,
			length);

		Close();
		return FALSE;
	}

	if (length > buffer_len)
	{
		TRACE(
			_T("%d: PullOutCore() - reported length (%d) is not in buffer (%d) - waiting for now\n"),
			m_Sid,
			length,
			buffer_len);
		return FALSE; // wait for more data
	}

	// Find the end position of the packet data.
	// From the start position, that is after 2 bytes for the length,
	// then the length of the data itself.
	int ePos = sPos + 2 + length;

	// We expect a 2 byte tail after the end position.
	if ((ePos + 2) > buffer_len)
	{
		TRACE(
			_T("%d: PullOutCore() - tail not in buffer - waiting for now\n"),
			m_Sid);
		return FALSE; // wait for more data
	}

	if (tmp_buffer[ePos] != PACKET_END1
		|| tmp_buffer[ePos + 1] != PACKET_END2)
	{
		TRACE(
			_T("%d: PullOutCore() - failed to detect tail (%02X, %02X)\n"),
			m_Sid,
			tmp_buffer[ePos],
			tmp_buffer[ePos + 1]);

		Close();
		return FALSE;
	}

	// We've found the entire packet.
	// Do we need to decrypt it?
	if (m_CryptionFlag)
	{
		// Encrypted packets contain a checksum (4) and sequence number (4).
		// We should also expect at least 1 byte for its data in addition to this.
		if (length <= 8)
		{
			TRACE(
				_T("%d: PullOutCore() - Insufficient packet length [%d] for a decrypted packet\n"),
				m_Sid,
				length);

			Close();
			return FALSE;
		}

		std::vector<BYTE> decryption_buffer(length);

		int decrypted_len = jct.JvDecryptionWithCRC32(length, &tmp_buffer[sPos + 2], &decryption_buffer[0]);
		if (decrypted_len < 0)
		{
			TRACE(
				_T("%d: PullOutCore() - Failed decryption\n"),
				m_Sid);

			Close();
			return FALSE;
		}

		int index = 0;
		DWORD recv_packet = GetDWORD((char*) &decryption_buffer[0], index);

		// Verify the sequence number.
		// If it wraps back around, we should simply let it reset.
		if (recv_packet != 0
			&& m_Rec_val > recv_packet)
		{
			TRACE(
				_T("%d: PullOutCore() - recv_packet error... len=%d, recv_packet=%d, prev=%d\n"),
				m_Sid,
				length,
				recv_packet,
				m_Rec_val);

			Close();
			return FALSE;
		}

		m_Rec_val = recv_packet;

		// Now we need to trim out the extra data from the packet, so it's just the base packet data remaining.
		// Make sure that there is still data for this.
		length = decrypted_len - index;
		if (length <= 0)
		{
			TRACE(
				_T("%d: PullOutCore() - decrypted packet length too small... len=%d\n"),
				m_Sid,
				length);

			Close();
			return FALSE;
		}

		data = new char[length];
		memcpy(data, &decryption_buffer[index], length);
	}
	// Packet not encrypted, we can just copy it over as-is.
	else
	{
		data = new char[length];
		memcpy(data, &tmp_buffer[sPos + 2], length);
	}

	m_pBuffer->HeadIncrease(6 + original_length); // 6: header (2) + end (2) + length (2)

	// Found a packet in this attempt.
	return TRUE;
}

BOOL CIOCPSocket2::AsyncSelect(long lEvent)
{
	int retEventResult = WSAEventSelect(m_Socket, m_hSockEvent, lEvent);
	int err = WSAGetLastError();

	return retEventResult == 0;
}

BOOL CIOCPSocket2::SetSockOpt(int nOptionName, const void* lpOptionValue, int nOptionLen, int nLevel)
{
	int retValue = setsockopt(m_Socket, nLevel, nOptionName, (char*) lpOptionValue, nOptionLen);
	return retValue == 0;
}

BOOL CIOCPSocket2::ShutDown(int nHow)
{
	int retValue = shutdown(m_Socket, nHow);
	return retValue == 0;
}

void CIOCPSocket2::Close()
{
	if (m_pIOCPort == nullptr)
		return;

	HANDLE	hComport = nullptr;
	OVERLAPPED* pOvl;
	pOvl = &m_RecvOverlapped;
	pOvl->Offset = OVL_CLOSE;

	if (m_Type == TYPE_ACCEPT)
		hComport = m_pIOCPort->m_hServerIOCPort;
	else
		hComport = m_pIOCPort->m_hClientIOCPort;

	int retValue = PostQueuedCompletionStatus(hComport, 0, m_Sid, pOvl);
	if (retValue == 0)
	{
		int errValue = GetLastError();
		TRACE(_T("PostQueuedCompletionStatus Error : %d\n"), errValue);
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

	m_CryptionFlag = 0;
	m_Sen_val = 0;
	m_Rec_val = 0;

	Initialize();
}

BOOL CIOCPSocket2::Accept(SOCKET listensocket, struct sockaddr* addr, int* len)
{
	m_Socket = accept(listensocket, addr, len);
	if (m_Socket == INVALID_SOCKET)
	{
		int err = WSAGetLastError();
		TRACE(_T("Socket Accepting Fail - %d\n"), err);

		TCHAR logstr[1024] = {};
		_stprintf(logstr, _T("Socket Accepting Fail - %d\r\n"), err);
		LogFileWrite(logstr);
		return FALSE;
	}

//	int flag = 1;
//	setsockopt(m_Socket, SOL_SOCKET, SO_DONTLINGER, (char *)&flag, sizeof(flag));

//	int lensize, socklen=0;

//	getsockopt( m_Socket, SOL_SOCKET, SO_RCVBUF, (char*)&socklen, &lensize);
//	TRACE(_T("getsockopt : %d\n"), socklen);

//	linger lingerOpt;
//	lingerOpt.l_onoff = 1;
//	lingerOpt.l_linger = 0;

//	setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (char *)&lingerOpt, sizeof(lingerOpt));

	return TRUE;
}

void CIOCPSocket2::Parsing(int length, char* pData)
{
}

void CIOCPSocket2::Initialize()
{
	m_wPacketSerial = 0;
	m_pRegionBuffer->iLength = 0;
	memset(m_pRegionBuffer->pDataBuff, 0, sizeof(m_pRegionBuffer->pDataBuff));
}

void CIOCPSocket2::SendCompressingPacket(const char* pData, int len)
{
	if (len <= 0
		|| len >= 49152)
	{
		TRACE(_T("### SendCompressingPacket Error : len = %d ### \n"), len);
		return;
	}

	int send_index = 0;
	char send_buff[32000] = {}, pBuff[32000] = {};
	unsigned int out_len = 0;

	out_len = lzf_compress(pData, len, pBuff, sizeof(pBuff));
	if (out_len == 0
		|| out_len > sizeof(pBuff))
	{
		TRACE(_T("Compressing Fail Packet\n"));
		Send((char*) pData, len);
		return;
	}

	SetByte(send_buff, WIZ_COMPRESS_PACKET, send_index);
	SetShort(send_buff, (short) out_len, send_index);
	SetShort(send_buff, (short) len, send_index);
	SetDWORD(send_buff, 0, send_index); // checksum
	SetString(send_buff, pBuff, out_len, send_index);
	Send(send_buff, send_index);
}

void CIOCPSocket2::RegionPacketAdd(char* pBuf, int len)
{
	int count = 0;
	do
	{
		if (m_pRegionBuffer->bFlag == W)
		{
			bb();
			count++;
			continue;
		}

		m_pRegionBuffer->bFlag = W;
		m_pRegionBuffer->dwThreadID = ::GetCurrentThreadId();
		bb();

		// Dual Lock System...
		if (m_pRegionBuffer->dwThreadID != ::GetCurrentThreadId())
		{
			count++;
			continue;
		}

		SetShort(m_pRegionBuffer->pDataBuff, len, m_pRegionBuffer->iLength);
		SetString(m_pRegionBuffer->pDataBuff, pBuf, len, m_pRegionBuffer->iLength);
		m_pRegionBuffer->bFlag = WR;
		break;
	}
	while (count < 30);

	if (count > 29)
	{
//		TRACE(_T("Region packet Add Drop\n"));
		Send(pBuf, len);
	}
}

void CIOCPSocket2::RegioinPacketClear(char* GetBuf, int& len)
{
	int count = 0;
	do
	{
		if (m_pRegionBuffer->bFlag == W)
		{
			bb();
			count++;
			continue;
		}

		m_pRegionBuffer->bFlag = W;
		m_pRegionBuffer->dwThreadID = ::GetCurrentThreadId();
		bb();

		// Dual Lock System...
		if (m_pRegionBuffer->dwThreadID != ::GetCurrentThreadId())
		{
			count++;
			continue;
		}

		int index = 0;
		SetByte(GetBuf, WIZ_CONTINOUS_PACKET, index);
		SetShort(GetBuf, m_pRegionBuffer->iLength, index);
		SetString(GetBuf, m_pRegionBuffer->pDataBuff, m_pRegionBuffer->iLength, index);
		len = index;

		memset(m_pRegionBuffer->pDataBuff, 0x00, REGION_BUFF_SIZE);
		m_pRegionBuffer->iLength = 0;
		m_pRegionBuffer->bFlag = E;
		break;
	}
	while (count < 30);

	if (count > 29)
		TRACE(_T("Region packet Clear Drop\n"));
}

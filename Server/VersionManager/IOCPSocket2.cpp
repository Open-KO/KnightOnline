// IOCPSocket2.cpp: implementation of the CIOCPSocket2 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IOCPSocket2.h"
#include <shared/CircularBuffer.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIOCPSocket2::CIOCPSocket2(CIOCPort* iocPort)
	: m_pIOCPort(iocPort), m_Socket(iocPort->GetIoContext())
{
	m_pBuffer = new CCircularBuffer(SOCKET_BUFF_SIZE);
}

CIOCPSocket2::~CIOCPSocket2()
{
	delete m_pBuffer;
}

int CIOCPSocket2::Send(char* pBuf, long length)
{
	if (length > MAX_PACKET_SIZE)
		return 0;

	constexpr int PacketHeaderSize = 6;

	// TODO: A circular buffer would be better for this.
	auto sendBuffer_ = std::make_unique<uint8_t[]>(length + PacketHeaderSize);
	uint8_t* sendBuffer = sendBuffer_.get();

	int index = 0;

	sendBuffer[index++] = PACKET_START1;
	sendBuffer[index++] = PACKET_START2;
	memcpy(&sendBuffer[index], &length, 2);
	index += 2;
	memcpy(&sendBuffer[index], pBuf, length);
	index += length;
	sendBuffer[index++] = (uint8_t) PACKET_END1;
	sendBuffer[index++] = (uint8_t) PACKET_END2;

	try
	{
		m_Socket.async_write_some(asio::buffer(sendBuffer, index),
			[this, sendBuffer_ = std::move(sendBuffer_)]
			(const asio::error_code& ec, size_t bytesTransferred) mutable
			{
				m_pIOCPort->OnPostSend(ec, bytesTransferred, this);
			});
	}
	catch (const asio::system_error& ex)
	{
		spdlog::error("IOCPSocket2::Send: failed to post send for socketId={}: {}",
			m_Sid, ex.what());
		Close();
		return -1;
	}

	return index;
}

void CIOCPSocket2::Receive()
{
	if (m_pIOCPort == nullptr)
		return;

	memset(m_pRecvBuff, 0, sizeof(m_pRecvBuff));

	try
	{
		m_Socket.async_read_some(asio::buffer(m_pRecvBuff),
			std::bind(&CIOCPort::OnPostReceive, m_pIOCPort, std::placeholders::_1, std::placeholders::_2, this));
	}
	catch (const asio::system_error& ex)
	{
		spdlog::error("IOCPSocket2::Receive: failed to post receive for socketId={}: {}",
			m_Sid, ex.what());
		Close();
	}
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

	len = m_pBuffer->GetValidCount();

	if (len == 0
		|| len < 0)
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
				CopyMemory((void*) data, (const void*) (pTmp + sPos + 2), length);
				data[length] = 0;
				foundCore = true;
				int head = m_pBuffer->GetHeadPos(), tail = m_pBuffer->GetTailPos();
//				TRACE("data : %s, len : %d\n", data, length);
//				TRACE("head : %d, tail : %d\n", head, tail );
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
		m_pBuffer->HeadIncrease(6 + length); // 6: header 2+ end 2+ length 2

	delete[] pTmp;

	return foundCore;

cancelRoutine:
	delete[] pTmp;
	return foundCore;
}

void CIOCPSocket2::Close()
{
	if (m_pIOCPort == nullptr)
		return;

	asio::error_code ec;
	try
	{
		auto threadPool = m_pIOCPort->GetWorkerPool();
		if (threadPool == nullptr)
			return;

		asio::post(*threadPool, std::bind(&CIOCPort::OnPostClose, m_pIOCPort, this));
	}
	catch (const asio::system_error& ex)
	{
		spdlog::error("IOCPSocket2::Close: failed to post close for socketId={}: {}",
			m_Sid, ex.what());
	}
}

void CIOCPSocket2::CloseProcess()
{
	m_State = STATE_DISCONNECTED;

	if (m_Socket.is_open())
	{
		asio::error_code ec;
		m_Socket.close(ec);

		if (ec)
		{
			spdlog::error("IOCPSocket2::CloseProcess: close() failed for socketId={}: {}",
				m_Sid, ec.message());
		}
	}
}

void CIOCPSocket2::InitSocket(asio::ip::tcp::socket&& socket)
{
	m_Socket = std::move(socket);
	m_pBuffer->SetEmpty();
	m_nSocketErr = 0;

	Initialize();
}

void CIOCPSocket2::Parsing(int length, char* pData)
{
}

void CIOCPSocket2::Initialize()
{
}

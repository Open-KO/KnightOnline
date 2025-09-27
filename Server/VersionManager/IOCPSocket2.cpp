// IOCPSocket2.cpp: implementation of the CIOCPSocket2 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IOCPSocket2.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIOCPSocket2::CIOCPSocket2(CIOCPort* iocPort)
	: _iocPort(iocPort),
	_recvCircularBuffer(SOCKET_BUFF_SIZE),
	_sendCircularBuffer(SOCKET_BUFF_SIZE),
	_socket(*iocPort->GetWorkerPool())
{
	_state = STATE_DISCONNECTED;
	_sendInProgress = false;
}

CIOCPSocket2::~CIOCPSocket2()
{
}

int CIOCPSocket2::Send(char* pBuf, int length)
{
	constexpr int PacketHeaderSize = 6;

	_ASSERT(length >= 0);
	_ASSERT(length + PacketHeaderSize <= MAX_PACKET_SIZE);

	if (length < 0
		|| length + PacketHeaderSize > MAX_PACKET_SIZE)
		return -1;

	char sendBuffer[MAX_PACKET_SIZE];
	int index = 0;
	SetByte(sendBuffer, PACKET_START1, index);
	SetByte(sendBuffer, PACKET_START2, index);
	SetShort(sendBuffer, length, index);
	SetString(sendBuffer, pBuf, length, index);
	SetByte(sendBuffer, PACKET_END1, index);
	SetByte(sendBuffer, PACKET_END2, index);

	std::lock_guard<std::recursive_mutex> lock(_sendMutex);

	// Add this packet to the circular buffer.
	// Ensure we do not allow resizing; we do not want these pointers invalidated.
	auto span = _sendCircularBuffer.PutData(sendBuffer, index, false);
	if (span.Buffer1 != nullptr
		&& span.Length1 > 0)
	{
		auto queuedSend = std::make_unique<QueuedSend>();
		queuedSend->IsOwned = false;
		queuedSend->BufferSpan = span;
		_sendQueue.push(std::move(queuedSend));
	}
	// Failed to add to the buffer, it has no room.
	// Allocate and queue.
	else
	{
		auto queuedSend = std::make_unique<QueuedSend>();
		queuedSend->IsOwned = true;
		queuedSend->BufferSpan.Buffer1 = new char[index];
		queuedSend->BufferSpan.Length1 = index;
		memcpy(queuedSend->BufferSpan.Buffer1, sendBuffer, index);
		_sendQueue.push(std::move(queuedSend));
	}

	if (!DoSend(false))
		return -1;

	return index;
}

bool CIOCPSocket2::DoSend(bool fromAsyncChain)
{
	std::lock_guard<std::recursive_mutex> lock(_sendMutex);

	// Send currently in progress.
	// Don't attempt to write; it's in the queue, it'll be processed once the send is completed.
	if (_sendInProgress)
		return false;

	// When we finish a send, we should pop the last entry before queueing up another send.
	if (fromAsyncChain)
	{
		_ASSERT(!_sendQueue.empty());
		_sendQueue.pop();
		_sendInProgress = false;
	}

	// Send queue is empty, nothing more to queue up.
	// Consider this successful.
	if (_sendQueue.empty())
		return true;

	// Fetch the next entry to send.
	// Note that we keep this in the queue until the send completes.
	const auto& queuedSend = _sendQueue.front();
	const auto& span = queuedSend->BufferSpan;

	try
	{
		std::array<asio::const_buffer, 2> buffers;
		size_t bufferCount = 1;
		buffers[0] = asio::buffer(span.Buffer1, span.Length1);

		if (span.Buffer2 != nullptr
			&& span.Length2 > 0)
		{
			buffers[1] = asio::buffer(span.Buffer2, span.Length2);
			++bufferCount;
		}

		_socket.async_write_some(buffers,
			std::bind(&CIOCPort::OnPostSend, _iocPort, std::placeholders::_1, std::placeholders::_2, this));

		_sendInProgress = true;
	}
	catch (const asio::system_error& ex)
	{
		spdlog::error("IOCPSocket2::Send: failed to post send for socketId={}: {}",
			_socketId, ex.what());
		Close();
		return false;
	}

	return true;
}

void CIOCPSocket2::Receive()
{
	if (_iocPort == nullptr)
		return;

	memset(_recvBuffer, 0, sizeof(_recvBuffer));

	try
	{
		_socket.async_read_some(asio::buffer(_recvBuffer),
			std::bind(&CIOCPort::OnPostReceive, _iocPort, std::placeholders::_1, std::placeholders::_2, this));
	}
	catch (const asio::system_error& ex)
	{
		spdlog::error("IOCPSocket2::Receive: failed to post receive for socketId={}: {}",
			_socketId, ex.what());
		Close();
	}
}

void CIOCPSocket2::ReceivedData(int length)
{
	if (length <= 0)
		return;

	int len = 0;
	_recvCircularBuffer.PutData(_recvBuffer, length);		// 받은 Data를 버퍼에 넣는다

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

	len = _recvCircularBuffer.GetValidCount();

	if (len == 0
		|| len < 0)
		return false;

	pTmp = new uint8_t[len];

	_recvCircularBuffer.GetData((char*) pTmp, len);

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
//				int head = _recvCircularBuffer.GetHeadPos(), tail = _recvCircularBuffer.GetTailPos();
//				TRACE("data : %s, len : %d\n", data, length);
//				TRACE("head : %d, tail : %d\n", head, tail );
				break;
			}
			else
			{
				_recvCircularBuffer.HeadIncrease(3);
				break;
			}
		}
	}

	if (foundCore)
		_recvCircularBuffer.HeadIncrease(6 + length); // 6: header 2+ end 2+ length 2

cancelRoutine:
	delete[] pTmp;
	return foundCore;
}

void CIOCPSocket2::Close()
{
	if (_iocPort == nullptr
		|| GetState() == STATE_DISCONNECTED)
		return;

	asio::error_code ec;
	try
	{
		auto threadPool = _iocPort->GetWorkerPool();
		if (threadPool == nullptr)
			return;

		asio::post(*threadPool, std::bind(&CIOCPort::OnPostClose, _iocPort, this));
	}
	catch (const asio::system_error& ex)
	{
		spdlog::error("IOCPSocket2::Close: failed to post close for socketId={}: {}",
			_socketId, ex.what());
	}
}

void CIOCPSocket2::CloseProcess()
{
	_state = STATE_DISCONNECTED;

	if (_socket.is_open())
	{
		asio::error_code ec;
		_socket.shutdown(asio::socket_base::shutdown_both, ec);
		if (ec)
		{
			spdlog::error("IOCPSocket2::CloseProcess: shutdown() failed for socketId={}: {}",
				_socketId, ec.message());
		}

		_socket.close(ec);
		if (ec)
		{
			spdlog::error("IOCPSocket2::CloseProcess: close() failed for socketId={}: {}",
				_socketId, ec.message());
		}
	}

	{
		std::lock_guard<std::recursive_mutex> lock(_sendMutex);
		while (!_sendQueue.empty())
			_sendQueue.pop();
	}
}

void CIOCPSocket2::InitSocket()
{
	_state = STATE_CONNECTED;

	_sendCircularBuffer.SetEmpty();
	_recvCircularBuffer.SetEmpty();
	_socketErrorCount = 0;

	Initialize();
}

void CIOCPSocket2::Parsing(int length, char* pData)
{
}

void CIOCPSocket2::Initialize()
{
}

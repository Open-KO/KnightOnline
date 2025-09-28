// IOCPSocket2.cpp: implementation of the CIOCPSocket2 class.
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

CIOCPSocket2::CIOCPSocket2(CIOCPort* iocPort)
	: _iocPort(iocPort),
	_recvCircularBuffer(SOCKET_BUFF_SIZE),
	_sendCircularBuffer(SOCKET_BUFF_SIZE),
	_socket(*iocPort->GetWorkerPool())
{
	_type = TYPE_ACCEPT;
	_state = STATE_DISCONNECTED;
	_socketId = -1;
	_recvValue = 0;
	_sendValue = 0;
	_sendInProgress = false;
	_socketErrorCount = 0;

	_jvCryptionEnabled = false;
	_remoteIpCached = false;

	memset(_recvBuffer, 0, sizeof(_recvBuffer));
	_regionBuffer = new _REGION_BUFFER();
}

CIOCPSocket2::~CIOCPSocket2()
{
	delete _regionBuffer;
}

bool CIOCPSocket2::Create()
{
	asio::error_code ec;

	_socket.open(asio::ip::tcp::v4(), ec);
	if (ec)
	{
		spdlog::error("IOCPSocket2::Create: failed to open socket: {}", ec.message());
		return false;
	}

	// Disable linger (close socket immediately regardless of existence of pending data)
	_socket.set_option(asio::socket_base::linger(false, 0), ec);
	if (ec)
	{
		spdlog::error("IOCPSocket2::Create: failed to set linger option: {}", ec.message());
		return false;
	}

	// Increase receive buffer size
	_socket.set_option(asio::socket_base::receive_buffer_size(SOCKET_BUFF_SIZE * 4), ec);
	if (ec)
	{
		spdlog::error("IOCPSocket2::Create: failed to set receive buffer size: {}", ec.message());
		return false;
	}

	// Increase send buffer size
	_socket.set_option(asio::socket_base::send_buffer_size(SOCKET_BUFF_SIZE * 4), ec);
	if (ec)
	{
		spdlog::error("IOCPSocket2::Create: failed to set send buffer size: {}", ec.message());
		return false;
	}

	return true;
}

bool CIOCPSocket2::Connect(const char* hostAddress, uint16_t hostPort)
{
	asio::error_code ec;

	asio::ip::address ip = asio::ip::make_address(hostAddress, ec);
	if (ec)
	{
		spdlog::error("IOCPSocket2::Connect: invalid address {}: {}",
			hostAddress, ec.message());
		return false;
	}

	asio::ip::tcp::endpoint endpoint(ip, hostPort);

	_socket.connect(endpoint, ec);
	if (ec)
	{
		spdlog::error("IOCPSocket2::Connect: failed to connect: {}", ec.message());
		_socket.close();
		return false;
	}

	ASSERT(_iocPort);

	if (!_iocPort->AcquireClientSocket(this))
	{
		spdlog::error("IOCPSocket2::Connect: failed to acquire client socket ID");
		return false;
	}

	InitSocket();

	_state = STATE_CONNECTED;
	_type = TYPE_CONNECT;

	_remoteIp = ip.to_string();
	_remoteIpCached = true;

	Receive();

	return true;
}

int CIOCPSocket2::Send(char* pBuf, int length)
{
	constexpr int PacketHeaderSize			= 6;
	constexpr int EncryptedPacketHeaderSize	= 5;

	char sendBuffer[MAX_PACKET_SIZE];
	int index = 0;

	if (_jvCryptionEnabled)
	{
		_ASSERT(length >= 0);
		_ASSERT((length + PacketHeaderSize + EncryptedPacketHeaderSize) <= MAX_PACKET_SIZE);

		if (length < 0
			|| length + (PacketHeaderSize + EncryptedPacketHeaderSize) > MAX_PACKET_SIZE)
			return -1;

		uint16_t encryptedLength = static_cast<uint16_t>(length + EncryptedPacketHeaderSize);

		_sendValue++;
		_sendValue &= 0x00ffffff;

		SetByte(sendBuffer, PACKET_START1, index);
		SetByte(sendBuffer, PACKET_START2, index);
		SetShort(sendBuffer, encryptedLength, index);

		int encryptIndex = index;

		SetByte(sendBuffer, 0xfc, index); // 암호가 정확한지
		SetByte(sendBuffer, 0x1e, index);
		SetString(sendBuffer, reinterpret_cast<const char*>(&_sendValue), 3, index);
		SetString(sendBuffer, pBuf, length, index);

		// This can encrypt in-place.
		uint8_t* bufferToEncrypt = reinterpret_cast<uint8_t*>(&sendBuffer[encryptIndex]);
		_jvCryption.JvEncryptionFast(index - encryptIndex, bufferToEncrypt, bufferToEncrypt);

		SetByte(sendBuffer, PACKET_END1, index);
		SetByte(sendBuffer, PACKET_END2, index);
	}
	else
	{
		_ASSERT(length >= 0);
		_ASSERT((length + PacketHeaderSize) <= MAX_PACKET_SIZE);

		if (length < 0
			|| (length + PacketHeaderSize) > MAX_PACKET_SIZE)
			return -1;

		SetByte(sendBuffer, PACKET_START1, index);
		SetByte(sendBuffer, PACKET_START2, index);
		SetShort(sendBuffer, length, index);
		SetString(sendBuffer, pBuf, length, index);
		SetByte(sendBuffer, PACKET_END1, index);
		SetByte(sendBuffer, PACKET_END2, index);
	}

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

	// When we finish a send, we should pop the last entry before queueing up another send.
	if (fromAsyncChain)
	{
		_ASSERT(_sendInProgress);
		_ASSERT(!_sendQueue.empty());
		_sendQueue.pop();
		_sendInProgress = false;
	}
	else
	{
		// Send currently in progress.
		// Don't attempt to write; it's in the queue, it'll be processed once the send is completed.
		if (_sendInProgress)
			return false;
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

	_recvCircularBuffer.PutData(_recvBuffer, length);

	char* extractedPacket = nullptr;
	int extractedPacketLength = 0;
	while (PullOutCore(extractedPacket, extractedPacketLength))
	{
		if (extractedPacket == nullptr)
			continue;

		Parsing(extractedPacketLength, extractedPacket);

		delete[] extractedPacket;
		extractedPacket = nullptr;
	}
}

bool CIOCPSocket2::PullOutCore(char*& data, int& length)
{
	int bufferLength = _recvCircularBuffer.GetValidCount();

	// We expect at least 7 bytes (header, length, data [at least 1 byte], tail)
	if (bufferLength < 7)
		return false; // wait for more data

	std::vector<uint8_t> tmp_buffer(bufferLength);
	_recvCircularBuffer.GetData((char*) &tmp_buffer[0], bufferLength);

	if (tmp_buffer[0] != PACKET_START1
		&& tmp_buffer[1] != PACKET_START2)
	{
		spdlog::error("IOCPSocket2::PullOutCore: {}: failed to detect header ({:2X}, {:2X})",
			_socketId, tmp_buffer[0], tmp_buffer[1]);
			
		Close();
		return false;
	}

	// Find the packet's start position - this is in front of the 2 byte header.
	int startPos = 2;

	// Build the length (2 bytes, network order)
	MYSHORT slen;
	slen.b[0] = tmp_buffer[startPos];
	slen.b[1] = tmp_buffer[startPos + 1];

	length = slen.w;

	int originalLength = length;

	if (length < 0)
	{
		spdlog::error("IOCPSocket2::PullOutCore: {}: invalid length ({})",
			_socketId, length);

		Close();
		return false;
	}

	if (length > bufferLength)
	{
		spdlog::debug("IOCPSocket2::PullOutCore: {}: reported length ({}) is not in buffer ({}) - waiting for now",
			_socketId, length, bufferLength);
		return false; // wait for more data
	}

	// Find the end position of the packet data.
	// From the start position, that is after 2 bytes for the length,
	// then the length of the data itself.
	int endPos = startPos + 2 + length;

	// We expect a 2 byte tail after the end position.
	if ((endPos + 2) > bufferLength)
	{
		spdlog::debug("IOCPSocket2::PullOutCore: {}: tail not in buffer - waiting for now",
			_socketId);
		return false; // wait for more data
	}

	if (tmp_buffer[endPos] != PACKET_END1
		|| tmp_buffer[endPos + 1] != PACKET_END2)
	{
		spdlog::error("IOCPSocket2::PullOutCore: {}: failed to detect tail ({:2X}, {:2X})",
			_socketId, tmp_buffer[endPos], tmp_buffer[endPos + 1]);

		Close();
		return false;
	}

	// We've found the entire packet.
	// Do we need to decrypt it?
	if (_jvCryptionEnabled)
	{
		// Encrypted packets contain a checksum (4) and sequence number (4).
		// We should also expect at least 1 byte for its data in addition to this.
		if (length <= 8)
		{
			spdlog::error("IOCPSocket2::PullOutCore: {}: Insufficient packet length [{}] for a decrypted packet",
				_socketId, length);
			Close();
			return false;
		}

		std::vector<uint8_t> decryption_buffer(length);

		int decryptedLength = _jvCryption.JvDecryptionWithCRC32(length, &tmp_buffer[startPos + 2], &decryption_buffer[0]);
		if (decryptedLength < 0)
		{
			spdlog::error("IOCPSocket2::PullOutCore: {}: Failed decryption",
				_socketId);
			Close();
			return false;
		}

		int index = 0;
		uint32_t recvValue = GetDWORD((char*) &decryption_buffer[0], index);

		// Verify the sequence number.
		// If it wraps back around, we should simply let it reset.
		if (recvValue != 0
			&& _recvValue > recvValue)
		{
			spdlog::error("IOCPSocket2::PullOutCore: {}: recvValue error... len={}, recvValue={}, prev={}",
				_socketId, length, recvValue, _recvValue);

			Close();
			return false;
		}

		_recvValue = recvValue;

		// Now we need to trim out the extra data from the packet, so it's just the base packet data remaining.
		// Make sure that there is still data for this.
		length = decryptedLength - index;
		if (length <= 0)
		{
			spdlog::error("IOCPSocket2::PullOutCore: {}: decrypted packet length too small... len={}",
				_socketId, length);

			Close();
			return false;
		}

		data = new char[length];
		memcpy(data, &decryption_buffer[index], length);
	}
	// Packet not encrypted, we can just copy it over as-is.
	else
	{
		data = new char[length];
		memcpy(data, &tmp_buffer[startPos + 2], length);
	}

	_recvCircularBuffer.HeadIncrease(6 + originalLength); // 6: header (2) + end (2) + length (2)

	// Found a packet in this attempt.
	return true;
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
	_sendCircularBuffer.SetEmpty();
	_recvCircularBuffer.SetEmpty();
	_socketErrorCount = 0;
	_remoteIp.clear();
	_remoteIpCached = false;

	Initialize();
}

void CIOCPSocket2::Parsing(int length, char* pData)
{
}

void CIOCPSocket2::Initialize()
{
	_jvCryptionEnabled = false;
	_sendValue = 0;
	_recvValue = 0;

	_regionBuffer->iLength = 0;
	memset(_regionBuffer->pDataBuff, 0, sizeof(_regionBuffer->pDataBuff));
}

void CIOCPSocket2::SendCompressingPacket(const char* pData, int len)
{
	if (len <= 0
		|| len >= 49152)
	{
		spdlog::error("IOCPSocket2::SendCompressingPacket: message length out of bounds [len={}]",
			len);
		return;
	}

	int send_index = 0;
	char send_buff[32000] = {}, pBuff[32000] = {};
	unsigned int out_len = 0;

	out_len = lzf_compress(pData, len, pBuff, sizeof(pBuff));
	if (out_len == 0
		|| out_len > sizeof(pBuff))
	{
		spdlog::error("IOCPSocket2::SendCompressingPacket: compression failed [out_len={} pBuffSize={}]",
			out_len, sizeof(pBuff));
		Send((char*) pData, len);
		return;
	}

	SetByte(send_buff, WIZ_COMPRESS_PACKET, send_index);
	SetShort(send_buff, (int16_t) out_len, send_index);
	SetShort(send_buff, (int16_t) len, send_index);
	SetDWORD(send_buff, 0, send_index); // checksum
	SetString(send_buff, pBuff, out_len, send_index);
	Send(send_buff, send_index);
}

void CIOCPSocket2::RegionPacketAdd(char* pBuf, int len)
{
	int count = 0;
	do
	{
		if (_regionBuffer->bFlag == W)
		{
			bb();
			count++;
			continue;
		}

		_regionBuffer->bFlag = W;
		_regionBuffer->dwThreadID = ::GetCurrentThreadId();
		bb();

		// Dual Lock System...
		if (_regionBuffer->dwThreadID != ::GetCurrentThreadId())
		{
			count++;
			continue;
		}

		SetShort(_regionBuffer->pDataBuff, len, _regionBuffer->iLength);
		SetString(_regionBuffer->pDataBuff, pBuf, len, _regionBuffer->iLength);
		_regionBuffer->bFlag = WR;
		break;
	}
	while (count < 30);

	if (count > 29)
	{
//		TRACE(_T("Region packet Add Drop\n"));
		Send(pBuf, len);
	}
}

void CIOCPSocket2::RegionPacketClear(char* GetBuf, int& len)
{
	int count = 0;
	do
	{
		if (_regionBuffer->bFlag == W)
		{
			bb();
			count++;
			continue;
		}

		_regionBuffer->bFlag = W;
		_regionBuffer->dwThreadID = ::GetCurrentThreadId();
		bb();

		// Dual Lock System...
		if (_regionBuffer->dwThreadID != ::GetCurrentThreadId())
		{
			count++;
			continue;
		}

		int index = 0;
		SetByte(GetBuf, WIZ_CONTINOUS_PACKET, index);
		SetShort(GetBuf, _regionBuffer->iLength, index);
		SetString(GetBuf, _regionBuffer->pDataBuff, _regionBuffer->iLength, index);
		len = index;

		memset(_regionBuffer->pDataBuff, 0x00, REGION_BUFF_SIZE);
		_regionBuffer->iLength = 0;
		_regionBuffer->bFlag = E;
		break;
	}
	while (count < 30);

	if (count > 29)
	{
		spdlog::error("IOCPSocket2::RegionPacketClear: count exceeds 29 [count{}]",
			count);
	}
}

const std::string& CIOCPSocket2::GetRemoteIP()
{
	if (!_remoteIpCached)
	{
		asio::error_code ec;

		asio::ip::tcp::endpoint endpoint = _socket.remote_endpoint(ec);
		if (!ec)
		{
			_remoteIp = endpoint.address().to_string();
			_remoteIpCached = true;
		}
		else
		{
			spdlog::warn("IOCPSocket2::GetRemoteIP: failed lookup. socketId={}",
				_socketId);
		}
	}

	return _remoteIp;
}

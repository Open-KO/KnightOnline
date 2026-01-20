#include "pch.h"
#include "TcpSocket.h"
#include "TcpSocketManager.h"

#include <cassert>

TcpSocket::TcpSocket(test_tag) :
	_recvBufferSize(DEFAULT_RECV_BUFFER_SIZE), _sendBufferSize(DEFAULT_SEND_BUFFER_SIZE),
	_recvCircularBuffer(DEFAULT_RECV_BUFFER_SIZE), _sendCircularBuffer(DEFAULT_SEND_BUFFER_SIZE)
{
	_recvBuffer.resize(_recvBufferSize);
}

TcpSocket::TcpSocket(TcpSocketManager* socketManager) :
	_socketManager(socketManager), _recvBufferSize(socketManager->GetRecvBufferSize()),
	_sendBufferSize(socketManager->GetSendBufferSize()),
	_recvCircularBuffer(socketManager->GetRecvBufferSize()),
	_sendCircularBuffer(socketManager->GetSendBufferSize())
{
	_socket = std::make_unique<RawSocket_t>(*socketManager->GetWorkerPool());
	_recvBuffer.resize(_recvBufferSize);
}

int TcpSocket::QueueAndSend(char* buffer, int length)
{
	std::lock_guard<std::recursive_mutex> lock(_sendMutex);

	if (_pendingDisconnect)
		return -1;

	// Add this packet to the circular buffer.
	// Ensure we do not allow resizing; we do not want these pointers invalidated.
	auto span = _sendCircularBuffer.PutData(buffer, length, false);
	if (span.Buffer1 != nullptr && span.Length1 > 0)
	{
		auto queuedSend        = std::make_unique<QueuedSend>();
		queuedSend->IsOwned    = false;
		queuedSend->BufferSpan = span;
		_sendQueue.push(std::move(queuedSend));
	}
	// Failed to add to the buffer, it has no room.
	// Allocate and queue.
	else
	{
		auto queuedSend                = std::make_unique<QueuedSend>();
		queuedSend->IsOwned            = true;
		queuedSend->BufferSpan.Buffer1 = new char[length];
		queuedSend->BufferSpan.Length1 = length;
		memcpy(queuedSend->BufferSpan.Buffer1, buffer, length);
		_sendQueue.push(std::move(queuedSend));
	}

	if (!AsyncSend(false))
		return -1;

	return length;
}

bool TcpSocket::AsyncSend(bool fromAsyncChain)
{
	std::unique_lock<std::recursive_mutex> lock(_sendMutex);

	// When we finish a send, we should pop the last entry before queueing up another send.
	if (fromAsyncChain)
	{
		assert(_sendInProgress);
		_sendInProgress = false;

		if (!_sendQueue.empty())
			_sendQueue.pop();

		if (_sendQueue.empty())
		{
			// Re-queue a disconnect now that all sends are complete
			if (_pendingDisconnect)
			{
				lock.unlock();
				Close();
			}

			// Send queue is empty, nothing more to queue up.
			// Consider this successful.
			return true;
		}
	}
	else
	{
		// Send queue is empty, nothing more to queue up.
		// Consider this successful.
		if (_sendQueue.empty())
			return true;

		// Send currently in progress.
		// Don't attempt to write; it's in the queue, it'll be processed once the send is completed.
		// Consider this successful; it is in the queue.
		if (_sendInProgress)
			return true;
	}

	if (_socket == nullptr)
		return false;

	// Fetch the next entry to send.
	// Note that we keep this in the queue until the send completes.
	const auto& queuedSend = _sendQueue.front();
	const auto& span       = queuedSend->BufferSpan;

	try
	{
		// Force shared pointer copy.
		auto self = shared_from_this();

		if (span.Buffer2 != nullptr && span.Length2 > 0)
		{
			std::array<asio::const_buffer, 2> buffers;
			buffers[0] = asio::buffer(span.Buffer1, span.Length1);
			buffers[1] = asio::buffer(span.Buffer2, span.Length2);

			_socket->async_write_some(buffers,
				[self](const asio::error_code& ec, size_t bytesTransferred)
				{ self->GetManager()->OnPostSend(ec, bytesTransferred, self.get()); });
		}
		else
		{
			_socket->async_write_some(asio::buffer(span.Buffer1, span.Length1),
				[self](const asio::error_code& ec, size_t bytesTransferred)
				{ self->GetManager()->OnPostSend(ec, bytesTransferred, self.get()); });
		}

		_sendInProgress = true;
	}
	catch (const asio::system_error& ex)
	{
		lock.unlock();

		spdlog::error("TcpSocket({})::AsyncSend: failed to post send. [socketId={} error={}]",
			GetImplName(), _socketId, ex.what());

		Close();
		return false;
	}

	return true;
}

void TcpSocket::AbortSend()
{
	std::lock_guard<std::recursive_mutex> lock(_sendMutex);

	while (!_sendQueue.empty())
		_sendQueue.pop();

	_sendInProgress = false;
}

void TcpSocket::AsyncReceive()
{
	if (_pendingDisconnect)
		return;

	memset(_recvBuffer.data(), 0, _recvBuffer.size());

	if (_socket == nullptr)
		return;

	try
	{
		// Force shared pointer copy.
		auto self = shared_from_this();
		_socket->async_read_some(asio::buffer(_recvBuffer),
			[self](const asio::error_code& ec, size_t bytesTransferred)
			{ self->GetManager()->OnPostReceive(ec, bytesTransferred, self.get()); });
	}
	catch (const asio::system_error& ex)
	{
		spdlog::error("TcpSocket({})::AsyncReceive: failed to post receive. [socketId={} error={}]",
			GetImplName(), _socketId, ex.what());
		Close();
	}
}

void TcpSocket::ReceivedData(int length)
{
	if (_pendingDisconnect)
		return;

	if (length <= 0)
		return;

	_recvCircularBuffer.PutData(_recvBuffer.data(), length);

	char* extractedPacket     = nullptr;
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

void TcpSocket::CloseProcess()
{
	_state = CONNECTION_STATE_DISCONNECTED;

	if (_socket != nullptr && _socket->is_open())
	{
		asio::error_code ec;
		_socket->shutdown(asio::socket_base::shutdown_both, ec);
		if (ec)
		{
			spdlog::error("TcpSocket({})::CloseProcess: shutdown() failed. [socketId={} error={}]",
				GetImplName(), _socketId, ec.message());
		}

		_socket->close(ec);
		if (ec)
		{
			spdlog::error("TcpSocket({})::CloseProcess: close() failed. [socketId={} error={}]",
				GetImplName(), _socketId, ec.message());
		}
	}

	{
		std::lock_guard<std::recursive_mutex> lock(_sendMutex);
		_sendInProgress = false;

		while (!_sendQueue.empty())
			_sendQueue.pop();
	}
}

void TcpSocket::InitSocket()
{
	_state = CONNECTION_STATE_CONNECTED;

	_sendCircularBuffer.SetEmpty();
	_recvCircularBuffer.SetEmpty();
	_socketErrorCount = 0;
	_remoteIp.clear();
	_remoteIpCached    = false;
	_sendInProgress    = false;
	_pendingDisconnect = false;

	Initialize();
}

void TcpSocket::Initialize()
{
	/* do nothing */
}

const std::string& TcpSocket::GetRemoteIP()
{
	if (!_remoteIpCached && _socket != nullptr)
	{
		asio::error_code ec;

		asio::ip::tcp::endpoint endpoint = _socket->remote_endpoint(ec);
		if (!ec)
		{
			_remoteIp       = endpoint.address().to_string();
			_remoteIpCached = true;
		}
		else
		{
			spdlog::warn("TcpSocket({})::GetRemoteIP: failed lookup. [socketId={}]", GetImplName(),
				_socketId);
		}
	}

	return _remoteIp;
}

void TcpSocket::SetSocket(RawSocket_t&& rawSocket)
{
	assert(_socket != nullptr);

	if (_socket != nullptr)
		*_socket = std::move(rawSocket);
}

void TcpSocket::Close()
{
	if (GetState() == CONNECTION_STATE_DISCONNECTED)
		return;

	{
		std::lock_guard<std::recursive_mutex> lock(_sendMutex);

		// From this point onward we're effectively disconnected.
		// We should stop handling or sending new packets, and just ensure any existing queued packets are sent.
		// Once all existing packets are sent, we can fully disconnect the socket.
		_pendingDisconnect = true;

		// Wait until the send chain is complete.
		// The send chain will trigger this again.
		if (_sendInProgress || !_sendQueue.empty())
			return;
	}

	asio::error_code ec;
	try
	{
		auto threadPool = _socketManager->GetWorkerPool();
		if (threadPool == nullptr)
			return;

		// Force shared pointer copy.
		auto self = shared_from_this();
		asio::post(*threadPool, [self]() { self->GetManager()->OnPostClose(self.get()); });
	}
	catch (const asio::system_error& ex)
	{
		spdlog::error("TcpSocket({})::Close: failed to post close. [socketId={} error={}]",
			GetImplName(), _socketId, ex.what());
	}
}

#include "stdafx.h"
#include "TcpSocket.h"
#include "SocketManager.h"

TcpSocket::TcpSocket(SocketManager* socketManager)
	: _socketManager(socketManager),
	_recvCircularBuffer(socketManager->GetRecvBufferSize()),
	_sendCircularBuffer(socketManager->GetSendBufferSize()),
	_socket(*socketManager->GetWorkerPool())
{
	_state = CONNECTION_STATE_DISCONNECTED;
	_socketId = -1;
	_sendInProgress = false;
	_socketErrorCount = 0;

	_remoteIpCached = false;
	_pendingDisconnect = false;

	_recvBufferSize = socketManager->GetRecvBufferSize();
	_sendBufferSize = socketManager->GetSendBufferSize();
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
		_ASSERT(_sendInProgress);
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
		// Send currently in progress.
		// Don't attempt to write; it's in the queue, it'll be processed once the send is completed.
		if (_sendInProgress)
			return false;

		// Send queue is empty, nothing more to queue up.
		// Consider this successful.
		if (_sendQueue.empty())
			return true;
	}

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
			std::bind(&SocketManager::OnPostSend, _socketManager, std::placeholders::_1, std::placeholders::_2, this));

		_sendInProgress = true;
	}
	catch (const asio::system_error& ex)
	{
		lock.unlock();

		spdlog::error("TcpSocket::AsyncSend: failed to post send for socketId={}: {}",
			_socketId, ex.what());
		Close();
		return false;
	}

	return true;
}

void TcpSocket::AsyncReceive()
{
	if (_pendingDisconnect)
		return;

	memset(_recvBuffer.data(), 0, _recvBuffer.size());

	try
	{
		_socket.async_read_some(asio::buffer(_recvBuffer),
			std::bind(&SocketManager::OnPostReceive, _socketManager, std::placeholders::_1, std::placeholders::_2, this));
	}
	catch (const asio::system_error& ex)
	{
		spdlog::error("TcpSocket::Receive: failed to post receive for socketId={}: {}",
			_socketId, ex.what());
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

void TcpSocket::CloseProcess()
{
	_state = CONNECTION_STATE_DISCONNECTED;

	if (_socket.is_open())
	{
		asio::error_code ec;
		_socket.shutdown(asio::socket_base::shutdown_both, ec);
		if (ec)
		{
			spdlog::error("TcpSocket::CloseProcess: shutdown() failed for socketId={}: {}",
				_socketId, ec.message());
		}

		_socket.close(ec);
		if (ec)
		{
			spdlog::error("TcpSocket::CloseProcess: close() failed for socketId={}: {}",
				_socketId, ec.message());
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
	_remoteIpCached = false;
	_sendInProgress = false;
	_pendingDisconnect = false;

	Initialize();
}

const std::string& TcpSocket::GetRemoteIP()
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
			spdlog::warn("TcpSocket::GetRemoteIP: failed lookup. socketId={}",
				_socketId);
		}
	}

	return _remoteIp;
}

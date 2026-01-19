#include "pch.h"
#include "TcpSocketManager.h"
#include "TcpSocket.h"
#include "TcpClientSocket.h"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cassert>

TcpSocketManager::TcpSocketManager(int recvBufferSize, int sendBufferSize) :
	_recvBufferSize(recvBufferSize), _sendBufferSize(sendBufferSize)
{
}

TcpSocketManager::~TcpSocketManager()
{
	// NOTE: We expect the implementer of this class to shutdown appropriately.
	assert(_io.stopped());
	assert(_workerPool == nullptr);
}

void TcpSocketManager::Init(
	int serverSocketCount, int clientSocketCount, uint32_t workerThreadCount /*= 0*/)
{
	_socketCount       = serverSocketCount;
	_clientSocketCount = clientSocketCount;

	_socketArray.resize(serverSocketCount);
	_inactiveSocketArray.resize(serverSocketCount);

	_clientSocketArray = new TcpClientSocket*[clientSocketCount];

	// NOTE: Specifically allocate the worker pool first, as we'll need this for our sockets.
	if (workerThreadCount == 0)
		_workerThreadCount = std::thread::hardware_concurrency() * 2;
	else
		_workerThreadCount = workerThreadCount;

	_workerPool = std::make_shared<asio::thread_pool>(_workerThreadCount);

	std::queue<int> socketIdQueue;
	for (int i = 0; i < serverSocketCount; i++)
		socketIdQueue.push(i);

	for (int i = 0; i < clientSocketCount; i++)
		_clientSocketArray[i] = nullptr;

	// NOTE: These don't strictly need to be guarded as the server's not yet operational,
	// but we do it for consistency.
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		_socketIdQueue.swap(socketIdQueue);
	}

	if (_startUserThreadCallback != nullptr)
		_startUserThreadCallback();
}

TcpSocket* TcpSocketManager::AcquireSocket(int& socketId)
{
	if (_socketIdQueue.empty())
		return nullptr;

	socketId = _socketIdQueue.front();

	// This is all self-contained so it should never be out of range.
	assert(socketId >= 0 && socketId < _socketCount);

	TcpSocket* tcpSocket = _inactiveSocketArray[socketId];
	if (tcpSocket == nullptr)
		return nullptr;

	_socketIdQueue.pop();

	_socketArray[socketId]         = tcpSocket;
	_inactiveSocketArray[socketId] = nullptr;

	tcpSocket->SetSocketID(socketId);
	return tcpSocket;
}

void TcpSocketManager::ReleaseSocket(TcpSocket* tcpSocket, int socketId)
{
	if (socketId < 0 || socketId >= _socketCount)
	{
		spdlog::error("TcpSocketManager::ReleaseServerSocket: out of range socketId={}", socketId);
		return;
	}

	_socketIdQueue.push(socketId);

	if (tcpSocket != nullptr)
	{
		_socketArray[socketId]         = nullptr;
		_inactiveSocketArray[socketId] = tcpSocket;
	}
}

bool TcpSocketManager::AcquireClientSocket(TcpClientSocket* tcpClientSocket)
{
	std::lock_guard<std::recursive_mutex> lock(_mutex);

	int socketId = GetAvailableClientSocketId();
	if (socketId < 0)
		return false;

	_clientSocketArray[socketId] = tcpClientSocket;
	tcpClientSocket->SetSocketID(socketId);
	return true;
}

void TcpSocketManager::ReleaseClientSocket(int socketId)
{
	if (socketId < 0 || socketId >= _clientSocketCount)
	{
		spdlog::error("TcpSocketManager::ReleaseClientSocket: out of range socketId={}", socketId);
		return;
	}

	// NOTE: These are managed externally, so we only have to detach them.
	_clientSocketArray[socketId] = nullptr;
}

int TcpSocketManager::GetAvailableClientSocketId() const
{
	for (int i = 0; i < _clientSocketCount; i++)
	{
		if (_clientSocketArray[i] == nullptr)
			return i;
	}

	return -1;
}

void TcpSocketManager::OnPostReceive(
	const asio::error_code& ec, size_t bytesTransferred, TcpSocket* tcpSocket)
{
	if (ec)
	{
		if (ec == asio::error::eof)
		{
			spdlog::debug("TcpSocketManager::OnPostReceive: peer closed connection. socketId={}",
				tcpSocket->GetSocketID());
		}
		else
		{
			spdlog::debug("TcpSocketManager::OnPostReceive: socketId={} error={}",
				tcpSocket->GetSocketID(), ec.message());

			if (++tcpSocket->_socketErrorCount < 2)
				return;
		}

		ProcessClose(tcpSocket);
		return;
	}

	if (bytesTransferred == 0)
	{
		spdlog::debug("TcpSocketManager::OnPostReceive: closed by 0 byte notify. socketId={}",
			tcpSocket->GetSocketID());
		ProcessClose(tcpSocket);
		return;
	}

	// NOTE: This is guarded officially, forcing the server to be effectively single-threaded as all parsing
	// and logic must be guarded under this mutex.
	// In the future, this crutch should be removed, but this continues to preserve official behaviour so
	// everything still behaves as it expects.
	std::lock_guard<std::recursive_mutex> lock(_mutex);

	tcpSocket->ReceivedData(static_cast<int>(bytesTransferred));
	tcpSocket->AsyncReceive();
}

void TcpSocketManager::OnPostSend(
	const asio::error_code& ec, size_t /*bytesTransferred*/, TcpSocket* tcpSocket)
{
	if (ec)
	{
		spdlog::error("TcpSocketManager::OnPostSend: socketId={} failed: {}",
			tcpSocket->GetSocketID(), ec.message());

		tcpSocket->Close();
		return;
	}

	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		tcpSocket->_socketErrorCount = 0;
	}

	// Pop this queued entry & dispatch next queued send if applicable.
	tcpSocket->AsyncSend(true);
}

void TcpSocketManager::OnPostSocketClose(TcpSocket* tcpSocket)
{
	if (!ProcessClose(tcpSocket))
		return;

	spdlog::debug("TcpSocketManager::OnPostSocketClose: socket closed by Close() socketId={}",
		tcpSocket->GetSocketID());
}

bool TcpSocketManager::ProcessClose(TcpSocket* tcpSocket)
{
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	if (tcpSocket->GetState() == CONNECTION_STATE_DISCONNECTED)
		return false;

	tcpSocket->CloseProcess();
	tcpSocket->ReleaseToManager();

	return true;
}

void TcpSocketManager::ShutdownImpl()
{
	// Shutdown any user-created threads
	if (_shutdownUserThreadCallback != nullptr)
		_shutdownUserThreadCallback();

	// Explicitly disconnect all sockets now.
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);

		for (int i = 0; i < _socketCount; i++)
		{
			TcpSocket* tcpSocket = _socketArray[i];
			if (tcpSocket == nullptr)
				continue;

			// Invoke immediate save and disconnect from within this thread
			tcpSocket->CloseProcess();
			ReleaseSocket(tcpSocket, i);
		}

		if (_clientSocketArray != nullptr)
		{
			for (int i = 0; i < _clientSocketCount; i++)
			{
				TcpClientSocket* tcpClientSocket = _clientSocketArray[i];
				if (tcpClientSocket == nullptr)
					continue;

				// Invoke immediate disconnect from within this thread
				tcpClientSocket->CloseProcess();
				ReleaseClientSocket(i);
			}
		}
	}

	// Force worker threads to finish up work.
	_io.stop();

	// Wait for the worker threads to finish.
	if (_workerPool != nullptr)
	{
		_workerPool->stop();
		_workerPool->join();
	}

	// Free our sessions.
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);

		for (TcpSocket* socket : _socketArray)
			delete socket;
		_socketArray.clear();

		for (TcpSocket* socket : _inactiveSocketArray)
			delete socket;
		_inactiveSocketArray.clear();

		// We don't own these instances so we should only free the array.
		delete[] _clientSocketArray;
		_clientSocketArray = nullptr;

		_socketCount       = 0;
		_clientSocketCount = 0;
	}

	// Finally free the worker pool; it needs to exist while otherwise tied to sessions.
	if (_workerPool != nullptr)
		_workerPool.reset();
}

#include "pch.h"
#include "TcpSocketManager.h"
#include "TcpSocket.h"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cassert>

TcpSocketManager::TcpSocketManager(
	int recvBufferSize, int sendBufferSize, std::string_view implName) :
	_recvBufferSize(recvBufferSize), _sendBufferSize(sendBufferSize),
	_implName(implName.data(), implName.length())
{
}

TcpSocketManager::~TcpSocketManager()
{
	// NOTE: We expect the implementer of this class to shutdown appropriately.
	assert(_io.stopped());
	assert(_workerPool == nullptr);
}

void TcpSocketManager::Init(int socketCount, uint32_t workerThreadCount /*= 0*/)
{
	_socketCount = socketCount;

	_socketArray.resize(socketCount);
	_inactiveSocketArray.resize(socketCount);

	// NOTE: Specifically allocate the worker pool first, as we'll need this for our sockets.
	if (workerThreadCount == 0)
		_workerThreadCount = std::thread::hardware_concurrency() * 2;
	else
		_workerThreadCount = workerThreadCount;

	_workerPool = std::make_shared<asio::thread_pool>(_workerThreadCount);

	std::queue<int> socketIdQueue;
	for (int i = 0; i < socketCount; i++)
		socketIdQueue.push(i);

	// NOTE: These don't strictly need to be guarded as the manager's not yet operational,
	// but we do it for consistency.
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		_socketIdQueue.swap(socketIdQueue);
	}

	if (_startUserThreadCallback != nullptr)
		_startUserThreadCallback();
}

std::shared_ptr<TcpSocket> TcpSocketManager::AcquireSocket(int& socketId)
{
	if (_socketIdQueue.empty())
		return nullptr;

	socketId = _socketIdQueue.front();

	// This is all self-contained so it should never be out of range.
	assert(socketId >= 0 && socketId < _socketCount);

	auto tcpSocket = _inactiveSocketArray[socketId];
	if (tcpSocket == nullptr)
		return nullptr;

	_socketIdQueue.pop();

	_socketArray[socketId]         = tcpSocket;
	_inactiveSocketArray[socketId] = nullptr;

	return tcpSocket;
}

void TcpSocketManager::ReleaseSocket(std::shared_ptr<TcpSocket> tcpSocket)
{
	if (tcpSocket == nullptr)
	{
		spdlog::error("TcpSocketManager({})::ReleaseSocket: tcpSocket is nullptr", _implName);
		return;
	}

	int socketId = tcpSocket->GetSocketID();
	_socketIdQueue.push(socketId);

	_socketArray[socketId]         = nullptr;
	_inactiveSocketArray[socketId] = std::move(tcpSocket);
}

void TcpSocketManager::OnPostReceive(
	const asio::error_code& ec, size_t bytesTransferred, TcpSocket* tcpSocket)
{
	if (ec)
	{
		if (ec == asio::error::eof)
		{
			spdlog::debug(
				"TcpSocketManager({})::OnPostReceive: peer closed connection. [socketId={}]",
				_implName, tcpSocket->GetSocketID());
		}
		else
		{
			spdlog::debug(
				"TcpSocketManager({})::OnPostReceive: unexpected error [socketId={} error={}]",
				_implName, tcpSocket->GetSocketID(), ec.message());

			if (++tcpSocket->_socketErrorCount < 2)
				return;
		}

		ProcessClose(tcpSocket);
		return;
	}

	if (bytesTransferred == 0)
	{
		spdlog::debug("TcpSocketManager({})::OnPostReceive: closed by 0 byte notify. [socketId={}]",
			_implName, tcpSocket->GetSocketID());
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
		spdlog::error("TcpSocketManager({})::OnPostSend: unexpected error [socketId={} error={}]",
			_implName, tcpSocket->GetSocketID(), ec.message());

		tcpSocket->AbortSend();
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

void TcpSocketManager::OnPostClose(TcpSocket* tcpSocket)
{
	if (!ProcessClose(tcpSocket))
		return;

	spdlog::debug("TcpSocketManager({})::OnPostClose: socket closed by Close() [socketId={}]",
		_implName, tcpSocket->GetSocketID());
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
			auto tcpSocket = _socketArray[i];
			if (tcpSocket == nullptr)
				continue;

			// Invoke immediate save and disconnect from within this thread
			tcpSocket->CloseProcess();
			ReleaseSocket(std::move(tcpSocket));
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

		_socketArray.clear();
		_inactiveSocketArray.clear();
		_socketCount = 0;
	}

	// Finally free the worker pool; it needs to exist while otherwise tied to sessions.
	if (_workerPool != nullptr)
		_workerPool.reset();
}

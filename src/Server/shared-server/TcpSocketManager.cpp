#include "pch.h"
#include "TcpSocketManager.h"
#include "TcpSocket.h"
#include "TcpClientSocket.h"

#include <spdlog/spdlog.h>

#include <algorithm>

TcpSocketManager::TcpSocketManager(int recvBufferSize, int sendBufferSize) :
	_recvBufferSize(recvBufferSize), _sendBufferSize(sendBufferSize)
{
}

TcpSocketManager::~TcpSocketManager()
{
	try
	{
		Shutdown();
	}
	catch (const std::exception& ex)
	{
		spdlog::error("TcpSocketManager::~TcpSocketManager: exception occurred - {}", ex.what());
	}
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

bool TcpSocketManager::Listen(int port)
{
	try
	{
		asio::error_code ec;

		// Attempt to setup the acceptor.
		_acceptor = std::make_unique<asio::ip::tcp::acceptor>(_workerPool->get_executor());

		// Setup the endpoint for TCPv4 0.0.0.0:port
		asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port);

		// Attempt to open the socket.
		_acceptor->open(endpoint.protocol(), ec);
		if (ec)
		{
			spdlog::error("TcpSocketManager::Listen: open() failed: {}", ec.message());
			return false;
		}

		// Attempt to bind the socket.
		_acceptor->bind(endpoint, ec);
		if (ec)
		{
			spdlog::error(
				"TcpSocketManager::Listen: bind() failed on 0.0.0.0:{}: {}", port, ec.message());
			return false;
		}

		// Allow address reuse (i.e. rebinding to the same port)
		_acceptor->set_option(asio::socket_base::reuse_address(true), ec);
		if (ec)
		{
			spdlog::error(
				"TcpSocketManager::Listen: set_option(reuse_address) failed: {}", ec.message());
			return false;
		}

		// Configure receive buffer size
		_acceptor->set_option(asio::socket_base::receive_buffer_size(_recvBufferSize * 4), ec);
		if (ec)
		{
			spdlog::error("TcpSocketManager::Listen: set_option(receive_buffer_size) failed: {}",
				ec.message());
			return false;
		}

		// Configure send buffer size
		_acceptor->set_option(asio::socket_base::send_buffer_size(_sendBufferSize * 4), ec);
		if (ec)
		{
			spdlog::error(
				"TcpSocketManager::Listen: set_option(send_buffer_size) failed: {}", ec.message());
			return false;
		}

		// Start listening with a backlog of 5
		_acceptor->listen(5, ec);
		if (ec)
		{
			spdlog::error("TcpSocketManager::Listen: listen() failed: {}", ec.message());
			return false;
		}
	}
	catch (const asio::system_error& ex)
	{
		spdlog::error(
			"TcpSocketManager::Listen: failed to bind on 0.0.0.0:{}: {}", port, ex.what());
		return false;
	}

	spdlog::info("TcpSocketManager::Listen: initialized port={:05}", port);
	return true;
}

void TcpSocketManager::StartAccept()
{
	if (_acceptingConnections.exchange(true))
	{
		// already accepting connections
		return;
	}

	AsyncAccept();
}

void TcpSocketManager::StopAccept()
{
	_acceptingConnections.store(false);

	if (_acceptor != nullptr && _acceptor->is_open())
	{
		asio::error_code ec;
		_acceptor->cancel(ec);

		if (ec)
			spdlog::error("TcpSocketManager::StopAccept: cancel() failed: {}", ec.message());
	}
}

void TcpSocketManager::AsyncAccept()
{
	if (!_acceptingConnections.load())
		return;

	try
	{
		_acceptor->async_accept(
			[this](const asio::error_code& ec, asio::ip::tcp::socket rawSocket)
			{
				if (!ec)
				{
					if (!_acceptingConnections.load())
					{
						rawSocket.close();
						return;
					}

					OnAccept(rawSocket);
				}
				else
				{
					if (ec == asio::error::operation_aborted)
					{
						spdlog::debug("TcpSocketManager::AsyncAccept: accept operation cancelled");
					}
					else
					{
						spdlog::error(
							"TcpSocketManager::AsyncAccept: accept failed: {}", ec.message());
					}
				}

				AsyncAccept();
			});
	}
	catch (const asio::system_error& ex)
	{
		spdlog::error("TcpSocketManager::AsyncAccept: async_accept() failed: {}", ex.what());
	}
}

void TcpSocketManager::OnAccept(asio::ip::tcp::socket& rawSocket)
{
	int socketId         = -1;
	TcpSocket* tcpSocket = nullptr;

	// NOTE: Handle the guarding externally so it's clear what's guarded and what's not,
	// which is critical when dealing with code needing to be fairly high performance here.
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		tcpSocket = AcquireSocket(socketId);
	}

	if (socketId == -1)
	{
		spdlog::error("TcpSocketManager::OnAccept: socketId list is empty");
		return;
	}

	// This should never happen.
	// If it does, the associated socket ID was never removed from the list so we don't have to restore it.
	if (tcpSocket == nullptr)
	{
		spdlog::error("TcpSocketManager::OnAccept: null socket [socketId:{}]", socketId);
		return;
	}

	if (tcpSocket->_socket == nullptr)
	{
		spdlog::error(
			"TcpSocketManager::OnAccept: no raw socket allocated [socketId:{}]", socketId);
		return;
	}

	*tcpSocket->_socket = std::move(rawSocket);

	tcpSocket->InitSocket();
	tcpSocket->AsyncReceive();

	spdlog::debug("TcpSocketManager::AcceptThread: successfully accepted socketId={}", socketId);
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

void TcpSocketManager::Shutdown()
{
	// Stop accepting new connections
	StopAccept();

	// Reset the acceptor.
	if (_acceptor != nullptr)
		_acceptor.reset();

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

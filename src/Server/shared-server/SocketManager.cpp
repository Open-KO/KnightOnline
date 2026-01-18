#include "pch.h"
#include "SocketManager.h"
#include "TcpSocket.h"
#include "TcpClientSocket.h"

#include <algorithm>
#include <spdlog/spdlog.h>

SocketManager::SocketManager(int recvBufferSize, int sendBufferSize) :
	_recvBufferSize(recvBufferSize), _sendBufferSize(sendBufferSize)
{
}

SocketManager::~SocketManager()
{
	try
	{
		Shutdown();
	}
	catch (const std::exception& ex)
	{
		spdlog::error("SocketManager::~SocketManager: exception occurred - {}", ex.what());
	}
}

void SocketManager::Init(
	int serverSocketCount, int clientSocketCount, uint32_t workerThreadCount /*= 0*/)
{
	_serverSocketCount         = serverSocketCount;
	_clientSocketCount         = clientSocketCount;

	_serverSocketArray         = new TcpSocket*[serverSocketCount];
	_inactiveServerSocketArray = new TcpSocket*[serverSocketCount];
	_clientSocketArray         = new TcpClientSocket*[clientSocketCount];

	// NOTE: Specifically allocate the worker pool first, as we'll need this for our sockets.
	if (workerThreadCount == 0)
		_workerThreadCount = std::thread::hardware_concurrency() * 2;
	else
		_workerThreadCount = workerThreadCount;

	_workerPool = std::make_shared<asio::thread_pool>(_workerThreadCount);

	std::queue<int> socketIdQueue;
	for (int i = 0; i < serverSocketCount; i++)
	{
		_serverSocketArray[i]         = nullptr;
		_inactiveServerSocketArray[i] = nullptr;

		socketIdQueue.push(i);
	}

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

bool SocketManager::Listen(int port)
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
			spdlog::error("SocketManager::Listen: open() failed: {}", ec.message());
			return false;
		}

		// Attempt to bind the socket.
		_acceptor->bind(endpoint, ec);
		if (ec)
		{
			spdlog::error(
				"SocketManager::Listen: bind() failed on 0.0.0.0:{}: {}", port, ec.message());
			return false;
		}

		// Allow address reuse (i.e. rebinding to the same port)
		_acceptor->set_option(asio::socket_base::reuse_address(true), ec);
		if (ec)
		{
			spdlog::error(
				"SocketManager::Listen: set_option(reuse_address) failed: {}", ec.message());
			return false;
		}

		// Configure receive buffer size
		_acceptor->set_option(asio::socket_base::receive_buffer_size(_recvBufferSize * 4), ec);
		if (ec)
		{
			spdlog::error(
				"SocketManager::Listen: set_option(receive_buffer_size) failed: {}", ec.message());
			return false;
		}

		// Configure send buffer size
		_acceptor->set_option(asio::socket_base::send_buffer_size(_sendBufferSize * 4), ec);
		if (ec)
		{
			spdlog::error(
				"SocketManager::Listen: set_option(send_buffer_size) failed: {}", ec.message());
			return false;
		}

		// Start listening with a backlog of 5
		_acceptor->listen(5, ec);
		if (ec)
		{
			spdlog::error("SocketManager::Listen: listen() failed: {}", ec.message());
			return false;
		}
	}
	catch (const asio::system_error& ex)
	{
		spdlog::error("SocketManager::Listen: failed to bind on 0.0.0.0:{}: {}", port, ex.what());
		return false;
	}

	spdlog::info("SocketManager::Listen: initialized port={:05}", port);
	return true;
}

void SocketManager::StartAccept()
{
	if (_acceptingConnections.exchange(true))
	{
		// already accepting connections
		return;
	}

	AsyncAccept();
}

void SocketManager::StopAccept()
{
	_acceptingConnections.store(false);

	if (_acceptor != nullptr && _acceptor->is_open())
	{
		asio::error_code ec;
		_acceptor->cancel(ec);

		if (ec)
			spdlog::error("SocketManager::StopAccept: cancel() failed: {}", ec.message());
	}
}

void SocketManager::AsyncAccept()
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
						spdlog::debug("SocketManager::AsyncAccept: accept operation cancelled");
					else
						spdlog::error(
							"SocketManager::AsyncAccept: accept failed: {}", ec.message());
				}

				AsyncAccept();
			});
	}
	catch (const asio::system_error& ex)
	{
		spdlog::error("SocketManager::AsyncAccept: async_accept() failed: {}", ex.what());
	}
}

void SocketManager::OnAccept(asio::ip::tcp::socket& rawSocket)
{
	int socketId         = -1;
	TcpSocket* tcpSocket = nullptr;

	// NOTE: Handle the guarding externally so it's clear what's guarded and what's not,
	// which is critical when dealing with code needing to be fairly high performance here.
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		tcpSocket = AcquireServerSocket(socketId);
	}

	if (socketId == -1)
	{
		spdlog::error("SocketManager::OnAccept: socketId list is empty");
		return;
	}

	// This should never happen.
	// If it does, the associated socket ID was never removed from the list so we don't have to restore it.
	if (tcpSocket == nullptr)
	{
		spdlog::error("SocketManager::OnAccept: null socket [socketId:{}]", socketId);
		return;
	}

	if (tcpSocket->_socket == nullptr)
	{
		spdlog::error("SocketManager::OnAccept: no raw socket allocated [socketId:{}]", socketId);
		return;
	}

	*tcpSocket->_socket = std::move(rawSocket);

	tcpSocket->InitSocket();
	tcpSocket->AsyncReceive();

	spdlog::debug("SocketManager::AcceptThread: successfully accepted socketId={}", socketId);
}

TcpSocket* SocketManager::AcquireServerSocket(int& socketId)
{
	if (_socketIdQueue.empty())
		return nullptr;

	socketId = _socketIdQueue.front();

	// This is all self-contained so it should never be out of range.
	assert(socketId >= 0 && socketId < _serverSocketCount);

	TcpSocket* tcpSocket = _inactiveServerSocketArray[socketId];
	if (tcpSocket == nullptr)
		return nullptr;

	_socketIdQueue.pop();

	_serverSocketArray[socketId]         = tcpSocket;
	_inactiveServerSocketArray[socketId] = nullptr;

	tcpSocket->SetSocketID(socketId);
	return tcpSocket;
}

void SocketManager::ReleaseServerSocket(TcpSocket* tcpSocket, int socketId)
{
	if (socketId < 0 || socketId >= _serverSocketCount)
	{
		spdlog::error("SocketManager::ReleaseServerSocket: out of range socketId={}", socketId);
		return;
	}

	_socketIdQueue.push(socketId);

	if (tcpSocket != nullptr)
	{
		_serverSocketArray[socketId]         = nullptr;
		_inactiveServerSocketArray[socketId] = tcpSocket;
	}
}

bool SocketManager::AcquireClientSocket(TcpClientSocket* tcpClientSocket)
{
	std::lock_guard<std::recursive_mutex> lock(_mutex);

	int socketId = GetAvailableClientSocketId();
	if (socketId < 0)
		return false;

	_clientSocketArray[socketId] = tcpClientSocket;
	tcpClientSocket->SetSocketID(socketId);
	return true;
}

void SocketManager::ReleaseClientSocket(int socketId)
{
	if (socketId < 0 || socketId >= _clientSocketCount)
	{
		spdlog::error("SocketManager::ReleaseClientSocket: out of range socketId={}", socketId);
		return;
	}

	// NOTE: These are managed externally, so we only have to detach them.
	_clientSocketArray[socketId] = nullptr;
}

int SocketManager::GetAvailableClientSocketId() const
{
	for (int i = 0; i < _clientSocketCount; i++)
	{
		if (_clientSocketArray[i] == nullptr)
			return i;
	}

	return -1;
}

void SocketManager::OnPostReceive(
	const asio::error_code& ec, size_t bytesTransferred, TcpSocket* tcpSocket)
{
	if (ec)
	{
		if (ec == asio::error::eof)
		{
			spdlog::debug("SocketManager::OnPostReceive: peer closed connection. socketId={}",
				tcpSocket->GetSocketID());
		}
		else
		{
			spdlog::debug("SocketManager::OnPostReceive: socketId={} error={}",
				tcpSocket->GetSocketID(), ec.message());

			if (++tcpSocket->_socketErrorCount < 2)
				return;
		}

		ProcessClose(tcpSocket);
		return;
	}

	if (bytesTransferred == 0)
	{
		spdlog::debug("SocketManager::OnPostReceive: closed by 0 byte notify. socketId={}",
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

void SocketManager::OnPostSend(
	const asio::error_code& ec, size_t /*bytesTransferred*/, TcpSocket* tcpSocket)
{
	if (ec)
	{
		spdlog::error("SocketManager::OnPostSend: socketId={} failed: {}", tcpSocket->GetSocketID(),
			ec.message());

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

void SocketManager::OnPostServerSocketClose(TcpSocket* tcpSocket)
{
	if (!ProcessClose(tcpSocket))
		return;

	spdlog::debug("SocketManager::OnPostServerSocketClose: socket closed by Close() socketId={}",
		tcpSocket->GetSocketID());
}

void SocketManager::OnPostClientSocketClose(TcpClientSocket* tcpSocket)
{
	if (!ProcessClose(tcpSocket))
		return;

	spdlog::debug("SocketManager::OnPostClientSocketClose: socket closed by Close() socketId={}",
		tcpSocket->GetSocketID());
}

bool SocketManager::ProcessClose(TcpSocket* tcpSocket)
{
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	if (tcpSocket->GetState() == CONNECTION_STATE_DISCONNECTED)
		return false;

	tcpSocket->CloseProcess();
	tcpSocket->ReleaseToManager();

	return true;
}

void SocketManager::Shutdown()
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

		if (_serverSocketArray != nullptr)
		{
			for (int i = 0; i < _serverSocketCount; i++)
			{
				TcpSocket* tcpSocket = _serverSocketArray[i];
				if (tcpSocket == nullptr)
					continue;

				// Invoke immediate save and disconnect from within this thread
				tcpSocket->CloseProcess();
				ReleaseServerSocket(tcpSocket, i);
			}
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

		for (int i = 0; i < _serverSocketCount; i++)
		{
			// NOTE: We're intentionally checking the outer array here and not the instance
			// in case the array was already freed.
			if (_serverSocketArray != nullptr)
			{
				delete _serverSocketArray[i];
				_serverSocketArray[i] = nullptr;
			}

			// NOTE: We're intentionally checking the outer array here and not the instance
			// in case the array was already freed.
			if (_inactiveServerSocketArray != nullptr)
			{
				delete _inactiveServerSocketArray[i];
				_inactiveServerSocketArray[i] = nullptr;
			}
		}

		delete[] _serverSocketArray;
		delete[] _inactiveServerSocketArray;

		_serverSocketArray         = nullptr;
		_inactiveServerSocketArray = nullptr;

		// We don't own these instances so we should only free the array.
		delete[] _clientSocketArray;
		_clientSocketArray = nullptr;
	}

	// Finally free the worker pool; it needs to exist while otherwise tied to sessions.
	if (_workerPool != nullptr)
		_workerPool.reset();
}

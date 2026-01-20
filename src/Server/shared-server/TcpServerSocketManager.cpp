#include "pch.h"
#include "TcpServerSocketManager.h"
#include "TcpSocket.h"

TcpServerSocketManager::TcpServerSocketManager(int recvBufferSize, int sendBufferSize,
	std::string_view implName) : TcpSocketManager(recvBufferSize, sendBufferSize, implName)
{
}

TcpServerSocketManager::~TcpServerSocketManager()
{
	try
	{
		Shutdown();
	}
	catch (const std::exception& ex)
	{
		spdlog::error(
			"TcpServerSocketManager({})::~TcpServerSocketManager: exception occurred - {}",
			_implName, ex.what());
	}
}

bool TcpServerSocketManager::Listen(int port)
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
			spdlog::error("TcpServerSocketManager({})::Listen: open() failed. [error={}]",
				_implName, ec.message());
			return false;
		}

		// Attempt to bind the socket.
		_acceptor->bind(endpoint, ec);
		if (ec)
		{
			spdlog::error(
				"TcpServerSocketManager({})::Listen: bind() failed on 0.0.0.0:{}. [error={}]",
				_implName, port, ec.message());
			return false;
		}

		// Allow address reuse (i.e. rebinding to the same port)
		_acceptor->set_option(asio::socket_base::reuse_address(true), ec);
		if (ec)
		{
			spdlog::error(
				"TcpServerSocketManager({})::Listen: set_option(reuse_address) failed. [error={}]",
				_implName, ec.message());
			return false;
		}

		// Configure receive buffer size
		_acceptor->set_option(asio::socket_base::receive_buffer_size(_recvBufferSize * 4), ec);
		if (ec)
		{
			spdlog::error("TcpServerSocketManager({})::Listen: set_option(receive_buffer_size) "
						  "failed. [error={}]",
				_implName, ec.message());
			return false;
		}

		// Configure send buffer size
		_acceptor->set_option(asio::socket_base::send_buffer_size(_sendBufferSize * 4), ec);
		if (ec)
		{
			spdlog::error("TcpServerSocketManager({})::Listen: set_option(send_buffer_size) "
						  "failed. [error={}]",
				_implName, ec.message());
			return false;
		}

		// Start listening with a backlog of 5
		_acceptor->listen(5, ec);
		if (ec)
		{
			spdlog::error("TcpServerSocketManager({})::Listen: listen() failed. [error={}]",
				_implName, ec.message());
			return false;
		}
	}
	catch (const asio::system_error& ex)
	{
		spdlog::error(
			"TcpServerSocketManager({})::Listen: failed to bind on 0.0.0.0:{}. [error={}]",
			_implName, port, ex.what());
		return false;
	}

	spdlog::info("TcpServerSocketManager({})::Listen: initialized port={:05}", _implName, port);
	return true;
}

void TcpServerSocketManager::Shutdown()
{
	// Stop accepting new connections
	StopAccept();

	// Reset the acceptor.
	if (_acceptor != nullptr)
		_acceptor.reset();

	ShutdownImpl();
}

void TcpServerSocketManager::StartAccept()
{
	if (_acceptingConnections.exchange(true))
	{
		// already accepting connections
		return;
	}

	AsyncAccept();
}

void TcpServerSocketManager::StopAccept()
{
	_acceptingConnections.store(false);

	if (_acceptor != nullptr && _acceptor->is_open())
	{
		asio::error_code ec;
		_acceptor->cancel(ec);

		if (ec)
		{
			spdlog::error("TcpServerSocketManager({})::StopAccept: cancel() failed. [error={}]",
				_implName, ec.message());
		}
	}
}

void TcpServerSocketManager::AsyncAccept()
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
						spdlog::debug(
							"TcpServerSocketManager({})::AsyncAccept: accept operation cancelled",
							_implName);
					}
					else
					{
						spdlog::error(
							"TcpServerSocketManager({})::AsyncAccept: accept failed. [error={}]",
							_implName, ec.message());
					}
				}

				AsyncAccept();
			});
	}
	catch (const asio::system_error& ex)
	{
		spdlog::error("TcpServerSocketManager({})::AsyncAccept: async_accept() failed. [error={}]",
			_implName, ex.what());
	}
}

void TcpServerSocketManager::OnAccept(asio::ip::tcp::socket& rawSocket)
{
	int socketId = -1;
	std::shared_ptr<TcpSocket> tcpSocket;

	// NOTE: Handle the guarding externally so it's clear what's guarded and what's not,
	// which is critical when dealing with code needing to be fairly high performance here.
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		tcpSocket = AcquireSocket(socketId);
	}

	if (socketId == -1)
	{
		spdlog::error("TcpServerSocketManager({})::OnAccept: socketId list is empty", _implName);
		return;
	}

	// This should never happen.
	// If it does, the associated socket ID was never removed from the list so we don't have to restore it.
	if (tcpSocket == nullptr)
	{
		spdlog::error(
			"TcpServerSocketManager({})::OnAccept: null socket [socketId={}]", _implName, socketId);
		return;
	}

	if (!tcpSocket->HasSocket())
	{
		spdlog::error("TcpServerSocketManager({})::OnAccept: no raw socket allocated [socketId={}]",
			_implName, socketId);
		return;
	}

	tcpSocket->SetSocket(std::move(rawSocket));
	tcpSocket->InitSocket();
	tcpSocket->AsyncReceive();

	spdlog::debug("TcpServerSocketManager({})::AcceptThread: successfully accepted [socketId={}]",
		_implName, socketId);
}

bool TcpServerSocketManager::ProcessClose(TcpSocket* tcpSocket)
{
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	if (tcpSocket->GetState() == CONNECTION_STATE_DISCONNECTED)
		return false;

	tcpSocket->CloseProcess();
	ReleaseSocket(tcpSocket->shared_from_this());

	return true;
}

#include "pch.h"
#include "TelnetThread.h"
#include "TelnetClientThread.h"

#include <spdlog/spdlog.h>

#include <chrono>
#include <ranges>

using namespace std::chrono_literals;

TelnetThread::TelnetThread(const std::string& listenAddress, const uint16_t port,
	std::unordered_set<std::string>&& addressWhitelist) :
	_listenAddress(listenAddress), _port(port), _addressWhitelist(std::move(addressWhitelist))
{
	_workerThreadCount = 1;
	_workerPool        = std::make_shared<asio::thread_pool>(_workerThreadCount);
	_nextSocketId      = 0;
}

TelnetThread::~TelnetThread()
{
	spdlog::debug("TelnetThread::~TelnetThread()");

	if (_acceptor != nullptr)
	{
		_acceptor.reset();
	}
}

void TelnetThread::thread_loop()
{
	if (!Listen())
	{
		spdlog::error("TelnetThread::thread_loop(): listen() failed");
		shutdown(false);
	}

	// start accepting connections
	AsyncAccept();

	// periodically check for client connection threads that need to be joined
	while (CanTick())
	{
		std::unordered_set<uint32_t> eraseKeys;
		for (auto& [key, clientThread] : _telnetThreadMap)
		{
			if (clientThread == nullptr)
			{
				// shouldn't happen
				eraseKeys.insert(key);
				continue;
			}

			if (clientThread->IsShutdown())
			{
				spdlog::debug(
					"TelnetThread::thread_loop: Joining TelnetClientThread with id: {}", key);
				clientThread->join();
				eraseKeys.insert(key);
				spdlog::debug("TelnetThread::thread_loop: TelnetClientThread joined");
			}
		}

		for (const uint32_t key : eraseKeys)
			_telnetThreadMap.erase(key);

		std::this_thread::sleep_for(100ms);
	}
}

void TelnetThread::before_shutdown()
{
	if (_acceptor != nullptr && _acceptor->is_open())
	{
		asio::error_code ec;
		_acceptor->cancel(ec);

		if (ec)
			spdlog::error("TelnetThread::before_shutdown: cancel() failed: {}", ec.message());
	}

	if (!_telnetThreadMap.empty())
	{
		spdlog::info("TelnetThread::before_shutdown: closing {} client connections",
			_telnetThreadMap.size());

		for (auto& clientSocket : _telnetThreadMap | std::views::values)
		{
			if (clientSocket != nullptr)
				clientSocket->Disconnect();
		}

		_io.stop();
	}

	for (auto& clientSocket : _telnetThreadMap | std::views::values)
	{
		if (clientSocket != nullptr)
			clientSocket->shutdown();
	}
}

bool TelnetThread::Listen()
{
	constexpr int RecvBufferSize = 1024;
	constexpr int SendBufferSize = 1024;

	try
	{
		asio::error_code ec;

		// Attempt to setup the acceptor.
		_acceptor = std::make_unique<asio::ip::tcp::acceptor>(_workerPool->get_executor());

		// Parse the address from a string.
		asio::ip::address_v4 listenAddress = asio::ip::make_address_v4(_listenAddress, ec);
		if (ec)
		{
			spdlog::error("TelnetThread::Listen: listen address ({}) is invalid: {}",
				_listenAddress, ec.message());
			return false;
		}

		// Setup the endpoint for TCPv4 listenAddress:port.
		asio::ip::tcp::endpoint endpoint(listenAddress, _port);

		// Attempt to open the socket.
		_acceptor->open(endpoint.protocol(), ec);
		if (ec)
		{
			spdlog::error("TelnetThread::Listen: open() failed: {}", ec.message());
			return false;
		}

		// Attempt to bind the socket.
		_acceptor->bind(endpoint, ec);
		if (ec)
		{
			spdlog::error("TelnetThread::Listen: bind() failed on {}:{}: {}", _listenAddress, _port,
				ec.message());
			return false;
		}

		// Allow address reuse (i.e. rebinding to the same port)
		_acceptor->set_option(asio::socket_base::reuse_address(true), ec);
		if (ec)
		{
			spdlog::error(
				"TelnetThread::Listen: set_option(reuse_address) failed: {}", ec.message());
			return false;
		}

		// Configure receive buffer size
		_acceptor->set_option(asio::socket_base::receive_buffer_size(RecvBufferSize), ec);
		if (ec)
		{
			spdlog::error(
				"TelnetThread::Listen: set_option(receive_buffer_size) failed: {}", ec.message());
			return false;
		}

		// Configure send buffer size
		_acceptor->set_option(asio::socket_base::send_buffer_size(SendBufferSize), ec);
		if (ec)
		{
			spdlog::error(
				"TelnetThread::Listen: set_option(send_buffer_size) failed: {}", ec.message());
			return false;
		}

		// Start listening with a backlog of 5
		_acceptor->listen(5, ec);
		if (ec)
		{
			spdlog::error("TelnetThread::Listen: listen() failed: {}", ec.message());
			return false;
		}
	}
	catch (const asio::system_error& ex)
	{
		spdlog::error("TelnetThread::Listen: failed to bind on 0.0.0.0:{}: {}", _port, ex.what());
		return false;
	}

	spdlog::info("TelnetThread::Listen: initialized port={:05}", _port);
	return true;
}

void TelnetThread::AsyncAccept()
{
	if (!CanTick())
	{
		// Thread is stopped; stop chain
		return;
	}

	try
	{
		_acceptor->async_accept(
			[this](const asio::error_code& ec, asio::ip::tcp::socket rawSocket)
			{
				if (!ec)
				{
					spdlog::info("TelnetThread::thread_loop(): client connecting");
					if (!IsAddressWhitelisted(rawSocket))
					{
						spdlog::warn(
							"TelnetThread::thread_loop(): non-local client connection refused");
						rawSocket.close();
						return;
					}

					auto clientThread = std::make_shared<TelnetClientThread>(
						std::move(rawSocket), _nextSocketId++);
					_telnetThreadMap.insert(std::make_pair(_nextSocketId, clientThread));
					clientThread->start();
				}
				else
				{
					if (ec == asio::error::operation_aborted)
						spdlog::debug("TelnetThread::AsyncAccept: accept operation cancelled");
					else
						spdlog::error("TelnetThread::AsyncAccept: accept failed: {}", ec.message());
				}

				AsyncAccept();
			});
	}
	catch (const std::exception& e)
	{
		spdlog::error("TelnetThread::AsyncAccept: {}", e.what());
	}
}

bool TelnetThread::IsAddressWhitelisted(asio::ip::tcp::socket& clientSocket) const
{
	const std::string remoteIp = clientSocket.remote_endpoint().address().to_string();
	return _addressWhitelist.contains(remoteIp);
}

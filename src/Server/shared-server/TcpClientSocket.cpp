#include "pch.h"
#include "TcpClientSocket.h"
#include "TcpClientSocketManager.h"

TcpClientSocket::TcpClientSocket(TcpClientSocketManager* socketManager) : TcpSocket(socketManager)
{
}

std::string_view TcpClientSocket::GetImplName() const
{
	return "TcpClientSocket";
}

bool TcpClientSocket::Create()
{
	asio::error_code ec;

	_socket = std::make_unique<RawSocket_t>(*_socketManager->GetWorkerPool());
	if (_socket == nullptr)
	{
		spdlog::error(
			"TcpClientSocket({})::Create: failed to allocate socket: out of memory", GetImplName());
		return false;
	}

	_socket->open(asio::ip::tcp::v4(), ec);
	if (ec)
	{
		spdlog::error(
			"TcpClientSocket({})::Create: failed to open socket: {}", GetImplName(), ec.message());
		return false;
	}

	// Disable linger (close socket immediately regardless of existence of pending data)
	_socket->set_option(asio::socket_base::linger(false, 0), ec);
	if (ec)
	{
		spdlog::error("TcpClientSocket({})::Create: failed to set linger option: {}", GetImplName(),
			ec.message());
		return false;
	}

	// Increase receive buffer size
	_socket->set_option(asio::socket_base::receive_buffer_size(_recvBufferSize * 4), ec);
	if (ec)
	{
		spdlog::error("TcpClientSocket({})::Create: failed to set receive buffer size: {}",
			GetImplName(), ec.message());
		return false;
	}

	// Increase send buffer size
	_socket->set_option(asio::socket_base::send_buffer_size(_sendBufferSize * 4), ec);
	if (ec)
	{
		spdlog::error("TcpClientSocket({})::Create: failed to set send buffer size: {}",
			GetImplName(), ec.message());
		return false;
	}

	return true;
}

bool TcpClientSocket::Connect(const char* remoteAddress, uint16_t remotePort)
{
	asio::error_code ec;

	// Each new connection requires a new socket.
	// We should ensure the old one's disconnected and shutdown first.
	if (_socket != nullptr && _socket->is_open())
		CloseProcess();

	// Create a new socket.
	if (!Create())
	{
		spdlog::error("TcpClientSocket({})::Connect: failed to create new socket [socketId={}]",
			GetImplName(), GetSocketID());
		return false;
	}

	asio::ip::address ip = asio::ip::make_address(remoteAddress, ec);
	if (ec)
	{
		spdlog::error("TcpClientSocket({})::Connect: invalid address {}: {} [socketId={}]",
			GetImplName(), remoteAddress, ec.message(), GetSocketID());
		return false;
	}

	asio::ip::tcp::endpoint endpoint(ip, remotePort);

	_socket->connect(endpoint, ec);
	if (ec)
	{
		spdlog::error("TcpClientSocket({})::Connect: failed to connect: {} [socketId={}]",
			GetImplName(), ec.message(), GetSocketID());
		_socket->close();
		return false;
	}

	InitSocket();

	_remoteIp       = ip.to_string();
	_remoteIpCached = true;

	AsyncReceive();

	return true;
}

#include "stdafx.h"
#include "TcpClientSocket.h"
#include "SocketManager.h"

TcpClientSocket::TcpClientSocket(SocketManager* socketManager)
	: TcpSocket(socketManager)
{
}

bool TcpClientSocket::Create()
{
	asio::error_code ec;

	_socket.open(asio::ip::tcp::v4(), ec);
	if (ec)
	{
		spdlog::error("TcpClientSocket::Create: failed to open socket: {}", ec.message());
		return false;
	}

	// Disable linger (close socket immediately regardless of existence of pending data)
	_socket.set_option(asio::socket_base::linger(false, 0), ec);
	if (ec)
	{
		spdlog::error("TcpClientSocket::Create: failed to set linger option: {}", ec.message());
		return false;
	}

	// Increase receive buffer size
	_socket.set_option(asio::socket_base::receive_buffer_size(_recvBufferSize * 4), ec);
	if (ec)
	{
		spdlog::error("TcpClientSocket::Create: failed to set receive buffer size: {}", ec.message());
		return false;
	}

	// Increase send buffer size
	_socket.set_option(asio::socket_base::send_buffer_size(_sendBufferSize * 4), ec);
	if (ec)
	{
		spdlog::error("TcpClientSocket::Create: failed to set send buffer size: {}", ec.message());
		return false;
	}

	return true;
}

bool TcpClientSocket::Connect(const char* remoteAddress, uint16_t remotePort)
{
	asio::error_code ec;

	asio::ip::address ip = asio::ip::make_address(remoteAddress, ec);
	if (ec)
	{
		spdlog::error("TcpClientSocket::Connect: invalid address {}: {}",
			remoteAddress, ec.message());
		return false;
	}

	asio::ip::tcp::endpoint endpoint(ip, remotePort);

	_socket.connect(endpoint, ec);
	if (ec)
	{
		spdlog::error("TcpClientSocket::Connect: failed to connect: {}", ec.message());
		_socket.close();
		return false;
	}

	ASSERT(_socketManager != nullptr);

	if (!_socketManager->AcquireClientSocket(this))
	{
		spdlog::error("TcpClientSocket::Connect: failed to acquire client socket ID");
		return false;
	}

	InitSocket();

	_state			= CONNECTION_STATE_CONNECTED;
	_type			= SOCKET_TYPE_CLIENT;

	_remoteIp		= ip.to_string();
	_remoteIpCached	= true;

	AsyncReceive();

	return true;
}

TcpClientSocket::~TcpClientSocket()
{
}

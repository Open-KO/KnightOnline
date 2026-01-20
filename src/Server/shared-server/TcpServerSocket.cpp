#include "pch.h"
#include "TcpServerSocket.h"
#include "TcpServerSocketManager.h"

TcpServerSocket::TcpServerSocket(test_tag tag) : TcpSocket(tag)
{
}

TcpServerSocket::TcpServerSocket(TcpServerSocketManager* socketManager) : TcpSocket(socketManager)
{
}

std::string_view TcpServerSocket::GetSocketClass() const
{
	return "TcpServerSocket";
}

#pragma once

#include "TcpSocket.h"

class TcpClientSocket : public TcpSocket
{
public:
	TcpClientSocket(SocketManager* socketManager);
	~TcpClientSocket() override;

	bool Create();
	bool Connect(const char* remoteAddress, uint16_t remotePort);
};

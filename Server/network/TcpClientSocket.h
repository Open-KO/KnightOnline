#pragma once

#include "TcpSocket.h"

class TcpClientSocket : public TcpSocket
{
public:
	TcpClientSocket(SocketManager* socketManager);
	bool Create();
	bool Connect(const char* remoteAddress, uint16_t remotePort);
	void Close() override;

protected:
	void ReleaseToManager() override;
};

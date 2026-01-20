#ifndef SERVER_SHAREDSERVER_TCPCLIENTSOCKET_H
#define SERVER_SHAREDSERVER_TCPCLIENTSOCKET_H

#pragma once

#include "TcpSocket.h"

class TcpClientSocketManager;
class TcpClientSocket : public TcpSocket
{
public:
	TcpClientSocket(TcpClientSocketManager* socketManager);

protected:
	bool Create();

public:
	bool Connect(const char* remoteAddress, uint16_t remotePort);
	void Close() override;
};

#endif // SERVER_SHAREDSERVER_TCPCLIENTSOCKET_H

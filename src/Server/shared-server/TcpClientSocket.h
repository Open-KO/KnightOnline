#ifndef SERVER_SHAREDSERVER_TCPCLIENTSOCKET_H
#define SERVER_SHAREDSERVER_TCPCLIENTSOCKET_H

#pragma once

#include "TcpSocket.h"

class TcpClientSocketManager;
class TcpClientSocket : public TcpSocket
{
public:
	TcpClientSocket(TcpClientSocketManager* socketManager);

private:
	std::string_view GetImplName() const override;

protected:
	bool Create();

public:
	bool Connect(const char* remoteAddress, uint16_t remotePort);
};

#endif // SERVER_SHAREDSERVER_TCPCLIENTSOCKET_H

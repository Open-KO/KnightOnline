#ifndef SERVER_SHAREDSERVER_TCPSERVERSOCKET_H
#define SERVER_SHAREDSERVER_TCPSERVERSOCKET_H

#pragma once

#include "TcpSocket.h"

class TcpServerSocketManager;
class TcpServerSocket : public TcpSocket
{
public:
	TcpServerSocket(test_tag);
	TcpServerSocket(TcpServerSocketManager* socketManager);
	void Close() override;

protected:
	void ReleaseToManager() override;
};

#endif // SERVER_SHAREDSERVER_TCPSERVERSOCKET_H

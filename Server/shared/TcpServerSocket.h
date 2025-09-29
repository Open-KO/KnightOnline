#pragma once

#include "TcpSocket.h"

class TcpServerSocket : public TcpSocket
{
public:
	TcpServerSocket(SocketManager* socketManager);
	void Close() override;

protected:
	void ReleaseToManager() override;
};

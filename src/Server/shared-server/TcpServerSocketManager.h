#ifndef SERVER_SHAREDSERVER_TCPSERVERSOCKETMANAGER_H
#define SERVER_SHAREDSERVER_TCPSERVERSOCKETMANAGER_H

#pragma once

#include "TcpSocketManager.h"

class TcpServerSocketManager : public TcpSocketManager
{
public:
	template <typename T, typename... Args>
	inline void AllocateSockets(Args&&... args)
	{
		// NOTE: The socket manager instance should be declared last.
		for (size_t i = 0; i < _inactiveSocketArray.size(); i++)
			_inactiveSocketArray[i] = new T(std::forward<Args>(args)..., this);
	}

public:
	TcpServerSocketManager(int recvBufferSize, int sendBufferSize);
	~TcpServerSocketManager() override;
	bool Listen(int port);
	void Shutdown();
	void StartAccept();
	void StopAccept();

private:
	void AsyncAccept();
	void OnAccept(asio::ip::tcp::socket& rawSocket);

protected:
	std::unique_ptr<asio::ip::tcp::acceptor> _acceptor = {};
	std::atomic<bool> _acceptingConnections            = false;
};

#endif // SERVER_SHAREDSERVER_TCPSERVERSOCKETMANAGER_H

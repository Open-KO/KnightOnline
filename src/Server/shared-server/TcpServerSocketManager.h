#ifndef SERVER_SHAREDSERVER_TCPSERVERSOCKETMANAGER_H
#define SERVER_SHAREDSERVER_TCPSERVERSOCKETMANAGER_H

#pragma once

#include "TcpSocketManager.h"
#include "TcpServerSocket.h"

class TcpServerSocketManager : public TcpSocketManager
{
public:
	template <typename T, typename... Args>
	inline bool AllocateSockets(Args&&... args)
	{
		// NOTE: The socket manager instance should be declared last.
		for (size_t i = 0; i < _inactiveSocketArray.size(); i++)
		{
			if (_inactiveSocketArray[i] != nullptr)
				continue;

			auto tcpSocket = std::make_unique<T>(std::forward<Args>(args)..., this);
			if (tcpSocket == nullptr)
				return false;

			tcpSocket->SetSocketID(static_cast<int>(i));
			_inactiveSocketArray[i] = std::move(tcpSocket);
		}

		return true;
	}

	inline std::shared_ptr<TcpServerSocket> GetSocket(int socketId) const
	{
		return std::static_pointer_cast<TcpServerSocket>(TcpSocketManager::GetSocket(socketId));
	}

	inline std::shared_ptr<TcpServerSocket> GetSocketUnchecked(int socketId) const
	{
		return std::static_pointer_cast<TcpServerSocket>(
			TcpSocketManager::GetSocketUnchecked(socketId));
	}

	inline std::shared_ptr<TcpServerSocket> GetInactiveSocket(int socketId) const
	{
		return std::static_pointer_cast<TcpServerSocket>(
			TcpSocketManager::GetInactiveSocket(socketId));
	}

	inline std::shared_ptr<TcpServerSocket> GetInactiveSocketUnchecked(int socketId) const
	{
		return std::static_pointer_cast<TcpServerSocket>(
			TcpSocketManager::GetInactiveSocketUnchecked(socketId));
	}

public:
	TcpServerSocketManager(int recvBufferSize, int sendBufferSize,
		std::string_view implName = "TcpServerSocketManager");
	~TcpServerSocketManager() override;
	bool Listen(int port);
	void Shutdown();
	void StartAccept();
	void StopAccept();

private:
	void AsyncAccept();
	void OnAccept(asio::ip::tcp::socket& rawSocket);

protected:
	bool ProcessClose(TcpSocket* tcpSocket) override;

protected:
	std::unique_ptr<asio::ip::tcp::acceptor> _acceptor = {};
	std::atomic<bool> _acceptingConnections            = false;
};

#endif // SERVER_SHAREDSERVER_TCPSERVERSOCKETMANAGER_H

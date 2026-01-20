#ifndef SERVER_SHAREDSERVER_TCPCLIENTSOCKETMANAGER_H
#define SERVER_SHAREDSERVER_TCPCLIENTSOCKETMANAGER_H

#pragma once

#include "TcpSocketManager.h"
#include "TcpClientSocket.h"

class TcpClientSocketManager : public TcpSocketManager
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

	inline std::shared_ptr<TcpClientSocket> GetSocket(int socketId) const
	{
		return std::static_pointer_cast<TcpClientSocket>(TcpSocketManager::GetSocket(socketId));
	}

	inline std::shared_ptr<TcpClientSocket> GetSocketUnchecked(int socketId) const
	{
		return std::static_pointer_cast<TcpClientSocket>(
			TcpSocketManager::GetSocketUnchecked(socketId));
	}

	inline std::shared_ptr<TcpClientSocket> GetInactiveSocket(int socketId) const
	{
		return std::static_pointer_cast<TcpClientSocket>(
			TcpSocketManager::GetInactiveSocket(socketId));
	}

	inline std::shared_ptr<TcpClientSocket> GetInactiveSocketUnchecked(int socketId) const
	{
		return std::static_pointer_cast<TcpClientSocket>(
			TcpSocketManager::GetInactiveSocketUnchecked(socketId));
	}

public:
	TcpClientSocketManager(int recvBufferSize, int sendBufferSize,
		std::string_view implName = "TcpClientSocketManager");
	std::shared_ptr<TcpClientSocket> AcquireSocket();
	void ReleaseSocket(std::shared_ptr<TcpClientSocket> tcpSocket);
	void Shutdown();
	~TcpClientSocketManager() override;

protected:
	bool ProcessClose(TcpSocket* tcpSocket) override;
};

#endif // SERVER_SHAREDSERVER_TCPCLIENTSOCKETMANAGER_H

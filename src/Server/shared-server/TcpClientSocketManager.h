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

			T* tcpSocket = new T(std::forward<Args>(args)..., this);
			if (tcpSocket == nullptr)
				return false;

			tcpSocket->SetSocketID(static_cast<int>(i));
			_inactiveSocketArray[i] = tcpSocket;
		}

		return true;
	}

	inline TcpClientSocket* GetSocket(int socketId) const
	{
		return static_cast<TcpClientSocket*>(TcpSocketManager::GetSocket(socketId));
	}

	inline TcpClientSocket* GetSocketUnchecked(int socketId) const
	{
		return static_cast<TcpClientSocket*>(TcpSocketManager::GetSocketUnchecked(socketId));
	}

	inline TcpClientSocket* GetInactiveSocket(int socketId) const
	{
		return static_cast<TcpClientSocket*>(TcpSocketManager::GetInactiveSocket(socketId));
	}

	inline TcpClientSocket* GetInactiveSocketUnchecked(int socketId) const
	{
		return static_cast<TcpClientSocket*>(
			TcpSocketManager::GetInactiveSocketUnchecked(socketId));
	}

public:
	TcpClientSocketManager(int recvBufferSize, int sendBufferSize);
	TcpClientSocket* AcquireSocket();
	void ReleaseSocket(TcpClientSocket* tcpSocket);
	void Shutdown();
	~TcpClientSocketManager() override;

protected:
	bool ProcessClose(TcpSocket* tcpSocket) override;
};

#endif // SERVER_SHAREDSERVER_TCPCLIENTSOCKETMANAGER_H

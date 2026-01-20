#ifndef SERVER_SHAREDSERVER_TCPSOCKETMANAGER_H
#define SERVER_SHAREDSERVER_TCPSOCKETMANAGER_H

#pragma once

#include <asio.hpp>

#include <functional>
#include <memory>
#include <mutex>
#include <queue>

class TcpSocket;
class TcpClientSocket;

// NOTE: As with official behaviour, this class manages both:
// 1. Server sockets (client connects to us) and
// 2. Client sockets (we connect to a remote server).
// Ideally this should be separated, but we preserve this because parsing for both sockets
// uses shared game data which relies on everything acted in an effectively single-threaded fashion.
// This should be refactored and improved later, but for now we continue to preserve this behaviour
// to avoid any such unintentional data access issues.
class TcpSocketManager
{
	friend class TcpSocket;
	friend class TcpClientSocket;
	friend class TcpServerSocket;

	using StartUserThreadCallback    = std::function<void()>;
	using ShutdownUserThreadCallback = std::function<void()>;

public:
	inline asio::io_context& GetIoContext()
	{
		return _io;
	}

	inline std::shared_ptr<asio::thread_pool> GetWorkerPool()
	{
		return _workerPool;
	}

	inline std::recursive_mutex& GetMutex()
	{
		return _mutex;
	}

	inline int GetRecvBufferSize() const
	{
		return _recvBufferSize;
	}

	inline int GetSendBufferSize() const
	{
		return _sendBufferSize;
	}

	inline int GetSocketCount() const
	{
		return _socketCount;
	}

	inline bool IsValidSocketId(int socketId) const
	{
		return socketId >= 0 && socketId < GetSocketCount();
	}

	inline TcpSocket* GetSocket(int socketId) const
	{
		if (socketId < 0 || socketId >= static_cast<int>(_socketArray.size()))
			return nullptr;

		return _socketArray[socketId];
	}

	inline TcpSocket* GetSocketUnchecked(int socketId) const
	{
		return _socketArray[socketId];
	}

	inline TcpSocket* GetInactiveSocket(int socketId) const
	{
		if (socketId < 0 || socketId >= static_cast<int>(_inactiveSocketArray.size()))
			return nullptr;

		return _inactiveSocketArray[socketId];
	}

	inline TcpSocket* GetInactiveSocketUnchecked(int socketId) const
	{
		return _inactiveSocketArray[socketId];
	}

protected:
	TcpSocketManager(int recvBufferSize, int sendBufferSize);
	virtual ~TcpSocketManager();

public:
	void Init(int socketCount, uint32_t workerThreadCount = 0);

protected:
	virtual TcpSocket* AcquireSocket(int& socketId);
	virtual void ReleaseSocket(TcpSocket* tcpSocket);
	virtual void OnPostReceive(
		const asio::error_code& ec, size_t bytesTransferred, TcpSocket* tcpSocket);
	virtual void OnPostSend(
		const asio::error_code& ec, size_t bytesTransferred, TcpSocket* tcpSocket);
	virtual void OnPostSocketClose(TcpSocket* tcpSocket);
	virtual bool ProcessClose(TcpSocket* tcpSocket) = 0;

protected:
	void ShutdownImpl();

protected:
	std::vector<TcpSocket*> _socketArray                   = {};
	std::vector<TcpSocket*> _inactiveSocketArray           = {};

	int _socketCount                                       = 0;
	int _recvBufferSize                                    = 0;
	int _sendBufferSize                                    = 0;

	uint32_t _workerThreadCount                            = 0;

	asio::io_context _io                                   = {};
	std::shared_ptr<asio::thread_pool> _workerPool         = {};

	std::queue<int> _socketIdQueue                         = {};
	std::recursive_mutex _mutex                            = {};

	StartUserThreadCallback _startUserThreadCallback       = nullptr;
	ShutdownUserThreadCallback _shutdownUserThreadCallback = nullptr;
};

#endif // SERVER_SHAREDSERVER_TCPSOCKETMANAGER_H

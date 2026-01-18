#ifndef SERVER_SHAREDSERVER_SOCKETMANAGER_H
#define SERVER_SHAREDSERVER_SOCKETMANAGER_H

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
class SocketManager
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

	inline int GetServerSocketCount() const
	{
		return _serverSocketCount;
	}

	inline int GetClientSocketCount() const
	{
		return _clientSocketCount;
	}

	inline bool IsValidServerSocketId(int socketId) const
	{
		return socketId >= 0 && socketId < GetServerSocketCount();
	}

	inline TcpSocket* GetServerSocket(int socketId) const
	{
		if (!IsValidServerSocketId(socketId))
			return nullptr;

		return _serverSocketArray[socketId];
	}

	inline TcpSocket* GetServerSocketUnchecked(int socketId) const
	{
		return _serverSocketArray[socketId];
	}

	inline TcpSocket* GetInactiveServerSocket(int socketId) const
	{
		if (!IsValidServerSocketId(socketId))
			return nullptr;

		return _inactiveServerSocketArray[socketId];
	}

	inline TcpSocket* GetInactiveServerSocketUnchecked(int socketId) const
	{
		return _inactiveServerSocketArray[socketId];
	}

public:
	template <typename T, typename... Args>
	inline void AllocateServerSockets(Args&&... args)
	{
		// NOTE: The socket manager instance should be declared last.
		int serverSocketCount = GetServerSocketCount();
		for (int i = 0; i < serverSocketCount; i++)
			_inactiveServerSocketArray[i] = new T(std::forward<Args>(args)..., this);
	}

public:
	SocketManager(int recvBufferSize, int sendBufferSize);
	virtual ~SocketManager();
	void Init(int serverSocketCount, int clientSocketCount, uint32_t workerThreadCount = 0);
	bool Listen(int port);
	void StartAccept();
	void StopAccept();

private:
	void AsyncAccept();
	void OnAccept(asio::ip::tcp::socket& rawSocket);

protected:
	TcpSocket* AcquireServerSocket(int& socketId);
	void ReleaseServerSocket(TcpSocket* tcpSocket, int socketId);
	bool AcquireClientSocket(TcpClientSocket* tcpClientSocket);
	void ReleaseClientSocket(int socketId);
	int GetAvailableClientSocketId() const;
	void OnPostReceive(const asio::error_code& ec, size_t bytesTransferred, TcpSocket* tcpSocket);
	void OnPostSend(const asio::error_code& ec, size_t bytesTransferred, TcpSocket* tcpSocket);
	void OnPostServerSocketClose(TcpSocket* tcpSocket);
	void OnPostClientSocketClose(TcpClientSocket* tcpSocket);
	bool ProcessClose(TcpSocket* tcpSocket);

public:
	void Shutdown();

protected:
	TcpSocket** _serverSocketArray         = nullptr;
	TcpSocket** _inactiveServerSocketArray = nullptr;
	TcpClientSocket** _clientSocketArray   = nullptr;

	int _serverSocketCount                 = 0;
	int _clientSocketCount                 = 0;

	int _recvBufferSize                    = 0;
	int _sendBufferSize                    = 0;

	uint32_t _workerThreadCount            = 0;

	asio::io_context _io;
	std::unique_ptr<asio::ip::tcp::acceptor> _acceptor;
	std::shared_ptr<asio::thread_pool> _workerPool;

	std::atomic<bool> _acceptingConnections = false;

	std::queue<int> _socketIdQueue;
	std::recursive_mutex _mutex;

	StartUserThreadCallback _startUserThreadCallback       = nullptr;
	ShutdownUserThreadCallback _shutdownUserThreadCallback = nullptr;
};

#endif // SERVER_SHAREDSERVER_SOCKETMANAGER_H

// IOCPort.h: interface for the CIOCPort class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IOCPORT_H__1555441D_142E_4C26_B889_D9DCFC5E54E8__INCLUDED_)
#define AFX_IOCPORT_H__1555441D_142E_4C26_B889_D9DCFC5E54E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SendWorkerThread.h"

#include <memory>
#include <mutex>
#include <queue>

class CIOCPSocket2;
class CIOCPort
{
	friend class CIOCPSocket2;

public:
	asio::io_context& GetIoContext()
	{
		return _io;
	}

	std::shared_ptr<asio::thread_pool> GetWorkerPool()
	{
		return _workerPool;
	}

	std::recursive_mutex& GetMutex()
	{
		return _socketMutex;
	}

	CIOCPort();
	virtual ~CIOCPort();
	void Init(int serversocksize, int clientsocksize, int workernum = 0);
	bool Listen(int port);
	void StartAccept();
	void StopAccept();
	CIOCPSocket2* AcquireServerSocket(int& socketId);
	void ReleaseServerSocket(CIOCPSocket2* iocpSocket, int socketId);
	bool AcquireClientSocket(CIOCPSocket2* iocpSocket);
	void ReleaseClientSocket(int socketId);
	void Shutdown();

private:
	int GetAvailableClientSocketId() const;
	void AsyncAccept();
	void OnAccept(asio::ip::tcp::socket& rawSocket);

protected:
	void OnPostReceive(const asio::error_code& ec, size_t bytesTransferred, CIOCPSocket2* iocpSocket);
	void OnPostSend(const asio::error_code& ec, size_t bytesTransferred, CIOCPSocket2* iocpSocket);
	void OnPostClose(CIOCPSocket2* iocpSocket);
	bool ProcessClose(CIOCPSocket2* iocpSocket);

public:
	int m_SocketArraySize;
	int m_ClientSockSize;

	CIOCPSocket2** m_SockArray;
	CIOCPSocket2** m_SockArrayInActive;
	CIOCPSocket2** m_ClientSockArray;		// Connect용 소켓

protected:
	uint32_t _numberOfWorkers;

	asio::io_context _io;
	std::unique_ptr<asio::ip::tcp::acceptor> _acceptor;
	std::shared_ptr<asio::thread_pool> _workerPool;

	std::atomic<bool> _acceptingConnections;

	std::queue<int> _socketIdQueue;
	std::recursive_mutex _socketMutex;

	SendWorkerThread _sendWorkerThread;
};

#endif // !defined(AFX_IOCPORT_H__1555441D_142E_4C26_B889_D9DCFC5E54E8__INCLUDED_)

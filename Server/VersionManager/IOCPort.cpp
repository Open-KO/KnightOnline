// IOCPort.cpp: implementation of the CIOCPort class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IOCPort.h"
#include "IOCPSocket2.h"
#include "Define.h"

#include <algorithm>
#include <spdlog/spdlog.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIOCPort::CIOCPort()
{
	m_SockArray = nullptr;
	m_SockArrayInActive = nullptr;
	m_SocketArraySize = 0;
	_numberOfWorkers = 0;
	_acceptingConnections = false;
}

CIOCPort::~CIOCPort()
{
	Shutdown();
}

void CIOCPort::Init(int serversocksize, int workernum)
{
	m_SocketArraySize = serversocksize;

	m_SockArray = new CIOCPSocket2* [serversocksize];
	m_SockArrayInActive = new CIOCPSocket2* [serversocksize];

	// NOTE: Specifically allocate the worker pool first, as we'll need this for our sockets.
	if (workernum == 0)
		_numberOfWorkers = std::thread::hardware_concurrency() * 2;
	else
		_numberOfWorkers = workernum;

	_workerPool = std::make_shared<asio::thread_pool>(_numberOfWorkers);

	std::queue<int> socketIdQueue;
	for (int i = 0; i < serversocksize; i++)
	{
		m_SockArray[i] = nullptr;
		m_SockArrayInActive[i] = nullptr;
		socketIdQueue.push(i);
	}

	// NOTE: These don't strictly need to be guarded as the server's not yet operational,
	// but we do it for consistency.
	{
		std::lock_guard<std::recursive_mutex> lock(_socketMutex);
		_socketIdQueue.swap(socketIdQueue);
	}
}

bool CIOCPort::Listen(int port)
{
	try
	{
		asio::error_code ec;

		// Attempt to setup the acceptor.
		_acceptor = std::make_unique<asio::ip::tcp::acceptor>(_workerPool->get_executor());

		// Setup the endpoint for TCPv4 0.0.0.0:port
		asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port);

		// Attempt to open the socket.
		_acceptor->open(endpoint.protocol(), ec);
		if (ec)
		{
			spdlog::error("IOCPort::Listen: open() failed: {}", ec.message());
			return false;
		}

		// Attempt to bind the socket.
		_acceptor->bind(endpoint, ec);
		if (ec)
		{
			spdlog::error("IOCPort::Listen: bind() failed on 0.0.0.0:{}: {}",
				port, ec.message());
			return false;
		}

		// Allow address reuse (i.e. rebinding to the same port)
		_acceptor->set_option(asio::socket_base::reuse_address(true), ec);
		if (ec)
		{
			spdlog::error("IOCPort::Listen: set_option(reuse_address) failed: {}", ec.message());
			return false;
		}

		// Configure receive buffer size
		_acceptor->set_option(asio::socket_base::receive_buffer_size(SOCKET_BUFF_SIZE * 4), ec);
		if (ec)
		{
			spdlog::error("IOCPort::Listen: set_option(receive_buffer_size) failed: {}", ec.message());
			return false;
		}

		// Configure send buffer size
		_acceptor->set_option(asio::socket_base::send_buffer_size(SOCKET_BUFF_SIZE * 4), ec);
		if (ec)
		{
			spdlog::error("IOCPort::Listen: set_option(send_buffer_size) failed: {}", ec.message());
			return false;
		}

		// Start listening with a backlog of 5
		_acceptor->listen(5, ec);
		if (ec)
		{
			spdlog::error("IOCPort::Listen: listen() failed: {}", ec.message());
			return false;
		}
	}
	catch (const asio::system_error& ex)
	{
		spdlog::error("IOCPort::Listen: failed to bind on 0.0.0.0:{}: {}",
			port, ex.what());
		return false;
	}

	spdlog::info("IOCPort::Listen: initialized port={:05}", port);
	return true;
}

void CIOCPort::StartAccept()
{
	_acceptingConnections = true;
	AsyncAccept();
}

void CIOCPort::StopAccept()
{
	_acceptingConnections = false;

	if (_acceptor != nullptr
		&& _acceptor->is_open())
	{
		asio::error_code ec;
		_acceptor->cancel(ec);

		if (ec)
			spdlog::error("IOCPort::StopAccept: cancel() failed: {}", ec.message());
	}
}

void CIOCPort::AsyncAccept()
{
	if (!_acceptingConnections)
		return;

	try
	{
		_acceptor->async_accept([this](const asio::error_code& ec, asio::ip::tcp::socket rawSocket) mutable
		{
			if (!ec)
			{
				OnAccept(rawSocket);
			}
			else
			{
				if (ec == asio::error::operation_aborted)
					spdlog::debug("IOCPort::AsyncAccept: accept operation cancelled");
				else
					spdlog::error("IOCPort::AsyncAccept: accept failed: {}", ec.message());
			}

			AsyncAccept();
		});
	}
	catch (const asio::system_error& ex)
	{
		spdlog::error("IOCPort::AsyncAccept: async_accept() failed: {}", ex.what());
	}
}

void CIOCPort::OnAccept(asio::ip::tcp::socket& rawSocket)
{
	int socketId = -1;
	CIOCPSocket2* iocpSocket = nullptr;

	// NOTE: Handle the guarding externally so it's clear what's guarded and what's not,
	// which is critical when dealing with code needing to be fairly high performance here.
	{
		std::lock_guard<std::recursive_mutex> lock(_socketMutex);
		iocpSocket = PopSocket(socketId);
	}

	if (socketId == -1)
	{
		spdlog::error("IOCPort::OnAccept: socketId list is empty");
		return;
	}

	// This should never happen.
	// If it does, the associated socket ID was never removed from the list so we don't have to restore it.
	if (iocpSocket == nullptr)
	{
		spdlog::error("IOCPort::OnAccept: null socket [socketId:{}]", socketId);
		return;
	}

	iocpSocket->_socket = std::move(rawSocket);

	iocpSocket->InitSocket();
	iocpSocket->Receive();

	spdlog::debug("IOCPort::AcceptThread: successfully accepted socketId={}", socketId);
}

void CIOCPort::OnPostReceive(const asio::error_code& ec, size_t bytesTransferred, CIOCPSocket2* iocpSocket)
{
	if (ec)
	{
		if (ec == asio::error::eof)
		{
			spdlog::debug("IOCPort::OnPostReceive: peer closed connection socketId={}",
				iocpSocket->GetSocketID());
		}
		else
		{
			spdlog::debug("IOCPort::OnPostReceive: socketId={} error={}",
				iocpSocket->GetSocketID(), ec.message());

			if (++iocpSocket->_socketErrorCount < 2)
				return;
		}

		ProcessClose(iocpSocket);
		return;
	}

	if (bytesTransferred == 0)
	{
		spdlog::debug("IOCPort::OnPostReceive: closed by 0 byte notify");
		ProcessClose(iocpSocket);
		return;
	}

	iocpSocket->ReceivedData(static_cast<int>(bytesTransferred));
	iocpSocket->Receive();
}

void CIOCPort::OnPostSend(const asio::error_code& ec, size_t bytesTransferred, CIOCPSocket2* iocpSocket)
{
	if (ec)
	{
		spdlog::error("IOCPort::OnPostSend: socketId={} failed: {}",
			iocpSocket->GetSocketID(), ec.message());

		iocpSocket->Close();
		return;
	}

	iocpSocket->_socketErrorCount = 0;

	// Pop this queued entry & dispatch next queued send if applicable.
	iocpSocket->DoSend(true);
}

void CIOCPort::OnPostClose(CIOCPSocket2* iocpSocket)
{
	if (!ProcessClose(iocpSocket))
		return;

	spdlog::debug("IOCPort::OnPostClose: closed by Close() socketId={}",
		iocpSocket->GetSocketID());
}

bool CIOCPort::ProcessClose(CIOCPSocket2* iocpSocket)
{
	std::lock_guard<std::recursive_mutex> lock(_socketMutex);
	if (iocpSocket->GetState() == STATE_DISCONNECTED)
		return false;

	iocpSocket->CloseProcess();

	PushSocket(iocpSocket, iocpSocket->GetSocketID());
	return true;
}

CIOCPSocket2* CIOCPort::PopSocket(int& socketId)
{
	if (_socketIdQueue.empty())
		return nullptr;

	socketId = _socketIdQueue.front();

	// This is all self-contained so it should never be out of range.
	_ASSERT(socketId >= 0 && socketId < m_SocketArraySize);

	CIOCPSocket2* iocpSocket = m_SockArrayInActive[socketId];
	if (iocpSocket == nullptr)
		return nullptr;

	_socketIdQueue.pop();

	m_SockArray[socketId] = iocpSocket;
	m_SockArrayInActive[socketId] = nullptr;

	iocpSocket->SetSocketID(socketId);
	return iocpSocket;
}

void CIOCPort::PushSocket(CIOCPSocket2* iocpSocket, int socketId)
{
	if (socketId < 0
		|| socketId >= m_SocketArraySize)
	{
		spdlog::error("IOCPort::PushSocket: out of range socketId={}", socketId);
		return;
	}

	_socketIdQueue.push(socketId);

	if (iocpSocket != nullptr)
	{
		m_SockArray[socketId] = nullptr;
		m_SockArrayInActive[socketId] = iocpSocket;
	}
}

void CIOCPort::Shutdown()
{
	// Stop accepting new connections
	StopAccept();

	// Reset the acceptor.
	if (_acceptor != nullptr)
		_acceptor.reset();

	// Explicitly disconnect all sockets now.
	{
		std::lock_guard<std::recursive_mutex> lock(_socketMutex);

		for (int i = 0; i < m_SocketArraySize; i++)
		{
			if (m_SockArray[i] != nullptr)
				m_SockArray[i]->CloseProcess();
		}
	}

	// Force worker threads to finish up work.
	_io.stop();

	// Wait for the worker threads to finish.
	if (_workerPool != nullptr)
		_workerPool->join();

	// Free our sessions.
	{
		std::lock_guard<std::recursive_mutex> lock(_socketMutex);

		for (int i = 0; i < m_SocketArraySize; i++)
		{
			delete m_SockArray[i];
			m_SockArray[i] = nullptr;

			delete m_SockArrayInActive[i];
			m_SockArrayInActive[i] = nullptr;
		}
	}

	// Finally free the worker pool; it needs to exist while otherwise tied to sessions.
	if (_workerPool != nullptr)
		_workerPool.reset();
}

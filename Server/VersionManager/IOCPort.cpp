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
	DeleteAllArray();
}

void CIOCPort::DeleteAllArray()
{
	for (int i = 0; i < m_SocketArraySize; i++)
	{
		delete m_SockArray[i];
		m_SockArray[i] = nullptr;
	}
	delete[] m_SockArray;

	for (int i = 0; i < m_SocketArraySize; i++)
	{
		delete m_SockArrayInActive[i];
		m_SockArrayInActive[i] = nullptr;
	}
	delete[] m_SockArrayInActive;

	while (!m_SidList.empty())
		m_SidList.pop_back();
}

void CIOCPort::Init(int serversocksize, int workernum)
{
	m_SocketArraySize = serversocksize;

	m_SockArray = new CIOCPSocket2* [serversocksize];
	for (int i = 0; i < serversocksize; i++)
		m_SockArray[i] = nullptr;

	m_SockArrayInActive = new CIOCPSocket2* [serversocksize];
	for (int i = 0; i < serversocksize; i++)
		m_SockArrayInActive[i] = nullptr;

	for (int i = 0; i < serversocksize; i++)
		m_SidList.push_back(i);

	CreateReceiveWorkerThread(workernum);
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
		// Allocate a socket for each new connection
		auto socket = std::make_unique<asio::ip::tcp::socket>(_workerPool->get_executor());

		// Copy a reference to the raw socket; we'll be moving socket so we can keep it alive
		// cheaper than using a shared pointer, so this won't be accessible after this point.
		auto& rawSocket = *socket;

		_acceptor->async_accept(rawSocket,
			[this, socket = std::move(socket)](const asio::error_code& ec) mutable
		{
			if (!ec)
			{
				OnAccept(socket);
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

void CIOCPort::OnAccept(std::unique_ptr<asio::ip::tcp::socket>& socket)
{
	int socketId;

	// NOTE: Handle the guarding externally so it's clear what's guarded and what's not,
	// which is critical when dealing with code needing to be fairly high performance here.
	{
		std::lock_guard<std::recursive_mutex> lock(_socketMutex);
		socketId = PopSocketId();
	}

	if (socketId == -1)
	{
		spdlog::error("IOCPort::OnAccept: socketId list is empty");
		return;
	}

	CIOCPSocket2* iocpSocket = GetIOCPSocket(socketId);
	if (iocpSocket == nullptr)
	{
		spdlog::error("IOCPort::OnAccept: null socket [socketId:{}]", socketId);

		std::lock_guard<std::recursive_mutex> lock(_socketMutex);
		PushSocketId(socketId);
		return;
	}

	iocpSocket->InitSocket(std::move(*socket));
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

			if (++iocpSocket->m_nSocketErr < 2)
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

	iocpSocket->m_nSocketErr = 0;
}

void CIOCPort::OnPostClose(CIOCPSocket2* iocpSocket)
{
	spdlog::debug("IOCPort::OnPostClose: closed by Close() socketId={}",
		iocpSocket->GetSocketID());

	ProcessClose(iocpSocket);
}

void CIOCPort::ProcessClose(CIOCPSocket2* iocpSocket)
{
	std::lock_guard<std::recursive_mutex> lock(_socketMutex);

	iocpSocket->CloseProcess();

	RidIOCPSocket(iocpSocket->GetSocketID(), iocpSocket);
	PushSocketId(iocpSocket->GetSocketID());
}

int CIOCPort::PopSocketId()
{
	if (m_SidList.empty())
		return -1;

	int socketId = m_SidList.front();
	m_SidList.pop_front();

	return socketId;
}

void CIOCPort::PushSocketId(int socketId)
{
	if (socketId < 0
		|| socketId >= m_SocketArraySize)
	{
		spdlog::error("IOCPort::PushSocketId: out of range socketId={}", socketId);
		return;
	}

	m_SidList.push_back(socketId);
}

void CIOCPort::CreateReceiveWorkerThread(int workernum)
{
	if (workernum == 0)
		_numberOfWorkers = std::thread::hardware_concurrency() * 2;
	else
		_numberOfWorkers = workernum;

	_workerPool = std::make_shared<asio::thread_pool>(_numberOfWorkers);
}

CIOCPSocket2* CIOCPort::GetIOCPSocket(int index)
{
	if (index >= m_SocketArraySize)
	{
		spdlog::error("IOCPort::GetIOCPSocket: socketArray overflow index={}", index);
		return nullptr;
	}

	if (m_SockArrayInActive[index] == nullptr)
	{
		spdlog::error("IOCPort::GetIOCPSocket: null socket index={}", index);
		return nullptr;
	}

	CIOCPSocket2* pIOCPSock = m_SockArrayInActive[index];

	m_SockArray[index] = pIOCPSock;
	m_SockArrayInActive[index] = nullptr;

	pIOCPSock->SetSocketID(index);

	return pIOCPSock;
}

void CIOCPort::RidIOCPSocket(int index, CIOCPSocket2* pSock)
{
	if (index < 0
		|| index >= m_SocketArraySize)
	{
		spdlog::error("IOCPort::RidIOCPSocket: invalid index={}", index);
		return;
	}

	m_SockArray[index] = nullptr;
	m_SockArrayInActive[index] = pSock;
}

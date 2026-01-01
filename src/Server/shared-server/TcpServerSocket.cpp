#include "pch.h"
#include "TcpServerSocket.h"
#include "SocketManager.h"

TcpServerSocket::TcpServerSocket(SocketManager* socketManager)
	: TcpSocket(socketManager)
{
}

void TcpServerSocket::Close()
{
	if (GetState() == CONNECTION_STATE_DISCONNECTED)
		return;

	{
		std::lock_guard<std::recursive_mutex> lock(_sendMutex);

		// From this point onward we're effectively disconnected.
		// We should stop handling or sending new packets, and just ensure any existing queued packets are sent.
		// Once all existing packets are sent, we can fully disconnect the socket.
		_pendingDisconnect = true;

		// Wait until the send chain is complete.
		// The send chain will trigger this again.
		if (_sendInProgress
			|| !_sendQueue.empty())
			return;
	}

	asio::error_code ec;
	try
	{
		auto threadPool = _socketManager->GetWorkerPool();
		if (threadPool == nullptr)
			return;

		asio::post(*threadPool, std::bind(&SocketManager::OnPostServerSocketClose, _socketManager, this));
	}
	catch (const asio::system_error& ex)
	{
		spdlog::error("TcpServerSocket::Close: failed to post close for socketId={}: {}",
			_socketId, ex.what());
	}
}

void TcpServerSocket::ReleaseToManager()
{
	_socketManager->ReleaseServerSocket(this, GetSocketID());
}

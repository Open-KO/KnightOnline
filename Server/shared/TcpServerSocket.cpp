#include "stdafx.h"
#include "TcpServerSocket.h"
#include "SocketManager.h"

TcpServerSocket::TcpServerSocket(SocketManager* socketManager)
	: TcpSocket(socketManager)
{
}

void TcpServerSocket::Close()
{
	if (_socketManager == nullptr
		|| GetState() == CONNECTION_STATE_DISCONNECTED)
		return;

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

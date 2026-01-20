#include "pch.h"
#include "TcpClientSocketManager.h"
#include "TcpSocket.h"

TcpClientSocketManager::TcpClientSocketManager(int recvBufferSize, int sendBufferSize,
	std::string_view implName) : TcpSocketManager(recvBufferSize, sendBufferSize, implName)
{
}

TcpClientSocketManager::~TcpClientSocketManager()
{
	try
	{
		Shutdown();
	}
	catch (const std::exception& ex)
	{
		spdlog::error(
			"TcpClientSocketManager({})::~TcpClientSocketManager: exception occurred - {}",
			_implName, ex.what());
	}
}

std::shared_ptr<TcpClientSocket> TcpClientSocketManager::AcquireSocket()
{
	int socketId = -1;
	std::shared_ptr<TcpClientSocket> tcpSocket;

	// NOTE: Handle the guarding externally so it's clear what's guarded and what's not,
	// which is critical when dealing with code needing to be fairly high performance here.
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		tcpSocket = std::static_pointer_cast<TcpClientSocket>(
			TcpSocketManager::AcquireSocket(socketId));
	}

	if (socketId == -1)
	{
		spdlog::error(
			"TcpClientSocketManager({})::AcquireSocket: socketId list is empty", _implName);
		return nullptr;
	}

	// This should never happen.
	// If it does, the associated socket ID was never removed from the list so we don't have to restore it.
	if (tcpSocket == nullptr)
	{
		spdlog::error("TcpClientSocketManager({})::AcquireSocket: null socket [socketId:{}]",
			_implName, socketId);
		return nullptr;
	}

	return tcpSocket;
}

bool TcpClientSocketManager::ProcessClose(TcpSocket* tcpSocket)
{
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	if (tcpSocket->GetState() == CONNECTION_STATE_DISCONNECTED)
		return false;

	// NOTE: Unlike server sockets, client sockets don't get removed
	// from the manager upon disconnect.
	// This allows the user to reconnect to the same socket instance.
	tcpSocket->CloseProcess();
	return true;
}

void TcpClientSocketManager::ReleaseSocket(std::shared_ptr<TcpClientSocket> tcpSocket)
{
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	TcpSocketManager::ReleaseSocket(std::move(tcpSocket));
}

void TcpClientSocketManager::Shutdown()
{
	ShutdownImpl();
}

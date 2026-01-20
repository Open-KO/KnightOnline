#include "pch.h"
#include "EbenezerSocketManager.h"
#include "Define.h"
#include "SendWorkerThread.h"
#include "User.h"

namespace Ebenezer
{

EbenezerSocketManager::EbenezerSocketManager() :
	TcpServerSocketManager(SOCKET_BUFF_SIZE, SOCKET_BUFF_SIZE, "EbenezerSocketManager")
{
	_sendWorkerThread        = new SendWorkerThread(this);

	_startUserThreadCallback = [this]()
	{
		if (_sendWorkerThread != nullptr)
			_sendWorkerThread->start();
	};

	_shutdownUserThreadCallback = [this]()
	{
		if (_sendWorkerThread != nullptr)
			_sendWorkerThread->shutdown();
	};
}

EbenezerSocketManager::~EbenezerSocketManager()
{
	delete _sendWorkerThread;
	_sendWorkerThread = nullptr;
}

std::shared_ptr<CUser> EbenezerSocketManager::GetUser(int socketId) const
{
	return std::static_pointer_cast<CUser>(TcpSocketManager::GetSocket(socketId));
}

std::shared_ptr<CUser> EbenezerSocketManager::GetUserUnchecked(int socketId) const
{
	return std::static_pointer_cast<CUser>(TcpSocketManager::GetSocketUnchecked(socketId));
}

std::shared_ptr<CUser> EbenezerSocketManager::GetInactiveUser(int socketId) const
{
	return std::static_pointer_cast<CUser>(TcpSocketManager::GetInactiveSocket(socketId));
}

std::shared_ptr<CUser> EbenezerSocketManager::GetInactiveUserUnchecked(int socketId) const
{
	return std::static_pointer_cast<CUser>(TcpSocketManager::GetInactiveSocketUnchecked(socketId));
}

} // namespace Ebenezer

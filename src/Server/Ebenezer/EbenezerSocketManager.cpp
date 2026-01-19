#include "pch.h"
#include "EbenezerSocketManager.h"
#include "Define.h"
#include "SendWorkerThread.h"
#include "User.h"

namespace Ebenezer
{

EbenezerSocketManager::EbenezerSocketManager() :
	TcpSocketManager(SOCKET_BUFF_SIZE, SOCKET_BUFF_SIZE)
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

CUser* EbenezerSocketManager::GetUser(int socketId) const
{
	return static_cast<CUser*>(GetSocket(socketId));
}

CUser* EbenezerSocketManager::GetUserUnchecked(int socketId) const
{
	return static_cast<CUser*>(GetSocketUnchecked(socketId));
}

CUser* EbenezerSocketManager::GetInactiveUser(int socketId) const
{
	return static_cast<CUser*>(GetInactiveSocket(socketId));
}

CUser* EbenezerSocketManager::GetInactiveUserUnchecked(int socketId) const
{
	return static_cast<CUser*>(GetInactiveSocketUnchecked(socketId));
}

} // namespace Ebenezer

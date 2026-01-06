#include "pch.h"
#include "EbenezerSocketManager.h"
#include "Define.h"
#include "SendWorkerThread.h"
#include "User.h"

namespace Ebenezer
{

EbenezerSocketManager::EbenezerSocketManager() : SocketManager(SOCKET_BUFF_SIZE, SOCKET_BUFF_SIZE)
{
	_sendWorkerThread           = new SendWorkerThread(this);
	_startUserThreadCallback    = [this]() { _sendWorkerThread->start(); };
	_shutdownUserThreadCallback = [this]() { _sendWorkerThread->shutdown(); };
}

EbenezerSocketManager::~EbenezerSocketManager()
{
	delete _sendWorkerThread;
}

CUser* EbenezerSocketManager::GetUser(int socketId) const
{
	return static_cast<CUser*>(GetServerSocket(socketId));
}

CUser* EbenezerSocketManager::GetUserUnchecked(int socketId) const
{
	return static_cast<CUser*>(GetServerSocketUnchecked(socketId));
}

CUser* EbenezerSocketManager::GetInactiveUser(int socketId) const
{
	return static_cast<CUser*>(GetInactiveServerSocket(socketId));
}

CUser* EbenezerSocketManager::GetInactiveUserUnchecked(int socketId) const
{
	return static_cast<CUser*>(GetInactiveServerSocketUnchecked(socketId));
}

} // namespace Ebenezer

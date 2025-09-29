#include "stdafx.h"
#include "EbenezerSocketManager.h"
#include "Define.h"
#include "SendWorkerThread.h"
#include "User.h"

EbenezerSocketManager::EbenezerSocketManager()
	: SocketManager(SOCKET_BUFF_SIZE, SOCKET_BUFF_SIZE)
{
	_sendWorkerThread = new SendWorkerThread(this);
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

void EbenezerSocketManager::StartUserThreads()
{
	_sendWorkerThread->start();
}

void EbenezerSocketManager::ShutdownUserThreads()
{
	_sendWorkerThread->shutdown();
}

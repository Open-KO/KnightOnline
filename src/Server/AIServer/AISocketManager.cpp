#include "pch.h"
#include "AISocketManager.h"
#include "SendThreadMain.h"
#include "GameSocket.h"

namespace AIServer
{

AISocketManager::AISocketManager() : SocketManager(SOCKET_BUFF_SIZE, SOCKET_BUFF_SIZE)
{
	_sendThreadMain             = new SendThreadMain(this);
	_startUserThreadCallback    = [this]() { _sendThreadMain->start(); };
	_shutdownUserThreadCallback = [this]() { _sendThreadMain->shutdown(); };
}

AISocketManager::~AISocketManager()
{
	delete _sendThreadMain;
}

CGameSocket* AISocketManager::GetServerSocket(int socketId) const
{
	return static_cast<CGameSocket*>(SocketManager::GetServerSocket(socketId));
}

CGameSocket* AISocketManager::GetServerSocketUnchecked(int socketId) const
{
	return static_cast<CGameSocket*>(SocketManager::GetServerSocketUnchecked(socketId));
}

void AISocketManager::QueueSendData(_SEND_DATA* sendData)
{
	_sendThreadMain->queue(sendData);
}

} // namespace AIServer

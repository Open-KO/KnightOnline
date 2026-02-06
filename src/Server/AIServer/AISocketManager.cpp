#include "pch.h"
#include "AISocketManager.h"
#include "SendThreadMain.h"
#include "GameSocket.h"

namespace AIServer
{

AISocketManager::AISocketManager() :
	TcpServerSocketManager(SOCKET_BUFF_SIZE, SOCKET_BUFF_SIZE, "AISocketManager")
{
	_sendThreadMain          = new SendThreadMain(this);

	_startUserThreadCallback = [this]()
	{
		if (_sendThreadMain != nullptr)
			_sendThreadMain->start();
	};

	_shutdownUserThreadCallback = [this]()
	{
		if (_sendThreadMain != nullptr)
			_sendThreadMain->shutdown();
	};
}

AISocketManager::~AISocketManager()
{
	delete _sendThreadMain;
	_sendThreadMain = nullptr;
}

std::shared_ptr<CGameSocket> AISocketManager::GetSocket(int socketId) const
{
	return std::static_pointer_cast<CGameSocket>(TcpSocketManager::GetSocket(socketId));
}

std::shared_ptr<CGameSocket> AISocketManager::GetSocketUnchecked(int socketId) const
{
	return std::static_pointer_cast<CGameSocket>(TcpSocketManager::GetSocketUnchecked(socketId));
}

void AISocketManager::QueueSendData(_SEND_DATA* sendData)
{
	_sendThreadMain->queue(sendData);
}

} // namespace AIServer

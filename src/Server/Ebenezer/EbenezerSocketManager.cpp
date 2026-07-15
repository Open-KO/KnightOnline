#include "pch.h"
#include "EbenezerSocketManager.h"
#include "Define.h"
#include "SendWorkerThread.h"
#include "User.h"

namespace Ebenezer
{

EbenezerSocketManager::EbenezerSocketManager() :
	TcpServerSocketManager(
		GAME_SOCKET_RECV_BUFF_SIZE, GAME_SOCKET_SEND_BUFF_SIZE, "EbenezerSocketManager")
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

std::vector<std::string> EbenezerSocketManager::SnapshotCharacterNames()
{
	std::lock_guard<std::recursive_mutex> lock(GetMutex());
	std::vector<std::string> names;
	for (int socketId = 0; socketId < GetSocketCount(); ++socketId)
	{
		auto user = GetUserUnchecked(socketId);
		if (user != nullptr && user->m_pUserData != nullptr && user->m_pUserData->m_id[0] != '\0')
			names.emplace_back(user->m_pUserData->m_id);
	}
	return names;
}

} // namespace Ebenezer

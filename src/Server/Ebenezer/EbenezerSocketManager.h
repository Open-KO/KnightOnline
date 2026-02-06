#ifndef SERVER_EBENEZER_EBENEZERSOCKETMANAGER_H
#define SERVER_EBENEZER_EBENEZERSOCKETMANAGER_H

#pragma once

#include <shared-server/TcpServerSocketManager.h>

namespace Ebenezer
{

class CUser;
class SendWorkerThread;
class EbenezerSocketManager : public TcpServerSocketManager
{
public:
	EbenezerSocketManager();
	~EbenezerSocketManager() override;

	std::shared_ptr<CUser> GetUser(int socketId) const;
	std::shared_ptr<CUser> GetUserUnchecked(int socketId) const;

	std::shared_ptr<CUser> GetInactiveUser(int socketId) const;
	std::shared_ptr<CUser> GetInactiveUserUnchecked(int socketId) const;

protected:
	SendWorkerThread* _sendWorkerThread;
};

} // namespace Ebenezer

#endif // SERVER_EBENEZER_EBENEZERSOCKETMANAGER_H

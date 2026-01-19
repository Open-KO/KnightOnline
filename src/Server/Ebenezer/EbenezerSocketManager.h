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

	CUser* GetUser(int socketId) const;
	CUser* GetUserUnchecked(int socketId) const;

	CUser* GetInactiveUser(int socketId) const;
	CUser* GetInactiveUserUnchecked(int socketId) const;

protected:
	SendWorkerThread* _sendWorkerThread;
};

} // namespace Ebenezer

#endif // SERVER_EBENEZER_EBENEZERSOCKETMANAGER_H

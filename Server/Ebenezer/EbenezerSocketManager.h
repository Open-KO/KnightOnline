#pragma once

#include <shared-server/SocketManager.h>

class CUser;
class SendWorkerThread;
class EbenezerSocketManager : public SocketManager
{
public:
	EbenezerSocketManager();
	~EbenezerSocketManager() override;

	CUser* GetUser(int socketId) const;
	CUser* GetUserUnchecked(int socketId) const;

	CUser* GetInactiveUser(int socketId) const;
	CUser* GetInactiveUserUnchecked(int socketId) const;

protected:
	void StartUserThreads() override;
	void ShutdownUserThreads() override;

protected:
	SendWorkerThread* _sendWorkerThread;
};

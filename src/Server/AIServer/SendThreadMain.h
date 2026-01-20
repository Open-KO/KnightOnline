#ifndef SERVER_AISERVER_SENDTHREADMAIN_H
#define SERVER_AISERVER_SENDTHREADMAIN_H

#pragma once

#include <shared/Thread.h>

#include <queue>

namespace AIServer
{

class AISocketManager;
struct _SEND_DATA;
class SendThreadMain : public Thread
{
public:
	SendThreadMain(AISocketManager* socketManager);
	void shutdown(bool waitForShutdown = true) override;
	void queue(_SEND_DATA* sendData);
	~SendThreadMain() override;

protected:
	void thread_loop() override;
	void tick(std::queue<_SEND_DATA*>& processingQueue);
	void clear();

protected:
	AISocketManager* _serverSocketManager;
	std::queue<_SEND_DATA*> _insertionQueue;
	int _nextRoundRobinSocketId;
};

} // namespace AIServer

#endif // SERVER_AISERVER_SENDTHREADMAIN_H

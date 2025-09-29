#pragma once

#include <shared/Thread.h>

#include <queue>

class AISocketManager;
struct _SEND_DATA;
class SendThreadMain : public Thread
{
public:
	SendThreadMain(AISocketManager* socketManager);
	bool shutdown() override;
	void queue(_SEND_DATA* sendData);
	~SendThreadMain() override;

protected:
	void thread_loop() override;
	void tick(std::queue<_SEND_DATA*>& processingQueue);
	void clear();

protected:
	AISocketManager*		_socketManager;
	std::queue<_SEND_DATA*>	_insertionQueue;
	int						_aiSocketCount;
};

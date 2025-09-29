#pragma once

#include <shared/Thread.h>

class EbenezerSocketManager;
class SendWorkerThread : public Thread
{
public:
	SendWorkerThread(EbenezerSocketManager* socketManager);
	~SendWorkerThread() override;

protected:
	void thread_loop() override;
	void tick();

protected:
	EbenezerSocketManager* _socketManager;
};

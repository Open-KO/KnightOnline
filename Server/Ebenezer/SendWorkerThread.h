#pragma once

#include <shared/Thread.h>

class CIOCPort;
class SendWorkerThread : public Thread
{
public:
	SendWorkerThread(CIOCPort* iocPort);
	~SendWorkerThread() override;

protected:
	void thread_loop() override;
	void tick();

protected:
	CIOCPort* _iocPort;
};

#pragma once

#include <shared/Thread.h>

class SharedMemoryQueue;
class ReadQueueThread : public Thread
{
public:
	ReadQueueThread(SharedMemoryQueue& sharedMemoryQueue);

protected:
	void thread_loop() override;
	virtual void process_packet(const char* buffer, int len) = 0;

protected:
	SharedMemoryQueue& _sharedMemoryQueue;
};

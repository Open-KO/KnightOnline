#include "pch.h"
#include "ReadQueueThread.h"
#include "SharedMemoryQueue.h"

#include <array>

using namespace std::chrono_literals;

ReadQueueThread::ReadQueueThread(SharedMemoryQueue& sharedMemoryQueue) :
	_sharedMemoryQueue(sharedMemoryQueue)
{
}

void ReadQueueThread::thread_loop()
{
	std::array<char, SharedMemoryQueue::MAX_MSG_SIZE> buffer {};
	int len = 0;

	while (CanTick())
	{
		len = _sharedMemoryQueue.GetData(buffer.data());
		if (len >= SMQ_ERROR_RANGE)
		{
			std::unique_lock<std::mutex> lock(ThreadMutex());
			ThreadCondition().wait_for(lock, 100ms);

			if (!CanTick())
				break;

			continue;
		}

		process_packet(buffer.data(), len);
		buffer.fill(0);
	}

	// Read everything remaining in the buffer on shutdown
	len = _sharedMemoryQueue.GetData(buffer.data());
	while (len < SMQ_ERROR_RANGE)
	{
		process_packet(buffer.data(), len);
		buffer.fill(0);

		len = _sharedMemoryQueue.GetData(buffer.data());
	}
}

#include "pch.h"
#include "ReadQueueThread.h"
#include "SharedMemoryQueue.h"

using namespace std::chrono_literals;

ReadQueueThread::ReadQueueThread(SharedMemoryQueue& sharedMemoryQueue)
	: _sharedMemoryQueue(sharedMemoryQueue)
{
}

void ReadQueueThread::thread_loop()
{
	char buffer[SharedMemoryQueue::MAX_MSG_SIZE] = {};
	int len;

	while (_canTick)
	{
		len = _sharedMemoryQueue.GetData(buffer);
		if (len >= SMQ_ERROR_RANGE)
		{
			std::unique_lock<std::mutex> lock(_mutex);
			_cv.wait_for(lock, 100ms);

			if (!_canTick)
				break;

			continue;
		}

		process_packet(buffer, len);
		memset(buffer, 0, sizeof(buffer));
	}

	// Read everything remaining in the buffer on shutdown
	len = _sharedMemoryQueue.GetData(buffer);
	while (len < SMQ_ERROR_RANGE)
	{
		process_packet(buffer, len);
		memset(buffer, 0, sizeof(buffer));

		len = _sharedMemoryQueue.GetData(buffer);
	}
}

#include "stdafx.h"
#include "ReadQueueThread.h"
#include "SharedMemoryQueue.h"

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
			std::cv_status status = _cv.wait_for(lock, std::chrono::milliseconds(100));

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

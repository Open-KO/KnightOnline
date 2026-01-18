#include "pch.h"
#include "TimerThread.h"

#include <spdlog/spdlog.h>

TimerThread::TimerThread(std::chrono::milliseconds tickDelay, TickCallback_t&& tickCallback) :
	Thread(), _tickDelay(tickDelay), _tickCallback(std::move(tickCallback))
{
}

void TimerThread::thread_loop()
{
	while (CanTick())
	{
		{
			std::unique_lock<std::mutex> lock(ThreadMutex());
			std::cv_status status = ThreadCondition().wait_for(lock, _tickDelay);

			if (!CanTick())
				break;

			// Only tick every _tickDelay, ignore spurious wakeups
			if (status != std::cv_status::timeout)
				continue;
		}

		try
		{
			if (_tickCallback != nullptr)
				_tickCallback();
		}
		catch (const std::exception& ex)
		{
			spdlog::error("TimerThread({}) caught unhandled exception: {}",
				static_cast<void*>(this), ex.what());
		}
	}
}

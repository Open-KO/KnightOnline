#include "pch.h"
#include "TimerThread.h"

#include <spdlog/spdlog.h>

TimerThread::TimerThread(std::chrono::milliseconds tickDelay, TickCallback_t&& tickCallback) :
	Thread(), _tickDelay(tickDelay), _tickCallback(std::move(tickCallback))
{
}

void TimerThread::thread_loop()
{
	auto nextTick = std::chrono::steady_clock::now();
	while (CanTick())
	{
		nextTick += _tickDelay;

		{
			std::unique_lock<std::mutex> lock(ThreadMutex());
			ThreadCondition().wait_until(lock, nextTick, [this]() { return IsSignaled(); });
			ResetSignal();

			if (!CanTick())
				break;
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

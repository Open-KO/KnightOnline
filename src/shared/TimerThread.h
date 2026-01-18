#ifndef SHARED_TIMERTHREAD_H
#define SHARED_TIMERTHREAD_H

#pragma once

#include "Thread.h"

#include <chrono>
#include <functional>

class TimerThread : public Thread
{
public:
	using TickCallback_t = std::function<void()>;

	TimerThread(std::chrono::milliseconds tickDelay, TickCallback_t&& tickCallback);

protected:
	void thread_loop() override;

private:
	std::chrono::milliseconds _tickDelay;
	TickCallback_t _tickCallback;
};

#endif // SHARED_TIMERTHREAD_H

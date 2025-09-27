#include "stdafx.h"
#include "Thread.h"

Thread::Thread()
{
	_running = false;
}

void Thread::start()
{
	if (_running)
		return;

	_running = true;
	_thread = std::thread(&Thread::thread_loop, this);
}

bool Thread::shutdown()
{
	if (!_running)
		return false;

	{
		std::lock_guard<std::mutex> lock(_mutex);
		_running = false;
		_cv.notify_one();
	}

	if (_thread.joinable())
		_thread.join();

	return true;
}

Thread::~Thread()
{
	shutdown();
}

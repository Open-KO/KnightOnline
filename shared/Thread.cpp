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

bool Thread::shutdown(bool join /*= true*/)
{
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (!_running)
			return false;

		_running = false;
		before_shutdown();

		_cv.notify_one();
	}

	if (join && _thread.joinable())
		_thread.join();

	return true;
}

Thread::~Thread()
{
	shutdown();
}

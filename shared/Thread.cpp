#include "stdafx.h"
#include "Thread.h"

Thread::Thread()
{
	_running.store(false);
	_stopped.store(true);
}

void Thread::start()
{
	if (_running.load())
		return;

	_running.store(true);
	_thread = std::thread(&Thread::thread_loop, this);
	_stopped.store(false);
}

bool Thread::shutdown(bool join /*= true*/)
{
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (!_running.load())
			return false;

		_running.store(false);
		before_shutdown();

		_cv.notify_one();
	}

	if (join && _thread.joinable())
		_thread.join();

	_stopped.store(true);
	_stopped.notify_all();
	return true;
}

void Thread::BlockUntilShutdown()
{
	if (!_stopped.load())
	{
		_stopped.wait(true);
	}
}

Thread::~Thread()
{
	shutdown();
}

#include "pch.h"
#include "Thread.h"

#include <spdlog/spdlog.h>

Thread::Thread() : _canTick(false), _isShutdown(true)
{
}

void Thread::start()
{
	if (_canTick)
		return;

	_canTick    = true;
	_isShutdown = false;
	_thread     = std::thread(&Thread::thread_loop_wrapper, this);
}

void Thread::shutdown(bool waitForShutdown /*= true*/)
{
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_canTick)
		{
			_canTick = false;
			before_shutdown();

			_cv.notify_one();
		}
	}

	if (waitForShutdown)
		join();
}

void Thread::join()
{
	if (!_thread.joinable())
		return;

	try
	{
		_thread.join();
	}
	catch (const std::system_error& ex)
	{
		if (ex.code() == std::errc::resource_deadlock_would_occur)
		{
			assert(!"Thread::join: cannot join from same thread, would cause deadlock");
			spdlog::error("Thread::join: cannot join from same thread, would cause deadlock");
		}
		else if (ex.code() == std::errc::no_such_process)
		{
			assert(!"Thread::join: thread is not valid");
			spdlog::error("Thread::join: thread is not valid");
		}
		else
		{
			throw;
		}
	}
}

void Thread::thread_loop_wrapper()
{
	thread_loop();

	_canTick    = false;
	_isShutdown = true;
}

Thread::~Thread()
{
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_canTick)
		{
			_canTick = false;
			_cv.notify_one();
		}
	}

	join();
}

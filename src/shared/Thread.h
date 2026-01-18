#ifndef SHARED_THREAD_H
#define SHARED_THREAD_H

#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

class Thread
{
public:
	bool CanTick() const
	{
		return _canTick;
	}

	bool IsShutdown() const
	{
		return _isShutdown;
	}

	std::mutex& ThreadMutex() const
	{
		return _mutex;
	}

	std::condition_variable& ThreadCondition()
	{
		return _cv;
	}

	Thread();
	virtual void start();
	virtual void shutdown(bool waitForShutdown = true);
	void join();
	virtual ~Thread();

protected:
	void thread_loop_wrapper();

	virtual void thread_loop() = 0;
	virtual void before_shutdown()
	{
	}

private:
	mutable std::mutex _mutex;
	std::condition_variable _cv;
	std::thread _thread;
	bool _canTick;
	bool _isShutdown;
};

#endif // SHARED_THREAD_H

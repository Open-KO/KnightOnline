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

protected:
	bool IsSignaled() const
	{
		return _isSignaled;
	}

public:
	std::mutex& ThreadMutex() const
	{
		return _mutex;
	}

	std::condition_variable& ThreadCondition()
	{
		return _cv;
	}

	void SetSignaled()
	{
		_isSignaled = true;
	}

protected:
	void ResetSignal()
	{
		_isSignaled = false;
	}

public:
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
	bool _canTick    = false;
	bool _isShutdown = true;
	bool _isSignaled = false;
};

#endif // SHARED_THREAD_H

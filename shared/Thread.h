#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

class Thread
{
public:
	bool IsRunning() const
	{
		return _running;
	}

	Thread();
	virtual void start();
	virtual bool shutdown();
	virtual ~Thread();

protected:
	virtual void thread_loop() = 0;
	virtual void before_shutdown() {}

protected:
	std::mutex				_mutex;
	std::condition_variable	_cv;
	std::thread				_thread;
	bool					_running;
};

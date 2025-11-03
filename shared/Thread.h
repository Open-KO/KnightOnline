#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

class Thread
{
public:
	/// \brief Returns true if the thread has been started and has not
	/// been ordered to shutdown yet
	bool IsRunning() const
	{
		return _running.load();
	}

	/// \brief Returns true if the thread has not been fully started, or
	/// if the thread has been fully shut down.
	bool IsStopped() const
	{
		return _stopped.load();
	}

	Thread();
	virtual void start();
	virtual bool shutdown(bool join = true);
	void BlockUntilShutdown();
	virtual ~Thread();

protected:
	virtual void thread_loop() = 0;
	virtual void before_shutdown() {}

protected:
	std::mutex				_mutex;
	std::condition_variable	_cv;
	std::thread				_thread;
	std::atomic<bool>		_running;
	std::atomic<bool>		_stopped;
};

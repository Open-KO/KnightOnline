#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

class CIOCPort;
struct _SEND_DATA;
class SendThreadMain
{
public:
	SendThreadMain(CIOCPort* iocPort);
	void start();
	void shutdown();
	void queue(_SEND_DATA* sendData);
	~SendThreadMain();

protected:
	void run();
	void clear();

protected:
	CIOCPort* _iocPort;
	std::queue<_SEND_DATA*>	_sendDataQueue;
	std::mutex _queueMutex;
	std::condition_variable _cv;
	std::thread _workerThread;
	bool _running;
	int _aiSocketCount;
};

#pragma once

#include <shared/Thread.h>

#include <queue>

class CIOCPort;
struct _SEND_DATA;
class SendThreadMain : public Thread
{
public:
	SendThreadMain(CIOCPort* iocPort);
	bool shutdown() override;
	void queue(_SEND_DATA* sendData);
	~SendThreadMain() override;

protected:
	void thread_loop() override;
	void tick();
	void clear();

protected:
	CIOCPort* _iocPort;
	std::queue<_SEND_DATA*>	_sendDataQueue;
	int _aiSocketCount;
};

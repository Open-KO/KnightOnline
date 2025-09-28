#pragma once

#include <shared/Thread.h>

class CUdpSocket;
class RecvUDPThread : public Thread
{
public:
	RecvUDPThread(CUdpSocket* udpSocket);
	void thread_loop() override;
	~RecvUDPThread() override;

protected:
	CUdpSocket* _udpSocket;
};

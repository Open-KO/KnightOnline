#ifndef SERVER_EBENEZER_RECVUDPTHREAD_H
#define SERVER_EBENEZER_RECVUDPTHREAD_H

#pragma once

#include <shared/Thread.h>

class CUdpSocket;
class RecvUDPThread : public Thread
{
public:
	RecvUDPThread(CUdpSocket* udpSocket);
	~RecvUDPThread() override;

protected:
	void thread_loop() override;
	void before_shutdown() override;

protected:
	CUdpSocket* _udpSocket;
};

#endif // SERVER_EBENEZER_RECVUDPTHREAD_H

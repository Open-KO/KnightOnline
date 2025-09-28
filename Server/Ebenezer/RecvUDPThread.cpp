#include "StdAfx.h"
#include "RecvUDPThread.h"
#include "UdpSocket.h"

RecvUDPThread::RecvUDPThread(CUdpSocket* udpSocket)
	: _udpSocket(udpSocket)
{
}

void RecvUDPThread::thread_loop()
{
	// Kick off our first receive in the chain.
	_udpSocket->AsyncReceive();

	// Handle the main thread loop
	_udpSocket->_io.run();
}

RecvUDPThread::~RecvUDPThread()
{
	_udpSocket->_io.stop();
}

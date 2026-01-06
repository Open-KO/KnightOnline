#include "pch.h"
#include "RecvUDPThread.h"
#include "UdpSocket.h"

namespace Ebenezer
{

RecvUDPThread::RecvUDPThread(CUdpSocket* udpSocket) : _udpSocket(udpSocket)
{
}

void RecvUDPThread::thread_loop()
{
	// Kick off our first receive in the chain.
	_udpSocket->AsyncReceive();

	// Handle the main thread loop
	_udpSocket->_io.run();
}

void RecvUDPThread::before_shutdown()
{
	_udpSocket->_io.stop();
}

RecvUDPThread::~RecvUDPThread()
{
	shutdown();
}

} // namespace Ebenezer

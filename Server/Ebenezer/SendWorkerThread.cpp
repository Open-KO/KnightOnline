#include "stdafx.h"
#include "SendWorkerThread.h"
#include "IOCPSocket2.h"
#include "IOCPort.h"

#include <spdlog/spdlog.h>

SendWorkerThread::SendWorkerThread(CIOCPort* iocPort)
	: _iocPort(iocPort)
{
}

void SendWorkerThread::thread_loop()
{
	std::unique_lock<std::mutex> lock(_mutex);
	while (_running)
	{
		std::cv_status status = _cv.wait_for(lock, std::chrono::milliseconds(200));

		if (!_running)
			break;

		// only tick every 200ms as per official, ignore spurious wakeups
		if (status != std::cv_status::timeout)
			continue;

		// don't really need this locked here
		lock.unlock();

		tick();

		// restore the lock state for the next tick
		lock.lock();
	}
}

void SendWorkerThread::tick()
{
	std::lock_guard<std::recursive_mutex> lock(_iocPort->GetMutex());

	char regionBuffer[REGION_BUFF_SIZE];

	for (int i = 0; i < MAX_USER; i++)
	{
		CIOCPSocket2* pSocket = _iocPort->m_SockArray[i];
		if (pSocket == nullptr)
			continue;

		if (pSocket->_regionBuffer->iLength == 0)
			continue;

		int len = 0;
		memset(regionBuffer, 0, REGION_BUFF_SIZE);
		pSocket->RegionPacketClear(regionBuffer, len);
		if (len < 500)
		{
			pSocket->Send(regionBuffer, len);
		}
		else
		{
			pSocket->SendCompressingPacket(regionBuffer, len);
			// TRACE(_T("Region Packet %d Bytes\n"), len);
		}
	}
}

SendWorkerThread::~SendWorkerThread()
{
	shutdown();
}

#include "stdafx.h"
#include "SendWorkerThread.h"
#include "EbenezerSocketManager.h"
#include "User.h"
#include "Define.h"

#include <spdlog/spdlog.h>

SendWorkerThread::SendWorkerThread(EbenezerSocketManager* socketManager)
	: _socketManager(socketManager)
{
}

void SendWorkerThread::thread_loop()
{
	while (_running)
	{
		{
			std::unique_lock<std::mutex> lock(_mutex);
			std::cv_status status = _cv.wait_for(lock, std::chrono::milliseconds(200));

			if (!_running)
				break;

			// Only tick every 200ms as per official, ignore spurious wakeups
			if (status != std::cv_status::timeout)
				continue;
		}

		// Our thread mutex doesn't need to be locked while processing external sockets.
		// For that we should use its own mutex.
		tick();
	}
}

void SendWorkerThread::tick()
{
	char regionBuffer[REGION_BUFF_SIZE];

	int socketCount = _socketManager->GetServerSocketCount();
	for (int i = 0; i < socketCount; i++)
	{
		CUser* userSocket = _socketManager->GetUserUnchecked(i);
		if (userSocket == nullptr)
			continue;

		if (userSocket->_regionBuffer->iLength == 0)
			continue;

		int len = 0;
		memset(regionBuffer, 0, REGION_BUFF_SIZE);

		{
			std::lock_guard<std::recursive_mutex> lock(_socketManager->GetMutex());
			userSocket->RegionPacketClear(regionBuffer, len);
		}

		if (len < 500)
		{
			userSocket->Send(regionBuffer, len);
		}
		else
		{
			userSocket->SendCompressingPacket(regionBuffer, len);
			// TRACE(_T("Region Packet %d Bytes\n"), len);
		}
	}
}

SendWorkerThread::~SendWorkerThread()
{
	shutdown();
}

#include "pch.h"
#include "SendWorkerThread.h"
#include "EbenezerSocketManager.h"
#include "User.h"
#include "Define.h"

#include <spdlog/spdlog.h>

using namespace std::chrono_literals;

SendWorkerThread::SendWorkerThread(EbenezerSocketManager* socketManager)
	: _socketManager(socketManager)
{
}

void SendWorkerThread::thread_loop()
{
	while (_canTick)
	{
		{
			std::unique_lock<std::mutex> lock(_mutex);
			std::cv_status status = _cv.wait_for(lock, 200ms);

			if (!_canTick)
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
	int socketCount = _socketManager->GetServerSocketCount();
	for (int i = 0; i < socketCount; i++)
	{
		CUser* userSocket = _socketManager->GetUserUnchecked(i);
		if (userSocket == nullptr)
			continue;

		char regionBuffer[REGION_BUFF_SIZE] = {};
		int len = userSocket->RegionPacketClear(regionBuffer);
		if (len <= 0)
			continue;

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

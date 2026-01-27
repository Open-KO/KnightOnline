#include "pch.h"
#include "SendWorkerThread.h"
#include "EbenezerSocketManager.h"
#include "User.h"
#include "Define.h"

#include <spdlog/spdlog.h>

using namespace std::chrono_literals;

namespace Ebenezer
{

SendWorkerThread::SendWorkerThread(EbenezerSocketManager* socketManager) :
	_serverSocketManager(socketManager)
{
}

void SendWorkerThread::thread_loop()
{
	auto nextTick = std::chrono::steady_clock::now();
	while (CanTick())
	{
		nextTick += 200ms;

		{
			std::unique_lock<std::mutex> lock(ThreadMutex());
			ThreadCondition().wait_until(lock, nextTick, [this]() { return IsSignaled(); });
			ResetSignal();

			if (!CanTick())
				break;
		}

		// Our thread mutex doesn't need to be locked while processing external sockets.
		// For that we should use its own mutex.
		tick();
	}
}

void SendWorkerThread::tick()
{
	int socketCount = _serverSocketManager->GetSocketCount();
	for (int i = 0; i < socketCount; i++)
	{
		auto userSocket = _serverSocketManager->GetUserUnchecked(i);
		if (userSocket == nullptr)
			continue;

		char regionBuffer[REGION_BUFF_SIZE] {};
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

} // namespace Ebenezer

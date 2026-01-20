#include "pch.h"
#include "SendThreadMain.h"
#include "GameSocket.h"
#include "AISocketManager.h"

#include <spdlog/spdlog.h>

namespace AIServer
{

SendThreadMain::SendThreadMain(AISocketManager* socketManager) :
	_serverSocketManager(socketManager), _nextRoundRobinSocketId(0)
{
}

void SendThreadMain::shutdown(bool waitForShutdown /*= true*/)
{
	Thread::shutdown(waitForShutdown);
	clear();
}

void SendThreadMain::queue(_SEND_DATA* sendData)
{
	{
		std::lock_guard<std::mutex> lock(ThreadMutex());
		if (!CanTick())
		{
			delete sendData;
			return;
		}

		_insertionQueue.push(sendData);
	}

	// Ensure mutex is unlocked before notification to avoid unnecessary
	// contention on thread wakeup
	ThreadCondition().notify_one();
}

void SendThreadMain::thread_loop()
{
	std::queue<_SEND_DATA*> processingQueue;

	// Use a predicate here to avoid spurious wakeups
	auto waitUntilPredicate = [this]() -> bool
	{
		// Wait until we're shutting down
		return !CanTick()
			   // Or there's something in the queue
			   || !_insertionQueue.empty();
	};

	while (CanTick())
	{
		{
			std::unique_lock<std::mutex> lock(ThreadMutex());
			ThreadCondition().wait(lock, waitUntilPredicate);

			if (!CanTick())
				break;

			// As tick() processes the entire queue, we don't need to worry
			// about memory management or entries being lost
			processingQueue.swap(_insertionQueue);
		}

		// tick() will process the entire queue
		tick(processingQueue);
	}
}

void SendThreadMain::tick(std::queue<_SEND_DATA*>& processingQueue)
{
	int socketCount = _serverSocketManager->GetSocketCount();
	if (socketCount <= 0)
		return;

	while (!processingQueue.empty())
	{
		_SEND_DATA* sendData = processingQueue.front();

		bool sent            = false;
		for (int checkedSockets = 0; checkedSockets < socketCount;
			++checkedSockets, ++_nextRoundRobinSocketId)
		{
			_nextRoundRobinSocketId %= socketCount;

			int socketId             = _nextRoundRobinSocketId;
			++_nextRoundRobinSocketId;

			auto gameSocket = _serverSocketManager->GetSocketUnchecked(socketId);
			if (gameSocket == nullptr)
				continue;

			int ret = gameSocket->Send(sendData->pBuf, sendData->sLength);
			if (ret <= 0)
			{
				spdlog::warn("SendThreadMain::tick: send failed, trying next - ret={} "
							 "opcode={:02X} packetSize={} socketId={}",
					ret, sendData->pBuf[0], sendData->sLength, socketId);
				continue;
			}

			//TRACE(_T("SendThreadMain - Send : size=%d, socket_num=%d\n"), size, count);
			sent = true;
			break;
		}

		if (!sent)
		{
			spdlog::error(
				"SendThreadMain::tick: send failed, skipped packet - opcode={:02X} packetSize={}",
				sendData->pBuf[0], sendData->sLength);
		}

		delete sendData;
		processingQueue.pop();
	}
}

void SendThreadMain::clear()
{
	std::lock_guard<std::mutex> lock(ThreadMutex());
	while (!_insertionQueue.empty())
	{
		delete _insertionQueue.front();
		_insertionQueue.pop();
	}
}

SendThreadMain::~SendThreadMain()
{
	SendThreadMain::shutdown();
}

} // namespace AIServer

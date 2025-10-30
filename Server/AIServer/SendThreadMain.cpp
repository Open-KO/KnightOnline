#include "stdafx.h"
#include "SendThreadMain.h"
#include "GameSocket.h"
#include "AISocketManager.h"

#include <spdlog/spdlog.h>

SendThreadMain::SendThreadMain(AISocketManager* socketManager)
	: _socketManager(socketManager), _aiSocketCount(0)
{
}

bool SendThreadMain::shutdown(bool join /*= true*/)
{
	if (!Thread::shutdown(join))
		return false;

	clear();
	return true;
}

void SendThreadMain::queue(_SEND_DATA* sendData)
{
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (!_running)
			return;

		_insertionQueue.push(sendData);
	}

	// Ensure mutex is unlocked before notification to avoid unnecessary
	// contention on thread wakeup
	_cv.notify_one();
}

void SendThreadMain::thread_loop()
{
	std::queue<_SEND_DATA*> processingQueue;

	// Use a predicate here to avoid spurious wakeups
	auto waitUntilPredicate = [this] -> bool
	{
		// Wait until we're shutting down
		return !_running
			// Or there's something in the queue
			|| !_insertionQueue.empty();
	};

	while (_running)
	{
		{
			std::unique_lock<std::mutex> lock(_mutex);
			_cv.wait(lock, waitUntilPredicate);

			if (!_running)
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
	int socketCount = _socketManager->GetServerSocketCount();
	while (!processingQueue.empty())
	{
		_SEND_DATA* sendData = processingQueue.front();

		int count = -1;
		for (int i = 0; i < socketCount; i++)
		{
			CGameSocket* gameSocket = _socketManager->GetServerSocketUnchecked(i);
			if (gameSocket == nullptr)
				continue;

			count++;

			if (_aiSocketCount == count)
			{
				int size = gameSocket->Send(sendData->pBuf, sendData->sLength);
				if (size <= 0)
				{
					spdlog::error("SendThreadMain::tick: send failed: size={} socket_num={}",
						size, count);
					count--;
					continue;
				}

				if (++_aiSocketCount >= MAX_AI_SOCKET)
					_aiSocketCount = 0;

				//TRACE(_T("SendThreadMain - Send : size=%d, socket_num=%d\n"), size, count);
				break;
			}
		}

		delete sendData;
		processingQueue.pop();
	}
}

void SendThreadMain::clear()
{
	std::lock_guard<std::mutex> lock(_mutex);
	while (!_insertionQueue.empty())
	{
		delete _insertionQueue.front();
		_insertionQueue.pop();
	}
}

SendThreadMain::~SendThreadMain()
{
	shutdown();
	clear();
}

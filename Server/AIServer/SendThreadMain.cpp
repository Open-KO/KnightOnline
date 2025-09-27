#include "stdafx.h"
#include "SendThreadMain.h"
#include "GameSocket.h"
#include "IOCPort.h"

#include <spdlog/spdlog.h>

SendThreadMain::SendThreadMain(CIOCPort* iocPort)
	: _iocPort(iocPort), _aiSocketCount(0)
{
}

bool SendThreadMain::shutdown()
{
	if (!Thread::shutdown())
		return false;

	clear();
	return true;
}

void SendThreadMain::queue(_SEND_DATA* sendData)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_sendDataQueue.push(sendData);
	_cv.notify_one();
}

void SendThreadMain::thread_loop()
{
	while (_running)
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_cv.wait(lock);

		if (!_running)
			break;

		tick();
	}
}

void SendThreadMain::tick()
{
	while (!_sendDataQueue.empty())
	{
		_SEND_DATA* pSendData = _sendDataQueue.front();

		int count = -1;
		for (int i = 0; i < MAX_SOCKET; i++)
		{
			CGameSocket* pSocket = (CGameSocket*) _iocPort->m_SockArray[i];
			if (pSocket == nullptr)
				continue;

			count++;

			if (_aiSocketCount == count)
			{
				int size = pSocket->Send(pSendData->pBuf, pSendData->sLength);
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

		delete pSendData;
		_sendDataQueue.pop();
	}
}

void SendThreadMain::clear()
{
	std::lock_guard<std::mutex> lock(_mutex);
	while (!_sendDataQueue.empty());
	{
		delete _sendDataQueue.front();
		_sendDataQueue.pop();
	}
}

SendThreadMain::~SendThreadMain()
{
	shutdown();
	clear();
}

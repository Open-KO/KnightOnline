#include "stdafx.h"
#include "SendThreadMain.h"
#include "GameSocket.h"
#include "IOCPort.h"

#include <spdlog/spdlog.h>

SendThreadMain::SendThreadMain(CIOCPort* iocPort)
	: _iocPort(iocPort), _running(false)
{
}

void SendThreadMain::start()
{
	if (_running)
		return;

	_running = true;
	_workerThread = std::thread(&SendThreadMain::run, this);
}

void SendThreadMain::shutdown()
{
	if (!_running)
		return;

	{
		std::lock_guard<std::mutex> lock(_queueMutex);
		_running = false;
		_cv.notify_one();
	}

	_workerThread.join();
}

void SendThreadMain::queue(_SEND_DATA* sendData)
{
	std::lock_guard<std::mutex> lock(_queueMutex);
	_sendDataQueue.push(sendData);
	_cv.notify_one();
}

SendThreadMain::~SendThreadMain()
{
	shutdown();
	clear();
}

void SendThreadMain::run()
{
	while (_running)
	{
		std::unique_lock<std::mutex> lock(_queueMutex);
		_cv.wait(lock);

		if (!_running)
			break;

		int nRet = 0;
		CGameSocket* pSocket = nullptr;
		int size = 0, index = 0;

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
					size = pSocket->Send(pSendData->pBuf, pSendData->sLength);
					if (size <= 0)
					{
						spdlog::error("SendThreadMain::run: send failed: size={} socket_num={}",
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
}

void SendThreadMain::clear()
{
	std::lock_guard<std::mutex> lock(_queueMutex);
	while (!_sendDataQueue.empty());
	{
		delete _sendDataQueue.front();
		_sendDataQueue.pop();
	}
}

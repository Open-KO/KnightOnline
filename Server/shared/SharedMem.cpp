#include "stdafx.h"
#include "SharedMem.h"

#include <process.h>
#include <spdlog/spdlog.h>

#include <boost/interprocess/ipc/message_queue.hpp>

using namespace boost::interprocess;

struct message_queue_impl : public message_queue
{
	using message_queue::message_queue;
};

CSharedMemQueue::CSharedMemQueue(int sendRetryCount /*= 0*/)
{
	_sendRetryCount = std::max(0, sendRetryCount);
}

bool CSharedMemQueue::InitializeMMF(uint32_t maxMsgSize, uint32_t maxNumMsg, const char* name, bool openOrCreate /*= true*/)
{
	if (maxNumMsg < MinNumMsg)
	{
		spdlog::error("SharedMem::InitializeMMF: maxNumMsg too small. maxNumMsg={} name='{}'", maxNumMsg, name);
		return false;
	}

	try
	{
		if (openOrCreate)
			_queue = std::make_unique<message_queue_impl>(open_or_create, name, maxNumMsg, maxMsgSize);
		else
			_queue = std::make_unique<message_queue_impl>(open_only, name);
	}
	catch (const interprocess_exception& ex)
	{
		if (openOrCreate)
			spdlog::error("SharedMem::InitializeMMF: failed to open or create shared memory. name='{}' ex='{}'", name, ex.what());
		else
			spdlog::error("SharedMem::InitializeMMF: failed to open existing shared memory. name='{}', ex='{}'", name, ex.what());

		return false;
	}

	// As with previous behaviour, as the expected 'creator' of the queue, flush it, even if we just reopened it.
	if (openOrCreate)
	{
		std::vector<char> buffer(maxMsgSize);
		size_t received;
		uint32_t priority;

		while (true)
		{
			try
			{
				if (!_queue->try_receive(buffer.data(), buffer.size(), received, priority))
					break; // queue is now empty
			}
			catch (interprocess_exception&)
			{
				break;
			}
		}
	}

	return true;
}

int CSharedMemQueue::PutData(const char* pBuf, int size)
{
	if (size > static_cast<int>(_queue->get_max_msg_size()))
	{
		spdlog::error("SharedMem::PutData: data size overflow: {} bytes", size);
		return SMQ_PKTSIZEOVER;
	}
	
	const uint32_t priority = 0;
	int attemptCount = _sendRetryCount + 1;

	for (int i = 0; i < attemptCount; i++)
	{
		try
		{
			if (_queue->try_send(pBuf, size, priority))
				return SMQ_OK;
		}
		catch (interprocess_exception& ex)
		{
			spdlog::error("SharedMem::PutData: fatal exception: {}", ex.what());
			return SMQ_GENERIC_ERROR;
		}

		std::this_thread::yield();
	}

	return SMQ_FULL;
}

int CSharedMemQueue::GetData(char* pBuf)
{
	uint32_t receivedSize = 0;
	uint32_t priority = 0;

	try
	{
		if (!_queue->try_receive(pBuf, _queue->get_max_msg_size(), receivedSize, priority))
			return SMQ_EMPTY;
	}
	catch (interprocess_exception& ex)
	{
		spdlog::error("SharedMem::GetData: fatal exception: {}", ex.what());
		return SMQ_GENERIC_ERROR;
	}

	// On success, return the number of bytes received
	return static_cast<int>(receivedSize);
}

CSharedMemQueue::~CSharedMemQueue()
{
}

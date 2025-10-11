#pragma once

#include <memory>

constexpr int SMQ_ERROR_RANGE = 10000;

enum e_SharedMemQueueError
{
	SMQ_OK			= 1,
	SMQ_FULL		= SMQ_ERROR_RANGE,
	SMQ_EMPTY,
	SMQ_PKTSIZEOVER,
	SMQ_GENERIC_ERROR
};

struct message_queue_impl;
class SharedMemoryQueue
{
	static constexpr uint32_t MinNumMsg = 100;

public:
	SharedMemoryQueue(int sendRetryCount = 0);
	bool Create(const char* name, uint32_t maxMsgSize, uint32_t maxNumMsg);
	bool OpenOrCreate(const char* name, uint32_t maxMsgSize, uint32_t maxNumMsg);
	bool Open(const char* name);
	int GetData(char* pBuf);
	int PutData(const char* pBuf, int size);
	~SharedMemoryQueue();

private:
	void FlushQueue();

private:
	std::unique_ptr<message_queue_impl> _queue;
	int _sendRetryCount;
};

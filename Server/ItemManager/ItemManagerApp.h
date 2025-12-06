#pragma once

#include "Define.h"

#include <shared-server/AppThread.h>
#include <shared-server/SharedMemoryQueue.h>

class ItemManagerLogger;
class ReadQueueThread;
class ItemManagerApp : public AppThread
{
public:
	SharedMemoryQueue m_LoggerRecvQueue;
	std::unique_ptr<ReadQueueThread> _readQueueThread;

public:
	static ItemManagerApp* instance()
	{
		return static_cast<ItemManagerApp*>(s_instance);
	}

	ItemManagerApp(ItemManagerLogger& logger);
	~ItemManagerApp() override;

	bool OnStart() override;

	void ItemLogWrite(const char* pBuf);
	void ExpLogWrite(const char* pBuf);
};

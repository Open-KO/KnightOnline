#ifndef SERVER_ITEMMANAGER_ITEMMANAGERAPP_H
#define SERVER_ITEMMANAGER_ITEMMANAGERAPP_H

#pragma once

#include "Define.h"

#include <shared-server/AppThread.h>
#include <shared-server/SharedMemoryQueue.h>
#include <shared/TimerThread.h>

class ItemManagerLogger;
class ReadQueueThread;
class ItemManagerApp : public AppThread
{
public:
	SharedMemoryQueue					LoggerRecvQueue;

private:
	std::unique_ptr<ReadQueueThread>	_readQueueThread;
	std::unique_ptr<TimerThread>		_smqOpenThread;

public:
	static ItemManagerApp* instance()
	{
		return static_cast<ItemManagerApp*>(s_instance);
	}

	ItemManagerApp(ItemManagerLogger& logger);
	~ItemManagerApp() override;

protected:
	/// \returns The application's ini config path.
	std::filesystem::path ConfigPath() const override;

	bool OnStart() override;

	/// \brief Attempts to open shared memory queue.
	bool AttemptOpenSharedMemory();

	/// \brief Thread tick attempting to open shared memory queue.
	/// \see AttemptOpenSharedMemory
	void AttemptOpenSharedMemoryThreadTick();

	/// \brief Finishes server initialization and starts processing threads.
	void OnSharedMemoryOpened();

public:
	void ItemLogWrite(const char* pBuf);
	void ExpLogWrite(const char* pBuf);
};

#endif // SERVER_ITEMMANAGER_ITEMMANAGERAPP_H

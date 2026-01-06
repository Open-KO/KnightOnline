#ifndef SERVER_ITEMMANAGER_ITEMMANAGERREADQUEUETHREAD_H
#define SERVER_ITEMMANAGER_ITEMMANAGERREADQUEUETHREAD_H

#pragma once

#include <shared-server/ReadQueueThread.h>

namespace ItemManager
{

class ItemManagerReadQueueThread : public ReadQueueThread
{
public:
	ItemManagerReadQueueThread();

protected:
	void process_packet(const char* buffer, int len) override;
};

} // namespace ItemManager

#endif // SERVER_ITEMMANAGER_ITEMMANAGERREADQUEUETHREAD_H

#ifndef SERVER_EBENEZER_EBENEZERREADQUEUETHREAD_H
#define SERVER_EBENEZER_EBENEZERREADQUEUETHREAD_H

#pragma once

#include <shared-server/ReadQueueThread.h>

namespace Ebenezer
{

class EbenezerReadQueueThread : public ReadQueueThread
{
public:
	EbenezerReadQueueThread();

protected:
	void process_packet(const char* buffer, int len) override;
};

} // namespace Ebenezer

#endif // SERVER_EBENEZER_EBENEZERREADQUEUETHREAD_H

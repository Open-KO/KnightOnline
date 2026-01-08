#ifndef SERVER_AUJARD_AUJARDREADQUEUETHREAD_H
#define SERVER_AUJARD_AUJARDREADQUEUETHREAD_H

#pragma once

#include <shared-server/ReadQueueThread.h>

namespace Aujard
{

class CAujardDlg;
class AujardReadQueueThread : public ReadQueueThread
{
public:
	AujardReadQueueThread();

protected:
	void process_packet(const char* buffer, int len) override;
};

} // namespace Aujard

#endif // SERVER_AUJARD_AUJARDREADQUEUETHREAD_H

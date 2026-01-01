#pragma once

#include <shared-server/ReadQueueThread.h>

class CAujardDlg;
class AujardReadQueueThread : public ReadQueueThread
{
public:
	AujardReadQueueThread();

protected:
	void process_packet(const char* buffer, int len) override;
};

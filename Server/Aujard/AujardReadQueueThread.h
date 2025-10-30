#pragma once

#include <shared-server/ReadQueueThread.h>

class CAujardDlg;
class AujardReadQueueThread : public ReadQueueThread
{
public:
	AujardReadQueueThread(CAujardDlg* main);

protected:
	void process_packet(const char* buffer, int len) override;

	CAujardDlg* _main;
};

#pragma once

#include <shared-server/ReadQueueThread.h>

class CEbenezerDlg;
class EbenezerReadQueueThread : public ReadQueueThread
{
public:
	EbenezerReadQueueThread(CEbenezerDlg* main);

protected:
	void process_packet(const char* buffer, int len) override;

	CEbenezerDlg* _main;
};

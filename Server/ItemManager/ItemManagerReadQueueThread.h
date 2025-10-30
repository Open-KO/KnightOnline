#pragma once

#include <shared-server/ReadQueueThread.h>

class CItemManagerDlg;
class ItemManagerReadQueueThread : public ReadQueueThread
{
public:
	ItemManagerReadQueueThread(CItemManagerDlg* main);

protected:
	void process_packet(const char* buffer, int len) override;

	CItemManagerDlg* _main;
};

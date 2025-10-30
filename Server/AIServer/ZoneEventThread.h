#pragma once

#include <shared/Thread.h>

class CServerDlg;
class ZoneEventThread : public Thread
{
public:
	ZoneEventThread(CServerDlg* main);
	void thread_loop() override;

protected:
	CServerDlg* _main;
};

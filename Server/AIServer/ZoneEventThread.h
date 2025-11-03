#pragma once

#include <shared/Thread.h>

class AiServerInstance;
class ZoneEventThread : public Thread
{
public:
	ZoneEventThread();
	void thread_loop() override;

protected:
	AiServerInstance* _main;
};

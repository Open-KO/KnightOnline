#ifndef SERVER_AISERVER_ZONEEVENTTHREAD_H
#define SERVER_AISERVER_ZONEEVENTTHREAD_H

#pragma once

#include <shared/Thread.h>

class AIServerApp;
class ZoneEventThread : public Thread
{
public:
	ZoneEventThread();
	void thread_loop() override;

protected:
	AIServerApp* _main;
};

#endif // SERVER_AISERVER_ZONEEVENTTHREAD_H

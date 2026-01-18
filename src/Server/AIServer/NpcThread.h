#ifndef SERVER_AISERVER_NPCTHREAD_H
#define SERVER_AISERVER_NPCTHREAD_H

#pragma once

#include <shared/Thread.h>

#include "Define.h" // NPC_NUM

namespace AIServer
{

class CNpc;
class CNpcThread : public Thread
{
public:
	CNpcThread();
	void thread_loop() override;

public:
	CNpc* m_pNpc[NPC_NUM];

	int16_t m_sThreadNumber; // thread number ,, test
};

} // namespace AIServer

#endif // SERVER_AISERVER_NPCTHREAD_H

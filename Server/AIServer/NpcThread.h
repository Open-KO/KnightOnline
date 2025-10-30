#pragma once

#include <shared/Thread.h>

#include "Define.h" // NPC_NUM

class CNpc;
class CNpcThread : public Thread
{
public:
	CNpcThread();
	void thread_loop() override;

public:
	CNpc*	m_pNpc[NPC_NUM];
	uint8_t	m_byNpcUsed[NPC_NUM];

	int16_t	m_sThreadNumber;	// thread number ,, test
};

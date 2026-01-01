// NpcThread.cpp: implementation of the CNpcThread class.
//
//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "NpcThread.h"
#include "Npc.h"
#include "Extern.h"
#include "AIServerApp.h"

#include <chrono>

//////////////////////////////////////////////////////////////////////
// NPC Thread Callback Function
//
void CNpcThread::thread_loop()
{
	uint32_t dwTickTime	= 0;

	srand((unsigned int) time(nullptr));
	myrand(1, 10000);
	myrand(1, 10000);

	double fTime2 = 0.0, fTime3 = 0.0;

	while (_canTick)
	{
		fTime2 = TimeGet();

		for (int i = 0; i < NPC_NUM; i++)
		{
			CNpc* pNpc = m_pNpc[i];
			if (pNpc == nullptr)
				continue;

			//if((pNpc->m_tNpcType == NPCTYPE_DOOR || pNpc->m_tNpcType == NPCTYPE_ARTIFACT || pNpc->m_tNpcType == NPCTYPE_PHOENIX_GATE || pNpc->m_tNpcType == NPCTYPE_GATE_LEVER) && !pNpc->m_bFirstLive) continue;
			//if( pNpc->m_bFirstLive ) continue;

			// 잘못된 몬스터 (임시코드 2002.03.24)
			if (pNpc->m_sNid < 0)
				continue;

			fTime3 = fTime2 - pNpc->m_fDelayTime;
			dwTickTime = static_cast<uint32_t>(fTime3 * 1000);

			//if(i==0)
			//TRACE(_T("thread time = %.2f, %.2f, %.2f, delay=%d, state=%d, nid=%d\n"), pNpc->m_fDelayTime, fTime2, fTime3, dwTickTime, pNpc->m_NpcState, pNpc->m_sNid+NPC_BAND);

			if (pNpc->m_Delay > (int) dwTickTime
				&& !pNpc->m_bFirstLive
				&& pNpc->m_Delay != 0)
			{
				if (pNpc->m_Delay < 0)
					pNpc->m_Delay = 0;

				//적발견시... (2002. 04.23수정, 부하줄이기)
				if (pNpc->m_NpcState == NPC_STANDING
					&& pNpc->CheckFindEnemy())
				{
					if (pNpc->FindEnemy())
					{
						pNpc->m_NpcState = NPC_ATTACKING;
						pNpc->m_Delay = 0;
					}
				}

				continue;
			}

			fTime3 = fTime2 - pNpc->m_fHPChangeTime;
			dwTickTime = static_cast<uint32_t>(fTime3 * 1000);

			// 10초마다 HP를 회복 시켜준다
			if (10000 < dwTickTime)
				pNpc->HpChange();

			pNpc->DurationMagic_4(fTime2);		// 마법 처리...
			pNpc->DurationMagic_3(fTime2);		// 지속마법..

			switch (pNpc->m_NpcState)
			{
				case NPC_LIVE:					// 방금 살아난 경우
					pNpc->NpcLive();
					break;

				case NPC_STANDING:						// 하는 일 없이 서있는 경우
					pNpc->NpcStanding();
					break;

				case NPC_MOVING:
					pNpc->NpcMoving();
					break;

				case NPC_ATTACKING:
					pNpc->NpcAttacking();
					break;

				case NPC_TRACING:
					pNpc->NpcTracing();
					break;

				case NPC_FIGHTING:
					pNpc->NpcFighting();
					break;

				case NPC_BACK:
					pNpc->NpcBack();
					break;

				case NPC_STRATEGY:
					break;

				case NPC_DEAD:
					//pNpc->NpcTrace(_T("NpcDead"));
					pNpc->m_NpcState = NPC_LIVE;
					break;

				case NPC_SLEEPING:
					pNpc->NpcSleeping();
					break;

				case NPC_FAINTING:
					pNpc->NpcFainting(fTime2);
					break;

				case NPC_HEALING:
					pNpc->NpcHealing();
					break;
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

CNpcThread::CNpcThread()
{
	m_sThreadNumber = -1;

	for (int i = 0; i < NPC_NUM; i++)
		m_pNpc[i] = nullptr;
}

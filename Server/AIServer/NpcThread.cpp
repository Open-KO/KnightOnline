// NpcThread.cpp: implementation of the CNpcThread class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "server.h"
#include "NpcThread.h"
#include "Npc.h"
#include "Extern.h"
#include "Mmsystem.h"
#include "ServerDlg.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define DELAY				250

//////////////////////////////////////////////////////////////////////
// NPC Thread Callback Function
//
UINT NpcThreadProc(LPVOID pParam /* NPC_THREAD_INFO ptr */)
{
	NPC_THREAD_INFO*	pInfo		= (NPC_THREAD_INFO*) pParam;
	CNpc*				pNpc		= nullptr;
	CIOCPort*			pIOCP		= nullptr;
	CPoint				pt;

	int					i			= 0;
	DWORD				dwDiffTime	= 0;
	DWORD				dwSleep		= 250;
	DWORD				dwTickTime	= 0;
	srand((unsigned int) time(nullptr));
	myrand(1, 10000);
	myrand(1, 10000);

	float  fTime2 = 0.0f;
	float  fTime3 = 0.0f;
	int    duration_damage = 0;

	if (pInfo == nullptr)
		return 0;

	while (!g_bNpcExit)
	{
		fTime2 = TimeGet();

		for (i = 0; i < NPC_NUM; i++)
		{
			pNpc = pInfo->pNpc[i];
			pIOCP = pInfo->pIOCP;
			if (pNpc == nullptr)
				continue;

			//if((pNpc->m_tNpcType == NPCTYPE_DOOR || pNpc->m_tNpcType == NPCTYPE_ARTIFACT || pNpc->m_tNpcType == NPCTYPE_PHOENIX_GATE || pNpc->m_tNpcType == NPCTYPE_GATE_LEVER) && !pNpc->m_bFirstLive) continue;
			//if( pNpc->m_bFirstLive ) continue;

			// 잘못된 몬스터 (임시코드 2002.03.24)
			if (pNpc->m_sNid < 0)
				continue;

			fTime3 = fTime2 - pNpc->m_fDelayTime;
			dwTickTime = fTime3 * 1000;

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
			dwTickTime = fTime3 * 1000;

			// 10초마다 HP를 회복 시켜준다
			if (10000 < dwTickTime)
				pNpc->HpChange(pIOCP);

			pNpc->DurationMagic_4(pIOCP, fTime2);		// 마법 처리...
			pNpc->DurationMagic_3(pIOCP, fTime2);		// 지속마법..

			switch (pNpc->m_NpcState)
			{
				case NPC_LIVE:					// 방금 살아난 경우
					pNpc->NpcLive(pIOCP);
					break;

				case NPC_STANDING:						// 하는 일 없이 서있는 경우
					pNpc->NpcStanding();
					break;

				case NPC_MOVING:
					pNpc->NpcMoving(pIOCP);
					break;

				case NPC_ATTACKING:
					pNpc->NpcAttacking(pIOCP);
					break;

				case NPC_TRACING:
					pNpc->NpcTracing(pIOCP);
					break;

				case NPC_FIGHTING:
					pNpc->NpcFighting(pIOCP);
					break;

				case NPC_BACK:
					pNpc->NpcBack(pIOCP);
					break;

				case NPC_STRATEGY:
					break;

				case NPC_DEAD:
					//pNpc->NpcTrace(_T("NpcDead"));
					pNpc->m_NpcState = NPC_LIVE;
					break;

				case NPC_SLEEPING:
					pNpc->NpcSleeping(pIOCP);
					break;

				case NPC_FAINTING:
					pNpc->NpcFainting(pIOCP, fTime2);
					break;

				case NPC_HEALING:
					pNpc->NpcHealing(pIOCP);
					break;
			}
		}

		dwSleep = 100;
		Sleep(dwSleep);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////
// NPC Thread Callback Function
//
UINT ZoneEventThreadProc(LPVOID pParam/* = nullptr */)
{
	CServerDlg* m_pMain = (CServerDlg*) pParam;

	while (!g_bNpcExit)
	{
		float fCurrentTime = TimeGet();
		for (MAP* pMap : m_pMain->m_ZoneArray)
		{
			if (pMap == nullptr)
				continue;

			// 현재의 존이 던젼담당하는 존이 아니면 리턴..
			if (pMap->m_byRoomEvent == 0)
				continue;

			// 전체방이 클리어 되었다면
			if (pMap->IsRoomStatusCheck())
				continue;

			// 방번호는 1번부터 시작
			for (auto& [_, pRoom] : pMap->m_arRoomEventArray)
			{
				if (pRoom == nullptr)
					continue;

				// 1:init, 2:progress, 3:clear
				if (pRoom->m_byStatus == 1
					|| pRoom->m_byStatus == 3)  
					continue;

				// 여기서 처리하는 로직...
				pRoom->MainRoom(fCurrentTime);
			}
		}

		Sleep(1000);	// 1초당 한번
	}

	return 0;
}

float TimeGet()
{
	static bool bInit = false;
	static bool bUseHWTimer = false;
	static LARGE_INTEGER nTime, nFrequency;

	if (!bInit)
	{
		if (::QueryPerformanceCounter(&nTime))
		{
			::QueryPerformanceFrequency(&nFrequency);
			bUseHWTimer = true;
		}
		else
		{
			bUseHWTimer = false;
		}

		bInit = true;
	}

	if (bUseHWTimer)
	{
		::QueryPerformanceCounter(&nTime);
		return (float) ((double) (nTime.QuadPart) / (double) nFrequency.QuadPart);
	}

	return (float) timeGetTime();
}

CNpcThread::CNpcThread()
{
	pIOCP = nullptr;
//	m_pNpc =	nullptr;
	m_pThread = nullptr;
	m_sThreadNumber = -1;

	for (int i = 0; i < NPC_NUM; i++)
		m_pNpc[i] = nullptr;
}

CNpcThread::~CNpcThread()
{
/*	for( int i = 0; i < NPC_NUM; i++ )
	{
		if(m_pNpc[i])
		{
			delete m_pNpc[i];
			m_pNpc[i] = nullptr;
		}
	}	*/
}

void CNpcThread::InitThreadInfo(HWND hwnd)
{
	m_ThreadInfo.hWndMsg = hwnd;
	m_ThreadInfo.pIOCP = pIOCP;
}

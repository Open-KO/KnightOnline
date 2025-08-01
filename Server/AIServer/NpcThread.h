// NpcThread.h: interface for the CNpcThread class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NPCTHREAD_H__EBEC0AA4_EEF9_4F28_AAF0_6FB20E2891EB__INCLUDED_)
#define AFX_NPCTHREAD_H__EBEC0AA4_EEF9_4F28_AAF0_6FB20E2891EB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "Npc.h"
#include "IOCPort.h"

UINT NpcThreadProc(LPVOID pParam /* NPC_THREAD_INFO ptr */);
UINT ZoneEventThreadProc(LPVOID pParam /* = nullptr */);
float TimeGet();

class CNpc;

typedef struct _NPC_THREAD_INFO
{
	CIOCPort*	pIOCP;
	CNpc*		pNpc[NPC_NUM];
	BYTE		m_byNpcUsed[NPC_NUM];
	HWND		hWndMsg;
} NPC_THREAD_INFO;

class CNpcThread
{
public:
	CNpcThread();
	virtual ~CNpcThread();

public:
	void InitThreadInfo(HWND hwnd);
	CIOCPort* pIOCP;
	CNpc* m_pNpc[NPC_NUM];

	NPC_THREAD_INFO		m_ThreadInfo;
	CWinThread* m_pThread;
	short m_sThreadNumber;					// thread number ,, test

};

#endif // !defined(AFX_NPCTHREAD_H__EBEC0AA4_EEF9_4F28_AAF0_6FB20E2891EB__INCLUDED_)

// GameEvent.cpp: implementation of the CGameEvent class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ebenezer.h"
#include "GameEvent.h"
#include "User.h"
#include "GameDefine.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGameEvent::CGameEvent()
{
	m_bType = 0;

	for (int i = 0; i < 5; i++)
	{
		m_iCond[i] = 0;
		m_iExec[i] = 0;
	}
}

CGameEvent::~CGameEvent()
{
}

void CGameEvent::RunEvent(CUser* pUser)
{
	switch (m_bType)
	{
		case ZONE_CHANGE:
			if (pUser->m_bWarp)
				break;

			pUser->ZoneChange(m_iExec[0], (float) m_iExec[1], (float) m_iExec[2]);
			break;

		case ZONE_TRAP_DEAD:
		//	TRACE(_T("&&& User - zone trap dead ,, name=%hs\n"), pUser->m_pUserData->m_id);
		//	pUser->Dead();
			break;

		case ZONE_TRAP_AREA:
			pUser->TrapProcess();
			break;
	}
}

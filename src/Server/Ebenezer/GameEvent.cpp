#include "pch.h"
#include "GameEvent.h"
#include "User.h"
#include "GameDefine.h"

#include <spdlog/spdlog.h>

namespace Ebenezer
{

CGameEvent::CGameEvent()
{
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

		default:
			spdlog::warn("GameEvent::RunEvent: Unhandled event type [type={} characterName={}]",
				m_bType, pUser->m_pUserData->m_id);
			break;
	}
}

} // namespace Ebenezer

// EventManager.cpp: implementation of the CEventManager class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "EventManager.h"
#include "GameProcedure.h"
#include "PlayerMySelf.h"
#include "N3FXMgr.h"

#include <FileIO/FileReader.h>

constexpr int EVENT_TYPE_POISON = 3;

CEventCell::CEventCell()
{
	m_sEventType = 1;
	m_Rect       = {};
}

CEventCell::~CEventCell()
{
}

void CEventCell::Load(File& file)
{
	file.Read(&m_Rect, sizeof(RECT));
	file.Read(&m_sEventType, sizeof(int16_t));
}

CEventManager::CEventManager()
{
	Release();
}

CEventManager::~CEventManager()
{
	Release();
}

bool CEventManager::LoadFromFile(const char* szFileName)
{
	Release();

	FileReader gevFile;
	if (!gevFile.OpenExisting(szFileName))
		return false;

	int nEventCellCount = 0;
	gevFile.Read(&nEventCellCount, sizeof(int));

	for (int i = 0; i < nEventCellCount; i++)
	{
		CEventCell* pEventCell = new CEventCell();
		pEventCell->Load(gevFile);
		m_lstEvents.push_back(pEventCell);
	}

	return true;
}

void CEventManager::Release()
{
	m_sEventType = -1;
	memset(&m_rcEvent, 0, sizeof(RECT));

	for (CEventCell* pEventCell : m_lstEvents)
		delete pEventCell;
	m_lstEvents.clear();
}

int16_t CEventManager::SetPos(float fX, float fZ)
{
	int x = (int) fX;
	int y = (int) fZ;

	if (PtInRect(x, y, m_rcEvent))
		return m_sEventType;

	for (CEventCell* pEventCell : m_lstEvents)
	{
		if (pEventCell == nullptr)
			continue;

		if (!PtInRect(x, y, pEventCell->m_Rect))
			continue;

		if (m_sEventType != pEventCell->m_sEventType)
			Behavior(pEventCell->m_sEventType, m_sEventType);

		m_rcEvent    = pEventCell->m_Rect;
		m_sEventType = pEventCell->m_sEventType;
		return pEventCell->m_sEventType;
	}

	if (m_sEventType != -1)
	{
		Behavior(-1, m_sEventType);
		m_sEventType = -1;
		memset(&m_rcEvent, 0, sizeof(RECT));
	}

	return m_sEventType;
}

bool CEventManager::PtInRect(int x, int z, RECT rc)
{
	if (x < rc.left)
		return false;
	if (x > rc.right)
		return false;
	if (z < rc.top)
		return false;
	if (z > rc.bottom)
		return false;

	return true;
}

void CEventManager::Behavior(int16_t sEventType, int16_t sPreEventType)
{
	switch (sPreEventType)
	{
		case EVENT_TYPE_POISON:
		{
			int iID = CGameBase::s_pPlayer->IDNumber();
			int iFX = FXID_REGION_POISON;
			CGameProcedure::s_pFX->Stop(iID, iID, iFX, -1, true);
		}
		break;

		default:
			break;
	}

	switch (sEventType)
	{
		case EVENT_TYPE_POISON:
		{
			int iID = CGameBase::s_pPlayer->IDNumber();
			int iFX = FXID_REGION_POISON;
			CGameProcedure::s_pFX->TriggerBundle(iID, 0, iFX, iID, -1, FX_BUNDLE_REGION_POISON);
		}
		break;

		default:
			break;
	}
}

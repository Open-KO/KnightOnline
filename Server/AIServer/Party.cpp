// Party.cpp: implementation of the CParty class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "server.h"
#include "ServerDlg.h"
#include "Party.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

extern CRITICAL_SECTION g_region_critical;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CParty::CParty()
{
	m_pMain = nullptr;
}

CParty::~CParty()
{
}

void CParty::Initialize()
{
	m_pMain = (CServerDlg*) AfxGetMainWnd();
}

void CParty::PartyProcess(char* pBuf)
{
	int index = 0;
	BYTE subcommand;

	subcommand = GetByte(pBuf, index);
	switch (subcommand)
	{
		case PARTY_CREATE:
			PartyCreate(pBuf + index);
			break;

		case PARTY_INSERT:
			PartyInsert(pBuf + index);
			break;

		case PARTY_REMOVE:
			PartyRemove(pBuf + index);
			break;

		case PARTY_DELETE:
			PartyDelete(pBuf + index);
			break;
	}
}

void CParty::PartyCreate(char* pBuf)
{
	int index = 0;
	short sPartyIndex = 0;
	short sUid = 0, sHP = 0, sClass = 0;
	BYTE  byLevel = 0;
	_PARTY_GROUP* pParty = nullptr;
	CUser* pUser = nullptr;

	sPartyIndex = GetShort(pBuf, index);
	sUid = GetShort(pBuf, index);
	//sHP = GetShort(pBuf, index);
	//byLevel = GetByte(pBuf, index);
	//sClass = GetShort(pBuf, index);

	pUser = m_pMain->GetUserPtr(sUid);
	if (pUser)
	{
		pUser->m_byNowParty = 1;
		pUser->m_sPartyNumber = sPartyIndex;
	}

	EnterCriticalSection(&g_region_critical);

	pParty = new _PARTY_GROUP;
	pParty->wIndex = sPartyIndex;
	pParty->uid[0] = sUid;

	if (m_pMain->m_arParty.PutData(pParty->wIndex, pParty))
	{
		TRACE("Party - Create() : Party 생성  number = %d, uid=%d, %d \n", sPartyIndex, pParty->uid[0], pParty->uid[1]);
	}
	LeaveCriticalSection(&g_region_critical);
}

void CParty::PartyInsert(char* pBuf)
{
	int index = 0;
	short sPartyIndex = 0;
	BYTE  byIndex = -1;
	short sUid = 0, sHP = 0, sClass = 0;
	BYTE  byLevel = 0;
	_PARTY_GROUP* pParty = nullptr;
	CUser* pUser = nullptr;

	sPartyIndex = GetShort(pBuf, index);
	byIndex = GetByte(pBuf, index);
	sUid = GetShort(pBuf, index);
	//sHP = GetShort(pBuf, index);
	//byLevel = GetByte(pBuf, index);
	//sClass = GetShort(pBuf, index);

	pParty = m_pMain->m_arParty.GetData(sPartyIndex);

	// 이상한 경우
	if (!pParty)
		return;

	if (byIndex >= 0
		&& byIndex < 8)
	{
		pParty->uid[byIndex] = sUid;

		pUser = m_pMain->GetUserPtr(sUid);
		if (pUser)
		{
			pUser->m_byNowParty = 1;
			pUser->m_sPartyNumber = sPartyIndex;
		}
	}
}

void CParty::PartyRemove(char* pBuf)
{
	int index = 0;
	short sPartyIndex = 0;
	short sUid = 0;
	_PARTY_GROUP* pParty = nullptr;
	CUser* pUser = nullptr;

	sPartyIndex = GetShort(pBuf, index);
	sUid = GetShort(pBuf, index);

	if (sUid < 0
		|| sUid > MAX_USER)
		return;

	if (sPartyIndex <= -1)
		return;

	pParty = m_pMain->m_arParty.GetData(sPartyIndex);

	// 이상한 경우
	if (!pParty)
		return;

	for (int i = 0; i < 8; i++)
	{
		if (pParty->uid[i] != -1)
		{
			if (pParty->uid[i] == sUid)
			{
				pParty->uid[i] = -1;

				pUser = m_pMain->GetUserPtr(sUid);
				if (pUser)
				{
					pUser->m_byNowParty = 0;
					pUser->m_sPartyNumber = -1;
				}
			}
		}
	}
}

void CParty::PartyDelete(char* pBuf)
{
	int index = 0;
	short sPartyIndex = 0;
	_PARTY_GROUP* pParty = nullptr;
	CUser* pUser = nullptr;

	sPartyIndex = GetShort(pBuf, index);

	if (sPartyIndex <= -1)
		return;

	pParty = m_pMain->m_arParty.GetData(sPartyIndex);

	// 이상한 경우
	if (!pParty)
		return;

	for (int i = 0; i < 8; i++)
	{
		if (pParty->uid[i] != -1)
		{
			pUser = m_pMain->GetUserPtr(pParty->uid[i]);
			if (pUser)
			{
				pUser->m_byNowParty = 0;
				pUser->m_sPartyNumber = -1;
			}
		}
	}

	EnterCriticalSection(&g_region_critical);

	m_pMain->m_arParty.DeleteData(pParty->wIndex);

	LeaveCriticalSection(&g_region_critical);
}

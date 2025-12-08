#include "pch.h"
#include "AIServerApp.h"
#include "Party.h"

#include <spdlog/spdlog.h>

extern std::mutex g_region_mutex;

CParty::CParty()
{
	m_pMain = AIServerApp::instance();
}

CParty::~CParty()
{
	m_pMain = nullptr;
}

void CParty::PartyProcess(char* pBuf)
{
	int index = 0;
	uint8_t subcommand;

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
	int16_t sPartyIndex = 0;
	int16_t sUid = 0;
	_PARTY_GROUP* pParty = nullptr;
	CUser* pUser = nullptr;

	sPartyIndex = GetShort(pBuf, index);
	sUid = GetShort(pBuf, index);

	pUser = m_pMain->GetUserPtr(sUid);
	if (pUser != nullptr)
	{
		pUser->m_byNowParty = 1;
		pUser->m_sPartyNumber = sPartyIndex;
	}

	std::unique_lock<std::mutex> lock(g_region_mutex);

	pParty = new _PARTY_GROUP;
	pParty->wIndex = sPartyIndex;
	pParty->uid[0] = sUid;

	bool ret = m_pMain->_partyMap.PutData(pParty->wIndex, pParty);

	lock.unlock();

	if (!ret)
	{
		spdlog::error("Party::PartyCreate: failed [partyId={} uid0={} uid1={}]",
			sPartyIndex, pParty->uid[0], pParty->uid[1]);
		delete pParty;
		return;
	}

	spdlog::debug("Party::PartyCreate: success [partyId={} uid0={} uid1={}]",
		sPartyIndex, pParty->uid[0], pParty->uid[1]);
}

void CParty::PartyInsert(char* pBuf)
{
	int index = 0;
	int16_t sPartyIndex = 0;
	uint8_t  byIndex = -1;
	int16_t sUid = 0;
	_PARTY_GROUP* pParty = nullptr;
	CUser* pUser = nullptr;

	sPartyIndex = GetShort(pBuf, index);
	byIndex = GetByte(pBuf, index);
	sUid = GetShort(pBuf, index);

	pParty = m_pMain->_partyMap.GetData(sPartyIndex);

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
	int16_t sPartyIndex = 0;
	int16_t sUid = 0;
	_PARTY_GROUP* pParty = nullptr;
	CUser* pUser = nullptr;

	sPartyIndex = GetShort(pBuf, index);
	sUid = GetShort(pBuf, index);

	if (sUid < 0
		|| sUid > MAX_USER)
		return;

	if (sPartyIndex <= -1)
		return;

	pParty = m_pMain->_partyMap.GetData(sPartyIndex);

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
	int16_t sPartyIndex = 0;
	_PARTY_GROUP* pParty = nullptr;
	CUser* pUser = nullptr;

	sPartyIndex = GetShort(pBuf, index);

	if (sPartyIndex <= -1)
		return;

	pParty = m_pMain->_partyMap.GetData(sPartyIndex);

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

	std::lock_guard<std::mutex> lock(g_region_mutex);
	m_pMain->_partyMap.DeleteData(pParty->wIndex);
}

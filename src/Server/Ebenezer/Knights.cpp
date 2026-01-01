#include "pch.h"
#include "Knights.h"
#include "User.h"
#include "GameDefine.h"
#include "EbenezerApp.h"

#include <shared/packets.h>

CKnights::CKnights()
{
}

CKnights::~CKnights()
{
}

void CKnights::InitializeValue()
{
	m_sIndex = 0;
	m_byFlag = 0;			// 1 : Clan, 2 : 기사단
	m_byNation = 0;			// nation
	m_byGrade = 0;			// clan 등급 (1 ~ 5등급)
	m_byRanking = 0;		// clan 등급 (1 ~ 5등)
	m_sMembers = 0;
	memset(m_strName, 0, sizeof(m_strName));
	memset(m_strChief, 0, sizeof(m_strChief));
	memset(m_strViceChief_1, 0, sizeof(m_strViceChief_1));	// 부단장 1
	memset(m_strViceChief_2, 0, sizeof(m_strViceChief_2));	// 부단장 2 (기사단에서는 장교)
	memset(m_strViceChief_3, 0, sizeof(m_strViceChief_3));	// 부단장 3 (기사단에서는 사용하지 않음)
	memset(m_Image, 0, sizeof(m_Image));
	m_nMoney = 0;
	m_sAllianceKnights = 0;
	m_sMarkVersion = 0;
	m_sCape = -1;
	m_sDomination = 0;
	m_nPoints = 0;

	for (int i = 0; i < MAX_CLAN; i++)
	{
		m_arKnightsUser[i].byUsed = 0;
		memset(m_arKnightsUser[i].strUserName, 0, sizeof(m_arKnightsUser[i].strUserName));
	}

	for (int i = 0; i < MAX_KNIGHTS_BANK; i++)
	{
		m_StashItem[i].nNum = 0;
		m_StashItem[i].sCount = 0;
		m_StashItem[i].sDuration = 0;
	}
}

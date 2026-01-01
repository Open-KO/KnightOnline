#ifndef SERVER_EBENEZER_KNIGHTS_H
#define SERVER_EBENEZER_KNIGHTS_H

#pragma once

#include "Define.h"
#include "GameDefine.h"

class CUser;
class EbenezerApp;
class CKnights
{
public:
	int16_t			m_sIndex;
	uint8_t			m_byFlag;							// 1 : Clan, 2 : 기사단
	uint8_t			m_byNation;							// nation
	uint8_t			m_byGrade;							// clan 등급 (1 ~ 5등급)
	uint8_t			m_byRanking;						// clan 등급 (1 ~ 5등)
	char			m_strName[MAX_ID_SIZE + 1];
	int16_t			m_sMembers;
	char			m_strChief[MAX_ID_SIZE + 1];
	char			m_strViceChief_1[MAX_ID_SIZE + 1];	// 부단장 1
	char			m_strViceChief_2[MAX_ID_SIZE + 1];	// 부단장 2 (기사단에서는 장교)
	char			m_strViceChief_3[MAX_ID_SIZE + 1];	// 부단장 3	(기사단에서는 사용하지 않음)
	int64_t			m_nMoney;
	int16_t			m_sAllianceKnights;
	int16_t			m_sMarkVersion;
	int16_t			m_sCape;
	int16_t			m_sDomination;
	int				m_nPoints;
	uint8_t			m_Image[MAX_KNIGHTS_MARK];
	_ITEM_DATA		m_StashItem[MAX_KNIGHTS_BANK];
	_KNIGHTS_USER	m_arKnightsUser[MAX_CLAN];			// 클랜원의 정보

public:
	CKnights();
	virtual ~CKnights();

	void InitializeValue();
};

#endif // SERVER_EBENEZER_KNIGHTS_H

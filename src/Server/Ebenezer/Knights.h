#ifndef SERVER_EBENEZER_KNIGHTS_H
#define SERVER_EBENEZER_KNIGHTS_H

#pragma once

#include "Define.h"
#include "GameDefine.h"

namespace Ebenezer
{

class CUser;
class EbenezerApp;
class CKnights
{
public:
	int16_t m_sIndex                         = 0;
	uint8_t m_byFlag                         = 0; // 1 : Clan, 2 : 기사단
	uint8_t m_byNation                       = 0; // nation
	uint8_t m_byGrade                        = 0; // clan 등급 (1 ~ 5등급)
	uint8_t m_byRanking                      = 0; // clan 등급 (1 ~ 5등)
	char m_strName[MAX_ID_SIZE + 1]          = {};
	int16_t m_sMembers                       = 0;
	char m_strChief[MAX_ID_SIZE + 1]         = {};
	char m_strViceChief_1[MAX_ID_SIZE + 1]   = {}; // 부단장 1
	char m_strViceChief_2[MAX_ID_SIZE + 1]   = {}; // 부단장 2 (기사단에서는 장교)
	char m_strViceChief_3[MAX_ID_SIZE + 1]   = {}; // 부단장 3	(기사단에서는 사용하지 않음)
	int64_t m_nMoney                         = 0;
	int16_t m_sAllianceKnights               = 0;
	int16_t m_sMarkVersion                   = 0;
	int16_t m_sCape                          = 0;
	int16_t m_sDomination                    = 0;
	int m_nPoints                            = 0;
	uint8_t m_Image[MAX_KNIGHTS_MARK]        = {};
	_ITEM_DATA m_StashItem[MAX_KNIGHTS_BANK] = {};
	_KNIGHTS_USER m_arKnightsUser[MAX_CLAN]  = {}; // 클랜원의 정보

public:
	CKnights();
	virtual ~CKnights();

	void InitializeValue();
};

} // namespace Ebenezer

#endif // SERVER_EBENEZER_KNIGHTS_H

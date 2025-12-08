#ifndef SERVER_SHAREDSERVER__USER_DATA_H
#define SERVER_SHAREDSERVER__USER_DATA_H

#pragma once

#include <shared/globals.h>

struct _ITEM_DATA
{
	int32_t		nNum;		// item 번호
	int16_t		sDuration;	// item 내구력
	int16_t		sCount;		// item 갯수 or item 축복 속성에 해당 값
	uint8_t		byFlag;
	uint16_t	sTimeRemaining;
	int64_t		nSerialNum;	// item serial code
};

struct _WAREHOUSE_ITEM_DATA
{
	int32_t	nNum;		// item 번호
	int16_t	sDuration;	// item 내구력
	int16_t	sCount;		// item 갯수 or item 축복 속성에 해당 값
	int64_t	nSerialNum;	// item serial code
};

struct _USER_QUEST
{
	int16_t	sQuestID;
	uint8_t	byQuestState;
};

struct _USER_DATA
{
	char		m_id[MAX_ID_SIZE + 1];			// 유저 ID
	char		m_Accountid[MAX_ID_SIZE + 1];	// 계정 ID

	uint8_t		m_bZone;						// 현재 Zone
	float		m_curx;							// 현재 X 좌표
	float		m_curz;							// 현재 Z 좌표
	float		m_cury;							// 현재 Y 좌표

	uint8_t		m_bNation;						// 소속국가
	uint8_t		m_bRace;						// 종족
	int16_t		m_sClass;						// 직업
	uint8_t		m_bHairColor;					// 머리색깔
	uint8_t		m_bRank;						// 작위
	uint8_t		m_bTitle;						// 지위
	uint8_t		m_bLevel;						// 레벨
	int32_t		m_iExp;							// 경험치
	int32_t		m_iLoyalty;						// 로열티
	int32_t		m_iLoyaltyMonthly;				// 로열티
	uint8_t		m_bFace;						// 얼굴모양
	uint8_t		m_bCity;						// 소속도시
	int16_t		m_bKnights;						// 소속 기사단
	uint8_t		m_bFame;						// 명성
	int16_t		m_sHp;							// HP
	int16_t		m_sMp;							// MP
	int16_t		m_sSp;							// SP
	uint8_t		m_bStr;							// 힘
	uint8_t		m_bSta;							// 생명력
	uint8_t		m_bDex;							// 공격, 회피율
	uint8_t		m_bIntel;						// 지혜(?), 캐릭터 마법력 결정
	uint8_t		m_bCha;							// 마법 성공률, 물건 가격 결정(?)
	uint8_t		m_bAuthority;					// 유저 권한
	uint8_t		m_bPoints;						// 보너스 포인트
	int32_t		m_iGold;						// 캐릭이 지닌 돈(21억)
	int16_t		m_sBind;						// Saved Bind Point
	int32_t		m_iBank;						// 창고의 돈(21억)

	uint8_t		m_bstrSkill[9];					// 직업별 스킬
	_ITEM_DATA	m_sItemArray[HAVE_MAX + SLOT_MAX];	// 42*8 bytes
	_WAREHOUSE_ITEM_DATA m_sWarehouseArray[WAREHOUSE_MAX];	// 창고 아이템	192*8 bytes

	uint8_t		m_bLogout;						// 로그아웃 플래그
	uint8_t		m_bWarehouse;					// 창고 거래 했었나?
	uint32_t	m_dwTime;						// 플레이 타임...

	uint8_t		m_byPremiumType;
	int16_t		m_sPremiumTime;
	int32_t		m_iMannerPoint;

	int16_t		m_sQuestCount;
	_USER_QUEST	m_quests[MAX_QUEST];
};

constexpr int ALLOCATED_USER_DATA_BLOCK = 8000;
static_assert(sizeof(_USER_DATA) <= ALLOCATED_USER_DATA_BLOCK);

#endif // SERVER_SHAREDSERVER__USER_DATA_H

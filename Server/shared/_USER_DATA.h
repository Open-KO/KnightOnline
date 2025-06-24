#pragma once

#include "globals.h"

struct _ITEM_DATA
{
	int		nNum;		// item 번호
	short	sDuration;	// item 내구력
	short	sCount;		// item 갯수 or item 축복 속성에 해당 값
	int64_t	nSerialNum;	// item serial code
};

struct _USER_DATA
{
	char	m_id[MAX_ID_SIZE + 1];			// 유저 ID
	char	m_Accountid[MAX_ID_SIZE + 1];	// 계정 ID

	BYTE	m_bZone;						// 현재 Zone
	float	m_curx;							// 현재 X 좌표
	float	m_curz;							// 현재 Z 좌표
	float	m_cury;							// 현재 Y 좌표

	BYTE	m_bNation;						// 소속국가
	BYTE	m_bRace;						// 종족
	short	m_sClass;						// 직업
	BYTE	m_bHairColor;					// 머리색깔
	BYTE	m_bRank;						// 작위
	BYTE	m_bTitle;						// 지위
	BYTE	m_bLevel;						// 레벨
	int		m_iExp;							// 경험치
	int		m_iLoyalty;						// 로열티
	BYTE	m_bFace;						// 얼굴모양
	BYTE	m_bCity;						// 소속도시
	short	m_bKnights;						// 소속 기사단
	BYTE	m_bFame;						// 명성
	short	m_sHp;							// HP
	short	m_sMp;							// MP
	short	m_sSp;							// SP
	BYTE	m_bStr;							// 힘
	BYTE	m_bSta;							// 생명력
	BYTE	m_bDex;							// 공격, 회피율
	BYTE	m_bIntel;						// 지혜(?), 캐릭터 마법력 결정
	BYTE	m_bCha;							// 마법 성공률, 물건 가격 결정(?)
	BYTE	m_bAuthority;					// 유저 권한
	BYTE	m_bPoints;						// 보너스 포인트
	int		m_iGold;						// 캐릭이 지닌 돈(21억)
	short	m_sBind;						// Saved Bind Point
	int		m_iBank;						// 창고의 돈(21억)

	BYTE    m_bstrSkill[9];					// 직업별 스킬
	_ITEM_DATA m_sItemArray[HAVE_MAX + SLOT_MAX];	// 42*8 bytes
	_ITEM_DATA m_sWarehouseArray[WAREHOUSE_MAX];	// 창고 아이템	192*8 bytes

	BYTE	m_bLogout;						// 로그아웃 플래그
	BYTE	m_bWarehouse;					// 창고 거래 했었나?
	DWORD	m_dwTime;						// 플레이 타임...
};

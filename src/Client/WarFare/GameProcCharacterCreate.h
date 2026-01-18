// GameProcCharacterCreate.h: interface for the CGameProcCharacterCreate class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GAMEPROCCHARACTERCREATE_H__DC02405A_668C_4A6F_A2A5_A050431CF900__INCLUDED_)
#define AFX_GAMEPROCCHARACTERCREATE_H__DC02405A_668C_4A6F_A2A5_A050431CF900__INCLUDED_

#pragma once

#include "GameProcedure.h"

enum e_ChrValue : uint8_t
{
	CVAL_STR   = 0,
	CVAL_STA   = 1,
	CVAL_DEX   = 2,
	CVAL_INT   = 3,
	CVAL_CHA   = 4,
	CVAL_BONUS = 5,
	CVAL_NUM   = 6
};

enum e_ErrorCharacterCreate : uint8_t
{
	ERROR_CHARACTER_CREATE_SUCCESS                         = 0,
	ERROR_CHARACTER_CREATE_NO_MORE_CHARACTER               = 1,
	ERROR_CHARACTER_CREATE_INVALID_NATION_AND_INVALID_RACE = 2,
	ERROR_CHARACTER_CREATE_OVERLAPPED_ID                   = 3,
	ERROR_CHARACTER_CREATE_DB_CREATE                       = 4,
	ERROR_CHARACTER_CREATE_INVALID_NAME_HAS_SPECIAL_LETTER = 5,
	ERROR_CHARACTER_CREATE_INVALID_NAME,
	ERROR_CHARACTER_CREATE_INVALID_RACE,
	ERROR_CHARACTER_CREATE_NOT_SUPPORTED_RACE,
	ERROR_CHARACTER_CREATE_INVALID_CLASS,
	ERROR_CHARACTER_CREATE_REMAIN_BONUS_POINT,
	ERROR_CHARACTER_CREATE_INVALID_STAT_POINT
};

struct __TABLE_NEW_CHR
{
	uint32_t dwID      = 0;  // NPC 고유 ID
	std::string szName = {}; // 종족이름..
	int iStr           = 0;
	int iSta           = 0;
	int iDex           = 0;
	int iInt           = 0;
	int iMAP           = 0; // 마법 공격력 Magic Attack Point
	int iBonus         = 0;
	uint32_t dwIDK[12] = {};
};

class CGameProcCharacterCreate : public CGameProcedure
{
public:
	class CUICharacterCreate* m_pUICharacterCreate;
	RECT m_rcChr;
	CN3TableBase<__TABLE_NEW_CHR> m_Tbl_InitValue; // 사운드 소스 정보 테이블..

												   //	int						m_InitValue[TRIBE_NUM][CVAL_NUM];

protected:
	bool ProcessPacket(Packet& pkt) override;

public:
	void ReportErrorCharacterCreate(e_ErrorCharacterCreate eErr);
	bool MsgSendCharacterCreate();
	void RenderChr();
	void SetChr();
	void SetStats();

	void Release() override;
	void Init() override;
	void Tick() override;
	void Render() override;

	CGameProcCharacterCreate();
	~CGameProcCharacterCreate() override;
};

#endif // !defined(AFX_GAMEPROCCHARACTERCREATE_H__DC02405A_668C_4A6F_A2A5_A050431CF900__INCLUDED_)

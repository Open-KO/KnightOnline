// N3UIArea.h: interface for the CN3UIArea class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3UIAREA_H__895A2972_7C58_4264_92AA_B740D40B0C22__INCLUDED_)
#define AFX_N3UIAREA_H__895A2972_7C58_4264_92AA_B740D40B0C22__INCLUDED_

#pragma once

#include "N3UIBase.h"

// NOLINTNEXTLINE(performance-enum-size): used by the file format (albeit indirectly), must be this size
enum eUI_AREA_TYPE : int32_t
{
	UI_AREA_TYPE_NONE = 0,
	UI_AREA_TYPE_SLOT,
	UI_AREA_TYPE_INV,
	UI_AREA_TYPE_TRADE_NPC,
	UI_AREA_TYPE_PER_TRADE_MY,
	UI_AREA_TYPE_PER_TRADE_OTHER,
	UI_AREA_TYPE_DROP_ITEM,
	UI_AREA_TYPE_SKILL_TREE,
	UI_AREA_TYPE_SKILL_HOTKEY,
	UI_AREA_TYPE_REPAIR_INV,
	UI_AREA_TYPE_REPAIR_NPC,
	UI_AREA_TYPE_TRADE_MY,
	UI_AREA_TYPE_PER_TRADE_INV,
};

class CN3UIArea : public CN3UIBase
{
public:
	CN3UIArea();
	~CN3UIArea() override;

public:
	eUI_AREA_TYPE m_eAreaType;

public:
	void Release() override;
	bool Load(File& file) override;
	void SetRegion(const RECT& Rect) override;

	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld) override;
#ifndef _REPENT
#ifdef _N3GAME
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
#endif
#endif

#ifdef _N3TOOL
	// 툴에서 사용하기 위한 함수
	CN3UIArea& operator=(const CN3UIArea& other);
	bool Save(File& file) override;
#endif
};

#endif // !defined(AFX_N3UIAREA_H__895A2972_7C58_4264_92AA_B740D40B0C22__INCLUDED_)


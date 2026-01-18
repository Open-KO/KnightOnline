// N3UIWndBase.h: interface for the CN3UIWndBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3UIWNDBASE_H__A30E8AD0_2EB8_4F27_8E0D_3E0979560761__INCLUDED_)
#define AFX_N3UIWNDBASE_H__A30E8AD0_2EB8_4F27_8E0D_3E0979560761__INCLUDED_

#pragma once

#include <N3Base/N3UIArea.h>
#include "N3UIIcon.h"
#include "GameProcedure.h"
#include "GameDef.h"
#include "IconItemSkill.h"

#include <string>

//////////////////////////////////////////////////////////////////////

// Wnd Info..
enum e_UIWND : uint8_t
{
	UIWND_INVENTORY = 0,  // Inventory Wnd..
	UIWND_TRANSACTION,    // Transaction Wnd..
	UIWND_DROPITEM,       // Drop Item Wnd..
	UIWND_PER_TRADE,      // 개인간 거래..
	UIWND_SKILL_TREE,     // Skill Icon Info Wnd..
	UIWND_HOTKEY,         // Hot Key Wnd..
	UIWND_PER_TRADE_EDIT, // Per Trade Edit Wnd..
	UIWND_WARE_HOUSE,     // 보관함..
	UIWND_UNKNOWN,        // Wnd Unknown..
};

// District Info..
enum e_UIWND_DISTRICT : uint8_t
{
	UIWND_DISTRICT_INVENTORY_SLOT = 0, // Slot district of Inventory Wnd..
	UIWND_DISTRICT_INVENTORY_INV,      // Inv district of Inventory Wnd..
	UIWND_DISTRICT_TRADE_NPC,          // Transaction district of Transaction Wnd of Npc..
	UIWND_DISTRICT_PER_TRADE_MY,       // My Transaction district of Per Transaction Wnd..
	UIWND_DISTRICT_PER_TRADE_OTHER,    // Other Transaction district of Per Transaction Wnd..
	UIWND_DISTRICT_DROPITEM,           // Dropitem district of Drop item wnd..
	UIWND_DISTRICT_SKILL_TREE,         // Skillicon district of Skill icon wnd..
	UIWND_DISTRICT_SKILL_HOTKEY,       // Skillicon district of Hotkey icon wnd..
	UIWND_DISTRICT_TRADE_MY,           // Npc 와의 거래에서 내 영역..
	UIWND_DISTRICT_PER_TRADE_INV,      // Inv District of Per Trade Wnd ..
	UIWND_DISTRICT_UNKNOWN,            // District Unknown..
};

enum e_UIIconState : uint8_t
{
	UIICON_SELECTED = 0,               // Icon Selected..
	UIICON_NOT_SELECTED_BUT_HIGHLIGHT, // Icon Not Selected But Highlight..
};

// Total Wnd Info..
struct __UIWndIconInfo
{
	e_UIWND UIWnd;
	e_UIWND_DISTRICT UIWndDistrict;
	int iOrder;
};

// enum Icon Type Info..//
enum e_UIIconType : uint8_t
{
	UIICON_TYPE_ITEM = 0, // Icon type item..
	UIICON_TYPE_SKILL,    // Icon type skill..
};

// Select Icon Info..
struct __InfoSelectedIcon
{
	__UIWndIconInfo UIWndSelect  = {};
	__IconItemSkill* pItemSelect = nullptr;
};

// Recovery Job Info..
struct __RecoveryJobInfo
{
	__IconItemSkill* pItemSource     = nullptr;
	__UIWndIconInfo UIWndSourceStart = {};
	__UIWndIconInfo UIWndSourceEnd   = {};
	__IconItemSkill* pItemTarget     = nullptr;
	__UIWndIconInfo UIWndTargetStart = {};
	__UIWndIconInfo UIWndTargetEnd   = {};
	int m_iPage                      = 0;
};

struct __SkillSelectInfo
{
	e_UIWND UIWnd                   = UIWND_UNKNOWN;
	int iOrder                      = -1;
	__IconItemSkill* pSkillDoneInfo = nullptr;
};

constexpr int UIITEM_TYPE_ONLYONE         = 0;
constexpr int UIITEM_TYPE_COUNTABLE       = 1;
constexpr int UITEIM_TYPE_GOLD            = 2;
constexpr int UIITEM_TYPE_COUNTABLE_SMALL = 3;
constexpr int UIITEM_TYPE_SOMOONE         = 4;

constexpr int UIITEM_TYPE_USABLE_ID_MIN   = 450000;

constexpr int UIITEM_COUNT_MANY           = 9999;
constexpr int UIITEM_COUNT_FEW            = 500;

//////////////////////////////////////////////////////////////////////

class CUIImageTooltipDlg;
class CCountableItemEditDlg;

// Class ^^
class CN3UIWndBase : public CN3UIBase // 가상 함수로 자식의 Area 갯수를 파악할 수 있는 함수가 있어야 하지 않을 까???
									  // int GetChildCountByAreaType(eAreatype .. ) ^^
{
	void PlayItemEtcSound();
	void PlayItemWeaponSound();
	void PlayItemArmorSound();

public:
	static __InfoSelectedIcon s_sSelectedIconInfo;
	static __RecoveryJobInfo s_sRecoveryJobInfo;
	static __SkillSelectInfo s_sSkillSelectInfo;
	static CN3UIImage* s_pSelectionImage;
	static CCountableItemEditDlg* s_pCountableItemEdit;

protected:
	e_UIWND m_eUIWnd;

	static int s_iRefCount; // 참조 카운트...
	static CN3SndObj* s_pSnd_Item_Etc;
	static CN3SndObj* s_pSnd_Item_Weapon;
	static CN3SndObj* s_pSnd_Item_Armor;
	static CN3SndObj* s_pSnd_Gold;
	static CN3SndObj* s_pSnd_Repair;

protected:
	virtual void InitIconWnd(e_UIWND eWnd);
	virtual void InitIconUpdate() = 0;

public:
	CN3UIWndBase();
	~CN3UIWndBase() override;

	e_UIWND GetUIWnd()
	{
		return m_eUIWnd;
	}

	virtual CN3UIArea* GetChildAreaByiOrder(eUI_AREA_TYPE eUAT, int iOrder);
	virtual CN3UIString* GetChildStringByiOrder(int iOrder);

	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld) override;
	virtual void AllHighLightIconFree();

	virtual __IconItemSkill* GetHighlightIconItem(CN3UIIcon* pUIIcon) = 0;

	virtual void IconRestore()
	{
	}

	virtual bool CheckIconDropFromOtherWnd(__IconItemSkill* /*spItem*/)
	{
		return false;
	}

	virtual bool ReceiveIconDrop(__IconItemSkill* /*spItem*/, POINT /*ptCur*/)
	{
		return false;
	}

	virtual void CancelIconDrop(__IconItemSkill* /*spItem*/)
	{
	}

	virtual void AcceptIconDrop(__IconItemSkill* /*spItem*/)
	{
	}

	virtual void PlayItemSound(__TABLE_ITEM_BASIC* pBasic);
	virtual void PlayGoldSound();
	virtual void PlayRepairSound();
};

#endif // !defined(AFX_N3UIWNDBASE_H__A30E8AD0_2EB8_4F27_8E0D_3E0979560761__INCLUDED_)

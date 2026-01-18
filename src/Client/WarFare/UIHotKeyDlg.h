// UIHotKeyDlg.h: interface for the CUIHotKeyDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIHOTKEYDLG_H__9B85201C_0294_4023_8658_923A6A2174BF__INCLUDED_)
#define AFX_UIHOTKEYDLG_H__9B85201C_0294_4023_8658_923A6A2174BF__INCLUDED_

#pragma once

#include "GameDef.h"
#include "N3UIWndBase.h"

class CUIHotKeyDlg : public CN3UIWndBase
{
public:
	POINT m_ptOffset;
	int m_iCurPage;
	__IconItemSkill* m_pMyHotkey[MAX_SKILL_HOTKEY_PAGE][MAX_SKILL_IN_HOTKEY];
	CN3UIString* m_pTooltipStr[MAX_SKILL_IN_HOTKEY];
	CN3UIString* m_pCountStr[MAX_SKILL_IN_HOTKEY];

	int m_iSelectIndex;
	int m_iSelectPage;

protected:
	RECT GetSampleRect();

public:
	bool OnKeyPress(int iKey) override;
	void RenderSelectIcon(CN3UIIcon* pUIIcon);
	bool EffectTriggerByMouse();
	CUIHotKeyDlg();
	~CUIHotKeyDlg() override;
	void Release() override;
	void ReleaseItem();

	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	void Render() override;
	void Open();
	void Close();

	void InitIconWnd(e_UIWND eWnd) override;
	void InitIconUpdate() override;

	__IconItemSkill* GetHighlightIconItem(CN3UIIcon* pUIIcon) override;

	void SetHotKeyPage(int iPageNum);

	void PageUp();
	void PageDown();

	bool CalcMoveOffset();
	int GetAreaiOrder();

	void DoOperate(__IconItemSkill* pSkill);

	void EffectTriggerByHotKey(int iIndex);

	void CloseIconRegistry();
	void ClassChangeHotkeyFlush();

	CN3UIString* GetTooltipStrControl(int iIndex);
	CN3UIString* GetCountStrControl(int iIndex);
	void DisplayTooltipStr(__IconItemSkill* pSkill);
	void DisableTooltipDisplay();
	void DisplayCountStr(__IconItemSkill* pSkill);
	void DisableCountStrDisplay();
	int GetTooltipCurPageIndex(__IconItemSkill* pSkill);
	int GetCountCurPageIndex(__IconItemSkill* pSkill);

	//	bool				ReceiveSelectedSkill();
	bool IsSelectedSkillInRealIconArea();
	void SetReceiveSelectedSkill(int iIndex);
	bool SetReceiveSelectedItem(int iIndex);
	bool GetEmptySlotIndex(int& iIndex);

	void AllFactorClear();
	void UpdateDisableCheck();

	bool ReceiveIconDrop(__IconItemSkill* spItem, POINT ptCur) override;
	void RenderCooldown(const __IconItemSkill* pSkill, float fCooldown);

	void SetHotKeyTooltip(__IconItemSkill* spSkill);
};

#endif // !defined(AFX_UIHOTKEYDLG_H__9B85201C_0294_4023_8658_923A6A2174BF__INCLUDED_)

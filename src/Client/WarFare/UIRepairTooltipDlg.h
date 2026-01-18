// UIRepairTooltipDlg.h: interface for the UIRepairTooltipDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIREPAIRTOOLTIPDLG_H__BC9FC233_F483_41D2_8D9B_B3656A00A637__INCLUDED_)
#define AFX_UIREPAIRTOOLTIPDLG_H__BC9FC233_F483_41D2_8D9B_B3656A00A637__INCLUDED_

#pragma once

#include <N3Base/N3UIBase.h>

#include "N3UIWndBase.h" // __IconItemSkill

class CUIRepairTooltipDlg : public CN3UIBase
{
	static constexpr int MAX_REPAIR_TOOLTIP_COUNT = 4;

	CN3UIString* m_pStr[MAX_REPAIR_TOOLTIP_COUNT];  // 스트륑.. ^^
	D3DCOLOR m_pStrColor[MAX_REPAIR_TOOLTIP_COUNT]; // 스트륑 색깔.. ^^
	int m_iPosXBack, m_iPosYBack;
	__IconItemSkill* m_spItemBack;

public:
	bool m_bBRender;
	int m_iBxpos;
	int m_iBypos;
	__IconItemSkill* m_pBspItem;
	int m_iBRequiredGold;
	bool m_bBHaveEnough;

	int m_iBRequiredGoldBackup;

protected:
	void SetTooltipTextColor(bool bHaveEnough);

public:
	CUIRepairTooltipDlg();
	~CUIRepairTooltipDlg() override;
	void Release() override;
	void InitPos();
	void BackupStrColor();

	void DisplayTooltipsEnable(int xpos, int ypos, __IconItemSkill* spItem, int iRequiredGold, bool bHaveEnough);
	void DisplayTooltipsDisable();

	void Render() override;
};

#endif // !defined(AFX_UIREPAIRTOOLTIPDLG_H__BC9FC233_F483_41D2_8D9B_B3656A00A637__INCLUDED_)

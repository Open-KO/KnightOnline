// UIImageTooltipDlg.h: interface for the CUIImageTooltipDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIIMAGETOOLTIPDLG_H__CC12484D_5DC0_4F7A_ABF7_0506B2F36292__INCLUDED_)
#define AFX_UIIMAGETOOLTIPDLG_H__CC12484D_5DC0_4F7A_ABF7_0506B2F36292__INCLUDED_

#pragma once

#include "N3UIWndBase.h" // __IconItemSkill

class CUIImageTooltipDlg : public CN3UIBase
{
public:
	static constexpr int MAX_TOOLTIP_COUNT       = 30;
	static constexpr int MIN_WORDS_TO_SPLIT_DESC = 5;

protected:
	const D3DCOLOR m_CYellow;               // 레어...
	const D3DCOLOR m_CBlue;                 // 매직...
	const D3DCOLOR m_CGold;                 // 유니크...
	const D3DCOLOR m_CIvory;                // 업그레이드..
	const D3DCOLOR m_CGreen;                // 옵션...	저항력, 능력치 상승, 마법속성..
	const D3DCOLOR m_CWhite;                // 일반...
	const D3DCOLOR m_CRed;                  // 뭔가 제한에 걸릴때..
public:
	CN3UIString* m_pStr[MAX_TOOLTIP_COUNT]; // 스트륑.. ^^
	std::string m_pstdstr[MAX_TOOLTIP_COUNT];
	CN3UIImage* m_pImg;

	int m_iPosXBack;
	int m_iPosYBack;
	int m_iTooltipNum;

	uint32_t m_dwID_Basic;
	uint32_t m_dwID_Ext;
	int m_iCount;
	int m_iDurability;

protected:
	bool SetTooltipTextColor(int iMyValue, int iTooltipValue);
	bool SetTooltipTextColor(e_Race eMyValue, e_Race eTooltipValue);
	bool SetTooltipTextColor(e_Class eMyValue, e_Class eTooltipValue);
	int CalcTooltipStringNumAndWrite(__IconItemSkill* spItem, bool bPrice, bool bBuy);
	void CalcTooltipStringNumAndWriteImpl(__IconItemSkill* spItem, bool bPrice, int& iIndex);
	void SetPosSomething(int xpos, int ypos);

public:
	CUIImageTooltipDlg();
	~CUIImageTooltipDlg() override;
	void Release() override;
	void InitPos();

	void DisplayTooltipsEnable(int xpos, int ypos, __IconItemSkill* spItem, bool bPrice = false, bool bBuy = true);
	void DisplayTooltipsDisable();

protected:
	bool IsItemChanged(const __IconItemSkill* spItem) const;
	void UpdateItem(const __IconItemSkill* spItem);
	void ResetItem();
};

#endif // !defined(AFX_UIIMAGETOOLTIPDLG_H__CC12484D_5DC0_4F7A_ABF7_0506B2F36292__INCLUDED_)

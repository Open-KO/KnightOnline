// UIImageTooltipDlg.h: interface for the CUIImageTooltipDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIIMAGETOOLTIPDLG_H__CC12484D_5DC0_4F7A_ABF7_0506B2F36292__INCLUDED_)
#define AFX_UIIMAGETOOLTIPDLG_H__CC12484D_5DC0_4F7A_ABF7_0506B2F36292__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3UIBase.h>

#include "N3UIWndBase.h" // __IconItemSkill

//////////////////////////////////////////////////////////////////////

#define MAX_TOOLTIP_COUNT 30

class CUIImageTooltipDlg : public CN3UIBase
{
	const D3DCOLOR		m_CYellow;	// 레어...
	const D3DCOLOR		m_CBlue;	// 매직...	
	const D3DCOLOR		m_CGold;	// 유니크...
	const D3DCOLOR		m_CIvory;	// 업그레이드..
	const D3DCOLOR		m_CGreen;	// 옵션...	저항력, 능력치 상승, 마법속성..
	const D3DCOLOR		m_CWhite;	// 일반...
	const D3DCOLOR		m_CRed;		// 뭔가 제한에 걸릴때..

public:
	CN3UIString*		m_pStr[MAX_TOOLTIP_COUNT];			// 스트륑.. ^^
	std::string			m_pstdstr[MAX_TOOLTIP_COUNT];
	CN3UIImage*			m_pImg;

	int					m_iPosXBack, m_iPosYBack;
	__IconItemSkill*	m_spItemBack;

protected:
	bool				SetTooltipTextColor(int iMyValue, int iTooltipValue);
	bool				SetTooltipTextColor(e_Race eMyValue, e_Race eTooltipValue);
	bool				SetTooltipTextColor(e_Class eMyValue, e_Class eTooltipValue);
	int					CalcTooltipStringNumAndWrite(__IconItemSkill* spItem, bool bPrice, bool bBuy);
	void				SetPosSomething(int xpos, int ypos, int iNum);

public:
	CUIImageTooltipDlg();
	virtual ~CUIImageTooltipDlg();
	void	Release();
	void	InitPos();

	void	DisplayTooltipsEnable(int xpos, int ypos, __IconItemSkill* spItem, bool bPrice = false, bool bBuy = true);
	void	DisplayTooltipsDisable();
};

#endif // !defined(AFX_UIIMAGETOOLTIPDLG_H__CC12484D_5DC0_4F7A_ABF7_0506B2F36292__INCLUDED_)

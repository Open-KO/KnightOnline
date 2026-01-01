#if !defined(AFX_MACSLIDERCTRL_H__021A6935_B2FE_11D3_90FC_0020AFBC499D__INCLUDED_)
#define AFX_MACSLIDERCTRL_H__021A6935_B2FE_11D3_90FC_0020AFBC499D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MacSliderCtrl.h : header file
//
//	CMacSliderCtrl class, version 1.0
//
//	Copyright (c) 1999 Paul M. Meidinger (pmmeidinger@yahoo.com)
//
// Feel free to modifiy and/or distribute this file, but
// do not remove this header.
//
// I would appreciate a notification of any bugs discovered or 
// improvements that could be made.
//
// This file is provided "as is" with no expressed or implied warranty.
//
//	History:
//		PMM	12/21/1999		Initial implementation.		

/////////////////////////////////////////////////////////////////////////////
// CMacSliderCtrl window
const float FRM_PRECISION = 0.2f;
#include <N3Base/N3AnimControl.h> // tigger

class CMacSliderCtrl : public CSliderCtrl
{
protected:
	__AnimData m_AnimData;
public:
	void SetAnimationData(const __AnimData& Data);
	void CalcRectToFill(const RECT& rcWhole, RECT& rc, float fValue);
	void CalcRectToFill(const RECT& rcWhole, RECT& rc, float fValueStart, float fValueEnd);
	float GetCurrentFrame();
	void SetCurrentFrame(float fFrm);





// Construction
public:
	CMacSliderCtrl();

// Attributes
public:

// Operations
public:
	void SetChannelColor(COLORREF crColor);
	COLORREF GetChannelColor();
	void SetThumbColor(COLORREF crColor);
	COLORREF GetThumbColor();
	void SetSelectionColor(COLORREF crColor);
	COLORREF GetSelectionColor();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMacSliderCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMacSliderCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMacSliderCtrl)
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
private:
	COLORREF LightenColor(const COLORREF crColor, BYTE byIncreaseVal);
	COLORREF DarkenColor(const COLORREF crColor, BYTE byReduceVal);

	void DrawHorizontalThumb(CDC* pDC, const CRect &rect);
	void DrawVerticalThumb(CDC* pDC, const CRect &rect);
	void DrawChannel(CDC* pDC, const CRect &rect, BOOL bHoriz);
	void DeletePens();
	void CreatePens();
	CPen m_penThumb;
	CPen m_penThumbLight;
	CPen m_penThumbLighter;
	CPen m_penThumbDark;
	CPen m_penThumbDarker;
	CPen m_penDkShadow;
	CPen m_penShadow;
	CPen m_penHilight;
	void GetColors();
	COLORREF m_crSelection;
	COLORREF m_crChannel;
	COLORREF m_crThumb;
	COLORREF m_crThumbLight;
	COLORREF m_crThumbLighter;
	COLORREF m_crThumbDark;
	COLORREF m_crThumbDarker;
	COLORREF m_crDkShadow;
	COLORREF m_crShadow;
	COLORREF m_crHilight;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MACSLIDERCTRL_H__021A6935_B2FE_11D3_90FC_0020AFBC499D__INCLUDED_)

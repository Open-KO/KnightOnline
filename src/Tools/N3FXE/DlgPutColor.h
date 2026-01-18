#if !defined(AFX_DLGPUTCOLOR_H__BE9E4B7C_5FA8_40AD_BDAD_C7B51EA9C897__INCLUDED_)
#define AFX_DLGPUTCOLOR_H__BE9E4B7C_5FA8_40AD_BDAD_C7B51EA9C897__INCLUDED_

#pragma once

// DlgPutColor.h : header file
//

#include <N3Base/N3FXDef.h>

/////////////////////////////////////////////////////////////////////////////
// CDlgPutColor dialog

class CDlgPutColor : public CDialog
{
	// Construction
public:
	CDlgPutColor(CWnd* pParent = nullptr); // standard constructor

	DWORD m_Color[NUM_KEY_COLOR];
	DWORD m_Opacity[NUM_KEY_COLOR];

	bool m_bColorKey[NUM_KEY_COLOR];
	bool m_bAlphaKey[NUM_KEY_COLOR];

	// Dialog Data
	//{{AFX_DATA(CDlgPutColor)
	enum
	{
		IDD = IDD_DLG_PUTCOLOR
	};
	CStatic m_stcOpacity;
	CStatic m_stcColor;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgPutColor)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	void GradateColor(int start, int end);
	void GradateAlpha(int start, int end);

	// Generated message map functions
	//{{AFX_MSG(CDlgPutColor)
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGPUTCOLOR_H__BE9E4B7C_5FA8_40AD_BDAD_C7B51EA9C897__INCLUDED_)

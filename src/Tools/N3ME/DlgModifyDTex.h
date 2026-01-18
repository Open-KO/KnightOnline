#if !defined(AFX_DLGMODIFYDTEX_H__2C4EB5E6_3321_4B9C_A818_DDE640421C9B__INCLUDED_)
#define AFX_DLGMODIFYDTEX_H__2C4EB5E6_3321_4B9C_A818_DDE640421C9B__INCLUDED_

#pragma once

// DlgModifyDTex.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgModifyDTex dialog

class CDlgModifyDTex : public CDialog
{
	// Construction
public:
	int m_State;
	CPoint m_Pos;
	CDlgModifyDTex(CWnd* pParent = nullptr); // standard constructor

											 // Dialog Data
	//{{AFX_DATA(CDlgModifyDTex)
	enum
	{
		IDD = IDD_MODIFY_DTEX
	};
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgModifyDTex)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgModifyDTex)
	afx_msg void OnModifyDtexMirror();
	virtual BOOL OnInitDialog();
	afx_msg void OnModifyDtexCw90d();
	afx_msg void OnModifyDtexCw270d();
	afx_msg void OnModifyDtexCw180d();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGMODIFYDTEX_H__2C4EB5E6_3321_4B9C_A818_DDE640421C9B__INCLUDED_)

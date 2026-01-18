#if !defined(AFX_DLGADDDTEX_H__703821EA_F9D1_4949_BE8E_97401A0B8236__INCLUDED_)
#define AFX_DLGADDDTEX_H__703821EA_F9D1_4949_BE8E_97401A0B8236__INCLUDED_

#pragma once

// DlgAddDTex.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgAddDTex dialog

class CDlgAddDTex : public CDialog
{
	// Construction
public:
	CDlgAddDTex(CWnd* pParent = nullptr); // standard constructor

	CString m_TexName;

	// Dialog Data
	//{{AFX_DATA(CDlgAddDTex)
	enum
	{
		IDD = IDD_ADDDTEX
	};
	CListBox m_TexList;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgAddDTex)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgAddDTex)
	afx_msg void OnSelchangeAdddtexlist();
	afx_msg void OnDblclkAdddtexlist();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGADDDTEX_H__703821EA_F9D1_4949_BE8E_97401A0B8236__INCLUDED_)

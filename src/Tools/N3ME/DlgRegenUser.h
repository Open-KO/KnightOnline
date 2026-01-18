#if !defined(AFX_DLGREGENUSER_H__0975EF8F_1946_499B_BC7C_6237A0554796__INCLUDED_)
#define AFX_DLGREGENUSER_H__0975EF8F_1946_499B_BC7C_6237A0554796__INCLUDED_

#pragma once

// DlgRegenUser.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgRegenUser dialog

#include "RegenUser.h"

class CDlgRegenUser : public CDialog
{
	// Construction
public:
	CDlgRegenUser(CWnd* pParent = nullptr); // standard constructor

											// Dialog Data
	//{{AFX_DATA(CDlgRegenUser)
	enum
	{
		IDD = IDD_MAKE_REGEN_USER
	};
	CListBox m_LBRegion;
	CString m_PathName;
	//}}AFX_DATA

	CRegenUser* m_pRefRegenUser;

	void PutRegion(VERTEXRECT* pVR);
	void ClearList();

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgRegenUser)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgRegenUser)
	afx_msg void OnBtnLoadPathset();
	afx_msg void OnBtnPathDelete();
	afx_msg void OnBtnSavePathset();
	afx_msg void OnSelchangeListRegenuserregion();
	afx_msg void OnDblclkListRegenuserregion();
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnSaveAs();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGREGENUSER_H__0975EF8F_1946_499B_BC7C_6237A0554796__INCLUDED_)

#if !defined(AFX_DLGMAKEWALL_H__081595A7_E433_408C_85FA_EB69AC055ED0__INCLUDED_)
#define AFX_DLGMAKEWALL_H__081595A7_E433_408C_85FA_EB69AC055ED0__INCLUDED_

#pragma once

// DlgMakeWall.h : header file
//

class CWallMgr;
class CWall;

/////////////////////////////////////////////////////////////////////////////
// CDlgMakeWall dialog

class CDlgMakeWall : public CDialog
{
	// Construction
public:
	void ResetAll();
	CDlgMakeWall(CWnd* pParent = nullptr); // standard constructor

										   // Dialog Data
	//{{AFX_DATA(CDlgMakeWall)
	enum
	{
		IDD = IDD_MAKE_WALL
	};
	CListBox m_WallList;
	//}}AFX_DATA

	// Variables..
	CWallMgr* m_pRefWallMgr;
	CWall* m_pSelWall;

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgMakeWall)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgMakeWall)
	afx_msg void OnBtnDelete();
	afx_msg void OnBtnWallEdit();
	afx_msg void OnBtnWallRecord();
	afx_msg void OnSelchangeListWall();
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkListWall();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGMAKEWALL_H__081595A7_E433_408C_85FA_EB69AC055ED0__INCLUDED_)

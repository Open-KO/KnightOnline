#if !defined(AFX_DLGINPUTATTR_H__8399E1A8_C5C7_41F1_9F95_BFD410431261__INCLUDED_)
#define AFX_DLGINPUTATTR_H__8399E1A8_C5C7_41F1_9F95_BFD410431261__INCLUDED_

#pragma once

// DlgInputAttr.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgInputAttr dialog

class CDlgInputAttr : public CDialog
{
	// Construction
public:
	int m_SelectedGroupID;
	POINT m_Point;

	CDlgInputAttr(CWnd* pParent = nullptr); // standard constructor

											// Dialog Data
	//{{AFX_DATA(CDlgInputAttr)
	enum
	{
		IDD = IDD_INPUT_ATTR
	};
	CListBox m_LBGroup;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgInputAttr)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgInputAttr)
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkLbGroup();
	afx_msg void OnSelchangeLbGroup();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGINPUTATTR_H__8399E1A8_C5C7_41F1_9F95_BFD410431261__INCLUDED_)

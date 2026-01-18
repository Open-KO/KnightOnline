#if !defined(AFX_DLGINPUTGROUP_H__A52E3220_BEC2_45A0_8F89_2BF0B6982D26__INCLUDED_)
#define AFX_DLGINPUTGROUP_H__A52E3220_BEC2_45A0_8F89_2BF0B6982D26__INCLUDED_

#pragma once

// DlgInputGroup.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgInputGroup dialog

class CDlgInputGroup : public CDialog
{
	// Construction
public:
	CDlgInputGroup(CWnd* pParent = nullptr); // standard constructor

											 // Dialog Data
	//{{AFX_DATA(CDlgInputGroup)
	enum
	{
		IDD = IDD_INPUT_GROUP
	};
	CString m_GroupName;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgInputGroup)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgInputGroup)
	// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGINPUTGROUP_H__A52E3220_BEC2_45A0_8F89_2BF0B6982D26__INCLUDED_)

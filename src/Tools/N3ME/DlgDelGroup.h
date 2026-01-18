#if !defined(AFX_DLGDELGROUP_H__6104C116_7C65_46CF_BABC_2E6D5CE233CF__INCLUDED_)
#define AFX_DLGDELGROUP_H__6104C116_7C65_46CF_BABC_2E6D5CE233CF__INCLUDED_

#pragma once

// DlgDelGroup.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgDelGroup dialog

class CDlgDelGroup : public CDialog
{
	// Construction
public:
	int m_SelGroupID;
	CDlgDelGroup(CWnd* pParent = nullptr); // standard constructor

										   // Dialog Data
	//{{AFX_DATA(CDlgDelGroup)
	enum
	{
		IDD = IDD_DEL_GROUP
	};
	CListBox m_GroupList;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgDelGroup)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgDelGroup)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeListDelgroup();
	afx_msg void OnDblclkListDelgroup();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGDELGROUP_H__6104C116_7C65_46CF_BABC_2E6D5CE233CF__INCLUDED_)

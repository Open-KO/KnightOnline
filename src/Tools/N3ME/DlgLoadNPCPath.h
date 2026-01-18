#if !defined(AFX_DLGLOADNPCPATH_H__023D7039_A240_4525_BEFF_49C037E84C68__INCLUDED_)
#define AFX_DLGLOADNPCPATH_H__023D7039_A240_4525_BEFF_49C037E84C68__INCLUDED_

#pragma once

// DlgLoadNPCPath.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgLoadNPCPath dialog

class CDlgLoadNPCPath : public CDialog
{
	// Construction
public:
	CDlgLoadNPCPath(CWnd* pParent = nullptr); // standard constructor

	CString m_SelFileName;

	// Dialog Data
	//{{AFX_DATA(CDlgLoadNPCPath)
	enum
	{
		IDD = IDD_LOAD_NPCPATH
	};
	CListBox m_NPCPathFileList;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgLoadNPCPath)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgLoadNPCPath)
	afx_msg void OnSelchangeListNpcpath();
	afx_msg void OnDblclkListNpcpath();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGLOADNPCPATH_H__023D7039_A240_4525_BEFF_49C037E84C68__INCLUDED_)

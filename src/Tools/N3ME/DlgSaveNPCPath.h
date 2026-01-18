#if !defined(AFX_DLGSAVENPCPATH_H__CCC83D6F_FF16_43FF_9F50_4822CA953B64__INCLUDED_)
#define AFX_DLGSAVENPCPATH_H__CCC83D6F_FF16_43FF_9F50_4822CA953B64__INCLUDED_

#pragma once

// DlgSaveNPCPath.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgSaveNPCPath dialog

class CDlgSaveNPCPath : public CDialog
{
	// Construction
public:
	CDlgSaveNPCPath(CWnd* pParent = nullptr); // standard constructor

											  // Dialog Data
	//{{AFX_DATA(CDlgSaveNPCPath)
	enum
	{
		IDD = IDD_SAVE_NPCPATH
	};
	CListBox m_SavedFileList;
	CString m_NewFileName;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgSaveNPCPath)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgSaveNPCPath)
	afx_msg void OnSelchangeListNpcpathfilename();
	afx_msg void OnDblclkListNpcpathfilename();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSAVENPCPATH_H__CCC83D6F_FF16_43FF_9F50_4822CA953B64__INCLUDED_)

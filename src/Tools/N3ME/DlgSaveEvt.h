#if !defined(AFX_DLGSAVEEVT_H__6D9859E6_F6BF_4C64_9120_A0DA2D27E456__INCLUDED_)
#define AFX_DLGSAVEEVT_H__6D9859E6_F6BF_4C64_9120_A0DA2D27E456__INCLUDED_

#pragma once

// DlgSaveEvt.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgSaveEvt dialog

class CDlgSaveEvt : public CDialog
{
	// Construction
public:
	CDlgSaveEvt(CWnd* pParent = nullptr); // standard constructor

										  // Dialog Data
	//{{AFX_DATA(CDlgSaveEvt)
	enum
	{
		IDD = IDD_SAVE_EVENT
	};
	CListBox m_SavedFileList;
	CString m_FileName;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgSaveEvt)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgSaveEvt)
	afx_msg void OnSelchangeListSavedEvtFile();
	afx_msg void OnDblclkListSavedEvtFile();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSAVEEVT_H__6D9859E6_F6BF_4C64_9120_A0DA2D27E456__INCLUDED_)

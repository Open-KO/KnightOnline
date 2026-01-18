#if !defined(AFX_DLGLOADEVT_H__BBB80EB7_9FCA_47C7_8461_15349AD880A7__INCLUDED_)
#define AFX_DLGLOADEVT_H__BBB80EB7_9FCA_47C7_8461_15349AD880A7__INCLUDED_

#pragma once

// DlgLoadEvt.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgLoadEvt dialog

class CDlgLoadEvt : public CDialog
{
	// Construction
public:
	CDlgLoadEvt(CWnd* pParent = nullptr); // standard constructor

	CString m_SelFileName;
	// Dialog Data
	//{{AFX_DATA(CDlgLoadEvt)
	enum
	{
		IDD = IDD_LOAD_EVENT
	};
	CListBox m_EvtFileList;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgLoadEvt)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgLoadEvt)
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkListSavedEvtFile();
	afx_msg void OnSelchangeListSavedEvtFile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGLOADEVT_H__BBB80EB7_9FCA_47C7_8461_15349AD880A7__INCLUDED_)

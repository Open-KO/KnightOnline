#if !defined(AFX_DLGUNUSEDFILES_H__992E7C18_8A67_4AD3_A702_FC98AED8CF67__INCLUDED_)
#define AFX_DLGUNUSEDFILES_H__992E7C18_8A67_4AD3_A702_FC98AED8CF67__INCLUDED_

#pragma once

// DlgUnusedFiles.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgUnusedFiles dialog

class CDlgUnusedFiles : public CDialog
{
public:
	CStringArray m_FileNames;
	CStringArray m_InvalidFileNames;

public:
	void UpdateAll();
	CDlgUnusedFiles(CWnd* pParent = nullptr); // standard constructor

											  // Dialog Data
	//{{AFX_DATA(CDlgUnusedFiles)
	enum
	{
		IDD = IDD_UNUSED_FILES
	};
	CListBox m_ListInvalidObjects;
	CListBox m_ListFiles;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgUnusedFiles)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgUnusedFiles)
	afx_msg void OnDelete();
	afx_msg void OnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGUNUSEDFILES_H__992E7C18_8A67_4AD3_A702_FC98AED8CF67__INCLUDED_)

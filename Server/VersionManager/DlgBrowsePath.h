// CDlgBrowsePath.h : header file
//

#if !defined(AFX_CDlgBrowsePath_H__F0B620A7_9D4F_11D1_B10E_B8F603C10000__INCLUDED_)
#define AFX_CDlgBrowsePath_H__F0B620A7_9D4F_11D1_B10E_B8F603C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "Resource.h"
#include "ShellTree.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgBrowsePath dialog
//
// This source is part of CShellTree - Selom Ofori
// 
// Version: 1.02 (any previously unversioned copies are older/inferior
//
// This code is free for all to use. Mutatilate it as much as you want
// See MFCENUM sample from microsoft

class CDlgBrowsePath : public CDialog
{
// Construction
public:
	CString m_szPath;
	CString GetPath() { return m_szPath; }
	CDlgBrowsePath(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgBrowsePath)
	enum { IDD = IDD_BROWSE_PATH };
	CComboBox	m_CBPath;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgBrowsePath)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	CShellTree* m_TreeCtl;
	// Generated message map functions
	//{{AFX_MSG(CDlgBrowsePath)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnItemexpandingBrowseTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickBrowseTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeleteitemBrowseTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSynchFolders();
	afx_msg void OnSelchangedBrowseTree(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CDlgBrowsePath_H__F0B620A7_9D4F_11D1_B10E_B8F603C10000__INCLUDED_)

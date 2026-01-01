#if !defined(AFX_FOLDERSELECTORDLG_H__28851341_4EEA_11D3_9559_00C026EE03FC__INCLUDED_)
#define AFX_FOLDERSELECTORDLG_H__28851341_4EEA_11D3_9559_00C026EE03FC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgFolderSelect.h : header file
//

#include "../Common Control/DirTreeCtrl.h"
#include "Resource.h"
/////////////////////////////////////////////////////////////////////////////
// CDlgFolderSelect dialog

class CDlgFolderSelect : public CDialog
{
// Construction
public:
	CString GetFolderName();
	void SetInitFolderName(CString& FolderName);
	CDlgFolderSelect(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgFolderSelect)
	enum { IDD = IDD_FOLDER_SELECT };
	CDirTreeCtrl	m_DirTree;
	CComboBox		m_CBFolder;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgFolderSelect)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
private:
	CString m_FolderName;

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgFolderSelect)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangedBrowseTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTunnelTree();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSelchangeFolderList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FOLDERSELECTORDLG_H__28851341_4EEA_11D3_9559_00C026EE03FC__INCLUDED_)

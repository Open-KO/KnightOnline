#if !defined(AFX_DLGSAVENEWTILESET_H__6A49B1B4_7253_44B6_AEFC_A079DC336D97__INCLUDED_)
#define AFX_DLGSAVENEWTILESET_H__6A49B1B4_7253_44B6_AEFC_A079DC336D97__INCLUDED_

#pragma once

// DlgSaveNewTileSet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgSaveNewTileSet dialog

class CDlgSaveNewTileSet : public CDialog
{
	// Construction
public:
	CDlgSaveNewTileSet(CWnd* pParent = nullptr); // standard constructor

												 // Dialog Data
	//{{AFX_DATA(CDlgSaveNewTileSet)
	enum
	{
		IDD = IDD_SAVE_NEW_TILESET
	};
	CListBox m_ListTileSet;
	CString m_NewTileSetName;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgSaveNewTileSet)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgSaveNewTileSet)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSAVENEWTILESET_H__6A49B1B4_7253_44B6_AEFC_A079DC336D97__INCLUDED_)

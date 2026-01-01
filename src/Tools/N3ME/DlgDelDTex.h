#if !defined(AFX_DLGDELDTEX_H__790BA71D_B58D_4E71_8438_790CC327F643__INCLUDED_)
#define AFX_DLGDELDTEX_H__790BA71D_B58D_4E71_8438_790CC327F643__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgDelDTex.h : header file
//

class CDlgSetDTex;

/////////////////////////////////////////////////////////////////////////////
// CDlgDelDTex dialog

class CDlgDelDTex : public CDialog
{
// Construction
public:
	CString m_SelDTexName;
	CDlgDelDTex(CWnd* pParent = NULL);   // standard constructor

	CDlgSetDTex* m_pParent;

	void	SetFileList(CComboBox* pCB);

// Dialog Data
	//{{AFX_DATA(CDlgDelDTex)
	enum { IDD = IDD_DELDTEX };
	CListBox	m_TexList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgDelDTex)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgDelDTex)
	afx_msg void OnSelchangeListDeldtex();
	afx_msg void OnDblclkListDeldtex();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGDELDTEX_H__790BA71D_B58D_4E71_8438_790CC327F643__INCLUDED_)

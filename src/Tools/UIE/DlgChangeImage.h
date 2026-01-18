#if !defined(AFX_DLGCHANGEIMAGE_H__F9B2B454_50C6_48B0_9BC8_0CA624174D32__INCLUDED_)
#define AFX_DLGCHANGEIMAGE_H__F9B2B454_50C6_48B0_9BC8_0CA624174D32__INCLUDED_

#pragma once

// DlgChangeImage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgChangeImage dialog

class CDlgChangeImage : public CDialog
{
public:
	CString m_szFN_Old;
	CString m_szFN_New;
	// Construction
public:
	CDlgChangeImage(CWnd* pParent = nullptr); // standard constructor

											  // Dialog Data
	//{{AFX_DATA(CDlgChangeImage)
	enum
	{
		IDD = IDD_CHANGE_IMAGE_FILE_NAME
	};
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgChangeImage)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgChangeImage)
	afx_msg void OnBBrowseOldFile();
	afx_msg void OnBBrowseNewFile();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCHANGEIMAGE_H__F9B2B454_50C6_48B0_9BC8_0CA624174D32__INCLUDED_)

#if !defined(AFX_DLGNEWPART_H__687CF3CB_2C19_4718_AD8E_6B7287FB27F8__INCLUDED_)
#define AFX_DLGNEWPART_H__687CF3CB_2C19_4718_AD8E_6B7287FB27F8__INCLUDED_

#pragma once

// DlgNewPart.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgNewPart dialog

class CDlgNewPart : public CDialog
{
	// Construction
public:
	CDlgNewPart(CWnd* pParent = nullptr); // standard constructor

										  // Dialog Data
	//{{AFX_DATA(CDlgNewPart)
	enum
	{
		IDD = IDD_DLG_NEWPART
	};
	int m_iPartType;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgNewPart)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgNewPart)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGNEWPART_H__687CF3CB_2C19_4718_AD8E_6B7287FB27F8__INCLUDED_)

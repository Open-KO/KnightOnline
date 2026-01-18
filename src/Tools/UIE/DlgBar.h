#if !defined(AFX_DLGBAR_H__C665D39A_F8D8_44DA_80BA_1517EA83BB90__INCLUDED_)
#define AFX_DLGBAR_H__C665D39A_F8D8_44DA_80BA_1517EA83BB90__INCLUDED_

#pragma once

// DlgBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgBar dialog

class CDlgBar : public CDialog
{
	// Construction
public:
	CDlgBar(CWnd* pParent = nullptr); // standard constructor

									  // Dialog Data
	//{{AFX_DATA(CDlgBar)
	enum
	{
		IDD = IDR_MAINFRAME
	};
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgBar)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgBar)
	afx_msg void OnButtonBasepath();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGBAR_H__C665D39A_F8D8_44DA_80BA_1517EA83BB90__INCLUDED_)

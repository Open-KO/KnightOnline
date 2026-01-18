#if !defined(AFX_DLGANIMATE_H__583DE641_1A8A_4756_8227_9780B1FF7D86__INCLUDED_)
#define AFX_DLGANIMATE_H__583DE641_1A8A_4756_8227_9780B1FF7D86__INCLUDED_

#pragma once

// DlgAnimate.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgAnimate dialog

class CDlgAnimate : public CDialog
{
	// Construction
public:
	CDlgAnimate(CWnd* pParent = nullptr); // standard constructor

										  // Dialog Data
	//{{AFX_DATA(CDlgAnimate)
	enum
	{
		IDD = IDD_ANIMATE
	};
	int m_iCount;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgAnimate)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgAnimate)
	// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGANIMATE_H__583DE641_1A8A_4756_8227_9780B1FF7D86__INCLUDED_)

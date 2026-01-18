#if !defined(AFX_DLGPERCENT_H__6C79643F_FFAC_4AA1_9414_C93ACF3AC8BE__INCLUDED_)
#define AFX_DLGPERCENT_H__6C79643F_FFAC_4AA1_9414_C93ACF3AC8BE__INCLUDED_

#pragma once

// DlgPercent.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgPercent dialog

class CDlgPercent : public CDialog
{
	// Construction
public:
	CDlgPercent(CWnd* pParent = nullptr); // standard constructor

										  // Dialog Data
	//{{AFX_DATA(CDlgPercent)
	enum
	{
		IDD = IDD_DLG_PERCENT
	};
	CSliderCtrl m_Slider;
	float m_fPercent;
	int m_iRealValue;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgPercent)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgPercent)
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGPERCENT_H__6C79643F_FFAC_4AA1_9414_C93ACF3AC8BE__INCLUDED_)

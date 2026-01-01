#if !defined(AFX_DLGBAR_H__B4FC5A3B_1FBC_43AB_96E4_E7C8D837DF91__INCLUDED_)
#define AFX_DLGBAR_H__B4FC5A3B_1FBC_43AB_96E4_E7C8D837DF91__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgBar dialog
#include "Resource.h"

class CDlgBar : public CDialog
{
// Construction
public:
	void UpdateDataFromControl();
	void UpdateInfo();
	BOOL m_bUpdateingNow;
	CDlgBar();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgBar)
	enum { IDD = IDR_MAINFRAME };
	CSliderCtrl	m_sld_Light;
	CSliderCtrl	m_TileRegion_Slider;
	CSliderCtrl	m_FP_Slider;
	int		m_iZoneID;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgBar)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgBar)
	afx_msg void OnBSetResourcePath();
	afx_msg void OnChangeETransformX();
	afx_msg void OnChangeETransformY();
	afx_msg void OnChangeETransformZ();
	afx_msg void OnReleasedcaptureSliderFarplane(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	afx_msg void OnReleasedcaptureSliderTileregion(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeEZoneid();
	afx_msg void OnReleasedcaptureSliderLight(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGBAR_H__B4FC5A3B_1FBC_43AB_96E4_E7C8D837DF91__INCLUDED_)

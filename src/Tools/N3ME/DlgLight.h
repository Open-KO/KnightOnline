#if !defined(AFX_DLGLIGHT_H__61DD6BB5_F982_4ECF_ABE3_C181081BC28C__INCLUDED_)
#define AFX_DLGLIGHT_H__61DD6BB5_F982_4ECF_ABE3_C181081BC28C__INCLUDED_

#pragma once

// DlgLight.h : header file
//

#include "LightObjMgr.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgLight dialog

class CDlgLight : public CDialog
{
	// Construction
public:
	CDlgLight(CWnd* pParent = nullptr); // standard constructor

	CLightObjMgr* m_pRefLightObjMgr;
	LPLIGHTOBJ m_pSelLO;
	COLORREF m_pLOColor;

	// Dialog Data
	//{{AFX_DATA(CDlgLight)
	enum
	{
		IDD = IDD_LIGHT
	};
	CStatic m_stcColor;
	CSliderCtrl m_sld_Atten;
	CSliderCtrl m_sld_Range;
	CListBox m_ListLO;
	CString m_strPathName;
	CString m_strLightObjName;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgLight)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	void RefreshCurrLight();

	// Generated message map functions
	//{{AFX_MSG(CDlgLight)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBtnExport();
	afx_msg void OnBtnImport();
	afx_msg void OnBtnUpload();
	afx_msg void OnBtnDownload();
	afx_msg void OnBtnDelete();
	afx_msg void OnSelchangeListLights();
	afx_msg void OnReleasedcaptureSldAtten(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnReleasedcaptureSldRange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPaint();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void InsertLOList(LPLIGHTOBJ pLO);
	void Reset();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGLIGHT_H__61DD6BB5_F982_4ECF_ABE3_C181081BC28C__INCLUDED_)

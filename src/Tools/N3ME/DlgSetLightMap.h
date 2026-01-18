#if !defined(AFX_DLGSETLIGHTMAP_H__B62A60ED_ED79_4ADD_BABB_76A91158DC41__INCLUDED_)
#define AFX_DLGSETLIGHTMAP_H__B62A60ED_ED79_4ADD_BABB_76A91158DC41__INCLUDED_

#pragma once

// DlgSetLightMap.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgSetLightMap dialog

class CDlgSetLightMap : public CDialog
{
	// Construction
public:
	COLORREF m_SelDCColor;
	DWORD m_SelDXColor;
	int m_ColorWidth, m_ColorHeight;

public:
	bool MoreLight();
	int GetHardness();
	DWORD GetColor();
	int GetPenSize();
	float GetAlpha();
	CDlgSetLightMap(CWnd* pParent = nullptr); // standard constructor

											  // Dialog Data
	//{{AFX_DATA(CDlgSetLightMap)
	enum
	{
		IDD = IDD_SET_LIGHTMAP
	};
	CSliderCtrl m_sld_Opacity;
	CStatic m_ColorView;
	CSliderCtrl m_sld_PenSize;
	CSliderCtrl m_sld_Hard;
	int m_rdo_MoreLight;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgSetLightMap)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgSetLightMap)
	virtual BOOL OnInitDialog();
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnBtnSelectColor();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSETLIGHTMAP_H__B62A60ED_ED79_4ADD_BABB_76A91158DC41__INCLUDED_)

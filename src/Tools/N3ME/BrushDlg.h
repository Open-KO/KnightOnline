#if !defined(AFX_BRUSHDLG_H__1EC13A8F_61C0_4A29_A6FA_F5B686485348__INCLUDED_)
#define AFX_BRUSHDLG_H__1EC13A8F_61C0_4A29_A6FA_F5B686485348__INCLUDED_

#pragma once

// BrushDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBrushDlg dialog
class CLyTerrain;
class CBrushDlg : public CDialog
{
	// Construction
public:
	CBrushDlg(CWnd* pParent = nullptr); // standard constructor

										// Attributes
protected:
	CLyTerrain* m_pTerrain;

	// Operations
public:
	void SetTerrain(CLyTerrain* pTerrain);

protected:
	void SetTerrainBrush();

public:
	void SetFlatHeight(float height);
	// Dialog Data
	//{{AFX_DATA(CBrushDlg)
	enum
	{
		IDD = IDD_BRUSH
	};
	CSliderCtrl m_sldSize;
	CSliderCtrl m_sldFalloff;
	float m_fFalloff;
	int m_iSize;
	int m_iShape;
	BOOL m_bFlat;
	int m_rdoFlatMode;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBrushDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CBrushDlg)
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnSelchangeShape();
	afx_msg void OnCustomdrawSliderFalloff(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderSize(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckFlat();
	afx_msg void OnRdoFlaten();
	afx_msg void OnRdoGetHeight();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BRUSHDLG_H__1EC13A8F_61C0_4A29_A6FA_F5B686485348__INCLUDED_)

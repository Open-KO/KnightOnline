#if !defined(AFX_DLGSAVEDIVISION_H__6FC05E94_A0AA_4FD6_B202_EF1A99E3C349__INCLUDED_)
#define AFX_DLGSAVEDIVISION_H__6FC05E94_A0AA_4FD6_B202_EF1A99E3C349__INCLUDED_

#pragma once

// DlgSaveDivision.h : header file
//

const int TEX_VIEW_SIZE = 512;

/////////////////////////////////////////////////////////////////////////////
// CDlgSaveDivision dialog

class CDlgSaveDivision : public CDialog
{
	// Construction
public:
	CDlgSaveDivision(CWnd* pParent = nullptr); // standard constructor

											   // Variables...
	class CLyTerrain* m_pTerrain;
	class CMapMng* m_pMapMng;
	class CN3Texture* m_pMiniMap;

	int m_iTotalSize;
	int m_iDivisionSize;
	POINT m_ptMousePos;
	CRect m_rtDrawRegion;

	// Functions...
	void SetTexView();
	void RenderTexnRegion();

	// Dialog Data
	//{{AFX_DATA(CDlgSaveDivision)
	enum
	{
		IDD = IDD_SAVE_DIVIDE
	};
	CComboBox m_cbDivision;
	CString m_strTotalSize;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgSaveDivision)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgSaveDivision)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeCbDivision();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSAVEDIVISION_H__6FC05E94_A0AA_4FD6_B202_EF1A99E3C349__INCLUDED_)

#if !defined(AFX_DLGRIVERPROPERTY_H__031E22B6_02F1_44F4_BA14_68DF0CBB7A8E__INCLUDED_)
#define AFX_DLGRIVERPROPERTY_H__031E22B6_02F1_44F4_BA14_68DF0CBB7A8E__INCLUDED_

#pragma once

// DlgRiverProperty.h : header file
//

#include "../Common Control/PropertyList.h"
/////////////////////////////////////////////////////////////////////////////
// CDlgRiverProperty dialog

class CRiverMng;
class CDlgRiverProperty : public CDialog
{
	// Construction
public:
	CDlgRiverProperty(CRiverMng* pRiverMng, CWnd* pParent = nullptr); // standard constructor

																	  // Attributes
public:
	BOOL m_IsModalDialog;

protected:
	CRiverMng* m_pRiverMng;
	// Operations
public:
	void UpdateInfo();
	void RiverComboFinding(int iRiverID);
	void RiverComboSetting();

public:
	// Dialog Data
	//{{AFX_DATA(CDlgRiverProperty)
	enum
	{
		IDD = IDD_RIVER_PROPERTY
	};
	CPropertyList m_LPRiver;
	CComboBox m_ComGoRiver;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgRiverProperty)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

	// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgRiverProperty)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnButtonExtrude();
	afx_msg void OnButtonRecalcUv();
	afx_msg void OnButtonDeleteRiver();
	afx_msg void OnButtonDeleteVertex();
	afx_msg void OnButtonRecalcvertex();
	afx_msg void OnCobGORiver();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGRIVERPROPERTY_H__031E22B6_02F1_44F4_BA14_68DF0CBB7A8E__INCLUDED_)

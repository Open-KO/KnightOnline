#if !defined(AFX_DLGEDITWARP_H__F8240459_A3C8_4CE7_A176_1B4A04DA2B87__INCLUDED_)
#define AFX_DLGEDITWARP_H__F8240459_A3C8_4CE7_A176_1B4A04DA2B87__INCLUDED_

#pragma once

// DlgEditWarp.h : header file
//

#include "WarpMgr.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgEditWarp dialog

class CDlgEditWarp : public CDialog
{
	// Construction
public:
	CDlgEditWarp(CWnd* pParent = nullptr); // standard constructor

										   // variables...
	CWarpMgr* m_pRefWarpMgr;
	WARPINFO m_tmpInfo;

	// functions..
	void ResetAll();
	void SetEditor();
	void GetEditor();

	// Dialog Data
	//{{AFX_DATA(CDlgEditWarp)
	enum
	{
		IDD = IDD_EDIT_WARP
	};
	CListBox m_ListWarp;
	float m_fRadius;
	float m_fPosX;
	float m_fPosY;
	float m_fPosZ;
	CString m_strName;
	int m_iMoney;
	short m_sID;
	short m_sZoneID;
	CString m_AgreeState;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgEditWarp)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgEditWarp)
	afx_msg void OnBtnDelete();
	afx_msg void OnBtnEdit();
	afx_msg void OnBtnUpdate();
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	virtual void OnOK();
	afx_msg void OnBtnExport();
	afx_msg void OnBtnImport();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGEDITWARP_H__F8240459_A3C8_4CE7_A176_1B4A04DA2B87__INCLUDED_)

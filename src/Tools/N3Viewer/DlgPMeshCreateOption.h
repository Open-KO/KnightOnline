#if !defined(AFX_DLGPMESHCREATEOPTION_H__E03BEC53_E5C6_498A_BFEC_04DE5A9A9923__INCLUDED_)
#define AFX_DLGPMESHCREATEOPTION_H__E03BEC53_E5C6_498A_BFEC_04DE5A9A9923__INCLUDED_

#pragma once

// DlgPMeshCreateOption.h : header file
//
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgPMeshCreateOption dialog

class CDlgPMeshCreateOption : public CDialog
{
	// Construction
public:
	CDlgPMeshCreateOption(CWnd* pParent = nullptr); // standard constructor

													// Dialog Data
	//{{AFX_DATA(CDlgPMeshCreateOption)
	enum
	{
		IDD = IDD_PMESH_CREATE_OPTION
	};
	BOOL m_bTriangleLoss;
	float m_fWeight;
	int m_iTriangle;
	int m_iLoss;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgPMeshCreateOption)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgPMeshCreateOption)
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckTriangleLoss();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGPMESHCREATEOPTION_H__E03BEC53_E5C6_498A_BFEC_04DE5A9A9923__INCLUDED_)

#if !defined(AFX_DLGBASE_H__44297C60_64E5_4263_9814_AEA9AF4BC07A__INCLUDED_)
#define AFX_DLGBASE_H__44297C60_64E5_4263_9814_AEA9AF4BC07A__INCLUDED_

#pragma once

// DlgBase.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgBase dialog
#include "../Common Control/PropertyList.h"
#include "Resource.h"

class CMapMng;
class CDlgBase : public CDialog
{
	// Construction
public:
	void UpdateInfo();
	CDlgBase(CMapMng* pMapMng, CWnd* pParent = nullptr); // standard constructor

	CMapMng* m_pMapMng;

	// Dialog Data
	//{{AFX_DATA(CDlgBase)
	enum
	{
		IDD = IDD_BASE
	};
	CPropertyList m_LPMaterial;
	CPropertyList m_LPLight;
	CPropertyList m_LPCamera;
	CComboBox m_CBShapePart; // Shape Part Number
	CComboBox m_CBChrPart;   // Character Part Number
	CComboBox m_CBChrLOD;    // Character LOD Number
	CComboBox m_CBChrPlug;   // Chraracter Plug Number
	CPropertyList m_LPTransform;
	CPropertyList m_LPShape;
	CPropertyList m_LPChr;
	CPropertyList m_LPCPart;
	CPropertyList m_LPCPlug;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgBase)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

	// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgBase)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEName();
	afx_msg void OnSelchangeCbPartShape();
	afx_msg void OnSelchangeCbPartCharacter();
	afx_msg void OnSelchangeCbLod();
	afx_msg void OnSelchangeCbPlugCharacter();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGBASE_H__44297C60_64E5_4263_9814_AEA9AF4BC07A__INCLUDED_)

// DlgPondProperty.h: interface for the CDlgPondProperty class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DLGPONDPROPERTY_H__58CA9C39_270B_4DB7_ADED_464F786C84CF__INCLUDED_)
#define AFX_DLGPONDPROPERTY_H__58CA9C39_270B_4DB7_ADED_464F786C84CF__INCLUDED_

#pragma once

#include "../Common Control/PropertyList.h"

class CPondMng;
class CPondMesh;

class CDlgPondProperty : public CDialog
{
	// Construction
public:
	CDlgPondProperty(CPondMng* pPondMng, CWnd* pParent = nullptr); // standard constructor

																   // Attributes
public:
	BOOL m_IsModalDialog;

protected:
	CPondMng* m_pPondMng;
	// Operations
public:
	void UpdateInfo();
	void UpdateWaterLength(CPondMesh* pSelPond);
	void PondComboFinding(int iPondID);
	void PondComboSetting();

public:
	// Dialog Data
	//{{AFX_DATA(CDlgPondProperty)
	enum
	{
		IDD = IDD_POND_PROPERTY
	};
	CPropertyList m_LPPond;
	CComboBox m_ComGoPond;
	float m_fWidthLength;
	float m_fHeightLength;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgPondProperty)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

	// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgPondProperty)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnButtonDeletePond();
	afx_msg void OnButtonEditPond();
	afx_msg void OnButtonMovePond();
	afx_msg void OnButtonStationPond();
	afx_msg void OnButtonRecalUV();
	afx_msg void OnButtonGroup();
	afx_msg void OnCobGOPond();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_DLGPONDPROPERTY_H__58CA9C39_270B_4DB7_ADED_464F786C84CF__INCLUDED_)

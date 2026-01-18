#if !defined(AFX_DLGADDSOUNDGROUP_H__47BED37A_47AC_4E7D_8A9C_59C261A15364__INCLUDED_)
#define AFX_DLGADDSOUNDGROUP_H__47BED37A_47AC_4E7D_8A9C_59C261A15364__INCLUDED_

#pragma once

// DlgAddSoundGroup.h : header file
//

#include "SoundMgr.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgAddSoundGroup dialog

class CDlgAddSoundGroup : public CDialog
{
	// Construction
public:
	CDlgAddSoundGroup(CWnd* pParent = nullptr); // standard constructor

												// Dialog Data
	//{{AFX_DATA(CDlgAddSoundGroup)
	enum
	{
		IDD = IDD_ADD_SOUNDGROUP
	};
	float m_fBGE_Regen_1;
	float m_fBGE_Regen_2;
	float m_fBGE_Regen_3;
	float m_fBGE_Regen_4;
	float m_fBGM_Regen_1;
	float m_fBGM_Regen_2;
	float m_fBGM_Regen_3;
	float m_fBGM_Regen_4;
	//}}AFX_DATA

public:
	SOUNDINFO m_SndInfo;

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgAddSoundGroup)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

public:
	void SetSndInfo(LPSOUNDINFO pSndInfo);

	// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgAddSoundGroup)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnBgeName1();
	afx_msg void OnBtnBgeName2();
	afx_msg void OnBtnBgeName3();
	afx_msg void OnBtnBgeName4();
	afx_msg void OnBtnBgmName1();
	afx_msg void OnBtnBgmName2();
	afx_msg void OnBtnBgmName3();
	afx_msg void OnBtnBgmName4();
	virtual void OnOK();
	afx_msg void OnBtnDelBgeName1();
	afx_msg void OnBtnDelBgeName2();
	afx_msg void OnBtnDelBgeName3();
	afx_msg void OnBtnDelBgeName4();
	afx_msg void OnBtnDelBgmName1();
	afx_msg void OnBtnDelBgmName2();
	afx_msg void OnBtnDelBgmName3();
	afx_msg void OnBtnDelBgmName4();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGADDSOUNDGROUP_H__47BED37A_47AC_4E7D_8A9C_59C261A15364__INCLUDED_)

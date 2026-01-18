#if !defined(AFX_DLGEDITPARTGROUND_H__CCE20799_81F0_4582_B9AA_B7D0F5C0E142__INCLUDED_)
#define AFX_DLGEDITPARTGROUND_H__CCE20799_81F0_4582_B9AA_B7D0F5C0E142__INCLUDED_

#pragma once

// DlgEditPartGround.h : header file
//

class CMainFrame;

/////////////////////////////////////////////////////////////////////////////
// CDlgEditPartGround dialog

class CDlgEditPartGround : public CDialog
{
	// Construction
public:
	CDlgEditPartGround(CWnd* pParent = nullptr); // standard constructor

	CMainFrame* m_pRefFrm;

	// Dialog Data
	//{{AFX_DATA(CDlgEditPartGround)
	enum
	{
		IDD = IDD_DLG_PART_GROUND
	};
	CComboBox m_CBSrcBlend;
	CComboBox m_CBDestBlend;
	BOOL m_chkAlpha;
	BOOL m_chkTexLoop;
	float m_fAccelX;
	float m_fAccelZ;
	float m_fFadeIn;
	float m_fFadeOut;
	float m_fLife;
	float m_fMoveX;
	float m_fMoveZ;
	int m_iNumTex;
	float m_fPosX;
	float m_fPosZ;
	float m_fRot;
	float m_fScaleAccelX;
	float m_fScaleAccelZ;
	float m_fScaleVelX;
	float m_fScaleVelZ;
	float m_fScaleX;
	float m_fScaleZ;
	float m_fTexVelocity;
	CString m_strTexName;
	CString m_strPathName;
	float m_fGap;
	BOOL m_bDoubleSide;
	BOOL m_bLight;
	BOOL m_bZBuffer;
	BOOL m_bZWrite;
	//}}AFX_DATA

	bool LoadPartScript(const char* szPath);

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgEditPartGround)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgEditPartGround)
	afx_msg void OnPartGroundBtnLoadTex();
	afx_msg void OnPartGroundBtnSave();
	afx_msg void OnPartGroundBtnSaveAs();
	virtual void OnOK();
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGEDITPARTGROUND_H__CCE20799_81F0_4582_B9AA_B7D0F5C0E142__INCLUDED_)

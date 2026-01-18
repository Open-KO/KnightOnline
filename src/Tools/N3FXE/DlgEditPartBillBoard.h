#if !defined(AFX_DLGEDITPARTBILLBOARD_H__4593EFF6_FFDC_40F5_ADD9_00F5B26481E3__INCLUDED_)
#define AFX_DLGEDITPARTBILLBOARD_H__4593EFF6_FFDC_40F5_ADD9_00F5B26481E3__INCLUDED_

#pragma once

// DlgEditPartBillBoard.h : header file
//

class CMainFrame;

/////////////////////////////////////////////////////////////////////////////
// CDlgEditPartBillBoard dialog

class CDlgEditPartBillBoard : public CDialog
{
	// Construction
public:
	CDlgEditPartBillBoard(CWnd* pParent = nullptr); // standard constructor

	CMainFrame* m_pRefFrm;

	// Dialog Data
	//{{AFX_DATA(CDlgEditPartBillBoard)
	enum
	{
		IDD = IDD_DLG_PART_BILLBOARD
	};
	CComboBox m_CBSrcBlend;
	CComboBox m_CBDestBlend;
	BOOL m_bAlphaBlend;
	BOOL m_bTexLoop;
	float m_fAccelX;
	float m_fAccelY;
	float m_fAccelZ;
	float m_fDist4Vol;
	float m_fFadeOut;
	float m_fLife;
	float m_fMoveX;
	float m_fMoveY;
	float m_fMoveZ;
	int m_iNumTex;
	float m_fPosX;
	float m_fPosY;
	float m_fPosZ;
	CString m_strPathName;
	float m_fScaleX;
	float m_fScaleY;
	float m_fTexVelocity;
	CString m_strTexName;
	BOOL m_bRotateY;
	float m_fFadeIn;
	float m_fScaleAccelX;
	float m_fScaleAccelY;
	float m_fScaleVelX;
	float m_fScaleVelY;
	BOOL m_bOnGround;
	float m_fRotVelocity;
	BOOL m_bDoubleSide;
	BOOL m_bLight;
	BOOL m_bZBuffer;
	BOOL m_bZWrite;
	float m_fRotBillBoardX;
	float m_fRotBillBoardY;
	float m_fRotBillBoardZ;
	//}}AFX_DATA

	bool LoadPartScript(const char* szPath);

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgEditPartBillBoard)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgEditPartBillBoard)
	afx_msg void OnPartBoardBtnLoadTex();
	afx_msg void OnPartBoardBtnSave();
	afx_msg void OnPartBoardBtnSaveAs();
	virtual void OnOK();
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGEDITPARTBILLBOARD_H__4593EFF6_FFDC_40F5_ADD9_00F5B26481E3__INCLUDED_)

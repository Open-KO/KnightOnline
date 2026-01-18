#if !defined(AFX_DLGEDITPARTPARTICLE_H__34F3B502_A145_4084_AC04_CA2C62CDFAFB__INCLUDED_)
#define AFX_DLGEDITPARTPARTICLE_H__34F3B502_A145_4084_AC04_CA2C62CDFAFB__INCLUDED_

#pragma once

// DlgEditPartParticle.h : header file
//

#include <N3Base/N3FXDef.h>

class CMainFrame;

/////////////////////////////////////////////////////////////////////////////
// CDlgEditPartParticle dialog

class CDlgEditPartParticle : public CDialog
{
	// Construction
public:
	CDlgEditPartParticle(CWnd* pParent = nullptr); // standard constructor

	CMainFrame* m_pRefFrm;
	DWORD m_Color[NUM_KEY_COLOR];
	bool m_bColorKey[NUM_KEY_COLOR];
	bool m_bAlphaKey[NUM_KEY_COLOR];

	// Dialog Data
	//{{AFX_DATA(CDlgEditPartParticle)
	enum
	{
		IDD = IDD_DLG_PART_PARTICLE
	};
	CComboBox m_CBEmitType;
	CComboBox m_CBSrcBlend;
	CComboBox m_CBDestBlend;
	BOOL m_bAlphaBlend;
	float m_fAccelX;
	float m_fAccelY;
	float m_fAccelZ;
	float m_fFadeOut;
	float m_fCreateInterval;
	float m_fLife;
	float m_fMoveX;
	float m_fMoveY;
	float m_fMoveZ;
	int m_iCreateNum;
	int m_iNumParticle;
	int m_iNumTex;
	float m_fPosX;
	float m_fPosY;
	float m_fPosZ;
	float m_fSize;
	float m_fTexVelocity;
	CString m_strPathName;
	CString m_strTexName;
	float m_fEmitAccel;
	float m_fEmitGravity;
	float m_fEmitRot;
	float m_fEmitVel;
	float m_fCreateMaxX;
	float m_fCreateMaxY;
	float m_fCreateMaxZ;
	float m_fCreateMinX;
	float m_fCreateMinY;
	float m_fCreateMinZ;
	float m_fParticleLifeMin;
	float m_fParticleLifeMax;
	float m_fEmitArg1;
	float m_fEmitArg2;
	float m_fEmitArg3;
	float m_EmitDirX;
	float m_EmitDirY;
	float m_EmitDirZ;
	BOOL m_bChangeColor;
	CString m_strShapeName;
	float m_fShapeFPS;
	BOOL m_bAnimKey;
	float m_fFadeIn;
	float m_fSizeMax;
	float m_fSizeMin;
	BOOL m_bOnGround;
	BOOL m_bDoubleSide;
	BOOL m_bLight;
	BOOL m_bZBuffer;
	BOOL m_bZWrite;
	float m_fTexRotVelocity;
	float m_fScaleVelX;
	float m_fScaleVelY;
	//}}AFX_DATA

	bool LoadPartScript(const char* szPath);
	void SetEmitTypeState(int EmitType);

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgEditPartParticle)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgEditPartParticle)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnPartParticleBtnLoadTex();
	afx_msg void OnPartParticleBtnSave();
	afx_msg void OnPartParticleBtnSaveAs();
	virtual void OnOK();
	afx_msg void OnSelchangePartParticleCbEmitType();
	afx_msg void OnPartParticleBtnPutColor();
	afx_msg void OnPartParticleBtnLoadMesh();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGEDITPARTPARTICLE_H__34F3B502_A145_4084_AC04_CA2C62CDFAFB__INCLUDED_)

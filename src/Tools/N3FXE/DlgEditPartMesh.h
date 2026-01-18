#if !defined(AFX_DLGEDITPARTMESH_H__F7DE8495_B22C_49BF_A4BF_4E7C27E6687E__INCLUDED_)
#define AFX_DLGEDITPARTMESH_H__F7DE8495_B22C_49BF_A4BF_4E7C27E6687E__INCLUDED_

#pragma once

// DlgEditPartMesh.h : header file
//

class CMainFrame;

/////////////////////////////////////////////////////////////////////////////
// CDlgEditPartMesh dialog

class CDlgEditPartMesh : public CDialog
{
	// Construction
public:
	CDlgEditPartMesh(CWnd* pParent = nullptr); // standard constructor

	CMainFrame* m_pRefFrm;

	// Dialog Data
	//{{AFX_DATA(CDlgEditPartMesh)
	enum
	{
		IDD = IDD_DLG_PART_MESH
	};
	CComboBox m_CBTexUVMove;
	CComboBox m_CBSrcBlend;
	CComboBox m_CBDestBlend;
	CString m_strPathName;
	BOOL m_bAlphaBlend;
	BOOL m_bTexLoop;
	CString m_strShapeName;
	float m_fTexUVVelocity;
	float m_fScaleZ;
	float m_fScaleY;
	float m_fScaleX;
	float m_fAccelX;
	float m_fAccelY;
	float m_fAccelZ;
	float m_fBaseScaleX;
	float m_fBaseScaleY;
	float m_fBaseScaleZ;
	float m_fFadeOut;
	float m_fLife;
	float m_fMoveX;
	float m_fMoveY;
	float m_fMoveZ;
	float m_fPosX;
	float m_fPosY;
	float m_fPosZ;
	float m_fRotX;
	float m_fRotY;
	float m_fRotZ;
	float m_fScaleAccelX;
	float m_fScaleAccelY;
	float m_fScaleAccelZ;
	float m_fMeshFPS;
	float m_fTexVelocity;
	float m_fFadeIn;
	BOOL m_bOnGround;
	BOOL m_bDoubleSide;
	BOOL m_bLight;
	BOOL m_bZBuffer;
	BOOL m_bZWrite;
	//}}AFX_DATA

	bool LoadPartScript(const char* szPath);

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgEditPartMesh)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgEditPartMesh)
	afx_msg void OnPartMeshBtnSave();
	afx_msg void OnPartMeshBtnSaveAs();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnPartMeshBtnLoadShape();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGEDITPARTMESH_H__F7DE8495_B22C_49BF_A4BF_4E7C27E6687E__INCLUDED_)

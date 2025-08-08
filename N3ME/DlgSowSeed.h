#if !defined(AFX_DLGSOWSEED_H__FB12A635_1E40_49A7_A037_F5D5512DC126__INCLUDED_)
#define AFX_DLGSOWSEED_H__FB12A635_1E40_49A7_A037_F5D5512DC126__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgSowSeed.h : header file
//

#include "LyTerrainDef.h"
#include <N3Base/N3Texture.h>
//#include <list>

class CMainFrame;

typedef struct __SeedTex
{
	CN3Texture* pTex;
	int			ID;

	__SeedTex()
	{
		ID = 0;
		pTex = NULL;
	}
	~__SeedTex()
	{
		if(pTex) delete pTex;
	}
} SEEDTEX, *LPSEEDTEX;

/////////////////////////////////////////////////////////////////////////////
// CDlgSowSeed dialog
enum E_TPYE { CS_NONE = 0 ,CS_SOW = 1, CS_SELECT =2};
class CDlgSowSeed : public CDialog
{
// Construction
public:
	CDlgSowSeed(CWnd* pParent = NULL);   // standard constructor
	
public:
	CMainFrame*	m_pRefFrm;
	//std::list<LPSEEDGROUP> m_SeedGroupList;
	//char	m_FileName[MAX_PATH];
	int Brush_Size;
	int SowDetail;
	int Sow_Array[MAX_BRUSH_SIZE][MAX_BRUSH_SIZE];
	int Obj_Index;
	int  Sow_Select_Flage;

// normal functions..
	void	Release();
	void	RePaint();

// Dialog Data
	//{{AFX_DATA(CDlgSowSeed)
	enum { IDD = IDD_SOW_SEED };
	CSliderCtrl	m_SowDetail;
	CComboBox	m_CB_TileGroup;
	CSliderCtrl	m_Brush_Size;
	CComboBox	m_CB_Seed;
	//}}AFX_DATA



// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgSowSeed)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgSowSeed)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnDelSeed();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnBtnGrassinfoload();
	afx_msg void OnBtnSavegame();
	afx_msg void OnDblclkLbSowed();
	afx_msg void OnBtnGrassinfosave();
	afx_msg void OnBtnGrassRead();
	afx_msg void OnBtnGrassDelete();
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSelchangeCbSeed();
	afx_msg void OnSelchangeCbTilegroup();
	afx_msg void OnSeedExit();
	afx_msg void OnCustomdrawSowBrushSize(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonRandom();
	afx_msg void OnCustomdrawSTileDetail(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSowseed();
	afx_msg void OnRadioSow();
	afx_msg void OnRadioSelect();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSOWSEED_H__FB12A635_1E40_49A7_A037_F5D5512DC126__INCLUDED_)

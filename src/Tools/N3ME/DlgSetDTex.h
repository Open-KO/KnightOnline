#if !defined(AFX_DLGSETDTEX_H__AE822ABC_F44E_4C09_BA6D_FEF8D41EF5B8__INCLUDED_)
#define AFX_DLGSETDTEX_H__AE822ABC_F44E_4C09_BA6D_FEF8D41EF5B8__INCLUDED_

#pragma once

// DlgSetDTex.h : header file
//

class CN3Texture;

/////////////////////////////////////////////////////////////////////////////
// CDlgSetDTex dialog
class CDlgSetDTex : public CDialog
{
public:
	float m_fTexSurfaceSize;
	CN3Texture* m_pTex;
	LPDIRECT3DVERTEXBUFFER9 m_pTexVB;
	LPDIRECT3DVERTEXBUFFER9 m_pGridVB;

	// Construction
public:
	CDlgSetDTex(CWnd* pParent = nullptr); // standard constructor

										  // Dialog Data
	//{{AFX_DATA(CDlgSetDTex)
	enum
	{
		IDD = IDD_SETDETAILTEXTURE
	};
	CListBox m_GroupList;
	CComboBox m_FileList;
	CString m_szCurrAttr;
	CString m_TileSetName;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgSetDTex)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgSetDTex)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSelchangeComboFilelist();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnBtnSave();
	afx_msg void OnBtnAddgroup();
	afx_msg void OnBtnDelgroup();
	afx_msg void OnBtnAdddtex();
	afx_msg void OnBtnDeldtex();
	afx_msg void OnBtnSaveNewnameTileset();
	afx_msg void OnBtnSaveTileset();
	afx_msg void OnBtnLoadTileset();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void RenderGrid(LPDIRECT3DDEVICE9 lpDDev);
	void RenderTex(LPDIRECT3DDEVICE9 lpDDev);
	void InitVB();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSETDTEX_H__AE822ABC_F44E_4C09_BA6D_FEF8D41EF5B8__INCLUDED_)

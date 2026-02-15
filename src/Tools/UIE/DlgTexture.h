#if !defined(AFX_DLGTEXTURE_H__A7C76815_D640_4A33_AEE0_2C53DC8D7CD8__INCLUDED_)
#define AFX_DLGTEXTURE_H__A7C76815_D640_4A33_AEE0_2C53DC8D7CD8__INCLUDED_

#pragma once

// DlgTexture.h : header file
//
#include "TexViewer.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgTexture dialog
class CTexViewer;
class CDlgTexture : public CDialog
{
	// Construction
public:
	CDlgTexture(CWnd* pParent = nullptr); // standard constructor
	virtual ~CDlgTexture();

										  // Dialog Data
	//{{AFX_DATA(CDlgTexture)
	enum
	{
		IDD = IDD_TEXTURE
	};
	CComboBox m_ImageType;
	int m_RadioEditMode;
	//}}AFX_DATA

	// Attributes
public:
protected:
	CTexViewer* m_pTexViewer;
	int m_iImageTypeCount;
	char m_szImageTypeNames[MAX_IMAGETYPE][_MAX_PATH];
	HACCEL m_hAccelTable;
	// Operations
public:
	void SetTexture(LPCTSTR pszFileName);
	BOOL GetSelectedUVRect(struct __FLOAT_RECT* pFRect) const;
	void SetSelectedUVRect(const struct __FLOAT_RECT* pFRect); // 현재 선택된 UV좌표 넣기
	CRect GetSelectedRect() const;
	void SetImageTypes(int iCount, char** pszNames);
	CRect GetImageRect(int iIndex, __FLOAT_RECT* pUVRect = nullptr);

protected:
	void Resize();

public:
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgTexture)
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgTexture)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnRadioSelect();
	afx_msg void OnRadioZoom();
	afx_msg void OnRadioHand();
	afx_msg void OnDoubleclickedRadioZoom();
	virtual void OnOK();
	afx_msg void OnSelchangeComboImagetype();
	afx_msg void OnButtonAutoselectH();
	afx_msg void OnButtonAutoselectV();
	//}}AFX_MSG
	afx_msg LRESULT OnUpdateInfo(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGTEXTURE_H__A7C76815_D640_4A33_AEE0_2C53DC8D7CD8__INCLUDED_)

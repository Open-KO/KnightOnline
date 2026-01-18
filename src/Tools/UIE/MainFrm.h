// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__6A31AAD9_D434_4C15_949D_A76014174F08__INCLUDED_)
#define AFX_MAINFRM_H__6A31AAD9_D434_4C15_949D_A76014174F08__INCLUDED_

#pragma once

#include "DlgBar.h"
#include <N3Base/N3EngTool.h>

class CUIEView;
class CHierarchyView;
class CPropertyView;
class CMainFrame : public CFrameWnd
{
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

	// Attributes
protected:
	CSplitterWnd m_wndSplitter;
	CSplitterWnd m_wndSplitterLeft;
	HACCEL m_hDefaultAccelTable;

public:
	CN3EngTool m_Eng;

	// Operations
public:
	void SetBasePath(LPCTSTR pszPath);
	void SetStatusText(LPCTSTR pszText);
	void EnableAccelerator(BOOL bEnable);
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

	// Implementation
public:
	virtual ~CMainFrame();
	CUIEView* GetRightPane();
	CHierarchyView* GetHierarchyView();
	CPropertyView* GetPropertyView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected: // control bar embedded members
	CStatusBar m_wndStatusBar;
	CToolBar m_wndToolBar;
	CReBar m_wndReBar;
	CDlgBar m_wndDlgBar;

	// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewPreview();
	afx_msg void OnUpdateViewPreview(CCmdUI* pCmdUI);
	afx_msg void OnViewEdit();
	afx_msg void OnUpdateViewEdit(CCmdUI* pCmdUI);
	afx_msg void OnEditrectIncreaseX();
	afx_msg void OnEditrectIncreaseY();
	afx_msg void OnEditrectDecreaseX();
	afx_msg void OnEditrectDecreaseY();
	afx_msg void OnEditrectIncreaseWidth();
	afx_msg void OnEditrectIncreaseHeight();
	afx_msg void OnEditrectDecreaseWidth();
	afx_msg void OnEditrectDecreaseHeight();
	afx_msg void OnDestroy();
	afx_msg void OnOptionBkcolor();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__6A31AAD9_D434_4C15_949D_A76014174F08__INCLUDED_)

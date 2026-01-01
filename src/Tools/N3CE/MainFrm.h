// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__8AE789B9_D3EB_4161_B5B1_AE5DF24FAD80__INCLUDED_)
#define AFX_MAINFRM_H__8AE789B9_D3EB_4161_B5B1_AE5DF24FAD80__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CN3CEView;
class CFormViewProperty;
class CFormViewTool;
class CFormViewAnimation;

//#include "DlgChrProperty.h"
#include <N3Base/N3EngTool.h>
#include <N3Base/N3SndMgr.h>

class CMainFrame : public CFrameWnd
{
public:
	CN3EngTool		m_Eng;
	CN3SndMgr		m_SndMgr;
	CN3SndObj*		m_pSndObj0;
	CN3SndObj*		m_pSndObj1;
//	CDlgChrProperty m_DlgProperty;

public:
	CFormViewProperty* GetPaneProperty();
	CN3CEView* GetPaneRender();
	CFormViewTool* GetPaneTool();
	CFormViewAnimation* GetPaneAnimation();
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
protected:
	CSplitterWnd m_wndSplitter;

// Operations
public:

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

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnEditChrProperty();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__8AE789B9_D3EB_4161_B5B1_AE5DF24FAD80__INCLUDED_)

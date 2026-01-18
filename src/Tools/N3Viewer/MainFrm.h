// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__9737F766_0201_4670_B069_EDF41F7C1BDF__INCLUDED_)
#define AFX_MAINFRM_H__9737F766_0201_4670_B069_EDF41F7C1BDF__INCLUDED_

#pragma once

#include <N3Base/N3EngTool.h>

#include "N3ViewerView.h"
#include "ViewSceneTree.h"
#include "ViewProperty.h"

#include "DlgPMeshEdit.h"

enum eToolMode
{
	MODE_NORMAL     = 0,
	MODE_EDIT_PMESH = 1,
};

class CMainFrame : public CFrameWnd
{
public:
	CSplitterWnd m_wndSplitter;
	CSplitterWnd m_wndSplitterLeft;

	CDlgPMeshEdit m_DlgPMeshEdit;

	CN3EngTool m_Eng;
	eToolMode m_eMode;
	BOOL m_IsPlaying;

protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

public:
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

	CN3ViewerView* GetView();
	CViewSceneTree* GetViewSceneTree();
	CViewProperty* GetViewProperty();
	void SetStatusBarText(LPCTSTR lpszText)
	{
		m_wndStatusBar.SetPaneText(0, lpszText);
	}

	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected: // control bar embedded members
	CStatusBar m_wndStatusBar;
	CToolBar m_wndToolBar;

	// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnUpdateEditPmesh(CCmdUI* pCmdUI);
	afx_msg void OnEditPmesh();
	afx_msg void OnProjectSet();
	afx_msg void OnAddLod();
	afx_msg void OnPlay();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnToolFixProgressiveMesh();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__9737F766_0201_4670_B069_EDF41F7C1BDF__INCLUDED_)

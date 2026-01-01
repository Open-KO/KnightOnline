// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__38685DF5_9C08_49EB_8984_E5E5BB13CE5C__INCLUDED_)
#define AFX_MAINFRM_H__38685DF5_9C08_49EB_8984_E5E5BB13CE5C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3EngTool.h>
#include <N3Base/N3Shape.h>
#include <N3Base/N3Camera.h>
#include <N3Base/N3Light.h>

class CMainFrame : public CFrameWnd
{
public:
	CN3EngTool	m_Eng;
	CN3Shape	m_ObjectBundle;
	CN3Camera	m_Camera;
	CN3Light	m_Lights[3];
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation
public:
	class CSkyViewerView*		GetViewRender();
	class CFormViewProperty*	GetViewProperty();

	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar		m_wndStatusBar;
	CToolBar		m_wndToolBar;
	CSplitterWnd	m_wndSplitter;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnImportObject();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__38685DF5_9C08_49EB_8984_E5E5BB13CE5C__INCLUDED_)

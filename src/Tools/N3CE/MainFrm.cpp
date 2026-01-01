// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "N3CE.h"

#include "MainFrm.h"
#include "FormViewProperty.h"
#include "N3CEDoc.h"
#include "N3CEView.h"
#include "FormViewTool.h"
#include "FormViewAnimation.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_EDIT_CHR_PROPERTY, OnEditChrProperty)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	m_pSndObj0 = nullptr;
	m_pSndObj1 = nullptr;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	// Engine 생성
//	m_Eng.InitEnv();
	if(m_Eng.Init(TRUE, m_hWnd, 64, 64, 0, TRUE) == false) return -1;
	m_Eng.GridCreate(20, 20); // 그리드 만들기..
	m_SndMgr.Init();

	return 0;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/,
	CCreateContext* pContext)
{
	// create splitter window
	if (!m_wndSplitter.CreateStatic(this, 2, 2))
		return FALSE;

	CRect rc;
	GetClientRect(rc);
	int nW1 = rc.Width() / 4;
	int nH1 = rc.Height() * 186 / 300;
	if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CFormViewProperty), CSize(nW1, nH1), pContext) ||
		!m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CN3CEView), CSize(nW1, nH1), pContext) || 
		!m_wndSplitter.CreateView(1, 0, RUNTIME_CLASS(CFormViewTool), CSize(nW1, nH1), pContext) ||
		!m_wndSplitter.CreateView(1, 1, RUNTIME_CLASS(CFormViewAnimation), CSize(nW1, nH1), pContext) )
	{
		m_wndSplitter.DestroyWindow();
		return FALSE;
	}

	return TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	cs.style = WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE
		| WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	cs.x = GetSystemMetrics(SM_CXSCREEN) / 6;
	cs.y = GetSystemMetrics(SM_CYSCREEN) / 6;
	cs.cx = GetSystemMetrics(SM_CXSCREEN) * 2 / 3;
	cs.cy = GetSystemMetrics(SM_CYSCREEN) * 2 / 3;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

CFormViewProperty* CMainFrame::GetPaneProperty()
{
	if(m_wndSplitter.GetSafeHwnd() == nullptr) return nullptr;

	CWnd* pWnd = m_wndSplitter.GetPane(0, 0);
	CFormViewProperty* pView = DYNAMIC_DOWNCAST(CFormViewProperty, pWnd);
	return pView;
}

CN3CEView* CMainFrame::GetPaneRender()
{
	if(m_wndSplitter.GetSafeHwnd() == nullptr) return nullptr;

	CWnd* pWnd = m_wndSplitter.GetPane(0, 1);
	CN3CEView* pView = DYNAMIC_DOWNCAST(CN3CEView, pWnd);
	return pView;
}

CFormViewTool* CMainFrame::GetPaneTool()
{
	if(m_wndSplitter.GetSafeHwnd() == nullptr) return nullptr;

	CWnd* pWnd = m_wndSplitter.GetPane(1, 0);
	CFormViewTool* pView = DYNAMIC_DOWNCAST(CFormViewTool, pWnd);
	return pView;
}

CFormViewAnimation* CMainFrame::GetPaneAnimation()
{
	if(m_wndSplitter.GetSafeHwnd() == nullptr) return nullptr;

	CWnd* pWnd = m_wndSplitter.GetPane(1, 1);
	CFormViewAnimation* pView = DYNAMIC_DOWNCAST(CFormViewAnimation, pWnd);
	return pView;
}


void CMainFrame::OnEditChrProperty() 
{
//	if(m_DlgProperty.GetSafeHwnd() == nullptr) m_DlgProperty.Create(IDD_CHR_PROPERTY, this);
//	
//	m_DlgProperty.UpdateInfo();
//	m_DlgProperty.ShowWindow(SW_SHOW);
}

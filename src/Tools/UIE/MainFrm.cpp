// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "UIE.h"

#include "MainFrm.h"
#include "HierarchyView.h"
#include "UIEView.h"
#include "PropertyView.h"
#include <N3Base/N3SndMgr.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
//{{AFX_MSG_MAP(CMainFrame)
ON_WM_CREATE()
ON_COMMAND(ID_VIEW_PREVIEW, OnViewPreview)
ON_UPDATE_COMMAND_UI(ID_VIEW_PREVIEW, OnUpdateViewPreview)
ON_COMMAND(ID_VIEW_EDIT, OnViewEdit)
ON_UPDATE_COMMAND_UI(ID_VIEW_EDIT, OnUpdateViewEdit)
ON_COMMAND(ID_EDITRECT_INCREASE_X, OnEditrectIncreaseX)
ON_COMMAND(ID_EDITRECT_INCREASE_Y, OnEditrectIncreaseY)
ON_COMMAND(ID_EDITRECT_DECREASE_X, OnEditrectDecreaseX)
ON_COMMAND(ID_EDITRECT_DECREASE_Y, OnEditrectDecreaseY)
ON_COMMAND(ID_EDITRECT_INCREASE_WIDTH, OnEditrectIncreaseWidth)
ON_COMMAND(ID_EDITRECT_INCREASE_HEIGHT, OnEditrectIncreaseHeight)
ON_COMMAND(ID_EDITRECT_DECREASE_WIDTH, OnEditrectDecreaseWidth)
ON_COMMAND(ID_EDITRECT_DECREASE_HEIGHT, OnEditrectDecreaseHeight)
ON_WM_DESTROY()
ON_COMMAND(ID_OPTION_BKCOLOR, OnOptionBkcolor)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] = {
	ID_SEPARATOR, // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_hDefaultAccelTable = nullptr;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this) || !m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1; // fail to create
	}
	if (!m_wndDlgBar.Create(IDR_MAINFRAME, this))
	//	if (!m_wndDlgBar.Create(this, IDR_MAINFRAME,
	//		CBRS_ALIGN_TOP, AFX_IDW_DIALOGBAR))
	{
		TRACE0("Failed to create dialogbar\n");
		return -1; // fail to create
	}

	if (!m_wndReBar.Create(this) || !m_wndReBar.AddBar(&m_wndToolBar)
		|| !m_wndReBar.AddBar(&m_wndDlgBar))
	{
		TRACE0("Failed to create rebar\n");
		return -1; // fail to create
	}

	if (!m_wndStatusBar.Create(this)
		|| !m_wndStatusBar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1; // fail to create
	}

	// TODO: Remove this if you don't want tool tips
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);

	// Engine 생성
	//m_Eng.InitEnv();

	if (!m_Eng.Init(TRUE, GetRightPane()->m_hWnd, 64, 64, 0, TRUE))
		return -1;
	m_Eng.s_SndMgr.Init();

	char szPath[_MAX_PATH];
	char szDrive[_MAX_DRIVE];
	char szDir[_MAX_DIR];
	GetModuleFileName(nullptr, szPath, _MAX_PATH);
	_splitpath(szPath, szDrive, szDir, nullptr, nullptr);
	_makepath(szPath, szDrive, szDir, nullptr, nullptr);
	//	SetCurrentDirectory(szPath);
	//	SetBasePath(szPath);

	CWinApp* pApp = AfxGetApp();
	ASSERT(pApp);
	CString szRet = pApp->GetProfileString("Work", "Path", "empty");
	if (szRet == "empty")
	{
		pApp->WriteProfileString("Work", "Path", szPath);
		SetBasePath(szPath);
	}
	else
		SetBasePath(szRet);

	return 0;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/, CCreateContext* pContext)
{
	// create splitter window
	//	if (!m_wndSplitter.CreateStatic(this, 1, 2))
	//		return FALSE;

	//	if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CHierarchyView), CSize(100, 100), pContext) ||
	//		!m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CUIEView), CSize(100, 100), pContext))
	//	{
	//		m_wndSplitter.DestroyWindow();
	//		return FALSE;
	//	}

	CRect rc;
	GetClientRect(rc);
	int iW = rc.Width() / 4;
	int iH = rc.Height() / 2;

	if (m_wndSplitter.CreateStatic(this, 1, 2) == FALSE)
		return FALSE;
	if (m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CUIEView), CSize(100, 100), pContext) == FALSE)
		return FALSE;

	if (m_wndSplitterLeft.CreateStatic(&m_wndSplitter, 2, 1) == FALSE)
		return FALSE;
	if (m_wndSplitterLeft.CreateView(0, 0, RUNTIME_CLASS(CHierarchyView), CSize(iW, iH), pContext)
		== FALSE)
		return FALSE;
	if (m_wndSplitterLeft.CreateView(1, 0, RUNTIME_CLASS(CPropertyView), CSize(iW, iH), pContext)
		== FALSE)
		return FALSE;

	m_wndSplitter.SetColumnInfo(0, iW, 0);

	return TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CFrameWnd::PreCreateWindow(cs))
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

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

CUIEView* CMainFrame::GetRightPane()
{
	CWnd* pWnd      = m_wndSplitter.GetPane(0, 1);
	CUIEView* pView = DYNAMIC_DOWNCAST(CUIEView, pWnd);
	return pView;
}

CPropertyView* CMainFrame::GetPropertyView()
{
	CWnd* pWnd           = m_wndSplitterLeft.GetPane(1, 0);
	CPropertyView* pView = DYNAMIC_DOWNCAST(CPropertyView, pWnd);
	return pView;
}

CHierarchyView* CMainFrame::GetHierarchyView()
{
	CWnd* pWnd            = m_wndSplitterLeft.GetPane(0, 0);
	CHierarchyView* pView = DYNAMIC_DOWNCAST(CHierarchyView, pWnd);
	return pView;
}

void CMainFrame::OnViewPreview()
{
	GetRightPane()->SetMode(CUIEView::UIEMODE_PREVIEW);
}

void CMainFrame::OnUpdateViewPreview(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(CUIEView::UIEMODE_PREVIEW == GetRightPane()->GetMode() ? TRUE : FALSE);
}

void CMainFrame::OnViewEdit()
{
	GetRightPane()->SetMode(CUIEView::UIEMODE_EDIT);
}

void CMainFrame::OnUpdateViewEdit(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(CUIEView::UIEMODE_EDIT == GetRightPane()->GetMode() ? TRUE : FALSE);
}

void CMainFrame::SetBasePath(LPCTSTR pszPath)
{
	m_Eng.PathSet(pszPath);
	if (nullptr == pszPath)
		m_wndDlgBar.SetDlgItemText(IDC_EDIT_BASEPATH, _T(""));
	else
		m_wndDlgBar.SetDlgItemText(IDC_EDIT_BASEPATH, m_Eng.PathGet().c_str());
}

void CMainFrame::SetStatusText(LPCTSTR pszText)
{
	m_wndStatusBar.SetPaneText(0, pszText);
}

void CMainFrame::OnEditrectIncreaseX()
{
	GetRightPane()->MoveSelectedRect(1, 0);
}

void CMainFrame::OnEditrectIncreaseY()
{
	GetRightPane()->MoveSelectedRect(0, 1);
}

void CMainFrame::OnEditrectDecreaseX()
{
	GetRightPane()->MoveSelectedRect(-1, 0);
}

void CMainFrame::OnEditrectDecreaseY()
{
	GetRightPane()->MoveSelectedRect(0, -1);
}

void CMainFrame::OnEditrectIncreaseWidth()
{
	GetRightPane()->ResizeSelectedRect(1, 0);
}

void CMainFrame::OnEditrectIncreaseHeight()
{
	GetRightPane()->ResizeSelectedRect(0, 1);
}

void CMainFrame::OnEditrectDecreaseWidth()
{
	GetRightPane()->ResizeSelectedRect(-1, 0);
}

void CMainFrame::OnEditrectDecreaseHeight()
{
	GetRightPane()->ResizeSelectedRect(0, -1);
}

void CMainFrame::OnDestroy()
{
	CFrameWnd::OnDestroy();
	m_Eng.Release();
}

void CMainFrame::EnableAccelerator(BOOL bEnable)
{
	// 처음 호출되면 원래 Accel table 저장해놓기
	if (nullptr == m_hDefaultAccelTable)
		m_hDefaultAccelTable = m_hAccelTable;
	ASSERT(m_hDefaultAccelTable);

	if (bEnable)
		m_hAccelTable = m_hDefaultAccelTable;
	else
		m_hAccelTable = nullptr;
}

void CMainFrame::OnOptionBkcolor()
{
	GetRightPane()->ChangeBkgndColor();
	Invalidate();
}

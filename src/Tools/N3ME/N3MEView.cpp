// N3MEView.cpp : implementation of the CN3MEView class
//

#include "stdafx.h"
#include "N3ME.h"
#include <N3Base/N3EngTool.h>
#include "N3MEDoc.h"
#include "N3MEView.h"
#include "MainFrm.h"
#include "LyTerrain.h"
#include "MapMng.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CN3MEView

IMPLEMENT_DYNCREATE(CN3MEView, CView)

BEGIN_MESSAGE_MAP(CN3MEView, CView)
//{{AFX_MSG_MAP(CN3MEView)
ON_WM_SETCURSOR()
ON_WM_SIZE()
ON_WM_ERASEBKGND()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CN3MEView construction/destruction

CN3MEView::CN3MEView()
{
	m_pMapMng = nullptr;
}

CN3MEView::~CN3MEView()
{
}

BOOL CN3MEView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CN3MEView drawing

void CN3MEView::OnDraw(CDC* pDC)
{
	CN3MEDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	Render();
}

/////////////////////////////////////////////////////////////////////////////
// CN3MEView diagnostics

#ifdef _DEBUG
void CN3MEView::AssertValid() const
{
	CView::AssertValid();
}

void CN3MEView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CN3MEDoc* CN3MEView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CN3MEDoc)));
	return (CN3MEDoc*) m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CN3MEView message handlers

void CN3MEView::Render()
{
	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	CN3EngTool* pEng = ((CMainFrame*) AfxGetMainWnd())->m_pEng;

	if (m_pMapMng)
		m_pMapMng->Tick();

	CRect rc;
	GetClientRect(&rc);
	pEng->SetViewPort(rc);
	pEng->Clear(0xff606060, rc);

	pEng->s_lpD3DDev->BeginScene();

	//	그리기...
	if (m_pMapMng)
		m_pMapMng->Render();

	CN3Base::s_AlphaMgr.Render(); // Alpha Primitive Manager Rendering.. 안하면 언젠가 뻑난다.
	pEng->s_lpD3DDev->EndScene();
	pEng->Present(m_hWnd);
}

LRESULT CN3MEView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (WM_MOUSEMOVE == message)
	{
		m_CurrMousePos.x = (short) (LOWORD(lParam));
		m_CurrMousePos.y = (short) (HIWORD(lParam));
	}
	if (WM_DESTROY == message)
		m_pMapMng = nullptr;
	if (m_pMapMng)
	{
		MSG msg;
		msg.hwnd    = m_hWnd;
		msg.message = message;
		msg.wParam  = wParam;
		msg.lParam  = lParam;
		if (m_pMapMng->m_SowSeedMng.bActive == TRUE)
		{
			m_pMapMng->m_SowSeedMng.MouseMessage(&msg);
			this->Invalidate(FALSE);
		}
		if (m_pMapMng->MouseMsgFilter(&msg))
			this->Invalidate(FALSE);
	}

	return CView::WindowProc(message, wParam, lParam);
}

void CN3MEView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	m_pMapMng = ((CMainFrame*) AfxGetMainWnd())->GetMapMng();
	CRect rc;
	GetClientRect(&rc);
	this->OnSize(SIZE_RESTORED, rc.Width(), rc.Height());
}

BOOL CN3MEView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (m_pMapMng)
	{
		int CursorMode = m_pMapMng->GetCursorMode();
		if (CursorMode == CM_POS || CursorMode == CM_ROTATE || CursorMode == CM_SCALE)
		{
			SetCursor(LoadCursor(AfxGetResourceHandle(), MAKEINTRESOURCE(IDC_TRANSFORM)));
			return TRUE;
		}
	}

	return CView::OnSetCursor(pWnd, nHitTest, message);
}

void CN3MEView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	if (cx <= 0 || cy <= 0)
		return;

	CWnd* pMainWnd   = AfxGetMainWnd();
	CWnd* pWndParent = nullptr;
	if (pMainWnd)
	{
		pWndParent = pMainWnd->GetParent();
	}

	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	if (pFrm && pWndParent == nullptr && pFrm->m_pEng)
	{
		pFrm->m_pEng->Reset(TRUE, cx, cy, 0);
	}
}

BOOL CN3MEView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;

	return CView::OnEraseBkgnd(pDC);
}

// N3FXEView.cpp : implementation of the CN3FXEView class
//

#include "stdafx.h"
#include "N3FXE.h"

#include "N3FXEDoc.h"
#include "N3FXEView.h"

#include "MainFrm.h"

#include <N3Base/N3FXBundle.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CN3FXEView

IMPLEMENT_DYNCREATE(CN3FXEView, CView)

BEGIN_MESSAGE_MAP(CN3FXEView, CView)
	//{{AFX_MSG_MAP(CN3FXEView)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CN3FXEView construction/destruction

CN3FXEView::CN3FXEView()
{
	// TODO: add construction code here	
}

CN3FXEView::~CN3FXEView()
{
}

BOOL CN3FXEView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CN3FXEView drawing

void CN3FXEView::OnDraw(CDC* pDC)
{
	CN3FXEDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here

	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	pFrm->TickRender();
}


/////////////////////////////////////////////////////////////////////////////
// CN3FXEView diagnostics

#ifdef _DEBUG
void CN3FXEView::AssertValid() const
{
	CView::AssertValid();
}

void CN3FXEView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CN3FXEDoc* CN3FXEView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CN3FXEDoc)));
	return (CN3FXEDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CN3FXEView message handlers

void CN3FXEView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	if(pFrm) pFrm->m_Eng.Reset(TRUE, cx, cy, 0);
}	


LRESULT CN3FXEView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	if(pFrm)
	{
		MSG	msg;
		msg.hwnd = m_hWnd;
		msg.message = message;
		msg.wParam = wParam;
		msg.lParam = lParam;
		if(pFrm->m_Camera.MoveByWindowMessage(&msg))
		{
			this->Invalidate(FALSE);
			return TRUE;
		}
	}
	
	return CView::WindowProc(message, wParam, lParam);
}

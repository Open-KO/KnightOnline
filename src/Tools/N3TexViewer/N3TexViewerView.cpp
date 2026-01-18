// N3TexViewerView.cpp : implementation of the CN3TexViewerView class
//

#include "stdafx.h"
#include "N3TexViewer.h"

#include "MainFrm.h"
#include "N3TexViewerDoc.h"
#include "N3TexViewerView.h"

#include <N3Base/N3EngTool.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CN3TexViewerView

IMPLEMENT_DYNCREATE(CN3TexViewerView, CView)

BEGIN_MESSAGE_MAP(CN3TexViewerView, CView)
//{{AFX_MSG_MAP(CN3TexViewerView)
ON_COMMAND(ID_VIEW_ALPHA, &CN3TexViewerView::OnViewAlpha)
ON_UPDATE_COMMAND_UI(ID_VIEW_ALPHA, &CN3TexViewerView::OnUpdateViewAlpha)
ON_WM_SIZE()
ON_WM_ERASEBKGND()
//}}AFX_MSG_MAP
// Standard printing commands
ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CN3TexViewerView construction/destruction

CN3TexViewerView::CN3TexViewerView()
{
	// TODO: add construction code here
}

CN3TexViewerView::~CN3TexViewerView()
{
}

BOOL CN3TexViewerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CN3TexViewerView drawing

void CN3TexViewerView::OnDraw(CDC* pDC)
{
	CN3TexViewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here
	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	CMenu* pMenu     = pFrm->GetMenu();

	CN3Texture* pTex = nullptr;

	DWORD dwStat     = pMenu->GetMenuState(ID_VIEW_ALPHA, MF_BYCOMMAND);
	if (dwStat & MF_CHECKED)
		pTex = GetDocument()->m_pTexAlpha;
	else
		pTex = GetDocument()->m_pTex;

	pFrm->m_Eng.RenderTexturePreview(pTex, m_hWnd);
}

/////////////////////////////////////////////////////////////////////////////
// CN3TexViewerView printing

BOOL CN3TexViewerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CN3TexViewerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CN3TexViewerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

BOOL CN3TexViewerView::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
			case VK_UP:
			case VK_LEFT:
				if (GetDocument()->HasMultipleTextures())
				{
					GetDocument()->SelectPreviousTexture();
					return TRUE;
				}
				break;

			case VK_DOWN:
			case VK_RIGHT:
				if (GetDocument()->HasMultipleTextures())
				{
					GetDocument()->SelectNextTexture();
					return TRUE;
				}
				break;

			default:
				break;
		}
	}

	return CView::PreTranslateMessage(pMsg);
}

/////////////////////////////////////////////////////////////////////////////
// CN3TexViewerView diagnostics

#ifdef _DEBUG
void CN3TexViewerView::AssertValid() const
{
	CView::AssertValid();
}

void CN3TexViewerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CN3TexViewerDoc* CN3TexViewerView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CN3TexViewerDoc)));
	return (CN3TexViewerDoc*) m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CN3TexViewerView message handlers

void CN3TexViewerView::OnViewAlpha()
{
	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	CMenu* pMenu     = pFrm->GetMenu();
	if (pMenu)
	{
		DWORD dwState = pMenu->GetMenuState(ID_VIEW_ALPHA, MF_BYCOMMAND);
		if (dwState & MF_CHECKED)
			pMenu->CheckMenuItem(ID_VIEW_ALPHA, MF_UNCHECKED | MF_BYCOMMAND);
		else
			pMenu->CheckMenuItem(ID_VIEW_ALPHA, MF_CHECKED | MF_BYCOMMAND);
	}

	this->InvalidateRect(nullptr, FALSE);
}

void CN3TexViewerView::OnUpdateViewAlpha(CCmdUI* pCmdUI)
{
}

void CN3TexViewerView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	pFrm->AdjustWindowSize();
}

void CN3TexViewerView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	pFrm->m_Eng.Reset(TRUE, cx, cy, 0);
}

BOOL CN3TexViewerView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

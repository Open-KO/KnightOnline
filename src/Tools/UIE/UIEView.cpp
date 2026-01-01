// UIEView.cpp : implementation of the CUIEView class
//

#include "stdafx.h"
#include "UIE.h"

#include "UIEDoc.h"
#include "UIEView.h"
#include "MainFrm.h"
#include "PropertyView.h"

#include <N3Base/N3UIButton.h>
#include <N3Base/N3SndMgr.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUIEView

IMPLEMENT_DYNCREATE(CUIEView, CView)

BEGIN_MESSAGE_MAP(CUIEView, CView)
	//{{AFX_MSG_MAP(CUIEView)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_COMMAND(ID_EDIT_MODE_MOVE_AND_RESIZE, OnEditModeMoveAndResize)
	ON_COMMAND(ID_EDIT_MODE_SELECT, OnEditModeSelect)
	ON_COMMAND(ID_VIEW_GRID, OnViewGrid)
	ON_UPDATE_COMMAND_UI(ID_VIEW_GRID, OnUpdateViewGrid)
	ON_WM_CREATE()
	ON_WM_DROPFILES()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUIEView construction/destruction

CUIEView::CUIEView()
{
	m_hAccelTable = nullptr;

	CWinApp* pApp = AfxGetApp();
	ASSERT(pApp);
	m_RegionRectColor = pApp->GetProfileInt("Rect Display", "Region", RGB(0xFF,0x00,0xE4));
	m_MoveRectColor = pApp->GetProfileInt("Rect Display", "Move", RGB(0x00,0xFF,0x06));
	m_ClickRectColor = pApp->GetProfileInt("Rect Display", "Click", RGB(0x00,0xFC,0xFF));

	m_ptOldMouse = CPoint(0,0);
	m_ptOldLBPos = CPoint(0,0);
	m_eSelectedRectType = RT_NONE;
	m_eDragType = DRAGTYPE_NONE;
	m_rcSelectedRect.SetRect(-1,-1,-1,-1);

	m_BkgndColor = pApp->GetProfileInt("Option", "BkGnd Color", 0xff606060);

	m_bViewGrid = FALSE;
}

CUIEView::~CUIEView()
{
	CWinApp* pApp = AfxGetApp();
	ASSERT(pApp);
	pApp->WriteProfileInt("Rect Display", "Region", m_RegionRectColor);
	pApp->WriteProfileInt("Rect Display", "Move", m_MoveRectColor);
	pApp->WriteProfileInt("Rect Display", "Click", m_ClickRectColor);
	pApp->WriteProfileInt("Option", "BkGnd Color", m_BkgndColor);
}

BOOL CUIEView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CUIEView drawing

void CUIEView::OnDraw(CDC* pDC)
{
	CUIEDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (UIEMODE_PREVIEW == m_eMode) return;	// preview일때는 그냥 리턴

	int iUIC = pDoc->GetSelectedUICount();
	for(int i = 0 ; i < iUIC; i++)
	{
		CN3UIBase* pUI = pDoc->GetSelectedUI(i);
		if (nullptr == pUI) continue;
		pDC->SelectStockObject(NULL_BRUSH);

		CN3UIBase* pUIP = pUI->GetParent();
		if(pUIP)
		{
			CRect rcParent = pUIP->GetRegion();
			CPen ParentPen(PS_SOLID, 1, RGB(255,255,0));
			CPen* pOldPen = pDC->SelectObject(&ParentPen);
			pDC->Rectangle(&rcParent);
			pDC->SelectObject(pOldPen);
		}

		if (RT_NONE == m_eSelectedRectType)
		{	// Rect 수정중이 아닐때 각각 Rect 표시

			// region
			RECT rcRegion = pUI->GetRegion();
			if (0 != (rcRegion.right - rcRegion.left) && 0 != (rcRegion.bottom - rcRegion.top))
			{
				CPen RegionPen(PS_SOLID, 1, m_RegionRectColor);
				CPen* pOldPen = pDC->SelectObject(&RegionPen);
				pDC->Rectangle(&rcRegion);
				pDC->SelectObject(pOldPen);
			}

			// move rect
			RECT rcMove = pUI->GetMoveRect();
			if (0 != (rcMove.right - rcMove.left) && 0 != (rcMove.bottom - rcMove.top))
			{
				CPen MovePen(PS_SOLID, 1, m_MoveRectColor);
				CPen* pOldPen = pDC->SelectObject(&MovePen);
				pDC->Rectangle(&rcMove);
				pDC->SelectObject(pOldPen);
			}

			// click rect
			if (UI_TYPE_BUTTON == pUI->UIType())
			{
				CN3UIButton* pBtn = (CN3UIButton*)pUI;
				RECT rcClick = pBtn->GetClickRect();
				if (0 != (rcClick.right - rcClick.left) && 0 != (rcClick.bottom - rcClick.top))
				{
					CPen MovePen(PS_SOLID, 1, m_ClickRectColor);
					CPen* pOldPen = pDC->SelectObject(&MovePen);
					pDC->Rectangle(&rcClick);
					pDC->SelectObject(pOldPen);
				}
			}
		}
	}
	
	if(RT_NONE != m_eSelectedRectType)// Rect 수정중일때 각각 Rect 표시
	{
		CPen SelPen(PS_DOT, 1, RGB(0,0,0));
		CPen* pOldPen = pDC->SelectObject(&SelPen);
		pDC->Rectangle(&m_rcSelectedRect);
		pDC->SelectObject(pOldPen);
	}

	if(m_bViewGrid) // 그리드 보기..
	{
		CRect rc;
		CPen pen, penThick;
		pen.CreatePen(PS_SOLID, 1, RGB(128, 128, 128));
		penThick.CreatePen(PS_SOLID, 1, RGB(255,255,255));
		CPen* pPenOld = pDC->SelectObject(&pen);

		this->GetClientRect(&rc);
		for(int y = rc.top; y < rc.bottom; y += 10)
		{
			if(y%100)
				pDC->SelectObject(&pen);
			else
				pDC->SelectObject(&penThick);

			pDC->MoveTo(rc.left, y);
			pDC->LineTo(rc.right, y);
		}
		for(int x = rc.left; x < rc.right; x += 10)
		{
			if(x%100)
				pDC->SelectObject(&pen);
			else
				pDC->SelectObject(&penThick);

			pDC->MoveTo(x, rc.top);
			pDC->LineTo(x, rc.bottom);
		}

		pDC->SelectObject(pPenOld);
	}

	if (-1 == m_rcSelectedRect.left) return;
}

/////////////////////////////////////////////////////////////////////////////
// CUIEView diagnostics

#ifdef _DEBUG
void CUIEView::AssertValid() const
{
	CView::AssertValid();
}

void CUIEView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CUIEDoc* CUIEView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CUIEDoc)));
	return (CUIEDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CUIEView message handlers

void CUIEView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	if(pFrm)
	{
		static bool bInitMainWnd = false;
		if(bInitMainWnd && pFrm->m_Eng.Reset(TRUE, cx, cy, 0))
		{
			Invalidate();
		}
		bInitMainWnd = true;
	}
	
}
BOOL CUIEView::OnEraseBkgnd(CDC* pDC) 
{
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	CN3EngTool* pEng = &(pFrm->m_Eng);

	pEng->s_SndMgr.Tick(); // Sound Tick...

	CRect rc;
	GetClientRect(&rc);
	pEng->SetViewPort(rc);
	pEng->Clear(m_BkgndColor, rc);

	pEng->s_lpD3DDev->BeginScene();

	//	그리기...
	switch(m_eMode)
	{
	case UIEMODE_PREVIEW:
		RenderPreview();
		break;
	case UIEMODE_EDIT:
		RenderEditview();
		break;
	}

	pEng->s_lpD3DDev->EndScene();
	pEng->Present(m_hWnd);

	return TRUE;
	//return CView::OnEraseBkgnd(pDC);
}

void CUIEView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	SetMode(UIEMODE_EDIT);

	m_hAccelTable = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_ACCEL_VIEW));
}

void CUIEView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CUIEDoc* pDoc = GetDocument();
	ASSERT(pDoc);
	if (UIEMODE_PREVIEW == m_eMode)
	{
		CN3UIBase* pRootUI = pDoc->GetRootUI();
		if (pRootUI) {pRootUI->MouseProc(UI_MOUSE_LBCLICK,point, m_ptOldMouse);Invalidate();}
	}
	else if (UIEMODE_EDIT == m_eMode)
	{
		CN3UIBase* pSelectedUI = pDoc->GetSelectedUI();
		if (RT_NONE != m_eSelectedRectType && pSelectedUI)
		{	// 지정된 사각형 변형일때
			if (-1000 != m_rcSelectedRect.left) m_eDragType = CheckDragType(m_rcSelectedRect, point);
			else m_eDragType = DRAGTYPE_NONE;
		}

		if (DRAGTYPE_NONE == m_eDragType)
		{	// m_RootUI의 자식중에서 point에 위치한 ui 선택하기
			if(!(nFlags & MK_CONTROL)) pDoc->SetSelectedUI(nullptr); // 컨트롤 키를 누르지 않으면 멀티 셀렉트 해제후..
			
			CN3UIBase* pRootUI = GetDocument()->GetRootUI();
			CN3UIBase* pUISelected = nullptr;
			for(UIListItor itor = pRootUI->m_Children.begin(); pRootUI->m_Children.end() != itor; ++itor)
			{
				CN3UIBase* pUI = (*itor);
				pUISelected = this->Pick(point, pUI);
				if(pUISelected) break;
			}

			if(nullptr == pUISelected && pRootUI->IsIn(point.x, point.y)) pUISelected = pRootUI; // 암것도 못찍으면 루트UI를 찍어본다.
			if(pUISelected) pDoc->SetSelectedUI(pUISelected);
		}
		else SetCapture();
		UpdateStatusBarText();
	}
	m_ptOldMouse = point;
	m_ptOldLBPos = point;
	
	CView::OnLButtonDown(nFlags, point);
}

void CUIEView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CUIEDoc* pDoc = GetDocument();
	ASSERT(pDoc);
	if (UIEMODE_PREVIEW == m_eMode)
	{
		CN3UIBase* pRootUI = pDoc->GetRootUI();
		if (pRootUI) {pRootUI->MouseProc(UI_MOUSE_LBCLICKED,point, m_ptOldMouse);Invalidate();}
	}
	else if (UIEMODE_EDIT == m_eMode)
	{
		CPoint ptDiff = point - m_ptOldMouse;
		switch(m_eDragType)
		{
		case DRAGTYPE_NONE:			break;
		case DRAGTYPE_MOVE:			m_rcSelectedRect.OffsetRect(ptDiff);		break;
		case DRAGTYPE_LEFT:			m_rcSelectedRect.left += ptDiff.x;			break;
		case DRAGTYPE_RIGHT:		m_rcSelectedRect.right += ptDiff.x;			break;
		case DRAGTYPE_TOP:			m_rcSelectedRect.top += ptDiff.y;			break;
		case DRAGTYPE_BOTTOM:		m_rcSelectedRect.bottom += ptDiff.y;		break;
		case DRAGTYPE_LEFTTOP:		m_rcSelectedRect.left += ptDiff.x;	m_rcSelectedRect.top += ptDiff.y;	break;
		case DRAGTYPE_RIGHTTOP:		m_rcSelectedRect.right += ptDiff.x;	m_rcSelectedRect.top += ptDiff.y;	break;
		case DRAGTYPE_LEFTBOTTOM:	m_rcSelectedRect.left += ptDiff.x;	m_rcSelectedRect.bottom += ptDiff.y;	break;
		case DRAGTYPE_RIGHTBOTTOM:	m_rcSelectedRect.right += ptDiff.x;	m_rcSelectedRect.bottom += ptDiff.y;	break;
		}
		if (DRAGTYPE_NONE != m_eDragType)
		{
			UpdateUIInfo_SelectedRect();
			m_eDragType = DRAGTYPE_NONE;
		}
		UpdateStatusBarText();
		Invalidate();
	}

	m_ptOldMouse = point;

	ReleaseCapture();
	
	CView::OnLButtonUp(nFlags, point);
}

void CUIEView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CUIEDoc* pDoc = GetDocument();
	ASSERT(pDoc);
	if (UIEMODE_PREVIEW == m_eMode)
	{
		CN3UIBase* pRootUI = pDoc->GetRootUI();
		if (pRootUI)
		{
			if (nFlags & MK_LBUTTON) pRootUI->MouseProc(UI_MOUSE_LBDOWN,point, m_ptOldMouse);
			else  pRootUI->MouseProc(0,point, m_ptOldMouse);
			Invalidate();
		}
	}
	else if (UIEMODE_EDIT == m_eMode)
	{
		CPoint ptDiff = point - m_ptOldMouse;
		switch(m_eDragType)
		{
		case DRAGTYPE_NONE:			break;
		case DRAGTYPE_MOVE:			m_rcSelectedRect.OffsetRect(ptDiff);		break;
		case DRAGTYPE_LEFT:			m_rcSelectedRect.left += ptDiff.x;			break;
		case DRAGTYPE_RIGHT:		m_rcSelectedRect.right += ptDiff.x;			break;
		case DRAGTYPE_TOP:			m_rcSelectedRect.top += ptDiff.y;			break;
		case DRAGTYPE_BOTTOM:		m_rcSelectedRect.bottom += ptDiff.y;		break;
		case DRAGTYPE_LEFTTOP:		m_rcSelectedRect.left += ptDiff.x;	m_rcSelectedRect.top += ptDiff.y;	break;
		case DRAGTYPE_RIGHTTOP:		m_rcSelectedRect.right += ptDiff.x;	m_rcSelectedRect.top += ptDiff.y;	break;
		case DRAGTYPE_LEFTBOTTOM:	m_rcSelectedRect.left += ptDiff.x;	m_rcSelectedRect.bottom += ptDiff.y;	break;
		case DRAGTYPE_RIGHTBOTTOM:	m_rcSelectedRect.right += ptDiff.x;	m_rcSelectedRect.bottom += ptDiff.y;	break;
		}
		if (DRAGTYPE_NONE != m_eDragType) {Invalidate();}
		UpdateStatusBarText();
	}
	m_ptOldMouse = point;
	
	CView::OnMouseMove(nFlags, point);
}

BOOL CUIEView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if (UIEMODE_EDIT == m_eMode && RT_NONE != m_eSelectedRectType)
	{
		char* pszRsrcID = nullptr;
		eDRAGTYPE		eDT = m_eDragType;
		if (DRAGTYPE_NONE == m_eDragType)	// 드레그 중이 아니면 cursor의 위치를 얻어서 테스트하기
		{
			CPoint pt;
			if (GetCursorPos(&pt))
			{
				ScreenToClient(&pt);
				eDT = CheckDragType(m_rcSelectedRect, pt);
			}
		}

		switch(eDT)
		{
		case DRAGTYPE_NONE:			break;
		case DRAGTYPE_MOVE:
			pszRsrcID=IDC_SIZEALL;		break;
		case DRAGTYPE_LEFT:
		case DRAGTYPE_RIGHT:
			pszRsrcID=IDC_SIZEWE;			break;
		case DRAGTYPE_TOP:
		case DRAGTYPE_BOTTOM:
			pszRsrcID=IDC_SIZENS;			break;
		case DRAGTYPE_LEFTTOP:
		case DRAGTYPE_RIGHTBOTTOM:
			pszRsrcID=IDC_SIZENWSE;			break;
		case DRAGTYPE_RIGHTTOP:
		case DRAGTYPE_LEFTBOTTOM:
			pszRsrcID=IDC_SIZENESW;			break;
		}

		if (nullptr != pszRsrcID)
		{
			HCURSOR hCur = LoadCursor(nullptr, pszRsrcID);
			if (hCur) {	SetCursor(hCur); return TRUE;}
		}
	}
	
	return CView::OnSetCursor(pWnd, nHitTest, message);
}

// mode 바꾸기
void CUIEView::SetMode(eUIE_MODE eMode)
{
	m_eMode = eMode;
	UpdateStatusBarText();
	Invalidate();
}

// 미리보기 render
void CUIEView::RenderPreview()
{
	CUIEDoc* pDoc = GetDocument();
	if (nullptr == pDoc) return;
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	CN3EngTool* pEng = &(pFrm->m_Eng);
	LPDIRECT3DDEVICE9 lpD3DDev = pEng->s_lpD3DDev;

	// back up old state
	DWORD dwZEnable, dwAlphaBlend, dwSrcBlend, dwDestBlend, dwFog;
	lpD3DDev->GetRenderState(D3DRS_ZENABLE, &dwZEnable);
	lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlphaBlend);
	lpD3DDev->GetRenderState(D3DRS_SRCBLEND, &dwSrcBlend);
	lpD3DDev->GetRenderState(D3DRS_DESTBLEND, &dwDestBlend);
	lpD3DDev->GetRenderState(D3DRS_FOGENABLE, &dwFog);
	DWORD dwMagFilter, dwMinFilter, dwMipFilter;
	lpD3DDev->GetSamplerState(0, D3DSAMP_MAGFILTER, &dwMagFilter);//lpD3DDev->GetTextureStageState(0, D3DTSS_MAGFILTER,   &dwMagFilter);
	lpD3DDev->GetSamplerState(0, D3DSAMP_MINFILTER, &dwMinFilter);//lpD3DDev->GetTextureStageState(0, D3DTSS_MINFILTER,   &dwMinFilter);
	lpD3DDev->GetSamplerState(0, D3DSAMP_MIPFILTER, &dwMipFilter);//lpD3DDev->GetTextureStageState(0, D3DTSS_MIPFILTER,   &dwMipFilter);

	// set state
	if (D3DZB_FALSE != dwZEnable) lpD3DDev->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	if (TRUE != dwAlphaBlend) lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	if (D3DBLEND_SRCALPHA != dwSrcBlend) lpD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	if (D3DBLEND_INVSRCALPHA != dwDestBlend) lpD3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	if (FALSE != dwFog) lpD3DDev->SetRenderState(D3DRS_FOGENABLE   , FALSE);	// 2d도 fog를 먹는다 ㅡ.ㅡ;
	if (D3DTEXF_POINT != dwMagFilter) lpD3DDev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT); //lpD3DDev->SetTextureStageState(0, D3DTSS_MAGFILTER,   D3DTEXF_POINT);
	if (D3DTEXF_POINT != dwMinFilter) lpD3DDev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT); //lpD3DDev->SetTextureStageState(0, D3DTSS_MINFILTER,   D3DTEXF_POINT);
	if (D3DTEXF_NONE != dwMipFilter) lpD3DDev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE); //lpD3DDev->SetTextureStageState(0, D3DTSS_MIPFILTER,   D3DTEXF_NONE);

	// render
	CN3UIBase* pRootUI = pDoc->GetRootUI();
	if (pRootUI) pRootUI->Render();

	// restore
	if (D3DZB_FALSE != dwZEnable) lpD3DDev->SetRenderState(D3DRS_ZENABLE, dwZEnable);
	if (TRUE != dwAlphaBlend) lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlphaBlend);
	if (D3DBLEND_SRCALPHA != dwSrcBlend) lpD3DDev->SetRenderState(D3DRS_SRCBLEND, dwSrcBlend);
	if (D3DBLEND_INVSRCALPHA != dwDestBlend) lpD3DDev->SetRenderState(D3DRS_DESTBLEND, dwDestBlend);
	if (FALSE != dwFog) lpD3DDev->SetRenderState(D3DRS_FOGENABLE   , dwFog);
	if (D3DTEXF_POINT != dwMagFilter) lpD3DDev->SetSamplerState(0, D3DSAMP_MAGFILTER, dwMagFilter);//lpD3DDev->SetTextureStageState(0, D3DTSS_MAGFILTER,   dwMagFilter);
	if (D3DTEXF_POINT != dwMinFilter) lpD3DDev->SetSamplerState(0, D3DSAMP_MINFILTER, dwMinFilter);//lpD3DDev->SetTextureStageState(0, D3DTSS_MINFILTER,   dwMinFilter);
	if (D3DTEXF_NONE != dwMipFilter) lpD3DDev->SetSamplerState(0, D3DSAMP_MIPFILTER, dwMipFilter);//lpD3DDev->SetTextureStageState(0, D3DTSS_MIPFILTER,   dwMipFilter);
}

// Edit render
void CUIEView::RenderEditview()
{
	CUIEDoc* pDoc = GetDocument();
	if (nullptr == pDoc) return;
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	CN3EngTool* pEng = &(pFrm->m_Eng);
	LPDIRECT3DDEVICE9 lpD3DDev = pEng->s_lpD3DDev;

	// back up old state
	DWORD dwZEnable, dwAlphaBlend, dwSrcBlend, dwDestBlend, dwFog;
	lpD3DDev->GetRenderState(D3DRS_ZENABLE, &dwZEnable);
	lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlphaBlend);
	lpD3DDev->GetRenderState(D3DRS_SRCBLEND, &dwSrcBlend);
	lpD3DDev->GetRenderState(D3DRS_DESTBLEND, &dwDestBlend);
	lpD3DDev->GetRenderState(D3DRS_FOGENABLE, &dwFog);
	DWORD dwMagFilter, dwMinFilter, dwMipFilter;
	lpD3DDev->GetSamplerState(0, D3DSAMP_MAGFILTER, &dwMagFilter); //lpD3DDev->GetTextureStageState(0, D3DTSS_MAGFILTER,   &dwMagFilter);
	lpD3DDev->GetSamplerState(0, D3DSAMP_MINFILTER, &dwMinFilter); //lpD3DDev->GetTextureStageState(0, D3DTSS_MINFILTER,   &dwMinFilter);
	lpD3DDev->GetSamplerState(0, D3DSAMP_MIPFILTER, &dwMipFilter); //lpD3DDev->GetTextureStageState(0, D3DTSS_MIPFILTER,   &dwMipFilter);

	// set state
	if (D3DZB_FALSE != dwZEnable) lpD3DDev->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	if (TRUE != dwAlphaBlend) lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	if (D3DBLEND_SRCALPHA != dwSrcBlend) lpD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	if (D3DBLEND_INVSRCALPHA != dwDestBlend) lpD3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	if (FALSE != dwFog) lpD3DDev->SetRenderState(D3DRS_FOGENABLE   , FALSE);	// 2d도 fog를 먹는다 ㅡ.ㅡ;
	if (D3DTEXF_POINT != dwMagFilter) lpD3DDev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT); //lpD3DDev->SetTextureStageState(0, D3DTSS_MAGFILTER,   D3DTEXF_POINT);
	if (D3DTEXF_POINT != dwMinFilter ) lpD3DDev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT); //lpD3DDev->SetTextureStageState(0, D3DTSS_MINFILTER,   D3DTEXF_POINT);
	if (D3DTEXF_NONE != dwMipFilter ) lpD3DDev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE); //lpD3DDev->SetTextureStageState(0, D3DTSS_MIPFILTER,   D3DTEXF_NONE);

	// render
	CN3UIBase* pRootUI = pDoc->GetRootUI();
	if (pRootUI) pRootUI->Render();
	
	int iUIC = pDoc->GetSelectedUICount();
	for(int i = 0; i < iUIC; i++)
	{
		CN3UIBase* pUI = pDoc->GetSelectedUI(i);
		if (pUI) pUI->Render();	// 선택된 UI한번 더 그리기(뒤에 가릴 수도 있으니까 한번 더 그린다. button같은 경우 특히)
	}

	// restore
	if (D3DZB_FALSE != dwZEnable) lpD3DDev->SetRenderState(D3DRS_ZENABLE, dwZEnable);
	if (TRUE != dwAlphaBlend) lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlphaBlend);
	if (D3DBLEND_SRCALPHA != dwSrcBlend) lpD3DDev->SetRenderState(D3DRS_SRCBLEND, dwSrcBlend);
	if (D3DBLEND_INVSRCALPHA != dwDestBlend) lpD3DDev->SetRenderState(D3DRS_DESTBLEND, dwDestBlend);
	if (FALSE != dwFog) lpD3DDev->SetRenderState(D3DRS_FOGENABLE   , dwFog);
	if (D3DTEXF_POINT != dwMagFilter ) lpD3DDev->SetSamplerState(0, D3DSAMP_MAGFILTER, dwMagFilter);//lpD3DDev->SetTextureStageState(0, D3DTSS_MAGFILTER,   dwMagFilter);
	if (D3DTEXF_POINT != dwMinFilter ) lpD3DDev->SetSamplerState(0, D3DSAMP_MINFILTER, dwMinFilter);//lpD3DDev->SetTextureStageState(0, D3DTSS_MINFILTER,   dwMinFilter);
	if (D3DTEXF_NONE != dwMipFilter ) lpD3DDev->SetSamplerState(0, D3DSAMP_MIPFILTER, dwMipFilter);//lpD3DDev->SetTextureStageState(0, D3DTSS_MIPFILTER,   dwMipFilter);
}

void CUIEView::SelectRectType(eRECTTYPE eRectType)
{
	m_eSelectedRectType = eRectType;
	m_rcSelectedRect.SetRect(-1000,-1000,-1000,-1000);
	CN3UIBase* pSelectedUI = GetDocument()->GetSelectedUI();

	// 선택된 UI가 없으면 RT_NONE으로 만들고 리턴
	if (nullptr == pSelectedUI){	m_eSelectedRectType = RT_NONE;	Invalidate(); return;}

	// 선택된 UI에서 RectType에 맞는 사각형 가져오기
	switch(m_eSelectedRectType)
	{
	case RT_NONE:
	case RT_REGION:	m_rcSelectedRect = pSelectedUI->GetRegion();		break;
	case RT_MOVE:	m_rcSelectedRect = pSelectedUI->GetMoveRect();		break;
	case RT_CLICK:	
		if (UI_TYPE_BUTTON == pSelectedUI->UIType())
			m_rcSelectedRect = ((CN3UIButton*)pSelectedUI)->GetClickRect();
		else m_eSelectedRectType = RT_NONE;
		break;
	default:
		m_eSelectedRectType = RT_NONE;
		break;
	}

	if(m_rcSelectedRect.Width() <= 0) m_rcSelectedRect.right = m_rcSelectedRect.left + 20;
	if(m_rcSelectedRect.Height() <= 0) m_rcSelectedRect.bottom = m_rcSelectedRect.top + 20;

	UpdateStatusBarText();
	SetFocus();
	Invalidate();
}

CUIEView::eDRAGTYPE CUIEView::CheckDragType(CRect rcSel, CPoint point)
{
	CPoint pt;
	CRect rcCheck;
	const int W = 4;
	pt = rcSel.TopLeft();
	rcCheck.SetRect(pt.x-W, pt.y-W, pt.x+W, pt.y+W);
	if (rcCheck.PtInRect(point)) return DRAGTYPE_LEFTTOP;
	pt = rcSel.BottomRight();
	rcCheck.SetRect(pt.x-W, pt.y-W, pt.x+W, pt.y+W);
	if (rcCheck.PtInRect(point)) return DRAGTYPE_RIGHTBOTTOM;
	pt = CPoint(rcSel.right, rcSel.top);
	rcCheck.SetRect(pt.x-W, pt.y-W, pt.x+W, pt.y+W);
	if (rcCheck.PtInRect(point)) return DRAGTYPE_RIGHTTOP;
	pt = CPoint(rcSel.left, rcSel.bottom);
	rcCheck.SetRect(pt.x-W, pt.y-W, pt.x+W, pt.y+W);
	if (rcCheck.PtInRect(point)) return DRAGTYPE_LEFTBOTTOM;

	rcCheck.SetRect(rcSel.left-W, rcSel.top, rcSel.left+W, rcSel.bottom);
	if (rcCheck.PtInRect(point)) return DRAGTYPE_LEFT;
	rcCheck.SetRect(rcSel.right-W, rcSel.top, rcSel.right+W, rcSel.bottom);
	if (rcCheck.PtInRect(point)) return DRAGTYPE_RIGHT;
	rcCheck.SetRect(rcSel.left, rcSel.top-W, rcSel.right, rcSel.top+W);
	if (rcCheck.PtInRect(point)) return DRAGTYPE_TOP;
	rcCheck.SetRect(rcSel.left, rcSel.bottom-W, rcSel.right, rcSel.bottom+W);
	if (rcCheck.PtInRect(point)) return DRAGTYPE_BOTTOM;

	if (rcSel.PtInRect(point)) return DRAGTYPE_MOVE;
	return DRAGTYPE_NONE;
}

void CUIEView::UpdateStatusBarText()
{
	CString strText;
	if (UIEMODE_PREVIEW == m_eMode)
	{
		strText = _T("Preview mode");
	}
	else if (UIEMODE_EDIT == m_eMode) 
	{
		strText = _T("Edit mode");
		CString strRect = _T("");
		switch(m_eSelectedRectType)
		{
		case RT_NONE:			break;
		case RT_REGION:			strRect = _T("Region");			break;
		case RT_MOVE:			strRect = _T("Move Rect");			break;
		case RT_CLICK:			strRect = _T("Click Rect");			break;
		}
		if (-1000 != m_rcSelectedRect.left && RT_NONE != m_eSelectedRectType)
		{
			strText.Format("%s  (left=%04d top=%04d right=%04d bottom=%04d)  (Width=%04d Height=%04d)",
				strRect,
				m_rcSelectedRect.left, m_rcSelectedRect.top,
				m_rcSelectedRect.right, m_rcSelectedRect.bottom,
				m_rcSelectedRect.Width(), m_rcSelectedRect.Height());
		}
		else
		{
			CPoint pt;
			GetCursorPos(&pt);
			ScreenToClient(&pt);
			strText.Format("x=%04d\ty=%04d", pt.x, pt.y);
		}
	}

	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	ASSERT(pFrm);
	pFrm->SetStatusText(strText);
}

BOOL CUIEView::ResizeSelectedRect(int dx, int dy)
{
	CN3UIBase* pSelectedUI = GetDocument()->GetSelectedUI();
	if (pSelectedUI && UIEMODE_EDIT == m_eMode && RT_NONE != m_eSelectedRectType && -1000 != m_rcSelectedRect.left)
	{
		m_rcSelectedRect.right += dx;
		m_rcSelectedRect.bottom += dy;

		UpdateUIInfo_SelectedRect();
		return TRUE;
	}
	return FALSE;
}

BOOL CUIEView::MoveSelectedRect(int dx, int dy)
{
	CN3UIBase* pSelectedUI = GetDocument()->GetSelectedUI();
	if (pSelectedUI && UIEMODE_EDIT == m_eMode && RT_NONE != m_eSelectedRectType && -1000 != m_rcSelectedRect.left)
	{
		m_rcSelectedRect.OffsetRect(dx, dy);

		UpdateUIInfo_SelectedRect();
		return TRUE;
	}
	return FALSE;
}

// selected rect정보를 토대로 UI 정보를 갱신하기
void CUIEView::UpdateUIInfo_SelectedRect()
{
	CUIEDoc* pDoc = this->GetDocument();
	CN3UIBase* pSelectedUI = pDoc->GetSelectedUI(pDoc->GetSelectedUICount()-1);
	if (pSelectedUI && UIEMODE_EDIT == m_eMode && RT_NONE != m_eSelectedRectType && -1000 != m_rcSelectedRect.left)
	{
		switch(m_eSelectedRectType)
		{
		case RT_REGION:
			{
				int iUIC = pDoc->GetSelectedUICount();
				for(int i = 0; i < iUIC; i++)
				{
					pSelectedUI = pDoc->GetSelectedUI(i);
					if(nullptr == pSelectedUI) continue;

					if(DRAGTYPE_MOVE == m_eDragType)
					{
						CPoint ptMouse;
						GetCursorPos(&ptMouse);
						ScreenToClient(&ptMouse);
						CPoint ptOffset = ptMouse - m_ptOldLBPos;
						pSelectedUI->MoveOffset(ptOffset.x, ptOffset.y);
					}
					else if(i == 0 && m_eDragType >= DRAGTYPE_LEFT && m_eDragType <= DRAGTYPE_RIGHTBOTTOM) // 마지막에 선택한 UI
					{
						pSelectedUI->SetRegion(m_rcSelectedRect);
						pSelectedUI->SetSize(m_rcSelectedRect.Width(), m_rcSelectedRect.Height());
					}
					else
					{
						pSelectedUI->SetRegion(m_rcSelectedRect);
						pSelectedUI->SetSize(m_rcSelectedRect.Width(), m_rcSelectedRect.Height());
					}

					if(pSelectedUI->GetParent()) // 부모 UI 가 있으면..
					{
						pSelectedUI->GetParent()->ResizeAutomaticalyByChild(); // 자동으로 영역 다시 계산..
					}
				}

			}
			break;
		case RT_MOVE:
			pSelectedUI->SetMoveRect(m_rcSelectedRect);
			break;
		case RT_CLICK:
			if (UI_TYPE_BUTTON == pSelectedUI->UIType()) ((CN3UIButton*)pSelectedUI)->SetClickRect(m_rcSelectedRect);
			break;
		}		
		UpdateStatusBarText();
		CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
		pFrm->GetPropertyView()->UpdateUIBaseInfo();
		Invalidate();
	}
}


void CUIEView::OnSetFocus(CWnd* pOldWnd) 
{
	CView::OnSetFocus(pOldWnd);
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	pFrm->EnableAccelerator(TRUE);
}

void CUIEView::OnKillFocus(CWnd* pNewWnd) 
{
	CView::OnKillFocus(pNewWnd);
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	pFrm->EnableAccelerator(FALSE);
}

void CUIEView::ChangeBkgndColor()
{
	CColorDialog dlg;
	if (IDCANCEL == dlg.DoModal()) return;
	COLORREF clr = dlg.GetColor();

	m_BkgndColor = 0xff000000 | ((clr&0x00ff0000)>>16) | (clr&0x0000ff00) | ((clr&0x000000ff)<<16);

	CWinApp* pApp = AfxGetApp();	ASSERT(pApp);
	pApp->WriteProfileInt("Option", "BkGnd Color", m_BkgndColor);
}

CN3UIBase* CUIEView::Pick(const POINT &point, CN3UIBase* pUI)
{
	if(nullptr == pUI) return nullptr;
	if(pUI->IsVisible() == false) return nullptr;

	for(UIListItor itor = pUI->m_Children.begin(); pUI->m_Children.end() != itor; ++itor)
	{
		CN3UIBase* pChild = (*itor);
		CN3UIBase* pUI2 = this->Pick(point, pChild);
		if(pUI2)
		{
			if(pUI2->GetParent() && pUI->GetParent()->UIType() != UI_TYPE_BUTTON)
				return pUI2;
		}
	}
	
	if(pUI->IsIn(point.x, point.y))
	{
		if(	pUI->GetParent() && 
			pUI->GetParent()->UIType() != UI_TYPE_BUTTON &&
			pUI->GetParent()->UIType() != UI_TYPE_TRACKBAR &&
			pUI->GetParent()->UIType() != UI_TYPE_SCROLLBAR &&
			pUI->GetParent()->UIType() != UI_TYPE_PROGRESS )
		{
			return pUI;
		}
	}
	return nullptr;
}

void CUIEView::OnEditModeMoveAndResize() 
{
	this->SelectRectType(CUIEView::RT_REGION);
	this->InvalidateRect(nullptr, FALSE);
}

void CUIEView::OnEditModeSelect() 
{
	this->SelectRectType(CUIEView::RT_NONE);
	this->InvalidateRect(nullptr, FALSE);
}

BOOL CUIEView::PreTranslateMessage(MSG* pMsg) 
{
	if (m_hAccelTable) TranslateAccelerator(m_hWnd, m_hAccelTable, pMsg);
	return CView::PreTranslateMessage(pMsg);
}

void CUIEView::OnViewGrid() 
{
	m_bViewGrid = !m_bViewGrid; // 그리드 보기..
	this->InvalidateRect(nullptr, FALSE);
}

void CUIEView::OnUpdateViewGrid(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bViewGrid);
}

int CUIEView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	DragAcceptFiles();
	return 0;
}

void CUIEView::OnDropFiles(HDROP hDropInfo) 
{
	CUIEDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	char szFile[MAX_PATH];
	char* szExt = nullptr;
	UINT uiFiles;

	uiFiles = DragQueryFile(hDropInfo,0xFFFF,nullptr,0);

	::DragQueryFile(hDropInfo, 0, szFile, MAX_PATH - 1);
	::DragFinish(hDropInfo);

	size_t len = strlen(szFile);
	szExt = (szFile + len) - 3;

	if (len >= 3
		&& 0 == lstrcmpi(szExt, "uif") && pDoc)
	{
		pDoc->OnOpenDocument(szFile);
		pDoc->UpdateAllViews(nullptr);
	}

	CView::OnDropFiles(hDropInfo);
}

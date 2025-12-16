// SkyViewerView.cpp : implementation of the CSkyViewerView class
//

#include "stdafx.h"
#include "SkyViewer.h"

#include "MainFrm.h"
#include "SkyViewerDoc.h"
#include "SkyViewerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSkyViewerView

IMPLEMENT_DYNCREATE(CSkyViewerView, CView)

BEGIN_MESSAGE_MAP(CSkyViewerView, CView)
	//{{AFX_MSG_MAP(CSkyViewerView)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSkyViewerView construction/destruction

CSkyViewerView::CSkyViewerView()
{
}

CSkyViewerView::~CSkyViewerView()
{
}

BOOL CSkyViewerView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CSkyViewerView drawing

void CSkyViewerView::OnDraw(CDC* pDC)
{
	CSkyViewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();

	pFrm->m_Camera.Tick();
	pFrm->m_Camera.Apply();
	
	pDoc->m_Sky.Tick();
	pDoc->m_Weather.Tick();

	for(int i = 0; i < MAX_GAME_LIGHT; i++)
	{
		pFrm->m_Lights[i].m_Data.Diffuse = _D3DCOLOR_To_D3DCOLORVALUE(pDoc->m_Sky.GetLightDiffuseColor(i));
		pFrm->m_Lights[i].m_Data.Ambient = _D3DCOLOR_To_D3DCOLORVALUE(pDoc->m_Sky.GetLightAmbientColor(i));

		if(0 == i)
		{
			float fAngle = pDoc->m_Sky.GetSunAngleByRadin();
			__Matrix44 mtxRot; mtxRot.RotationZ(fAngle);
			__Vector3 vDir(-1,0,1);
			vDir *= mtxRot;
			vDir.Normalize();
			pFrm->m_Lights[i].m_Data.Direction = { vDir.x, vDir.y, vDir.z };
		}
		else if(1 == i)
		{
//			__Vector3 vDir = CN3Base::s_CameraData.vAt - CN3Base::s_CameraData.vEye;
			__Vector3 vDir(2,-3, 2);
			vDir.Normalize();
			pFrm->m_Lights[i].m_Data.Direction = { vDir.x, vDir.y, vDir.z };
		}
		else if(2 == i)
		{
			pFrm->m_Lights[i].PosSet(CN3Base::s_CameraData.vEye);
		}

		pFrm->m_Lights[i].Tick();
		pFrm->m_Lights[i].Apply();
	}

	pFrm->m_Camera.m_FogColor = pDoc->m_Sky.GetFogColor();
	pFrm->m_Eng.Clear(pDoc->m_Sky.GetSkyColor());
	pFrm->m_Eng.s_lpD3DDev->BeginScene();

	__Matrix44 mtxScale; mtxScale.Scale(4.0f, 4.0f, 4.0f);
//	pFrm->m_Eng.RenderGrid(mtxScale);
	pFrm->m_Eng.RenderAxis();

	pDoc->m_Sky.Render();
	pFrm->m_ObjectBundle.Tick();
	pFrm->m_ObjectBundle.Render();
	pDoc->m_Weather.Render();
	
	CN3Base::s_AlphaMgr.Render(); // Draw Alpha primitive...
	pFrm->m_Eng.s_lpD3DDev->EndScene();
	pFrm->m_Eng.Present(m_hWnd);

	// 프레임 표시
	CString szInfo0;
	szInfo0.Format("FPS : %6.2f", pFrm->m_Eng.s_fFrmPerSec);
	pDC->TextOut(0, 0, szInfo0);
}

/////////////////////////////////////////////////////////////////////////////
// CSkyViewerView printing

BOOL CSkyViewerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CSkyViewerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

void CSkyViewerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

/////////////////////////////////////////////////////////////////////////////
// CSkyViewerView diagnostics

#ifdef _DEBUG
void CSkyViewerView::AssertValid() const
{
	CView::AssertValid();
}

void CSkyViewerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSkyViewerDoc* CSkyViewerView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSkyViewerDoc)));
	return (CSkyViewerDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSkyViewerView message handlers

LRESULT CSkyViewerView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(m_pDocument)
	{
		CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
		if(pFrm)
		{
			MSG	msg;	msg.hwnd = m_hWnd;	msg.message = message;	msg.wParam = wParam;	msg.lParam = lParam;
			if(pFrm->m_Camera.MoveByWindowMessage(&msg))
			{
				this->Invalidate(FALSE);
				return TRUE;
			}
		}
	}
	
	return CView::WindowProc(message, wParam, lParam);
}

BOOL CSkyViewerView::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
	
	return CView::OnEraseBkgnd(pDC);
}

void CSkyViewerView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	if(pFrm && pFrm->m_Eng.Reset(TRUE, cx, cy, 0))
		this->InvalidateRect(nullptr, FALSE);
}

void CSkyViewerView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if(!::_IsKeyDown(VK_MENU))
	{
		CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
		__Vector3 vCol(0,0,0);
		if(pFrm->m_ObjectBundle.CheckCollisionPrecisely(true, point.x, point.y, &vCol) >= 0)
		{
			__Vector3 vEye = pFrm->m_Camera.EyePos();
			__Vector3 vAt = pFrm->m_Camera.AtPos();
			__Vector3 vDir = vEye - vAt;

			float fDist = vDir.Magnitude();
			vDir.Normalize();

			pFrm->m_Camera.AtPosSet(vCol);
			pFrm->m_Camera.EyePosSet(vCol + (vDir * fDist));

			this->InvalidateRect(nullptr, FALSE);
		}
	}
		
	CView::OnLButtonDown(nFlags, point);
}

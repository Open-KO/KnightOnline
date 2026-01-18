// N3ViewerView.cpp : implementation of the CN3ViewerView class
//

#include "stdafx.h"

#include <N3Base/N3Camera.h>
#include <N3Base/N3Light.h>
#include <N3Base/N3Shape.h>
#include <N3Base/N3Chr.h>

#include "N3Viewer.h"
#include "MainFrm.h"
#include "N3ViewerDoc.h"
#include "N3ViewerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CN3ViewerView

IMPLEMENT_DYNCREATE(CN3ViewerView, CView)

BEGIN_MESSAGE_MAP(CN3ViewerView, CView)
//{{AFX_MSG_MAP(CN3ViewerView)
ON_WM_ERASEBKGND()
ON_WM_LBUTTONDOWN()
ON_WM_SIZE()
ON_COMMAND(ID_VIEW_AXIS, OnViewAxis)
ON_UPDATE_COMMAND_UI(ID_VIEW_AXIS, OnUpdateViewAxis)
ON_COMMAND(ID_VIEW_GRID, OnViewGrid)
ON_UPDATE_COMMAND_UI(ID_VIEW_GRID, OnUpdateViewGrid)
ON_COMMAND(ID_VIEW_OBJECT_AXIS, OnViewObjectAxis)
ON_UPDATE_COMMAND_UI(ID_VIEW_OBJECT_AXIS, OnUpdateViewObjectAxis)
ON_COMMAND(ID_VIEW_SOLID, OnViewSolid)
ON_UPDATE_COMMAND_UI(ID_VIEW_SOLID, OnUpdateViewSolid)
ON_COMMAND(ID_VIEW_WIREFRAME, OnViewWireframe)
ON_UPDATE_COMMAND_UI(ID_VIEW_WIREFRAME, OnUpdateViewWireframe)
ON_COMMAND(ID_VIEW_COLLISION_MESH, OnViewCollisionMesh)
ON_UPDATE_COMMAND_UI(ID_VIEW_COLLISION_MESH, OnUpdateViewCollisionMesh)
ON_COMMAND(ID_VIEW_CLIMB_MESH, OnViewClimbMesh)
ON_UPDATE_COMMAND_UI(ID_VIEW_CLIMB_MESH, OnUpdateViewClimbMesh)
ON_COMMAND(ID_VIEW_WIREFRAME_SELECTED_MESH, OnViewWireframeSelectedMesh)
ON_UPDATE_COMMAND_UI(ID_VIEW_WIREFRAME_SELECTED_MESH, OnUpdateViewWireframeSelectedMesh)
ON_COMMAND(ID_VIEW_FOCUS, OnViewFocus)
ON_COMMAND(ID_SET_BACKGROUND_COLOR, OnSetBackgroundColor)
ON_WM_MOUSEMOVE()
ON_WM_MOUSEWHEEL()
ON_WM_KEYDOWN()
//}}AFX_MSG_MAP
// Standard printing commands
ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CN3ViewerView construction/destruction

CN3ViewerView::CN3ViewerView()
{
	// TODO: add construction code here
	m_bViewGrid                  = true;
	m_bViewAxis                  = true;
	m_bViewWireFrame             = false;
	m_bViewObjectAxis            = true;
	m_bViewCollisionMesh         = false;
	m_bViewClimbMesh             = false;
	m_bViewSelectedMeshWireFrame = false;

	m_crBkg                      = 0xff606060; // 기본 배경색은 회색.
}

CN3ViewerView::~CN3ViewerView()
{
}

BOOL CN3ViewerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CN3ViewerView drawing

void CN3ViewerView::OnDraw(CDC* pDC)
{
	CN3ViewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CN3Camera* pCamera = pDoc->m_Scene.CameraGetActive();
	if (nullptr == pCamera)
		return;

	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();

	if (pCamera->m_bFogUse) // 안개 사용이면..
	{
		D3DCOLOR crFog = pCamera->m_FogColor;
		pFrm->m_Eng.Clear(crFog);
	}
	else
	{
		pFrm->m_Eng.Clear(m_crBkg);
	}

	if (pFrm->m_IsPlaying)
		pDoc->m_Scene.Tick();
	else
		pDoc->m_Scene.Tick(pDoc->m_Scene.m_fFrmCur);

	pFrm->m_Eng.s_lpD3DDev->BeginScene();

	if (m_bViewGrid)
	{
		__Matrix44 mtxScale;
		mtxScale.Scale(4.0f, 4.0f, 4.0f);
		pFrm->m_Eng.RenderGrid(mtxScale);
	}
	if (m_bViewAxis)
		pFrm->m_Eng.RenderAxis();
	if (m_bViewWireFrame)
		pFrm->m_Eng.s_lpD3DDev->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	else
		pFrm->m_Eng.s_lpD3DDev->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	if (MODE_NORMAL == pFrm->m_eMode)
	{
		pDoc->m_Scene.Render();   // Scene 렌더링..
		if (pDoc->m_pSelectedObj) // 선택된것 렌더링..
		{
			DWORD dwType = pDoc->m_pSelectedObj->Type();

			if (dwType & OBJ_TRANSFORM_COLLISION)
			{
				CN3TransformCollision* pTC = (CN3TransformCollision*) (pDoc->m_pSelectedObj);
				if (m_bViewCollisionMesh)
					pTC->RenderCollisionMesh();
				if (m_bViewClimbMesh)
					pTC->RenderClimbMesh();
			}

			if (dwType & OBJ_SHAPE)
			{
				CN3Shape* pShape = (CN3Shape*) (pDoc->m_pSelectedObj);
				int nPart        = pFrm->GetViewProperty()->m_CBShapePart.GetCurSel();
				CN3SPart* pPD    = pShape->Part(nPart);
				if (pPD)
					pPD->RenderSelected(m_bViewSelectedMeshWireFrame); // 선택된 부분만 렌더링..
				else
					pShape->RenderSelected(m_bViewSelectedMeshWireFrame);

				if (m_bViewObjectAxis
					&& pFrm->GetViewProperty()->m_CBShapePart.GetSafeHwnd()) // 축 보이기..
				{
					int nPart       = pFrm->GetViewProperty()->m_CBShapePart.GetCurSel();
					CN3SPart* pPart = pShape->Part(nPart);
					if (pPart)
						pPart->RenderAxis();
				}
			}
			//		else if(dwType & OBJ_TRANSFORM) ((CN3Shape*)pDoc->m_pSelectedObj)->RenderSelected(m_bViewSelectedMeshWireFrame);
		}
	}
	else if (MODE_EDIT_PMESH == pFrm->m_eMode
			 && pFrm->m_DlgPMeshEdit.m_pShapeRef) // PMesh Edit Mode
	{
		CN3Shape* pShape = pFrm->m_DlgPMeshEdit.m_pShapeRef;

		int nPart        = -1;
		int nPC          = -1;
		if (pFrm->m_DlgPMeshEdit.m_bPreview) // Preview 모드
		{
			float fFrm = pDoc->m_Scene.m_fFrmCur;
			pShape->Tick(fFrm);              // 자동으로 LOD 계산..
		}
		else
		{
			nPart         = pFrm->m_DlgPMeshEdit.m_CBPart.GetCurSel();
			nPC           = pShape->PartCount();

			CN3SPart* pPD = nullptr;
			for (int i = 0; i < nPC; i++)
			{
				pPD = pShape->Part(i);
				_ASSERT(pPD->MeshInstance());

				if (i == nPart)
				{
					int nVC = pFrm->m_DlgPMeshEdit.GetDlgItemInt(IDC_EDIT_NUMVERTICES);
					pPD->MeshInstance()->SetLODByNumVertices(nVC);
				}
				else
				{
					pPD->MeshInstance()->SetLOD(0); // 몽땅 보게끔..
				}
			}
		}

		pFrm->m_DlgPMeshEdit.UpdateInfo();

		pShape->Render();
		if (nPart >= 0 && nPart < nPC)
			pShape->RenderSelected(nPart, m_bViewSelectedMeshWireFrame);
	}

	CN3Base::s_AlphaMgr.Render(); // Draw Alpha primitive...
	pFrm->m_Eng.s_lpD3DDev->EndScene();
	pFrm->m_Eng.Present(m_hWnd);

	// 프레임 표시
#ifdef _DEBUG
	static CString szInfo0, szInfo1, szInfo2;
	szInfo0.Format("FPS : %6.2f", pFrm->m_Eng.s_fFrmPerSec);
	szInfo1.Format("Shape Polygon : %d", pFrm->m_Eng.s_RenderInfo.nShape_Polygon);
	szInfo2.Format("Character Polygon : %d", pFrm->m_Eng.s_RenderInfo.nChr_Polygon);
	pDC->TextOut(0, 0, szInfo0);
	pDC->TextOut(0, 20, szInfo1);
	pDC->TextOut(0, 40, szInfo2);
#endif
}

/////////////////////////////////////////////////////////////////////////////
// CN3ViewerView printing

BOOL CN3ViewerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CN3ViewerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CN3ViewerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CN3ViewerView diagnostics

#ifdef _DEBUG
void CN3ViewerView::AssertValid() const
{
	CView::AssertValid();
}

void CN3ViewerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CN3ViewerDoc* CN3ViewerView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CN3ViewerDoc)));
	return (CN3ViewerDoc*) m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CN3ViewerView message handlers

BOOL CN3ViewerView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;

	return CView::OnEraseBkgnd(pDC);
}

void CN3ViewerView::OnLButtonDown(UINT nFlags, CPoint point)
{
	this->SetFocus();
	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	if (MODE_NORMAL == pFrm->m_eMode)
	{
		CN3ViewerDoc* pDoc   = this->GetDocument();
		int nPart            = -1;
		pDoc->m_pSelectedObj = this->Pick(point, &nPart);            // Picking

		pFrm->GetViewSceneTree()->SelectObject(
			TVI_ROOT, GetDocument()->m_pSelectedObj);                // Tree Select
		if (pDoc->m_pSelectedObj && pDoc->m_pSelectedObj->Type() & OBJ_SHAPE)
		{
			pFrm->GetViewProperty()->m_CBShapePart.SetCurSel(nPart); // 파트 선택..
		}
		pFrm->GetViewProperty()->UpdateInfo();
		this->InvalidateRect(nullptr, FALSE);
	}

	CView::OnLButtonDown(nFlags, point);
}

CN3Base* CN3ViewerView::Pick(POINT point, int* pnPart)
{
	memset(m_pSelObjs, 0, sizeof(m_pSelObjs));
	static __Sort sort[8192];
	int nSortCount = 0;
	memset(sort, 0, sizeof(sort));

	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();

	CN3Scene* pScene = &(GetDocument()->m_Scene);
	int nSC          = pScene->ShapeCount();
	CN3Shape* pShape;
	__Vector3 vDist;
	for (int i = 0; i < nSC; i++)
	{
		pShape = pScene->ShapeGet(i);
		_ASSERT(pShape);

		vDist                     = pShape->s_CameraData.vEye - pShape->Pos();

		sort[nSortCount].pObj     = pShape;
		sort[nSortCount].fCamDist = vDist.Magnitude();

		nSortCount++;
	}

	CN3Chr* pChr;
	int nCC = pScene->ChrCount();
	for (int i = 0; i < nCC; i++)
	{
		pChr = pScene->ChrGet(i);
		_ASSERT(pChr);

		vDist                     = pChr->s_CameraData.vEye - pChr->Pos();

		sort[nSortCount].pObj     = pChr;
		sort[nSortCount].fCamDist = vDist.Magnitude();
		nSortCount++;
	}

	qsort(sort, nSortCount, sizeof(__Sort), SortByCameraDistance);

	int nPart = -1;
	for (int i = 0; i < nSortCount; i++)
	{
		nPart = sort[i].pObj->CheckCollisionPrecisely(true, point.x, point.y);
		if (nPart >= 0)
		{
			m_pSelObjs[0] = sort[i].pObj;
			if (pnPart)
				*pnPart = nPart; // 몇번째 파트가 찍혔나...
			return m_pSelObjs[0];
		}
	}

	return FALSE;
}

int CN3ViewerView::SortByCameraDistance(const void* pArg1, const void* pArg2)
{
	__Sort* pObj1 = (__Sort*) pArg1;
	__Sort* pObj2 = (__Sort*) pArg2;

	if (pObj1->fCamDist < pObj2->fCamDist)
		return -1;
	else if (pObj1->fCamDist > pObj2->fCamDist)
		return 1;
	else
		return 0;
}

void CN3ViewerView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	this->InvalidateRect(nullptr, FALSE);
}

void CN3ViewerView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	CRect rc;
	GetClientRect(&rc);
	pFrm->m_Eng.Reset(TRUE, rc.Width(), rc.Height(), 0);
}

void CN3ViewerView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	if (pFrm)
	{
		static bool bInitMainWnd = false;
		if (bInitMainWnd && pFrm->m_Eng.Reset(TRUE, cx, cy, 0))
		{
			this->InvalidateRect(nullptr, FALSE);
		}
		bInitMainWnd = true;
		CString strText;
		strText.Format("View Width : %d , Height : %d", cx, cy);
		pFrm->SetStatusBarText(strText);
	}
}

void CN3ViewerView::OnViewAxis()
{
	m_bViewAxis = !m_bViewAxis;
	this->InvalidateRect(nullptr, FALSE);
}

void CN3ViewerView::OnUpdateViewAxis(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_bViewAxis);
}

void CN3ViewerView::OnViewGrid()
{
	m_bViewGrid = !m_bViewGrid;
	this->InvalidateRect(nullptr, FALSE);
}

void CN3ViewerView::OnUpdateViewGrid(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_bViewGrid);
}

void CN3ViewerView::OnViewObjectAxis()
{
	m_bViewObjectAxis = !m_bViewObjectAxis;
	this->InvalidateRect(nullptr, FALSE);
}

void CN3ViewerView::OnUpdateViewObjectAxis(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_bViewObjectAxis);
}

void CN3ViewerView::OnViewSolid()
{
	m_bViewWireFrame = false;
	this->InvalidateRect(nullptr, FALSE);
}

void CN3ViewerView::OnUpdateViewSolid(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(TRUE);
}

void CN3ViewerView::OnViewWireframe()
{
	m_bViewWireFrame = true;
	this->InvalidateRect(nullptr, FALSE);
}

void CN3ViewerView::OnUpdateViewWireframe(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(TRUE);
}

void CN3ViewerView::OnViewCollisionMesh()
{
	m_bViewCollisionMesh = !m_bViewCollisionMesh;
	this->InvalidateRect(nullptr, FALSE);
}

void CN3ViewerView::OnUpdateViewCollisionMesh(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_bViewCollisionMesh);
}

void CN3ViewerView::OnViewClimbMesh()
{
	m_bViewClimbMesh = !m_bViewClimbMesh;
	this->InvalidateRect(nullptr, FALSE);
}

void CN3ViewerView::OnUpdateViewClimbMesh(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_bViewClimbMesh);
}

void CN3ViewerView::SetFocusToSelectedObject()
{
	CN3ViewerDoc* pDoc = GetDocument();
	CN3Camera* pCamera = pDoc->m_Scene.CameraGetActive();
	if (nullptr == pCamera)
		return;

	CN3Base* pBase = pDoc->m_pSelectedObj;
	if (pBase && (pBase->Type() & OBJ_TRANSFORM))
	{
		CN3Transform* pTransform = (CN3Transform*) pBase;
		float fVol               = 5.0f;
		if (pBase->Type() & OBJ_TRANSFORM_COLLISION)
		{
			fVol = ((CN3TransformCollision*) pBase)->Radius() * 1.2f;
			//					__Vector3 vMax = ((CN3TransformCollision*)pBase)->Max();
			//					__Vector3 vMin = ((CN3TransformCollision*)pBase)->Min();
			//					fVol = (vMax - vMin).Magnitude();
		}

		if (fVol < CN3Base::s_CameraData.fNP)
			fVol = CN3Base::s_CameraData.fNP * 3.0f;

		__Vector3 vEye = pCamera->EyePos();
		__Vector3 vAt  = pCamera->AtPos();
		__Vector3 vDir = vEye - vAt;
		vDir.Normalize();

		if (pTransform->Type() & OBJ_SHAPE)
		{
			CN3Shape* pShape = (CN3Shape*) pTransform;
			__Vector3 vMin   = pShape->Min();
			__Vector3 vMax   = pShape->Max();
			vAt              = vMin + ((vMax - vMin) / 2.0f);
		}
		else
			vAt = pTransform->Pos();

		vEye = vAt + (vDir * fVol);

		pCamera->EyePosSet(vEye);
		pCamera->AtPosSet(vAt);
		pCamera->UpVectorSet(0, 1, 0);
		pCamera->Tick();
		pCamera->Apply();

		this->InvalidateRect(nullptr, FALSE);
	}
}

LRESULT CN3ViewerView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (m_pDocument)
	{
		CN3ViewerDoc* pDoc = GetDocument();
		CN3Camera* pCamera = pDoc->m_Scene.CameraGetActive();
		if (pCamera)
		{
			MSG msg;
			msg.hwnd    = m_hWnd;
			msg.message = message;
			msg.wParam  = wParam;
			msg.lParam  = lParam;
			if (pCamera->MoveByWindowMessage(&msg))
			{
				this->Invalidate(FALSE);
				return TRUE;
			}
		}
	}

	return CView::WindowProc(message, wParam, lParam);
}

void CN3ViewerView::OnViewWireframeSelectedMesh()
{
	m_bViewSelectedMeshWireFrame = !m_bViewSelectedMeshWireFrame;
	this->InvalidateRect(nullptr, FALSE);
}

void CN3ViewerView::OnUpdateViewWireframeSelectedMesh(CCmdUI* pCmdUI)
{
	if (m_bViewSelectedMeshWireFrame)
	{
		pCmdUI->SetText("View selected mesh wireframe");
	}
	else
	{
		pCmdUI->SetText("View selected mesh hilighted");
	}
}

void CN3ViewerView::OnViewFocus()
{
	this->SetFocusToSelectedObject();
}

void CN3ViewerView::OnSetBackgroundColor()
{
	CColorDialog dlg;
	if (IDCANCEL == dlg.DoModal())
		return;

	DWORD dwColor = dlg.GetColor();
	m_crBkg       = D3DCOLOR_ARGB(
        0xff, (dwColor & 0x000000ff), (dwColor & 0x0000ff00) >> 8, (dwColor & 0x00ff0000) >> 16);

	this->InvalidateRect(nullptr, FALSE);
}

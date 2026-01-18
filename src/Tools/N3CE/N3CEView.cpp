// N3CEView.cpp : implementation of the CN3CEView class
//

#include "stdafx.h"
#include "N3CE.h"
#include "N3CEDoc.h"
#include "N3CEView.h"
#include "MainFrm.h"
#include "FormViewAnimation.h"
#include "FormViewProperty.h"
#include "FormViewTool.h"
#include "mmsystem.h"
#include "PosDummy.h"

#include <N3Base/N3SndObj.h>
#include <N3Base/N3FXPlug.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CN3CEView

IMPLEMENT_DYNCREATE(CN3CEView, CView)

BEGIN_MESSAGE_MAP(CN3CEView, CView)
//{{AFX_MSG_MAP(CN3CEView)
ON_WM_KEYDOWN()
ON_WM_ERASEBKGND()
ON_WM_LBUTTONDOWN()
ON_COMMAND(ID_VIEW_XRAY, OnViewXray)
ON_UPDATE_COMMAND_UI(ID_VIEW_XRAY, OnUpdateViewXray)
ON_COMMAND(ID_VIEW_JOINT, OnViewJoint)
ON_UPDATE_COMMAND_UI(ID_VIEW_JOINT, OnUpdateViewJoint)
ON_WM_DROPFILES()
ON_WM_SIZE()
ON_WM_MOUSEMOVE()
//}}AFX_MSG_MAP
// Standard printing commands
ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CN3CEView construction/destruction

CN3CEView::CN3CEView()
{
	// TODO: add construction code here
	m_bRenderOptionXRay  = false;
	m_bRenderOptionJoint = false;
	m_bPlayingNow        = false;

	m_ptPrev.x = m_ptPrev.y = 0;

	m_pJointSelected        = nullptr;
	m_fTickPrev             = CN3Base::TimeGet();

	m_eCursorMode           = eCM_Nothing; // 커서 모드 보통..

	m_pFXPosTransform       = new CN3Transform;
	m_pPosDummy             = new CPosDummy;
	m_pPosDummy->SetSelObj(m_pFXPosTransform);
}

CN3CEView::~CN3CEView()
{
	delete m_pFXPosTransform;
	delete m_pPosDummy;
}

BOOL CN3CEView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CN3CEView drawing

void CN3CEView::OnDraw(CDC* pDC)
{
	CN3CEDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (TRUE == pDoc->m_bLoadingNow)
		return;                           // 읽는 도중에는 렌더링하지 않는다...

	float fFrm = pDoc->m_Scene.m_fFrmCur; // 일부러 프레임을 컨트롤 하려고 이렇게 해놓았다..
	pDoc->m_Scene.TickCameras();          // 카메라만 Tick
	pDoc->m_Scene.TickLights();

	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	pFrm->m_Eng.Clear(0xff606060);
	pFrm->m_Eng.s_lpD3DDev->BeginScene();

	__Matrix44 mtx;
	mtx.Identity();
	mtx.Scale(0.2f, 0.2f, 0.2f);
	pFrm->m_Eng.RenderGrid(mtx);
	pFrm->m_Eng.RenderAxis();

	CN3Chr* pChr = pDoc->m_Scene.ChrGet(0);
	if (nullptr == pChr || nullptr == pChr->Joint())
	{
		CN3Base::s_AlphaMgr.Render(); // Draw Alpha primitive...
		pFrm->m_Eng.s_lpD3DDev->EndScene();
		pFrm->m_Eng.Present(m_hWnd);
		return;
	}

	DWORD dwAlphaBlend = 0, dwAlphaOP = 0, dwAlphaArg1 = 0;
	if (m_bRenderOptionXRay) // 반투명 옵션이 켜져 있으면..
	{
		// backup state
		pFrm->m_Eng.s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlphaBlend);
		pFrm->m_Eng.s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAOP, &dwAlphaOP);
		pFrm->m_Eng.s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAARG1, &dwAlphaArg1);

		// render state 세팅
		pFrm->m_Eng.s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		pFrm->m_Eng.s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		pFrm->m_Eng.s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		pFrm->m_Eng.s_lpD3DDev->SetRenderState(
			D3DRS_TEXTUREFACTOR, 0x80000000); // alpha factor 설정

		// texture state 세팅(alpha)
		pFrm->m_Eng.s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		pFrm->m_Eng.s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
	}
	else
	{
		pFrm->m_Eng.s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	}

	if (m_pJointSelected) // 관절이 선택되어 있으면..
	{
		DWORD dwZ = 0;
		pFrm->m_Eng.s_lpD3DDev->GetRenderState(D3DRS_ZENABLE, &dwZ);
		pFrm->m_Eng.s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, FALSE);
		m_pJointSelected->Render(&(pChr->m_Matrix), 0.02f);
		//		m_pJointSelected->CN3Transform::Render(&(pChr->m_Matrix), 0.3f); // 축 그리기..
		pFrm->m_Eng.s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, dwZ);
	}

	// Character rendering
	pFrm->m_SndMgr.Tick(); // Sound Tick...
	if (m_bPlayingNow)
	{
		pChr->Tick();
		if (pChr->IsAnimEnd())
		{
			pChr->PosSet(0, 0, 0);
			if (!m_DequeAnimation.empty()) // 에니메이션 큐가 있으면..
			{
				int iAni = m_DequeAnimation[0];
				pChr->AniCurSet(iAni);
				m_DequeAnimation.pop_front();
			}
		}
		else
		{
			CString szMS;
			pFrm->GetPaneAnimation()->GetDlgItemText(IDC_E_MOVE_SPEED, szMS);
			float fZ = (float) (atof(szMS));
			__Vector3 vDir(0, 0, fZ);
			__Vector3 vPos  = pChr->Pos();
			vPos           += vDir * CN3Base::s_fSecPerFrm;
			pChr->PosSet(vPos);
		}
	}
	else
	{
		pChr->PosSet(0, 0, 0);
		pChr->Tick(fFrm);
	}

	int nLOD = pFrm->GetPaneTool()->m_CBLOD.GetCurSel();
	if (nLOD < 0)
		nLOD = 0;
	if (nLOD >= MAX_CHR_LOD)
		nLOD = MAX_CHR_LOD - 1;
	pChr->m_nLOD = nLOD; // LOD 강제 설정..

	pChr->Render();

	/////////////////////////////////////////////
	// Wire Frame Render
	CFormViewProperty* pPView = pFrm->GetPaneProperty();
	HTREEITEM hItem           = pPView->m_TreeChr.GetSelectedItem();
	if (hItem)
	{
		CN3Base* pBase = (CN3Base*) pPView->m_TreeChr.GetItemData(hItem);
		if (pBase && (pBase->Type() & OBJ_CHARACTER_PART))
		{
			CN3CPart* pPart = (CN3CPart*) pBase;

			int nLOD        = pFrm->GetPaneTool()->m_CBLOD.GetCurSel();
			CN3Base::s_lpD3DDev->SetTransform(D3DTS_WORLD, pChr->m_Matrix.toD3D());
			pPart->RenderSelected(nLOD);
		}
		else if (pBase && (pBase->Type() & OBJ_CHARACTER_PLUG))
		{
			CN3CPlug* pPlug = (CN3CPlug*) pBase;
			if (pChr->MatrixGet(pPlug->m_nJointIndex))
			{
				__Matrix44 mtxJ = *(pChr->MatrixGet(pPlug->m_nJointIndex));
				pPlug->RenderFXLines(pChr->m_Matrix, mtxJ); // FX 들어갈 곳에 선을 그려준다.
			}
		}
	}
	// Wire Frame Render
	/////////////////////////////////////////////

	if (eCM_PlugFXPosition0 == m_eCursorMode || eCM_PlugFXPosition1 == m_eCursorMode
		|| eCM_PlugFXPosition2 == m_eCursorMode || eCM_PlugFXPosition3 == m_eCursorMode
		|| eCM_PlugFXPosition4 == m_eCursorMode || eCM_PlugFXPosition5 == m_eCursorMode
		|| eCM_PlugFXPosition6 == m_eCursorMode
		|| eCM_PlugFXPosition7 == m_eCursorMode) // 플러그 효과 위치 0
	{
		m_pPosDummy->Tick();
		m_pPosDummy->Render();
	}

	if (m_bRenderOptionJoint && pChr->Joint())
	{
		pChr->Joint()->Render(&(pChr->m_Matrix), 0.02f);
	}
	/*
	pChr->BuildMesh();
	int nPC = pChr->PartCount();
	pChr->s_lpD3DDev->SetTransform(D3DTS_WORLD, &pChr->m_Matrix);
	for(int i = 0; i < nPC; i++)
	{
		CN3CPart* pPart = pChr->Part(i);
		if(nullptr == pPart) continue;
		
		pPart->Render(0);
	}

	// Character Plug Rendering...
	CN3Base* pBase = pFrm->GetPaneProperty()->GetSelectedObject();
	if(pBase && (pBase->Type() & OBJ_CHARACTER_PLUG))
	{
		CN3CPlug* pPlug = (CN3CPlug*)pBase;
		__Matrix44* pMtxJ = pChr->MatrixGet(pPlug->m_nJointIndex);
		if(pMtxJ) pPlug->Render(pChr->m_Matrix, *pMtxJ);
	}
*/
	if (m_bRenderOptionXRay) // 반투명 옵션이 켜져 있으면..
	{
		// Restore state
		pFrm->m_Eng.s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlphaBlend);
		pFrm->m_Eng.s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, dwAlphaOP);
		pFrm->m_Eng.s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, dwAlphaArg1);
	}

	// Sound 재생 테스트...
	if (pFrm->m_pSndObj0 && pChr->NeedPlaySound0())
	{
		pFrm->m_pSndObj0->Play();
	}
	if (pFrm->m_pSndObj1 && pChr->NeedPlaySound1())
	{
		pFrm->m_pSndObj1->Play();
	}

	CN3Base::s_AlphaMgr.Render(); // Draw Alpha primitive...
	pFrm->m_Eng.s_lpD3DDev->EndScene();
	pFrm->m_Eng.Present(m_hWnd);

	// 프레임 표시
	float fTick = CN3Base::TimeGet();
	if (fTick > m_fTickPrev + 0.3f)
	{
		CString szAniName;
		if (pChr->AniDataCur())
			szAniName = pChr->AniDataCur()->szName.c_str();
		m_szFPS.Format("%s - %6.2f Frame/Sec", szAniName, pFrm->m_Eng.s_fFrmPerSec);
		m_fTickPrev = fTick;
	}
	pDC->TextOut(0, 0, m_szFPS);

	int iAC = static_cast<int>(m_DequeAnimation.size());
	for (int i = 0; i < iAC; i++)
	{
		CN3AnimControl* pAC = pChr->AniCtrl();
		if (pAC == nullptr)
			continue;

		int iAni         = m_DequeAnimation[i];

		__AnimData* pAni = pAC->DataGet(iAni);
		if (pAni == nullptr)
			continue;

		CString szAniName = pAni->szName.c_str();
		pDC->TextOut(20, 20 + i * 18, szAniName);
	}
}

void CN3CEView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	// TODO: You may populate your ListView with items by directly accessing
	//  its list control through a call to GetListCtrl().

	DragAcceptFiles(); // Drag File 을 받는다..

	m_pJointSelected = nullptr;

	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	CRect rc;
	GetClientRect(rc);
	pFrm->m_Eng.Reset(TRUE, rc.Width(), rc.Height(), 0);

	this->SetCameraToDefault(); // 카메라를 기본값으로 하고..
	this->InvalidateRect(nullptr, FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// CN3CEView printing

BOOL CN3CEView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CN3CEView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CN3CEView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CN3CEView diagnostics

#ifdef _DEBUG
void CN3CEView::AssertValid() const
{
	CView::AssertValid();
}

void CN3CEView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CN3CEDoc* CN3CEView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CN3CEDoc)));
	return (CN3CEDoc*) m_pDocument;
}
#endif //_DEBUG

void CN3CEView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if ('F' == nChar)
	{
		SetCameraToDefault();
	} // TODO: Add your message handler code here and/or call default

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CN3CEView::SetCameraToDefault()
{
	CN3Camera* pCamera = GetDocument()->m_Scene.CameraGetActive();
	if (nullptr == pCamera)
		return;

	pCamera->m_Data.fNP  = 0.1f;
	pCamera->m_Data.fFP  = 256.0f;
	pCamera->m_Data.fFOV = DegreesToRadians(72.0f);
	pCamera->EyePosSet(1.0f, 1.2f, 1.5f);
	pCamera->AtPosSet(0.0f, 0.8f, 0.0f);
	pCamera->UpVectorSet(0.0f, 1.0f, 0.0f);

	this->InvalidateRect(nullptr, FALSE);
}

BOOL CN3CEView::OnEraseBkgnd(CDC* pDC)
{
	return 0;
	return CView::OnEraseBkgnd(pDC);
}

void CN3CEView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (!(nFlags & MK_CONTROL) && !(nFlags & MK_SHIFT) && !::_IsKeyDown(VK_MENU))
	{
		m_pJointSelected = nullptr;

		CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
		CN3Scene* pScene = &(GetDocument()->m_Scene);
		CN3Chr* pChr     = pScene->ChrGet(0);
		CN3Joint* pJoint = pChr->Joint();
		if (pJoint)
		{
			int n = 0;
			static __Matrix44 mtxs[256];
			pJoint->MatricesGet(mtxs, n);

			float fC = 100000.0f;
			int nJI  = -1;
			for (int i = 0; i < n; i++)
			{
				mtxs[i]        *= pChr->m_Matrix;
				__Vector3 vPos  = mtxs[i].Pos();
				_POINT pt       = ::_Convert3D_To_2DCoordinate(vPos, pScene->s_CameraData.mtxView,
						  pScene->s_CameraData.mtxProjection, pScene->s_CameraData.vp.Width,
						  pScene->s_CameraData.vp.Height);

				if (pt.x < 0 || pt.y < 0)
					continue;

				int x     = point.x - pt.x;
				int y     = point.y - pt.y;
				float fC2 = sqrtf((float) (x * x + y * y));

				if (fC2 < fC)
				{
					nJI = i;
					fC  = fC2;
				}
			}

			if (nJI >= 0 && nJI < n)
			{
				pJoint->FindPointerByID(nJI, m_pJointSelected);
			}

			CN3Base* pBase = pFrm->GetPaneProperty()
								 ->GetSelectedObject(); // Plug 에 Joint Index 를 넣는다..
			if (eCM_PickJoint == this->m_eCursorMode && // 조인트 선택 모드..
				pBase && (pBase->Type() & OBJ_CHARACTER_PLUG))
			{
				CN3CPlug* pPlug      = (CN3CPlug*) pBase;
				pPlug->m_nJointIndex = nJI;

				pFrm->GetPaneProperty()->UpdateInfo();
			}
			//////////////////////////////////////////////////
			//	Coded (By Dino On 2002-10-11 오후 1:50:47 )
			//	FXPlug
			else if (
				pBase
				&& (pBase->Type()
					& OBJ_FX_PLUG_PART) // FXPlugPart가 선택되어 있을경우 참조하는 조인트를 설정한다.
				&& (nJI >= 0 && nJI < n))
			{
				CN3FXPlugPart* pFXPPart = (CN3FXPlugPart*) pBase;
				pFXPPart->SetRefIdx(nJI);

				pFrm->GetPaneProperty()->UpdateInfo();
			}
			//	End Of Code (By Dino On 2002-10-11 오후 1:50:47 )
			//////////////////////////////////////////////////
			else if (m_pJointSelected && m_bRenderOptionJoint)
			{
				pFrm->GetPaneProperty()->SelectJointNode(TVI_ROOT, m_pJointSelected);
			}
		}
	}

	this->InvalidateRect(nullptr, FALSE);

	m_ptPrev = point;

	CView::OnLButtonDown(nFlags, point);
}

void CN3CEView::OnViewXray()
{
	m_bRenderOptionXRay = !m_bRenderOptionXRay;
	this->InvalidateRect(nullptr, FALSE);
}

void CN3CEView::OnUpdateViewXray(CCmdUI* pCmdUI)
{
}

void CN3CEView::OnViewJoint()
{
	m_bRenderOptionJoint = !m_bRenderOptionJoint;
	this->InvalidateRect(nullptr, FALSE);
}

void CN3CEView::OnUpdateViewJoint(CCmdUI* pCmdUI)
{
}

void CN3CEView::UpdateAllInfo()
{
	SetCameraToDefault(); // 카메라를 기본값으로 만들고...
	m_pJointSelected = nullptr;
	this->InvalidateRect(nullptr, FALSE);
}

DROPEFFECT CN3CEView::OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	// TODO: Add your specialized code here and/or call the base class

	return CView::OnDragEnter(pDataObject, dwKeyState, point);
}

void CN3CEView::OnDragLeave()
{
	// TODO: Add your specialized code here and/or call the base class

	CView::OnDragLeave();
}

DROPEFFECT CN3CEView::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	// TODO: Add your specialized code here and/or call the base class

	return CView::OnDragOver(pDataObject, dwKeyState, point);
}

BOOL CN3CEView::OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	// TODO: Add your specialized code here and/or call the base class

	return CView::OnDrop(pDataObject, dropEffect, point);
}

void CN3CEView::OnDropFiles(HDROP hDropInfo)
{
	// TODO: Add your message handler code here and/or call default
	char szFN[256];
	::DragQueryFile(hDropInfo, 0, szFN, 256);
	::DragFinish(hDropInfo);

	if (lstrlen(szFN) > 0)
	{
		GetDocument()->OnOpenDocument(szFN);
	}

	CView::OnDropFiles(hDropInfo);
}

void CN3CEView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	this->InvalidateRect(nullptr, FALSE);
}

void CN3CEView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	if (pFrm)
	{
		pFrm->m_Eng.Reset(TRUE, cx, cy, 0);
		this->InvalidateRect(nullptr, FALSE);
	}
}

LRESULT CN3CEView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (m_pDocument)
	{
		CN3CEDoc* pDoc     = GetDocument();
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

		if (GetDocument()->m_Scene.ChrGet(0) && m_pPosDummy)
		{
			m_pPosDummy->Tick();
			MSG msg;
			msg.hwnd    = m_hWnd;
			msg.message = message;
			msg.wParam  = wParam;
			msg.lParam  = lParam;
			if (m_pPosDummy->MouseMsgFilter(&msg))
			{
				CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
				CN3CEDoc* pDoc   = GetDocument();

				if (!pDoc || !pFrm)
					return TRUE;

				CN3CPlug* pPlug = (CN3CPlug*) (pFrm->GetPaneProperty()->GetSelectedObject());
				CN3Chr* pChr    = pDoc->m_Scene.ChrGet(0);

				if (eCM_PlugFXPosition0 == m_eCursorMode || eCM_PlugFXPosition1 == m_eCursorMode
					|| eCM_PlugFXPosition2 == m_eCursorMode || eCM_PlugFXPosition3 == m_eCursorMode
					|| eCM_PlugFXPosition4 == m_eCursorMode || eCM_PlugFXPosition5 == m_eCursorMode
					|| eCM_PlugFXPosition6 == m_eCursorMode
					|| eCM_PlugFXPosition7 == m_eCursorMode) // 플러그 효과 위치 0
				{
					int iIndex = m_eCursorMode - eCM_PlugFXPosition0;
					if (iIndex >= 0 && iIndex < MAX_PLUG_FX_POSITION)
					{
						__Matrix44 mtx;
						if (pChr->MatrixGet(pPlug->m_nJointIndex))
						{
							__Matrix44 mtxJ  = *(pChr->MatrixGet(pPlug->m_nJointIndex));
							mtx              = pPlug->m_Matrix;
							mtx             *= mtxJ;
							mtx             *= pChr->m_Matrix;
						}

						__Matrix44 inv_mtx = mtx.Inverse();
						__Vector3 vPos     = m_pFXPosTransform->Pos() * inv_mtx;

						//pPlug->m_vFXs[iIndex] = vPos;
					}
				}

				pPlug->ReCalcMatrix();
				pFrm->GetPaneProperty()->UpdateInfo();
				this->Invalidate(FALSE);
			}
		}
	}

	return CView::WindowProc(message, wParam, lParam);
}

void CN3CEView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!::_IsKeyDown(VK_MENU)
		&& ((nFlags & MK_LBUTTON) || (nFlags & MK_MBUTTON)
			|| (nFlags & MK_RBUTTON))) // 커서 모드에 따라 조정..
	{
		// 일단 플러그가 선택되었는지 본다.
		CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
		CN3Base* pBase   = pFrm->GetPaneProperty()->GetSelectedObject();
		CN3Chr* pChr     = GetDocument()->m_Scene.ChrGet(0);
		if (eCM_Nothing != m_eCursorMode && // 커서 모드가 보통게 아니고..
			pChr && pBase && (pBase->Type() & OBJ_CHARACTER_PLUG))
		{
			CN3CPlug* pPlug = (CN3CPlug*) pBase;

			__Vector3 vAxis;
			if (nFlags & MK_LBUTTON)
				vAxis.Set(1, 0, 0);
			else if (nFlags & MK_MBUTTON)
				vAxis.Set(0, 1, 0);
			else if (nFlags & MK_RBUTTON)
				vAxis.Set(0, 0, 1);

			if (eCM_PlugPosition == m_eCursorMode) // 플러그 위치
			{
				const __Matrix44* pMtxTmp = pChr->MatrixGet(pPlug->m_nJointIndex);
				if (pMtxTmp)
				{
					__Matrix44 mtx = CN3Base::s_CameraData.mtxViewInverse;
					mtx.PosSet(0, 0, 0);

					CPoint ptDelta = point - m_ptPrev;
					__Vector3 vDelta(ptDelta.x * 0.003f, -ptDelta.y * 0.003f, 0);
					vDelta *= mtx;

					mtx     = pMtxTmp->Inverse();
					mtx.PosSet(0, 0, 0);
					vDelta *= mtx;

					pPlug->PositionSet(pPlug->Position() + vDelta);
				}
			}
			else if (eCM_PlugScale == m_eCursorMode) // 플러그 스케일
			{
				CPoint ptDelta = point - m_ptPrev;
				if (_IsKeyDown(VK_CONTROL))
					vAxis.Set(1, 1, 1);

				__Vector3 vScale = pPlug->Scale() + vAxis * (ptDelta.x * 0.003f);

				pPlug->ScaleSet(vScale);
			}
			else if (eCM_PlugRotation == m_eCursorMode) // 플러그 스케일
			{
				const __Matrix44* pMtxTmp = pChr->MatrixGet(pPlug->m_nJointIndex);
				if (pMtxTmp)
				{
					__Matrix44 mtx;
					mtx.Identity();

					mtx = pMtxTmp->Inverse();
					mtx.PosSet(0, 0, 0);

					vAxis  = CN3Base::s_CameraData.vAt - CN3Base::s_CameraData.vEye;
					vAxis *= mtx;
					vAxis.Normalize();

					CPoint ptDelta = point - m_ptPrev;
					float fAngle   = (ptDelta.x) * 0.01f;
					__Quaternion qtRot;
					qtRot.RotationAxis(vAxis, fAngle);

					mtx = qtRot;
					mtx = pPlug->Rotation() * mtx;
					pPlug->RotationSet(mtx);
				}
			}
			/*
			else if(eCM_PlugFXPosition0 == m_eCursorMode ||
					eCM_PlugFXPosition1 == m_eCursorMode ||
					eCM_PlugFXPosition2 == m_eCursorMode ||
					eCM_PlugFXPosition3 == m_eCursorMode ||
					eCM_PlugFXPosition4 == m_eCursorMode ||
					eCM_PlugFXPosition5 == m_eCursorMode ||
					eCM_PlugFXPosition6 == m_eCursorMode ||
					eCM_PlugFXPosition7 == m_eCursorMode ) // 플러그 효과 위치 0
			{
				int iIndex = m_eCursorMode - eCM_PlugFXPosition0;
				if(iIndex >= 0 && iIndex < MAX_PLUG_FX_POSITION)
				{
					CPoint ptDelta = point - m_ptPrev;
					vAxis *= (ptDelta.x) * 0.001f;
					
					pPlug->m_vFXs[iIndex] += vAxis;
				}
			}
			*/

			pPlug->ReCalcMatrix();
			pFrm->GetPaneProperty()->UpdateInfo();

			this->InvalidateRect(nullptr, FALSE);
		}
		//////////////////////////////////////////////////
		//	Coded (By Dino On 2002-10-11 오후 2:04:29 )
		//	FXPlug
		else if (pChr && pBase && (pBase->Type() & OBJ_FX_PLUG_PART))
		{
			CN3FXPlugPart* pFXPPart = (CN3FXPlugPart*) pBase;

			__Vector3 vAxis;
			if (nFlags & MK_LBUTTON)
				vAxis.Set(1, 0, 0);
			else if (nFlags & MK_MBUTTON)
				vAxis.Set(0, 1, 0);
			else if (nFlags & MK_RBUTTON)
				vAxis.Set(0, 0, 1);

			if (nFlags & MK_SHIFT) // 위치
			{
				const __Matrix44* pMtxTmp = pChr->MatrixGet(pFXPPart->GetRefIndex());
				if (pMtxTmp)
				{
					__Matrix44 mtx = CN3Base::s_CameraData.mtxViewInverse;
					mtx.PosSet(0, 0, 0);

					CPoint ptDelta = point - m_ptPrev;
					__Vector3 vDelta(ptDelta.x * 0.003f, -ptDelta.y * 0.003f, 0);
					vDelta *= mtx;

					mtx     = pMtxTmp->Inverse();
					mtx.PosSet(0, 0, 0);
					vDelta                 *= mtx;

					pFXPPart->m_vOffsetPos += vDelta;
				}
			}
			else if (nFlags & MK_CONTROL) // 회전
			{
				const __Matrix44* pMtxTmp = pChr->MatrixGet(pFXPPart->GetRefIndex());
				if (pMtxTmp)
				{
					__Matrix44 mtx;
					mtx.Identity();

					mtx = pMtxTmp->Inverse();
					mtx.PosSet(0, 0, 0);

					vAxis  = CN3Base::s_CameraData.vAt - CN3Base::s_CameraData.vEye;
					vAxis *= mtx;
					vAxis.Normalize();

					CPoint ptDelta = point - m_ptPrev;
					float fAngle   = (ptDelta.x) * 0.01f;
					__Quaternion qtRot;
					qtRot.RotationAxis(vAxis, fAngle);

					mtx                     = qtRot;
					pFXPPart->m_vOffsetDir *= mtx;
					pFXPPart->m_vOffsetDir.Normalize();
				}
			}

			pFrm->GetPaneProperty()->UpdateInfo();
			this->InvalidateRect(nullptr, FALSE);
		}
		//	End Of Code (By Dino On 2002-10-11 오후 2:04:29 )
		//////////////////////////////////////////////////
	}

	m_ptPrev = point;

	CView::OnMouseMove(nFlags, point);
}

void CN3CEView::InitFXPos()
{
	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	CN3CEDoc* pDoc   = GetDocument();

	if (!pDoc || !pFrm)
		return;

	CN3CPlug* pPlug = (CN3CPlug*) (pFrm->GetPaneProperty()->GetSelectedObject());
	CN3Chr* pChr    = pDoc->m_Scene.ChrGet(0);

	if (!pChr || !pPlug)
		return;

	if (eCM_PlugFXPosition0 == m_eCursorMode || eCM_PlugFXPosition1 == m_eCursorMode
		|| eCM_PlugFXPosition2 == m_eCursorMode || eCM_PlugFXPosition3 == m_eCursorMode
		|| eCM_PlugFXPosition4 == m_eCursorMode || eCM_PlugFXPosition5 == m_eCursorMode
		|| eCM_PlugFXPosition6 == m_eCursorMode
		|| eCM_PlugFXPosition7 == m_eCursorMode) // 플러그 효과 위치 0
	{
		int iIndex = m_eCursorMode - eCM_PlugFXPosition0;
		if (iIndex >= 0 && iIndex < MAX_PLUG_FX_POSITION)
		{
			__Matrix44 mtx;
			if (pChr->MatrixGet(pPlug->m_nJointIndex))
			{
				__Matrix44 mtxJ  = *(pChr->MatrixGet(pPlug->m_nJointIndex));
				mtx              = pPlug->m_Matrix;
				mtx             *= mtxJ;
				mtx             *= pChr->m_Matrix;
			}

			__Vector3 vPos;
			//vPos = pPlug->m_vFXs[iIndex] * mtx;
			m_pFXPosTransform->PosSet(vPos);
			m_pPosDummy->SetSelObj(m_pFXPosTransform);
		}
	}
}

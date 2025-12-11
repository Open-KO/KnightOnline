// ViewProperty.cpp : implementation file
//

#include "stdafx.h"
#include "N3Viewer.h"
#include "ViewProperty.h"

#include "MainFrm.h"
#include "N3ViewerDoc.h"

#include <N3Base/N3Scene.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewProperty

IMPLEMENT_DYNCREATE(CViewProperty, CFormView)

CViewProperty::CViewProperty()
	: CFormView(CViewProperty::IDD)
{
	//{{AFX_DATA_INIT(CViewProperty)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CViewProperty::~CViewProperty()
{
}

void CViewProperty::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CViewProperty)
	DDX_Control(pDX, IDC_LP_MATERIAL, m_LPMaterial);
	DDX_Control(pDX, IDC_LP_LIGHT, m_LPLight);
	DDX_Control(pDX, IDC_LP_CAMERA, m_LPCamera);
	DDX_Control(pDX, IDC_CB_PART_SHAPE, m_CBShapePart);
	DDX_Control(pDX, IDC_CB_PART_CHARACTER, m_CBChrPart);
	DDX_Control(pDX, IDC_CB_PLUG_CHARACTER, m_CBChrPlug);
	DDX_Control(pDX, IDC_CB_LOD, m_CBChrLOD);
	DDX_Control(pDX, IDC_LP_TRANSFORM, m_LPTransform);
	DDX_Control(pDX, IDC_LP_SHAPE, m_LPShape);
	DDX_Control(pDX, IDC_LP_CHR, m_LPChr);
	DDX_Control(pDX, IDC_LP_CHR_PART, m_LPCPart);
	DDX_Control(pDX, IDC_LP_CHR_PLUG, m_LPCPlug);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CViewProperty, CFormView)
	//{{AFX_MSG_MAP(CViewProperty)
	ON_WM_SIZE()
	ON_CBN_SELCHANGE(IDC_CB_PART_SHAPE, OnSelchangeCbPartShape)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewProperty diagnostics

#ifdef _DEBUG
void CViewProperty::AssertValid() const
{
	CFormView::AssertValid();
}

void CViewProperty::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CN3ViewerDoc* CViewProperty::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CN3ViewerDoc)));
	return (CN3ViewerDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CViewProperty message handlers

void CViewProperty::UpdateInfo()
{
	if(m_LPTransform.GetSafeHwnd() == nullptr) return;

	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	CN3Base* pBase = GetDocument()->m_pSelectedObj;

	CRect rc; 
	GetClientRect(&rc);
	int nW = rc.Width() - 8;
	int nH = 0;

	CWnd* pWnd = GetDlgItem(IDC_E_NAME);
	pWnd->GetWindowRect(&rc);
	this->ScreenToClient(&rc);
	pWnd->SetWindowPos(nullptr, 0, 0, nW, rc.Height(), SWP_NOMOVE | SWP_NOZORDER);
	nH = rc.Height();

	if(pBase == nullptr)
	{
		SetDlgItemText(IDC_E_NAME, "Not Selected...");
		m_LPTransform.ShowWindow(SW_HIDE);

		m_LPCamera.ShowWindow(SW_HIDE);
		m_LPLight.ShowWindow(SW_HIDE);

		m_LPMaterial.ShowWindow(SW_HIDE);

		m_CBShapePart.ShowWindow(SW_HIDE);
		m_LPShape.ShowWindow(SW_HIDE);
		
		m_CBChrPart.ShowWindow(SW_HIDE);
		m_CBChrLOD.ShowWindow(SW_HIDE);
		m_CBChrPlug.ShowWindow(SW_HIDE);
		m_LPChr.ShowWindow(SW_HIDE);
		m_LPCPart.ShowWindow(SW_HIDE);
		m_LPCPlug.ShowWindow(SW_HIDE);
		
		return;
	}
	
	CPoint pt(rc.left, rc.top);
	pt.y += nH;

	SetDlgItemText(IDC_E_NAME, pBase->m_szName.c_str());
	if(pBase->Type() & OBJ_TRANSFORM)
	{
		nH = m_LPTransform.GetItemHeight(0) * m_LPTransform.GetCount() + 4;
		m_LPTransform.ShowWindow(SW_SHOW);
		m_LPTransform.SetWindowPos(nullptr, pt.x, pt.y, nW, nH, SWP_NOZORDER);
		pt.y += nH + 3;

		CN3Transform* pT = (CN3Transform*)pBase;
		CPropertyItem* pItem;

		pItem = m_LPTransform.GetPropItem("Position");
		if(pItem) pItem->VectorSet(pT->Pos());
//		pItem = m_LPTransform.GetPropItem("Rotation");
//		if(pItem) pItem->VectorSet(pT->RotGet());
		pItem = m_LPTransform.GetPropItem("Scale");
		if(pItem) pItem->VectorSet(pT->Scale());

		m_LPTransform.Invalidate();
	}
	else
	{
		m_LPTransform.ShowWindow(SW_HIDE);
	}

	if(pBase->Type() & OBJ_CAMERA)
	{
		nH = m_LPCamera.GetItemHeight(0) * m_LPCamera.GetCount() + 4;
		m_LPCamera.ShowWindow(SW_SHOW);
		m_LPCamera.SetWindowPos(nullptr, pt.x, pt.y, nW, nH, SWP_NOZORDER);
		pt.y += nH + 3;

		CN3Camera* pC = (CN3Camera*)pBase;
		CPropertyItem* pItem;

		pItem = m_LPCamera.GetPropItem("Eye");
		if(pItem) pItem->VectorSet(pC->EyePos());
		pItem = m_LPCamera.GetPropItem("At");
		if(pItem) pItem->VectorSet(pC->AtPos());
		pItem = m_LPCamera.GetPropItem("Up");
		if(pItem) pItem->VectorSet(pC->UpVector());
		pItem = m_LPCamera.GetPropItem("Field Of View");
		if(pItem) pItem->m_curValue.Format("%f", pC->m_Data.fFOV);
		pItem = m_LPCamera.GetPropItem("Near Plane");
		if(pItem) pItem->m_curValue.Format("%f", pC->m_Data.fNP);
		pItem = m_LPCamera.GetPropItem("Far Plane");
		if(pItem) pItem->m_curValue.Format("%f", pC->m_Data.fFP);
		pItem = m_LPCamera.GetPropItem("안개 사용");
		if(pItem)
		{
			if(pC->m_bFogUse) pItem->m_curValue = "On";
			else pItem->m_curValue = "Off";
		}
//		pItem = m_LPCamera.GetPropItem("안개 밀도");
//		if(pItem) pItem->m_curValue.Format("%f", pC->m_fFogDensity);
		pItem = m_LPCamera.GetPropItem("안개 색");
		if(pItem) pItem->D3DColorSet(pC->m_FogColor);
//		pItem = m_LPCamera.GetPropItem("안개 시작");
//		if(pItem) pItem->m_curValue.Format("%f", pC->m_fFogStart);
//		pItem = m_LPCamera.GetPropItem("안개 끝");
//		if(pItem) pItem->m_curValue.Format("%f", pC->m_fFogEnd);

		m_LPCamera.Invalidate();
	}
	else
	{
		m_LPCamera.ShowWindow(SW_HIDE);
	}

	if(pBase->Type() & OBJ_LIGHT)
	{
		nH = m_LPLight.GetItemHeight(0) * m_LPLight.GetCount() + 4;
		m_LPLight.ShowWindow(SW_SHOW);
		m_LPLight.SetWindowPos(nullptr, pt.x, pt.y, nW, nH, SWP_NOZORDER);
		pt.y += nH + 3;

		CN3Light* pL = (CN3Light*)pBase;
		CPropertyItem* pItem;

		pItem = m_LPLight.GetPropItem("On");
		if(pItem) 
		{
			if(pL->m_Data.bOn == FALSE) pItem->m_curValue = "Off";
			else pItem->m_curValue = "On";
		}
		pItem = m_LPLight.GetPropItem("Number");
		if(pItem) pItem->m_curValue.Format("%d", pL->m_Data.nNumber);
		pItem = m_LPLight.GetPropItem("Type");
		if(pItem) pItem->m_crColor = pL->m_Data.Type;
		pItem = m_LPLight.GetPropItem("Direction");
		if(pItem) pItem->VectorSet(pL->m_Data.Direction);
		pItem = m_LPLight.GetPropItem("Ambient Color");
		if(pItem) pItem->D3DColorValueSet(pL->m_Data.Ambient);
		pItem = m_LPLight.GetPropItem("Diffuse Color");
		if(pItem) pItem->D3DColorValueSet(pL->m_Data.Diffuse);
		pItem = m_LPLight.GetPropItem("Specular Color");
		if(pItem) pItem->D3DColorValueSet(pL->m_Data.Specular);
		pItem = m_LPLight.GetPropItem("Range");
		if(pItem) pItem->m_curValue.Format("%f", pL->m_Data.Range);
		pItem = m_LPLight.GetPropItem("Attenuation0");
		if(pItem) pItem->m_curValue.Format("%f", pL->m_Data.Attenuation0);
		pItem = m_LPLight.GetPropItem("Attenuation1");
		if(pItem) pItem->m_curValue.Format("%f", pL->m_Data.Attenuation1);
		pItem = m_LPLight.GetPropItem("Attenuation2");
		if(pItem) pItem->m_curValue.Format("%f", pL->m_Data.Attenuation2);

		m_LPLight.Invalidate();
	}
	else
	{
		m_LPLight.ShowWindow(SW_HIDE);
	}

	__Material* pMtl = nullptr;

	if(pBase->Type() & OBJ_SHAPE)
	{
		m_CBShapePart.GetWindowRect(&rc);
		nH = rc.Height();
		m_CBShapePart.SetWindowPos(nullptr, pt.x, pt.y, nW, nH, SWP_NOZORDER);
		m_CBShapePart.ShowWindow(SW_SHOW);
		pt.y += nH + 3;

		nH = m_LPShape.GetItemHeight(0) * m_LPShape.GetCount() + 4;
		m_LPShape.ShowWindow(SW_SHOW);
		m_LPShape.SetWindowPos(nullptr, pt.x, pt.y, nW, nH, SWP_NOZORDER);
		pt.y += nH + 3;

		CN3Shape* pS = (CN3Shape*)pBase;
		CPropertyItem* pItem;

		pItem = m_LPShape.GetPropItem("소속");			if(pItem) pItem->m_curValue.Format("%d", pS->m_iBelong);
		pItem = m_LPShape.GetPropItem("Event ID");		if(pItem) pItem->m_curValue.Format("%d", pS->m_iEventID);
		pItem = m_LPShape.GetPropItem("Event Type");	if(pItem) pItem->m_curValue.Format("%d", pS->m_iEventType);
		pItem = m_LPShape.GetPropItem("NPC ID");		if(pItem) pItem->m_curValue.Format("%d", pS->m_iNPC_ID);
		pItem = m_LPShape.GetPropItem("NPC Status");	if(pItem) pItem->m_curValue.Format("%d", pS->m_iNPC_Status); // NPC 로 쓰는 오브젝트일 경우 NPC Type

		int nPartCount = pS->PartCount();

		int nPart = m_CBShapePart.GetCurSel();
		m_CBShapePart.ResetContent();
		for(int i = 0; i < nPartCount; i++)
		{
			CN3SPart* pPD = pS->Part(i);
			CN3PMesh* pPMesh = pPD->Mesh();

			CString szTmp = "??? Null Mesh Pointer ???";
			if(pPMesh) szTmp.Format("Part%2d : %s", i, pPMesh->m_szName.c_str());
			m_CBShapePart.AddString(szTmp);
		}
		if(nPart < 0 || nPart >= nPartCount) nPart = 0;
		m_CBShapePart.SetCurSel(nPart);

		if(nPart >= 0 && nPart < nPartCount)
		{
			CN3SPart* pPD = pS->Part(nPart);

			pMtl = &(pPD->m_Mtl); // Material Pointer
			CN3PMesh* pPMesh = pPD->Mesh();

			pItem = m_LPShape.GetPropItem("Face, Vertex Count");
			if(pItem)
			{
				if(pPMesh) pItem->m_curValue.Format("%4d, %4d", pPMesh->GetMaxNumIndices()/3, pPMesh->GetMinNumVertices());
				else pItem->m_curValue = "";
			}

			pItem = m_LPShape.GetPropItem("Collision Mesh File");
			if(pItem)
			{
				CN3VMesh* pMC = pS->CollisionMesh();
				if(pMC) pItem->m_curValue = pMC->m_szName.c_str();
				else pItem->m_curValue = "";
			}

			pItem = m_LPShape.GetPropItem("Texture File");
			if(pItem)
			{
				CN3Texture* pTex = pPD->Tex(0);
				if(pTex) pItem->m_curValue = pTex->FileName().c_str();
				else pItem->m_curValue = "";
			}

			pItem = m_LPShape.GetPropItem("Texture Animaion Speed");
			if(pItem)
			{
				pItem->m_curValue.Format("%.2f", pPD->m_fTexFPS);
			}

			pItem = m_LPShape.GetPropItem("Mesh File");
			if(pItem)
			{
				if(pPMesh) pItem->m_curValue = pPMesh->FileName().c_str();
				else pItem->m_curValue = "";
			}
		}

		m_LPShape.Invalidate();
	}
	else
	{
		m_CBShapePart.ShowWindow(SW_HIDE);
		m_LPShape.ShowWindow(SW_HIDE);
	}


	if(pBase->Type() & OBJ_CHARACTER)
	{
		nH = m_LPChr.GetItemHeight(0) * m_LPChr.GetCount() + 4;
		m_LPChr.ShowWindow(SW_SHOW);
		m_LPChr.SetWindowPos(nullptr, pt.x, pt.y, nW, nH, SWP_NOZORDER);
		pt.y += nH + 3;

		m_CBChrPart.GetWindowRect(&rc);
		m_CBChrPart.SetWindowPos(nullptr, pt.x, pt.y, (nW/2) - 2, rc.Height(), SWP_NOZORDER);
		m_CBChrPart.ShowWindow(SW_SHOW);

		m_CBChrLOD.GetWindowRect(&rc);
		m_CBChrLOD.SetWindowPos(nullptr, pt.x + (nW/2) + 2, pt.y, (nW/2) - 2, rc.Height(), SWP_NOZORDER);
		m_CBChrLOD.ShowWindow(SW_SHOW);
		pt.y += rc.Height() + 2;

		nH = m_LPCPart.GetItemHeight(0) * m_LPCPart.GetCount() + 4;
		m_LPCPart.ShowWindow(SW_SHOW);
		m_LPCPart.SetWindowPos(nullptr, pt.x, pt.y, nW, nH, SWP_NOZORDER);
		pt.y += nH + 3;

		m_CBChrPlug.GetWindowRect(&rc);
		nH = rc.Height();
		m_CBChrPlug.SetWindowPos(nullptr, pt.x, pt.y, nW, nH, SWP_NOZORDER);
		m_CBChrPlug.ShowWindow(SW_SHOW);
		pt.y += nH + 3;

		nH = m_LPCPlug.GetItemHeight(0) * m_LPCPlug.GetCount() + 4;
		m_LPCPlug.ShowWindow(SW_SHOW);
		m_LPCPlug.SetWindowPos(nullptr, pt.x, pt.y, nW, nH, SWP_NOZORDER);
		pt.y += nH + 3;

		CN3Chr* pC = (CN3Chr*)pBase;
		CPropertyItem* pItem;

		int nLOD = m_CBChrLOD.GetCurSel();
		if(nLOD < 0 || nLOD >= MAX_CHR_LOD)
		{
			nLOD = 0;
			m_CBChrLOD.SetCurSel(nLOD);
		}

		int nPart = m_CBChrPart.GetCurSel();
		int nPartCount = pC->PartCount();
		m_CBChrPart.ResetContent();
		for(int i = 0; i < nPartCount; i++)
		{
			CString szTmp;
			szTmp.Format("Part : %d", i);
			m_CBChrPart.AddString(szTmp);
		}
		
		if(nPart < 0) nPart = 0;
		if(nPart >= nPartCount) nPart = nPartCount - 1;
		m_CBChrPart.SetCurSel(nPart);

		pItem = m_LPChr.GetPropItem("Joint File");
		if(pItem)
		{
			CN3Joint* pJoint = pC->Joint();
			if(pJoint) pItem->m_curValue = pJoint->m_szName.c_str();
			else pItem->m_curValue = "";
		}

		pItem = m_LPChr.GetPropItem("Collision Mesh File");
		if(pItem)
		{
			CN3VMesh* pMC = pC->CollisionMesh();
			if(pMC) pItem->m_curValue = pMC->m_szName.c_str();
			else pItem->m_curValue = "";
		}

//		pItem = m_LPChr.GetPropItem("Collision Skin File");
//		if(pItem)
//		{
//			CN3Skin* pSkin = pC->CollisionSkin();
//			if(pSkin) pItem->m_curValue = pSkin->m_szName.c_str();
//			else pItem->m_curValue = "";
//		}

		CN3CPart* pPart = pC->Part(nPart);
		if(pPart)
		{
			pMtl = &(pPart->m_Mtl); // Material Pointer

			pItem = m_LPCPart.GetPropItem("Part File");
			if(pItem) pItem->m_curValue = pPart->m_szName.c_str();

//			pItem = m_LPCPart.GetPropItem("Part Type");
//			if(pItem) pItem->m_crColor = m_Parts[nPart].m_Type;

			pItem = m_LPCPart.GetPropItem("Texture File");
			if(pItem)
			{
				CN3Texture* pTex = pPart->Tex();
				if(pTex) pItem->m_curValue = pTex->FileName().c_str();
				else pItem->m_curValue = "";
			}
		}

		// 붙이는 오브젝트(무기, 장신구 등...) 정보 표시
		int nPlug = m_CBChrPlug.GetCurSel();
		int nPlugCount = pC->PlugCount();
		m_CBChrPlug.ResetContent();
		for(int i = 0; i < nPlugCount; i++)
		{
			CString szTmp;
			szTmp.Format("Plug : %d", i);
			m_CBChrPlug.AddString(szTmp);
		}
		if(nPlug < 0) nPlug = 0;
		if(nPlug >= nPlugCount) nPlug = nPlugCount-1;
		m_CBChrPlug.SetCurSel(nPlug);

		if(nPlug >= 0 && nPlug < nPlugCount)
		{
			CN3CPlugBase* pPlug = pC->Plug(nPlug);

			pItem = m_LPCPlug.GetPropItem("Plug Name");
			if(pItem) pItem->m_curValue = pPlug->m_szName.c_str();
			
//			pItem = m_LPCPlug.GetPropItem("Plug Type");
//			if(pItem) pItem->m_crColor = pPlug->m_Type;
			
			pItem = m_LPCPlug.GetPropItem("Plug Joint");
			if(pItem) pItem->m_curValue.Format("%d", pPlug->m_nJointIndex);
			
			pItem = m_LPCPlug.GetPropItem("Plug Offset");
			if(pItem) pItem->VectorSet(pPlug->Position());

			pItem = m_LPCPlug.GetPropItem("Plug Scale");
			if(pItem) pItem->VectorSet(pPlug->Scale());

			pItem = m_LPCPlug.GetPropItem("Plug Mesh File");
			if(pItem)
			{
				if(pPlug->PMesh()) pItem->m_curValue = pPlug->PMesh()->m_szName.c_str();
				else pItem->m_curValue = "";
			}
		
			pItem = m_LPCPlug.GetPropItem("Plug Texture File");
			if(pItem)
			{
				if(pPlug->Tex()) pItem->m_curValue = pPlug->Tex()->m_szName.c_str();
				else pItem->m_curValue = "";
			}
		}

		m_LPChr.Invalidate(); // Property List Update
		m_LPCPart.Invalidate(); // Property List Update
		m_LPCPlug.Invalidate(); // Property List Update
	}
	else
	{
		m_CBChrLOD.ShowWindow(SW_HIDE);
		m_CBChrPart.ShowWindow(SW_HIDE);
		m_CBChrPlug.ShowWindow(SW_HIDE);
		m_LPChr.ShowWindow(SW_HIDE);
		m_LPCPart.ShowWindow(SW_HIDE);
		m_LPCPlug.ShowWindow(SW_HIDE);
	}

	if(pMtl)
	{
		nH = m_LPMaterial.GetItemHeight(0) * m_LPMaterial.GetCount() + 4;
		m_LPMaterial.ShowWindow(SW_SHOW);
		m_LPMaterial.SetWindowPos(nullptr, pt.x, pt.y, nW, nH, SWP_NOZORDER);
		pt.y += nH + 3;

		CPropertyItem* pItem = nullptr;

		pItem = m_LPMaterial.GetPropItem("Render Flags");
		if(pItem) pItem->m_curValue.Format("%d", pMtl->nRenderFlags);

		pItem = m_LPMaterial.GetPropItem("Src Blend");
		if(pItem) pItem->m_crColor = pMtl->dwSrcBlend;

		pItem = m_LPMaterial.GetPropItem("Dest Blend");
		if(pItem) pItem->m_crColor = pMtl->dwDestBlend;

		pItem = m_LPMaterial.GetPropItem("Color Operation");
		if(pItem) pItem->m_crColor = pMtl->dwColorOp;

		pItem = m_LPMaterial.GetPropItem("Color Arg1");
		if(pItem) pItem->m_crColor = pMtl->dwColorArg1;

		pItem = m_LPMaterial.GetPropItem("Color Arg2");
		if(pItem) pItem->m_crColor = pMtl->dwColorArg2;

		pItem = m_LPMaterial.GetPropItem("Ambient Color");
		if(pItem) pItem->D3DColorValueSet(pMtl->Ambient);

		pItem = m_LPMaterial.GetPropItem("Diffuse Color");
		if(pItem) pItem->D3DColorValueSet(pMtl->Diffuse);

		pItem = m_LPMaterial.GetPropItem("Specular Color");
		if(pItem) pItem->D3DColorValueSet(pMtl->Specular);

		pItem = m_LPMaterial.GetPropItem("Emissive Color");
		if(pItem) pItem->D3DColorValueSet(pMtl->Emissive);

		m_LPMaterial.Invalidate();
	}
	else
	{
		m_LPMaterial.ShowWindow(SW_HIDE);
	}
}

BOOL CViewProperty::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	CN3Base* pBase = GetDocument()->m_pSelectedObj;
	if(pBase != nullptr)
	{
		if((void*)wParam == &m_LPTransform && (pBase->Type() & OBJ_TRANSFORM))
		{
			CN3Transform* pT = (CN3Transform*)pBase;

			CPropertyItem* pItem = (CPropertyItem*)lParam;
			if(pItem->m_propName == "Position") pT->PosSet(__Vector3(pItem->VectorGet()));
//			else if(pItem->m_propName == "Rotation") pT->RotSet(__Vector3(pItem->VectorGet()));
			else if(pItem->m_propName == "Scale") pT->ScaleSet(__Vector3(pItem->VectorGet()));
		}
		
		if((void*)wParam == &m_LPCamera && (pBase->Type() & OBJ_CAMERA))
		{
			CN3Camera* pC = (CN3Camera*)pBase;

			CPropertyItem* pItem = (CPropertyItem*)lParam;
			if(pItem->m_propName == "Eye") pC->EyePosSet(__Vector3(pItem->VectorGet()));
			else if(pItem->m_propName == "At") pC->AtPosSet(__Vector3(pItem->VectorGet()));
			else if(pItem->m_propName == "Up") pC->UpVectorSet(__Vector3(pItem->VectorGet()));
			else if(pItem->m_propName == "Field Of View") pC->m_Data.fFOV = (float)atof(pItem->m_curValue);
			else if(pItem->m_propName == "Near Plane") pC->m_Data.fNP = (float)atof(pItem->m_curValue);
			else if(pItem->m_propName == "Far Plane") pC->m_Data.fFP = (float)atof(pItem->m_curValue);
			else if(pItem->m_propName == "안개 사용")
			{
				if(lstrcmpi(pItem->m_curValue, "on") == 0) pC->m_bFogUse = TRUE;
				else pC->m_bFogUse = FALSE;
			}
//			else if(pItem->m_propName == "안개 밀도") pC->m_fFogDensity = (float)atof(pItem->m_curValue);
			else if(pItem->m_propName == "안개 색") pC->m_FogColor = pItem->D3DColorGet();
//			else if(pItem->m_propName == "안개 시작") pC->m_fFogStart = (float)atof(pItem->m_curValue);
//			else if(pItem->m_propName == "안개 끝") pC->m_fFogEnd = (float)atof(pItem->m_curValue);
		}

		if((void*)wParam == &m_LPLight && (pBase->Type() & OBJ_LIGHT))
		{
			CN3Light* pL = (CN3Light*)pBase;

			CPropertyItem* pItem = (CPropertyItem*)lParam;
			if(pItem->m_propName == "On")
			{
				if(pItem->m_curValue != "0" && lstrcmpi(pItem->m_curValue, "off")) pL->m_Data.bOn = TRUE;
				else pL->m_Data.bOn = FALSE;
			}
			if(pItem->m_propName == "Number") pL->m_Data.nNumber = atoi(pItem->m_curValue);
			if(pItem->m_propName == "Type")
			{
				if(pItem->m_curValue == "Point") pL->m_Data.Type = D3DLIGHT_POINT;
				else if(pItem->m_curValue == "Directional") pL->m_Data.Type = D3DLIGHT_DIRECTIONAL;
				else if(pItem->m_curValue == "Spot") pL->m_Data.Type = D3DLIGHT_SPOT;
			}
			else if(pItem->m_propName == "Direction") pL->m_Data.Direction = pItem->VectorGet();
			else if(pItem->m_propName == "Ambient Color") pL->m_Data.Ambient = pItem->D3DColorValueGet();
			else if(pItem->m_propName == "Diffuse Color") pL->m_Data.Diffuse = pItem->D3DColorValueGet();
			else if(pItem->m_propName == "Specular Color") pL->m_Data.Specular = pItem->D3DColorValueGet();
			else if(pItem->m_propName == "Range") pL->m_Data.Range = (float)atof(pItem->m_curValue);
			else if(pItem->m_propName == "Attenuation0") pL->m_Data.Attenuation0 = (float)atof(pItem->m_curValue);
			else if(pItem->m_propName == "Attenuation1") pL->m_Data.Attenuation1 = (float)atof(pItem->m_curValue);
			else if(pItem->m_propName == "Attenuation2") pL->m_Data.Attenuation2 = (float)atof(pItem->m_curValue);
		}

		if((void*)wParam == &m_LPShape && (pBase->Type() & OBJ_SHAPE))
		{
			CN3Shape* pS = (CN3Shape*)pBase;
			int nPart = m_CBShapePart.GetCurSel();
			CN3SPart* pPD = pS->Part(nPart);
			CPropertyItem* pItem = (CPropertyItem*)lParam;

			if(pItem->m_propName == "소속" && pItem->m_curValue.GetLength() > 0)			pS->m_iBelong = atoi(pItem->m_curValue);
			else if(pItem->m_propName == "Event ID" && pItem->m_curValue.GetLength() > 0)	pS->m_iEventID = atoi(pItem->m_curValue);
			else if(pItem->m_propName == "Event Type" && pItem->m_curValue.GetLength() > 0) pS->m_iEventType = atoi(pItem->m_curValue);
			else if(pItem->m_propName == "NPC ID" && pItem->m_curValue.GetLength() > 0)		pS->m_iNPC_ID = atoi(pItem->m_curValue);
			else if(pItem->m_propName == "NPC Status" && pItem->m_curValue.GetLength() > 0) pS->m_iNPC_Status = atoi(pItem->m_curValue); // NPC 로 쓰는 오브젝트일 경우 NPC Type

			else if(pItem->m_propName == "Collision Mesh File" && pItem->m_curValue.GetLength() > 0) 
				pS->CollisionMeshSet(std::string(pItem->m_curValue));

			else if(pItem->m_propName == "Collision Mesh Delete") 
				pS->CollisionMeshSet("");

			else if(pItem->m_propName == "Texture File" && pItem->m_curValue.GetLength() > 0) 
			{
				CStringArray szArr;
				int n = 0, nPrev = 0;
				while(1)
				{
					n = pItem->m_curValue.Find('\n', n);
					if(-1 == n) break;
					
					szArr.Add(pItem->m_curValue.Mid(nPrev, n - nPrev));
					nPrev = n+1;
					n++;
				}

				CN3BaseFileAccess tmp;

				int nTC = static_cast<int>(szArr.GetSize());
				pPD->TexAlloc(nTC);

				for (int i = 0; i < nTC; i++)
				{
					tmp.FileNameSet(std::string(szArr[i]));
					pPD->TexSet(i, tmp.FileName());
				}
				
				UpdateInfo();

			}

			else if(pItem->m_propName == "Texture Animaion Speed")
			{
				pPD->m_fTexFPS = (float)(atof(pItem->m_curValue));
				if(pPD->m_fTexFPS < 0.1f) pPD->m_fTexFPS = 0.1f;
			}
			
			else if(pItem->m_propName == "Mesh File" && pItem->m_curValue.GetLength() > 0)
			{
				CN3BaseFileAccess tmp;
				tmp.FileNameSet(std::string(pItem->m_curValue));
				pPD->MeshSet(tmp.FileName());

				this->UpdateInfo();
			}

			else if(pItem->m_propName == "Part Add")
			{
				pS->PartAdd();
				
				this->UpdateInfo();
			}

			else if(pItem->m_propName == "Part Delete")
			{
				pS->PartDelete(nPart);
				
				this->UpdateInfo();
			}

			else if(pItem->m_propName == "Make Smooth")
			{
				if(pPD->Mesh()) pPD->Mesh()->ReGenerateSmoothNormal();
			}

			else if(pItem->m_propName == "Make Sharp")
			{
				if(pPD->Mesh()) pPD->Mesh()->ReGenerateSharpNormal();
			}
		}

		if((void*)wParam == &m_LPChr && (pBase->Type() & OBJ_CHARACTER))
		{
			CN3Chr* pC = (CN3Chr*)pBase;
			CPropertyItem* pItem = (CPropertyItem*)lParam;

			int nLOD = m_CBChrLOD.GetCurSel();
			int nPart = m_CBChrPart.GetCurSel();
			int nPlug = m_CBChrPlug.GetCurSel();
			int nPlugCount = pC->PlugCount();
			
			if(pItem->m_propName == "Joint File" && pItem->m_curValue.GetLength() > 0)
			{
				CN3BaseFileAccess tmp;
				tmp.FileNameSet(std::string(pItem->m_curValue));
				pC->JointSet(tmp.FileName());
				this->UpdateInfo();
			}
//			else if(pItem->m_propName == "Collision Mesh File" && pItem->m_curValue.GetLength() > 0) 
//			{
//				pC->CollisionMeshSet(pItem->m_curValue);
//				this->UpdateInfo();
//			}
//			else if(pItem->m_propName == "Collision Mesh Delete") 
//			{
//				pC->CollisionMeshSet("");
//				this->UpdateInfo();
//			}
//			else if(pItem->m_propName == "Collision Skin File" && pItem->m_curValue.GetLength() > 0) 
//			{
//				pC->CollisionSkinSet(pItem->m_curValue);
//				this->UpdateInfo();
//			}
//			else if(pItem->m_propName == "Collision Skin Delete") 
//			{
//				pC->CollisionSkinSet("");
//				this->UpdateInfo();
//			}
			else if(pItem->m_propName == "Part Add")
			{
				pC->PartAdd();
				this->UpdateInfo();
			}
			else if(pItem->m_propName == "Part Delete")
			{
				pC->PartDelete(nPart);
				this->UpdateInfo();
			}

			else if(pItem->m_propName == "Plug Add")
			{
				pC->PlugAdd();
				this->UpdateInfo();
			}
			else if(pItem->m_propName == "Plug Delete")
			{
				pC->PlugDelete(nPlug);
				this->UpdateInfo();
			}
		}

		if((void*)wParam == &m_LPCPart && (pBase->Type() & OBJ_CHARACTER))
		{
			CN3Chr* pC = (CN3Chr*)pBase;
			int nLOD = m_CBChrLOD.GetCurSel();
			int nPart = m_CBChrPart.GetCurSel();
			int nPartCount = pC->PartCount();
			CN3CPart* pPart = pC->Part(nPart);

			CPropertyItem* pItem = (CPropertyItem*)lParam;
			
			if(pPart)
			{
				if(pItem->m_propName == "Part File") pPart->m_szName = pItem->m_curValue;
//				else if(pItem->m_propName == "Part Type")
//				{
//					CN3CPart* pPD = pC->Part(nPart);
//					if(pPD)
//					{
//						if(pItem->m_curValue == "머리카락") pPD->m_Type = PART_HAIR;
//						else if(pItem->m_curValue == "얼굴") pPD->m_Type = PART_FACE;
//						else if(pItem->m_curValue == "상체") pPD->m_Type = PART_UPPER;
//						else if(pItem->m_curValue == "하체") pPD->m_Type = PART_LOWER;
//						else if(pItem->m_curValue == "손") pPD->m_Type = PART_HAND;
//						else if(pItem->m_curValue == "발") pPD->m_Type = PART_FOOT;
//						else if(pItem->m_curValue == "??") pPD->m_Type = PART_UNKNOWN;
//					}
//					this->UpdateInfo();
//				}
				else if(pItem->m_propName == "Texture File" && pItem->m_curValue.GetLength() > 0)
				{
					CN3BaseFileAccess tmp;
					tmp.FileNameSet(std::string(pItem->m_curValue));
					pPart->TexSet(tmp.FileName());

					this->UpdateInfo();
				}
			}
		}
		
		if((void*)wParam == &m_LPCPlug && (pBase->Type() & OBJ_CHARACTER))
		{
			CN3Chr* pC = (CN3Chr*)pBase;
			int nPlug = m_CBChrPlug.GetCurSel();
			int nPlugCount = pC->PlugCount();
			CPropertyItem* pItem = (CPropertyItem*)lParam;
			if(nPlug >= 0 && nPlug < nPlugCount)
			{
				CN3CPlugBase* pPlug = pC->Plug(nPlug);

				if(pItem->m_propName == "Plug Name") pPlug->m_szName = pItem->m_curValue;
//				else if(pItem->m_propName == "Plug Type") pPlug->m_Type = (e_PlugType)pItem->m_crColor;
				else if(pItem->m_propName == "Plug Joint")
				{
					int nJI = atoi(pItem->m_curValue); // Joint Index 가 영역을 벗어나지 못하도록...
					int nJC = 0;
					CN3Joint* pJ = pC->Joint();
					if(pJ) pJ->NodeCount(nJC); // Joint Node Count;
					if(nJI >= 0 && nJI < nJC) pPlug->m_nJointIndex = nJI;
				}
				else if(pItem->m_propName == "Plug Offset") pPlug->PositionSet(__Vector3(pItem->VectorGet()));
				else if(pItem->m_propName == "Plug Rotation") { __Matrix44 mtx; mtx.Identity(); pPlug->RotationSet(mtx); }
				else if(pItem->m_propName == "Plug Scale") pPlug->ScaleSet(__Vector3(pItem->VectorGet()));
				else if(pItem->m_propName == "Plug Mesh File" && pItem->m_curValue.GetLength() > 0)
				{
					if(pItem->m_curValue != "")
					{
						CN3BaseFileAccess tmp;
						tmp.FileNameSet(std::string(pItem->m_curValue));
						pPlug->PMeshSet(tmp.FileName());
					}
					this->UpdateInfo();
				}
				else if(pItem->m_propName == "Plug Texture File" && pItem->m_curValue.GetLength() > 0)
				{
					CN3BaseFileAccess tmp;
					tmp.FileNameSet(std::string(pItem->m_curValue));
					pPlug->TexSet(tmp.FileName());

					this->UpdateInfo();
				}
			}
		}

		if((void*)wParam == &m_LPMaterial)
		{
			__Material* pMtl = nullptr;
			if(pBase->Type() & OBJ_SHAPE)
			{
				int nPart = m_CBShapePart.GetCurSel();
				pMtl = &(((CN3Shape*)pBase)->Part(nPart)->m_Mtl);
			}
			else if(pBase->Type() & OBJ_CHARACTER)
			{
				int nPart = m_CBChrPart.GetCurSel();
				pMtl = &(((CN3Chr*)pBase)->Part(nPart)->m_Mtl);
			}

			if(pMtl)
			{
				CPropertyItem* pItem = (CPropertyItem*)lParam;

				if(pItem->m_propName == "Render Flags") pMtl->nRenderFlags = atoi(pItem->m_curValue);
				if(pItem->m_propName == "Src Blend") pMtl->dwSrcBlend = pItem->m_crColor;
				if(pItem->m_propName == "Dest Blend") pMtl->dwDestBlend = pItem->m_crColor;
				if(pItem->m_propName == "Color Operation") pMtl->dwColorOp = pItem->m_crColor;
				if(pItem->m_propName == "Color Arg1") pMtl->dwColorArg1 = pItem->m_crColor;
				if(pItem->m_propName == "Color Arg2") pMtl->dwColorArg2 = pItem->m_crColor;
				if(pItem->m_propName == "Ambient Color") pMtl->Ambient = pItem->D3DColorValueGet();
				if(pItem->m_propName == "Diffuse Color") pMtl->Diffuse = pItem->D3DColorValueGet();
				if(pItem->m_propName == "Specular Color") pMtl->Specular = pItem->D3DColorValueGet();
				if(pItem->m_propName == "Emissive Color") pMtl->Emissive = pItem->D3DColorValueGet();
			}
		}
	}

	pFrm->GetView()->InvalidateRect(nullptr, FALSE);

	return TRUE;	
//	return CFormView::OnNotify(wParam, lParam, pResult);
}

void CViewProperty::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();

	static bool bInitCtrl = false;
	if(false == bInitCtrl)
	{
		CRect rc;
		int nH = 0;
		
		/////////////////////////////////////
		// Transform 등록 정보
		m_LPTransform.AddPropItem("Position", "", PIT_EDIT, "");
		m_LPTransform.AddPropItem("Rotation", "", PIT_EDIT, "");
		m_LPTransform.AddPropItem("Scale", "", PIT_EDIT, "");
		// Transform 등록 정보
		/////////////////////////////////////

		/////////////////////////////////////
		// Camera 등록 정보
		m_LPCamera.AddPropItem("Eye", "", PIT_EDIT, "");
		m_LPCamera.AddPropItem("At", "", PIT_EDIT, "");
		m_LPCamera.AddPropItem("Up", "", PIT_EDIT, "");
		m_LPCamera.AddPropItem("Field Of View", "", PIT_EDIT, "");
		m_LPCamera.AddPropItem("Near Plane", "", PIT_EDIT, "");
		m_LPCamera.AddPropItem("Far Plane", "", PIT_EDIT, "");
		m_LPCamera.AddPropItem("안개 사용", "", PIT_EDIT, "");
//		m_LPCamera.AddPropItem("안개 밀도", "", PIT_EDIT, "");
		m_LPCamera.AddPropItem("안개 색", "", PIT_COLOR, "");
//		m_LPCamera.AddPropItem("안개 시작", "", PIT_EDIT, "");
//		m_LPCamera.AddPropItem("안개 끝", "", PIT_EDIT, "");
		// Camera 등록 정보
		/////////////////////////////////////

		/////////////////////////////////////
		// Light 등록 정보
		m_LPLight.AddPropItem("On", "", PIT_EDIT, "");
		m_LPLight.AddPropItem("Number", "", PIT_EDIT, "");
		m_LPLight.AddPropItem("Type", "", PIT_COMBO, "Null|Point|Spot|Directional|");
		m_LPLight.AddPropItem("Direction", "", PIT_EDIT, "");
		m_LPLight.AddPropItem("Ambient Color", "", PIT_COLOR, "");
		m_LPLight.AddPropItem("Diffuse Color", "", PIT_COLOR, "");
		m_LPLight.AddPropItem("Specular Color", "", PIT_COLOR, "");
		m_LPLight.AddPropItem("Range", "", PIT_EDIT, "");
		m_LPLight.AddPropItem("Attenuation0", "", PIT_EDIT, "");
		m_LPLight.AddPropItem("Attenuation1", "", PIT_EDIT, "");
		m_LPLight.AddPropItem("Attenuation2", "", PIT_EDIT, "");
		// Light 등록 정보
		/////////////////////////////////////


		int i = 0;
		CString str, strTmp;

		/////////////////////////////////////
		// Material 등록 정보
		m_LPMaterial.AddPropItem("Render Flags", "0", PIT_EDIT, "", 0);
		strTmp = "Null|D3DBLEND_ZERO|D3DBLEND_ONE|D3DBLEND_SRCCOLOR|D3DBLEND_INVSRCCOLOR|D3DBLEND_SRCALPHA|D3DBLEND_INVSRCALPHA|D3DBLEND_DESTALPHA|\
	D3DBLEND_INVDESTALPHA|D3DBLEND_DESTCOLOR|D3DBLEND_INVDESTCOLOR|D3DBLEND_SRCALPHASAT|D3DBLEND_BOTHSRCALPHA|D3DBLEND_BOTHINVSRCALPHA|";
		m_LPMaterial.AddPropItem("Src Blend", "", PIT_COMBO, strTmp);
		m_LPMaterial.AddPropItem("Dest Blend", "", PIT_COMBO, strTmp);

		strTmp = "Null|D3DTOP_DISABLE|D3DTOP_SELECTARG1|D3DTOP_SELECTARG2|D3DTOP_MODULATE|D3DTOP_MODULATE2X|D3DTOP_MODULATE4X|\
	D3DTOP_ADD|D3DTOP_ADDSIGNED|D3DTOP_ADDSIGNED2X|\
	D3DTOP_SUBTRACT|D3DTOP_ADDSMOOTH|\
	D3DTOP_BLENDDIFFUSEALPHA|D3DTOP_BLENDTEXTUREALPHA|D3DTOP_BLENDFACTORALPHA|D3DTOP_BLENDTEXTUREALPHAPM|D3DTOP_BLENDCURRENTALPHA|\
	D3DTOP_PREMODULATE|D3DTOP_MODULATEALPHA_ADDCOLOR|D3DTOP_MODULATECOLOR_ADDALPHA|D3DTOP_MODULATEINVALPHA_ADDCOLOR|D3DTOP_MODULATEINVCOLOR_ADDALPHA|\
	D3DTOP_BUMPENVMAP|D3DTOP_BUMPENVMAPLUMINANCE|D3DTOP_DOTPRODUCT|D3DTOP_MULTIPLYADD|D3DTOP_LERP|";
		m_LPMaterial.AddPropItem("Color Operation", "", PIT_COMBO, strTmp);
		strTmp = "D3DTA_DIFFUSE|D3DTA_CURRENTD|3DTA_TEXTURE|D3DTA_TFACTOR|D3DTA_SPECULAR|";
		m_LPMaterial.AddPropItem("Color Arg1", "", PIT_COMBO, strTmp);
		m_LPMaterial.AddPropItem("Color Arg2", "", PIT_COMBO, strTmp);
		m_LPMaterial.AddPropItem("Ambient Color", "", PIT_COLOR, "");
		m_LPMaterial.AddPropItem("Diffuse Color", "", PIT_COLOR, "");
		m_LPMaterial.AddPropItem("Specular Color", "", PIT_COLOR, "");
		m_LPMaterial.AddPropItem("Emissive Color", "", PIT_COLOR, "");
		// Material 등록 정보
		/////////////////////////////////////

		
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Shape
		m_CBShapePart.ResetContent();
	//	int nPartCount = 
	//	for(i = 0; i < nPartCount; i++) { strTmp.Format("%d", i); m_CBShapePart.AddString(strTmp); }
	//	m_CBShapePart.SetCurSel(0);

		m_LPShape.AddPropItem("소속", "", PIT_EDIT, "");
		m_LPShape.AddPropItem("Event ID", "", PIT_EDIT, "");
		m_LPShape.AddPropItem("Event Type", "", PIT_EDIT, "");
		m_LPShape.AddPropItem("NPC ID", "", PIT_EDIT, "");
		m_LPShape.AddPropItem("NPC Status", "", PIT_EDIT, "");

		m_LPShape.AddPropItem("Part Add", "Part 추가", PIT_BUTTON, "");
		m_LPShape.AddPropItem("Part Delete", "Part 삭제", PIT_BUTTON, "");

		m_LPShape.AddPropItem("Face, Vertex Count", "", PIT_EDIT, "");
		m_LPShape.AddPropItem("Texture File", "", PIT_FILE_MULTI, "Texture 로 쓸수 있는 그림 파일(*.DXT; *.BMP; *.TGA)|*.DXT; *.BMP; *.TGA|");
		m_LPShape.AddPropItem("Texture Animaion Speed", "", PIT_EDIT, "");
		m_LPShape.AddPropItem("Mesh File", "", PIT_FILE, "N3 Progressive Mesh File(*.N3PMesh)|*.N3PMesh||");
		m_LPShape.AddPropItem("Make Smooth", "", PIT_BUTTON, "");
		m_LPShape.AddPropItem("Make Sharp", "", PIT_BUTTON, "");
		m_LPShape.AddPropItem("Collision Mesh File", "", PIT_FILE, "N3 Vector Mesh(*.N3VMesh)|*.N3VMesh||");
		m_LPShape.AddPropItem("Collision Mesh Delete", "Collision Mesh 삭제", PIT_BUTTON, "");
		// Shape
		/////////////////////////////////////////////////////////////////////////////////////////////////////////


		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 캐릭터
		m_LPChr.AddPropItem("Joint File", "", PIT_FILE, "N3 Joint File(*.N3Joint)|*.N3Joint||");
		m_LPChr.AddPropItem("Collision Mesh File", "", PIT_FILE, "N3 Vector Mesh(*.N3VMesh)|*.N3VMesh||");
		m_LPChr.AddPropItem("Collision Mesh Delete", "Collision Mesh 삭제", PIT_BUTTON, "");
	//	m_LPChr.AddPropItem("Collision Skin File", "", PIT_FILE, "N3 Skin File(*.N3Skin)|*.N3Skin||");
	//	m_LPChr.AddPropItem("Collision Skin Delete", "Collision Skin 삭제", PIT_BUTTON, "");

		m_LPChr.AddPropItem("Part Add", "Part 추가", PIT_BUTTON, "");
		m_LPChr.AddPropItem("Part Delete", "Part 삭제", PIT_BUTTON, "");
		m_LPChr.AddPropItem("Plug Add", "Plug 추가", PIT_BUTTON, "");
		m_LPChr.AddPropItem("Plug Delete", "Plug 삭제", PIT_BUTTON, "");

		m_CBChrPart.ResetContent();
		m_CBChrPart.SetCurSel(0);

		m_CBChrLOD.ResetContent();
		for(i = 0; i < MAX_CHR_LOD; i++) { strTmp.Format("LOD : %d", i); m_CBChrLOD.AddString(strTmp); }
		m_CBChrLOD.SetCurSel(0);

		m_LPCPart.AddPropItem("Part File", "", PIT_FILE, "N3 Character part file(*.N3CPart)|*.N3CPart||");
//		m_LPCPart.AddPropItem("Part Type", "", PIT_COMBO, "머리카락|얼굴|상체|하체|손|발|??|");
		m_LPCPart.AddPropItem("Texture File", "", PIT_FILE, "Texture 로 쓸수 있는 그림 파일(*.DXT; *.BMP; *.TGA)|*.DXT; *.BMP; *.TGA|");

		m_LPCPlug.AddPropItem("Plug Name", "", PIT_EDIT, "");
//		m_LPCPlug.AddPropItem("Plug Type", "", PIT_COMBO, "오른손장착|왼손장착|양손장착");
		m_LPCPlug.AddPropItem("Plug Joint", "", PIT_EDIT, "");
		m_LPCPlug.AddPropItem("Plug Mesh File", "", PIT_FILE, "N3 Progressive Mesh File(*.N3PMesh)|*.N3PMesh||");
		m_LPCPlug.AddPropItem("Plug Texture File", "", PIT_FILE, "Texture 로 쓸수 있는 그림 파일(*.DXT; *.BMP; *.TGA)|*.DXT; *.BMP; *.TGA|");
		m_LPCPlug.AddPropItem("Plug Offset", "", PIT_EDIT, "");
		m_LPCPlug.AddPropItem("Plug Rotation", "Reset", PIT_BUTTON, "");
		m_LPCPlug.AddPropItem("Plug Scale", "", PIT_EDIT, "");
		// 캐릭터
		/////////////////////////////////////////////////////////////////////////////////////////////////////////

		int nW = 100;
		m_LPTransform.SetDividerWidth(nW);
		m_LPCamera.SetDividerWidth(nW);
		m_LPLight.SetDividerWidth(nW);
		m_LPMaterial.SetDividerWidth(nW);
		m_LPShape.SetDividerWidth(nW);
		m_LPChr.SetDividerWidth(nW);
		m_LPCPart.SetDividerWidth(nW);
		m_LPCPlug.SetDividerWidth(nW);

		bInitCtrl = true;
	}
	this->UpdateInfo();
}

void CViewProperty::OnSize(UINT nType, int cx, int cy) 
{
	CFormView::OnSize(nType, cx, cy);
	
	this->UpdateInfo();
}

void CViewProperty::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	this->UpdateInfo();
}

void CViewProperty::OnSelchangeCbPartShape() 
{
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	pFrm->GetView()->InvalidateRect(nullptr, FALSE);

	this->UpdateInfo();
}

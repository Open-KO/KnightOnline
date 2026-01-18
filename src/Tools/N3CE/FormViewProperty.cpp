// FormViewProperty.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include <N3Base/N3Chr.h>

#include "MainFrm.h"
#include "N3CEDoc.h"
#include "N3CEView.h"
#include "FormViewAnimation.h"

#include "FormViewProperty.h"
#include <N3Base/N3FXPlug.h>
#include <N3Base/N3FXBundle.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CFormViewProperty, CFormView)

CFormViewProperty::CFormViewProperty() : CFormView(CFormViewProperty::IDD)
{
	//{{AFX_DATA_INIT(CFormViewProperty)
	//}}AFX_DATA_INIT
}

CFormViewProperty::~CFormViewProperty()
{
}

/////////////////////////////////////////////////////////////////////////////
// CFormViewProperty diagnostics

#ifdef _DEBUG
void CFormViewProperty::AssertValid() const
{
	CFormView::AssertValid();
}

void CFormViewProperty::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CN3CEDoc* CFormViewProperty::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CN3CEDoc)));
	return (CN3CEDoc*) m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFormViewProperty message handlers

void CFormViewProperty::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	static bool bInitCtrl = false;
	if (false == bInitCtrl)
	{
		CRect rc;
		this->GetClientRect(&rc);
		/////////////////////////////////////
		// Material 등록 정보
		CString strTmp;

		m_LPMtl.AddPropItem("Render Flags", "0", PIT_EDIT, "", 0);
		//		strTmp = "Null|D3DBLEND_ZERO|D3DBLEND_ONE|D3DBLEND_SRCCOLOR|D3DBLEND_INVSRCCOLOR|D3DBLEND_SRCALPHA|D3DBLEND_INVSRCALPHA|D3DBLEND_DESTALPHA|\
//D3DBLEND_INVDESTALPHA|D3DBLEND_DESTCOLOR|D3DBLEND_INVDESTCOLOR|D3DBLEND_SRCALPHASAT|D3DBLEND_BOTHSRCALPHA|D3DBLEND_BOTHINVSRCALPHA|";
		//		m_LPMtl.AddPropItem("Src Blend", "", PIT_COMBO, strTmp);
		//		m_LPMtl.AddPropItem("Dest Blend", "", PIT_COMBO, strTmp);

		//		strTmp = "Null|D3DTOP_DISABLE|D3DTOP_SELECTARG1|D3DTOP_SELECTARG2|D3DTOP_MODULATE|D3DTOP_MODULATE2X|D3DTOP_MODULATE4X|\
//D3DTOP_ADD|D3DTOP_ADDSIGNED|D3DTOP_ADDSIGNED2X|\
//D3DTOP_SUBTRACT|D3DTOP_ADDSMOOTH|\
//D3DTOP_BLENDDIFFUSEALPHA|D3DTOP_BLENDTEXTUREALPHA|D3DTOP_BLENDFACTORALPHA|D3DTOP_BLENDTEXTUREALPHAPM|D3DTOP_BLENDCURRENTALPHA|\
//D3DTOP_PREMODULATE|D3DTOP_MODULATEALPHA_ADDCOLOR|D3DTOP_MODULATECOLOR_ADDALPHA|D3DTOP_MODULATEINVALPHA_ADDCOLOR|D3DTOP_MODULATEINVCOLOR_ADDALPHA|\
//D3DTOP_BUMPENVMAP|D3DTOP_BUMPENVMAPLUMINANCE|D3DTOP_DOTPRODUCT|D3DTOP_MULTIPLYADD|D3DTOP_LERP|";
		//		m_LPMtl.AddPropItem("Color Operation", "", PIT_COMBO, strTmp);
		//		strTmp = "D3DTA_DIFFUSE|D3DTA_CURRENTD|3DTA_TEXTURE|D3DTA_TFACTOR|D3DTA_SPECULAR|";
		//		m_LPMtl.AddPropItem("Color Arg1", "", PIT_COMBO, strTmp);
		//		m_LPMtl.AddPropItem("Color Arg2", "", PIT_COMBO, strTmp);
		m_LPMtl.AddPropItem("Ambient Color", "", PIT_COLOR, "");
		m_LPMtl.AddPropItem("Diffuse Color", "", PIT_COLOR, "");
		m_LPMtl.AddPropItem("Specular Color", "", PIT_COLOR, "");
		m_LPMtl.AddPropItem("Emissive Color", "", PIT_COLOR, "");
		m_LPMtl.AddPropItem("Texture", "", PIT_FILE, "");

		m_LPMtl.SetDividerWidth(100); // 중간 분할선 조정
		// Material 등록 정보
		/////////////////////////////////////

		/////////////////////////////////////
		// Plug 등록 정보
		m_LPPlug.AddPropItem("Joint", "", PIT_BUTTON, "", 0);
		m_LPPlug.AddPropItem("Position", "", PIT_BUTTON, "", 0);
		m_LPPlug.AddPropItem("Scale", "", PIT_BUTTON, "", 0);
		m_LPPlug.AddPropItem("Rotation", "", PIT_BUTTON, "", 0);
		m_LPPlug.AddPropItem("Mesh", "", PIT_FILE, "Progressive Mesh(*.N3PMesh)|*.N3PMesh||", 0);
		m_LPPlug.AddPropItem("Texture", "", PIT_FILE, "Texture(*.DXT)|*.DXT||", 0);
		m_LPPlug.AddPropItem("Trace Step", "", PIT_EDIT, "", 0);
		m_LPPlug.AddPropItem("Trace Color", "", PIT_COLOR, "", 0);
		m_LPPlug.AddPropItem("Trace 0", "", PIT_EDIT, "", 0);
		m_LPPlug.AddPropItem("Trace 1", "", PIT_EDIT, "", 0);
		m_LPPlug.AddPropItem("FX Board Size", "", PIT_EDIT, "", 0);
		m_LPPlug.AddPropItem("FX Board Offset", "", PIT_EDIT, "", 0);
		m_LPPlug.AddPropItem("FX Line Count", "", PIT_EDIT, "", 0);
		strTmp = "";
		for (int i = 0; i < MAX_PLUG_FX_POSITION; i++)
		{
			strTmp += (char) ('0' + i);
			strTmp += "|";
		}
		m_LPPlug.AddPropItem("FX Pos", "", PIT_COMBO, strTmp);

		m_LPPlug.SetDividerWidth(100); // 중간 분할선 조정
		// Plug 등록 정보
		/////////////////////////////////////

		//////////////////////////////////////////////////
		//	Coded (By Dino On 2002-10-11 오전 9:53:37 )
		//	FX Plug 등록 정보
		m_LPFXPlugPart.AddPropItem("FXB", "", PIT_FILE, "FX Bundle(*.FXB)|*.FXB||", 0);
		m_LPFXPlugPart.AddPropItem("Referance Index", "", PIT_EDIT, "", 0);
		m_LPFXPlugPart.AddPropItem("Offset Pos", "", PIT_EDIT, "", 0);
		m_LPFXPlugPart.AddPropItem("Offset Dir", "", PIT_EDIT, "", 0);

		m_LPFXPlugPart.SetDividerWidth(100);
		//	End Of Code (By Dino On 2002-10-11 오전 9:53:37 )
		//////////////////////////////////////////////////

		// Tree Control 에 이미지 리스트 연결
		m_ImgList.Create(IDB_N3BASE_NODES, 20, 16, RGB(255, 0, 255));
		m_TreeChr.SetImageList(&m_ImgList, TVSIL_NORMAL);

		bInitCtrl = true;
	}

	this->UpdateAllInfo();
}

void CFormViewProperty::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFormViewProperty)
	DDX_Control(pDX, IDC_LP_FXPLUG_PART, m_LPFXPlugPart);
	DDX_Control(pDX, IDC_TREE_CHR, m_TreeChr);
	DDX_Control(pDX, IDC_LP_MATERIAL, m_LPMtl);
	DDX_Control(pDX, IDC_LP_PLUG, m_LPPlug);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFormViewProperty, CFormView)
//{{AFX_MSG_MAP(CFormViewProperty)
ON_WM_SIZE()
ON_NOTIFY(TVN_ENDLABELEDIT, IDC_TREE_CHR, OnTreeChrEndLabelEdit)
ON_NOTIFY(NM_DBLCLK, IDC_TREE_CHR, OnTreeChrDblclk)
ON_NOTIFY(NM_RCLICK, IDC_TREE_CHR, OnTreeChrRclick)
ON_COMMAND(ID_EDIT_CHR_JOINT_SET, OnEditChrJointSet)
ON_COMMAND(ID_EDIT_CHR_PART_ADD, OnEditChrPartAdd)
ON_COMMAND(ID_EDIT_CHR_PART_DELETE, OnEditChrPartDelete)
ON_COMMAND(ID_EDIT_CHR_PLUG_ADD, OnEditChrPlugAdd)
ON_COMMAND(ID_EDIT_CHR_PLUG_DELETE, OnEditChrPlugDelete)
ON_COMMAND(ID_EDIT_CHR_PART_SET, OnEditChrPartSet)
ON_COMMAND(ID_EDIT_CHR_PLUG_SET, OnEditChrPlugSet)
ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_CHR, OnSelchangedTreeChr)
ON_COMMAND(ID_FILE_SAVE_SELECTED, OnFileSaveSelected)
ON_COMMAND(ID_EDIT_CHR_PLUG_ROTATION_RESET, OnEditChrPlugRotationReset)
ON_COMMAND(ID_EDIT_CHR_PLUG_ADDCLOAK, OnEditChrPlugAddcloak)
ON_COMMAND(ID_EDIT_SET_VISIBLE, OnEditSetVisible)
ON_UPDATE_COMMAND_UI(ID_EDIT_SET_VISIBLE, OnUpdateEditSetVisible)
ON_COMMAND(ID_EDIT_CHR_PLUG_IMPORT_PMESH, OnEditChrPlugImportPMesh)
ON_COMMAND(ID_EDIT_FXPLUG_SET, OnEditFxplugSet)
ON_COMMAND(ID_EDIT_FXPLUG_DELETE, OnEditFxplugDelete)
ON_COMMAND(ID_EDIT_FXPLUG_PART_ADD, OnEditFxplugPartAdd)
ON_COMMAND(ID_EDIT_FXPLUG_PART_DELETE, OnEditFxplugPartDelete)
ON_COMMAND(ID_EDIT_FXPLUG_START, OnEditFxplugStart)
ON_COMMAND(ID_EDIT_FXPLUG_STOP, OnEditFxplugStop)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFormViewProperty message handlers
void CFormViewProperty::UpdateWindowPos()
{
	// 일단 숨기고..
	m_LPPlug.ShowWindow(SW_HIDE);
	m_LPMtl.ShowWindow(SW_HIDE);
	m_LPFXPlugPart.ShowWindow(SW_HIDE);

	CN3Base* pBase = this->GetSelectedObject();

	CRect rc;
	this->GetClientRect(&rc);
	int x            = rc.left + 3;
	int y            = rc.bottom - 5;
	int cx           = rc.Width() - 6;

	__Material* pMtl = nullptr;
	CN3Texture* pTex = nullptr;
	if (pBase && pBase->Type() & OBJ_CHARACTER_PART)
	{
		CN3CPart* pCPart = (CN3CPart*) pBase;
		pMtl             = &(pCPart->m_Mtl);
		pTex             = pCPart->Tex();
	}
	else if (pBase && pBase->Type() & OBJ_CHARACTER_PLUG)
	{
		CN3CPlug* pPlug  = (CN3CPlug*) pBase;
		pMtl             = &(pPlug->m_Mtl);
		pTex             = pPlug->Tex();

		int h            = m_LPPlug.GetCount() * m_LPPlug.GetItemHeight(0);
		y               -= h;
		m_LPPlug.SetWindowPos(nullptr, x, y, cx, h, SWP_NOZORDER);
		m_LPPlug.ShowWindow(SW_SHOW);
		y -= 5;
	}

	if (pMtl || pTex)
	{
		int h  = m_LPMtl.GetCount() * m_LPMtl.GetItemHeight(0);
		y     -= h;
		m_LPMtl.SetWindowPos(nullptr, x, y, cx, h, SWP_NOZORDER);
		m_LPMtl.ShowWindow(SW_SHOW);
		y -= 5;
	}

	//////////////////////////////////////////////////
	//	Coded (By Dino On 2002-10-11 오전 10:24:21 )
	//	FXPlugPart
	if (pBase && pBase->Type() & OBJ_FX_PLUG_PART)
	{
		int h  = m_LPFXPlugPart.GetCount() * m_LPFXPlugPart.GetItemHeight(0);
		y     -= h;
		m_LPFXPlugPart.SetWindowPos(nullptr, x, y, cx, h, SWP_NOZORDER);
		m_LPFXPlugPart.ShowWindow(SW_SHOW);
		y -= 5;
	}
	//	End Of Code (By Dino On 2002-10-11 오전 10:24:21 )
	//////////////////////////////////////////////////

	m_TreeChr.SetWindowPos(nullptr, x, 5, cx, y, SWP_NOZORDER);

	this->UpdateInfo();
}

void CFormViewProperty::UpdateInfo()
{
	CN3Base* pBase = this->GetSelectedObject();
	if (nullptr == pBase)
		return;

	CPropertyItem* pItem = nullptr;

	__Material* pMtl     = nullptr;
	CN3Texture* pTex     = nullptr;
	if (pBase->Type() & OBJ_CHARACTER_PART)
	{
		CN3CPart* pCPart = (CN3CPart*) pBase;
		pMtl             = &(pCPart->m_Mtl);
		pTex             = pCPart->Tex();
	}
	else if (pBase->Type() & OBJ_CHARACTER_PLUG)
	{
		CN3CPlug* pPlug = (CN3CPlug*) pBase;
		pMtl            = &(pPlug->m_Mtl);
		pTex            = pPlug->Tex();

		pItem           = m_LPPlug.GetPropItem("Joint");
		if (pItem)
			pItem->m_curValue.Format("%d", pPlug->m_nJointIndex);
		pItem = m_LPPlug.GetPropItem("Position");
		if (pItem)
			pItem->VectorSet(pPlug->m_vPosition);
		pItem = m_LPPlug.GetPropItem("Scale");
		if (pItem)
			pItem->VectorSet(pPlug->m_vScale);
		pItem = m_LPPlug.GetPropItem("Rotation");
		if (pItem)
			pItem->m_curValue.Format("");
		pItem = m_LPPlug.GetPropItem("Mesh");
		if (pItem)
		{
			if (pPlug->PMesh())
				pItem->m_curValue = pPlug->PMesh()->FileName().c_str();
			else
				pItem->m_curValue = "";
		}
		pItem = m_LPPlug.GetPropItem("Texture");
		if (pItem)
		{
			if (pPlug->Tex())
				pItem->m_curValue = pPlug->Tex()->FileName().c_str();
			else
				pItem->m_curValue = "";
		}
		pItem = m_LPPlug.GetPropItem("Trace Step");
		if (pItem)
			pItem->m_curValue.Format("%d", pPlug->m_nTraceStep);
		pItem = m_LPPlug.GetPropItem("Trace Color");
		if (pItem)
			pItem->D3DColorSet(pPlug->m_crTrace);
		pItem = m_LPPlug.GetPropItem("Trace 0");
		if (pItem)
			pItem->m_curValue.Format("%.2f", pPlug->m_fTrace0);
		pItem = m_LPPlug.GetPropItem("Trace 1");
		if (pItem)
			pItem->m_curValue.Format("%.2f", pPlug->m_fTrace1);
		pItem = m_LPPlug.GetPropItem("FX Board Size");
		if (pItem)
			pItem->m_curValue.Format("%.2f", 0);
		pItem = m_LPPlug.GetPropItem("FX Board Offset");
		if (pItem)
			pItem->m_curValue.Format("%.2f", 0);
		pItem = m_LPPlug.GetPropItem("FX Line Count");
		if (pItem)
			pItem->m_curValue.Format("%d", 0);
	}
	//////////////////////////////////////////////////
	//	Coded (By Dino On 2002-10-11 오전 10:31:36 )
	//	FXPlugPart
	else if (pBase->Type() & OBJ_FX_PLUG_PART)
	{
		CN3FXPlugPart* pFXPPart = (CN3FXPlugPart*) pBase;

		pItem                   = m_LPFXPlugPart.GetPropItem("FXB");
		if (pItem)
		{
			if (pFXPPart->GetFXB())
				pItem->m_curValue = pFXPPart->GetFXB()->FileName().c_str();
			else
				pItem->m_curValue = "";
		}
		pItem = m_LPFXPlugPart.GetPropItem("Referance Index");
		if (pItem)
			pItem->m_curValue.Format("%d", pFXPPart->m_nRefIndex);
		pItem = m_LPFXPlugPart.GetPropItem("Offset Pos");
		if (pItem)
			pItem->VectorSet(pFXPPart->m_vOffsetPos);
		pItem = m_LPFXPlugPart.GetPropItem("Offset Dir");
		if (pItem)
			pItem->VectorSet(pFXPPart->m_vOffsetDir);

		m_LPFXPlugPart.Invalidate();
	}
	//	End Of Code (By Dino On 2002-10-11 오전 10:31:36 )
	//////////////////////////////////////////////////

	// 재질
	if (pMtl)
	{
		pItem = m_LPMtl.GetPropItem("Render Flags");
		if (pItem)
			pItem->m_curValue.Format("%d", pMtl->nRenderFlags);

		//		pItem = m_LPMtl.GetPropItem("Src Blend");
		//		if(pItem) pItem->m_crColor = pMtl->dwSrcBlend;

		//		pItem = m_LPMtl.GetPropItem("Dest Blend");
		//		if(pItem) pItem->m_crColor = pMtl->dwDestBlend;

		//		pItem = m_LPMtl.GetPropItem("Color Operation");
		//		if(pItem) pItem->m_crColor = pMtl->dwColorOp;

		//		pItem = m_LPMtl.GetPropItem("Color Arg1");
		//		if(pItem) pItem->m_crColor = pMtl->dwColorArg1;

		//		pItem = m_LPMtl.GetPropItem("Color Arg2");
		//		if(pItem) pItem->m_crColor = pMtl->dwColorArg2;

		pItem = m_LPMtl.GetPropItem("Ambient Color");
		if (pItem)
			pItem->D3DColorValueSet(pMtl->Ambient);

		pItem = m_LPMtl.GetPropItem("Diffuse Color");
		if (pItem)
			pItem->D3DColorValueSet(pMtl->Diffuse);

		pItem = m_LPMtl.GetPropItem("Specular Color");
		if (pItem)
			pItem->D3DColorValueSet(pMtl->Specular);

		pItem = m_LPMtl.GetPropItem("Emissive Color");
		if (pItem)
			pItem->D3DColorValueSet(pMtl->Emissive);
	}

	if (pTex)
	{
		pItem = m_LPMtl.GetPropItem("Texture");
		if (pItem)
			pItem->m_curValue = pTex->FileName().c_str();
	}
}

BOOL CFormViewProperty::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if ((void*) wParam == &m_LPMtl)
	{
		CN3Base* pBase = this->GetSelectedObject();
		if (nullptr == pBase)
			return FALSE;

		CPropertyItem* pItem = (CPropertyItem*) lParam;

		__Material* pMtl     = nullptr;
		if (pBase->Type() & OBJ_CHARACTER_PART)
			pMtl = &(((CN3CPart*) pBase)->m_Mtl);
		else if (pBase->Type() & OBJ_CHARACTER_PLUG)
			pMtl = &(((CN3CPlug*) pBase)->m_Mtl);

		if (pMtl)
		{
			if (pItem->m_propName == "Render Flags")
				pMtl->nRenderFlags = atoi(pItem->m_curValue);
			//			if(pItem->m_propName == "Src Blend") pMtl->dwSrcBlend = pItem->m_crColor;
			//			if(pItem->m_propName == "Dest Blend") pMtl->dwDestBlend = pItem->m_crColor;
			//			if(pItem->m_propName == "Color Operation") pMtl->dwColorOp = pItem->m_crColor;
			//			if(pItem->m_propName == "Color Arg1") pMtl->dwColorArg1 = pItem->m_crColor;
			//			if(pItem->m_propName == "Color Arg2") pMtl->dwColorArg2 = pItem->m_crColor;
			if (pItem->m_propName == "Ambient Color")
				pMtl->Ambient = pItem->D3DColorValueGet();
			if (pItem->m_propName == "Diffuse Color")
				pMtl->Diffuse = pItem->D3DColorValueGet();
			if (pItem->m_propName == "Specular Color")
				pMtl->Specular = pItem->D3DColorValueGet();
			if (pItem->m_propName == "Emissive Color")
				pMtl->Emissive = pItem->D3DColorValueGet();

			CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
			pFrm->GetPaneRender()->InvalidateRect(nullptr, FALSE);
		}
		if (pItem->m_propName == "Texture")
		{
			if (pBase->Type() & OBJ_CHARACTER_PART)
				((CN3CPart*) pBase)->TexSet((LPCTSTR) pItem->m_curValue);
			else if (pBase->Type() & OBJ_CHARACTER_PLUG)
				((CN3CPlug*) pBase)->TexSet((LPCTSTR) pItem->m_curValue);
			UpdateInfo();
		}
	}
	else if ((void*) wParam == &m_LPPlug)
	{
		CN3Base* pBase = this->GetSelectedObject();
		if (nullptr == pBase || !(pBase->Type() & OBJ_CHARACTER_PLUG))
			return FALSE;

		CMainFrame* pFrm                     = (CMainFrame*) AfxGetMainWnd();
		CPropertyItem* pItem                 = (CPropertyItem*) lParam;

		CN3CPlug* pPlug                      = (CN3CPlug*) pBase;
		pFrm->GetPaneRender()->m_eCursorMode = eCM_Nothing; // 커서모드를 보통으로 해놓고..
		if (pItem->m_propName == "Joint")
			pFrm->GetPaneRender()->m_eCursorMode = eCM_PickJoint;
		else if (pItem->m_propName == "Position")
			pFrm->GetPaneRender()->m_eCursorMode = eCM_PlugPosition;
		else if (pItem->m_propName == "Scale")
			pFrm->GetPaneRender()->m_eCursorMode = eCM_PlugScale;
		else if (pItem->m_propName == "Rotation")
			pFrm->GetPaneRender()->m_eCursorMode = eCM_PlugRotation;
		else if (pItem->m_propName == "Mesh")
			pPlug->PMeshSet((const char*) pItem->m_curValue);
		else if (pItem->m_propName == "Texture")
			pPlug->TexSet((const char*) pItem->m_curValue);
		else if (pItem->m_propName == "Trace Step")
		{
			pPlug->m_nTraceStep = atoi(pItem->m_curValue);
			CN3Chr* pChr        = this->GetDocument()->m_Scene.ChrGet(0);
			pChr->RemakePlugTracePolygons();
		}
		else if (pItem->m_propName == "Trace Color")
			pPlug->m_crTrace = pItem->D3DColorGet();
		else if (pItem->m_propName == "Trace 0")
			pPlug->m_fTrace0 = atof(pItem->m_curValue);
		else if (pItem->m_propName == "Trace 1")
			pPlug->m_fTrace1 = atof(pItem->m_curValue);
		//else if(pItem->m_propName == "FX Board Size") //pPlug->m_fFXSize = atof(pItem->m_curValue);
		//else if(pItem->m_propName == "FX Board Offset") pPlug->m_fFXOffset = atof(pItem->m_curValue);
		/*
		else if(pItem->m_propName == "FX Line Count") 
		{
			int iFXPosCount = atoi(pItem->m_curValue);
			if(iFXPosCount >= 0 && iFXPosCount < MAX_PLUG_FX_POSITION / 2)
				pPlug->m_iFXPosCount = iFXPosCount;
		}
		else if(pItem->m_propName == "FX Pos")
		{
			pFrm->GetPaneRender()->m_eCursorMode = (e_CursorMode)(eCM_PlugFXPosition0 + pItem->m_crColor);
			pFrm->GetPaneRender()->InitFXPos();

		}
		*/

		pFrm->GetPaneRender()->InvalidateRect(nullptr, FALSE);
	}
	//////////////////////////////////////////////////
	//	Coded (By Dino On 2002-10-11 오전 10:37:54 )
	//	FXPlugPart
	else if ((void*) wParam == &m_LPFXPlugPart)
	{
		CN3Base* pBase = this->GetSelectedObject();
		if (nullptr == pBase || !(pBase->Type() & OBJ_FX_PLUG_PART))
			return FALSE;
		CPropertyItem* pItem    = (CPropertyItem*) lParam;

		CN3FXPlugPart* pFXPPart = (CN3FXPlugPart*) pBase;
		if ("FXB" == pItem->m_propName)
		{
			pFXPPart->SetFXB((LPCTSTR) pItem->m_curValue);
			if (pFXPPart->GetFXB())
				pItem->m_curValue = pFXPPart->GetFXB()->FileName().c_str();
			else
				pItem->m_curValue = "";
		}
		else if ("Referance Index" == pItem->m_propName)
		{
			CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
			CN3CEDoc* pDoc   = (CN3CEDoc*) pFrm->GetActiveDocument();
			CN3Chr* pChr     = pDoc->m_Scene.ChrGet(0);

			int nRefIndex    = atoi(pItem->m_curValue);
			if (nRefIndex > 0)
				pFXPPart->SetRefIdx(nRefIndex);
		}
		else if ("Offset Pos" == pItem->m_propName)
			pFXPPart->m_vOffsetPos = pItem->VectorGet();
		else if ("Offset Dir" == pItem->m_propName)
		{
			pFXPPart->m_vOffsetDir = pItem->VectorGet();
			pFXPPart->m_vOffsetDir.Normalize();
			pItem->VectorSet(pFXPPart->m_vOffsetDir);
		}
	}
	//	End Of Code (By Dino On 2002-10-11 오전 10:37:54 )
	//////////////////////////////////////////////////
	else
	{
		CFormView::OnNotify(wParam, lParam, pResult);
	}

	return TRUE;
}

void CFormViewProperty::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);
	if (m_TreeChr.GetSafeHwnd() == nullptr)
		return;

	this->UpdateWindowPos();
}

void CFormViewProperty::UpdateAllInfo()
{
	if (m_TreeChr.GetSafeHwnd() == nullptr)
		return;

	int nNodeCur     = this->GetSelectedNode(); // 현재 선택되어 있는 노드 번호..

	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	CN3CEDoc* pDoc   = (CN3CEDoc*) pFrm->GetActiveDocument();
	CN3Chr* pChr     = pDoc->m_Scene.ChrGet(0);

	// Character Tree 에 노드 삽입..
	m_TreeChr.DeleteAllItems(); // 모두 지우고..
	m_hTI_Joint  = m_TreeChr.InsertItem("Joints", 9, 9);
	m_hTI_Parts  = m_TreeChr.InsertItem("Parts", 10, 10);
	m_hTI_Plugs  = m_TreeChr.InsertItem("Plugs", 11, 11);
	m_hTI_FXPlug = m_TreeChr.InsertItem("FXPlug", 16, 16);
	m_TreeChr.Expand(TVI_ROOT, TVE_EXPAND);

	if (nullptr == pChr)
		return;
	HTREEITEM hInsert = nullptr;
	//////////////////////////////////////////////////
	//	Coded (By Dino On 2002-10-10 오후 4:44:49 )
	//	FXPlug
	CN3FXPlug* pFXP   = pChr->FXPlugGet();
	if (pFXP != nullptr)
	{
		for (size_t i = 0; i < pFXP->m_FXPParts.size(); i++)
		{
			CN3FXPlugPart* pPart = pFXP->m_FXPParts[i];

			TCHAR szName[_MAX_PATH] {};
			_stprintf(szName, _T("FXPlugPart_%03zu"), i);
			hInsert = m_TreeChr.InsertItem(szName, 11, 11, m_hTI_FXPlug);
			m_TreeChr.SetItemData(hInsert, (DWORD_PTR) pPart);
		}
	}
	//	End Of Code (By Dino On 2002-10-10 오후 4:44:49 )
	//////////////////////////////////////////////////

	CN3Joint* pJoint = pChr->Joint();
	if (pJoint)
	{
		CString szJN;
		szJN.Format("Joints - 상체(%d ~ %d), 하체(%d ~ %d), 머리카락(%d ~ %d), 망토(%d ~ %d)",
			pChr->JointPartStart(0), pChr->JointPartEnd(0), pChr->JointPartStart(1),
			pChr->JointPartEnd(1), pChr->JointPartStart(2), pChr->JointPartEnd(2),
			pChr->JointPartStart(3), pChr->JointPartEnd(3));
		m_TreeChr.SetItemText(m_hTI_Joint, szJN);
	}
	int iNumber = 0;
	this->UpdateJointItem(m_hTI_Joint, pJoint, iNumber); // Joint item update

	int nPC = pChr->PartCount();
	for (int i = 0; i < nPC; i++)
	{
		if (nullptr == pChr->Part(i))
		{
			hInsert = m_TreeChr.InsertItem("Null Pointer!!!", 10, 10, m_hTI_Parts);
			continue;
		}

		hInsert = m_TreeChr.InsertItem(pChr->Part(i)->FileName().c_str(), 10, 10, m_hTI_Parts);
		m_TreeChr.SetItemData(hInsert, (DWORD_PTR) pChr->Part(i));
	}

	m_TreeChr.Expand(m_hTI_Parts, TVE_EXPAND);

	nPC = pChr->PlugCount();
	for (int i = 0; i < nPC; i++)
	{
		if (nullptr == pChr->Plug(i))
		{
			hInsert = m_TreeChr.InsertItem("Null Pointer!!!", 11, 11, m_hTI_Plugs);
			continue;
		}
		CN3CPlugBase* pPlug = pChr->Plug(i);

		hInsert             = m_TreeChr.InsertItem(pPlug->FileName().c_str(), 11, 11, m_hTI_Plugs);
		m_TreeChr.SetItemData(hInsert, (DWORD_PTR) pPlug);

		CString szTmp;
	}
	m_TreeChr.Expand(m_hTI_Plugs, TVE_EXPAND);

	this->SelectNode(nNodeCur); // 전의 선택된 노드 번호대로 선택한다..
	this->UpdateWindowPos();
}

void CFormViewProperty::OnTreeChrEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMTVDISPINFOA* pTDI = (TV_DISPINFO*) pNMHDR;

	*pResult            = 0;

	HTREEITEM hI        = pTDI->item.hItem;
	if (nullptr == hI)
		return;

	if (hI == m_hTI_Joint)
	{
		CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
		CN3CEDoc* pDoc   = (CN3CEDoc*) pFrm->GetActiveDocument();
		CN3Chr* pChr     = pDoc->m_Scene.ChrGet(0);
		if (pChr && pTDI->item.pszText)
		{
			int nS0 = -1, nS1 = -1, nS2 = -1, nS3 = -10;
			int nE0 = -1, nE1 = -1, nE2 = -1, nE3 = -1;
			sscanf(pTDI->item.pszText,
				"Joints - 상체(%d ~ %d), 하체(%d ~ %d), 머리카락(%d ~ %d), 망토(%d ~ %d)", &nS0,
				&nE0, &nS1, &nE1, &nS2, &nE2, &nS3, &nE3);
			pChr->JointPartSet(0, nS0, nE0);
			pChr->JointPartSet(1, nS1, nE1);
			pChr->JointPartSet(2, nS2, nE2);
			pChr->JointPartSet(3, nS3, nE3);

			*pResult = 1;
			return;
		}
	}

	CN3CEDoc* pDoc = GetDocument();
	CN3Chr* pChr   = pDoc->m_Scene.ChrGet(0);

	HTREEITEM hPI  = m_TreeChr.GetParentItem(hI);
	if (nullptr == hPI)
		return;

	CN3Base* pBase = (CN3Base*) m_TreeChr.GetItemData(hI);
	if (pBase)
	{
		if (pTDI->item.pszText)
		{
			if (pBase->Type() & OBJ_BASE_FILEACCESS)
				((CN3BaseFileAccess*) pBase)
					->FileNameSet(pTDI->item.pszText); // 파일 이름을 바꾸어 준다..
			else
				pBase->m_szName = pTDI->item.pszText;
			*pResult = 1;
		}
	}
}

void CFormViewProperty::OnTreeChrRclick(NMHDR* pNMHDR, LRESULT* pResult)
{
	CPoint pt;
	::GetCursorPos(&pt);
	CPoint pt2 = pt;
	m_TreeChr.ScreenToClient(&pt2);
	HTREEITEM hI = m_TreeChr.HitTest(pt2); // 현재 위치에서 찍힌
	if (nullptr == hI)
		return;

	m_TreeChr.SelectItem(hI);
	HTREEITEM hPI    = m_TreeChr.GetParentItem(hI);
	//	if(nullptr == hPI) return;

	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	CMenu* pMenu     = pFrm->GetMenu();
	CMenu* pSM       = pMenu->GetSubMenu(1); // 편집 메뉴

	if (m_hTI_Joint == hI || m_hTI_Joint == hPI)
	{
		pSM = pSM->GetSubMenu(0); // Joint 편집 메뉴
		pSM->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
	}

	if (m_hTI_Parts == hI || m_hTI_Parts == hPI)
	{
		pSM = pSM->GetSubMenu(1); // Part 편집 메뉴
		pSM->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
	}

	if (m_hTI_Plugs == hI || m_hTI_Plugs == hPI)
	{
		pSM = pSM->GetSubMenu(2); // Plug 편집 메뉴
		pSM->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
	}

	if (m_hTI_FXPlug == hI || m_hTI_FXPlug == hPI)
	{
		pSM = pSM->GetSubMenu(4); // FXPlug 편집 메뉴
		pSM->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
	}
	*pResult = 0;
}

void CFormViewProperty::OnTreeChrDblclk(NMHDR* pNMHDR, LRESULT* pResult)
{
	CN3Base* pBase = this->GetSelectedObject();
	CN3CEDoc* pDoc = GetDocument();
	CN3Chr* pChr   = pDoc->m_Scene.ChrGet(0);

	if (pBase && (pBase->Type() & OBJ_JOINT))
	{
		this->OnEditChrJointSet();
	}
	else if (pBase && (pBase->Type() & OBJ_CHARACTER_PART)) // Part일 경우.
	{
		this->OnEditChrPartSet();
	}
	else if (pBase && (pBase->Type() & OBJ_CHARACTER_PLUG)) // 플러그일 경우.
	{
		this->OnEditChrPlugSet();
	}

	*pResult = 0;
}

void CFormViewProperty::OnEditChrJointSet()
{
	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	CN3CEDoc* pDoc   = (CN3CEDoc*) pFrm->GetActiveDocument();
	CN3Chr* pChr     = pDoc->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;

	pFrm->GetPaneRender()->m_pJointSelected = nullptr;

	CString FileName;
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	CFileDialog dlg(
		TRUE, "N3Joint", nullptr, dwFlags, "Joint File(*.N3Joint)|*.N3Joint||", nullptr);
	if (dlg.DoModal() == IDCANCEL)
		return;

	FileName = dlg.GetPathName();
	pChr->JointSet(std::string(FileName));

	pDoc->UpdateAllViews(nullptr);
	;
}

void CFormViewProperty::OnEditChrPartAdd()
{
	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	CN3CEDoc* pDoc   = (CN3CEDoc*) pFrm->GetActiveDocument();
	CN3Chr* pChr     = pDoc->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;

	pChr->PartAdd();
	pDoc->UpdateAllViews(nullptr);
	;
}

void CFormViewProperty::OnEditChrPartDelete()
{
	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	CN3CEDoc* pDoc   = (CN3CEDoc*) pFrm->GetActiveDocument();
	CN3Chr* pChr     = pDoc->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;

	HTREEITEM hI = m_TreeChr.GetSelectedItem();
	if (nullptr == hI)
		return;
	CN3Base* pBase = (CN3Base*) m_TreeChr.GetItemData(hI);
	if (nullptr == pBase || !(pBase->Type() & OBJ_CHARACTER_PART))
		return;

	CN3CPart* pPart = (CN3CPart*) pBase;
	int nPC         = pChr->PartCount();
	for (int i = 0; i < nPC; i++)
	{
		if (pChr->Part(i) == pPart)
		{
			pChr->PartDelete(i);
			break;
		}
	}

	pDoc->UpdateAllViews(nullptr);
	;
}

void CFormViewProperty::OnEditChrPartSet()
{
	CN3Base* pBase = this->GetSelectedObject();
	if (nullptr == pBase || !(pBase->Type() & OBJ_CHARACTER_PART))
		return;

	CN3CPart* pPart = (CN3CPart*) pBase;

	CString FileName;
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	CFileDialog dlg(
		TRUE, "N3CPart", nullptr, dwFlags, "Character Part File(*.N3CPart)|*.N3CPart||", nullptr);
	if (dlg.DoModal() == IDCANCEL)
		return;

	FileName = dlg.GetPathName();
	pPart->Release();
	pPart->LoadFromFile(std::string(FileName)); // 파일에서 읽기.

	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	GetDocument()->UpdateAllViews(nullptr);
	;
}

void CFormViewProperty::OnEditChrPlugAdd()
{
	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	CN3CEDoc* pDoc   = (CN3CEDoc*) pFrm->GetActiveDocument();
	CN3Chr* pChr     = pDoc->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;

	CN3CPlugBase* pPlug = pChr->PlugAdd();
	if (pPlug)
	{
		pPlug->m_szName = "Untitled";
		pPlug->FileNameSet("Item\\Untitled.N3CPlug");
	}
	pDoc->UpdateAllViews(nullptr);
}

void CFormViewProperty::OnEditChrPlugAddcloak()
{
	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	CN3CEDoc* pDoc   = (CN3CEDoc*) pFrm->GetActiveDocument();
	CN3Chr* pChr     = pDoc->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;

	CN3CPlugBase* pPlug = pChr->PlugAdd(/*PLUGTYPE_CLOAK*/);
	if (pPlug)
	{
		pPlug->m_szName = "Untitled";
		pPlug->FileNameSet("Item\\Untitled.N3CPlug_Cloak");
	}
	pDoc->UpdateAllViews(nullptr);
}

void CFormViewProperty::OnEditChrPlugDelete()
{
	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	CN3CEDoc* pDoc   = (CN3CEDoc*) pFrm->GetActiveDocument();
	CN3Chr* pChr     = pDoc->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;

	HTREEITEM hI = m_TreeChr.GetSelectedItem();
	if (nullptr == hI)
		return;
	CN3Base* pBase = (CN3Base*) m_TreeChr.GetItemData(hI);
	if (nullptr == pBase || !(pBase->Type() & OBJ_CHARACTER_PLUG))
		return;

	CN3CPlug* pPlug = (CN3CPlug*) pBase;
	int nPC         = pChr->PlugCount();
	for (int i = 0; i < nPC; i++)
	{
		if (pChr->Plug(i) == pPlug)
		{
			pChr->PlugDelete(i);
			break;
		}
	}

	pDoc->UpdateAllViews(nullptr);
	;
}

void CFormViewProperty::OnEditChrPlugSet()
{
	CN3Base* pBase = this->GetSelectedObject();
	if (nullptr == pBase || !(pBase->Type() & OBJ_CHARACTER_PLUG))
		return;

	CN3CPlug* pPlug = (CN3CPlug*) pBase;

	CString FileName;
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	CFileDialog dlg(
		TRUE, "N3CPlug", nullptr, dwFlags, "Character Plug File(*.N3CPlug)|*.N3CPlug||", nullptr);
	if (dlg.DoModal() == IDCANCEL)
		return;

	FileName = dlg.GetPathName();
	pPlug->Release();
	pPlug->LoadFromFile(std::string(FileName));

	CN3Chr* pChr = GetDocument()->m_Scene.ChrGet(0);
	if (pChr)
		pChr->RemakePlugTracePolygons();

	GetDocument()->UpdateAllViews(nullptr);
	;
}

CN3Base* CFormViewProperty::GetSelectedObject()
{
	HTREEITEM hItem = m_TreeChr.GetSelectedItem();
	if (nullptr == hItem)
		return nullptr;

	if (m_TreeChr.GetItemData(hItem) == 0)
	{
		hItem = m_TreeChr.GetParentItem(hItem);
		if (nullptr == hItem)
			return nullptr;
	}

	return (CN3Base*) m_TreeChr.GetItemData(hItem);
}

void CFormViewProperty::OnSelchangedTreeChr(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*) pNMHDR;

	CN3Base* pBase           = this->GetSelectedObject();

	CMainFrame* pFrm         = (CMainFrame*) AfxGetMainWnd();
	if (pBase && (pBase->Type() & OBJ_JOINT))
	{
		pFrm->GetPaneRender()->m_pJointSelected = (CN3Joint*) pBase;
		pFrm->GetPaneRender()->InvalidateRect(nullptr, FALSE);
	}

	pFrm->GetPaneRender()->InvalidateRect(nullptr, FALSE);
	this->UpdateWindowPos();

	*pResult = 0;
}

void CFormViewProperty::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	this->UpdateAllInfo();
}

void CFormViewProperty::SelectNode(int nNodeIndex)
{
	int nNode = 0;
	for (HTREEITEM hNext = m_TreeChr.GetChildItem(TVI_ROOT); nullptr != hNext;
		hNext            = m_TreeChr.GetNextItem(hNext, TVGN_NEXTVISIBLE), nNode++)
	{
		if (nNode == nNodeIndex)
		{
			m_TreeChr.SelectItem(hNext);
			break;
		}
	}
}

int CFormViewProperty::GetSelectedNode()
{
	int nNode       = 0;
	HTREEITEM hItem = m_TreeChr.GetSelectedItem();
	for (HTREEITEM hNext = m_TreeChr.GetChildItem(TVI_ROOT); nullptr != hNext;
		hNext            = m_TreeChr.GetNextItem(hNext, TVGN_NEXTVISIBLE), nNode++)
	{
		if (hItem == hNext)
		{
			break;
		}
	}

	return nNode;
}

void CFormViewProperty::UpdateJointItem(HTREEITEM hItem, CN3Joint* pJoint, int& iNumber)
{
	if (pJoint)
	{
		char szBuff[128];
		sprintf(szBuff, "%d : %s", iNumber, pJoint->m_szName.c_str());
		HTREEITEM hInsert = m_TreeChr.InsertItem(szBuff, 9, 9, hItem);
		m_TreeChr.SetItemData(hInsert, (DWORD_PTR) pJoint);
		iNumber++;

		int nCJC = pJoint->ChildCount();
		for (int i = 0; i < nCJC; i++)
			UpdateJointItem(hInsert, (CN3Joint*) (pJoint->Child(i)), iNumber);
	}
	else
	{
		m_TreeChr.InsertItem("Null Pointer!!!", 9, 9, hItem);
	}
}

void CFormViewProperty::OnFileSaveSelected()
{
	CN3Base* pBase = this->GetSelectedObject();
	if (pBase->Type() & OBJ_BASE_FILEACCESS)
	{
		CN3BaseFileAccess* pBase2 = (CN3BaseFileAccess*) pBase;

		pBase2->SaveToFile();
	}
}

void CFormViewProperty::OnEditChrPlugRotationReset()
{
	CN3Base* pBase = this->GetSelectedObject();
	if (pBase->Type() & OBJ_CHARACTER_PLUG)
	{
		CN3CPlug* pPlug = (CN3CPlug*) pBase;
		__Matrix44 mtxRot;
		mtxRot.Identity();
		pPlug->RotationSet(mtxRot); // 회전값 초기화..

		CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
		pFrm->GetPaneRender()->InvalidateRect(nullptr, FALSE);
	}
}

void CFormViewProperty::SelectJointNode(HTREEITEM hItemParent, CN3Joint* pJoint)
{
	if (nullptr == hItemParent || nullptr == pJoint)
		return;

	for (HTREEITEM hNext = m_TreeChr.GetChildItem(hItemParent); nullptr != hNext;
		hNext            = m_TreeChr.GetNextItem(hNext, TVGN_NEXTVISIBLE))
	{
		void* pPtr = (void*) (m_TreeChr.GetItemData(hNext));
		if (pPtr == pJoint)
		{
			m_TreeChr.SelectItem(hNext);
			return;
		}

		this->SelectJointNode(hNext, pJoint); // 재귀 호출..
	}
}

void CFormViewProperty::OnEditSetVisible()
{
	CN3Base* pBase = this->GetSelectedObject();
	if (nullptr == pBase)
		return;

	if (pBase->Type() & OBJ_CHARACTER_PART)
	{
		CN3CPart* pCPart   = (CN3CPart*) pBase;
		pCPart->m_bVisible = !pCPart->m_bVisible;
	}
	else if (pBase->Type() & OBJ_CHARACTER_PLUG)
	{
		CN3CPlug* pPlug   = (CN3CPlug*) pBase;
		pPlug->m_bVisible = !pPlug->m_bVisible;
	}

	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	pFrm->GetPaneRender()->InvalidateRect(nullptr, FALSE);
}

void CFormViewProperty::OnUpdateEditSetVisible(CCmdUI* pCmdUI)
{
	CN3Base* pBase = this->GetSelectedObject();
	if (nullptr == pBase)
		return;

	bool bVisible = false;
	if (pBase->Type() & OBJ_CHARACTER_PART)
		bVisible = ((CN3CPart*) pBase)->m_bVisible;
	else if (pBase->Type() & OBJ_CHARACTER_PLUG)
		bVisible = ((CN3CPlug*) pBase)->m_bVisible;
	else
		return;

	if (bVisible)
		pCmdUI->SetText("숨기기\t(&V)");
	else
		pCmdUI->SetText("보이기\t(&V)");
}

void CFormViewProperty::OnEditChrPlugImportPMesh()
{
	CN3Base* pBase = this->GetSelectedObject();
	if (nullptr == pBase || !(pBase->Type() & OBJ_CHARACTER_PLUG))
		return;

	CString FileName;
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	CFileDialog dlg(
		TRUE, "N3PMesh", nullptr, dwFlags, "N3 Progressive Mesh(*.N3PMesh)|*.N3PMesh||", nullptr);
	if (dlg.DoModal() == IDCANCEL)
		return;

	FileName        = dlg.GetPathName();

	CN3CPlug* pPlug = (CN3CPlug*) pBase;
	pPlug->ImportPMesh((const char*) FileName);

	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	pFrm->GetPaneRender()->InvalidateRect(nullptr, FALSE);
}

void CFormViewProperty::OnEditFxplugSet()
{
	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	CN3CEDoc* pDoc   = (CN3CEDoc*) pFrm->GetActiveDocument();
	CN3Chr* pChr     = pDoc->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;

	CString FileName;
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	CFileDialog dlg(
		TRUE, "N3FXPlug", nullptr, dwFlags, "FXPlug File(*.N3FXPlug)|*.N3FXPlug||", nullptr);
	if (dlg.DoModal() == IDCANCEL)
		return;

	FileName = dlg.GetPathName();
	pChr->FXPlugSet(std::string(FileName));

	pDoc->UpdateAllViews(nullptr);
}

void CFormViewProperty::OnEditFxplugDelete()
{
	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	CN3CEDoc* pDoc   = (CN3CEDoc*) pFrm->GetActiveDocument();
	CN3Chr* pChr     = pDoc->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;

	pChr->FXPlugDelete();
	pDoc->UpdateAllViews(nullptr);
}

void CFormViewProperty::OnEditFxplugPartAdd()
{
	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	CN3CEDoc* pDoc   = (CN3CEDoc*) pFrm->GetActiveDocument();
	CN3Chr* pChr     = pDoc->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;
	CN3FXPlug* pFXPlug = pChr->FXPlugGet();
	if (nullptr == pFXPlug)
		pFXPlug = pChr->FXPlugCreate();
	pFXPlug->FXPPartAdd();

	pDoc->UpdateAllViews(nullptr);
}

void CFormViewProperty::OnEditFxplugPartDelete()
{
	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	CN3CEDoc* pDoc   = (CN3CEDoc*) pFrm->GetActiveDocument();
	CN3Chr* pChr     = pDoc->m_Scene.ChrGet(0);
	if (pChr == nullptr)
		return;

	CN3FXPlug* pFXPlug = pChr->FXPlugGet();
	if (pFXPlug == nullptr)
		return;

	HTREEITEM hI = m_TreeChr.GetSelectedItem();
	if (hI == nullptr)
		return;

	CN3Base* pBase = (CN3Base*) m_TreeChr.GetItemData(hI);
	if (pBase == nullptr || !(pBase->Type() & OBJ_FX_PLUG_PART))
		return;

	CN3FXPlugPart* pPart = (CN3FXPlugPart*) pBase;
	int nPC              = static_cast<int>(pFXPlug->m_FXPParts.size());
	for (int i = 0; i < nPC; i++)
	{
		if (pFXPlug->m_FXPParts[i] == pPart)
		{
			pFXPlug->FXPPartDelete(i);
			break;
		}
	}

	pDoc->UpdateAllViews(nullptr);
}

void CFormViewProperty::OnEditFxplugStart()
{
	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	CN3CEDoc* pDoc   = (CN3CEDoc*) pFrm->GetActiveDocument();
	CN3Chr* pChr     = pDoc->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;
	CN3FXPlug* pFXPlug = pChr->FXPlugGet();
	if (nullptr == pFXPlug)
		return;

	pFXPlug->TriggerAll();

	pDoc->UpdateAllViews(nullptr);
}

void CFormViewProperty::OnEditFxplugStop()
{
	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	CN3CEDoc* pDoc   = (CN3CEDoc*) pFrm->GetActiveDocument();
	CN3Chr* pChr     = pDoc->m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;
	CN3FXPlug* pFXPlug = pChr->FXPlugGet();
	if (nullptr == pFXPlug)
		return;

	pFXPlug->StopAll();

	pDoc->UpdateAllViews(nullptr);
}

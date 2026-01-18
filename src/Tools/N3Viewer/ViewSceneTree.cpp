// ViewSceneTree.cpp : implementation file
//

#include "stdafx.h"
#include "N3Viewer.h"
#include "ViewSceneTree.h"

#include "MainFrm.h"
#include "N3ViewerDoc.h"

#include <N3Base/N3Scene.h>
#include <N3Base/N3IMesh.h>
#include <N3Base/N3PMesh.h>
#include <N3Base/N3PMeshInstance.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewSceneTree

IMPLEMENT_DYNCREATE(CViewSceneTree, CTreeView)

CViewSceneTree::CViewSceneTree()
{
}

CViewSceneTree::~CViewSceneTree()
{
}

BEGIN_MESSAGE_MAP(CViewSceneTree, CTreeView)
//{{AFX_MSG_MAP(CViewSceneTree)
ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
ON_NOTIFY_REFLECT(NM_RCLICK, OnRclick)
ON_WM_SIZE()
ON_WM_CREATE()
ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginlabeledit)
ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndlabeledit)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewSceneTree drawing

void CViewSceneTree::OnDraw(CDC* pDC)
{
	// CN3ViewerDoc* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CViewSceneTree diagnostics

#ifdef _DEBUG
void CViewSceneTree::AssertValid() const
{
	CTreeView::AssertValid();
}

void CViewSceneTree::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}

CN3ViewerDoc* CViewSceneTree::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CN3ViewerDoc)));
	return (CN3ViewerDoc*) m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CViewSceneTree message handlers

/////////////////////////////////////////////////////////////////////////////
// CViewSceneTree message handlers
void CViewSceneTree::UpdateAllInfo()
{
	GetTreeCtrl().DeleteAllItems();

	this->UpdateTreeItem(TVI_ROOT, &(GetDocument()->m_Scene));
	this->ExpandTree(TVI_ROOT);
	this->SelectObject(TVI_ROOT, GetDocument()->m_pSelectedObj);
}

void CViewSceneTree::ExpandTree(HTREEITEM hItem)
{
	if (hItem == nullptr)
		return;

	GetTreeCtrl().Expand(hItem, TVE_EXPAND);

	HTREEITEM hChild = GetTreeCtrl().GetChildItem(hItem);
	while (hChild != nullptr)
	{
		this->ExpandTree(hChild);
		hChild = GetTreeCtrl().GetNextItem(hChild, TVGN_NEXT);
	}
}

void CViewSceneTree::UpdateTreeItem(HTREEITEM hParent, CN3Base* pBase)
{
	if (pBase == nullptr)
		return;

	DWORD dwType = pBase->Type();
	int nItem    = 0;
	if (dwType & OBJ_SCENE)
		nItem = 0;
	else if (dwType & OBJ_CAMERA)
		nItem = 3;
	else if (dwType & OBJ_LIGHT)
		nItem = 4;
	else if (dwType & OBJ_SHAPE)
	{
		CN3Shape* pShape = (CN3Shape*) pBase;
		if (pShape->IsPMeshProcessed())
			nItem = 6; // PMesh 처리가 되어 있는가...??
		else
			nItem = 7;
	}
	else if (dwType & OBJ_CHARACTER)
		nItem = 8;
	else if (dwType & OBJ_MESH)
		nItem = 15;
	else if (dwType & OBJ_MESH_PROGRESSIVE)
		nItem = 12;
	else if (dwType & OBJ_MESH_INDEXED)
		nItem = 13;
	else if (dwType & OBJ_JOINT)
		nItem = 9;
	else if (dwType & OBJ_SKIN)
		nItem = 14;
	else if (dwType & OBJ_TRANSFORM)
		nItem = 0;

	CN3BaseFileAccess* pBFA = nullptr;
	if (pBase->Type() & OBJ_BASE_FILEACCESS)
		pBFA = (CN3BaseFileAccess*) pBase;

	HTREEITEM hItem = nullptr;
	if (pBFA != nullptr)
		hItem = GetTreeCtrl().InsertItem(pBFA->FileName().c_str(), nItem, nItem, hParent, nullptr);
	else
		hItem = GetTreeCtrl().InsertItem(pBase->m_szName.c_str(), nItem, nItem, hParent, nullptr);

	GetTreeCtrl().SetItemData(hItem, (DWORD_PTR) pBase);

	if (dwType & OBJ_SCENE)
	{
		CN3Scene* pScene = (CN3Scene*) pBase;

		CString str;
		str.Format("Scene : %s", pScene->m_szName.c_str());
		GetTreeCtrl().SetItemText(hItem, str);

		int i            = 0;
		HTREEITEM hItem2 = nullptr;

		hItem2           = GetTreeCtrl().InsertItem("Camera", 3, 3, hItem);
		int nCC          = pScene->CameraCount();
		for (i = 0; i < nCC; i++)
			this->UpdateTreeItem(hItem2, pScene->CameraGet(i));

		hItem2  = GetTreeCtrl().InsertItem("Light", 4, 4, hItem);
		int nLC = pScene->LightCount();
		for (i = 0; i < nLC; i++)
			this->UpdateTreeItem(hItem2, pScene->LightGet(i));

		hItem2  = GetTreeCtrl().InsertItem("Shape", 6, 6, hItem);
		int nSC = pScene->ShapeCount();
		for (i = 0; i < nSC; i++)
			this->UpdateTreeItem(hItem2, pScene->ShapeGet(i));
		GetTreeCtrl().SortChildren(hItem2);

		hItem2   = GetTreeCtrl().InsertItem("Character", 8, 8, hItem);
		int nCC2 = pScene->ChrCount();
		for (i = 0; i < nCC2; i++)
			this->UpdateTreeItem(hItem2, pScene->ChrGet(i));
		GetTreeCtrl().SortChildren(hItem2);

		/*		hItem2 = GetTreeCtrl().InsertItem("Mesh Resource", hItem);
		int nMC = pScene->m_MngMesh.Count();
		for(i = 0; i < nMC; i++) this->UpdateTreeItem(hItem2, pScene->m_MngMesh.Get(i));

		hItem2 = GetTreeCtrl().InsertItem("Progressive Mesh Resource", hItem);
		int nPMC = pScene->m_MngPMesh.Count();
		for(i = 0; i < nPMC; i++) this->UpdateTreeItem(hItem2, pScene->m_MngPMesh.Get(i));

		hItem2 = GetTreeCtrl().InsertItem("Indexed Mesh Resource", hItem);
		int nIMC = pScene->m_MngIMesh.Count();
		for(i = 0; i < nIMC; i++) this->UpdateTreeItem(hItem2, pScene->m_MngIMesh.Get(i));

		hItem2 = GetTreeCtrl().InsertItem("Joint Resource", hItem);
		int nJC = pScene->m_MngJoint.Count();
		for(i = 0; i < nJC; i++) this->UpdateTreeItem(hItem2, pScene->m_MngJoint.Get(i));

		hItem2 = GetTreeCtrl().InsertItem("Skin Resource", hItem);
		int nSC2 = pScene->m_MngSkin.Count();
		for(i = 0; i < nSC2; i++) this->UpdateTreeItem(hItem2, pScene->m_MngSkin.Get(i));
*/
	}
	/*	else if(pBase->Type() & OBJ_CHARACTER)
	{
		CN3Chr* pChr = (CN3Chr*)pBase;

		HTREEITEM hItem2 = nullptr, hItem3;
		CString str, szTmp;

		for(int i = 0; i < MAX_CHR_LOD; i++)
		{
			str.Format("Level Of Detail %d", i); hItem2 = GetTreeCtrl().InsertItem(str, hItem);

			if(pChr->JointGet(i) != nullptr) szTmp = pChr->JointGet(i)->Name(); else szTmp = "";
			str.Format("Joint : %s", szTmp);
			hItem3 = GetTreeCtrl().InsertItem(str, hItem2); GetTreeCtrl().SetItemData(hItem3, pChr->JointGet(i));
			
			for(int j = 0; j < MAX_CHR_PART; j++)
			{
				if(pChr->MeshGet(i, j) != nullptr) szTmp = pChr->MeshGet(i, j)->Name(); else szTmp = "";
				str .Format("Mesh : %s", szTmp);
				hItem3 = GetTreeCtrl().InsertItem(str, hItem2); GetTreeCtrl().SetItemData(hItem3, pChr->MeshGet(i, j));
				
				if(pChr->SkinGet(i, j) != nullptr) szTmp = pChr->SkinGet(i, j)->Name(); else szTmp = "";
				str .Format("Skin : %s", szTmp);
				hItem3 = GetTreeCtrl().InsertItem(str, hItem2); GetTreeCtrl().SetItemData(hItem3, pChr->SkinGet(i, j));
			}
		}
	}
	else if(pBase->Type() & OBJ_SHAPE)
	{
		CN3Shape* pShape = (CN3Shape*)pBase;

		CString str;
		HTREEITEM hItem2 = nullptr;
		for(int i = 0; i < MAX_SHAPE_OBJECT; i++)
		{
			CN3PMeshInstance* pPMeshInstance = pShape->MeshGet(i);
			if(pPMeshInstance == nullptr) continue;
			
			str.Format("Mesh : %s", pPMeshInstance->Name());
			hItem2 = GetTreeCtrl().InsertItem(str, hItem); GetTreeCtrl().SetItemData(hItem2, pPMeshInstance);
		}
	}
*/
	GetTreeCtrl().Expand(hItem, TVE_EXPAND);
}

/*
void CViewSceneTree::SelectTree(HTREEITEM hParent, int nDepth, int* nSeq)
{
	static int nTmp = 0;
	if(hParent == TVI_ROOT) nTmp = 0;
	if(nTmp == nDepth) return;

	HTREEITEM hChild = nullptr;
	for(	int i = 0, HTREEITEM hChild = GetTreeCtrl().GetChildItem(hParent);
			hChild != nullptr && nSeq[nDepth] != i;
			i++, hChild = GetTreeCtrl().GetNextItem(hChild, TVGN_NEXT))
	{
		if(
	}


	nTmp++;
}

void CViewSceneTree::GetSeq(HTREEITEM hParent, HTREEITEM hItem, int& nDepth, int* nSeq)
{
}
*/

void CViewSceneTree::SelectObject(HTREEITEM hItem, void* pItemData)
{
	if (pItemData == nullptr || hItem == nullptr)
		return;

	if (hItem != TVI_ROOT)
	{
		if (GetTreeCtrl().GetItemData(hItem) == (DWORD_PTR) pItemData)
		{
			GetTreeCtrl().SelectItem(hItem);
			return;
		}
	}

	HTREEITEM hChild = GetTreeCtrl().GetChildItem(hItem);
	while (hChild != nullptr)
	{
		SelectObject(hChild, pItemData);
		hChild = GetTreeCtrl().GetNextItem(hChild, TVGN_NEXT);
	}
}

void CViewSceneTree::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*) pNMHDR;

	// TODO: Add your control notification handler code here
	HTREEITEM hItem          = pNMTreeView->itemNew.hItem;
	if (hItem == nullptr)
		return;

	CN3Base* pBaseOld             = GetDocument()->m_pSelectedObj;
	CN3Base* pBase                = (CN3Base*) GetTreeCtrl().GetItemData(hItem);
	CMainFrame* pFrm              = (CMainFrame*) AfxGetMainWnd();
	GetDocument()->m_pSelectedObj = pBase;

	if (pBaseOld != pBase)
	{
		pFrm->GetViewProperty()->UpdateInfo();
		pFrm->GetView()->InvalidateRect(nullptr, FALSE);

		if (pFrm->m_DlgPMeshEdit.GetSafeHwnd() && (pBase->Type() & OBJ_SHAPE))
		{
			pFrm->m_DlgPMeshEdit.m_pShapeRef = (CN3Shape*) pBase;
			pFrm->m_DlgPMeshEdit.UpdateAllInfo();
			pFrm->GetView()->SetFocusToSelectedObject();
		}
	}

	*pResult = 0;
}

void CViewSceneTree::OnRclick(NMHDR* pNMHDR, LRESULT* pResult)
{
	CPoint point;
	GetCursorPos(&point);
	CPoint pt = point;
	GetTreeCtrl().ScreenToClient(&pt);
	HTREEITEM hItem = GetTreeCtrl().HitTest(pt);
	if (hItem == nullptr)
		return;

	GetTreeCtrl().SelectItem(hItem);
	CN3Base* pBase = (CN3Base*) (GetTreeCtrl().GetItemData(hItem));
	if (pBase != nullptr)
	{
		CMenu popup;
		popup.LoadMenu(IDR_POPUP_SCENE);
		CMenu* pSubMenu = popup.GetSubMenu(0);
		pSubMenu->TrackPopupMenu(
			TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this); // 편집 메뉴를 펼친다..
	}

	*pResult = 0;
}

void CViewSceneTree::OnInitialUpdate()
{
	CTreeView::OnInitialUpdate();

	static bool bInitCtrl = false;
	if (true == bInitCtrl)
		this->UpdateAllInfo();
	if (GetTreeCtrl().GetSafeHwnd() && false == bInitCtrl)
	{
		m_ImgList.Create(IDB_N3BASE_NODES, 20, 16, RGB(255, 0, 255));
		GetTreeCtrl().SetImageList(&m_ImgList, TVSIL_NORMAL);

		bInitCtrl = true;
	}
}

BOOL CViewSceneTree::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= TVS_HASBUTTONS | TVS_HASLINES | TVS_SHOWSELALWAYS | TVS_LINESATROOT
				| TVS_EDITLABELS;

	return CTreeView::PreCreateWindow(cs);
}

void CViewSceneTree::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	this->UpdateAllInfo();
}

void CViewSceneTree::OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMTVDISPINFOA* pTDI = (TV_DISPINFO*) pNMHDR;
	*pResult            = 1;

	HTREEITEM hI        = pTDI->item.hItem;
	if (nullptr == hI)
		return;

	CN3Base* pBase = (CN3Base*) GetTreeCtrl().GetItemData(hI);
	if (nullptr == pBase)
		return;

	*pResult = 0;
}

void CViewSceneTree::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMTVDISPINFOA* pTDI = (TV_DISPINFO*) pNMHDR;
	*pResult            = 0;

	HTREEITEM hI        = pTDI->item.hItem;
	if (nullptr == hI)
		return;

	CN3Base* pBase = (CN3Base*) GetTreeCtrl().GetItemData(hI);
	if (nullptr == pBase || !(pBase->Type() & OBJ_BASE_FILEACCESS))
		return;

	CN3BaseFileAccess* pBFA = (CN3BaseFileAccess*) pBase;
	if (pTDI->item.pszText)
		pBFA->FileNameSet(pTDI->item.pszText); // 파일 이름 정하기..

	*pResult = 1;
}

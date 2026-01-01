// DlgSceneGraph.cpp : implementation file
//

#include "stdafx.h"
#include "DlgSceneGraph.h"
#include "MainFrm.h"
#include "MapMng.h"

#include <N3Base/N3Scene.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgSceneGraph dialog
CDlgSceneGraph::CDlgSceneGraph(CWnd* pParent /*=nullptr*/, DWORD dwFlag)
	: CDialog(CDlgSceneGraph::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgSceneGraph)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_dwFlag = dwFlag;
	m_IsSourceObj = TRUE;
}

void CDlgSceneGraph::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSceneGraph)
	DDX_Control(pDX, IDC_TREE_OBJ, m_Tree);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgSceneGraph, CDialog)
	//{{AFX_MSG_MAP(CDlgSceneGraph)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_OBJ, OnSelchangedTreeObj)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_OBJ, OnDblclkTreeObj)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_TREE_OBJ, OnRclickTreeObj)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgSceneGraph message handlers

void CDlgSceneGraph::UpdateTree(CN3Scene* pScene)
{
	m_Tree.DeleteAllItems();

	m_pSceneRef = pScene;

	this->UpdateTreeItem(TVI_ROOT, m_pSceneRef);
	this->ExpandTree(TVI_ROOT);
}

void CDlgSceneGraph::ExpandTree(HTREEITEM hItem)
{
	if(hItem == nullptr) return;

	m_Tree.Expand(hItem, TVE_EXPAND);

	HTREEITEM hChild = m_Tree.GetChildItem(hItem);
	while(hChild != nullptr)
	{
		this->ExpandTree(hChild);
		hChild = m_Tree.GetNextItem(hChild, TVGN_NEXT);
	}
}

void CDlgSceneGraph::UpdateTreeItem(HTREEITEM hParent, CN3Base *pBase)
{
	if(pBase == nullptr) return;
	
	DWORD dwType = pBase->Type();
	int nItem = 0;
	if(dwType & OBJ_SCENE) nItem = 0;
	else if(dwType & OBJ_CAMERA) nItem = 1;
	else if(dwType & OBJ_LIGHT) nItem = 2;
	else if(dwType & OBJ_SHAPE) nItem = 3;
	else if(dwType & OBJ_CHARACTER) nItem = 4;
	else if(dwType & OBJ_MESH) nItem = 5;
	else if(dwType & OBJ_MESH_PROGRESSIVE) nItem = 6;
	else if(dwType & OBJ_MESH_INDEXED) nItem = 7;
	else if(dwType & OBJ_JOINT) nItem = 8;
	else if(dwType & OBJ_SKIN) nItem = 9;
	else if(dwType & OBJ_TRANSFORM) nItem = 10;

	HTREEITEM hItem = m_Tree.InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT, pBase->m_szName.c_str(), nItem, nItem, 0, 0, 0, hParent, nullptr);
	m_Tree.SetItemData(hItem, (DWORD_PTR) pBase);

	if(dwType & OBJ_SCENE)
	{
		CString str;
		str.Format("Scene : %s", m_pSceneRef->m_szName.c_str());
		m_Tree.SetItemText(hItem, str);

		int i = 0;
		HTREEITEM hItem2 = nullptr;

		if (m_dwFlag & OBJ_CAMERA)
		{
			hItem2 = m_Tree.InsertItem("Camera", hItem);
			int nCC = m_pSceneRef->CameraCount();
			for(i = 0; i < nCC; i++) this->UpdateTreeItem(hItem2, m_pSceneRef->CameraGet(i));
		}

		if (m_dwFlag & OBJ_LIGHT)
		{
			hItem2 = m_Tree.InsertItem("Light", hItem);
			int nLC = m_pSceneRef->LightCount();
			for(i = 0; i < nLC; i++) this->UpdateTreeItem(hItem2, m_pSceneRef->LightGet(i));
		}
		
		if (m_dwFlag & OBJ_SHAPE)
		{
			hItem2 = m_Tree.InsertItem("Shape", hItem);
			int nSC = m_pSceneRef->ShapeCount();
			for(i = 0; i < nSC; i++) this->UpdateTreeItem(hItem2, m_pSceneRef->ShapeGet(i));
			m_Tree.SortChildren(hItem2);
		}

		if (m_dwFlag & OBJ_CHARACTER)
		{
			hItem2 = m_Tree.InsertItem("Character", hItem);
			int nCC2 = m_pSceneRef->ChrCount();
			for(i = 0; i < nCC2; i++) this->UpdateTreeItem(hItem2, m_pSceneRef->ChrGet(i));
		}

		if (m_dwFlag & OBJ_MESH)
		{
			hItem2 = m_Tree.InsertItem("Mesh Resource", hItem);
			int nMC = m_pSceneRef->s_MngMesh.Count();
			for(i = 0; i < nMC; i++) this->UpdateTreeItem(hItem2, m_pSceneRef->s_MngMesh.Get(i));
		}

		if (m_dwFlag & OBJ_MESH_PROGRESSIVE)
		{
			hItem2 = m_Tree.InsertItem("Progressive Mesh Resource", hItem);
			int nPMC = m_pSceneRef->s_MngPMesh.Count();
			for(i = 0; i < nPMC; i++) this->UpdateTreeItem(hItem2, m_pSceneRef->s_MngPMesh.Get(i));
		}

		if (m_dwFlag & OBJ_JOINT)
		{
			hItem2 = m_Tree.InsertItem("Joint Resource", hItem);
			int nJC = m_pSceneRef->s_MngJoint.Count();
			for(i = 0; i < nJC; i++) this->UpdateTreeItem(hItem2, m_pSceneRef->s_MngJoint.Get(i));
		}

	}
	m_Tree.Expand(hItem, TVE_EXPAND);
}

void CDlgSceneGraph::OnSelchangedTreeObj(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	
	// TODO: Add your control notification handler code here
	//pNMTreeView->itemNew->lParam	//이것이 선택된 아이템의 (CN3Base*)이다.
	m_pMapMng->SelectObject((CN3Base*)pNMTreeView->itemNew.lParam, m_IsSourceObj);
	Invalidate(FALSE);
	
	*pResult = 0;
}

void CDlgSceneGraph::OnDblclkTreeObj(NMHDR* pNMHDR, LRESULT* pResult) 
{

	*pResult = 0;
}

void CDlgSceneGraph::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	const int iPreviewCX = 200;	const int iPreviewCY = 150;
	if(m_Tree.GetSafeHwnd() != nullptr)
	{
		int iPreviewTop = cy-iPreviewCY;
		int iPreviewLeft = (cx-iPreviewCX)/2; if (iPreviewLeft<0) iPreviewLeft = 0;
		m_Tree.SetWindowPos(nullptr, 0, 0, cx, iPreviewTop, SWP_NOZORDER);
		GetDlgItem(IDC_PREVIEW)->SetWindowPos(nullptr, iPreviewLeft, iPreviewTop, iPreviewCX, iPreviewCY, SWP_NOZORDER);
	}
}

void CDlgSceneGraph::OnRclickTreeObj(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
}

void CDlgSceneGraph::SelectObject(HTREEITEM hItem, void* pItemData)
{
	if (hItem == nullptr)
		return;

	if (hItem != TVI_ROOT)
	{
		if (m_Tree.GetItemData(hItem) == (DWORD_PTR) pItemData)
		{
			m_Tree.SelectItem(hItem);
			return;
		}
	}

	HTREEITEM hChild = m_Tree.GetChildItem(hItem);
	while (hChild != nullptr)
	{
		SelectObject(hChild, pItemData);
		hChild = m_Tree.GetNextItem(hChild, TVGN_NEXT);
	}
}

void CDlgSceneGraph::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CWnd* pWnd = GetDlgItem(IDC_PREVIEW);
	if (pWnd && m_Tree.GetSafeHwnd())
	{
		HTREEITEM hItem = m_Tree.GetSelectedItem();

		if (hItem)
		{
			LPARAM lParam = m_Tree.GetItemData(hItem);
			CN3Base* pObj = (CN3Base*)lParam;
			if (pObj)
			{
				DWORD dwType = pObj->Type();
				if (dwType & OBJ_CHARACTER || dwType & OBJ_SHAPE)
					m_pMapMng->RenderObjectToWindow((CN3TransformCollision*)pObj, pWnd->m_hWnd);
			}
			else
			{
				CRect rc;
				pWnd->GetClientRect(&rc);
				pWnd->ClientToScreen(&rc);
				ScreenToClient(&rc);
				dc.FillSolidRect(&rc, RGB(192, 192,192));
			}
		}
	}
	
	// Do not call CDialog::OnPaint() for painting messages
}

BOOL CDlgSceneGraph::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_ImgList.Create(IDB_SCENE_NODES, 20, 11, RGB(255,0,255));
	m_Tree.SetImageList(&m_ImgList, TVSIL_NORMAL);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CDlgSceneGraph::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	HACCEL hAccel = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
	if (hAccel) TranslateAccelerator(AfxGetMainWnd()->m_hWnd, hAccel, pMsg);
	return CDialog::PreTranslateMessage(pMsg);
}

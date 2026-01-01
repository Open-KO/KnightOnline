// DlgDTexGroupView.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "LyTerrainDef.h"
#include "DTexGroupMng.h"
#include "DTexGroup.h"

#include "DlgDTexGroupView.h"
#include "MainFrm.h"
#include <N3Base/N3EngTool.h>
#include "DTexMng.h"
#include "DTex.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgDTexGroupView dialog


CDlgDTexGroupView::CDlgDTexGroupView(CWnd* pParent /*=nullptr*/)
	: CDialog(CDlgDTexGroupView::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgDTexGroupView)
	m_fTexSurfaceSize = 128.0f;
	m_pPrevTex = nullptr;
	//}}AFX_DATA_INIT

	m_iRadioState = 0;
}


void CDlgDTexGroupView::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgDTexGroupView)
	DDX_Control(pDX, IDC_SLIDER_DTEX, m_SliderBrushSize);
	DDX_Control(pDX, IDC_TREE_DTEX_GROUP, m_Tree);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgDTexGroupView, CDialog)
	//{{AFX_MSG_MAP(CDlgDTexGroupView)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_DTEX_GROUP, OnSelchangedTreeDtexGroup)
	ON_BN_CLICKED(IDC_TILEAUTO, OnTileauto)
	ON_BN_CLICKED(IDC_TILEMANUEL, OnTilemanuel)
	ON_BN_CLICKED(IDC_TILEMANUELNOMIX, OnTilemanuelNomix)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgDTexGroupView message handlers

void CDlgDTexGroupView::AddGroup(CDTexGroup *pGroup)
{
	HTREEITEM hGroup = m_Tree.InsertItem(pGroup->m_Name, nullptr);
	m_Tree.SetItemData(hGroup, (DWORD_PTR) pGroup);

	int i;
	HTREEITEM hAttr;
	CString Attr;
	for(i=DTEX_FULL; i<DTEX_MAX; i++)
	{
		//DTEX_FULL=0, DTEX_1PER2, DTEX_1PER4, DTEX_3PER4, DTEX_1PER8, DTEX_7PER8, DTEX_5PER8, DTEX_3PER8, DTEX_MAX=8
		switch(i)
		{
		case DTEX_FULL:
			Attr.Format("FULL");
			break;
		case DTEX_1PER2:
			Attr.Format("1/2");
			break;
		case DTEX_1PER4:
			Attr.Format("1/4");
			break;
		case DTEX_3PER4:
			Attr.Format("3/4");
			break;
		case DTEX_1PER8:
			Attr.Format("1/8");
			break;
		case DTEX_7PER8:
			Attr.Format("7/8");
			break;
		case DTEX_5PER8:
			Attr.Format("5/8");
			break;
		case DTEX_3PER8:
			Attr.Format("3/8");
			break;
		}
		hAttr = m_Tree.InsertItem((LPCTSTR) Attr, hGroup);
		m_Tree.SetItemData(hAttr, (DWORD_PTR) i);
		//m_Tree.SetItemData(hAttr, 0);

		auto it = pGroup->m_Attributes[i].begin();
		int iSize = static_cast<int>(pGroup->m_Attributes[i].size());
		for (int j = 0; j < iSize; j++, it++)
		{
			LPDTEXTILEATTR pAttr = (*it);
			CString name;
			name.Format("%s%2d", (LPCTSTR) Attr, j);
			HTREEITEM hElem = m_Tree.InsertItem(name, hAttr);
			m_Tree.SetItemData(hElem, (DWORD_PTR) pAttr);
		}
	}
}

void CDlgDTexGroupView::ResetAll()
{
	m_Tree.DeleteAllItems();

	for (CDTexGroup* pDTG : m_pDTexGroupMng->m_Groups)
		AddGroup(pDTG);
}

void CDlgDTexGroupView::OnSelchangedTreeDtexGroup(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here

	HTREEITEM hTree = m_Tree.GetSelectedItem();
	HTREEITEM hChild = m_Tree.GetChildItem(hTree);
	HTREEITEM hParent = m_Tree.GetParentItem(hTree);

	if(hParent==nullptr)// 그 그룹의 대표적인 썸네일만 보여주자..
	{
		hTree = m_Tree.GetChildItem(hTree);
		hTree = m_Tree.GetChildItem(hTree);
		hChild = m_Tree.GetChildItem(hTree);
	}
	

	if(nullptr == hChild)
	{
		LPDTEXTILEATTR pDTTAttr = (LPDTEXTILEATTR)m_Tree.GetItemData(hTree);
		if ((DWORD_PTR) pDTTAttr > 10)
		{
			m_pDTexGroupMng->m_SelectedDTexTile = *pDTTAttr;
		
			HTREEITEM hAttr = m_Tree.GetParentItem(hTree);
			if(hAttr) 
			{
				m_pDTexGroupMng->m_SelectedDTex.Attr = (int)m_Tree.GetItemData(hAttr);

				HTREEITEM hGroup = m_Tree.GetParentItem(hAttr);
				if(hGroup)
				{
					CDTexGroup* pGroup = (CDTexGroup*)m_Tree.GetItemData(hGroup);
					m_pDTexGroupMng->m_SelectedDTex.Group = pGroup->m_ID;

					//set prev tex..
					CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
					CDTexMng* pDTexMng = pFrm->GetDTexMng();

					CDTex* pDTex = pDTexMng->GetDTexByID(pDTTAttr->TexID);
					m_pPrevTex = nullptr;
					if(pDTex)
					{
						m_pPrevTex = pDTex->m_pTex;
					}

					if(m_pPrevTex)
					{
						m_PrevTexRect.left = (long)(m_pDTexGroupMng->m_SelectedDTexTile.TileX * m_fTexSurfaceSize);
						m_PrevTexRect.top = (long)(m_pDTexGroupMng->m_SelectedDTexTile.TileY * m_fTexSurfaceSize);
						m_PrevTexRect.right = (long)(m_PrevTexRect.left + m_fTexSurfaceSize);
						m_PrevTexRect.bottom = (long)(m_PrevTexRect.top + m_fTexSurfaceSize);
					}

				}
			}
		}
	}
	else
	{
		m_pPrevTex = nullptr;
		SetRect(&m_PrevTexRect, 0,0,0,0);
	}
	Invalidate(FALSE);

	*pResult = 0;
}

void CDlgDTexGroupView::OnTileauto() 
{
	UpdateData(TRUE);	

	m_iRadioState = 0;
}

void CDlgDTexGroupView::OnTilemanuel() 
{
	UpdateData(TRUE);	

	m_iRadioState = 2;
}

void CDlgDTexGroupView::OnTilemanuelNomix()
{
	UpdateData(TRUE);	

	m_iRadioState = 3;
}

void CDlgDTexGroupView::OnDestroy() 
{
	CDialog::OnDestroy();
}

void CDlgDTexGroupView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	BOOL b = m_Tree.SortChildren(nullptr);

	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	if(!pFrm) return;
	
	CWnd* pView = GetDlgItem(IDC_PREVIEW_DTEX_GROUP);
	if(!pView) return;

	pFrm->m_pEng->RenderTexturePreview(m_pPrevTex, pView->m_hWnd, &m_PrevTexRect);
}

BOOL CDlgDTexGroupView::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
//	CWnd* pView = GetDlgItem(IDC_PREVIEW_DTEX_GROUP);
//	pView->SetWindowPos(nullptr, 0, 0, (int)m_fTexSurfaceSize, (int)m_fTexSurfaceSize, SWP_DRAWFRAME|SWP_NOMOVE);

	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();

	m_SliderBrushSize.SetRange(1,20);
	m_SliderBrushSize.SetPos(1);
	m_SliderBrushSize.SetTicFreq(1);

	m_fTexSurfaceSize = 128.0f;
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgDTexGroupView::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	ResetAll();
	
}

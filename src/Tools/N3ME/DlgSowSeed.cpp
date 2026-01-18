// DlgSowSeed.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "DlgSowSeed.h"
#include "DlgShapeList.h"
#include "MainFrm.h"
#include "DTexGroupMng.h"
#include "DTexGroup.h"
#include "MapMng.h"
#include <N3Base/N3Texture.h>
#include <N3Base/N3EngTool.h>
#include <N3Base/N3Shape.h>
#include "DTex.h"
#include "DTexMng.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgSowSeed dialog

CDlgSowSeed::CDlgSowSeed(CWnd* pParent /*=nullptr*/) : CDialog(CDlgSowSeed::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgSowSeed)
	//}}AFX_DATA_INIT
	m_pRefFrm = nullptr;
	//ZeroMemory(m_FileName, MAX_PATH);
	//m_SeedGroupList.clear();
}

void CDlgSowSeed::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSowSeed)
	DDX_Control(pDX, IDC_S_TILE_DETAIL, m_SowDetail);
	DDX_Control(pDX, IDC_CB_TILEGROUP, m_CB_TileGroup);
	DDX_Control(pDX, IDC_SOW_BRUSH_SIZE, m_Brush_Size);
	DDX_Control(pDX, IDC_CB_SEED, m_CB_Seed);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgSowSeed, CDialog)
//{{AFX_MSG_MAP(CDlgSowSeed)
ON_BN_CLICKED(IDC_BTN_DEL_SEED, OnBtnDelSeed)
ON_WM_SHOWWINDOW()
ON_BN_CLICKED(IDC_BTN_GRASSINFOLOAD, OnBtnGrassinfoload)
ON_BN_CLICKED(IDC_BTN_SAVEGAME, OnBtnSavegame)
ON_BN_CLICKED(IDC_BTN_GRASSINFOSAVE, OnBtnGrassinfosave)
ON_BN_CLICKED(IDC_BTN_GRASS_READ, OnBtnGrassRead)
ON_BN_CLICKED(IDC_BTN_GRASS_DELETE, OnBtnGrassDelete)
ON_WM_DESTROY()
ON_WM_PAINT()
ON_CBN_SELCHANGE(IDC_CB_SEED, OnSelchangeCbSeed)
ON_CBN_SELCHANGE(IDC_CB_TILEGROUP, OnSelchangeCbTilegroup)
ON_BN_CLICKED(IDC_SEED_EXIT, OnSeedExit)
ON_NOTIFY(NM_CUSTOMDRAW, IDC_SOW_BRUSH_SIZE, OnCustomdrawSowBrushSize)
ON_BN_CLICKED(IDC_BUTTON_RANDOM, OnButtonRandom)
ON_NOTIFY(NM_CUSTOMDRAW, IDC_S_TILE_DETAIL, OnCustomdrawSTileDetail)
ON_BN_CLICKED(IDC_RADIO_SOW, OnRadioSow)
ON_BN_CLICKED(IDC_RADIO_SELECT, OnRadioSelect)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgSowSeed message handlers

BOOL CDlgSowSeed::OnInitDialog()
{
	CDialog::OnInitDialog();

	//if(m_SeedGroupList.size()) return TRUE;
	m_CB_Seed.ResetContent();
	m_CB_TileGroup.ResetContent();
	//	m_LB_Sowed.ResetContent();
	//m_SeedGroupList.clear();

	m_Brush_Size.SetRange(1, 15);
	m_SowDetail.SetRange(1, 10);
	m_SowDetail.SetPos(5);
	Brush_Size       = 1;
	SowDetail        = 1;
	Sow_Select_Flage = CS_NONE;
	for (int i = 0; i < MAX_BRUSH_SIZE; i++)
	{
		for (int j = 0; j < MAX_BRUSH_SIZE; j++)
		{
			Sow_Array[i][j] = rand() % 3;
		}
	}
	// Seed List 읽어 오기..
	//SetDlgItemText(IDC_STATIC_GRASSINFO, m_FileName);

	return TRUE; // return TRUE unless you set the focus to a control
				 // EXCEPTION: OCX Property Pages should return FALSE
}

//void CDlgSowSeed::OnBtnSow()
//{
/*
	int idx = m_CB_Seed.GetCurSel();
	if(idx<0) return;

	LPSEEDTEX pSeed = (LPSEEDTEX)m_CB_Seed.GetItemDataPtr(idx);

	if(!pSeed) return;

	int SeedID = pSeed->ID;
	char SeedName[80];
	m_CB_Seed.GetLBText(idx, SeedName);

	idx = m_CB_TileGroup.GetCurSel();
	if(idx<0) return;

	int GroupID = m_CB_TileGroup.GetItemData(idx);

	LPSEEDGROUP pSeedGroup = new SEEDGROUP;
	pSeedGroup->iDTexGroupID = GroupID;
	pSeedGroup->iSeedID = SeedID;

	m_pRefFrm->m_SeedGroupList.push_back(pSeedGroup);

	CDTexGroupMng* pDTexGroup = m_pRefFrm->GetDTexGroupMng();
	char* pGroupName = pDTexGroup->GetGroupName(GroupID);

	if(pGroupName)
	{
		CString Str;
		Str.Format("%s - %s", pGroupName, SeedName);
		idx = m_LB_Sowed.AddString((LPCTSTR)Str);
		m_LB_Sowed.SetItemDataPtr(idx, pSeedGroup);
	}
	else
	{
		CString Str;
		Str.Format("Unknown - %s", SeedName);
		idx = m_LB_Sowed.AddString((LPCTSTR)Str);
		m_LB_Sowed.SetItemDataPtr(idx, pSeedGroup);
	}
*/
//}

void CDlgSowSeed::OnBtnDelSeed()
{
	CMainFrame* pFrame                             = (CMainFrame*) AfxGetMainWnd();

	pFrame->GetMapMng()->m_SowSeedMng.Render_Grass = FALSE;

	auto it   = pFrame->GetMapMng()->m_SowSeedMng.Grass_Group.begin();
	int size  = static_cast<int>(pFrame->GetMapMng()->m_SowSeedMng.Grass_Group.size());
	int index = m_CB_TileGroup.GetCurSel();
	for (int ii = 0, listCount = 0; ii < size; ii++, it++)
	{
		if (index == ii)
		{
			LPGRASS_GROUP group_list = *it;
			it_Grass it_grass        = group_list->grass.begin();
			for (size_t jj = 0; jj < group_list->grass.size(); jj++, it_grass++)
			{
				LPGRASS grass_list = *it_grass;
				//				group_list->grass.remove(grass_list);
				delete grass_list;
			}

			group_list->grass.clear();
			pFrame->GetMapMng()->m_SowSeedMng.Grass_Group.remove(group_list);
			delete group_list;
			pFrame->GetMapMng()->m_SowSeedMng.Render_Grass = TRUE;
			RePaint();
			return;
		}
	}
}

void CDlgSowSeed::RePaint()
{
	CMainFrame* pFrame = (CMainFrame*) AfxGetMainWnd();

	m_CB_Seed.ResetContent();
	m_CB_TileGroup.ResetContent();

	char text[256];

	auto it  = pFrame->GetMapMng()->m_SowSeedMng.Grass_Group.begin();
	int size = static_cast<int>(pFrame->GetMapMng()->m_SowSeedMng.Grass_Group.size());
	for (int ii = 0, listCount = 0, groupCount = 0; ii < size; ii++, it++, groupCount++)
	{
		LPGRASS_GROUP group_list = *it;
		auto it_grass            = group_list->grass.begin();
		for (size_t jj = 0; jj < group_list->grass.size(); jj++, it_grass++)
		{
			LPGRASS grass_list = *it_grass;

			sprintf(text, "SubID:%d, x :%f, y :%f, z :%f", listCount, grass_list->Pos.x,
				grass_list->Pos.y, grass_list->Pos.z);
			m_CB_Seed.AddString(text);
			listCount++;
		}

		char Name[60];
		pFrame->GetMapMng()->m_pDlgSourceList->m_ListShape.GetText(group_list->Obj_ID, Name);
		group_list->Group_id = groupCount;
		sprintf(text, "풀 ID: %d , 파일명:%s", group_list->Group_id, Name);
		int CurPos = m_CB_TileGroup.GetCount();
		m_CB_TileGroup.AddString(text);
		m_CB_TileGroup.SetCurSel(CurPos);
	}
}

void CDlgSowSeed::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	if (bShow)
		RePaint();
}

void CDlgSowSeed::Release()
{
	int cnt = m_CB_Seed.GetCount();
	for (int i = 0; i < cnt; i++)
	{
		LPSEEDTEX pSeed = (LPSEEDTEX) m_CB_Seed.GetItemDataPtr(i);
		if (pSeed)
		{
			pSeed->pTex->Release();
			delete pSeed->pTex;
			pSeed->pTex = nullptr;
		}
		delete pSeed;
		pSeed = nullptr;
	}
}

void CDlgSowSeed::OnBtnGrassinfoload()
{
	m_pRefFrm->GetMapMng()->m_SowSeedMng.LoadData();
	this->RePaint();
}

void CDlgSowSeed::OnBtnSavegame()
{
}

void CDlgSowSeed::OnBtnGrassinfosave()
{
	m_pRefFrm->GetMapMng()->m_SowSeedMng.SaveData();
	this->RePaint();
}

void CDlgSowSeed::OnBtnGrassRead()
{
	int count = m_CB_Seed.GetCount();
	if (count >= 8)
	{
		MessageBox("더이상 텍스쳐를 넣을 수가 없어요.", "빠떼루");
		return;
	}

	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_HIDEREADONLY;
	CFileDialog dlg(TRUE, "dxt", nullptr, dwFlags, "풀 텍스쳐(*.dxt)|*.dxt||", nullptr);

	if (dlg.DoModal() == IDCANCEL)
		return;

	int idx             = m_CB_Seed.AddString((LPCTSTR) dlg.GetFileName());

	unsigned char NewID = 1;
	int id;

	for (int i = 0; i < count; i++)
	{
		LPSEEDTEX pSeed = (LPSEEDTEX) m_CB_Seed.GetItemDataPtr(i);
		if (!pSeed)
			continue;

		NewID += (unsigned char) pSeed->ID;
	}

	if (!(NewID & 0x01))
		id = 0x01;
	else if (!(NewID & 0x02))
		id = 0x02;
	else if (!(NewID & 0x04))
		id = 0x04;
	else if (!(NewID & 0x08))
		id = 0x08;
	else if (!(NewID & 0x10))
		id = 0x10;
	else if (!(NewID & 0x20))
		id = 0x20;
	else if (!(NewID & 0x40))
		id = 0x40;
	else if (!(NewID & 0x80))
		id = 0x80;

	LPSEEDTEX pSeed = new SEEDTEX;
	pSeed->ID       = id;
	pSeed->pTex     = new CN3Texture;

	pSeed->pTex->LoadFromFile((LPCTSTR) dlg.GetPathName());

	m_CB_Seed.SetItemDataPtr(idx, pSeed);
	this->Invalidate(FALSE);
}

void CDlgSowSeed::OnBtnGrassDelete()
{
	int idx = m_CB_Seed.GetCurSel();
	if (idx < 0)
		return;

	LPSEEDTEX pSeed = (LPSEEDTEX) m_CB_Seed.GetItemDataPtr(idx);
	if (!pSeed)
		return;

	m_CB_Seed.DeleteString(idx);

	pSeed->pTex->Release();
	delete pSeed->pTex;
	pSeed->pTex = nullptr;
	delete pSeed;
	pSeed = nullptr;

	this->Invalidate(FALSE);
}

void CDlgSowSeed::OnDestroy()
{
	CDialog::OnDestroy();
	Release();
}

void CDlgSowSeed::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	if (m_pRefFrm->GetMapMng()->m_SowSeedMng.bActive == TRUE)
	{
		CWnd* pWnd = GetDlgItem(IDC_STATIC_GRASS);
		if (pWnd && m_pRefFrm->GetMapMng()->GetDlgSourceList()->GetSafeHwnd())
		{
			Obj_Index = m_pRefFrm->GetMapMng()->m_pDlgSourceList->m_ListShape.GetCurSel();
			if (Obj_Index < 0)
				return;

			CN3Shape* pShape = (CN3Shape*) m_pRefFrm->GetMapMng()
								   ->m_pDlgSourceList->m_ListShape.GetItemDataPtr(Obj_Index);
			if (pShape)
			{
				m_pRefFrm->GetMapMng()->RenderObjectToWindow(
					(CN3TransformCollision*) pShape, pWnd->m_hWnd);
			}
			else
			{
				CRect rc;
				pWnd->GetClientRect(&rc);
				CDC* pDC = pWnd->GetDC();
				pDC->FillSolidRect(&rc, RGB(192, 192, 192));
				pWnd->ReleaseDC(pDC);
			}
		}
	}
}

void CDlgSowSeed::OnSelchangeCbSeed()
{
	this->Invalidate(FALSE);
}

void CDlgSowSeed::OnSelchangeCbTilegroup()
{
	CMainFrame* pFrame = (CMainFrame*) AfxGetMainWnd();

	m_CB_Seed.ResetContent();

	it_Grass_Group it = pFrame->GetMapMng()->m_SowSeedMng.Grass_Group.begin();
	int size          = static_cast<int>(pFrame->GetMapMng()->m_SowSeedMng.Grass_Group.size());
	int index         = m_CB_TileGroup.GetCurSel();
	pFrame->GetMapMng()->m_SowSeedMng.Select_Group_Id = index;
	for (size_t ii = 0, listCount = 0; ii < size; ii++, it++)
	{
		if (index == ii)
		{
			LPGRASS_GROUP group_list = *it;
			for (LPGRASS grass_list : group_list->grass)
			{
				char text[256];
				sprintf(text, "SubID:%zu, x :%f, y :%f, z :%f", listCount, grass_list->Pos.x,
					grass_list->Pos.y, grass_list->Pos.z);
				m_CB_Seed.AddString(text);
				listCount++;
			}
		}
	}

	this->Invalidate(FALSE);
}

void CDlgSowSeed::OnSeedExit()
{
	// TODO: Add your control notification handler code here
	if (m_pRefFrm->GetMapMng()->GetTerrain())
		m_pRefFrm->GetMapMng()->GetTerrain()->SetEditMode(TEM_NOT);

	m_pRefFrm->GetMapMng()->m_SowSeedMng.SetActive(FALSE);
	ShowWindow(false);
}

void CDlgSowSeed::OnCustomdrawSowBrushSize(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here
	Brush_Size = m_Brush_Size.GetPos();
	m_pRefFrm->GetMapMng()->GetTerrain()->UpdateBrushIntensityMap(2, Brush_Size, 0);
	m_pRefFrm->GetMapMng()->m_SowSeedMng.Render_Grass = FALSE;
	OnButtonRandom();
	*pResult = 0;
}

void CDlgSowSeed::OnButtonRandom()
{
	// TODO: Add your control notification handler code here
	int Count = 0;
	for (int i = 0; i < MAX_BRUSH_SIZE; i++)
	{
		for (int j = 0; j < MAX_BRUSH_SIZE; j++)
		{
			if (m_pRefFrm->GetMapMng()->GetTerrain()->m_fBrushIntensityMap[i][j] <= 0)
				continue;

			Sow_Array[i][j] = rand() % 3;
			Count++;
		}
	}

	m_pRefFrm->GetMapMng()->m_SowSeedMng.Render_Grass = FALSE;
	m_pRefFrm->GetMapMng()->m_SowSeedMng.Create_Grass(Count);
	m_pRefFrm->GetMapMng()->m_SowSeedMng.Random_Grass();
}

void CDlgSowSeed::OnCustomdrawSTileDetail(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here
	SowDetail                                         = m_SowDetail.GetPos();
	m_pRefFrm->GetMapMng()->m_SowSeedMng.Render_Grass = FALSE;
	m_pRefFrm->GetMapMng()->m_SowSeedMng.Random_Grass();
	*pResult = 0;
}

void CDlgSowSeed::OnRadioSow()
{
	// TODO: Add your control notification handler code here
	Sow_Select_Flage                                     = CS_SOW;
	m_pRefFrm->GetMapMng()->m_SowSeedMng.Select_Group_Id = -1;
	m_pRefFrm->GetMapMng()->GetTerrain()->SetEditMode(TEM_BRUSH_SHOW);
}
void CDlgSowSeed::OnRadioSelect()
{
	// TODO: Add your control notification handler code here
	Sow_Select_Flage = CS_SELECT;
	m_pRefFrm->GetMapMng()->GetTerrain()->SetEditMode(TEM_NOT);
}


// DlgSetDTex.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "DTex.h"
#include "DTexMng.h"
#include "DTexGroupMng.h"
#include "MainFrm.h"
#include <N3Base/N3EngTool.h>
#include <N3Base/N3Texture.h>
#include "DlgInputAttr.h"
#include "DlgInputGroup.h"
#include "DlgDelGroup.h"
#include "DlgAddDTex.h"
#include "MapMng.h"
#include "LyTerrain.h"
#include "DlgLoadTileSet.h"
#include "DlgSaveNewTileSet.h"

#include "DlgSetDTex.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgSetDTex dialog


CDlgSetDTex::CDlgSetDTex(CWnd* pParent /*=nullptr*/)
	: CDialog(CDlgSetDTex::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgSetDTex)
	m_szCurrAttr = _T("^__________^");
	m_TileSetName = _T("");
	//}}AFX_DATA_INIT

	m_pTex = nullptr;
	m_pTexVB = nullptr;
	m_fTexSurfaceSize = 512.0f;
}


void CDlgSetDTex::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSetDTex)
	DDX_Control(pDX, IDC_LIST_GROUP, m_GroupList);
	DDX_Control(pDX, IDC_COMBO_FILELIST, m_FileList);
	DDX_Text(pDX, IDC_ATTR, m_szCurrAttr);
	DDV_MaxChars(pDX, m_szCurrAttr, 80);
	DDX_Text(pDX, IDC_STATIC_TILESETNAME, m_TileSetName);
	DDV_MaxChars(pDX, m_TileSetName, 200);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgSetDTex, CDialog)
	//{{AFX_MSG_MAP(CDlgSetDTex)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_CBN_SELCHANGE(IDC_COMBO_FILELIST, OnSelchangeComboFilelist)
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONUP()
	ON_BN_CLICKED(IDC_BTN_SAVE, OnBtnSave)
	ON_BN_CLICKED(IDC_BTN_ADDGROUP, OnBtnAddgroup)
	ON_BN_CLICKED(IDC_BTN_DELGROUP, OnBtnDelgroup)
	ON_BN_CLICKED(IDC_BTN_ADDDTEX, OnBtnAdddtex)
	ON_BN_CLICKED(IDC_BTN_DELDTEX, OnBtnDeldtex)
	ON_BN_CLICKED(IDC_BTN_SAVE_NEWNAME_TILESET, OnBtnSaveNewnameTileset)
	ON_BN_CLICKED(IDC_BTN_SAVE_TILESET, OnBtnSaveTileset)
	ON_BN_CLICKED(IDC_BTN_LOAD_TILESET, OnBtnLoadTileset)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgSetDTex message handlers

BOOL CDlgSetDTex::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_FileList.ResetContent();

	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	CDTexMng* pDTexMng = pFrm->GetDTexMng();

	m_TileSetName = pFrm->m_DTexInfoFileName;

	int i = 0;
	for (CDTex* pDTex : pDTexMng->m_pDTex)
	{
		if (pDTex == nullptr)
			continue;

		m_FileList.InsertString(i, pDTex->m_pTex->FileName().c_str());
		m_FileList.SetItemDataPtr(i, pDTex);
		++i;
	}
	m_FileList.SetCurSel(0);

	CDTexGroupMng* pGroup = pFrm->GetDTexGroupMng();

	i = 0;
	for (CDTexGroup* pDTG : pGroup->m_Groups)
	{
		m_GroupList.InsertString(i, pDTG->m_Name);
		++i;
	}

	CWnd* pView = GetDlgItem(IDC_TEXTUREVIEW);
	pView->SetWindowPos(nullptr, 0, 0, (int)m_fTexSurfaceSize, (int)m_fTexSurfaceSize, SWP_DRAWFRAME|SWP_NOMOVE);

	//	texture 그리는 버퍼..
	pFrm->m_pEng->s_lpD3DDev->CreateVertexBuffer( 4*sizeof(__VertexTransformed), 0, FVF_TRANSFORMED, D3DPOOL_MANAGED, &m_pTexVB, nullptr );
	
	__VertexTransformed* pVertices = nullptr;
	m_pTexVB->Lock( 0, 0, (void**)&pVertices, 0 );
	float DTexSize = (float)DTEX_SIZE;
	pVertices[0].Set(0.0f,						0.0f,					0.1f, 0.5f, 0x00000000, 1.0f/DTexSize,		1.0f/DTexSize);
	pVertices[1].Set(m_fTexSurfaceSize-1.0f,	0.0f,					0.1f, 0.5f, 0x00000000, DTexSize/DTexSize,	1.0f/DTexSize);
	pVertices[2].Set(m_fTexSurfaceSize-1.0f,	m_fTexSurfaceSize-1.0f,	0.1f, 0.5f, 0x00000000, DTexSize/DTexSize,	DTexSize/DTexSize);
	pVertices[3].Set(0.0f,						m_fTexSurfaceSize-1.0f,	0.1f, 0.5f, 0x00000000, 1.0f/DTexSize,		DTexSize/DTexSize);
	m_pTexVB->Unlock();

	//	Grid 그리는 버퍼..
	pFrm->m_pEng->s_lpD3DDev->CreateVertexBuffer( ((NUM_DTEXTILE-1)<<2)*sizeof(__VertexTransformedColor), 0, FVF_TRANSFORMEDCOLOR, D3DPOOL_MANAGED, &m_pGridVB, nullptr );

	__VertexTransformedColor* pVerticesC = nullptr;
	m_pGridVB->Lock(0,0, (void**)&pVerticesC, 0);

	int GridInterval = (int) m_fTexSurfaceSize / NUM_DTEXTILE;
	for (i = 1; i < NUM_DTEXTILE; i++)
	{
		int index = (i - 1) * 4;
		pVerticesC[index].Set((float)((i*GridInterval)-1),	0.0f,							0.1f, 0.5f, 0xff800080);
		pVerticesC[index+1].Set((float)((i*GridInterval)-1),511.0f,							0.1f, 0.5f, 0xff800080);
		pVerticesC[index+2].Set(0.0f,						(float)((i*GridInterval)-1),	0.1f, 0.5f, 0xff800080);
		pVerticesC[index+3].Set(511.0f,						(float)((i*GridInterval)-1),	0.1f, 0.5f, 0xff800080);
	}
	m_pGridVB->Unlock();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgSetDTex::OnDestroy() 
{
	if(m_pTexVB)
	{
		m_pTexVB->Release();
		m_pTexVB = nullptr;
	}
	if(m_pGridVB)
	{
		m_pGridVB->Release();
		m_pGridVB = nullptr;
	}

	CDialog::OnDestroy();
}


void CDlgSetDTex::RenderTex(LPDIRECT3DDEVICE9 lpDDev)
{
	if(!m_pTexVB) return;
	
	HRESULT hr;	
	LPDIRECT3DTEXTURE9 lpTex = nullptr;
	int CurrTex = m_FileList.GetCurSel();
	if(CurrTex<0) return;

	CDTex* pDTex = (CDTex*)m_FileList.GetItemDataPtr(CurrTex);
	if(pDTex && pDTex->m_pTex) lpTex = pDTex->m_pTex->Get();

	hr = lpDDev->SetTexture(0, lpTex);
	DWORD ColorOp, ColorArg1;

	hr = lpDDev->GetTextureStageState( 0, D3DTSS_COLOROP, &ColorOp);
	hr = lpDDev->GetTextureStageState( 0, D3DTSS_COLORARG1, &ColorArg1);
	
	hr = lpDDev->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	hr = lpDDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		
	hr = lpDDev->SetStreamSource( 0, m_pTexVB, 0, sizeof(__VertexTransformed) );
    hr = lpDDev->SetFVF( FVF_TRANSFORMED );
/*
	DWORD IsAlpha, Srcblend, Destblend;

	lpDDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &IsAlpha);
	lpDDev->GetRenderState(D3DRS_SRCBLEND, &Srcblend);
	lpDDev->GetRenderState(D3DRS_DESTBLEND, &Destblend);
	
	lpDDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	lpDDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	lpDDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
*/		
	hr = lpDDev->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 2);
/*
	lpDDev->SetRenderState(D3DRS_ALPHABLENDENABLE, IsAlpha);
	lpDDev->SetRenderState(D3DRS_SRCBLEND, Srcblend);
	lpDDev->SetRenderState(D3DRS_DESTBLEND, Destblend);
*/
	hr = lpDDev->SetTextureStageState( 0, D3DTSS_COLOROP, ColorOp);
	hr = lpDDev->SetTextureStageState( 0, D3DTSS_COLORARG1, ColorArg1);
}


void CDlgSetDTex::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	if(!pFrm) return;

	CWnd* pView = GetDlgItem(IDC_TEXTUREVIEW);
	if(!pView) return;

	HRESULT hr;
	hr = pFrm->m_pEng->s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, TRUE);
	hr = pFrm->m_pEng->s_lpD3DDev->SetRenderState(D3DRS_FOGCOLOR, D3DCOLOR_ARGB(255,192,0,192));
	
	CRect rc;
	GetClientRect(&rc);
	pFrm->m_pEng->Clear(0xff606060, rc);
	hr = pFrm->m_pEng->s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, FALSE);
	hr = pFrm->m_pEng->s_lpD3DDev->BeginScene();

	//render....
	DWORD zEnable;
	pFrm->m_pEng->s_lpD3DDev->GetRenderState(D3DRS_ZENABLE, &zEnable);
	pFrm->m_pEng->s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	RenderTex(pFrm->m_pEng->s_lpD3DDev);
	RenderGrid(pFrm->m_pEng->s_lpD3DDev);
	pFrm->m_pEng->s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, zEnable);
	
	hr = pFrm->m_pEng->s_lpD3DDev->EndScene();
    pFrm->m_pEng->Present(pView->m_hWnd);
}


void CDlgSetDTex::OnSelchangeComboFilelist() 
{
	int index;
	index = m_FileList.GetCurSel();
	Invalidate(FALSE);
	//딴거 넣어야 돼...
}


void CDlgSetDTex::RenderGrid(LPDIRECT3DDEVICE9 lpDDev)
{
	HRESULT hr;
	hr = lpDDev->SetTexture(0, nullptr);

	DWORD ColorOp, ColorArg1;
	hr = lpDDev->GetTextureStageState( 0, D3DTSS_COLOROP, &ColorOp);
	hr = lpDDev->GetTextureStageState( 0, D3DTSS_COLORARG1, &ColorArg1);
	
	hr = lpDDev->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	hr = lpDDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	
	hr = lpDDev->SetStreamSource( 0, m_pGridVB, 0, sizeof(__VertexTransformedColor) );
    hr = lpDDev->SetFVF( FVF_TRANSFORMEDCOLOR );
	hr = lpDDev->DrawPrimitive( D3DPT_LINELIST, 0, (NUM_DTEXTILE-1)<<1 );

	hr = lpDDev->SetTextureStageState( 0, D3DTSS_COLOROP, ColorOp);
	hr = lpDDev->SetTextureStageState( 0, D3DTSS_COLORARG1, ColorArg1);
}

void CDlgSetDTex::OnMouseMove(UINT nFlags, CPoint point) 
{
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	if(!pFrm) return;
	
	CWnd* pView = GetDlgItem(IDC_TEXTUREVIEW);
	if(!pView) return;

	CRect rt;
	pView->GetWindowRect(&rt);
	this->ScreenToClient(&rt);

	int index = m_FileList.GetCurSel();

	if(rt.PtInRect(point) && (index>-1))
	{
		CPoint pt;
		pt.x = point.x - rt.left;
		pt.y = point.y - rt.top;

		int GridInterval = (int)m_fTexSurfaceSize / NUM_DTEXTILE;
		int ix = pt.x / GridInterval;
		int iy = pt.y / GridInterval;

		CDTex* pDTex = (CDTex*)m_FileList.GetItemDataPtr(index);

		DTEXATTR attr;
		if(pDTex) attr = pDTex->m_Attr[ix][iy];

		CString szAttr, szGroup;

		switch(attr.Attr)
		{
		case DTEX_FULL:
			szAttr.Format("FULL");
			break;
		case DTEX_1PER2:
			szAttr.Format("1/2");
			break;
		case DTEX_1PER4:
			szAttr.Format("1/4");
			break;
		case DTEX_3PER4:
			szAttr.Format("3/4");
			break;
		case DTEX_1PER8:
			szAttr.Format("1/8");
			break;
		case DTEX_7PER8:
			szAttr.Format("7/8");
			break;
		case DTEX_5PER8:
			szAttr.Format("5/8");
			break;
		case DTEX_3PER8:
			szAttr.Format("3/8");
			break;
		}

		int GroupID;
		if(pDTex) GroupID = pDTex->m_Attr[ix][iy].Group;
		szGroup = pFrm->GetDTexGroupMng()->GetGroupName(GroupID);

		m_szCurrAttr.Format("%s - %s",szGroup, szAttr);
		UpdateData(FALSE);
	}
	else 
	{
		m_szCurrAttr = _T(" ^_^v ");
		UpdateData(FALSE);
	}

	CDialog::OnMouseMove(nFlags, point);
}

void CDlgSetDTex::OnRButtonUp(UINT nFlags, CPoint point)
{
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	if(!pFrm) return;

	CWnd* pView = GetDlgItem(IDC_TEXTUREVIEW);
	if(!pView) return;

	CRect rt;
	pView->GetWindowRect(&rt);
	this->ScreenToClient(&rt);

	int index = m_FileList.GetCurSel();
	if(rt.PtInRect(point) && (index>-1))
	{
		CPoint pt;
		pt.x = point.x - rt.left;
		pt.y = point.y - rt.top;

		int GridInterval = (int)m_fTexSurfaceSize / NUM_DTEXTILE;
		int ix = pt.x / GridInterval;
		int iy = pt.y / GridInterval;		

		CDTexGroupMng* pDTexGroupMng = pFrm->GetDTexGroupMng();

		DTEXATTR attr;
		CDTex* pDTex = (CDTex*)m_FileList.GetItemDataPtr(index);
		if(pDTex) attr = pDTex->m_Attr[ix][iy];

		CDlgInputAttr dlg;
		dlg.m_Point = point;
		ClientToScreen(&(dlg.m_Point));

		dlg.m_SelectedGroupID = attr.Group;

		if(dlg.DoModal()==IDOK)
		{
			DTEXTILEATTR tile;
			tile.TexID = pDTex->m_ID;	//index;
			tile.TileY = iy;

			for(int i=0;i<NUM_DTEXTILE;i++)
			{
				tile.TileX = i;
				if(pDTex)
				{
					attr = pDTex->m_Attr[i][iy];
					pDTexGroupMng->DelTile(attr.Group, attr.Attr, tile);
					pDTex->m_Attr[i][iy].Attr = i;
					pDTex->m_Attr[i][iy].Group = dlg.m_SelectedGroupID;
				
					pDTexGroupMng->SetTile(pDTex->m_Attr[i][iy].Group, pDTex->m_Attr[i][iy].Attr, tile);
				}
			}
		}
	}
	CDialog::OnRButtonUp(nFlags, point);
}

void CDlgSetDTex::OnBtnSave() 
{
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	CDTexMng* pDTexMng = pFrm->GetDTexMng();
	CDTexGroupMng* pDTexGroupMng = pFrm->GetDTexGroupMng();

	if(m_TileSetName.IsEmpty()) return;

	pDTexGroupMng->SaveToFile(m_TileSetName);
	pDTexMng->SaveToFile(m_TileSetName);
	
	pFrm->m_DTexInfoFileName = m_TileSetName;
}

void CDlgSetDTex::OnBtnAddgroup() 
{
	CDlgInputGroup dlg;
	if(IDCANCEL == dlg.DoModal()) return;

	dlg.m_GroupName.Remove(' ');
	if(dlg.m_GroupName.IsEmpty()) return;

	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	CDTexGroupMng* pDTexGroupMng = pFrm->GetDTexGroupMng();

	pDTexGroupMng->SetGroup((LPCTSTR)dlg.m_GroupName);

	int NumGroup = m_GroupList.GetCount();
	int i;
	for(i=0;i<NumGroup;i++)
	{
		CString str;
		m_GroupList.GetText(i, str);
		if(str == dlg.m_GroupName) break;
	}
	if(i==NumGroup)	m_GroupList.InsertString(i, dlg.m_GroupName);
}

void CDlgSetDTex::OnBtnDelgroup() 
{
	CDlgDelGroup dlg;

	if(dlg.DoModal()==IDOK)
	{
		int DelGroupID = dlg.m_SelGroupID;
		if(DelGroupID<=0) return;

		CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
		CDTexGroupMng* pDTexGroupMng = pFrm->GetDTexGroupMng();

		char* pName = pDTexGroupMng->GetGroupName(DelGroupID);
		
		int NumGroup = m_GroupList.GetCount();
		for(int i=0;i<NumGroup;i++)
		{
			CString str;
			m_GroupList.GetText(i, str);
			if(strcmp((LPCTSTR)str, pName)==0)
			{
				m_GroupList.DeleteString(i);
				break;
			}				
		}
		pDTexGroupMng->DelGroup(DelGroupID);

		//맵정보도 갱신해야겠지? 무효한 그룹정보니까 지워버려야지....
		CLyTerrain* pRefTerrain = pFrm->GetMapMng()->GetTerrain();
		if(pRefTerrain)
		{
			int ix,iz;
			for(ix=0; ix<pRefTerrain->m_iHeightMapSize; ix++)
			{
				for(iz=0; iz<pRefTerrain->m_iHeightMapSize; iz++)
				{
					int TargetID;
					
					//2번...
					TargetID = pRefTerrain->m_ppMapData[ix][iz].DTexInfo2.Attr.Group;
					if(TargetID==DelGroupID)
					{
						pRefTerrain->m_ppMapData[ix][iz].DTexInfo2.Attr.Group = 0;
						pRefTerrain->m_ppMapData[ix][iz].DTexInfo2.Attr.Attr = 0;

						pRefTerrain->m_ppMapData[ix][iz].DTexInfo2.TexIdx.TexID = -1;
					}

					//1번...
					TargetID = pRefTerrain->m_ppMapData[ix][iz].DTexInfo1.Attr.Group;
					if(TargetID==DelGroupID)
					{
						pRefTerrain->m_ppMapData[ix][iz].DTexInfo1 = pRefTerrain->m_ppMapData[ix][iz].DTexInfo2;

						pRefTerrain->m_ppMapData[ix][iz].DTexInfo2.Attr.Group = 0;
						pRefTerrain->m_ppMapData[ix][iz].DTexInfo2.Attr.Attr = 0;

						pRefTerrain->m_ppMapData[ix][iz].DTexInfo2.TexIdx.TexID = -1;
					}
				}
			}
		}
	}
}

void CDlgSetDTex::OnBtnAdddtex() 
{
	CDlgAddDTex dlg;

	if(dlg.DoModal()==IDOK)
	{
		CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
		CDTexMng* pDTexMng = pFrm->GetDTexMng();
		CDTexGroupMng* pDTexGroupMng = pFrm->GetDTexGroupMng();

		dlg.m_TexName.MakeLower();

		if(pDTexMng->AddDTex(dlg.m_TexName))
		{
			CDTex* pDTex = pDTexMng->GetDTexByName(dlg.m_TexName);
			
			if(pDTex)
			{
				int idx = m_FileList.GetCount();
				m_FileList.InsertString(idx, (LPCTSTR)dlg.m_TexName);
				m_FileList.SetItemDataPtr(idx, pDTex);
				m_FileList.SetCurSel(idx);

				DTEXTILEATTR tile;
				tile.TexID = pDTex->m_ID;

				for(int y=0; y<NUM_DTEXTILE; y++)
				{
					for(int x=0; x<NUM_DTEXTILE; x++)
					{
						tile.TileX = x;
						tile.TileY = y;

						pDTexGroupMng->SetTile(0, x, tile);
					}
				}
				Invalidate(FALSE);
			}			
		}
	}
}

void CDlgSetDTex::OnBtnDeldtex() 
{
	int count = m_FileList.GetCount();
	if(count<=0) return;
	int index = m_FileList.GetCurSel();
	int DelDTexID;

	//dtex지우기...^^
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	CDTexGroupMng* pDTexGroupMng = pFrm->GetDTexGroupMng();

	CDTex* pDTex = (CDTex*)m_FileList.GetItemDataPtr(index);
	DelDTexID = pDTex->m_ID;

	pDTexGroupMng->DelTileByDTexID(DelDTexID);

	CDTexMng* pDTexMng = pFrm->GetDTexMng();
	pDTexMng->DelDTexByID(DelDTexID);

	m_FileList.SetItemDataPtr(index, nullptr);
	m_FileList.DeleteString(index);
	if(index==count-1) m_FileList.SetCurSel(index-1);
	else m_FileList.SetCurSel(index);

	//맵정보도 갱신해야겠지? 없는 텍스쳐정보는 같은 그룹의 다른 텍스쳐로 갱신..
	CLyTerrain* pRefTerrain = pFrm->GetMapMng()->GetTerrain();
	if(pRefTerrain)
	{
		int ix,iz;
		for(ix=0; ix<pRefTerrain->m_iHeightMapSize; ix++)
		{
			for(iz=0; iz<pRefTerrain->m_iHeightMapSize; iz++)
			{
				int TargetID;
				int Group,Attr;
				DTEXTILEATTR DTexTileAttr;

				//2번...
				TargetID = pRefTerrain->m_ppMapData[ix][iz].DTexInfo2.TexIdx.TexID;
				if(TargetID==DelDTexID)
				{
					Group = pRefTerrain->m_ppMapData[ix][iz].DTexInfo2.Attr.Group;
					Attr = pRefTerrain->m_ppMapData[ix][iz].DTexInfo2.Attr.Attr;
					DTexTileAttr = pDTexGroupMng->GetTileAttr(Group, Attr);
					pRefTerrain->m_ppMapData[ix][iz].DTexInfo2.TexIdx = DTexTileAttr;

					if(DTexTileAttr.TexID==-1)
					{
						pRefTerrain->m_ppMapData[ix][iz].DTexInfo2.Attr.Group = 0;						
					}
				}

				//1번...
				TargetID = pRefTerrain->m_ppMapData[ix][iz].DTexInfo1.TexIdx.TexID;
				if(TargetID==DelDTexID)
				{
					Group = pRefTerrain->m_ppMapData[ix][iz].DTexInfo1.Attr.Group;
					Attr = pRefTerrain->m_ppMapData[ix][iz].DTexInfo1.Attr.Attr;
					DTexTileAttr = pDTexGroupMng->GetTileAttr(Group, Attr);
					pRefTerrain->m_ppMapData[ix][iz].DTexInfo1.TexIdx = DTexTileAttr;

					if(DTexTileAttr.TexID==-1)
					{
						pRefTerrain->m_ppMapData[ix][iz].DTexInfo1 = pRefTerrain->m_ppMapData[ix][iz].DTexInfo2;
						pRefTerrain->m_ppMapData[ix][iz].DTexInfo2.Attr.Group = 0;
						pRefTerrain->m_ppMapData[ix][iz].DTexInfo2.TexIdx = DTexTileAttr;						
					}
				}
			}
		}
	}

	Invalidate(FALSE);
}

void CDlgSetDTex::OnBtnSaveNewnameTileset() 
{
	CDlgSaveNewTileSet dlg;

	if(dlg.DoModal()==IDOK)
	{
		m_TileSetName = dlg.m_NewTileSetName;
		OnBtnSave();
	}		
}

void CDlgSetDTex::OnBtnSaveTileset() 
{
	OnBtnSave();	
}

void CDlgSetDTex::OnBtnLoadTileset() 
{
	CDlgLoadTileSet dlg;
	
	if(dlg.DoModal()==IDOK)
	{
		CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
		
		if(dlg.m_SelFileName==m_TileSetName) return;

		pFrm->m_DTexInfoFileName = dlg.m_SelFileName;
		m_TileSetName = dlg.m_SelFileName;
		
		CDTexMng* pDTexMng = pFrm->GetDTexMng();
		CDTexGroupMng* pDTexGroupMng = pFrm->GetDTexGroupMng();

		pDTexGroupMng->LoadFromFile(m_TileSetName);
		pDTexMng->LoadFromFile(m_TileSetName);
		
		//reset dialog box...
		m_FileList.ResetContent();
		m_GroupList.ResetContent();
		int i = 0;
		for (CDTex* pDTex : pDTexMng->m_pDTex)
		{
			if (pDTex == nullptr)
				continue;

			m_FileList.InsertString(i, pDTex->m_pTex->FileName().c_str());
			m_FileList.SetItemDataPtr(i, pDTex);
			++i;
		}
		m_FileList.SetCurSel(0);

		i = 0;
		for (CDTexGroup* pDTG : pDTexGroupMng->m_Groups)
		{
			m_GroupList.InsertString(i, pDTG->m_Name);
			++i;
		}

		Invalidate(FALSE);
	}
}

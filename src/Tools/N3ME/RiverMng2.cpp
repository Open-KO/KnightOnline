// RiverMng2.cpp: implementation of the CRiverMng2 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "n3me.h"
#include "RiverMng2.h"

#include "../N3Base/N3Texture.h"
#include "LyTerrain.h"
#include "Dlg_River.h"
#include "MainFrm.h"
#include "MapMng.h"
#include "EventMgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRiverMng2::CRiverMng2()
{
	m_bActive = false;
	m_pRefTerrain = NULL;
	m_ppRiver = NULL;
	//m_ppIsRiver = NULL;
	m_nMapSize = 0;
	m_pDlgRiver = NULL;
	m_rtDrag.SetRectEmpty();
	m_bDrag = false;
	m_bDragRightBtn = false;

	m_pTexRiver[0] = NULL;
	m_pTexWave = NULL;
	m_fTexIndex = 0.0f;
	m_pRefEventMgr = NULL;
	m_nVer = VER_RIVER;
}

CRiverMng2::~CRiverMng2()
{
	Release();
	if (m_pDlgRiver)
	{
		m_pDlgRiver->DestroyWindow();
		delete m_pDlgRiver, m_pDlgRiver = NULL;
	}
}

void CRiverMng2::Release()
{
	if (m_ppRiver)
	{
		for (int i=0;i<m_nMapSize;i++)
			delete[] m_ppRiver[i];
		delete[] m_ppRiver, m_ppRiver = NULL;
	}
	if (m_pTexRiver[0])
	{
		for (int i=0;i<MAX_RIVER_TEX;i++)
		{
			s_MngTex.Delete(m_pTexRiver[i]);
		}
		s_MngTex.Delete(m_pTexWave);
	}
	m_pTexRiver[0] = NULL;
	m_nMapSize = 0;
}

void CRiverMng2::Tick()
{
}

void CRiverMng2::Render()
{
	if (!m_bActive) return;

	// Tick
	m_fTexIndex += s_fSecPerFrm*15.0f;
	if (m_fTexIndex >= 32.0f)
	{
		m_fTexIndex -= 32.0f;
	}
	//


	HRESULT hr;

	D3DXMATRIX mtx;	D3DXMatrixIdentity(&mtx);
		
	hr = s_lpD3DDev->SetTransform(D3DTS_WORLD, &mtx); // 월드 행렬 적용..

	// backup state
	DWORD dwAlphaEnable, dwSrcBlend, dwDestBlend;
	//DWORD dwZEnable, dwLighting;
	s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlphaEnable);
	s_lpD3DDev->GetRenderState(D3DRS_SRCBLEND, &dwSrcBlend);
	s_lpD3DDev->GetRenderState(D3DRS_DESTBLEND, &dwDestBlend);
	//hr = s_lpD3DDev->GetRenderState(D3DRS_ZENABLE, &dwZEnable);
	//hr = s_lpD3DDev->GetRenderState(D3DRS_LIGHTING, &dwLighting);

	// set state
	s_lpD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);
    s_lpD3DDev->SetRenderState( D3DRS_SRCBLEND,   D3DBLEND_SRCALPHA );
    s_lpD3DDev->SetRenderState( D3DRS_DESTBLEND,  D3DBLEND_INVSRCALPHA );
	//hr = s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	//hr = s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);

	// set texture
	s_lpD3DDev->SetTexture(0, m_pTexRiver[(int)m_fTexIndex]->Get());
	s_lpD3DDev->SetTexture(1, m_pTexWave->Get());
	//s_lpD3DDev->SetTexture(1, NULL);
	s_lpD3DDev->SetTexture(2, NULL);
	
	s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
    s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);	
	s_lpD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	s_lpD3DDev->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	//s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE);
	s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE);
//    s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
//    s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT);	


	s_lpD3DDev->SetVertexShader(D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX2);
	//s_lpD3DDev->SetVertexShader(D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1);

	int x,z;
	__VertexRiver Vtx[4];	

	// Render River Region.
	__CellData **ppCell = m_pRefEventMgr->GetCellData();
	for (x=m_pRefTerrain->m_VisibleRect.left;x<=m_pRefTerrain->m_VisibleRect.right;x++)
	{
		for (z = m_pRefTerrain->m_VisibleRect.top;z<=m_pRefTerrain->m_VisibleRect.bottom ;z++)
		{
			if (x <0 || z <0 || x >= m_nMapSize-1 || z >= m_nMapSize-1) continue;
			if (ppCell[x][z].byTerrainAttribute == TA_WATER)
			{
				MakeTileVB(Vtx, x, z);
				hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Vtx, sizeof(__VertexRiver));
			}
		}
	}

	// restore
	//s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, dwZEnable);
	//s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, dwLighting);
	s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlphaEnable);
	s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, dwSrcBlend);
	s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, dwDestBlend);

	s_lpD3DDev->SetTexture(1, NULL);

	// Render Drag Rect
	if (m_bDrag)
	{
		CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
		pFrame->GetMapMng()->RenderDragRect(&m_rtDragTransformed);
	}
	if (m_bDragRightBtn)
	{
		CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
		pFrame->GetMapMng()->RenderDragRect(&m_rtDragTransformed);
	}
}


BOOL CRiverMng2::MouseMsgFilter(LPMSG pMsg)
{
	if (!m_bActive) return false;
	
	switch(pMsg->message)
	{
	case WM_LBUTTONDOWN:
		{
			__Vector3 vPick;
			POINT pt = {short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam))};
			if(m_pRefTerrain->Pick(pt.x, pt.y, &vPick, NULL))
			{
				vPick.x /= TERRAIN_CELL_SIZE;
				vPick.z /= TERRAIN_CELL_SIZE;

				m_rtDrag.left = (int)vPick.x;
				m_rtDrag.top = (int)vPick.z;
				m_rtDrag.right = (int)vPick.x;
				m_rtDrag.bottom = (int)vPick.z;
				m_bDrag = true;

				m_rtDragTransformed.left = m_rtDragTransformed.right = pt.x;
				m_rtDragTransformed.top = m_rtDragTransformed.bottom = pt.y;
			}
		}
		break;
	case WM_MOUSEMOVE:
		if (m_bDrag)
		{
			POINT pt = {short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam))};

			__Vector3 vec;
			if(!m_pRefTerrain->Pick(pt.x, pt.y, &vec, NULL)) break;
			vec.x /= TERRAIN_CELL_SIZE;
			vec.z /= TERRAIN_CELL_SIZE;

			m_rtDrag.right = (int)vec.x;
			m_rtDrag.bottom = (int)vec.z;

			m_rtDragTransformed.right = pt.x;
			m_rtDragTransformed.bottom = pt.y;
		}
		if (m_bDragRightBtn)
		{
			POINT pt = {short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam))};

			__Vector3 vec;
			if(!m_pRefTerrain->Pick(pt.x, pt.y, &vec, NULL)) break;
			vec.x /= TERRAIN_CELL_SIZE;
			vec.z /= TERRAIN_CELL_SIZE;

			m_rtDrag.right = (int)vec.x;
			m_rtDrag.bottom = (int)vec.z;

			m_rtDragTransformed.right = pt.x;
			m_rtDragTransformed.bottom = pt.y;
		}
		break;
	case WM_LBUTTONUP:
		{
			POINT point = {short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam))};

			__Vector3 vec;
			if(!m_pRefTerrain->Pick(point.x, point.y, &vec, NULL)) break;
			vec.x /= TERRAIN_CELL_SIZE;
			vec.z /= TERRAIN_CELL_SIZE;

			m_rtDrag.right = (int)vec.x;
			m_rtDrag.bottom = (int)vec.z;
			m_rtDrag.NormalizeRect();
			m_bDrag = false;


			// Input River Data
			__CellData **ppCell = m_pRefEventMgr->GetCellData();
			int x,z;
			float		fHeight = m_pDlgRiver->GetHeight();
			D3DCOLOR	Diffuse = m_pDlgRiver->GetColor();
			for(x=m_rtDrag.left; x<=m_rtDrag.right; x++)
			{
				for(z=m_rtDrag.top; z<=m_rtDrag.bottom; z++)
				{
					if (x>=0 && z>=0 && x<m_nMapSize-1 && z<m_nMapSize-1) 
					{
						m_ppRiver[x  ][z  ].fHeight = fHeight;
						m_ppRiver[x+1][z  ].fHeight = fHeight;
						m_ppRiver[x  ][z+1].fHeight = fHeight;
						m_ppRiver[x+1][z+1].fHeight = fHeight;

						m_ppRiver[x  ][z  ].Diffuse = Diffuse;
						m_ppRiver[x+1][z  ].Diffuse = Diffuse;
						m_ppRiver[x  ][z+1].Diffuse = Diffuse;
						m_ppRiver[x+1][z+1].Diffuse = Diffuse;

						//m_ppIsRiver[x][z] = true;
						ppCell[x][z].byTerrainAttribute = TA_WATER;
					}
				}
			}
		}
		break;
	case WM_RBUTTONDOWN:
		{
			__Vector3 vPick;
			POINT pt = {short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam))};
			if(m_pRefTerrain->Pick(pt.x, pt.y, &vPick, NULL))
			{
				vPick.x /= TERRAIN_CELL_SIZE;
				vPick.z /= TERRAIN_CELL_SIZE;

				m_rtDrag.left = (int)vPick.x;
				m_rtDrag.top = (int)vPick.z;
				m_rtDrag.right = (int)vPick.x;
				m_rtDrag.bottom = (int)vPick.z;
				m_bDragRightBtn = true;

				m_rtDragTransformed.left = m_rtDragTransformed.right = pt.x;
				m_rtDragTransformed.top = m_rtDragTransformed.bottom = pt.y;
			}
		}
		break;
	case WM_RBUTTONUP:
		{
			POINT point = {short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam))};

			__Vector3 vec;
			if(!m_pRefTerrain->Pick(point.x, point.y, &vec, NULL)) break;
			vec.x /= TERRAIN_CELL_SIZE;
			vec.z /= TERRAIN_CELL_SIZE;

			m_rtDrag.right = (int)vec.x;
			m_rtDrag.bottom = (int)vec.z;
			m_rtDrag.NormalizeRect();
			m_bDragRightBtn = false;


			// Del River Data
			int x,z;
			__CellData **ppCell = m_pRefEventMgr->GetCellData();
			for(x=m_rtDrag.left; x<=m_rtDrag.right; x++)
			{
				for(z=m_rtDrag.top; z<=m_rtDrag.bottom; z++)
				{
					if (x>=0 && z>=0 && x<m_nMapSize-1 && z<m_nMapSize-1) 
					{
						ppCell[x][z].byTerrainAttribute = TA_NONE;
					}
				}
			}
		}
		break;
	}

	return true;
}

void CRiverMng2::SetActive(bool bActive, CLyTerrain *pTerrain)
{
	if (bActive)
	{
		ASSERT(pTerrain);		

		CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
		m_pRefEventMgr = pFrame->GetMapMng()->GetEventMgr();
		m_pRefTerrain = pTerrain;
		if (NULL == m_ppRiver)
		{
			m_nMapSize = pTerrain->m_iHeightMapSize;
			m_ppRiver = new __River* [m_nMapSize];
			for (int i=0;i<m_nMapSize;i++)
			{
				m_ppRiver[i] = new __River[m_nMapSize];
				ZeroMemory(m_ppRiver[i], sizeof(__River)*m_nMapSize);
			}
		}
//		if (NULL == m_ppIsRiver)
//		{
//			ASSERT(m_nMapSize);
//			m_ppIsRiver = new bool* [m_nMapSize];
//			for (int i=0;i<m_nMapSize;i++)
//			{
//				m_ppIsRiver[i] = new bool[m_nMapSize];
//				ZeroMemory(m_ppIsRiver[i], sizeof(bool)*m_nMapSize);
//			}		
//		}
		if (NULL == m_pTexRiver[0])
		{
			char szFileName[40];
			for (int i=0;i<MAX_RIVER_TEX;i++)
			{
				sprintf(szFileName, "misc\\river\\caust%02d.tga", i);
				//sprintf(szFileName, "misc\\river\\caust%02d.dxt", i);
				m_pTexRiver[i] = s_MngTex.Get(szFileName);
				__ASSERT(m_pTexRiver[i], "CN3River2::texture load failed");
			}
			m_pTexWave = s_MngTex.Get("misc\\river\\el_water.bmp");
		}

		if (pTerrain->m_iHeightMapSize != m_nMapSize)
		{
			MessageBox(NULL, "RiverMng2 MapSize != Terrain MapSize", "error", MB_OK);
		}
		ASSERT(m_pDlgRiver == NULL);
		m_pDlgRiver = new CDlg_River();
		m_pDlgRiver->Create(IDD_RIVER);
		m_pDlgRiver->ShowWindow(SW_SHOW);
	}
	else
	{
		if (m_pDlgRiver)
		{
			m_pDlgRiver->DestroyWindow();
			delete m_pDlgRiver, m_pDlgRiver = NULL;
		}
	}
	m_bActive = bActive;
}

void CRiverMng2::MakeTileVB(__VertexRiver *pVtx, int x, int z)
{
	//DWORD color = 0xa0ffffff;
	DWORD color = 0x80ffffff;

	//if((x+z)%2==0)	// 슬래쉬 모양의 타일..
	{
		__Vector3 v;
		v.x = x*TERRAIN_CELL_SIZE;
		v.z = z*TERRAIN_CELL_SIZE;
		v.y = m_ppRiver[x][z].fHeight;
		color = m_ppRiver[x][z].Diffuse ;
		pVtx[0].Set(v.x, v.y, v.z, 0.0f, 1.0f, 0.0f, color, 0.0f, 0.0f, 0.0f, 0.0f);
		//pVtx[0].Set(v.x, v.y, v.z, 0.0f, 1.0f, 0.0f, color, 0.0f, 0.0f);//, 0.0f, 0.0f);

		v.z = (z+1)*TERRAIN_CELL_SIZE;
		v.y = m_ppRiver[x][z+1].fHeight;
		color = m_ppRiver[x][z+1].Diffuse ;
		pVtx[1].Set(v.x, v.y, v.z, 0.0f, 1.0f, 0.0f, color, 0.0f, 1.0f, 0.0f, 1.0f);
		//pVtx[1].Set(v.x, v.y, v.z, 0.0f, 1.0f, 0.0f, color, 0.0f, 1.0f);//, 0.0f, 1.0f);

		v.x = (x+1)*TERRAIN_CELL_SIZE;
		v.y = m_ppRiver[x+1][z+1].fHeight;
		color = m_ppRiver[x+1][z+1].Diffuse ;
		pVtx[2].Set(v.x, v.y, v.z, 0.0f, 1.0f, 0.0f, color, 1.0f, 1.0f, 1.0f, 1.0f);
		//pVtx[2].Set(v.x, v.y, v.z, 0.0f, 1.0f, 0.0f, color, 1.0f, 1.0f);//, 1.0f, 1.0f);

		v.z = z*TERRAIN_CELL_SIZE;
		v.y = m_ppRiver[x+1][z].fHeight;
		color = m_ppRiver[x+1][z].Diffuse ;
		pVtx[3].Set(v.x, v.y, v.z, 0.0f, 1.0f, 0.0f, color, 1.0f, 0.0f, 1.0f, 0.0f);
		//pVtx[3].Set(v.x, v.y, v.z, 0.0f, 1.0f, 0.0f, color, 1.0f, 0.0f);//, 1.0f, 0.0f);

		return;
	}
//	if((x+z)%2==1)	//백슬레쉬 모양의 타일..
//	{
//		__Vector3 v;
//		v.x = x*TERRAIN_CELL_SIZE;
//		v.z = (z+1)*TERRAIN_CELL_SIZE;
//		v.y = m_ppRiver[x][z+1];
//		pVtx[0].Set(v.x, v.y, v.z, color);
//
//		v.x = (x+1)*TERRAIN_CELL_SIZE;
//		v.y = m_ppRiver[x+1][z+1];
//		pVtx[1].Set(v.x, v.y, v.z, color);
//
//		v.z = z*TERRAIN_CELL_SIZE;
//		v.y = m_ppRiver[x+1][z];
//		pVtx[2].Set(v.x, v.y, v.z, color);
//
//		v.x = x*TERRAIN_CELL_SIZE;
//		v.y = m_ppRiver[x][z];
//		pVtx[3].Set(v.x, v.y, v.z, color);
//		return;
//	}
}

bool CRiverMng2::Load(HANDLE hFile)
{	
	Release();

	//
	DWORD dwRWC;

	// Map Size
	ReadFile(hFile, &m_nVer, sizeof(m_nVer), &dwRWC, NULL);
	if (m_nVer != VER_RIVER)
	{
		char buf[80];
		sprintf(buf, "Old File Format,(Latest:%d, This:%d)", VER_RIVER, m_nVer);
		MessageBox(NULL, buf, "Version", MB_OK);
		return false;
	}
	ReadFile(hFile, &m_nMapSize, sizeof(m_nMapSize), &dwRWC, NULL);


	ASSERT(NULL == m_ppRiver );
	ASSERT(m_nMapSize>0);
	// __River 
	m_ppRiver = new __River*[m_nMapSize];
	for (int i = 0;i<m_nMapSize;i++)
	{
		m_ppRiver[i] = new __River[m_nMapSize];
		ReadFile(hFile, m_ppRiver[i], sizeof(__River)*m_nMapSize, &dwRWC, NULL);
	}

	return true;
}

bool CRiverMng2::Save(HANDLE hFile)
{
	DWORD dwRWC = 0;

	// Map Size
	WriteFile(hFile, &m_nVer, sizeof(m_nVer), &dwRWC, NULL);
	WriteFile(hFile, &m_nMapSize, sizeof(m_nMapSize), &dwRWC, NULL);

	// __River 
	for (int i = 0;i<m_nMapSize;i++)
	{
		WriteFile(hFile, m_ppRiver[i], sizeof(__River)*m_nMapSize, &dwRWC, NULL);
	}

	return true;
}
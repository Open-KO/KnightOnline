// LyTerrain.cpp: implementation of the CLyTerrain class.
//	- 2001. 9.24.~ Map Editor용으로 변환...
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LyTerrain.h"
#include "QTNode.h"
#include <N3Base/N3Texture.h>
#include <N3Base/N3Scene.h>
#include "MainFrm.h"
#include "DTexGroup.h"
#include "DTexGroupMng.h"
#include "DTexMng.h"
#include "DTex.h"
#include "ProgressBar.h"
#include <N3Base/N3EngTool.h>
#include <N3Base/BitMapFile.h>
#include <N3Base/N3VMesh.h>
#include "DlgDTexGroupView.h"
#include "DlgModifyDTex.h"
#include "DlgSetLightMap.h"
#include "DlgSowSeed.h"
#include "SowSeedMng.h"
#include "MapMng.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLyTerrain::CLyTerrain()
{
	m_iZoneID = 0;

	m_pDlgSetLightMap = nullptr;

	m_ppMapData = nullptr;
	m_ppRenderInfo = nullptr;
	m_ppIsLightMap = nullptr;

	m_FillMode = D3DFILL_SOLID;

	m_pColorTexture = nullptr;

	m_ppLightMapTexture = nullptr;
	m_iNumLightMap = 0;

	m_pRoot = nullptr;
	m_pRefCurrNode = nullptr;

	SetRectEmpty(&m_VisibleRect);

	m_EyePos.x = m_EyePos.y = 0;

	m_ColorMapVB = nullptr;	//	컬러맵쓸때..
	m_TileVB = nullptr;		//	타일
	m_LightMapVB = nullptr;	//	하나의 라이트맵
	
	m_iHeightLimit = 600;	// 작을 수록 더 정밀도가 높아진다..
	m_iDistLimit = 30;		// 클수록 정밀도가 높아진다..

	m_iHeightMapSize = 1025;		
	m_iMaxLevel = 9;				//(log2(m_iHeightMapSize-1)) - 1
	
	m_iColorMapTexSize = 128;	//컬러맵 텍스쳐의 크기..
	m_iColorMapPixelPerUnitDistance = 4;		//UnitDistance당 들어가는 컬러맵의 픽셀 수..
	m_iNumColorMap = 32;

	m_iNumTileMap = 0;

	m_pDTexMng = nullptr;

	m_pColorMapTmpVB = nullptr;
	m_pColorMapTmpVertices = nullptr;

	int DTexAttrTable[9][14] = {
#ifdef _KNIGHT
		{3, 3, 1, 1, 0, 1, 2, 1, 3, 3, 0, 3, 0, 2},
		{0, 3, 1, 3, 3, 1, 1, 3, 3, 0, 0, 3, 0, 1},
		{3, 1, 1, 3, 1, 2, 1, 0, 3, 0, 3, 3, 0, 2},
		{3, 0, 3, 1, 3, 3, 1, 1, 3, 3, 0, 0, 0, 1},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{3, 1, 3, 0, 1, 1, 3, 3, 0, 0, 3, 3, 0, 1},
		{1, 3, 3, 1, 1, 0, 1, 2, 3, 3, 3, 0, 0, 2},
		{1, 3, 0, 3, 1, 3, 3, 1, 0, 3, 3, 0, 0, 1},
		{1, 1, 3, 3, 2, 1, 0, 1, 0, 3, 3, 3, 0, 2}		
#endif
#ifdef _EMPEROR
		{7, 3, 4, 5, 0, 4, 6, 5, 3, 7, 0, 3, 0, 6},	//	asm
		{0, 3, 4, 3, 3, 4, 4, 3, 3, 0, 0, 3, 0, 4},
		{7, 5, 4, 3, 5, 6, 4, 0, 3, 0, 7, 3, 0, 6},
		{7, 0, 3, 5, 7, 3, 5, 5, 3, 7, 0, 0, 0, 5},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{7, 5, 3, 0, 5, 5, 3, 7, 0, 0, 7, 3, 0, 5},
		{1, 7, 3, 5, 1, 0, 5, 2, 3, 7, 7, 0, 0, 2},
		{1, 7, 0, 7, 1, 7, 7, 1, 0, 7, 7, 0, 0, 1},
		{1, 5, 3, 7, 2, 5, 0, 1, 0, 7, 7, 3, 0, 2}		
#endif
	};
	memcpy(m_DTexAttrTable, DTexAttrTable, sizeof(int)*9*14);

	int DTexDirTable[9][14] = {
#ifdef _KNIGHT
		{1, 3, 2, 3, 0, 2, 2, 3, 0, 1, 0, 3, 0, 2},
		{0, 3, 2, 0, 3, 2, 2, 0, 0, 0, 0, 3, 0, 2},
		{2, 1, 2, 0, 1, 1, 2, 0, 0, 0, 2, 3, 0, 1},
		{1, 0, 0, 5, 1, 0, 5, 5, 0, 1, 0, 0, 0, 5},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{2, 1, 3, 0, 1, 1, 3, 2, 0, 0, 2, 3, 0, 1},
		{0, 2, 0, 3, 0, 0, 3, 3, 0, 1, 2, 0, 0, 3},
		{4, 2, 0, 1, 4, 2, 1, 4, 0, 1, 2, 0, 0, 4},
		{0, 1, 3, 1, 0, 1, 0, 0, 0, 1, 2, 3, 0, 0}
#endif
#ifdef _EMPEROR
		{0, 4, 0, 0, 0, 0, 4, 0, 0, 0, 0, 4, 0, 4},	//	asm
		{0, 4, 0, 0, 4, 0, 0, 0, 0, 0, 0, 4, 0, 0},
		{4, 4, 0, 0, 4, 0, 0, 0, 0, 0, 4, 4, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{4, 4, 4, 0, 4, 4, 4, 4, 0, 0, 4, 4, 0, 4},
		{0, 4, 0, 0, 0, 0, 0, 4, 0, 0, 4, 0, 0, 4},
		{0, 4, 0, 0, 0, 4, 0, 0, 0, 0, 4, 0, 0, 0},
		{0, 4, 4, 0, 0, 4, 0, 0, 0, 0, 4, 4, 0, 0}
#endif
	};
	memcpy(m_DTexDirTable, DTexDirTable, sizeof(int)*9*14);

	float TileDirU[8][4] = {
		{	0.0f, .125f,  0.0f, .125f},		//[up][LT, RT, LB, RB]
		{	0.0f,  0.0f, .125f, .125f},		//[right][ // ]
		{  .125f,  0.0f, .125f,  0.0f},		//[bottom][ // ]
		{  .125f, .125f,  0.0f,  0.0f},		//[left][ // ]

#ifdef _KNIGHT
		{  .125f,  0.0f, .125f,  0.0f},		//[up_mirr][ // ]
		{	0.0f,  0.0f, .125f, .125f},		//[right_mirr][ // ]
		{	0.0f, .125f,  0.0f, .125f},		//[bottom_mirr][ // ]
		{  .125f, .125f,  0.0f,  0.0f}		//[left_mirr][ // ]
#endif
#ifdef _EMPEROR
		{  .125f,  0.0f, .125f,  0.0f},		//[up_mirr][ // ]
		{  .125f, .125f,  0.0f,  0.0f},		//[right_mirr][ // ]
		{	0.0f, .125f,  0.0f, .125f},		//[bottom_mirr][ // ]
		{	0.0f,  0.0f, .125f, .125f}		//[left_mirr][ // ]
#endif
	};
	memcpy(m_fTileDirU, TileDirU, sizeof(float)*8*4);

	float TileDirV[8][4] = {
		{	 0.0f,	0.0f, .125f, .125f},		//[up][ // ]
		{	.125f,	0.0f, .125f,  0.0f},		//[right][ // ]
		{	.125f, .125f,  0.0f,  0.0f},		//[bottom][ // ]
		{	 0.0f, .125f,  0.0f, .125f},		//[left][ // ]

#ifdef _KNIGHT
		{	 0.0f,	0.0f, .125f, .125f},		//[up_mirr][ // ]
		{	 0.0f, .125f,  0.0f, .125f},		//[right_mirr][ // ]
		{	.125f, .125f,  0.0f,  0.0f},		//[bottom_mirr][ // ]
		{	.125f,  0.0f, .125f,  0.0f}		//[left_mirr][ // ]
#endif
#ifdef _EMPEROR
		{	 0.0f,	0.0f, .125f, .125f},		//[up_mirr][ // ]
		{	.125f,  0.0f, .125f,  0.0f},			//[right_mirr][ // ]
		{	.125f, .125f,  0.0f,  0.0f},		//[bottom_mirr][ // ]
		{	 0.0f, .125f,  0.0f, .125f}		//[left_mirr][ // ]
#endif
	};
	memcpy(m_fTileDirV, TileDirV, sizeof(float)*8*4);

	float TileDirUforColorMap[8][4] = {
		{ 1.0f,		128.0f, 1.0f,	128.0f},		//[up][LT, RT, LB, RB]
		{ 1.0f,		1.0f,	128.0f, 128.0f},		//[right][ // ]
		{ 128.0f,	1.0f,	128.0f, 1.0f},		//[bottom][ // ]
		{ 128.0f,	128.0f,	1.0f,	1.0f},		//[left][ // ]

#ifdef _KNIGHT
		{ 128.0f,	1.0f,	128.0f,	1.0f},		//[up_mirr][ // ]
		{ 1.0f,		1.0f,	128.0f, 128.0f},		//[right_mirr][ // ]
		{ 1.0f,		128.0f,	1.0f,	128.0f},		//[bottom_mirr][ // ]
		{ 128.0f,	128.0f,	1.0f,	1.0f}		//[left_mirr][ // ]
#endif
#ifdef _EMPEROR
		{ 128.0f,	1.0f,	128.0f,	1.0f},		//[up_mirr][ // ]
		{ 128.0f,	128.0f,	1.0f,	1.0f},		//[right_mirr][ // ]
		{ 1.0f,		128.0f,	1.0f,	128.0f},		//[bottom_mirr][ // ]
		{ 1.0f,		1.0f,	128.0f, 128.0f}		//[left_mirr][ // ]
#endif
	};
	memcpy(m_fTileDirUforColorMap, TileDirUforColorMap, sizeof(float)*8*4);

	float TileDirVforColorMap[8][4] = {
		{ 1.0f,		1.0f,	128.0f, 128.0f},		//[up][ // ]
		{ 128.0f,	1.0f,	128.0f, 1.0f},		//[right][ // ]
		{ 128.0f,	128.0f, 1.0f, 1.0f},		//[left][ // ]
		{ 1.0f,		128.0f,	1.0f, 128.0f},		//[bottom][ // ]

#ifdef _KNIGHT
		{ 1.0f,		1.0f,	128.0f, 128.0f},		//[up_mirr][ // ]
		{ 1.0f,		128.0f,	1.0f,	128.0f},		//[right_mirr][ // ]
		{ 128.0f,	128.0f, 1.0f,	1.0f},		//[left_mirr][ // ]
		{ 128.0f,	1.0f,	128.0f,	1.0f}		//[bottom_mirr][ // ]		
#endif
#ifdef _EMPEROR
		{ 1.0f,		1.0f,	128.0f, 128.0f},		//[up_mirr][ // ]
		{ 128.0f,	1.0f,	128.0f,	1.0f},		//[right_mirr][ // ]
		{ 128.0f,	128.0f, 1.0f,	1.0f},		//[left_mirr][ // ]
		{ 1.0f,		128.0f,	1.0f,	128.0f}		//[bottom_mirr][ // ]		
#endif
	};
	memcpy(m_fTileDirVforColorMap, TileDirVforColorMap, sizeof(float)*8*4);

	MakeDistanceTable();

	m_pDTexVB = nullptr;
	m_pDTexVertices = nullptr;

	m_fFlatHeight = 0.0f;
	m_bFlaten = false;

	m_bDrawLineLightMap = false;

	m_fHeightScale = 0.5f;

	Init();
}

CLyTerrain::~CLyTerrain()
{
	Release();
}


//
//
//
void CLyTerrain::SetFillMode(D3DFILLMODE fillmode)
{
	m_FillMode = fillmode;
}

//
//	Release()
//
void CLyTerrain::Release()
{
	if(m_pDlgSetLightMap)
	{ 
		m_pDlgSetLightMap->DestroyWindow();
		delete m_pDlgSetLightMap;
		m_pDlgSetLightMap = nullptr;
	}

	int x,z;
	if(m_pColorTexture)
	{
		for(x=0;x<m_iNumColorMap;x++)
		{
			if(m_pColorTexture[x])
			{
//				for(z=0;z<m_iNumColorMap;z++) // 굳이 이렇게 안해도 다 지워진다...
//				{
//					m_pColorTexture[x][z].Release();
//				}
				delete[] m_pColorTexture[x];
				m_pColorTexture[x] = nullptr;
			}
		}
		delete[] m_pColorTexture;
		m_pColorTexture = nullptr;
	}

	int i;
	if(m_ppLightMapTexture)
	{
		for(x=0;x<m_iHeightMapSize;x++)
		{
			for(z=0;z<m_iHeightMapSize;z++)
			{
				if(m_ppLightMapTexture[x][z])
				{
					m_ppLightMapTexture[x][z]->Release();
					delete m_ppLightMapTexture[x][z];
					m_ppLightMapTexture[x][z] = nullptr;
				}
			}
			delete[] m_ppLightMapTexture[x];
			m_ppLightMapTexture[x] = nullptr;
		}
		delete[] m_ppLightMapTexture;
		m_ppLightMapTexture = nullptr;
	}

	delete m_pRoot;
	m_pRoot = nullptr;
	m_pRefCurrNode = nullptr;

	if( m_ColorMapVB )
	{
		m_ColorMapVB->Release();
		m_ColorMapVB = nullptr;
	}
	if(m_TileVB)
	{
		m_TileVB->Release();
		m_TileVB = nullptr;
	}
	if(m_LightMapVB)
	{
		m_LightMapVB->Release();
		m_LightMapVB = nullptr;
	}
	if(m_pColorMapTmpVB)
	{
		m_pColorMapTmpVB->Release();
		m_pColorMapTmpVB = nullptr;
	}
	
	if(m_ppMapData)
	{
		for(i=0;i<m_iHeightMapSize;i++)
		{
			delete[] m_ppMapData[i];
			m_ppMapData[i] = nullptr;
		}
		delete[] m_ppMapData;
		m_ppMapData = nullptr;
	}

	if(m_ppRenderInfo)
	{
		for(i=0;i<m_iHeightMapSize;i++)
		{
			delete [] m_ppRenderInfo[i];
			m_ppRenderInfo[i] = nullptr;
		}
		delete[] m_ppRenderInfo;
		m_ppRenderInfo = nullptr;
	}

	if(m_ppIsLightMap)
	{
		for(i=0;i<m_iHeightMapSize;i++)
		{
			delete [] m_ppIsLightMap[i];
			m_ppIsLightMap[i] = nullptr;
		}
		delete[] m_ppIsLightMap;
		m_ppIsLightMap = nullptr;
	}
	
	m_iBrushIndexCount = 0;
	m_iEditMode = TEM_NOT;
	
	m_RenderNodes.clear();

	m_pDTexVB = nullptr;
	m_pDTexVertices = nullptr;
}
 

//
//	Init()
//
void CLyTerrain::Init(int HeightMapSize)
{
	Release();

	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	m_pDTexMng = pFrm->GetDTexMng();

	if(m_pDlgSetLightMap)
	{ 
		m_pDlgSetLightMap->DestroyWindow();
		delete m_pDlgSetLightMap;
		m_pDlgSetLightMap = nullptr;
	}
	m_pDlgSetLightMap = new CDlgSetLightMap;
	m_pDlgSetLightMap->Create(IDD_SET_LIGHTMAP);

	m_Material.Init();

	m_iHeightMapSize = HeightMapSize;
	m_iMaxLevel = (Log2(m_iHeightMapSize-1)) - 1;

	m_iHeightLimit = 15000;
	UpDateDistLimit();

	m_iNumTileMap = 0;
	m_iNumLightMap = 0;

	m_iColorMapTexSize = 128;	//컬러맵 텍스쳐의 크기..
	m_iColorMapPixelPerUnitDistance = 4;		//UnitDistance당 들어가는 컬러맵의 픽셀 수..

	m_pColorMapTmpVertices = nullptr;
	s_lpD3DDev->CreateVertexBuffer( 8*sizeof(__VertexTransformedT2), 0, FVF_TRANSFORMEDT2, D3DPOOL_MANAGED, &m_pColorMapTmpVB, nullptr );
	
	//컬러맵 텍스쳐 만들기..
	m_iNumColorMap =  (((m_iHeightMapSize-1) * m_iColorMapPixelPerUnitDistance) / m_iColorMapTexSize) + 1;
	if( ((m_iHeightMapSize-1) * m_iColorMapPixelPerUnitDistance) % m_iColorMapTexSize == 0 ) m_iNumColorMap--;
	
	m_pColorTexture = new CN3Texture*[m_iNumColorMap];
	int x,z;
	for(x=0; x<m_iNumColorMap; x++)
	{
		m_pColorTexture[x] = new CN3Texture[m_iNumColorMap];
	}
	
	//D3DLOCKED_RECT d3dlr;
	
	for(x=0;x<m_iNumColorMap;x++)
	{
		for(z=0;z<m_iNumColorMap;z++)
		{
			m_pColorTexture[x][z].Create(m_iColorMapTexSize, m_iColorMapTexSize, D3DFMT_X8R8G8B8, TRUE);
			/*
			m_pColorTexture[x][z].Get()->LockRect( 0, &d3dlr, 0, 0 );
			DWORD* pTexPtr = (DWORD*)d3dlr.pBits;
			for(int tz=0;tz<m_iColorMapTexSize;tz++)
			{
				for(int tx=0;tx<m_iColorMapTexSize;tx++)
					pTexPtr[tx + tz*m_iColorMapTexSize] = 0xff004400;
			}
			m_pColorTexture[x][z].Get()->UnlockRect(0);
			*/
		}
	}

	m_ppMapData = new LPMAPDATA[m_iHeightMapSize];
	m_ppRenderInfo = new LPBOOL[m_iHeightMapSize];
	m_ppIsLightMap = new bool*[m_iHeightMapSize];
	m_ppLightMapTexture = new CN3Texture**[m_iHeightMapSize];
	
	int i;		
	for(i=0;i<m_iHeightMapSize;i++)
	{
		m_ppMapData[i] = new MAPDATA[m_iHeightMapSize];
		m_ppRenderInfo[i] = new BOOL[m_iHeightMapSize];
		m_ppIsLightMap[i] = new bool[m_iHeightMapSize];
		m_ppLightMapTexture[i] = new CN3Texture*[m_iHeightMapSize];
	}

	for(x=0;x<m_iHeightMapSize;x++)
	{
		for(z=0;z<m_iHeightMapSize;z++)
		{
			m_ppIsLightMap[x][z] = false;
			m_ppLightMapTexture[x][z] = nullptr;
		}
	}

	m_ColorMapTileTree.clear();

	s_lpD3DDev->CreateVertexBuffer( MAX_COLORMAPVB_SIZE*sizeof(__VertexT1), 0, FVF_VNT1, D3DPOOL_MANAGED, &m_ColorMapVB, nullptr );
	s_lpD3DDev->CreateVertexBuffer( MAX_TILEVB_SIZE*sizeof(__VertexT2), 0, FVF_VNT2, D3DPOOL_MANAGED, &m_TileVB, nullptr );
	s_lpD3DDev->CreateVertexBuffer( MAX_LIGHTMAPVB_SIZE*sizeof(__VertexT1), 0, FVF_VNT1, D3DPOOL_MANAGED, &m_LightMapVB, nullptr );

	m_pRoot = new CQTNode;
	m_pRoot->Init(0, this);
	m_pRoot->SetCenterPoint((1<<m_iMaxLevel), (1<<m_iMaxLevel));
	m_pRoot->LinkFriend();
		
	m_pRefCurrNode = nullptr;
	

	SetRectEmpty(&m_VisibleRect);	
}


//
//
//
void CLyTerrain::UpDateDistLimit()
{
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	if(pFrm)
	{
		m_iDistLimit = pFrm->GetTileRegion();
		if(m_ppRenderInfo)
		{
			SetVisibleRect();
			ClearRenderInfo();
			m_RenderNodes.clear();
			m_pRoot->Tick();
		}
		pFrm->Invalidate(FALSE);
	}
	else 
	{
		m_iDistLimit = 30;
	}	
}


//
//
//
CN3Texture* CLyTerrain::GetTileTex(int id)
{
	CDTex* pDTex = m_pDTexMng->GetDTexByID(id);
	if(pDTex) return pDTex->m_pTex;

	return nullptr;
}


//
//
//
bool CLyTerrain::SaveToFilePartition(const char* lpszPath, float psx, float psz, float width)
{
	HANDLE hFile = CreateFile(lpszPath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		MessageBox(::GetActiveWindow(), lpszPath,"Fail to save trn file!", MB_OK);
		return false;
	}
	
	int x,z;
	DWORD dwRWC;
	int version = -N3ME_DATA_VERSION;
	int HeightMapSize = (width / TERRAIN_CELL_SIZE) + 1;

	int sx = psx / TERRAIN_CELL_SIZE;
	int sz = psz / TERRAIN_CELL_SIZE;

	int ex = sx + HeightMapSize;
	int ez = sz + HeightMapSize;

	WriteFile(hFile, &(version), sizeof(int), &dwRWC, nullptr);

	WriteFile(hFile, &(HeightMapSize), sizeof(int), &dwRWC, nullptr);
		
	CProgressBar ProgressBar; // 진행 상황..

	if(HeightMapSize > 0)
	{
		//  그냥 하나씩 저장해도 차이 없지만 네트워크로 저장할때는 파일 엑세스 숫자를 줄이고 한꺼번에 저장해야  빠르다.. ...
		DWORD dwAlloc = HeightMapSize * HeightMapSize * sizeof(float);
		HGLOBAL hAlloc = ::GlobalAlloc(GMEM_FIXED, dwAlloc);
		__ASSERT(hAlloc, "Global allocation failed.");
		float* pfHeights = (float*)::GlobalLock(hAlloc);

		ProgressBar.Create("Save terrain data..", 50, m_iHeightMapSize);
		for(z=sz;z<ez;z++)
		{
			ProgressBar.StepIt();
			for(x=sx;x<ex;x++)
			{
				int idx = (z-sz)*HeightMapSize + (x-sx);
				pfHeights[idx] = m_ppMapData[x][z].fHeight;				
			}
		}
		WriteFile(hFile, pfHeights, dwAlloc, &dwRWC, nullptr); // 파일에 쓴다..
		pfHeights = nullptr;
		::GlobalUnlock(hAlloc);
		::GlobalFree(hAlloc);
		
		// Detail Texture 저장..
		dwAlloc = HeightMapSize * HeightMapSize * sizeof(DTEXINFO)*2;
		hAlloc = ::GlobalAlloc(GMEM_FIXED, dwAlloc);
		__ASSERT(hAlloc, "Global allocation failed.");
		DTEXINFO* pDTexInfos = (DTEXINFO*)::GlobalLock(hAlloc);

		ProgressBar.Create("Save tile map data..", 50, m_iHeightMapSize);
		for(z=sz;z<ez;z++)
		{
			ProgressBar.StepIt();
			for(x=sx;x<ex;x++)
			{
				////////////////////////////////////////////////////////////////////
				// 쓰레기 정보 차단...
				if(m_ppMapData[x][z].DTexInfo1.TexIdx.TileX<0 
					|| m_ppMapData[x][z].DTexInfo1.TexIdx.TileX>=NUM_DTEXTILE)
				{
					m_ppMapData[x][z].DTexInfo1.TexIdx.TileX = 0;
				}
				if(m_ppMapData[x][z].DTexInfo2.TexIdx.TileX<0 
					|| m_ppMapData[x][z].DTexInfo2.TexIdx.TileX>=NUM_DTEXTILE)
				{
					m_ppMapData[x][z].DTexInfo2.TexIdx.TileX = 0;
				}
				if(m_ppMapData[x][z].DTexInfo1.TexIdx.TileY<0 
					|| m_ppMapData[x][z].DTexInfo1.TexIdx.TileY>=NUM_DTEXTILE)
				{
					m_ppMapData[x][z].DTexInfo1.TexIdx.TileY = 0;
				}
				if(m_ppMapData[x][z].DTexInfo2.TexIdx.TileY<0 
					|| m_ppMapData[x][z].DTexInfo2.TexIdx.TileY>=NUM_DTEXTILE)
				{
					m_ppMapData[x][z].DTexInfo2.TexIdx.TileY = 0;
				}
				// 쓰레기 정보 차단...
				////////////////////////////////////////////////////////////////////
				
				int idx = (z-sz)*HeightMapSize + (x-sx);
				pDTexInfos[idx*2] = m_ppMapData[x][z].DTexInfo1;
				pDTexInfos[idx*2+1] = m_ppMapData[x][z].DTexInfo2;
			}
		}
		WriteFile(hFile, pDTexInfos, dwAlloc, &dwRWC, nullptr); // 파일에 쓴다..
		pDTexInfos = nullptr;
		::GlobalUnlock(hAlloc);
		::GlobalFree(hAlloc);
	}

	//라이트맵 정보 기록...
	//N3ME_DATA_VERSION 1이상부터..
	int NumLightMap = DetectRealLightMap(sx, sz, HeightMapSize);
	WriteFile(hFile, &(NumLightMap), sizeof(int), &dwRWC, nullptr); // LightMap의 갯수 기록..

	if(NumLightMap>0)
	{
		ProgressBar.Create("Save Light Map Data", 50, NumLightMap);	
		for(z=sz;z<ez;z++)
		{
			for(x=sx;x<ex;x++)
			{
				if(m_ppIsLightMap[x][z]==false) continue;

				short ssx, ssz;
				ssx = (short)(x - sx);
				ssz = (short)(z - sz);
				WriteFile(hFile, &(ssx), sizeof(short), &dwRWC, nullptr);
				WriteFile(hFile, &(ssz), sizeof(short), &dwRWC, nullptr);

				m_ppLightMapTexture[x][z]->Convert(m_ppLightMapTexture[x][z]->PixelFormat(), LIGHTMAP_TEX_SIZE, LIGHTMAP_TEX_SIZE);
				m_ppLightMapTexture[x][z]->Save(hFile);

				ProgressBar.StepIt();
				NumLightMap--;
				if(NumLightMap==0) break;
			}
			if(NumLightMap==0) break;
		}
	}

	//풀 관련 정보 저장..
	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();

	int NumSeedInfo = static_cast<int>(pFrm->m_SeedGroupList.size());
	WriteFile(hFile, &NumSeedInfo, sizeof(int), &dwRWC, nullptr);

	ProgressBar.Create("Save Grass Info", 50, NumSeedInfo);

	std::list<LPSEEDGROUP>::iterator sgit;
	sgit = pFrm->m_SeedGroupList.begin();
	for(int i=0;i<NumSeedInfo;i++)
	{
		ProgressBar.StepIt();
		LPSEEDGROUP pSeedGroup = (*sgit);
		WriteFile(hFile, pSeedGroup, sizeof(SEEDGROUP), &dwRWC, nullptr);
		sgit++;
	}

	CloseHandle(hFile);

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// 컬러맵 쓰기.
	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
	_splitpath(lpszPath, szDrive, szDir, szFName, szExt);
	char szNewFName[_MAX_PATH] = "";
	_makepath(szNewFName, szDrive, szDir, szFName, "tcm"); // 파일 이름과 동일한 이름으로 컬러맵 저장..

	HANDLE hCMFile = CreateFile(szNewFName, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	int NumColorMap = (((HeightMapSize-1) * m_iColorMapPixelPerUnitDistance) / m_iColorMapTexSize) + 1;
	if( ((HeightMapSize-1) * m_iColorMapPixelPerUnitDistance) % m_iColorMapTexSize == 0 ) NumColorMap--;

	//////////////////////////////////////////////////////////
	//일단 국지적인 컬러맵을 생성한 다음에....그걸 써야해...
	//
	ProgressBar.Create("Create ColorMap", 50, NumColorMap*NumColorMap + NumColorMap);

	CN3Texture** pColorTexture;
	pColorTexture = new CN3Texture*[NumColorMap];
	for(x=0; x<NumColorMap; x++)
	{
		ProgressBar.StepIt();
		pColorTexture[x] = new CN3Texture[NumColorMap];
	}
	
	//
	// fill color map..
	// bmp컬러맵 만들고...
	// 다시 읽어온 다음에...
	// 다시 잘라서 저장한 다음에...
	// 잘라 저장한 bmp를 Import하는 것처럼 읽어서 셋팅..
	CString strTmpColorMap("c:\\MiniMap.bmp");
	ColorMapExport((LPCTSTR)strTmpColorMap);

	CBitMapFile BMF;
	if(BMF.LoadFromFile((LPCTSTR)strTmpColorMap))
	{
		RECT rc;
		rc.left = sx * m_iColorMapPixelPerUnitDistance;
		rc.right = rc.left + (m_iColorMapTexSize * NumColorMap);
		rc.bottom = (m_iHeightMapSize - 1 - sz)  * m_iColorMapPixelPerUnitDistance;
		rc.top = rc.bottom - (m_iColorMapTexSize * NumColorMap);

		BMF.SaveRectToFile((LPCTSTR)strTmpColorMap, rc);

		if(BMF.LoadFromFile((LPCTSTR)strTmpColorMap))
		{
			for(x=0;x<NumColorMap;x++)
			{
				for(z=0;z<NumColorMap;z++)
				{
					ProgressBar.StepIt();
					rc.left = x * m_iColorMapTexSize;
					rc.top = (NumColorMap - z - 1)*m_iColorMapTexSize;

					rc.right = rc.left + m_iColorMapTexSize;
					rc.bottom = rc.top + m_iColorMapTexSize;
					BMF.SaveRectToFile((LPCTSTR)strTmpColorMap, rc);

					pColorTexture[x][z].LoadFromFile((LPCTSTR)strTmpColorMap);
					pColorTexture[x][z].Convert(D3DFMT_X8R8G8B8, m_iColorMapTexSize, m_iColorMapTexSize);
					DeleteFile((LPCTSTR)strTmpColorMap);
				}
			}
		}
	}	
	//
	//////////////////////////////////////////////////////////
	
	ProgressBar.Create("Save color map..", 50, NumColorMap * NumColorMap);
	for(x=0;x<NumColorMap;x++)
	{
		for(z=0;z<NumColorMap;z++)
		{
			ProgressBar.StepIt();
			pColorTexture[x][z].Save(hCMFile);
		}
	}
	CloseHandle(hCMFile);
//*/	
	return true;
}


//
//	Save()
//	맵에디터에서 쓰는 파일 타입으로 저장하기..
//
bool CLyTerrain::SaveToFile(const char* lpszPath)
{
	HANDLE hFile = CreateFile(lpszPath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		MessageBox(::GetActiveWindow(), lpszPath,"Fail to save trn file!", MB_OK);
		return false;
	}
	
	int x,z;
	DWORD dwRWC;
	int version = -N3ME_DATA_VERSION;
	WriteFile(hFile, &(version), sizeof(int), &dwRWC, nullptr);

	WriteFile(hFile, &(m_iHeightMapSize), sizeof(int), &dwRWC, nullptr);
		
	CProgressBar ProgressBar; // 진행 상황..

	if(m_iHeightMapSize > 0)
	{
		//  그냥 하나씩 저장해도 차이 없지만 네트워크로 저장할때는 파일 엑세스 숫자를 줄이고 한꺼번에 저장해야  빠르다.. ...
		DWORD dwAlloc = m_iHeightMapSize * m_iHeightMapSize * sizeof(float);
		HGLOBAL hAlloc = ::GlobalAlloc(GMEM_FIXED, dwAlloc);
		__ASSERT(hAlloc, "Global allocation failed.");
		float* pfHeights = (float*)::GlobalLock(hAlloc);

		ProgressBar.Create("Save terrain data..", 50, m_iHeightMapSize);
		for(z=0;z<m_iHeightMapSize;z++)
		{
			ProgressBar.StepIt();
			for(x=0;x<m_iHeightMapSize;x++)
			{
				pfHeights[z*m_iHeightMapSize+x] = m_ppMapData[x][z].fHeight;				
			}
		}
		WriteFile(hFile, pfHeights, dwAlloc, &dwRWC, nullptr); // 파일에 쓴다..
		pfHeights = nullptr;
		::GlobalUnlock(hAlloc);
		::GlobalFree(hAlloc);
		
		// Detail Texture 저장..
		dwAlloc = m_iHeightMapSize * m_iHeightMapSize * sizeof(DTEXINFO)*2;
		hAlloc = ::GlobalAlloc(GMEM_FIXED, dwAlloc);
		__ASSERT(hAlloc, "Global allocation failed.");
		DTEXINFO* pDTexInfos = (DTEXINFO*)::GlobalLock(hAlloc);

		ProgressBar.Create("Save tile map data..", 50, m_iHeightMapSize);
		for(z=0;z<m_iHeightMapSize;z++)
		{
			ProgressBar.StepIt();
			for(x=0;x<m_iHeightMapSize;x++)
			{
				////////////////////////////////////////////////////////////////////
				// 쓰레기 정보 차단...
				if(m_ppMapData[x][z].DTexInfo1.TexIdx.TileX<0 
					|| m_ppMapData[x][z].DTexInfo1.TexIdx.TileX>=NUM_DTEXTILE)
				{
					m_ppMapData[x][z].DTexInfo1.TexIdx.TileX = 0;
				}
				if(m_ppMapData[x][z].DTexInfo2.TexIdx.TileX<0 
					|| m_ppMapData[x][z].DTexInfo2.TexIdx.TileX>=NUM_DTEXTILE)
				{
					m_ppMapData[x][z].DTexInfo2.TexIdx.TileX = 0;
				}
				if(m_ppMapData[x][z].DTexInfo1.TexIdx.TileY<0 
					|| m_ppMapData[x][z].DTexInfo1.TexIdx.TileY>=NUM_DTEXTILE)
				{
					m_ppMapData[x][z].DTexInfo1.TexIdx.TileY = 0;
				}
				if(m_ppMapData[x][z].DTexInfo2.TexIdx.TileY<0 
					|| m_ppMapData[x][z].DTexInfo2.TexIdx.TileY>=NUM_DTEXTILE)
				{
					m_ppMapData[x][z].DTexInfo2.TexIdx.TileY = 0;
				}
				// 쓰레기 정보 차단...
				////////////////////////////////////////////////////////////////////
				
				pDTexInfos[(z*m_iHeightMapSize+x)*2] = m_ppMapData[x][z].DTexInfo1;
				pDTexInfos[(z*m_iHeightMapSize+x)*2+1] = m_ppMapData[x][z].DTexInfo2;
			}
		}
		WriteFile(hFile, pDTexInfos, dwAlloc, &dwRWC, nullptr); // 파일에 쓴다..
		pDTexInfos = nullptr;
		::GlobalUnlock(hAlloc);
		::GlobalFree(hAlloc);
	}

	//라이트맵 정보 기록...
	//N3ME_DATA_VERSION 1이상부터..
	DetectRealLightMap(0, 0, m_iHeightMapSize);
	WriteFile(hFile, &(m_iNumLightMap), sizeof(int), &dwRWC, nullptr); // LightMap의 갯수 기록..

	int CountLightMap = m_iNumLightMap;
	if(CountLightMap>0)
	{
		ProgressBar.Create("Save Light Map Data", 50, m_iNumLightMap);	
		
		short sx, sz;
		for(z=0;z<m_iHeightMapSize;z++)
		{
			for(x=0;x<m_iHeightMapSize;x++)
			{
				if(m_ppIsLightMap[x][z]==false) continue;

				sx = (short)x;	sz = (short)z;
				WriteFile(hFile, &(sx), sizeof(short), &dwRWC, nullptr);
				WriteFile(hFile, &(sz), sizeof(short), &dwRWC, nullptr);

				m_ppLightMapTexture[x][z]->Convert(m_ppLightMapTexture[x][z]->PixelFormat(), LIGHTMAP_TEX_SIZE, LIGHTMAP_TEX_SIZE);
				m_ppLightMapTexture[x][z]->Save(hFile);

				ProgressBar.StepIt();
				CountLightMap--;
				if(CountLightMap==0) break;
			}
			if(CountLightMap==0) break;
		}
	}

	//풀 관련 정보 저장..
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	
	int NumSeedInfo = static_cast<int>(pFrm->m_SeedGroupList.size());
	WriteFile(hFile, &NumSeedInfo, sizeof(int), &dwRWC, nullptr);

	ProgressBar.Create("Save Grass Info", 50, NumSeedInfo);

	std::list<LPSEEDGROUP>::iterator sgit;
	sgit = pFrm->m_SeedGroupList.begin();
	for(int i=0;i<NumSeedInfo;i++)
	{
		ProgressBar.StepIt();
		LPSEEDGROUP pSeedGroup = (*sgit);
		WriteFile(hFile, pSeedGroup, sizeof(SEEDGROUP), &dwRWC, nullptr);
		sgit++;
	}

	CloseHandle(hFile);

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// 컬러맵 쓰기.
	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
	_splitpath(lpszPath, szDrive, szDir, szFName, szExt);
	char szNewFName[_MAX_PATH] = "";
	_makepath(szNewFName, szDrive, szDir, szFName, "tcm"); // 파일 이름과 동일한 이름으로 컬러맵 저장..

	HANDLE hCMFile = CreateFile(szNewFName, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	ProgressBar.Create("Save color map..", 50, m_iNumColorMap * m_iNumColorMap);
	for(x=0;x<m_iNumColorMap;x++)
	{
		for(z=0;z<m_iNumColorMap;z++)
		{
			ProgressBar.StepIt();
			m_pColorTexture[x][z].Save(hCMFile);
		}
	}
	CloseHandle(hCMFile);

	/*	old version....
	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
	_splitpath(lpszPath, szDrive, szDir, szFName, szExt);
	char szNewFName[_MAX_PATH] = "", szAdd[_MAX_PATH] = "";

	ProgressBar.Create("Save color map..", 50, m_iNumColorMap * m_iNumColorMap);
	for(x=0;x<m_iNumColorMap;x++)
	{
		for(z=0;z<m_iNumColorMap;z++)
		{
			ProgressBar.StepIt();

			_makepath(szNewFName, szDrive, szDir, szFName, nullptr); // 파일 이름과 동일한 이름으로 컬러맵 저장..
			wsprintf(szAdd, "_%02d%02d.DXT", x, z);  // Tool 경로를 붙이고 번호와 확장자를 붙여서 저장..
			lstrcat(szNewFName, szAdd);
			m_pColorTexture[x][z].SaveToFile(szNewFName);
		}
	}
	*/
	// 컬러맵 쓰기.
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	
	return true;
}

//
//	Load..
//
bool CLyTerrain::LoadFromFile(const char* lpszPath)
{
	HANDLE hFile = CreateFile(lpszPath, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if(INVALID_HANDLE_VALUE == hFile) return false;

	DWORD dwRWC;
	int x, z;
	int HeightMapSize;
	int version = 0;
	ReadFile(hFile, &(version), sizeof(int), &dwRWC, nullptr);
	if(version<0)	//버전이 기록된 데이터들....
	{
		ReadFile(hFile, &(HeightMapSize), sizeof(int), &dwRWC, nullptr);
	}
	else HeightMapSize = version;	//버전이 기록되어 있지 않은 이전 데이터들..
	
	Init(HeightMapSize);

	CProgressBar ProgressBar; // 진행 상황..

	if(m_iHeightMapSize > 0)
	{
		//  그냥 하나씩 저장해도 차이 없지만 네트워크로 저장할때는 파일 엑세스 숫자를 줄이고 한꺼번에 저장해야  빠르다.. ...
		DWORD dwAlloc = m_iHeightMapSize * m_iHeightMapSize * sizeof(float);
		HGLOBAL hAlloc = ::GlobalAlloc(GMEM_FIXED, dwAlloc);
		__ASSERT(hAlloc, "Global allocation failed.");
		float* fHeights = (float*)GlobalLock(hAlloc);
		ReadFile(hFile, fHeights, dwAlloc, &dwRWC, nullptr);
		
		ProgressBar.Create("Load terrain data..", 50, m_iHeightMapSize);
		for(z=0;z<m_iHeightMapSize;z++)
		{
			ProgressBar.StepIt();
			for(x=0;x<m_iHeightMapSize;x++)
			{
				m_ppMapData[x][z].fHeight = fHeights[z*m_iHeightMapSize+x];				
			}
		}
		GlobalUnlock(hAlloc);
		GlobalFree(hAlloc); hAlloc = nullptr;
		fHeights = nullptr;

		//  그냥 하나씩 저장해도 차이 없지만 네트워크로 저장할때는 파일 엑세스 숫자를 줄이고 한꺼번에 저장해야  빠르다.. ...
		dwAlloc = m_iHeightMapSize * m_iHeightMapSize * sizeof(DTEXINFO) * 2;
		hAlloc = ::GlobalAlloc(GMEM_FIXED, dwAlloc);
		__ASSERT(hAlloc, "Global allocation failed.");
		DTEXINFO* pDTIs = (DTEXINFO*)GlobalLock(hAlloc);
		ReadFile(hFile, pDTIs, dwAlloc, &dwRWC, nullptr);

		ProgressBar.Create("Load tile map data..", 50, m_iHeightMapSize);
		for(z=0;z<m_iHeightMapSize;z++)
		{
			ProgressBar.StepIt();
			for(x=0;x<m_iHeightMapSize;x++)
			{
				memcpy(&(m_ppMapData[x][z].DTexInfo1), &(pDTIs[(z*m_iHeightMapSize+x)*2]), sizeof(DTEXINFO));
				memcpy(&(m_ppMapData[x][z].DTexInfo2), &(pDTIs[(z*m_iHeightMapSize+x)*2+1]), sizeof(DTEXINFO));
				VerifyTexInfo(&(m_ppMapData[x][z].DTexInfo1));
				VerifyTexInfo(&(m_ppMapData[x][z].DTexInfo2));
			}
		}
		GlobalUnlock(hAlloc);
		GlobalFree(hAlloc); hAlloc = nullptr;
		pDTIs = nullptr;
	}

	if(version <= -1)	//버전 1부터....^^
	{
		//라이트맵 정보 읽기..
		ReadFile(hFile, &(m_iNumLightMap), sizeof(int), &dwRWC, nullptr); // LightMap의 갯수 기록..
		ProgressBar.Create("Load Light Map Data", 50, m_iNumLightMap);
		
		if(version>-3)
		{
			short sx, sz;
			for(int i=0;i<m_iNumLightMap;i++)
			{
				ProgressBar.StepIt();
				ReadFile(hFile, &(sx), sizeof(short), &dwRWC, nullptr);
				ReadFile(hFile, &(sz), sizeof(short), &dwRWC, nullptr);

				if(!m_ppLightMapTexture[sx][sz]) m_ppLightMapTexture[sx][sz] = new CN3Texture;

				m_ppIsLightMap[sx][sz] = true;
				m_ppLightMapTexture[sx][sz]->Load(hFile);
			}
			ConvertLightMapToolDataV2toV3();
		}
		else
		{
			short sx, sz;
			for(int i=0;i<m_iNumLightMap;i++)
			{
				ProgressBar.StepIt();
				ReadFile(hFile, &(sx), sizeof(short), &dwRWC, nullptr);
				ReadFile(hFile, &(sz), sizeof(short), &dwRWC, nullptr);

				if(!m_ppLightMapTexture[sx][sz]) m_ppLightMapTexture[sx][sz] = new CN3Texture;

				m_ppIsLightMap[sx][sz] = true;
				m_ppLightMapTexture[sx][sz]->Load(hFile);
				if(m_ppLightMapTexture[sx][sz]->PixelFormat() != D3DFMT_A8R8G8B8)
					m_ppLightMapTexture[sx][sz]->Convert(D3DFMT_A8R8G8B8, LIGHTMAP_TEX_SIZE, LIGHTMAP_TEX_SIZE);				
			}
		}
	}
	if(version <= -2)	//버전 2부터....^^
	{
		// 풀씨에 관한 정보 읽기..
		int NumSeedInfo;
		ReadFile(hFile, &(NumSeedInfo), sizeof(int), &dwRWC, nullptr);

		CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
		pFrm->m_SeedGroupList.clear();
				
		ProgressBar.Create("Load Grass Info", 50, NumSeedInfo);
		
		for(int i=0;i<NumSeedInfo;i++)
		{
			ProgressBar.StepIt();
			LPSEEDGROUP pSeedGroup = new SEEDGROUP;
			ReadFile(hFile, pSeedGroup, sizeof(SEEDGROUP), &dwRWC, nullptr);

			pFrm->m_SeedGroupList.push_back(pSeedGroup);
		}
	}

	CloseHandle(hFile);

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// 컬러맵 읽기.
	ProgressBar.Create("Load color map..", 50, m_iNumColorMap * m_iNumColorMap);

	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
	_splitpath(lpszPath, szDrive, szDir, szFName, szExt);
	char szNewFName[_MAX_PATH] = "", szAdd[_MAX_PATH] = "";

	_makepath(szNewFName, szDrive, szDir, szFName, "tcm"); // 파일 이름과 동일한 이름으로 컬러맵 저장되어 있다.
	HANDLE hCMFile = CreateFile(szNewFName, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if(INVALID_HANDLE_VALUE == hCMFile)
	{
		for(x=0;x<m_iNumColorMap;x++)
		{
			for(z=0;z<m_iNumColorMap;z++)
			{
				ProgressBar.StepIt();

				_makepath(szNewFName, szDrive, szDir, szFName, nullptr); // 파일 이름과 동일한 이름으로 컬러맵 저장되어 있다.
				wsprintf(szAdd, "_%02d%02d.DXT", x, z);  // Tool 경로를 붙이고 번호와 확장자를 붙여서 저장되어 있다.
				lstrcat(szNewFName, szAdd);

				if(m_pColorTexture[x][z].LoadFromFile(szNewFName)==false)
				{
					MessageBox(::GetActiveWindow(), "컬러맵은 32bit dxt파일만 사용할 수 있어요..", "ㅠ.ㅠ", MB_OK);
				}
			}
		}
	}
	
	else
	{
		for(x=0;x<m_iNumColorMap;x++)
		{
			for(z=0;z<m_iNumColorMap;z++)
			{
				ProgressBar.StepIt();
				m_pColorTexture[x][z].Load(hCMFile);
			}
		}
		CloseHandle(hCMFile);
	}
	
	// 컬러맵 읽기.
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	//라이트맵텍스쳐 수도 있고, 텍스쳐도 세팅..
	m_pRoot->SetWholeClipped();
	m_pRoot->ClearMinMaxY();
	m_pRoot->SetMinMaxY();
	
	return true;
}


//
//
//
void CLyTerrain::ConvertLightMapToolDataV2toV3()
{
	DetectRealLightMap(0, 0, m_iHeightMapSize);

	if(m_iNumLightMap <= 0) return;

	int z,x;
	int px, pz;

	D3DLOCKED_RECT d3dlrSource;
	DWORD* pSrcBits;

	CBitMapFile* pBMP = new CBitMapFile;		
	unsigned char* pBMPImg;
	for(z=0;z<m_iHeightMapSize;z++)
	{
		for(x=0;x<m_iHeightMapSize;x++)
		{
			if(m_ppIsLightMap[x][z]==false) continue;
			if(!m_ppLightMapTexture[x][z])
			{
				m_ppIsLightMap[x][z] = false;
				continue;
			}

			m_ppLightMapTexture[x][z]->Convert(D3DFMT_A8R8G8B8, LIGHTMAP_TEX_SIZE, LIGHTMAP_TEX_SIZE);

			m_ppLightMapTexture[x][z]->Get()->LockRect(0, &d3dlrSource, 0, 0);
			pSrcBits = (DWORD*)d3dlrSource.pBits;

			//cn3texture -> cbitmapfile..
			pBMP->Create(LIGHTMAP_TEX_SIZE, LIGHTMAP_TEX_SIZE);
			
			for(px=0;px<LIGHTMAP_TEX_SIZE;px++)
			{
				for(pz=0;pz<LIGHTMAP_TEX_SIZE;pz++)
				{
					DWORD SrcColor = pSrcBits[px+(pz*LIGHTMAP_TEX_SIZE)];
					pBMPImg = (unsigned char*)pBMP->Pixels(px, pz);
					pBMPImg[0] = (unsigned char)((SrcColor<<24)>>24);
					pBMPImg[1] = (unsigned char)((SrcColor<<16)>>24);
					pBMPImg[2] = (unsigned char)((SrcColor<<8)>>24);
				}
			}
			
			//dc만들어서 비트맵축소..
			pBMP->SaveToFile("c:\\templightmap.bmp");

			int SmallSize = LIGHTMAP_TEX_SIZE - 2;

			HANDLE hSrcBitmap = LoadImage(0, "c:\\templightmap.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE|LR_DEFAULTSIZE);
			__ASSERT(hSrcBitmap, "");

			HDC hSmallDC = CreateCompatibleDC(nullptr);
			HDC hBMDC = CreateCompatibleDC(hSmallDC);

			HBITMAP hOldBM = (HBITMAP)SelectObject(hBMDC, (HBITMAP)hSrcBitmap);

			// Prepare to create a bitmap
			DWORD* pBitmapBits;
			BITMAPINFO bmi;
			ZeroMemory( &bmi.bmiHeader,  sizeof(BITMAPINFOHEADER) );
			bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth       =  (int)SmallSize;
			bmi.bmiHeader.biHeight      = -(int)SmallSize;
			bmi.bmiHeader.biPlanes      = 1;
			bmi.bmiHeader.biCompression = BI_RGB;
			bmi.bmiHeader.biBitCount    = 32;

			HBITMAP hbmBitmap = CreateDIBSection( hSmallDC, &bmi, DIB_RGB_COLORS, (VOID**)&pBitmapBits, nullptr, 0 );
			HBITMAP hOldBM2 = (HBITMAP) SelectObject( hSmallDC, hbmBitmap );

			SetStretchBltMode(hSmallDC, HALFTONE);
			StretchBlt(hSmallDC, 0,0, SmallSize, SmallSize, hBMDC, 0,0, LIGHTMAP_TEX_SIZE, LIGHTMAP_TEX_SIZE, SRCCOPY);

			for(px=0;px<SmallSize;px++)
			{
				for(pz=0;pz<SmallSize;pz++)
				{
					int bmpidx = px+(pz*SmallSize);
					if(pBitmapBits[bmpidx] == 0x00ffffff) pBitmapBits[bmpidx] = 0x00000000;
					else pBitmapBits[bmpidx] += 0xff000000;

					pSrcBits[(pz+1)*LIGHTMAP_TEX_SIZE + px + 1] = pBitmapBits[bmpidx];
				}
			}
			m_ppLightMapTexture[x][z]->Get()->UnlockRect(0);		

			SelectObject( hBMDC, hOldBM );
			SelectObject( hSmallDC, hOldBM2 );

			DeleteFile("c:\\templightmap.bmp"); // 임시 파일을 지워준다..	
		}
	}

	//테두리 만들기..
	D3DLOCKED_RECT d3dlrDest;
	DWORD* pDestBits;
	int srcX, srcZ;
	for(z=0;z<m_iHeightMapSize;z++)
	{
		for(x=0;x<m_iHeightMapSize;x++)
		{
			if(m_ppIsLightMap[x][z]==false) continue;

			m_ppLightMapTexture[x][z]->Get()->LockRect(0, &d3dlrDest, 0, 0);
			pDestBits = (DWORD*)d3dlrDest.pBits;

			// LT...
			srcX = x-1;
			srcZ = z+1;
			if( srcX>=0 && srcZ<m_iHeightMapSize && m_ppIsLightMap[srcX][srcZ]==true)
			{
				m_ppLightMapTexture[srcX][srcZ]->Get()->LockRect(0, &d3dlrSource, 0, 0);
				pSrcBits = (DWORD*)d3dlrSource.pBits;

				pDestBits[0] = pSrcBits[(LIGHTMAP_TEX_SIZE-2) + LIGHTMAP_TEX_SIZE*(LIGHTMAP_TEX_SIZE-2)];
				m_ppLightMapTexture[srcX][srcZ]->Get()->UnlockRect(0);
			}
			else pDestBits[0] = 0x00000000;

			// CT...
			srcX = x;
			srcZ = z+1;
			if( srcZ<m_iHeightMapSize && m_ppIsLightMap[srcX][srcZ]==true)
			{
				m_ppLightMapTexture[srcX][srcZ]->Get()->LockRect(0, &d3dlrSource, 0, 0);
				pSrcBits = (DWORD*)d3dlrSource.pBits;

				memcpy(&(pDestBits[1]), &(pSrcBits[ 1 + LIGHTMAP_TEX_SIZE*(LIGHTMAP_TEX_SIZE-2)]), sizeof(DWORD)*(LIGHTMAP_TEX_SIZE-2));
				m_ppLightMapTexture[srcX][srcZ]->Get()->UnlockRect(0);
			}
			else memset(&(pDestBits[1]), 0, sizeof(DWORD)*(LIGHTMAP_TEX_SIZE-2));

			// RT...
			srcX = x+1;
			srcZ = z+1;
			if( srcX<m_iHeightMapSize && srcZ<m_iHeightMapSize && m_ppIsLightMap[srcX][srcZ]==true)
			{
				m_ppLightMapTexture[srcX][srcZ]->Get()->LockRect(0, &d3dlrSource, 0, 0);
				pSrcBits = (DWORD*)d3dlrSource.pBits;

				pDestBits[LIGHTMAP_TEX_SIZE-1] = pSrcBits[ 1 + LIGHTMAP_TEX_SIZE*(LIGHTMAP_TEX_SIZE-2)];
				m_ppLightMapTexture[srcX][srcZ]->Get()->UnlockRect(0);
			}
			else pDestBits[LIGHTMAP_TEX_SIZE-1] = 0x00000000;

			// LM...
			srcX = x-1;
			srcZ = z;
			if( srcX>=0 && m_ppIsLightMap[srcX][srcZ]==true)
			{
				m_ppLightMapTexture[srcX][srcZ]->Get()->LockRect(0, &d3dlrSource, 0, 0);
				pSrcBits = (DWORD*)d3dlrSource.pBits;

				for(int i=1;i<(LIGHTMAP_TEX_SIZE-1);i++)
				{
					pDestBits[i*LIGHTMAP_TEX_SIZE] = pSrcBits[ (LIGHTMAP_TEX_SIZE-2) + (i*LIGHTMAP_TEX_SIZE)];
				}
				m_ppLightMapTexture[srcX][srcZ]->Get()->UnlockRect(0);
			}
			else
			{
				for(int i=1;i<(LIGHTMAP_TEX_SIZE-1);i++)
				{
					pDestBits[i*LIGHTMAP_TEX_SIZE] = 0x00000000;
				}
			}

			// RM...
			srcX = x+1;
			srcZ = z;
			if( srcX<m_iHeightMapSize && m_ppIsLightMap[srcX][srcZ]==true)
			{
				m_ppLightMapTexture[srcX][srcZ]->Get()->LockRect(0, &d3dlrSource, 0, 0);
				pSrcBits = (DWORD*)d3dlrSource.pBits;

				for(int i=1;i<(LIGHTMAP_TEX_SIZE-1);i++)
				{
					pDestBits[i*LIGHTMAP_TEX_SIZE + (LIGHTMAP_TEX_SIZE-2)] = pSrcBits[1 + (i*LIGHTMAP_TEX_SIZE)];
				}
				m_ppLightMapTexture[srcX][srcZ]->Get()->UnlockRect(0);
			}
			else
			{
				for(int i=1;i<(LIGHTMAP_TEX_SIZE-1);i++)
				{
					pDestBits[i*LIGHTMAP_TEX_SIZE + (LIGHTMAP_TEX_SIZE-2)] = 0x00000000;
				}
			}

			// LB...
			srcX = x-1;
			srcZ = z-1;
			if( srcX>=0 && srcZ>=0 && m_ppIsLightMap[srcX][srcZ]==true)
			{
				m_ppLightMapTexture[srcX][srcZ]->Get()->LockRect(0, &d3dlrSource, 0, 0);
				pSrcBits = (DWORD*)d3dlrSource.pBits;

				pDestBits[LIGHTMAP_TEX_SIZE*(LIGHTMAP_TEX_SIZE-1)] = pSrcBits[(LIGHTMAP_TEX_SIZE-2) + LIGHTMAP_TEX_SIZE];
				m_ppLightMapTexture[srcX][srcZ]->Get()->UnlockRect(0);
			}
			else pDestBits[LIGHTMAP_TEX_SIZE*(LIGHTMAP_TEX_SIZE-1)] = 0x00000000;

			// CB...
			srcX = x;
			srcZ = z-1;
			if( srcZ>=0 && m_ppIsLightMap[srcX][srcZ]==true)
			{
				m_ppLightMapTexture[srcX][srcZ]->Get()->LockRect(0, &d3dlrSource, 0, 0);
				pSrcBits = (DWORD*)d3dlrSource.pBits;

				memcpy(&(pDestBits[1 + LIGHTMAP_TEX_SIZE*(LIGHTMAP_TEX_SIZE-1)]), &(pSrcBits[1 + LIGHTMAP_TEX_SIZE]), sizeof(DWORD)*(LIGHTMAP_TEX_SIZE-2));
				m_ppLightMapTexture[srcX][srcZ]->Get()->UnlockRect(0);
			}
			else memset(&(pDestBits[1 + LIGHTMAP_TEX_SIZE*(LIGHTMAP_TEX_SIZE-1)]), 0, sizeof(DWORD)*(LIGHTMAP_TEX_SIZE-2));

			// RB...
			srcX = x+1;
			srcZ = z-1;
			if( srcX<m_iHeightMapSize && srcZ>=0 && m_ppIsLightMap[srcX][srcZ]==true)
			{
				m_ppLightMapTexture[srcX][srcZ]->Get()->LockRect(0, &d3dlrSource, 0, 0);
				pSrcBits = (DWORD*)d3dlrSource.pBits;

				pDestBits[LIGHTMAP_TEX_SIZE-1 + LIGHTMAP_TEX_SIZE*(LIGHTMAP_TEX_SIZE-1)] = pSrcBits[1 + LIGHTMAP_TEX_SIZE];
				m_ppLightMapTexture[srcX][srcZ]->Get()->UnlockRect(0);
			}
			else pDestBits[LIGHTMAP_TEX_SIZE-1 + LIGHTMAP_TEX_SIZE*(LIGHTMAP_TEX_SIZE-1)] = 0x00000000;

			m_ppLightMapTexture[x][z]->Get()->UnlockRect(0);
		}
	}
}


//
//	SaveGameData
//	게임에 쓸 데이타를 저장..
//
void CLyTerrain::SaveGameData(HANDLE hFile)
{
	MMap TexTree;
	MMIter TTIt;

	//
	///////////////////////
	// 데이타 재구성...
	//
	int x,z;
	short tmpTexIdx;
	short tmpTileIdx;
	///////////////////////

	CProgressBar ProgressBar;
	ProgressBar.Create("Analyze TileMap...", 50,  m_iHeightMapSize);

	TexTree.clear();
	for (x = 0; x < m_iHeightMapSize - 1; x++)
	{
		ProgressBar.StepIt();

		for (z = 0; z < m_iHeightMapSize - 1; z++)
		{
			tmpTexIdx = (m_ppMapData[x][z].DTexInfo1.TexIdx.TexID * NUM_DTEXTILE) + m_ppMapData[x][z].DTexInfo1.TexIdx.TileY;
			tmpTileIdx = m_ppMapData[x][z].DTexInfo1.TexIdx.TileX;

			if (GetTileTex(m_ppMapData[x][z].DTexInfo1.TexIdx.TexID) != nullptr)
			{
				TTIt = TexTree.lower_bound(tmpTexIdx);
				size_t TTCount = TexTree.count(tmpTexIdx);
				size_t i = 0;
				for (; i < TTCount; i++)
				{
					if ((*TTIt).second == tmpTileIdx)
						break;

					TTIt++;
				}

				if (i == TTCount && tmpTexIdx >= 0 && tmpTileIdx >= 0 && tmpTileIdx < 5000)
					TexTree.insert(std::make_pair(tmpTexIdx, tmpTileIdx));
			}

			tmpTexIdx = (m_ppMapData[x][z].DTexInfo2.TexIdx.TexID * NUM_DTEXTILE) + m_ppMapData[x][z].DTexInfo2.TexIdx.TileY;
			tmpTileIdx = m_ppMapData[x][z].DTexInfo2.TexIdx.TileX;

			if (GetTileTex(m_ppMapData[x][z].DTexInfo2.TexIdx.TexID) != nullptr)
			{
				TTIt = TexTree.lower_bound(tmpTexIdx);
				size_t TTCount = TexTree.count(tmpTexIdx);
				size_t i = 0;
				for (; i < TTCount; i++)
				{
					if ((*TTIt).second == tmpTileIdx)
						break;

					TTIt++;
				}

				if (i == TTCount && tmpTexIdx >= 0 && tmpTileIdx >= 0 && tmpTileIdx < 5000)
					TexTree.insert(std::make_pair(tmpTexIdx, tmpTileIdx));
			}
		}
	}
	
	int NumTile = static_cast<int>(TexTree.size());
	int NumTileSrcTex = 0;

	LList TileList;
	if (NumTile != 0)
	{
		TTIt = TexTree.begin();
		while (TTIt != TexTree.end())
		{
			NumTileSrcTex++;
			TileList.push_back((*TTIt).first);

			size_t TTCount = TexTree.count((*TTIt).first);
			std::advance(TTIt, TTCount);
		}
	}

	//
	// 기록 시작...^^...
	//
	DWORD dwRWC;
	WriteFile(hFile, &m_iHeightMapSize, sizeof(int), &dwRWC, nullptr);
	
	//	각 꼭지점 정보 저장...
	//  그냥 하나씩 저장해도 차이 없지만 네트워크로 저장할때는 파일 엑세스 숫자를 줄이고 한꺼번에 저장해야  빠르다.. ...
	DWORD dwAlloc = sizeof(GAMEMAPDATA) * m_iHeightMapSize * m_iHeightMapSize;
	HGLOBAL hAlloc = ::GlobalAlloc(GMEM_FIXED, dwAlloc);
	__ASSERT(hAlloc, "Global allocation failed.");
	GAMEMAPDATA* pGMDs = (GAMEMAPDATA*)GlobalLock(hAlloc);
	ProgressBar.Create("Writing TileMap...", 50, m_iHeightMapSize);
	for(x=0; x<m_iHeightMapSize;x++)
	{
		ProgressBar.StepIt();

		for (z = 0; z < m_iHeightMapSize; z++)
		{
			GAMEMAPDATA* pGMD = &pGMDs[x*m_iHeightMapSize+z];

			pGMD->bIsTileFull = true;
			pGMD->Tex1Idx = 1023;
			pGMD->Tex2Idx = 1023;
			pGMD->fHeight = m_ppMapData[x][z].fHeight;
			pGMD->Tex1Dir = (char)m_ppMapData[x][z].DTexInfo1.Dir;
			pGMD->Tex2Dir = (char)m_ppMapData[x][z].DTexInfo2.Dir;

			TTIt = TexTree.begin();
			for (int i = 0; i < NumTile; i++)
			{
				tmpTexIdx = (*TTIt).first;
				tmpTileIdx = (*TTIt).second;

				if ((tmpTexIdx == ((m_ppMapData[x][z].DTexInfo1.TexIdx.TexID * NUM_DTEXTILE) + m_ppMapData[x][z].DTexInfo1.TexIdx.TileY))
					&& (tmpTileIdx == m_ppMapData[x][z].DTexInfo1.TexIdx.TileX))
					pGMD->Tex1Idx = i;

				if ((tmpTexIdx == ((m_ppMapData[x][z].DTexInfo2.TexIdx.TexID * NUM_DTEXTILE) + m_ppMapData[x][z].DTexInfo2.TexIdx.TileY))
					&& (tmpTileIdx == m_ppMapData[x][z].DTexInfo2.TexIdx.TileX))
					pGMD->Tex2Idx = i;

				TTIt++;
			}

			if (pGMD->Tex2Idx < 0 && (m_ppMapData[x][z].DTexInfo1.Attr.Attr > 0))
				pGMD->bIsTileFull = false;
		}
	}
	
	WriteFile(hFile, pGMDs, dwAlloc, &dwRWC, nullptr); // 좀더 빨리 저장하려고 몰아서 저장....
	GlobalUnlock(hAlloc);
	GlobalFree(hAlloc); hAlloc = nullptr;
	pGMDs = nullptr;

	// 각 패치의 가운데 y값과 반지름 저장..
	int ti_PatchSize = 8;	// 32m = 8tiles..
	float re_HalfDist = (float)((ti_PatchSize>>1)<<2);	//패치길이의 절반..(m단위)
	float MinY, MaxY;
	
	int pat_HeightMapSize = (m_iHeightMapSize-1) / ti_PatchSize;

	float re_XZCrossHalfDistPow2 = pow((re_HalfDist * 1.4141592f),2);
	int sx, sz;
	float height;
	for(x=0; x<pat_HeightMapSize; x++)
	{
		for(z=0; z<pat_HeightMapSize; z++)
		{
			sx = x * ti_PatchSize;
			sz = z * ti_PatchSize;
			MinY = FLT_MAX;
			MaxY = -FLT_MAX;
			for(int px=0; px<=ti_PatchSize; px++)
			{
				for(int pz=0; pz<=ti_PatchSize; pz++)
				{
					height = m_ppMapData[sx+px][sz+pz].fHeight;
					if(MinY > height) MinY = height;
					if(MaxY < height) MaxY = height;					
				}
			}
			float MiddleY = (MaxY + MinY) / 2;
			float rad = sqrt( re_XZCrossHalfDistPow2 + pow((MiddleY-MinY),2) );
			WriteFile(hFile, &MiddleY, sizeof(float), &dwRWC, nullptr);
			WriteFile(hFile, &rad, sizeof(float), &dwRWC, nullptr);
		}
	}

	//타일에 풀 속성 저장..
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	pFrm->GetMapMng()->m_SowSeedMng.SaveDataGame();
	LPSEEDGROUP SeedAttr = new SEEDGROUP[m_iHeightMapSize*m_iHeightMapSize];
	ZeroMemory(SeedAttr, sizeof(unsigned char)*m_iHeightMapSize*m_iHeightMapSize);
	CDlgSowSeed* pSowSeed = pFrm->m_pDlgSowSeed;

	for (int i = 0; i < m_iHeightMapSize * m_iHeightMapSize; i++)
	{
		SeedAttr[i].Obj_Id = 0;
		SeedAttr[i].Seed_Count = 0;
		SeedAttr[i].SeedGroup_Sub = nullptr;
		SeedAttr[i].sub_flage = 0;
	}

	int temp_Id = 0;
	for (LPGRASS_GROUP group : pFrm->GetMapMng()->m_SowSeedMng.Grass_Group)
	{
		for (LPGRASS grass : group->grass)
		{
			temp_Id = 0;
			for (LPOBJ_NAME Obj : pFrm->GetMapMng()->m_SowSeedMng.Obj_Name)
			{
				if (strcmp(group->FileName, Obj->FileName) == 0)
					temp_Id = Obj->Id;
			}

			if (SeedAttr[grass->Tile_z + (grass->Tile_x * m_iHeightMapSize)].Obj_Id == 0)
			{
				SeedAttr[grass->Tile_z + (grass->Tile_x * m_iHeightMapSize)].Obj_Id = temp_Id + 1;

				if (SeedAttr[grass->Tile_z + (grass->Tile_x * m_iHeightMapSize)].Seed_Count < 15)
					SeedAttr[grass->Tile_z + (grass->Tile_x * m_iHeightMapSize)].Seed_Count += 1;
			}
			else
			{

				if (SeedAttr[grass->Tile_z + (grass->Tile_x * m_iHeightMapSize)].Obj_Id == temp_Id + 1)
				{
					if (SeedAttr[grass->Tile_z + (grass->Tile_x * m_iHeightMapSize)].Seed_Count < 15)
						SeedAttr[grass->Tile_z + (grass->Tile_x * m_iHeightMapSize)].Seed_Count += 1;
				}
				else
				{
					if (SeedAttr[grass->Tile_z + (grass->Tile_x * m_iHeightMapSize)].SeedGroup_Sub == nullptr)
					{
						SeedAttr[grass->Tile_z + (grass->Tile_x * m_iHeightMapSize)].sub_flage = 1;
						SeedAttr[grass->Tile_z + (grass->Tile_x * m_iHeightMapSize)].SeedGroup_Sub = new SEEDGROUP;
					}

					SeedAttr[grass->Tile_z + (grass->Tile_x * m_iHeightMapSize)].SeedGroup_Sub->Obj_Id = temp_Id + 1;
					if (SeedAttr[grass->Tile_z + (grass->Tile_x * m_iHeightMapSize)].SeedGroup_Sub->Seed_Count < 15)
						SeedAttr[grass->Tile_z + (grass->Tile_x * m_iHeightMapSize)].SeedGroup_Sub->Seed_Count += 1;
				}
			}
		}
	}
	for (int i = 0; i < m_iHeightMapSize * m_iHeightMapSize; i++)
	{
		WriteFile(hFile, &SeedAttr[i], sizeof(unsigned char), &dwRWC, nullptr);
		if (SeedAttr[i].SeedGroup_Sub != nullptr)
			WriteFile(hFile, SeedAttr[i].SeedGroup_Sub, sizeof(unsigned char), &dwRWC, nullptr);
	}



/*   원래의 풀 저장 
	int NumSeedInfo = pFrm->m_SeedGroupList.size();
	for(x=0; x<m_iHeightMapSize-1;x++)
	{
		for(z=0; z<m_iHeightMapSize-1;z++)
		{
			int Group = m_ppMapData[x][z].DTexInfo1.Attr.Group;

			std::list<LPSEEDGROUP>::iterator sgit = pFrm->m_SeedGroupList.begin();

			for(int i=0;i<NumSeedInfo;i++)
			{
				LPSEEDGROUP pSeedGroup = (*sgit);
				unsigned char seed = (unsigned char)pSeedGroup->iSeedID;
				if(Group == pSeedGroup->iDTexGroupID && ((seed & SeedAttr[z + (x*m_iHeightMapSize)])==0))
				{
					SeedAttr[z + (x*m_iHeightMapSize)] += seed;
					//break;
				}
				sgit++;
			}
		}
	}
	WriteFile(hFile, SeedAttr, sizeof(unsigned char)*m_iHeightMapSize*m_iHeightMapSize, &dwRWC, nullptr);
*/
	WriteFile(hFile, pFrm->m_SeedFileName, sizeof(char)*MAX_PATH, &dwRWC, nullptr);

	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
	_splitpath(m_szFileName.c_str(), szDrive, szDir, szFName, szExt);

	// 텍스트파일로 함 뽑아보자..
	FILE* stream = fopen("c:\\grass.txt", "w");
	for(z=0; z<m_iHeightMapSize;z++)
	{
		for(x=0; x<m_iHeightMapSize;x++)
		{
			SEEDGROUP v = SeedAttr[z + (x*m_iHeightMapSize)];
			fprintf(stream, "%d,%d\t",v.Obj_Id,v.Seed_Count );

			if( v.SeedGroup_Sub !=nullptr)
				fprintf(stream, "서브 %d,%d\t",v.SeedGroup_Sub->Obj_Id,v.SeedGroup_Sub->Seed_Count );


			fprintf(stream, "\n");
		}
	}
	fclose(stream);
	
	//WriteFile(hFile, szFName, _MAX_PATH, &dwRWC, nullptr); // 컬러맵 이름 저장.
	//컬러맵은 MapMng에서 따로 저장한다..
		
	//
	//	타일텍스쳐정보 저장...
	//
	WriteFile(hFile, &NumTile, sizeof(int), &dwRWC, nullptr);
	if(NumTile!=0)
	{
		WriteFile(hFile, &NumTileSrcTex, sizeof(int), &dwRWC, nullptr);

		// Tile Map Resource
		LLIter TLIt = TileList.begin();
		CN3Texture* pTexture;
		for (int i = 0; i < NumTileSrcTex; i++)
		{
			int TexIdx = (*TLIt) / NUM_DTEXTILE;
			int YIdx = (*TLIt) % NUM_DTEXTILE;

			pTexture = GetTileTex(TexIdx);
			if(pTexture)
			{
				// 경로를 빼고 파일이름과 확장자만 저장해준다.
				char szTileFN[MAX_PATH];
				_splitpath(pTexture->FileName().c_str(), nullptr, nullptr, szFName, nullptr);
				wsprintf(szTileFN, "dtex\\%s_%d.gtt", szFName, YIdx);
				WriteFile(hFile, szTileFN, MAX_PATH, &dwRWC, nullptr);
			}
			TLIt++;
		}

		int SrcIdx, TileIdx;
		TTIt = TexTree.begin();
		for (int i = 0; i < NumTile; i++)
		{
			SrcIdx = 0;
			for (TLIt = TileList.begin(); TLIt != TileList.end(); TLIt++)
			{
				if ((*TTIt).first == (*TLIt))
					WriteFile(hFile, &SrcIdx, sizeof(short), &dwRWC, nullptr);

				SrcIdx++;
			}
			TileIdx = (*TTIt).second;
			WriteFile(hFile, &TileIdx, sizeof(short), &dwRWC, nullptr);
			TTIt++;
		}
	}

	//라이트맵 정보 기록...
	DetectRealLightMap(0, 0, m_iHeightMapSize);
	int CountLightMap;

	CountLightMap = 0;	
	//라이트맵은 따로 파일로 저장해서 쓰기때문에 걍 0으로 해서 저장...
	//CountLightMap = m_iNumLightMap;	

	WriteFile(hFile, &(CountLightMap), sizeof(int), &dwRWC, nullptr); // LightMap의 갯수 기록..

	if(CountLightMap>0)
	{
		ProgressBar.Create("Save Light Map Data", 50, m_iNumLightMap);	
		
		short sx, sz;
		for(z=0;z<m_iHeightMapSize;z++)
		{
			for(x=0;x<m_iHeightMapSize;x++)
			{
				if(m_ppIsLightMap[x][z]==false) continue;

				sx = (short)x;	sz = (short)z;
				WriteFile(hFile, &(sx), sizeof(short), &dwRWC, nullptr);
				WriteFile(hFile, &(sz), sizeof(short), &dwRWC, nullptr);

				CN3Texture* pNewTex = new CN3Texture;
				LPDIRECT3DSURFACE9 pSurf;
				m_ppLightMapTexture[x][z]->Get()->GetSurfaceLevel(0, &pSurf);
				pNewTex->CreateFromSurface(pSurf, m_ppLightMapTexture[x][z]->PixelFormat(), true);
				pNewTex->Convert(D3DFMT_A4R4G4B4, LIGHTMAP_TEX_SIZE, LIGHTMAP_TEX_SIZE);
				pNewTex->Save(hFile);
				if(pSurf) pSurf->Release();
				delete pNewTex;

				ProgressBar.StepIt();
				CountLightMap--;
			}
			if(CountLightMap==0) break;
		}
	}
}


void CLyTerrain::MakeGameLightMap(char* szFullPathName)
{
	DetectRealLightMap(0, 0, m_iHeightMapSize);

	int PatchCount = (m_iHeightMapSize-1) / PATCH_TILE_SIZE;	//PATCH_TILE_SIZE = 8

	//int PatchInfo[4][4];
	int* PatchInfo = new int [PatchCount*PatchCount];
	ZeroMemory(PatchInfo, sizeof(int)*PatchCount*PatchCount);

	//각각의 파일을 만들어 보고 크기도 알아내자..
	int Size = sizeof(int)*PatchCount*PatchCount + sizeof(int);
	int px, pz;
	int tx, tz;
	int tpx, tpz;
	int TexCount;
	DWORD dwRWC;

	CProgressBar ProgressBar;
	ProgressBar.Create("Writing LightMap...", 50, PatchCount*PatchCount*2);
	for(px=0; px<PatchCount; px++)
	{
		for(pz=0; pz<PatchCount; pz++)
		{
			ProgressBar.StepIt();
			tpx = px*PATCH_TILE_SIZE;
			tpz = pz*PATCH_TILE_SIZE;
			TexCount = 0;
			for(tx=0; tx<PATCH_TILE_SIZE;tx++)
			{
				for(tz=0; tz<PATCH_TILE_SIZE;tz++)
				{
					if(m_ppIsLightMap[tpx + tx][tpz + tz]==true && m_ppLightMapTexture[tpx + tx][tpz + tz]) TexCount++;
				}
			}

			if(TexCount<=0) continue;

			//임시파일 만들고 저장함 해보고, 용량 알아낸다음....저장할까?

			char szTmpName[_MAX_PATH];
			sprintf(szTmpName, "c:\\temp_lightmap.binn");
			HANDLE hFile = CreateFile(szTmpName, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
						
			WriteFile(hFile, &(TexCount), sizeof(int), &dwRWC, nullptr); // LightMap의 갯수 기록..

			for(tx=0; tx<PATCH_TILE_SIZE;tx++)
			{
				for(tz=0; tz<PATCH_TILE_SIZE;tz++)
				{
					if(m_ppIsLightMap[tpx + tx][tpz + tz]==true && m_ppLightMapTexture[tpx + tx][tpz + tz])
					{
						WriteFile(hFile, &(tx), sizeof(int), &dwRWC, nullptr); // 패치안에서의 좌표
						WriteFile(hFile, &(tz), sizeof(int), &dwRWC, nullptr); // LightMap의 갯수 기록..

						m_ppLightMapTexture[tpx + tx][tpz + tz]->Convert(D3DFMT_A4R4G4B4, LIGHTMAP_TEX_SIZE, LIGHTMAP_TEX_SIZE);
						m_ppLightMapTexture[tpx + tx][tpz + tz]->Save(hFile);
					}
				}
			}

			DWORD dwPtr = SetFilePointer(hFile, 0, nullptr, FILE_BEGIN);
			DWORD dwSize = GetFileSize(hFile, nullptr);
			if(dwSize!=0xFFFFFFFF)
			{
				PatchInfo[px + (pz*PatchCount) ] = Size;
				Size += dwSize;
			}
			CloseHandle(hFile);	
			DeleteFile(szTmpName);
		}
	}
	
	HANDLE hFile = CreateFile(szFullPathName, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	int version = 0;
	WriteFile(hFile, &(version), sizeof(int), &dwRWC, nullptr);
	WriteFile(hFile, &(PatchInfo[0]), sizeof(int)*PatchCount*PatchCount, &dwRWC, nullptr);
	
	for(px=0; px<PatchCount; px++)
	{
		for(pz=0; pz<PatchCount; pz++)
		{
			ProgressBar.StepIt();
			//if(PatchInfo[px + (pz*PatchCount)]<=0) continue;

			tpx = px*PATCH_TILE_SIZE;
			tpz = pz*PATCH_TILE_SIZE;
			TexCount = 0;
			for(tx=0; tx<PATCH_TILE_SIZE;tx++)
			{
				for(tz=0; tz<PATCH_TILE_SIZE;tz++)
				{
					if(m_ppIsLightMap[tpx + tx][tpz + tz]==true && m_ppLightMapTexture[tpx + tx][tpz + tz]) TexCount++;
				}
			}

			if(TexCount<=0) continue;

			WriteFile(hFile, &(TexCount), sizeof(int), &dwRWC, nullptr); // LightMap의 갯수 기록..
			for(tx=0; tx<PATCH_TILE_SIZE;tx++)
			{
				for(tz=0; tz<PATCH_TILE_SIZE;tz++)
				{
					if(m_ppIsLightMap[tpx + tx][tpz + tz]==true && m_ppLightMapTexture[tpx + tx][tpz + tz])
					{
						WriteFile(hFile, &(tx), sizeof(int), &dwRWC, nullptr); // 패치안에서의 좌표
						WriteFile(hFile, &(tz), sizeof(int), &dwRWC, nullptr); // 

						CN3Texture* pNewTex = new CN3Texture;
						LPDIRECT3DSURFACE9 pSurf;
						m_ppLightMapTexture[tpx + tx][tpz + tz]->Get()->GetSurfaceLevel(0, &pSurf);
						pNewTex->CreateFromSurface(pSurf, m_ppLightMapTexture[tpx + tx][tpz + tz]->PixelFormat(), true);
						pNewTex->Convert(D3DFMT_A4R4G4B4, LIGHTMAP_TEX_SIZE, LIGHTMAP_TEX_SIZE);
						pNewTex->Save(hFile);
						if(pSurf) pSurf->Release();
					}
				}
			}
			
		}
	}
	CloseHandle(hFile);
	delete[] PatchInfo;
}

void CLyTerrain::MakeGameColorMap(char* szFullPathName)
{
	HANDLE hCMFile = CreateFile(szFullPathName, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	CProgressBar ProgressBar; // 진행 상황..
	ProgressBar.Create("Save game color map..", 50, m_iNumColorMap * m_iNumColorMap);
	ProgressBar.SetStep(1);
	
	///////////////////////////////////////////////////////////////
	//원본...(축소시키지 않는방법..)
	//*
	CN3Texture TexTmp;
	TexTmp.Create(m_iColorMapTexSize, m_iColorMapTexSize, D3DFMT_A1R5G5B5, TRUE);
	int x,z;
	for(x=0;x<m_iNumColorMap;x++)
	{
		for(z=0;z<m_iNumColorMap;z++)
		{
			ProgressBar.StepIt();
			
			LPDIRECT3DSURFACE9 lpSurfSrc = nullptr;
			m_pColorTexture[x][z].Get()->GetSurfaceLevel(0, &lpSurfSrc);
			if(nullptr == lpSurfSrc)
			{
				MessageBox(::GetActiveWindow(), "No Colormap", "Save Game Color Map Error.", MB_OK);
				continue;
			}

			TexTmp.GenerateMipMap(lpSurfSrc);
			TexTmp.Save(hCMFile);

			lpSurfSrc->Release();
		}
	}
	//*/
	///////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////
	//
	//	New...(축소시키는방법..)
	/*
	int x,z;
	CBitMapFile BMP[3][3];
	char buff[80];
	unsigned char* pBMPImg;
	for(x=0;x<3;x++)
	{
		for(z=0;z<3;z++) BMP[x][z].Create(m_iColorMapTexSize,m_iColorMapTexSize);
	}

	CN3Texture TexTmp;
	TexTmp.Create(m_iColorMapTexSize, m_iColorMapTexSize, D3DFMT_X8R8G8B8, TRUE);
	
	D3DLOCKED_RECT d3dlrTex;
	DWORD* pTexBits;		
	for(x=0;x<m_iNumColorMap;x++)
	{
		for(z=0;z<m_iNumColorMap;z++)
		{
			int ax, az;
			for(ax=0;ax<3;ax++)
			{
				for(az=0;az<3;az++)
				{
					sprintf(buff,"c:\\tmpcolormap%04d%04d.bmp", ax, az);
					if((x+ax-1)>=0 && (z+az-1)>=0 && (x+ax-1)<m_iNumColorMap && (z+az-1)<m_iNumColorMap) m_pColorTexture[x+ax-1][z+az-1].SaveToBitmapFile(buff);
					else BMP[ax][az].SaveToFile(buff);
				}	//for(int az=0;az<3;az++)
			}	//for(int ax=0;ax<3;ax++)

			//픽셀 재배열...
			int SmallSize = m_iColorMapTexSize - 2;
			TexTmp.Convert(D3DFMT_X8R8G8B8, m_iColorMapTexSize, m_iColorMapTexSize);
			TexTmp.Get()->LockRect(0, &d3dlrTex, 0, 0);
			pTexBits = (DWORD*)d3dlrTex.pBits;
			for(ax=0;ax<3;ax++)
			{
				for(az=0;az<3;az++)
				{
					sprintf(buff,"c:\\tmpcolormap%04d%04d.bmp", ax, az);
					HANDLE hSrcBitmap = LoadImage(0, buff, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE|LR_DEFAULTSIZE);
					__ASSERT(hSrcBitmap, "");

					HDC hSmallDC = CreateCompatibleDC(nullptr);
					HDC hBMDC = CreateCompatibleDC(hSmallDC);

					HBITMAP hOldBM = (HBITMAP)SelectObject(hBMDC, (HBITMAP)hSrcBitmap);

					// Prepare to create a bitmap
					DWORD* pBitmapBits;
					BITMAPINFO bmi;
					ZeroMemory( &bmi.bmiHeader,  sizeof(BITMAPINFOHEADER) );
					bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
					bmi.bmiHeader.biWidth       =  (int)SmallSize;
					bmi.bmiHeader.biHeight      = -(int)SmallSize;
					bmi.bmiHeader.biPlanes      = 1;
					bmi.bmiHeader.biCompression = BI_RGB;
					bmi.bmiHeader.biBitCount    = 32;

					HBITMAP hbmBitmap = CreateDIBSection( hSmallDC, &bmi, DIB_RGB_COLORS, (VOID**)&pBitmapBits, nullptr, 0 );
					HBITMAP hOldBM2 = (HBITMAP) SelectObject( hSmallDC, hbmBitmap );

					SetStretchBltMode(hSmallDC, HALFTONE);
					StretchBlt(hSmallDC, 0,0, SmallSize, SmallSize, hBMDC, 0,0, m_iColorMapTexSize, m_iColorMapTexSize, SRCCOPY);
				
					if(ax==0 && az==0) 
						pTexBits[(m_iColorMapTexSize-1)*m_iColorMapTexSize] = pBitmapBits[SmallSize-1];
					else if(ax==1 && az==0)
						memcpy(&(pTexBits[(m_iColorMapTexSize-1)*m_iColorMapTexSize + 1]), &(pBitmapBits[0]), sizeof(DWORD)*SmallSize);
					else if(ax==2 && az==0)
						pTexBits[(m_iColorMapTexSize*m_iColorMapTexSize)-1] = pBitmapBits[0];
					else if(ax==0 && az==1)
					{
						for(int i=0;i<SmallSize;i++)
						{
							pTexBits[(i+1)*m_iColorMapTexSize] = pBitmapBits[(i+1)*SmallSize - 1];
						}
					}
					else if(ax==1 && az==1)
					{
						for(int i=0;i<SmallSize;i++)
							memcpy(&(pTexBits[(i+1)*m_iColorMapTexSize + 1]), &(pBitmapBits[i*SmallSize]), sizeof(DWORD)*SmallSize);
					}
					else if(ax==2 && az==1)
					{
						for(int i=0;i<SmallSize;i++)
							pTexBits[(i+1)*m_iColorMapTexSize + (m_iColorMapTexSize-1)] = pBitmapBits[i*SmallSize];
					}
					else if(ax==0 && az==2) 
						pTexBits[0] = pBitmapBits[(SmallSize*SmallSize) - 1];
					else if(ax==1 && az==2)
					{
						memcpy(&(pTexBits[1]), &(pBitmapBits[(SmallSize-1)*SmallSize]), sizeof(DWORD)*SmallSize);
					}
					else if(ax==2 && az==2)
						pTexBits[m_iColorMapTexSize - 1] = pBitmapBits[(SmallSize-1)*SmallSize];

					SelectObject( hBMDC, hOldBM );
					SelectObject( hSmallDC, hOldBM2 );
					

					DeleteFile(buff); // 임시 파일을 지워준다..	
				}
			}
			TexTmp.Get()->UnlockRect(0);
			sprintf(buff,"tmpb%04d%04d.bmp", x,z);
			TexTmp.SaveToBitmapFile(buff);
			
			TexTmp.Convert(D3DFMT_A1R5G5B5, m_iColorMapTexSize, m_iColorMapTexSize);
			TexTmp.Save(hCMFile);
			
		}	//for(z=0;z<m_iNumColorMap;z++)
	}	//for(x=0;x<m_iNumColorMap;x++)
	//*/
	//
	///////////////////////////////////////////////////////////////////////////////////////////

	CloseHandle(hCMFile);
	
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 픽셀을 하나씩 밀어서 선을 없애는 텍스쳐 저장..
// 그런데 texturestagestate에서 mirror쓰면 좀 어색하긴 해도 어느정도는 되더라...-.-
//
/*
	HANDLE hCMFile = CreateFile(szFullPathName, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	CProgressBar ProgressBar; // 진행 상황..
	ProgressBar.Create("Save game color map..", 50, m_iNumColorMap * m_iNumColorMap);
	ProgressBar.SetStep(1);

	//bitmap가공...
	ColorMapExport("c:\\TempColormap.bmp");

	int OrgWidth, OrgHeight;
	OrgWidth = m_iColorMapTexSize * m_iNumColorMap;
	OrgHeight = m_iColorMapTexSize * m_iNumColorMap;

	int SmallWidth, SmallHeight;
	SmallWidth = OrgWidth - ((OrgWidth / m_iColorMapTexSize)<<1);
	SmallHeight = OrgHeight - ((OrgHeight / m_iColorMapTexSize)<<1);

	HANDLE hSrcBitmap = LoadImage(0, "c:\\TempColormap.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE|LR_DEFAULTSIZE);
	__ASSERT(hSrcBitmap, "");

	CBitmap PatchBitmap;
	HDC hSmallDC = CreateCompatibleDC(nullptr);
	HDC hBMDC = CreateCompatibleDC(hSmallDC);

	HBITMAP hOldBM = (HBITMAP)SelectObject(hBMDC, (HBITMAP)hSrcBitmap);

	// Prepare to create a bitmap
    DWORD* pBitmapBits;
    BITMAPINFO bmi;
    ZeroMemory( &bmi.bmiHeader,  sizeof(BITMAPINFOHEADER) );
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       =  (int)SmallWidth;
    bmi.bmiHeader.biHeight      = -(int)SmallHeight;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biBitCount    = 32;

	HBITMAP hbmBitmap = CreateDIBSection( hSmallDC, &bmi, DIB_RGB_COLORS,
                                          (VOID**)&pBitmapBits, nullptr, 0 );
    HBITMAP hOldBM2 = (HBITMAP) SelectObject( hSmallDC, hbmBitmap );

	StretchBlt(hSmallDC, 0,0, SmallWidth, SmallHeight,
				hBMDC, 0,0, OrgWidth, OrgHeight, SRCCOPY);


	// n3texture 가공..
	CN3Texture TexTmp;
	LPDWORD pImgTarget;
	D3DLOCKED_RECT d3dlrTarget;

	int TexSize = m_pColorTexture[0][0].Width();
	int x,z;
	int sx,sz;
	int DestWidth, DestHeight;
	for(x=0;x<m_iNumColorMap;x++)
	{
		for(z=m_iNumColorMap-1;z>=0;z--)
		{
			ProgressBar.StepIt();						
			TexTmp.Create(TexSize, TexSize, D3DFMT_X8R8G8B8, TRUE);
			TexTmp.Get()->LockRect( 0, &d3dlrTarget, 0, 0 );
			pImgTarget = (LPDWORD)d3dlrTarget.pBits;

			sx = x * (TexSize-2) - 1;
			sz = z * (TexSize-2) - 1;
			DestWidth = TexSize;
			DestHeight = TexSize;
			
			if(x==0)
			{
				sx = 0;
				DestWidth = TexSize - 1;
				pImgTarget++;
			}
			else if(x==m_iNumColorMap-1)
			{
				DestWidth = TexSize - 1;				
			}
			
			if(z==0)
			{
				sz = 0;
				DestHeight = TexSize - 1;
				pImgTarget += TexSize;
			}
			else if(z==m_iNumColorMap-1)
			{
				DestHeight = TexSize - 1;				
			}
			
			for(int i=0;i<DestHeight;i++)
				memcpy(&(pImgTarget[i*TexSize]), &(pBitmapBits[sx + (sz+i)*SmallWidth]), DestWidth*sizeof(DWORD));

			TexTmp.Get()->UnlockRect(0);
			TexTmp.Convert(D3DFMT_DXT1); // DXT1 형식으로 Convert
			TexTmp.Save(hCMFile);			
		}
	}

	CloseHandle(hCMFile);

	SelectObject( hBMDC, hOldBM );
	SelectObject( hSmallDC, hOldBM2 );

	DeleteFile("c:\\TempColormap.bmp"); // 임시 파일을 지워준다..
*/
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




	////////////////////////////////////////////////////////////////////////////////////////////////
	// 컬러맵을 저장해준다.(old version..)
	// 컬러 맵 이름.. 파일이름만(확장자는 없다.) 저장해준다.
	// 컬러맵을 패치별로  따로 저장...
	/*
	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
	_splitpath(m_szFileName, szDrive, szDir, szFName, szExt);
	
	CProgressBar ProgressBar; // 진행 상황..
	ProgressBar.Create("Save game color map..", m_iNumColorMap * m_iNumColorMap, 50);
	ProgressBar.SetStep(1);

	WriteFile(hFile, szFName, _MAX_PATH, &dwRWC, nullptr); // 컬러맵 이름 저장.

	CN3Texture TexTmp;
	char szNewFName[_MAX_PATH] = "", szAdd[_MAX_PATH] = "";
	for(x=0;x<m_iNumColorMap;x++)
	{
		for(z=0;z<m_iNumColorMap;z++)
		{
			ProgressBar.StepIt();

			m_pColorTexture[x][z].SaveToFile("c:\\TempConversion.DXT"); // 임시로 저장.
			TexTmp.LoadFromFile("c:\\TempConversion.DXT"); // 읽는다. 
			TexTmp.Convert(D3DFMT_DXT1); // DXT1 형식으로 Convert

			_makepath(szNewFName, szDrive, szDir, szFName, nullptr);
			wsprintf(szAdd, "_%02d%02d.dxt", x, z);
			lstrcat(szNewFName, szAdd);
			TexTmp.SaveToFile(szNewFName);
		}
	}
	DeleteFile("c:\\TempConversion.DXT"); // 임시 파일을 지워준다..
	*/
	//
	///////////////////////////////////////////////////////////////////////////////////////////
}


//
//	Tick...
//
void CLyTerrain::Tick()
{
	RECT	rt = m_VisibleRect;
	POINT	PrvEye = m_EyePos;

	SetVisibleRect();
	if( (PrvEye.x==m_EyePos.x) && (PrvEye.y==m_EyePos.y) && EqualRect(&rt, &m_VisibleRect) ) return;
	
	ClearRenderInfo();
	m_RenderNodes.clear();
	m_pRoot->Tick();
}


//
//	가시거리 범위 정하기...(타일단위)
//
void CLyTerrain::SetVisibleRect()
{
	int tx, tz;
	tx = (int)(CN3Base::s_CameraData.vEye.x / TERRAIN_CELL_SIZE);
	tz = (int)(CN3Base::s_CameraData.vEye.z / TERRAIN_CELL_SIZE);

	m_EyePos.x = tx;
	m_EyePos.y = tz;

	// for test..
	//SetRect(&m_VisibleRect, 0, 0, 1023, 1023);
	//return;
	//

	m_VisibleRect.left = m_VisibleRect.right = tx;
	m_VisibleRect.top = m_VisibleRect.bottom = tz;

	int i;


	// 사면체의 법선 벡터와 Far 네 귀퉁이 위치 계산..
	float fS = sinf(CN3Base::s_CameraData.fFOV / 2.0f);
	float fPL = CN3Base::s_CameraData.fFP;
	float fAspect = CN3Base::s_CameraData.fAspect; // 종횡비
	
	// Far Plane 의 네 귀퉁이 위치 계산
	__Vector3 vFPs[4] = {	__Vector3(fPL * -fS * fAspect, fPL * fS, fPL),	// LeftTop
							__Vector3(fPL * fS * fAspect, fPL * fS, fPL),		// rightTop
							__Vector3(fPL * fS * fAspect, fPL * -fS, fPL),	// RightBottom
							__Vector3(fPL * -fS * fAspect, fPL * -fS, fPL) }; // LeftBottom

	for(i=0;i<4;i++)
	{
		// 귀퉁이 위치에 회전 행렬을 적용한다..
		vFPs[i] = vFPs[i] * CN3Base::s_CameraData.mtxViewInverse;

		int tx, tz;// 타일좌표계...
		tx = (int)(vFPs[i].x / TERRAIN_CELL_SIZE);
		tz = (int)(vFPs[i].z / TERRAIN_CELL_SIZE);

		if(m_VisibleRect.left > tx) m_VisibleRect.left = tx;
		if(m_VisibleRect.right < tx) m_VisibleRect.right = tx;
		if(m_VisibleRect.bottom < tz) m_VisibleRect.bottom = tz;
		if(m_VisibleRect.top > tz) m_VisibleRect.top = tz;
	}

	m_VisibleRect.left -= 8;
	m_VisibleRect.top -= 8;
	m_VisibleRect.right += 8;
	m_VisibleRect.bottom += 8;

	if(m_VisibleRect.left < 0) m_VisibleRect.left = 0;
	if(m_VisibleRect.top < 0) m_VisibleRect.top = 0;
	if(m_VisibleRect.right >= m_iHeightMapSize) m_VisibleRect.right = m_iHeightMapSize - 2;
	if(m_VisibleRect.bottom >= m_iHeightMapSize) m_VisibleRect.bottom = m_iHeightMapSize - 2;
}


//
//	Render()
//
void CLyTerrain::Render()
{
	s_lpD3DDev->SetMaterial(&m_Material);

	__Matrix44 WorldMtx;
	WorldMtx.Identity();
	s_lpD3DDev->SetTransform(D3DTS_WORLD, &WorldMtx);

	s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	
	//s_lpD3DDev->SetRenderState(D3DRS_FILLMODE, m_FillMode);
	s_lpD3DDev->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

	DWORD AddressU1, AddressV1, AddressU2, AddressV2;
	HRESULT hr;
	hr = s_lpD3DDev->GetSamplerState( 0, D3DSAMP_ADDRESSU, &AddressU1 );
	hr = s_lpD3DDev->GetSamplerState( 0, D3DSAMP_ADDRESSV, &AddressV1 );
	hr = s_lpD3DDev->GetSamplerState( 1, D3DSAMP_ADDRESSU, &AddressU2 );
	hr = s_lpD3DDev->GetSamplerState( 1, D3DSAMP_ADDRESSV, &AddressV2 );

	// 각각의 텍스쳐들을 연결했을때 경계선을 없앨 수 있다..^^
	hr = s_lpD3DDev->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_MIRROR );
	hr = s_lpD3DDev->SetSamplerState( 0, D3DSAMP_ADDRESSV,  D3DTADDRESS_MIRROR );
	hr = s_lpD3DDev->SetSamplerState( 1, D3DSAMP_ADDRESSU,  D3DTADDRESS_MIRROR );
	hr = s_lpD3DDev->SetSamplerState( 1, D3DSAMP_ADDRESSV,  D3DTADDRESS_MIRROR );

	CMainFrame* pFrame = (CMainFrame*) AfxGetMainWnd();

	if (pFrame->m_pDlgMapView->Select_Map_Edit)
	{
		pFrame->m_pDlgMapView->Render();
	}
	else
	{
		for (CQTNode* pQTNode : m_RenderNodes)
			pQTNode->Render();
	}

	RenderBrushArea();

	//라이트맵 라인..
	//if(m_bDrawLineLightMap)
	if(m_vLineLightMap[0]!=m_vLineLightMap[1])
	{
		HRESULT hr;
		WorldMtx.Identity();
		s_lpD3DDev->SetTransform(D3DTS_WORLD, &WorldMtx);

		hr = s_lpD3DDev->SetTexture(0, nullptr);
		hr = s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		hr = s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);

		// backup state
		DWORD dwZEnable, dwLighting;
		hr = s_lpD3DDev->GetRenderState(D3DRS_ZENABLE, &dwZEnable);
		hr = s_lpD3DDev->GetRenderState(D3DRS_LIGHTING, &dwLighting);

		// set state
		hr = s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
		hr = s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);

		hr = s_lpD3DDev->SetFVF(FVF_XYZCOLOR);

		hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINELIST, 1, m_vLineLightMap, sizeof(__VertexXyzColor));

		hr = s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, dwZEnable);
		hr = s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, dwLighting);
	}
	
	hr = s_lpD3DDev->SetSamplerState( 0, D3DSAMP_ADDRESSU, AddressU1 );
	hr = s_lpD3DDev->SetSamplerState( 0, D3DSAMP_ADDRESSV, AddressV1 );
	hr = s_lpD3DDev->SetSamplerState( 1, D3DSAMP_ADDRESSU, AddressU2 );
	hr = s_lpD3DDev->SetSamplerState( 1, D3DSAMP_ADDRESSV, AddressV2 );
}


//
//	각점에 대해 그릴지아닐지를 초기화..('아니다'로 초기화)
//
inline	void CLyTerrain::ClearRenderInfo()
{
	for(int i=0;i<m_iHeightMapSize;i++)
		memset(m_ppRenderInfo[i], FALSE, m_iHeightMapSize*sizeof(BOOL));
}


//
//	Log2(x) = l..
//	2의 승수에 대해서만 제대로 작동...(x>0)
//
inline int CLyTerrain::Log2(int x)
{
	int l = 0;
	while(x!=1)
	{
		x = x >> 1;
		l++;
	}
	return l;
}


//
//	높이값 얻는 함수...
//
float CLyTerrain::GetHeight(float x, float z)
{
	int ix, iz;
	ix = ((int)x) / TERRAIN_CELL_SIZE;
	iz = ((int)z) / TERRAIN_CELL_SIZE;

	if(ix<0 || ix>(m_iHeightMapSize-2)) return -FLT_MAX;
	if(iz<0 || iz>(m_iHeightMapSize-2)) return -FLT_MAX;

	float dX, dZ;
	dX = (x - (ix*TERRAIN_CELL_SIZE)) / TERRAIN_CELL_SIZE;
	dZ = (z - (iz*TERRAIN_CELL_SIZE)) / TERRAIN_CELL_SIZE;

	float y;
	float h1, h2, h3, h12, h13;

	if((ix+iz)%2==0)	//사각형이 / 모양.. 
	{
		h1 = m_ppMapData[ix][iz].fHeight;
		h3 = m_ppMapData[ix+1][iz+1].fHeight;
		if (dZ > dX)	//윗쪽 삼각형..
		{
			h2 = m_ppMapData[ix][iz+1].fHeight;
						
			h12 = h1 + (h2-h1) * dZ;	// h1과 h2사이의 높이값
			h13 = h1 + (h3-h1) * dZ;	// h1과 h3사이의 높이값

			y = h12 + ((h13-h12) * (dX/dZ));	// 찾고자 하는 높이값
			return y;
		}
		else	//아래쪽 삼각형..
		{
			if(dX==0.0f) return h1;

			h2 = m_ppMapData[ix+1][iz].fHeight;
			
			h12 = h1 + (h2-h1) * dX;	// h1과 h2사이의 높이값
			h13 = h1 + (h3-h1) * dX;	// h1과 h3사이의 높이값
			
			y = h12 + ((h13-h12) * (dZ/dX));	// 찾고자 하는 높이값
			return y;
		}
	}
	else if ((ix+iz)%2==1)	//사각형이 역슬레쉬 모양..
	{
		h1 = m_ppMapData[ix+1][iz].fHeight;
		h3 = m_ppMapData[ix][iz+1].fHeight;

		if ((dX+dZ) > 1.0f)	//윗쪽 삼각형..
		{
			if(dZ==0.0f) return h1;
			h2 = m_ppMapData[ix+1][iz+1].fHeight;
			
			h12 = h1 + (h2-h1) * dZ;
			h13 = h1 + (h3-h1) * dZ;

			y = h12 + ((h13-h12) * ((1.0f-dX)/dZ));
			return y;
		}
		else	//아래쪽 삼각형..
		{
			if(dX==1.0f) return h1;
			h2 = m_ppMapData[ix][iz].fHeight;

			h12 = h2+(h1-h2)*dX;	// h1과 h2사이의 높이값
			h13 = h3+(h1-h3)*dX;	// h1과 h3사이의 높이값
			
			y = h12 + ((h13-h12) * (dZ/(1.0f-dX)));
			return y;
		}
	}
	return -FLT_MAX;
}


//
//	GetPatchNum(float fSize)
//	fSize크기의 패치가 전체 맵에서 가로세로로 몇개씩 나오는지....
//
SIZE CLyTerrain::GetPatchNum(float fSize)
{
	SIZE size;
	if(fSize==0) 
	{
		size.cx = size.cy = 0;
		return size;
	}

	size.cx = (long)(((TERRAIN_CELL_SIZE * (m_iHeightMapSize-1)) + (fSize-0.0001)) / fSize);
	size.cy = (long)(((TERRAIN_CELL_SIZE * (m_iHeightMapSize-1)) + (fSize-0.0001)) / fSize);
	return size;
}


//
//	MouseMsgFilter(LPMSG pMsg)
//	마우스 처리..
//
BOOL CLyTerrain::MouseMsgFilter(LPMSG pMsg)
{
	if (m_iEditMode == TEM_NOT) return FALSE;

	static BOOL bEditDrag = FALSE;
	static POINT ptSelHeightMapPos;
	static int	iPrevScreenY = 0;
	const float fDelta = 0.10f;
	static int	iSumOfEditedHeight=0;	// 이번 드래그로 변화된 지형높이의 합

	switch (pMsg->message)
	{
	case WM_MOUSEMOVE:
		{
			DWORD_PTR nFlags = pMsg->wParam;
			POINT point = {short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam))};
			if(nFlags & MK_LBUTTON && m_iEditMode == TEM_DTEX)
			{
				__Vector3 vPos;
				if (Pick(point.x, point.y, &vPos, &ptSelHeightMapPos))
				{
					int ix, iz;
					ix = (int)vPos.x/TERRAIN_CELL_SIZE;
					iz = (int)vPos.z/TERRAIN_CELL_SIZE;
					SetDTex(ix, iz);
				}
			}

			if(nFlags & MK_LBUTTON && m_iEditMode == TEM_LIGHTMAP)	//라이트맵칠하기..
			{
				__Vector3 vPos;
				if (Pick(point.x, point.y, &vPos, &ptSelHeightMapPos))
				{
					int ix, iz;
					ix = (int)vPos.x/TERRAIN_CELL_SIZE;
					iz = (int)vPos.z/TERRAIN_CELL_SIZE;

					int lcx, lcz;
					lcx = (int)( ( (vPos.x - (float)(ix * TERRAIN_CELL_SIZE)) / (float)TERRAIN_CELL_SIZE ) * (float)(LIGHTMAP_TEX_SIZE-2) );
					lcz = (int)( ( (vPos.z - (float)(iz * TERRAIN_CELL_SIZE)) / (float)TERRAIN_CELL_SIZE ) * (float)(LIGHTMAP_TEX_SIZE-2) );

					SetLightMap(ix, iz, lcx, lcz);
				}
			}

			if(nFlags & MK_RBUTTON && m_iEditMode == TEM_LIGHTMAP)	//라이트맵칠하기..
			{
				__Vector3 vPos;
				if (Pick(point.x, point.y, &vPos, &ptSelHeightMapPos))
				{
					m_vLineLightMap[1].Set(vPos.x, vPos.y+0.1f, vPos.z, 0xffff40ff);
				}
			}

			if(m_iEditMode == TEM_FLAT)
			{
				UpdateBrushArea(ptSelHeightMapPos);
			}
			
			if (nFlags & MK_LBUTTON && bEditDrag && m_iEditMode == TEM_HEIGHTEN)
			{
				int iDiff = iPrevScreenY-point.y;
				Heighten(ptSelHeightMapPos, iDiff*fDelta);
				iSumOfEditedHeight += iDiff;
				UpdateBrushArea(ptSelHeightMapPos);
			}
			else if (Pick(point.x, point.y, nullptr, &ptSelHeightMapPos))
			{
				UpdateBrushArea(ptSelHeightMapPos);
			}
			iPrevScreenY = point.y;
			return TRUE;
		}
		break;
	case WM_LBUTTONDOWN:
		{
			POINT point = {short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam))};
			__Vector3 vPos;
			if (Pick(point.x, point.y, &vPos, &ptSelHeightMapPos))
			{
				if(m_iEditMode == TEM_DTEX)
				{
					int ix, iz;
					ix = (int)vPos.x/TERRAIN_CELL_SIZE;
					iz = (int)vPos.z/TERRAIN_CELL_SIZE;
					SetDTex(ix, iz);
				}
				if(m_iEditMode == TEM_LIGHTMAP)	//라이트맵칠하기..
				{
					int ix, iz;
					ix = (int)vPos.x/TERRAIN_CELL_SIZE;
					iz = (int)vPos.z/TERRAIN_CELL_SIZE;

					int lcx, lcz;
					lcx = (int)( ( (vPos.x - (float)(ix * TERRAIN_CELL_SIZE)) / (float)TERRAIN_CELL_SIZE ) * (float)(LIGHTMAP_TEX_SIZE-2) );
					lcz = (int)( ( (vPos.z - (float)(iz * TERRAIN_CELL_SIZE)) / (float)TERRAIN_CELL_SIZE ) * (float)(LIGHTMAP_TEX_SIZE-2) );

					SetLightMap(ix, iz, lcx, lcz);
				}
				if (m_iEditMode == TEM_HEIGHTEN)
				{
					bEditDrag = TRUE;
					iSumOfEditedHeight = 0;
					SetCapture(pMsg->hwnd);
				}
				else if (m_iEditMode == TEM_SMOOTH)
				{
					Smooth(ptSelHeightMapPos);
				}
				else if(m_iEditMode == TEM_FLAT)
				{
					if(m_bFlaten) Flaten(ptSelHeightMapPos);
				}
				UpdateBrushArea(ptSelHeightMapPos);
			}
			iPrevScreenY = point.y;
			return TRUE;
		}
		break;
	case WM_LBUTTONUP:
		{
			POINT point = {short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam))};
			if (bEditDrag && m_iEditMode == TEM_HEIGHTEN)
			{
				ReleaseCapture();
				bEditDrag = FALSE;
				Heighten(ptSelHeightMapPos, (iPrevScreenY-point.y)*fDelta);
				m_pRoot->ClearMinMaxY();
				m_pRoot->SetMinMaxY();
				UpdateBrushArea(ptSelHeightMapPos);
			}
			else if (m_iEditMode == TEM_SMOOTH)
			{
				ReleaseCapture();
				bEditDrag = FALSE;
				Smooth(ptSelHeightMapPos);
				m_pRoot->ClearMinMaxY();
				m_pRoot->SetMinMaxY();
				UpdateBrushArea(ptSelHeightMapPos);
			}
			iPrevScreenY = point.y;
			return TRUE;
		}
		break;
	case WM_RBUTTONUP:
		{
			if(m_iEditMode == TEM_LIGHTMAP)
			{
				m_bDrawLineLightMap = false;
				POINT point = {short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam))};
				__Vector3 vPos;
				if (Pick(point.x, point.y, &vPos, &ptSelHeightMapPos))
				{
					int ix, iz, lcx, lcz;
					ix = (int)m_vLineLightMap[0].x/TERRAIN_CELL_SIZE;
					iz = (int)m_vLineLightMap[0].z/TERRAIN_CELL_SIZE;
										
					lcx = (int)( ( (m_vLineLightMap[0].x - (float)(ix * TERRAIN_CELL_SIZE)) / (float)TERRAIN_CELL_SIZE ) * (float)(LIGHTMAP_TEX_SIZE-2) );
					lcz = (int)( ( (m_vLineLightMap[0].z - (float)(iz * TERRAIN_CELL_SIZE)) / (float)TERRAIN_CELL_SIZE ) * (float)(LIGHTMAP_TEX_SIZE-2) );

					SetLightMap(ix, iz, lcx, lcz);


					if((vPos.x - m_vLineLightMap[0].x)!=0.0f)
					{
						float a = (vPos.z - m_vLineLightMap[0].z) / (vPos.x - m_vLineLightMap[0].x);
						float b = vPos.z - a * vPos.x;
						int sx, tx, sz, tz;
						sx = (int)(m_vLineLightMap[0].x * 10.0f);
						tx = (int)(vPos.x * 10.0f);
						sz = (int)(m_vLineLightMap[0].z * 10.0f);
						tz = (int)(vPos.z * 10.0f);

						if(sx>tx) { int tmp = sx;	sx = tx;	tx = tmp; }
						if(sz>tz) { int tmp = sz;	sz = tz;	tz = tmp; }

						if((tx-sx)>(tz-sz))
						{
							float rx,rz;
							for(; sx<=tx; sx++)
							{
								rx = (float)sx/10.0f;
								rz = a * rx + b;

								ix = (int)rx/TERRAIN_CELL_SIZE;
								iz = (int)rz/TERRAIN_CELL_SIZE;
													
								lcx = (int)( ( (rx - (float)(ix * TERRAIN_CELL_SIZE)) / (float)TERRAIN_CELL_SIZE ) * (float)(LIGHTMAP_TEX_SIZE-2) );
								lcz = (int)( ( (rz - (float)(iz * TERRAIN_CELL_SIZE)) / (float)TERRAIN_CELL_SIZE ) * (float)(LIGHTMAP_TEX_SIZE-2) );

								SetLightMap(ix, iz, lcx, lcz);
							}
						}
						else
						{
							float rx,rz;
							for(; sz<=tz; sz++)
							{
								rz = (float)sz/10.0f;
								//rz = a * rx + b;
								if(a!=0) rx = (rz-b)/a;
								else rx = 0.0f;
								
								ix = (int)rx/TERRAIN_CELL_SIZE;
								iz = (int)rz/TERRAIN_CELL_SIZE;
													
								lcx = (int)( ( (rx - (float)(ix * TERRAIN_CELL_SIZE)) / (float)TERRAIN_CELL_SIZE ) * (float)(LIGHTMAP_TEX_SIZE-2) );
								lcz = (int)( ( (rz - (float)(iz * TERRAIN_CELL_SIZE)) / (float)TERRAIN_CELL_SIZE ) * (float)(LIGHTMAP_TEX_SIZE-2) );

								SetLightMap(ix, iz, lcx, lcz);
							}
						}
					}
					m_vLineLightMap[0].Set(vPos.x, vPos.y+0.1f, vPos.z, 0xffff40ff);
					m_vLineLightMap[1] = m_vLineLightMap[0];
				}
			}
		}
	case WM_RBUTTONDOWN:
		{
			if (bEditDrag && m_iEditMode == TEM_HEIGHTEN)
			{
				ReleaseCapture();
				bEditDrag = FALSE;
				Heighten(ptSelHeightMapPos, -iSumOfEditedHeight*fDelta);
				UpdateBrushArea(ptSelHeightMapPos);
			}

			if(m_iEditMode == TEM_LIGHTMAP)
			{
				POINT point = {short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam))};
				__Vector3 vPos;
				if (Pick(point.x, point.y, &vPos, &ptSelHeightMapPos))
				{
					m_bDrawLineLightMap = true;
					m_vLineLightMap[0].Set(vPos.x, vPos.y+0.1f, vPos.z, 0xffff40ff);
					m_vLineLightMap[1] = m_vLineLightMap[0];
				}
			}

			if(m_iEditMode == TEM_DTEX)
			{
				POINT point = {short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam))};
				CDlgModifyDTex dlg;
				dlg.m_Pos = point;
				dlg.DoModal();

				__Vector3 vPos;
				if (Pick(point.x, point.y, &vPos, &ptSelHeightMapPos))
				{
					int ix, iz;
					ix = (int)vPos.x/TERRAIN_CELL_SIZE;
					iz = (int)vPos.z/TERRAIN_CELL_SIZE;

					if(dlg.m_State==1)	//mirror...
					{
						if(m_ppMapData[ix][iz].DTexInfo1.Dir<4)
							m_ppMapData[ix][iz].DTexInfo1.Dir+=4;
						else m_ppMapData[ix][iz].DTexInfo1.Dir-=4;

						if(m_ppMapData[ix][iz].DTexInfo2.Dir<4)
							m_ppMapData[ix][iz].DTexInfo2.Dir+=4;
						else m_ppMapData[ix][iz].DTexInfo2.Dir-=4;
					}
					if(dlg.m_State==2)	//cw 90d...
					{
						m_ppMapData[ix][iz].DTexInfo1.Dir++;
						m_ppMapData[ix][iz].DTexInfo2.Dir++;

						if(m_ppMapData[ix][iz].DTexInfo1.Dir==4) m_ppMapData[ix][iz].DTexInfo1.Dir = 0;
						if(m_ppMapData[ix][iz].DTexInfo2.Dir==4) m_ppMapData[ix][iz].DTexInfo2.Dir = 0;

						if(m_ppMapData[ix][iz].DTexInfo1.Dir>=8) m_ppMapData[ix][iz].DTexInfo1.Dir = 4;
						if(m_ppMapData[ix][iz].DTexInfo2.Dir>=8) m_ppMapData[ix][iz].DTexInfo2.Dir = 4;
					}
					if(dlg.m_State==3)	//cw 180d..
					{
						m_ppMapData[ix][iz].DTexInfo1.Dir+=2;
						m_ppMapData[ix][iz].DTexInfo2.Dir+=2;

						if(m_ppMapData[ix][iz].DTexInfo1.Dir==4) m_ppMapData[ix][iz].DTexInfo1.Dir = 0;
						if(m_ppMapData[ix][iz].DTexInfo2.Dir==4) m_ppMapData[ix][iz].DTexInfo2.Dir = 0;

						if(m_ppMapData[ix][iz].DTexInfo1.Dir==5) m_ppMapData[ix][iz].DTexInfo1.Dir = 1;
						if(m_ppMapData[ix][iz].DTexInfo2.Dir==5) m_ppMapData[ix][iz].DTexInfo2.Dir = 1;

						if(m_ppMapData[ix][iz].DTexInfo1.Dir==8) m_ppMapData[ix][iz].DTexInfo1.Dir = 4;
						if(m_ppMapData[ix][iz].DTexInfo2.Dir==8) m_ppMapData[ix][iz].DTexInfo2.Dir = 4;

						if(m_ppMapData[ix][iz].DTexInfo1.Dir==9) m_ppMapData[ix][iz].DTexInfo1.Dir = 5;
						if(m_ppMapData[ix][iz].DTexInfo2.Dir==9) m_ppMapData[ix][iz].DTexInfo2.Dir = 5;
					}
					if(dlg.m_State==4)	//cw 270...
					{
						m_ppMapData[ix][iz].DTexInfo1.Dir--;
						m_ppMapData[ix][iz].DTexInfo2.Dir--;

						if(m_ppMapData[ix][iz].DTexInfo1.Dir==3) m_ppMapData[ix][iz].DTexInfo1.Dir = 7;
						if(m_ppMapData[ix][iz].DTexInfo2.Dir==3) m_ppMapData[ix][iz].DTexInfo2.Dir = 7;

						if(m_ppMapData[ix][iz].DTexInfo1.Dir==-1) m_ppMapData[ix][iz].DTexInfo1.Dir = 3;
						if(m_ppMapData[ix][iz].DTexInfo2.Dir==-1) m_ppMapData[ix][iz].DTexInfo2.Dir = 3;
					}
				}
			}
			return TRUE;
		}
		break;
	}
	return FALSE;
}


//
// Terrain Picking
// vec : 지형에서 피킹된곳의 실좌표
// pHeightMapPos : 피킹된 곳에서 가장 가까운 높이맵의 좌표
bool CLyTerrain::Pick(int x, int y, __Vector3* vec, POINT* pHeightMapPos)
{
	__Vector3 vec2, vec3;		// vec1 & vec2 is 2D..  vec3 & vec4 is 3D..
	CRect rect;
	D3DVIEWPORT9 vp = CN3Base::s_CameraData.vp;
	rect.SetRect(vp.X, vp.Y, vp.X+vp.Width, vp.Y+vp.Height);

	if(x < rect.left || x>=rect.right ) return false;
	if(y < rect.top || y>=rect.bottom ) return false;
	
	__Matrix44 mtx; mtx.Identity();
	CN3Base::s_lpD3DDev->GetTransform( D3DTS_PROJECTION, &mtx );		// Perspective Division..

	__Vector3 vect;
	vect.x =  ( ( ( 2.0f * x ) / rect.Width()  ) - 1 ) / mtx._11;
	vect.y = -( ( ( 2.0f * y ) / rect.Height() ) - 1 ) / mtx._22;
	vect.z = 1.0f;

	BOOL boo = FALSE;
    // Get the inverse view matrix
    D3DXMATRIX matView, m; D3DXVECTOR3 vOrig, vDir;
	CN3Base::s_lpD3DDev->GetTransform( D3DTS_VIEW, &matView );		// 내 맘대로 되라..!! 얍..~~
    D3DXMatrixInverse( &m, nullptr, &matView );

    // Transform the screen space pick ray into 3D space
    vDir.x  = vect.x*m._11 + vect.y*m._21 + vect.z*m._31;
    vDir.y  = vect.x*m._12 + vect.y*m._22 + vect.z*m._32;
    vDir.z  = vect.x*m._13 + vect.y*m._23 + vect.z*m._33;
    vOrig.x = m._41;
    vOrig.y = m._42;
    vOrig.z = m._43;

	__Vector3 A, B, C, AB, AC;
	float ftx, fty, ftz;
	vec2 = vOrig;
	vec3 = vDir;

	while ( (vec2.x >= 0.0f ) && ( vec2.x <= (m_iHeightMapSize-2)*TERRAIN_CELL_SIZE ) && ( vec2.z >= 0.0f ) && ( vec2.z <= (m_iHeightMapSize-2)*TERRAIN_CELL_SIZE ) )
	{
		int ix = ((int)vec2.x)/TERRAIN_CELL_SIZE;
		int iz = ((int)vec2.z)/TERRAIN_CELL_SIZE;

		if ( boo == TRUE ) break;

		for ( int i = 0; i < 10; i++ )
		{
			switch( i )
			{
				case 0:			//  0, 0
					break;
				case 1:
					ix--;		// -1, 0
					break;
				case 2:			//  0, -1
					ix++;
					iz--;
					break;
				case 3:			// -1, -1
					ix--;
					break;
				case 4:			//  1, -1
					ix++;
					ix++;
					break;
				case 5:			// -1, 1
					ix--;
					ix--;
					iz++;
					iz++;
					break;
				case 6:			//  0, 1
					ix++;
					break;
				case 7:			//  1, 1		
					ix++;
					break;
				case 8:			//  1, 0
					iz--;
					break;
				case 9:			//  0, 0
					ix--;
					break;
			};

			if(ix<0) continue;
			if(iz<0) continue;
			if(ix>=m_iHeightMapSize-1) continue;
			if(iz>=m_iHeightMapSize-1) continue;


			if((ix+iz)%2==1)				// 당근.. 왼손 바인딩...
			{
				A.Set((float)ix*TERRAIN_CELL_SIZE, m_ppMapData[ix][iz].fHeight, (float)iz*TERRAIN_CELL_SIZE);
				C.Set((float)(ix+1)*TERRAIN_CELL_SIZE, m_ppMapData[ix+1][iz].fHeight, (float)iz*TERRAIN_CELL_SIZE);
				B.Set((float)ix*TERRAIN_CELL_SIZE, m_ppMapData[ix][iz+1].fHeight, (float)(iz+1)*TERRAIN_CELL_SIZE);

				boo = IntersectTriangle( vOrig, vDir, A, B, C, &ftx, &fty, &ftz );
				if ( boo == TRUE ) 
				{
					if (vec) *vec = vOrig + ftx*vDir;
					if (pHeightMapPos)
					{
						if (fty>0.5f) { pHeightMapPos->x=ix+1;	pHeightMapPos->y=iz; }
						else if(ftz>0.5f) { pHeightMapPos->x=ix;	pHeightMapPos->y=iz+1; }
						else { pHeightMapPos->x=ix;	pHeightMapPos->y=iz; }
					}
					return true;
				}

				A.Set((float)(ix+1)*TERRAIN_CELL_SIZE, m_ppMapData[ix+1][iz+1].fHeight, (float)(iz+1)*TERRAIN_CELL_SIZE);
				B.Set((float)(ix+1)*TERRAIN_CELL_SIZE, m_ppMapData[ix+1][iz].fHeight, (float)iz*TERRAIN_CELL_SIZE);
				C.Set((float)ix*TERRAIN_CELL_SIZE, m_ppMapData[ix][iz+1].fHeight, (float)(iz+1)*TERRAIN_CELL_SIZE);

				boo = IntersectTriangle( vOrig, vDir, A, B, C, &ftx, &fty, &ftz );
				if ( boo == TRUE )
				{
					if (vec) *vec = vOrig + ftx*vDir;
					if (pHeightMapPos)
					{
						if (fty>0.5f) { pHeightMapPos->x=ix+1;	pHeightMapPos->y=iz; }
						else if(ftz>0.5f) { pHeightMapPos->x=ix;	pHeightMapPos->y=iz+1; }
						else { pHeightMapPos->x=ix+1;	pHeightMapPos->y=iz+1; }
					}
					return true;
				}
			}
			else
			{
				A.Set((float)ix*TERRAIN_CELL_SIZE, m_ppMapData[ix][iz+1].fHeight, (float)(iz+1)*TERRAIN_CELL_SIZE);
				C.Set((float)ix*TERRAIN_CELL_SIZE, m_ppMapData[ix][iz].fHeight, (float)iz*TERRAIN_CELL_SIZE);
				B.Set((float)(ix+1)*TERRAIN_CELL_SIZE, m_ppMapData[ix+1][iz+1].fHeight, (float)(iz+1)*TERRAIN_CELL_SIZE);

				boo = IntersectTriangle( vOrig, vDir, A, B, C, &ftx, &fty, &ftz );
				if ( boo == TRUE )
				{
					if (vec) *vec = vOrig + ftx*vDir;
					if (pHeightMapPos)
					{
						if (fty>0.5f) { pHeightMapPos->x=ix;	pHeightMapPos->y=iz; }
						else if(ftz>0.5f) { pHeightMapPos->x=ix+1;	pHeightMapPos->y=iz+1; }
						else { pHeightMapPos->x=ix;	pHeightMapPos->y=iz+1; }
					}
					return true;
				}

				A.Set((float)(ix+1)*TERRAIN_CELL_SIZE, m_ppMapData[ix+1][iz].fHeight, (float)iz*TERRAIN_CELL_SIZE);
				B.Set((float)ix*TERRAIN_CELL_SIZE, m_ppMapData[ix][iz].fHeight, (float)iz*TERRAIN_CELL_SIZE);
				C.Set((float)(ix+1)*TERRAIN_CELL_SIZE, m_ppMapData[ix+1][iz+1].fHeight, (float)(iz+1)*TERRAIN_CELL_SIZE);

				boo = IntersectTriangle( vOrig, vDir, A, B, C, &ftx, &fty, &ftz );
				if ( boo == TRUE )
				{
					if (vec) *vec = vOrig + ftx*vDir;
					if (pHeightMapPos)
					{
						if (fty>0.5f) { pHeightMapPos->x=ix;	pHeightMapPos->y=iz; }
						else if(ftz>0.5f) { pHeightMapPos->x=ix+1;	pHeightMapPos->y=iz+1; }
						else { pHeightMapPos->x=ix+1;	pHeightMapPos->y=iz; }
					}
					return true;
				}
			}
		}
		vec2 += vec3;
	}

	return false;
}


//
//
//
int CLyTerrain::DetectRealLightMap(int sx, int sz, int range)
{
	int x,z;
	//int i;
	bool bIsEmpty;
	D3DLOCKED_RECT d3dlrt;
	
	int NumLightMap = 0;
	for(x=sx;x<(sx+range);x++)
	{
		for(z=sz;z<(sz+range);z++)
		{
			if(m_ppIsLightMap[x][z]==false) continue;
			if(!m_ppLightMapTexture[x][z]) continue;
			
			m_ppLightMapTexture[x][z]->Get()->LockRect( 0, &d3dlrt, 0, 0 );
			LPDWORD pImg = (LPDWORD)d3dlrt.pBits;

			bIsEmpty = true;

			//외각테두리 있는거..
			for(int bx=0; bx<LIGHTMAP_TEX_SIZE; bx++)
			{
				for(int bz=0; bz<LIGHTMAP_TEX_SIZE; bz++)
				{
					DWORD dwAlpha = pImg[bx + bz*LIGHTMAP_TEX_SIZE];
					dwAlpha = ((dwAlpha)>>24);
					//if(pImg[bx + bz*LIGHTMAP_TEX_SIZE]!=0xffffffff)
					if(dwAlpha!=0x00000000)
					{
						bIsEmpty = false;
						NumLightMap++;
						break;
					}
				}
			}
/*	외각테두리 없는거..
			for(i=0; i<LIGHTMAP_TEX_SIZE*LIGHTMAP_TEX_SIZE; i++)
			{
				if(pImg[i]!=0xffffffff)
				{
					bIsWhite = false;
					NumLightMap++;
					break;
				}
			}
*/
			m_ppLightMapTexture[x][z]->Get()->UnlockRect(0);

			if(bIsEmpty)	//텅빈 라이트맵이었당...-.-++
			{
				m_iNumLightMap--;
				m_ppIsLightMap[x][z] = false;
				m_ppLightMapTexture[x][z]->Release();
				delete m_ppLightMapTexture[x][z];
				m_ppLightMapTexture[x][z] = nullptr;
			}
		}
	}
	if(m_iNumLightMap<0) m_iNumLightMap = 0;
	return NumLightMap;
}



//
//
//
void CLyTerrain::FillLightMap(int x, int z, int lcx, int lcz, DWORD Color, float alpha)
{
	if(x<0 || z<0 || x>=m_iHeightMapSize || z>=m_iHeightMapSize) return;

	D3DLOCKED_RECT d3dlrt;
	if(!m_ppLightMapTexture[x][z])
	{
		m_ppLightMapTexture[x][z] = new CN3Texture;
		m_ppLightMapTexture[x][z]->Create(LIGHTMAP_TEX_SIZE, LIGHTMAP_TEX_SIZE, D3DFMT_A8R8G8B8, 0);
		m_ppLightMapTexture[x][z]->Get()->LockRect( 0, &d3dlrt, 0, 0 );
		memset(d3dlrt.pBits, 0x00, sizeof(DWORD)*LIGHTMAP_TEX_SIZE*LIGHTMAP_TEX_SIZE);
		
		///////////////////////////////////////
		//외각 라인 그리기...
		///////////////////////////////////////

		//왼쪽(LT) 칠하기..
		if((x-1)>=0 && (z+1)<m_iHeightMapSize && m_ppLightMapTexture[x-1][z+1])
		{
			D3DLOCKED_RECT d3dlrtSource;
			m_ppLightMapTexture[x-1][z+1]->Get()->LockRect( 0, &d3dlrtSource, 0, 0 );

			LPDWORD pSource = (LPDWORD)d3dlrtSource.pBits;
			LPDWORD pDest = (LPDWORD)d3dlrt.pBits;
			pDest[0] = pSource[((LIGHTMAP_TEX_SIZE-2)*LIGHTMAP_TEX_SIZE) + (LIGHTMAP_TEX_SIZE-2)];
			m_ppLightMapTexture[x-1][z+1]->Get()->UnlockRect(0);			
		}		
		//왼쪽(LM) 칠하기..
		if((x-1)>=0 && m_ppLightMapTexture[x-1][z])
		{
			D3DLOCKED_RECT d3dlrtSource;
			m_ppLightMapTexture[x-1][z]->Get()->LockRect( 0, &d3dlrtSource, 0, 0 );

			LPDWORD pSource = (LPDWORD)d3dlrtSource.pBits;
			LPDWORD pDest = (LPDWORD)d3dlrt.pBits;
			for(int i=1;i<(LIGHTMAP_TEX_SIZE-1);i++)
			{
				pDest[i*LIGHTMAP_TEX_SIZE] = pSource[((i+1)*LIGHTMAP_TEX_SIZE)-2];
			}
			m_ppLightMapTexture[x-1][z]->Get()->UnlockRect(0);			
		}
		//왼쪽(LB) 칠하기..
		if((x-1)>=0 && (z-1)>=0 && m_ppLightMapTexture[x-1][z-1])
		{
			D3DLOCKED_RECT d3dlrtSource;
			m_ppLightMapTexture[x-1][z-1]->Get()->LockRect( 0, &d3dlrtSource, 0, 0 );

			LPDWORD pSource = (LPDWORD)d3dlrtSource.pBits;
			LPDWORD pDest = (LPDWORD)d3dlrt.pBits;
			pDest[(LIGHTMAP_TEX_SIZE-1)*LIGHTMAP_TEX_SIZE] = pSource[LIGHTMAP_TEX_SIZE + (LIGHTMAP_TEX_SIZE-2)];
			m_ppLightMapTexture[x-1][z-1]->Get()->UnlockRect(0);			
		}
		//아래쪽(CB) 칠하기..
		if((z-1)>=0 && m_ppLightMapTexture[x][z-1])
		{
			
			D3DLOCKED_RECT d3dlrtSource;
			m_ppLightMapTexture[x][z-1]->Get()->LockRect( 0, &d3dlrtSource, 0, 0 );

			LPDWORD pSource = (LPDWORD)d3dlrtSource.pBits;
			LPDWORD pDest = (LPDWORD)d3dlrt.pBits;

			memcpy(&(pDest[LIGHTMAP_TEX_SIZE*(LIGHTMAP_TEX_SIZE-1)+1] ), &(pSource[LIGHTMAP_TEX_SIZE+1]), sizeof(DWORD)*(LIGHTMAP_TEX_SIZE-2));
			m_ppLightMapTexture[x][z-1]->Get()->UnlockRect(0);
		}
		//오른쪽(RT) 칠하기..
		if((x+1)<m_iHeightMapSize && (z+1)<m_iHeightMapSize && m_ppLightMapTexture[x+1][z+1])
		{
			D3DLOCKED_RECT d3dlrtSource;
			m_ppLightMapTexture[x+1][z+1]->Get()->LockRect( 0, &d3dlrtSource, 0, 0 );

			LPDWORD pSource = (LPDWORD)d3dlrtSource.pBits;
			LPDWORD pDest = (LPDWORD)d3dlrt.pBits;
			pDest[LIGHTMAP_TEX_SIZE-1] = pSource[1 + ((LIGHTMAP_TEX_SIZE-2)*LIGHTMAP_TEX_SIZE)];
			m_ppLightMapTexture[x+1][z+1]->Get()->UnlockRect(0);			
		}		
		//오른쪽(RM) 칠하기..
		if((x+1)<m_iHeightMapSize && m_ppLightMapTexture[x+1][z])
		{
			D3DLOCKED_RECT d3dlrtSource;
			m_ppLightMapTexture[x+1][z]->Get()->LockRect( 0, &d3dlrtSource, 0, 0 );

			LPDWORD pSource = (LPDWORD)d3dlrtSource.pBits;
			LPDWORD pDest = (LPDWORD)d3dlrt.pBits;
			for(int i=1;i<(LIGHTMAP_TEX_SIZE-1);i++)
			{
				pDest[((i+1)*LIGHTMAP_TEX_SIZE) - 1] = pSource[(i*LIGHTMAP_TEX_SIZE)+1];
			}
			m_ppLightMapTexture[x+1][z]->Get()->UnlockRect(0);
		}
		//오른쪽(RB) 칠하기..
		if((x+1)<m_iHeightMapSize && (z-1)>=0 && m_ppLightMapTexture[x+1][z-1])
		{
			D3DLOCKED_RECT d3dlrtSource;
			m_ppLightMapTexture[x+1][z-1]->Get()->LockRect( 0, &d3dlrtSource, 0, 0 );

			LPDWORD pSource = (LPDWORD)d3dlrtSource.pBits;
			LPDWORD pDest = (LPDWORD)d3dlrt.pBits;
			pDest[(LIGHTMAP_TEX_SIZE-1)+((LIGHTMAP_TEX_SIZE-1)*LIGHTMAP_TEX_SIZE)] = pSource[1 + LIGHTMAP_TEX_SIZE];
			m_ppLightMapTexture[x+1][z-1]->Get()->UnlockRect(0);			
		}
		//위쪽(CT) 칠하기..
		if((z+1)<m_iHeightMapSize && m_ppLightMapTexture[x][z+1])
		{
			D3DLOCKED_RECT d3dlrtSource;
			m_ppLightMapTexture[x][z+1]->Get()->LockRect( 0, &d3dlrtSource, 0, 0 );

			LPDWORD pSource = (LPDWORD)d3dlrtSource.pBits;
			LPDWORD pDest = (LPDWORD)d3dlrt.pBits;

			memcpy(&(pDest[1]), &(pSource[LIGHTMAP_TEX_SIZE*(LIGHTMAP_TEX_SIZE-2)+1]), sizeof(DWORD)*(LIGHTMAP_TEX_SIZE-2));
			m_ppLightMapTexture[x][z+1]->Get()->UnlockRect(0);
		}

		m_ppLightMapTexture[x][z]->Get()->UnlockRect(0);
	}
	
	if(m_ppIsLightMap[x][z] == false)
	{
		m_ppIsLightMap[x][z] = true;
		m_iNumLightMap++;
	}

	//본격칠하기...
	m_ppLightMapTexture[x][z]->Get()->LockRect( 0, &d3dlrt, 0, 0 );
	LPDWORD pDest = (LPDWORD)d3dlrt.pBits;

	int PixelPos = (lcx+1) + ( ( (LIGHTMAP_TEX_SIZE-2) - lcz - 1 ) + 1 ) * LIGHTMAP_TEX_SIZE;

	DWORD OrgColor = pDest[PixelPos];
	DWORD NewColor = 0x00000000;

	if(m_pDlgSetLightMap->MoreLight())	//옅게 그리기(지우기)
	{
		//if(OrgColor < Color) pDest[lcx + (LIGHTMAP_TEX_SIZE-lcz-1)*LIGHTMAP_TEX_SIZE] = 0xffffffff;
		NewColor = 0x00000000;
	}
	else
	{
		float OrigR, OrigG, OrigB, OrigA, ColorR, ColorG, ColorB;
		DWORD NewR, NewG, NewB;
		
		OrigA = (float)(OrgColor>>24);
		OrigR = (float)((OrgColor<<8)>>24);
		OrigG = (float)((OrgColor<<16)>>24);
		OrigB = (float)((OrgColor<<24)>>24);

		ColorR = (float)((Color<<8)>>24);
		ColorG = (float)((Color<<16)>>24);
		ColorB = (float)((Color<<24)>>24);

		NewR = (DWORD)(OrigR * (1.0f - alpha)) + (ColorR * alpha);
		NewG = (DWORD)(OrigG * (1.0f - alpha)) + (ColorG * alpha);
		NewB = (DWORD)(OrigB * (1.0f - alpha)) + (ColorB * alpha);

		DWORD dwAlpha;
		float NewAlpha = alpha * 255.0f;
		if(OrigA > NewAlpha) dwAlpha = (DWORD)OrigA;
		else dwAlpha = (DWORD)NewAlpha;
		dwAlpha = (dwAlpha<<24);
	
		NewColor = dwAlpha + (NewR<<16) + (NewG<<8) + NewB;
		//NewColor = 0x33000000 + (NewR<<16) + (NewG<<8) + NewB;
	}

	pDest[PixelPos] = NewColor;

	//LT
	if(lcx==0 && (x-1)>=0 && lcz==(LIGHTMAP_TEX_SIZE-3) && (z+1)<m_iHeightMapSize)
	{
		D3DLOCKED_RECT d3dlrtDest;
		if(!m_ppLightMapTexture[x-1][z+1])
		{
			m_ppLightMapTexture[x-1][z+1] = new CN3Texture;
			m_ppLightMapTexture[x-1][z+1]->Create(LIGHTMAP_TEX_SIZE, LIGHTMAP_TEX_SIZE, D3DFMT_A8R8G8B8, 0);
			m_ppLightMapTexture[x-1][z+1]->Get()->LockRect( 0, &d3dlrtDest, 0, 0 );
			memset(d3dlrtDest.pBits, 0x00, sizeof(DWORD)*LIGHTMAP_TEX_SIZE*LIGHTMAP_TEX_SIZE);
			
			m_ppIsLightMap[x-1][z+1] = true;
			m_iNumLightMap++;
		}
		else m_ppLightMapTexture[x-1][z+1]->Get()->LockRect( 0, &d3dlrtDest, 0, 0 );

		LPDWORD pDest = (LPDWORD)d3dlrtDest.pBits;
		pDest[(LIGHTMAP_TEX_SIZE-1) + (LIGHTMAP_TEX_SIZE-1)*LIGHTMAP_TEX_SIZE] = NewColor;
		m_ppLightMapTexture[x-1][z+1]->Get()->UnlockRect(0);
	}
	//LM
	if(lcx==0 && (x-1)>=0)
	{
		D3DLOCKED_RECT d3dlrtDest;
		if(!m_ppLightMapTexture[x-1][z])
		{
			m_ppLightMapTexture[x-1][z] = new CN3Texture;
			m_ppLightMapTexture[x-1][z]->Create(LIGHTMAP_TEX_SIZE, LIGHTMAP_TEX_SIZE, D3DFMT_A8R8G8B8, 0);
			m_ppLightMapTexture[x-1][z]->Get()->LockRect( 0, &d3dlrtDest, 0, 0 );
			memset(d3dlrtDest.pBits, 0x00, sizeof(DWORD)*LIGHTMAP_TEX_SIZE*LIGHTMAP_TEX_SIZE);
			
			m_ppIsLightMap[x-1][z] = true;
			m_iNumLightMap++;
		}
		else m_ppLightMapTexture[x-1][z]->Get()->LockRect( 0, &d3dlrtDest, 0, 0 );

		LPDWORD pDest = (LPDWORD)d3dlrtDest.pBits;
		pDest[PixelPos + (LIGHTMAP_TEX_SIZE-2)] = NewColor;
		m_ppLightMapTexture[x-1][z]->Get()->UnlockRect(0);
	}
	//LB
	if(lcx==0 && (x-1)>=0 && lcz==0 && (z-1)>=0)
	{
		D3DLOCKED_RECT d3dlrtDest;
		if(!m_ppLightMapTexture[x-1][z-1])
		{
			m_ppLightMapTexture[x-1][z-1] = new CN3Texture;
			m_ppLightMapTexture[x-1][z-1]->Create(LIGHTMAP_TEX_SIZE, LIGHTMAP_TEX_SIZE, D3DFMT_A8R8G8B8, 0);
			m_ppLightMapTexture[x-1][z-1]->Get()->LockRect( 0, &d3dlrtDest, 0, 0 );
			memset(d3dlrtDest.pBits, 0x00, sizeof(DWORD)*LIGHTMAP_TEX_SIZE*LIGHTMAP_TEX_SIZE);

			m_ppIsLightMap[x-1][z-1] = true;
			m_iNumLightMap++;
		}
		else m_ppLightMapTexture[x-1][z-1]->Get()->LockRect( 0, &d3dlrtDest, 0, 0 );

		LPDWORD pDest = (LPDWORD)d3dlrtDest.pBits;
		pDest[LIGHTMAP_TEX_SIZE-1] = NewColor;
		m_ppLightMapTexture[x-1][z-1]->Get()->UnlockRect(0);
	}
	//RT
	if(lcx==(LIGHTMAP_TEX_SIZE-3) && (x+1)<m_iHeightMapSize && lcz==(LIGHTMAP_TEX_SIZE-3) && (z+1)<m_iHeightMapSize)
	{
		D3DLOCKED_RECT d3dlrtDest;
		if(!m_ppLightMapTexture[x+1][z+1])
		{
			m_ppLightMapTexture[x+1][z+1] = new CN3Texture;
			m_ppLightMapTexture[x+1][z+1]->Create(LIGHTMAP_TEX_SIZE, LIGHTMAP_TEX_SIZE, D3DFMT_A8R8G8B8, 0);
			m_ppLightMapTexture[x+1][z+1]->Get()->LockRect( 0, &d3dlrtDest, 0, 0 );
			memset(d3dlrtDest.pBits, 0x00, sizeof(DWORD)*LIGHTMAP_TEX_SIZE*LIGHTMAP_TEX_SIZE);

			m_ppIsLightMap[x+1][z+1] = true;
			m_iNumLightMap++;
		}
		else m_ppLightMapTexture[x+1][z+1]->Get()->LockRect( 0, &d3dlrtDest, 0, 0 );

		LPDWORD pDest = (LPDWORD)d3dlrtDest.pBits;
		pDest[(LIGHTMAP_TEX_SIZE-1)*LIGHTMAP_TEX_SIZE] = NewColor;
		m_ppLightMapTexture[x+1][z+1]->Get()->UnlockRect(0);
	}
	//RM
	if(lcx==(LIGHTMAP_TEX_SIZE-3) && (x+1)<m_iHeightMapSize)
	{
		D3DLOCKED_RECT d3dlrtDest;
		if(!m_ppLightMapTexture[x+1][z])
		{
			m_ppLightMapTexture[x+1][z] = new CN3Texture;
			m_ppLightMapTexture[x+1][z]->Create(LIGHTMAP_TEX_SIZE, LIGHTMAP_TEX_SIZE, D3DFMT_A8R8G8B8, 0);
			m_ppLightMapTexture[x+1][z]->Get()->LockRect( 0, &d3dlrtDest, 0, 0 );
			memset(d3dlrtDest.pBits, 0x00, sizeof(DWORD)*LIGHTMAP_TEX_SIZE*LIGHTMAP_TEX_SIZE);

			m_ppIsLightMap[x+1][z] = true;
			m_iNumLightMap++;
		}
		else m_ppLightMapTexture[x+1][z]->Get()->LockRect( 0, &d3dlrtDest, 0, 0 );

		LPDWORD pDest = (LPDWORD)d3dlrtDest.pBits;
		pDest[PixelPos - (LIGHTMAP_TEX_SIZE-2)] = NewColor;
		m_ppLightMapTexture[x+1][z]->Get()->UnlockRect(0);
	}
	//RB
	if(lcx==(LIGHTMAP_TEX_SIZE-3) && (x+1)<m_iHeightMapSize && lcz==0 && (z-1)>=0)
	{
		D3DLOCKED_RECT d3dlrtDest;
		if(!m_ppLightMapTexture[x+1][z-1])
		{
			m_ppLightMapTexture[x+1][z-1] = new CN3Texture;
			m_ppLightMapTexture[x+1][z-1]->Create(LIGHTMAP_TEX_SIZE, LIGHTMAP_TEX_SIZE, D3DFMT_A8R8G8B8, 0);
			m_ppLightMapTexture[x+1][z-1]->Get()->LockRect( 0, &d3dlrtDest, 0, 0 );
			memset(d3dlrtDest.pBits, 0x00, sizeof(DWORD)*LIGHTMAP_TEX_SIZE*LIGHTMAP_TEX_SIZE);

			m_ppIsLightMap[x+1][z-1] = true;
			m_iNumLightMap++;
		}
		else m_ppLightMapTexture[x+1][z-1]->Get()->LockRect( 0, &d3dlrtDest, 0, 0 );

		LPDWORD pDest = (LPDWORD)d3dlrtDest.pBits;
		pDest[0] = NewColor;
		m_ppLightMapTexture[x+1][z-1]->Get()->UnlockRect(0);
	}
	//CB
	if(lcz==0 && (z-1)>=0)
	{
		D3DLOCKED_RECT d3dlrtDest;
		if(!m_ppLightMapTexture[x][z-1])
		{
			m_ppLightMapTexture[x][z-1] = new CN3Texture;
			m_ppLightMapTexture[x][z-1]->Create(LIGHTMAP_TEX_SIZE, LIGHTMAP_TEX_SIZE, D3DFMT_A8R8G8B8, 0);
			m_ppLightMapTexture[x][z-1]->Get()->LockRect( 0, &d3dlrtDest, 0, 0 );
			memset(d3dlrtDest.pBits, 0x00, sizeof(DWORD)*LIGHTMAP_TEX_SIZE*LIGHTMAP_TEX_SIZE);

			m_ppIsLightMap[x][z-1] = true;
			m_iNumLightMap++;
		}
		else m_ppLightMapTexture[x][z-1]->Get()->LockRect( 0, &d3dlrtDest, 0, 0 );

		LPDWORD pDest = (LPDWORD)d3dlrtDest.pBits;
		pDest[(lcx+1)] = NewColor;
		m_ppLightMapTexture[x][z-1]->Get()->UnlockRect(0);
	}
	//CT
	if(lcz==(LIGHTMAP_TEX_SIZE-3) && (z+1)<m_iHeightMapSize)
	{
		D3DLOCKED_RECT d3dlrtDest;
		if(!m_ppLightMapTexture[x][z+1])
		{
			m_ppLightMapTexture[x][z+1] = new CN3Texture;
			m_ppLightMapTexture[x][z+1]->Create(LIGHTMAP_TEX_SIZE, LIGHTMAP_TEX_SIZE, D3DFMT_A8R8G8B8, 0);
			m_ppLightMapTexture[x][z+1]->Get()->LockRect( 0, &d3dlrtDest, 0, 0 );
			memset(d3dlrtDest.pBits, 0x00, sizeof(DWORD)*LIGHTMAP_TEX_SIZE*LIGHTMAP_TEX_SIZE);

			m_ppIsLightMap[x][z+1] = true;
			m_iNumLightMap++;
		}
		else m_ppLightMapTexture[x][z+1]->Get()->LockRect( 0, &d3dlrtDest, 0, 0 );
			 
		LPDWORD pDest = (LPDWORD)d3dlrtDest.pBits;
		pDest[(lcx+1) + (LIGHTMAP_TEX_SIZE-1)*LIGHTMAP_TEX_SIZE] = NewColor;
		m_ppLightMapTexture[x][z+1]->Get()->UnlockRect(0);
	}
	
	m_ppLightMapTexture[x][z]->Get()->UnlockRect(0);
}


//
//	lcx, lcz는 라이트맵타일내에서의 좌표..
//
void CLyTerrain::SetLightMap(int x, int z, int lcx, int lcz)
{
	DWORD SrcColor;
	int PenSize, Hardness;
	PenSize = m_pDlgSetLightMap->GetPenSize();		//1~(LIGHTMAP_TEX_SIZE-2)
	Hardness = m_pDlgSetLightMap->GetHardness();	//1~100
	SrcColor = m_pDlgSetLightMap->GetColor();		//전체적인 컬러.

	int ix, iz;
	int dist;
	int nx, nz, nlx, nlz;
	int LimitFullColor = ( ((float)Hardness/100.0f) * (float)PenSize );	//중심에서 풀컬러로 채워지는 거리..
	for(ix=lcx-PenSize+1; ix<lcx+PenSize; ix++)
	{
		for(iz=lcz-PenSize+1; iz<lcz+PenSize; iz++)
		{
			dist = m_iDistanceTable[std::abs(lcx-ix)][std::abs(lcz-iz)] + 1;
			if(dist > PenSize) continue;
			
			nx = x;
			nz = z;
			nlx = ix;
			nlz = iz;

			if(ix<0)
			{
				nx = x-1;
				nlx = (LIGHTMAP_TEX_SIZE-2) + ix;
			}
			if(ix>=(LIGHTMAP_TEX_SIZE-2))
			{
				nx = x+1;
				nlx = ix - (LIGHTMAP_TEX_SIZE-2);
			}
			if(iz<0)
			{
				nz = z-1;
				nlz = (LIGHTMAP_TEX_SIZE-2) + iz;
			}
			if(iz>=(LIGHTMAP_TEX_SIZE-2))
			{
				nz = z+1;
				nlz = iz - (LIGHTMAP_TEX_SIZE-2);
			}

			float alpha = 0.0f;
			if(dist<=LimitFullColor || (PenSize <= LimitFullColor)) alpha = 1.0f;
			else alpha = 1.0f - (float)((float)(dist - LimitFullColor) / (float)(PenSize - LimitFullColor));

			alpha *= m_pDlgSetLightMap->GetAlpha();

			FillLightMap(nx, nz, nlx, nlz, SrcColor, alpha);			
		}
	}	
}


//
//
//	MakeDistanceTable
//	거리를 계산하지 말고 테이블에서 가져올 수 있게 미리 테이블 생성..
//	정수 단위 거리..
//
void CLyTerrain::MakeDistanceTable()
{
	int x,z;
	double dist;
	for(x=0;x<DISTANCE_TABLE_SIZE;x++)
	{
		for(z=0;z<DISTANCE_TABLE_SIZE;z++)
		{
			dist = sqrt( (double)((x*x) + (z*z)) ) + 0.6;
			m_iDistanceTable[x][z] = (int)dist;
		}
	}
}


//
//	지형에서 갈수 있는 타일과 갈 수 없는 타일 구분해서 테이블 만드는 함수.
//	char형으로 1이면 갈 수 있고, 0이면 갈 수 없다.
//
void CLyTerrain::MakeMoveTable(short** ppEvent)
{
	int x,z;
	float fMax, fMin;
	for(x=0; x<m_iHeightMapSize-1; x++)
	{
		for(z=0; z<m_iHeightMapSize-1; z++)
		{
			fMax = -FLT_MAX;
			fMin = FLT_MAX;
			
			if(fMax < m_ppMapData[x][z].fHeight) fMax = m_ppMapData[x][z].fHeight;
			if(fMin > m_ppMapData[x][z].fHeight) fMin = m_ppMapData[x][z].fHeight;

			if(fMax < m_ppMapData[x][z+1].fHeight) fMax = m_ppMapData[x][z+1].fHeight;
			if(fMin > m_ppMapData[x][z+1].fHeight) fMin = m_ppMapData[x][z+1].fHeight;

			if(fMax < m_ppMapData[x+1][z].fHeight) fMax = m_ppMapData[x+1][z].fHeight;
			if(fMin > m_ppMapData[x+1][z].fHeight) fMin = m_ppMapData[x+1][z].fHeight;

			if(fMax < m_ppMapData[x+1][z+1].fHeight) fMax = m_ppMapData[x+1][z+1].fHeight;
			if(fMin > m_ppMapData[x+1][z+1].fHeight) fMin = m_ppMapData[x+1][z+1].fHeight;

			if( NOTMOVE_HEIGHT <= std::abs((int)(fMax-fMin)) )
			{
				ppEvent[x][z] = 0;
			}
		}
	}
}


//
//	SaveServerData(HANDLE hFile)
//
void CLyTerrain::SaveServerData(HANDLE hFile)
{
	DWORD dwRWC;
	WriteFile(hFile, &m_iHeightMapSize, sizeof(int), &dwRWC, nullptr);

	float UnitDist = (float)TERRAIN_CELL_SIZE;
	WriteFile(hFile, &UnitDist, sizeof(float), &dwRWC, nullptr);

	int x, z;

	//  그냥 하나씩 저장해도 차이 없지만 네트워크로 저장할때는 파일 엑세스 숫자를 줄이고 한꺼번에 저장해야  빠르다.. ...
	DWORD dwAlloc = m_iHeightMapSize * m_iHeightMapSize * sizeof(float);
	HGLOBAL hAlloc = ::GlobalAlloc(GMEM_FIXED, dwAlloc);
	__ASSERT(hAlloc, "Global allocation failed.");
	float* pfHeights = (float*)::GlobalLock(hAlloc);

	CProgressBar ProgressBar;
	ProgressBar.Create("Save terrain data..", 50, m_iHeightMapSize);
	for(x=0;x<m_iHeightMapSize;x++)
	{
		ProgressBar.StepIt();
		for(z=0;z<m_iHeightMapSize;z++)
		{
			pfHeights[x*m_iHeightMapSize+z] = m_ppMapData[x][z].fHeight;
		}
	}
	WriteFile(hFile, pfHeights, dwAlloc, &dwRWC, nullptr); // 파일에 쓴다..
	pfHeights = nullptr;
	::GlobalUnlock(hAlloc);
	::GlobalFree(hAlloc);
}


//
//
//
void CLyTerrain::VerifyTexInfo(LPDTEXINFO pDTex)
{
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	CDTexGroupMng* pDTexGroupMng = pFrm->GetDTexGroupMng(); 
	if(!pDTexGroupMng) return;
	CDTexMng* pDTexMng = pFrm->GetDTexMng();
	if(!pDTexMng) return;

	if(pDTex->Attr.Attr < DTEX_FULL || pDTex->Attr.Attr >= DTEX_MAX) pDTex->Attr.Attr = DTEX_FULL;
//	if(pDTex->Attr.Group < 0 || pDTex->Attr.Group >= pDTexGroupMng->m_Groups.size()) pDTex->Attr.Group = 0;
	if(pDTex->Dir<0 || pDTex->Dir>7) pDTex->Dir = 0;

//	if(pDTex->TexIdx.TexID < 0 || pDTex->TexIdx.TexID > pDTexMng->GetMaxID())
//	{
//		pDTex->TexIdx = pDTexGroupMng->GetTileAttr(pDTex->Attr.Group, pDTex->Attr.Attr);
//	}
}



//
//	SetDTex..
//	자동완성 기능이 있는 것...
//
void CLyTerrain::SetDTex(int x, int y)
{
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	CDTexGroupMng* pDTexGroupMng = pFrm->GetDTexGroupMng(); 
	if(!pDTexGroupMng) return;
	CDTexMng* pDTexMng = pFrm->GetDTexMng();
	if(!pDTexMng) return;
	
	DTEXINFO NewDTex, Dest1DTex, Dest2DTex;
	NewDTex.Attr = pDTexGroupMng->m_SelectedDTex;
	NewDTex.TexIdx = pDTexGroupMng->m_SelectedDTexTile;
	NewDTex.Dir = 0;

	if(NewDTex.Attr.Attr >= DTEX_1PER8 || pDTexGroupMng->m_pGroupView->m_iRadioState>1)
	{
		SetDTexManual(x, y);
		return;
	}
	if(NewDTex.Attr.Attr < DTEX_FULL || NewDTex.Attr.Attr >= DTEX_MAX) return;
	//if(NewDTex.Attr.Group < 0 || (int)NewDTex.Attr.Group >= pDTexGroupMng->m_Groups.size()) return;
	if(NewDTex.TexIdx.TexID < 0 || (int)NewDTex.TexIdx.TexID > pDTexMng->GetMaxID()) return;
	if(NewDTex.TexIdx.TileX < 0 || NewDTex.TexIdx.TileX >= NUM_DTEXTILE) return;
	if(NewDTex.TexIdx.TileY < 0 || NewDTex.TexIdx.TileY >= NUM_DTEXTILE) return;
	
	int SIndex = 0;
	int	TIndex = 0;
		
	int ix, iz;
	int BrushSize = pDTexGroupMng->GetBrushSize();
	if(BrushSize<1 || BrushSize>20) return;

	for(int i=0; i<(BrushSize+2); i++)
	{
		for(int j=0; j<(BrushSize+2); j++)
		{
			ix = x + - 1 - (BrushSize>>1) + j;
			iz = y + - 1 - (BrushSize>>1) + i;

			SIndex = 4;
			if(j==0 && i==0) SIndex = 0;	//LB..
			else if(j==(BrushSize+1) && i==0) SIndex = 2;	//RB..
			else if(j==0 && i==(BrushSize+1)) SIndex = 6;	//LT..
			else if(j==(BrushSize+1) && i==(BrushSize+1)) SIndex = 8;	//RT..
						
			else if(i==0) SIndex = 1;	//MB..
			else if(j==0) SIndex = 3;	//LM..
			else if(j==(BrushSize+1)) SIndex = 5;	//RM..			
			else if(i==(BrushSize+1)) SIndex = 7;	//MT..
			
			if(ix<0 || ix>=(m_iHeightMapSize-1))
			{
				SIndex++;
				continue;
			}
			if(iz<0 || iz>=(m_iHeightMapSize-1))
			{
				SIndex++;
				continue;
			}

			Dest1DTex = m_ppMapData[ix][iz].DTexInfo1;
			Dest2DTex = m_ppMapData[ix][iz].DTexInfo2;

			VerifyTexInfo(&Dest1DTex);
			VerifyTexInfo(&Dest2DTex);

			if(NewDTex.Attr.Group==0)
			{
				Dest1DTex.Attr.Group = 0;
				Dest1DTex.Attr.Attr = DTEX_FULL;
				Dest1DTex.Dir = 0;
				Dest1DTex.TexIdx.TexID = -1;
				Dest1DTex.TexIdx.TileX = 0;
				Dest1DTex.TexIdx.TileY = 0;

				Dest2DTex = Dest1DTex;
				continue;
			}

			//
			//
			if(Dest2DTex.Attr.Group == NewDTex.Attr.Group)
			{
				DTEXINFO tmp = Dest1DTex;
				Dest1DTex = Dest2DTex;
				Dest2DTex = tmp;
			}

			if(Dest1DTex.Attr.Group == NewDTex.Attr.Group)
			{
				//int D1Attr = Dest1DTex.Attr.Attr;
#ifdef _KNIGHT
				if(Dest1DTex.Attr.Attr == DTEX_FULL) TIndex = 12;
				if(Dest1DTex.Attr.Attr < DTEX_1PER8 && Dest1DTex.Attr.Attr > DTEX_FULL)
				{
					if(Dest1DTex.Dir<4)	TIndex = ((Dest1DTex.Attr.Attr-1)<<2) + Dest1DTex.Dir;
					else
					{
						TIndex = ((Dest1DTex.Attr.Attr-1)<<2) + ((8 - Dest1DTex.Dir)%4);
					}
				}
#endif
#ifdef _EMPEROR
				switch(Dest1DTex.Attr.Attr)	//	asm
				{
				case DTEX_FULL:  TIndex = 12;	break;
				case DTEX_1PER2: TIndex = 0;	break;
				case DTEX_1PER4: TIndex = Dest1DTex.Dir? 7:4;break;
				case DTEX_3PER4: TIndex = Dest1DTex.Dir? 11:8; break;
				case DTEX_1PER8: TIndex = 2;	break;
				case DTEX_7PER8: TIndex = Dest1DTex.Dir? 1:3; break;
				case DTEX_5PER8: TIndex = Dest1DTex.Dir? 6:5; break;
				case DTEX_3PER8: TIndex = Dest1DTex.Dir? 10:9; break;
				}
#endif

				Dest1DTex = NewDTex;
				Dest1DTex.Attr.Attr = m_DTexAttrTable[SIndex][TIndex];
				Dest1DTex.Dir = m_DTexDirTable[SIndex][TIndex];
				Dest1DTex.TexIdx = pDTexGroupMng->GetTileAttr(Dest1DTex.Attr.Group, Dest1DTex.Attr.Attr);

				SetPertinentTile(pDTexGroupMng, &Dest1DTex, &Dest2DTex);
			}
			else
			{
				if(Dest1DTex.Attr.Group!=0)
				{

					Dest2DTex = Dest1DTex;
				}

				Dest1DTex = NewDTex;
				Dest1DTex.Attr.Attr = m_DTexAttrTable[SIndex][13];
				Dest1DTex.Dir = m_DTexDirTable[SIndex][13];
				Dest1DTex.TexIdx = pDTexGroupMng->GetTileAttr(Dest1DTex.Attr.Group, Dest1DTex.Attr.Attr);

				SetPertinentTile(pDTexGroupMng, &Dest1DTex, &Dest2DTex);
			}

			m_ppMapData[ix][iz].DTexInfo1 = Dest1DTex;
			m_ppMapData[ix][iz].DTexInfo2 = Dest2DTex;
			PutColorMapTile(ix, iz);
			
			//SIndex++;
		}	//end of for(int zz=0;zz<3;zz++)
	}	//end of for(int xx=0;xx<3;xx++)
}


//
//	SetDTexManual..
//	자동완성 기능이 없이 그냥 하나만 찍는 것.....
//
void CLyTerrain::SetDTexManual(int x, int y)
{
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	CDTexGroupMng* pDTexGroupMng = pFrm->GetDTexGroupMng(); 
	if(!pDTexGroupMng) return;
	CDTexMng* pDTexMng = pFrm->GetDTexMng();
	if(!pDTexMng) return;

	DTEXINFO NewDTex;
	NewDTex.Attr = pDTexGroupMng->m_SelectedDTex;
	NewDTex.TexIdx = pDTexGroupMng->m_SelectedDTexTile;
	NewDTex.Dir = 0;

	if(NewDTex.Attr.Attr < DTEX_FULL || NewDTex.Attr.Attr >= DTEX_MAX) return;
	//if(NewDTex.Attr.Group < 0 || (int)NewDTex.Attr.Group >= pDTexGroupMng->m_Groups.size()) return;
	if(NewDTex.TexIdx.TexID < 0 || (int)NewDTex.TexIdx.TexID > pDTexMng->GetMaxID()) return;
	if(NewDTex.TexIdx.TileX < 0 || NewDTex.TexIdx.TileX >= NUM_DTEXTILE) return;
	if(NewDTex.TexIdx.TileY < 0 || NewDTex.TexIdx.TileY >= NUM_DTEXTILE) return;

	if(x<0 || x>=m_iHeightMapSize) return;
	if(y<0 || y>=m_iHeightMapSize) return;
		
	DTEXINFO Dest1DTex, Dest2DTex;
	Dest1DTex = m_ppMapData[x][y].DTexInfo1;
	Dest2DTex = m_ppMapData[x][y].DTexInfo2;

	if(NewDTex.Attr.Group==0)
	{
		m_ppMapData[x][y].DTexInfo1.Attr.Group = 0;
		m_ppMapData[x][y].DTexInfo1.Attr.Attr = DTEX_FULL;
		m_ppMapData[x][y].DTexInfo1.Dir = 0;
		m_ppMapData[x][y].DTexInfo1.TexIdx.TexID = -1;
		m_ppMapData[x][y].DTexInfo1.TexIdx.TileX = 0;
		m_ppMapData[x][y].DTexInfo1.TexIdx.TileY = 0;

		m_ppMapData[x][y].DTexInfo2 = m_ppMapData[x][y].DTexInfo1;		
		return;
	}

//	if(Dest1DTex.Attr.Group == NewDTex.Attr.Group &&
//		Dest1DTex.Attr.Attr == NewDTex.Attr.Attr)
//	{
//		Dest1DTex.Dir++;
//		if(Dest1DTex.Dir>7) Dest1DTex.Dir = 0;
//
//		SetPertinentTile(pDTexGroupMng, &Dest1DTex, &Dest2DTex);
//
//		m_ppMapData[x][y].DTexInfo1 = Dest1DTex;
//		m_ppMapData[x][y].DTexInfo2 = Dest2DTex;
//		PutColorMapTile(x, z);
//		return;
//	}

	if(Dest2DTex.Attr.Group == NewDTex.Attr.Group)
	{
		DTEXINFO tmp = Dest1DTex;
		Dest1DTex = Dest2DTex;
		Dest2DTex = tmp;
	}

	if(Dest1DTex.Attr.Group == NewDTex.Attr.Group)
	{
		Dest1DTex = NewDTex;
		SetPertinentTile(pDTexGroupMng, &Dest1DTex, &Dest2DTex);
	}
	else
	{
		if(Dest1DTex.Attr.Group!=0)
		{
			Dest2DTex = Dest1DTex;
		}

		Dest1DTex = NewDTex;
		SetPertinentTile(pDTexGroupMng, &Dest1DTex, &Dest2DTex);
	}

	if(pDTexGroupMng->m_pGroupView->m_iRadioState==3)
		Dest2DTex.TexIdx.TexID = -1;	//	강제찍기의 풀로찍기
	
	m_ppMapData[x][y].DTexInfo1 = Dest1DTex;
	m_ppMapData[x][y].DTexInfo2 = Dest2DTex;
	PutColorMapTile(x, y);
}


//
//
//
void CLyTerrain::SetPertinentTile(CDTexGroupMng* pDTexGroupMng, LPDTEXINFO pDest1, LPDTEXINFO pDest2)
{
	pDest2->Dir = pDest1->Dir;

#ifdef _KNIGHT
	int dir[8] = { 2, 5, 4, 1, 2, 1, 4, 5 };

	switch(pDest1->Attr.Attr)
	{
	case DTEX_FULL:
		pDest2->Attr.Group = 0;
		pDest2->Attr.Attr = DTEX_FULL;
		break;
	case DTEX_1PER2:
		pDest2->Attr.Attr = DTEX_1PER2;
		pDest2->Dir = dir[pDest1->Dir];							
		break;
	case DTEX_1PER4:
		pDest2->Attr.Attr = DTEX_3PER4;			
		break;
	case DTEX_3PER4:
		pDest2->Attr.Attr = DTEX_1PER4;
		break;
	case DTEX_1PER8:
		pDest2->Attr.Attr = DTEX_7PER8;
		break;
	case DTEX_7PER8:
		pDest2->Attr.Attr = DTEX_1PER8;
		break;
	case DTEX_5PER8:
		pDest2->Attr.Attr = DTEX_3PER8;
		break;
	case DTEX_3PER8:
		pDest2->Attr.Attr = DTEX_5PER8;
		break;
	}
#endif
#ifdef _EMPEROR
	switch(pDest1->Attr.Attr)	//	asm
	{
	case DTEX_FULL:
		pDest2->Attr.Group = 0;
		pDest2->Attr.Attr = DTEX_FULL;
		break;
	case DTEX_1PER2:
		pDest2->Attr.Attr = DTEX_1PER8;
		break;
	case DTEX_1PER4:
		pDest2->Attr.Attr = DTEX_3PER4;			
		break;
	case DTEX_3PER4:
		pDest2->Attr.Attr = DTEX_1PER4;
		break;
	case DTEX_1PER8:
		pDest2->Attr.Attr = DTEX_1PER2;
		break;
	case DTEX_7PER8:
		pDest2->Attr.Attr = DTEX_7PER8;
		pDest2->Dir = pDest1->Dir? 0:4;
		break;
	case DTEX_5PER8:
		pDest2->Attr.Attr = DTEX_3PER8;
		break;
	case DTEX_3PER8:
		pDest2->Attr.Attr = DTEX_5PER8;
		break;
	}
#endif

	if(pDest2->Attr.Group < 0) pDest2->Attr.Group = 0;
	pDest2->TexIdx = pDTexGroupMng->GetTileAttr(pDest2->Attr.Group, pDest2->Attr.Attr);
} 


//
//
//
void CLyTerrain::TilingAll()
{
	int x,z;
	CProgressBar ProgressBar;
	ProgressBar.Create("Tiling All..", 50, (m_iHeightMapSize));
	ProgressBar.SetStep(1);
	for(z=0;z<m_iHeightMapSize;z++)
	{
		for(x=0;x<m_iHeightMapSize;x++)
		{
			SetDTexManual(x, z);				
		}
		ProgressBar.StepIt();
	}
	GeneraterColorMap(true);
}


//
//	컬러맵 만들 타일 넣기...
//	아직 안만들어진...그런 타일들 대기열에 넣는것..
//
void CLyTerrain::PutColorMapTile(int x, int z)
{
	auto it = m_ColorMapTileTree.lower_bound((short) x);
	size_t count = m_ColorMapTileTree.count((short) x);

	for (size_t i = 0; i < count; i++)
	{
		if ((*it).second == (short) z)
			return;
		it++;
	}
	m_ColorMapTileTree.insert(MMValue((short) x, (short) z));
}


//
//	컬러맵 만들지 않은 타일들 컬러맵 만들기..
//
void CLyTerrain::GeneraterColorMap(bool bIsAll)
{
	CProgressBar ProgressBar;
	
	if(bIsAll)
	{
		ProgressBar.Create("Generate ColorMap..", 50, (m_iHeightMapSize-1));
		ProgressBar.SetStep(1);

		int x,z;
		for(z=0;z<m_iHeightMapSize-1;z++)
		{
			for(x=0;x<m_iHeightMapSize-1;x++)
			{
				SetColorMap(x,z);				
			}
			ProgressBar.StepIt();
		}
		m_ColorMapTileTree.clear();
		return;
	}

	ProgressBar.Create("Generate ColorMap..", 50, static_cast<int>(m_ColorMapTileTree.size()));
	ProgressBar.SetStep(1);
	
	MMIter it;
	for(it = m_ColorMapTileTree.begin(); it != m_ColorMapTileTree.end(); it++)
	{
		SetColorMap((*it).first,(*it).second);
		ProgressBar.StepIt();
	}
	m_ColorMapTileTree.clear();
}

//
//	컬러맵텍스쳐 셋팅하기..
//
//
//	타일맵을 그대로 축소시켜서 컬러맵을 만드는 방법..
//
/*
void CLyTerrain::SetColorMap(int x, int y)
{
	float u1,v1,u2,v2;
	float TileTexSize = (float)(DTEX_SIZE/NUM_DTEXTILE);
	float fEndPoint = TileTexSize - 1;

	HRESULT hr;

	if(m_ppMapData[x][y].DTexInfo1.Attr.Group <=0 && 
		m_ppMapData[x][y].DTexInfo2.Attr.Group <=0) return;

	int dir1, dir2;
	dir1 = m_ppMapData[x][y].DTexInfo1.Dir;
	dir2 = m_ppMapData[x][y].DTexInfo2.Dir;

	u1 = (m_ppMapData[x][y].DTexInfo1.TexIdx.TileX * TileTexSize);
	v1 = (m_ppMapData[x][y].DTexInfo1.TexIdx.TileY * TileTexSize);

	u2 = (m_ppMapData[x][y].DTexInfo2.TexIdx.TileX * TileTexSize);
	v2 = (m_ppMapData[x][y].DTexInfo2.TexIdx.TileY * TileTexSize);

	hr = m_pColorMapTmpVB->Lock( 0, 0, (BYTE**)&m_pColorMapTmpVertices, 0 );

	m_pColorMapTmpVertices[0].Set(0.0f,		0.0f,		0.1f, 0.5f, 0xffffffff,
								(u1+m_fTileDirUforColorMap[dir1][0])/(float)(DTEX_SIZE),
								(v1+m_fTileDirVforColorMap[dir1][0])/(float)(DTEX_SIZE),
								(u2+m_fTileDirUforColorMap[dir2][0])/(float)(DTEX_SIZE),
								(v2+m_fTileDirVforColorMap[dir2][0])/(float)(DTEX_SIZE));
	m_pColorMapTmpVertices[1].Set(fEndPoint,0.0f,		0.1f, 0.5f, 0xffffffff,
								(u1+m_fTileDirUforColorMap[dir1][1])/(float)(DTEX_SIZE),
								(v1+m_fTileDirVforColorMap[dir1][1])/(float)(DTEX_SIZE),
								(u2+m_fTileDirUforColorMap[dir2][1])/(float)(DTEX_SIZE),
								(v2+m_fTileDirVforColorMap[dir2][1])/(float)(DTEX_SIZE));
	m_pColorMapTmpVertices[2].Set(fEndPoint,fEndPoint,	0.1f, 0.5f, 0xffffffff,
								(u1+m_fTileDirUforColorMap[dir1][3])/(float)(DTEX_SIZE),
								(v1+m_fTileDirVforColorMap[dir1][3])/(float)(DTEX_SIZE),
								(u2+m_fTileDirUforColorMap[dir2][3])/(float)(DTEX_SIZE),
								(v2+m_fTileDirVforColorMap[dir2][3])/(float)(DTEX_SIZE));								
	m_pColorMapTmpVertices[3].Set(0.0f,		fEndPoint,	0.1f, 0.5f, 0xffffffff,
								(u1+m_fTileDirUforColorMap[dir1][2])/(float)(DTEX_SIZE),
								(v1+m_fTileDirVforColorMap[dir1][2])/(float)(DTEX_SIZE),
								(u2+m_fTileDirUforColorMap[dir2][2])/(float)(DTEX_SIZE),
								(v2+m_fTileDirVforColorMap[dir2][2])/(float)(DTEX_SIZE));

	if(m_ppIsLightMap[x][y])
	{
		m_pColorMapTmpVertices[4].Set(0.0f,		0.0f,		0.1f, 0.5f, 0xffffffff,
								(1.0f / (float)LIGHTMAP_TEX_SIZE),
								(1.0f / (float)LIGHTMAP_TEX_SIZE),
								0.0f, 0.0f);
		m_pColorMapTmpVertices[5].Set(fEndPoint,0.0f,		0.1f, 0.5f, 0xffffffff,
								1.0f,
								(1.0f / (float)LIGHTMAP_TEX_SIZE),
								0.0f, 0.0f);
		m_pColorMapTmpVertices[6].Set(fEndPoint,fEndPoint,	0.1f, 0.5f, 0xffffffff,
								1.0f,
								1.0f,
								0.0f, 0.0f);
		m_pColorMapTmpVertices[7].Set(0.0f,		fEndPoint,	0.1f, 0.5f, 0xffffffff,
								(1.0f / (float)LIGHTMAP_TEX_SIZE),
								1.0f,
								0.0f, 0.0f);
	}
	
	hr = m_pColorMapTmpVB->Unlock();
	m_pColorMapTmpVertices = nullptr;

	hr = s_lpD3DDev->Clear(0, nullptr, D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET, D3DCOLOR_ARGB(255,0,0,0), 1.0f, 0);
	hr = s_lpD3DDev->BeginScene();

	DWORD dwCull, dwZ, dwLgt;
	hr = s_lpD3DDev->GetRenderState(D3DRS_CULLMODE, &dwCull);
	hr = s_lpD3DDev->GetRenderState(D3DRS_ZENABLE, &dwZ);
	hr = s_lpD3DDev->GetRenderState(D3DRS_LIGHTING, &dwLgt);

	hr = s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	hr = s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	hr = s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);

	hr = s_lpD3DDev->SetStreamSource( 0, m_pColorMapTmpVB, sizeof(__VertexTransformedT2) );
	hr = s_lpD3DDev->SetVertexShader( FVF_TRANSFORMEDT2 );
	
	DWORD ColorArg11, ColorArg12, ColorArg21, ColorArg22;
	DWORD ColorOP1, ColorOP2;

	hr = s_lpD3DDev->GetTextureStageState( 0, D3DTSS_COLOROP, &ColorOP1);
	hr = s_lpD3DDev->GetTextureStageState( 0, D3DTSS_COLORARG1, &ColorArg11);
	hr = s_lpD3DDev->GetTextureStageState( 0, D3DTSS_COLORARG2, &ColorArg12);

	hr = s_lpD3DDev->GetTextureStageState( 1, D3DTSS_COLOROP, &ColorOP2);
	hr = s_lpD3DDev->GetTextureStageState( 1, D3DTSS_COLORARG1, &ColorArg21);
	hr = s_lpD3DDev->GetTextureStageState( 1, D3DTSS_COLORARG2, &ColorArg22);

	hr = s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	hr = s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	hr = s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	int t1 = m_ppMapData[x][y].DTexInfo1.TexIdx.TexID;
	int t2 = m_ppMapData[x][y].DTexInfo2.TexIdx.TexID;

	if(m_ppMapData[x][y].DTexInfo1.Attr.Group>0 && GetTileTex(t1))
		hr = s_lpD3DDev->SetTexture( 0, GetTileTex(t1)->Get());
	else hr = s_lpD3DDev->SetTexture( 0, nullptr);

	
	if(m_ppMapData[x][y].DTexInfo2.Attr.Group>0 && GetTileTex(t2))
	{
		hr = s_lpD3DDev->SetTexture( 1, GetTileTex(t2)->Get());
		hr = s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_ADD);
		hr = s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT);
		hr = s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	}
	else
	{
		hr = s_lpD3DDev->SetTexture( 1, nullptr);
		hr = s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		hr = s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT);		
	}
	
	hr = s_lpD3DDev->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 2);

	hr = s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP, ColorOP1);
	hr = s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1, ColorArg11);
	hr = s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG2, ColorArg12);

	hr = s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP, ColorOP2);
	hr = s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLORARG1, ColorArg21);
	hr = s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLORARG2, ColorArg22);

	s_lpD3DDev->EndScene();

	//컬러맵 텍스쳐에 쓰기...
	LPDIRECT3DSURFACE9 pBackBuff;
	hr = s_lpD3DDev->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pBackBuff);

	D3DSURFACE_DESC desc;
	hr = pBackBuff->GetDesc(&desc);
	if(desc.Format!=D3DFMT_X8R8G8B8)
	{
		MessageBox(::GetActiveWindow(),"화면색상모드를 32bit모드로 바꾸세요.\n그렇지 않으면 컬러맵을 만들 수 없어요..ㅠ.ㅠ","경고!!",MB_OK);
		return;
	}

	D3DLOCKED_RECT srcLR, destLR, tmpLR;
	hr = pBackBuff->LockRect(&srcLR, nullptr, 0 );

	DWORD* pTexPtr = (DWORD*)srcLR.pBits;
	
	int iTilesPerColorTexture = m_iColorMapTexSize / m_iColorMapPixelPerUnitDistance;
	int idxX = x/iTilesPerColorTexture;
	int idxZ = y/iTilesPerColorTexture;

	int xx = (x%iTilesPerColorTexture)*m_iColorMapPixelPerUnitDistance;
	int zz = (iTilesPerColorTexture-1 - (y%iTilesPerColorTexture))*m_iColorMapPixelPerUnitDistance;

	m_pColorTexture[idxX][idxZ].Get()->LockRect( 0, &destLR, 0, 0 );
	DWORD* pDestPtr = (DWORD*)destLR.pBits;

	CN3Texture TmpTex;

	TmpTex.Create(TileTexSize, TileTexSize, D3DFMT_X8R8G8B8, TRUE);
	hr = TmpTex.Get()->LockRect(0, & tmpLR, 0, 0);
	DWORD* pTmpTexPtr = (DWORD*)tmpLR.pBits;

	int izz;
	for(izz=0; izz<TileTexSize; izz++)
	{
		memcpy(&(pTmpTexPtr[izz*(tmpLR.Pitch>>2)]), &(pTexPtr[izz*(srcLR.Pitch>>2)]), sizeof(DWORD)*TileTexSize);
	}
	TmpTex.Get()->UnlockRect(0);

	TmpTex.Convert(D3DFMT_X8R8G8B8, m_iColorMapPixelPerUnitDistance, m_iColorMapPixelPerUnitDistance);

	hr = TmpTex.Get()->LockRect(0, & tmpLR, 0, 0);
	pTmpTexPtr = (DWORD*)tmpLR.pBits;

	for(izz=0; izz<m_iColorMapPixelPerUnitDistance; izz++)
	{
		memcpy(&(pDestPtr[xx + (zz+izz)*m_iColorMapTexSize]), &(pTmpTexPtr[izz*(tmpLR.Pitch>>2)]), sizeof(DWORD)*m_iColorMapPixelPerUnitDistance);
	}
	TmpTex.Get()->UnlockRect(0);

	m_pColorTexture[idxX][idxZ].Get()->UnlockRect(0);

	hr = pBackBuff->UnlockRect();

	hr = s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, dwCull);
	hr = s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, dwZ);
	hr = s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, dwLgt);

	m_pColorTexture[idxX][idxZ].GenerateMipMap(); // Mip Map 을 만든다..
}
/*/

//
//	타일맵에서 픽셀을 샘플링해서 걸러맵을 만드는 방법...
//
//*
void CLyTerrain::SetColorMap(int x, int y)
{
	float u1,v1,u2,v2;
	float TileTexSize = (float)(DTEX_SIZE/NUM_DTEXTILE);
	float fEndPoint = TileTexSize - 1;

	HRESULT hr;

	if(m_ppMapData[x][y].DTexInfo1.Attr.Group <=0 && 
		m_ppMapData[x][y].DTexInfo2.Attr.Group <=0) return;

	int dir1, dir2;
	dir1 = m_ppMapData[x][y].DTexInfo1.Dir;
	dir2 = m_ppMapData[x][y].DTexInfo2.Dir;

	u1 = (m_ppMapData[x][y].DTexInfo1.TexIdx.TileX * TileTexSize);
	v1 = (m_ppMapData[x][y].DTexInfo1.TexIdx.TileY * TileTexSize);

	u2 = (m_ppMapData[x][y].DTexInfo2.TexIdx.TileX * TileTexSize);
	v2 = (m_ppMapData[x][y].DTexInfo2.TexIdx.TileY * TileTexSize);

	hr = m_pColorMapTmpVB->Lock( 0, 0, (void**)&m_pColorMapTmpVertices, 0 );

	m_pColorMapTmpVertices[0].Set(0.0f,		0.0f,		0.1f, 0.5f, 0xffffffff,
								(u1+m_fTileDirUforColorMap[dir1][0])/(float)(DTEX_SIZE),
								(v1+m_fTileDirVforColorMap[dir1][0])/(float)(DTEX_SIZE),
								(u2+m_fTileDirUforColorMap[dir2][0])/(float)(DTEX_SIZE),
								(v2+m_fTileDirVforColorMap[dir2][0])/(float)(DTEX_SIZE));
	m_pColorMapTmpVertices[1].Set(fEndPoint,0.0f,		0.1f, 0.5f, 0xffffffff,
								(u1+m_fTileDirUforColorMap[dir1][1])/(float)(DTEX_SIZE),
								(v1+m_fTileDirVforColorMap[dir1][1])/(float)(DTEX_SIZE),
								(u2+m_fTileDirUforColorMap[dir2][1])/(float)(DTEX_SIZE),
								(v2+m_fTileDirVforColorMap[dir2][1])/(float)(DTEX_SIZE));
	m_pColorMapTmpVertices[2].Set(fEndPoint,fEndPoint,	0.1f, 0.5f, 0xffffffff,
								(u1+m_fTileDirUforColorMap[dir1][3])/(float)(DTEX_SIZE),
								(v1+m_fTileDirVforColorMap[dir1][3])/(float)(DTEX_SIZE),
								(u2+m_fTileDirUforColorMap[dir2][3])/(float)(DTEX_SIZE),
								(v2+m_fTileDirVforColorMap[dir2][3])/(float)(DTEX_SIZE));								
	m_pColorMapTmpVertices[3].Set(0.0f,		fEndPoint,	0.1f, 0.5f, 0xffffffff,
								(u1+m_fTileDirUforColorMap[dir1][2])/(float)(DTEX_SIZE),
								(v1+m_fTileDirVforColorMap[dir1][2])/(float)(DTEX_SIZE),
								(u2+m_fTileDirUforColorMap[dir2][2])/(float)(DTEX_SIZE),
								(v2+m_fTileDirVforColorMap[dir2][2])/(float)(DTEX_SIZE));

	if(m_ppIsLightMap[x][y])
	{
		m_pColorMapTmpVertices[4].Set(0.0f,		0.0f,		0.1f, 0.5f, 0xffffffff,
								(1.0f / (float)LIGHTMAP_TEX_SIZE),
								(1.0f / (float)LIGHTMAP_TEX_SIZE),
								0.0f, 0.0f);
		m_pColorMapTmpVertices[5].Set(fEndPoint,0.0f,		0.1f, 0.5f, 0xffffffff,
								1.0f,
								(1.0f / (float)LIGHTMAP_TEX_SIZE),
								0.0f, 0.0f);
		m_pColorMapTmpVertices[6].Set(fEndPoint,fEndPoint,	0.1f, 0.5f, 0xffffffff,
								1.0f,
								1.0f,
								0.0f, 0.0f);
		m_pColorMapTmpVertices[7].Set(0.0f,		fEndPoint,	0.1f, 0.5f, 0xffffffff,
								(1.0f / (float)LIGHTMAP_TEX_SIZE),
								1.0f,
								0.0f, 0.0f);
	}
	
	hr = m_pColorMapTmpVB->Unlock();
	m_pColorMapTmpVertices = nullptr;

	hr = s_lpD3DDev->Clear(0, nullptr, D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET, D3DCOLOR_ARGB(255,0,0,0), 1.0f, 0);
	hr = s_lpD3DDev->BeginScene();

	DWORD dwCull, dwZ, dwLgt, dwFog;
	hr = s_lpD3DDev->GetRenderState(D3DRS_CULLMODE, &dwCull);
	hr = s_lpD3DDev->GetRenderState(D3DRS_ZENABLE, &dwZ);
	hr = s_lpD3DDev->GetRenderState(D3DRS_LIGHTING, &dwLgt);
	hr = s_lpD3DDev->GetRenderState(D3DRS_FOGENABLE, &dwFog);

	hr = s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	hr = s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	hr = s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);
	hr = s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, FALSE);

	hr = s_lpD3DDev->SetStreamSource( 0, m_pColorMapTmpVB, 0, sizeof(__VertexTransformedT2) );
	hr = s_lpD3DDev->SetFVF( FVF_TRANSFORMEDT2 );
	
	DWORD ColorArg11, ColorArg12, ColorArg21, ColorArg22;
	DWORD ColorOP1, ColorOP2;

	hr = s_lpD3DDev->GetTextureStageState( 0, D3DTSS_COLOROP, &ColorOP1);
	hr = s_lpD3DDev->GetTextureStageState( 0, D3DTSS_COLORARG1, &ColorArg11);
	hr = s_lpD3DDev->GetTextureStageState( 0, D3DTSS_COLORARG2, &ColorArg12);

	hr = s_lpD3DDev->GetTextureStageState( 1, D3DTSS_COLOROP, &ColorOP2);
	hr = s_lpD3DDev->GetTextureStageState( 1, D3DTSS_COLORARG1, &ColorArg21);
	hr = s_lpD3DDev->GetTextureStageState( 1, D3DTSS_COLORARG2, &ColorArg22);

	hr = s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	hr = s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	hr = s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	//hr = s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_ADD);
	//hr = s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	//hr = s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT);

	int t1 = m_ppMapData[x][y].DTexInfo1.TexIdx.TexID;
	int t2 = m_ppMapData[x][y].DTexInfo2.TexIdx.TexID;

	if(m_ppMapData[x][y].DTexInfo1.Attr.Group>0 && GetTileTex(t1))
		hr = s_lpD3DDev->SetTexture( 0, GetTileTex(t1)->Get());
	else hr = s_lpD3DDev->SetTexture( 0, nullptr);

	
	if(m_ppMapData[x][y].DTexInfo2.Attr.Group>0 && GetTileTex(t2))
	{
		hr = s_lpD3DDev->SetTexture( 1, GetTileTex(t2)->Get());
		hr = s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_ADD);
		hr = s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT);
		hr = s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	}
	else
	{
		hr = s_lpD3DDev->SetTexture( 1, nullptr);
		hr = s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		hr = s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT);		
	}
	
	hr = s_lpD3DDev->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 2);

	//light map 찍기...
	//
	//if(m_ppIsLightMap[x][y])
	//{
	//	s_lpD3DDev->SetTexture( 0, m_ppLightMapTexture[x][y]->Get());
	//	s_lpD3DDev->SetTexture( 1, nullptr);
	//
	//	DWORD dwAlphaEnable, dwSrcBlend, dwDestBlend;
	//
	//	hr = s_lpD3DDev->GetRenderState( D3DRS_ALPHABLENDENABLE, &dwAlphaEnable);
	//	hr = s_lpD3DDev->GetRenderState( D3DRS_SRCBLEND, &dwSrcBlend);
	//	hr = s_lpD3DDev->GetRenderState( D3DRS_DESTBLEND, &dwDestBlend);
	//
	//	hr = s_lpD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);
	//	hr = s_lpD3DDev->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ZERO);
	//	hr = s_lpD3DDev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
	//	
	//	hr = s_lpD3DDev->DrawPrimitive( D3DPT_TRIANGLEFAN, 4, 2);
	//
	//	hr = s_lpD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE, dwAlphaEnable);
	//	hr = s_lpD3DDev->SetRenderState( D3DRS_SRCBLEND, dwSrcBlend);
	//	hr = s_lpD3DDev->SetRenderState( D3DRS_DESTBLEND, dwDestBlend);
	//}
	//
	////////////

	hr = s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP, ColorOP1);
	hr = s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1, ColorArg11);
	hr = s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG2, ColorArg12);

	hr = s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP, ColorOP2);
	hr = s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLORARG1, ColorArg21);
	hr = s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLORARG2, ColorArg22);

	s_lpD3DDev->EndScene();
	//CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	//HWND hWnd = pFrm->GetActiveView()->m_hWnd;
	//CRect rcD(0,0,128,128), rcS(0,0,2,2);
	//s_lpD3DDev->Present(&rcD, &rcD, hWnd, nullptr);

	//컬러맵 텍스쳐에 쓰기...
	LPDIRECT3DSURFACE9 pBackBuff;
	hr = s_lpD3DDev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuff);

	//POINT ptCopy = {0,0};
	//RECT rcCopy = { 0,0,2,2};
	D3DSURFACE_DESC desc;
	hr = pBackBuff->GetDesc(&desc);
	if(desc.Format!=D3DFMT_X8R8G8B8)
	{
		MessageBox(::GetActiveWindow(),"화면색상모드를 32bit모드로 바꾸세요.\n그렇지 않으면 컬러맵을 만들 수 없어요..ㅠ.ㅠ","경고!!",MB_OK);
		return;
	}

	D3DLOCKED_RECT srcLR,destLR;
	hr = pBackBuff->LockRect(&srcLR, nullptr, 0 );

	DWORD* pTexPtr = (DWORD*)srcLR.pBits;
	
	int iTilesPerColorTexture = m_iColorMapTexSize / m_iColorMapPixelPerUnitDistance;
	int idxX = x/iTilesPerColorTexture;
	int idxZ = y/iTilesPerColorTexture;

	int xx = (x%iTilesPerColorTexture)*m_iColorMapPixelPerUnitDistance;
	int zz = (iTilesPerColorTexture-1 - (y%iTilesPerColorTexture))*m_iColorMapPixelPerUnitDistance;

	m_pColorTexture[idxX][idxZ].Get()->LockRect( 0, &destLR, 0, 0 );
	DWORD* pDestPtr = (DWORD*)destLR.pBits;

	int ixx, izz;
	POINT pt;
	int varRandom = ((int)TileTexSize / m_iColorMapPixelPerUnitDistance)-2;
	for(ixx=0;ixx<m_iColorMapPixelPerUnitDistance;ixx++)
	{
		for(izz=0;izz<m_iColorMapPixelPerUnitDistance;izz++)
		{
			pt.x = ixx*varRandom + (rand()%varRandom) + 1;
			pt.y = izz*varRandom + (rand()%varRandom) + 1;
			
			/*
			DWORD srcColor = pTexPtr[pt.x + pt.y*(srcLR.Pitch>>2)];
			
			DWORD r,g,b;
			int v = 40;
			r = (srcColor<<8)>>24;
			g = (srcColor<<16)>>24;
			b = (srcColor<<24)>>24;

			//if(r<v && g<v && b<v) continue;
			*/
			//
			//int rg,gb,br;
			//rg = (int)(r-g);
			//gb = (int)(g-b);
			//br = (int)(b-r);
			//if(((srcColor<<8)>>8)<0x00000000 && (ABS(rg)<v && ABS(gb)<v && ABS(br)<v)) continue;
			//
			
			pDestPtr[xx+ixx + (zz+izz)*m_iColorMapTexSize] = pTexPtr[pt.x + pt.y*(srcLR.Pitch>>2)];			
		}
	}

	m_pColorTexture[idxX][idxZ].Get()->UnlockRect(0);

	hr = pBackBuff->UnlockRect();

	hr = s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, dwCull);
	hr = s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, dwZ);
	hr = s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, dwLgt);
	hr = s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, dwFog);

	m_pColorTexture[idxX][idxZ].GenerateMipMap(); // Mip Map 을 만든다..
}
//*/


//
//	IntersectTriangle..
//
//	orig	: 시작점...
//	dir		: 피킹 방향..
//	v0, v1, v2 : 삼각형의 세점..
//	t		: orig부터 피킹된점까지 거리..
//	u, v	: v0(0,0) v1(1,0) v2(0,1)로 봤을때 피킹된 점의 uv좌표..
//
BOOL CLyTerrain::IntersectTriangle( const D3DXVECTOR3 orig, const D3DXVECTOR3 dir, D3DXVECTOR3& v0, D3DXVECTOR3& v1, 
								  D3DXVECTOR3& v2, float* t, float* u, float* v)
{
    // Find vectors for two edges sharing vert0
    D3DXVECTOR3 edge1 = v1 - v0;
    D3DXVECTOR3 edge2 = v2 - v0;

    // Begin calculating determinant - also used to calculate U parameter
    D3DXVECTOR3 pvec;
    D3DXVec3Cross( &pvec, &dir, &edge2 );

    // If determinant is near zero, ray lies in plane of triangle
    FLOAT det = D3DXVec3Dot( &edge1, &pvec );
    if( det < 0.0001f )		// 거의 0에 가까우면 삼각형 평면과 지나가는 선이 평행하다.
        return FALSE;

    // Calculate distance from vert0 to ray origin
    D3DXVECTOR3 tvec = orig - v0;

    // Calculate U parameter and test bounds
    *u = D3DXVec3Dot( &tvec, &pvec );
    if( *u < 0.0f || *u > det )
        return FALSE;

    // Prepare to test V parameter
    D3DXVECTOR3 qvec;
    D3DXVec3Cross( &qvec, &tvec, &edge1 );

    // Calculate V parameter and test bounds
    *v = D3DXVec3Dot( &dir, &qvec );
    if( *v < 0.0f || *u + *v > det )
        return FALSE;

    // Calculate t, scale parameters, ray intersects triangle
    *t = D3DXVec3Dot( &edge2, &qvec );
    FLOAT fInvDet = 1.0f / det;
    *t *= fInvDet;
    *u *= fInvDet;
    *v *= fInvDet;

	// t가 클수록 멀리 직선과 평면과 만나는 점이 멀다.
	// t*dir + orig 를 구하면 만나는 점을 구할 수 있다.
	// u와 v의 의미는 무엇일까?
	// 추측 : v0 (0,0), v1(1,0), v2(0,1) <괄호안은 (U, V)좌표> 이런식으로 어느 점에 가깝나 나타낸 것 같음
	//

    return TRUE;
}



//////////////////////////////////////////////////
//	Coded By : Dino
//	Coded On 2001-06-21 오후 12:18:31
//	아래로 지형 Edit관련 함수들


//
//	에디트 모드로 전환...
//
void CLyTerrain::SetEditMode(int iEditMode)
{
	m_iEditMode = iEditMode;
	if(iEditMode==TEM_LIGHTMAP)
	{
		if (m_pDlgSetLightMap) m_pDlgSetLightMap->ShowWindow(TRUE);
	}
	else
	{
		if (m_pDlgSetLightMap) m_pDlgSetLightMap->ShowWindow(FALSE);
	}

	if(iEditMode==TEM_DTEX)
	{
		CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
		CDTexGroupMng* pDTexGroupMng = pFrm->GetDTexGroupMng();
		if(pDTexGroupMng)
		{
			pDTexGroupMng->m_pGroupView->ShowWindow(TRUE);
		}
	}
	else
	{
		CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
		CDTexGroupMng* pDTexGroupMng = pFrm->GetDTexGroupMng();
		if(pDTexGroupMng)
		{
			pDTexGroupMng->m_pGroupView->ShowWindow(FALSE);
		}
	}

}


//
//	SetApexHeight
//
void CLyTerrain::SetApexHeight(int x, int z, float fHeight)
{
	if (m_ppMapData && x<m_iHeightMapSize && z<m_iHeightMapSize && x>=0 && z>=0)
	{
		m_ppMapData[x][z].fHeight = fHeight;
	}
}


//
//	GetApexHeight..
//	각타일의 꼭지점의 높이를 구하는 함수...
//
float CLyTerrain::GetApexHeight(int x, int z) const
{
	if (m_ppMapData && x<m_iHeightMapSize && z<m_iHeightMapSize && x>=0 && z>=0)
	{
		return m_ppMapData[x][z].fHeight;
	}
	return -FLT_MAX;
}


//
// 브러쉬의 모양과 사이즈에 따라서 IntensityMap을 다시 구성한다.
//
void CLyTerrain::UpdateBrushIntensityMap(int iShape, int iSize, float fFallOff)
{
	ASSERT(fFallOff>=0.0f && iSize>0);
	ZeroMemory(m_fBrushIntensityMap, sizeof(m_fBrushIntensityMap));
	int i, j, k;

	int iStart, iEnd, iHalfRadius;
	iStart = BRUSH_CENTER-(int)(iSize/2);	iEnd = iStart+iSize;	iHalfRadius = (iSize/2)+1;

	if (iShape == BRUSHSHAPE_CIRCLE)
	{
		if (iSize%2)		// Brush 사이즈가 홀수일때
		{
			for(i=iStart; i<iEnd; ++i)
				for(j=iStart; j<iEnd; ++j)
				{
					{
						int iTmpX = i-BRUSH_CENTER, iTmpY = j-BRUSH_CENTER;
						float fLen = (float)(iTmpX*iTmpX + iTmpY*iTmpY);
						fLen = fLen/(iHalfRadius*iHalfRadius);
						if (fLen>1.0f) continue;
						m_fBrushIntensityMap[i][j] = GetFallOffValue(fFallOff, fLen);
					}
				}
		}
		else				// Brush 사이즈가 짝수일때
		{
			float fTmp = (iHalfRadius-0.5f);
			for(i=iStart; i<iEnd; ++i)
				for(j=iStart; j<iEnd; ++j)
				{
					float fTmpX = i-BRUSH_CENTER+0.5f, fTmpY = j-BRUSH_CENTER+0.5f;
					float fLen = fTmpX*fTmpX + fTmpY*fTmpY;
					fLen = fLen/(fTmp*fTmp);
					if (fLen>1.0f) continue;
					m_fBrushIntensityMap[i][j] = GetFallOffValue(fFallOff, fLen);
				}
		}
	}
	else if (iShape == BRUSHSHAPE_SQUARE)
	{
		float fIntensity[MAX_BRUSH_SIZE];
		ZeroMemory(fIntensity, sizeof(fIntensity));

		if (iSize%2)		// Brush 사이즈가 홀수일때
		{
			for (i=0; i <= iSize/2; ++i) fIntensity[i] = GetFallOffValue(fFallOff, float(i)/iHalfRadius);

			for(i=iStart; i<iEnd; ++i)
				for(j=iStart; j<iEnd; ++j)
				{
					int iOffsetX = abs(i-BRUSH_CENTER),iOffsetZ = abs(j-BRUSH_CENTER);
					for (k=0; k<iHalfRadius; ++k)
					{
						if ((iOffsetX==k && iOffsetZ<=k) || (iOffsetX<=k && iOffsetZ==k) )
							m_fBrushIntensityMap[i][j] = fIntensity[k];
					}
				}
		}
		else				// Brush 사이즈가 짝수일때
		{
			for (i=0; i <= iSize/2; ++i) fIntensity[i] = GetFallOffValue(fFallOff, float(i+0.5f)/(iHalfRadius));

			for(i=iStart; i<iEnd; ++i)
			{
				int iT = 2*iStart+iSize-i;
				if (BRUSH_CENTER>i)
				{
					for(j=i; j<iT; ++j)	m_fBrushIntensityMap[i][j] = fIntensity[BRUSH_CENTER-1-i];
				}
				else
				{
					for(j=iT-1; j<=i; ++j)
					{
						ASSERT(i-BRUSH_CENTER>=0);
						m_fBrushIntensityMap[i][j] = fIntensity[i-BRUSH_CENTER];
					}
				}
			}
			for(i=iStart; i<iEnd; ++i)
			{
				int iT = 2*iStart+iSize-i;
				if (BRUSH_CENTER>i)
				{
					for(j=i+1; j<iT-1; ++j)	m_fBrushIntensityMap[j][i] = fIntensity[BRUSH_CENTER-1-i];
				}
				else
				{
					for(j=iT; j<i; ++j)
					{
						ASSERT(i-BRUSH_CENTER>=0);
						m_fBrushIntensityMap[j][i] = fIntensity[i-BRUSH_CENTER];
					}
				}
			}
		}
	}
}


//
// -1.0f ~ x ~ 1.0f
// return : 0.0f~1.0f
float CLyTerrain::GetFallOffValue(float fFallOff, float x)
{
	if (x<0.0f) x = -x;
	float fUpTo = 5.0f;
	if (fFallOff<0.1f) fUpTo = 10.0f;
	return (1.0f - powf(x, fUpTo - (fUpTo-1.2f)*fFallOff));
}


//
//	지형 높이 조절함수....
//
void CLyTerrain::Heighten(POINT ptCenter, float fHeight)
{
	int i, j;

	float fNewHeight = FLT_MAX;

	float fMax = -FLT_MIN;
	float fMin = FLT_MIN;
	for (i=0; i<MAX_BRUSH_SIZE; ++i)
	{
		for (j=0; j<MAX_BRUSH_SIZE; ++j)
		{
			int iMapX, iMapZ;
			iMapX = ptCenter.x - BRUSH_CENTER + i;
			iMapZ = ptCenter.y - BRUSH_CENTER + j;
			if (iMapX<0 || iMapZ<0 || iMapX>=m_iHeightMapSize || iMapZ>=m_iHeightMapSize) continue;
			if(m_fBrushIntensityMap[i][j]<=0) continue;

			fNewHeight = GetApexHeight(iMapX,iMapZ);
			break;
		}
		if(fNewHeight != FLT_MAX) break;
	}

	for (i=0; i<MAX_BRUSH_SIZE; ++i)
	{
		for (j=0; j<MAX_BRUSH_SIZE; ++j)
		{
			int iMapX, iMapZ;
			iMapX = ptCenter.x - BRUSH_CENTER + i;
			iMapZ = ptCenter.y - BRUSH_CENTER + j;
			if (iMapX<0 || iMapZ<0 || iMapX>=m_iHeightMapSize || iMapZ>=m_iHeightMapSize) continue;

			if(m_bBrushFlat)
			{
				if(m_fBrushIntensityMap[i][j]<=0) continue;
				SetApexHeight(iMapX, iMapZ, fNewHeight+m_fBrushIntensityMap[i][j]*fHeight );
			}
			else SetApexHeight(iMapX, iMapZ, GetApexHeight(iMapX,iMapZ) + m_fBrushIntensityMap[i][j]*fHeight );
		}
	}
}


//
//
//
void CLyTerrain::Flaten(POINT ptCenter)
{	
	int i, j;
	// 새로운 값과 기존 높이값을 차이를 계산해서 버퍼에 저장.
	for (i=0; i<MAX_BRUSH_SIZE; ++i)
	{
		for (j=0; j<MAX_BRUSH_SIZE; ++j)
		{
			int iMapX, iMapZ;
			iMapX = ptCenter.x - BRUSH_CENTER + i;
			iMapZ = ptCenter.y - BRUSH_CENTER + j;
			if (iMapX<0 || iMapZ<0 || iMapX>=m_iHeightMapSize || iMapZ>=m_iHeightMapSize) continue;

			if (m_fBrushIntensityMap[i][j]>0.0f)
			{
				m_ppMapData[iMapX][iMapZ].fHeight = m_fFlatHeight;
			}
		}
	}	
}


//
//	지형 완만하게 만드는 함수..
//
void CLyTerrain::Smooth(POINT ptCenter)
{
	float fMapHeightDiff[MAX_BRUSH_SIZE][MAX_BRUSH_SIZE];
	ZeroMemory(fMapHeightDiff, sizeof(fMapHeightDiff));
	const int iWSize = 5;		// 홀수여야 한다.
	const int iHalfWSize = iWSize/2;
	float fWeight[iWSize][iWSize] =	{	{1,2,3,2,1},
										{2,3,4,3,2},
										{3,4,5,4,3},
										{2,3,4,3,2},
										{1,2,3,2,1}};

	int i, j, k, l;
	// 새로운 값과 기존 높이값을 차이를 계산해서 버퍼에 저장.
	for (i=0; i<MAX_BRUSH_SIZE; ++i)
	{
		for (j=0; j<MAX_BRUSH_SIZE; ++j)
		{
			int iMapX, iMapZ;
			iMapX = ptCenter.x - BRUSH_CENTER + i;
			iMapZ = ptCenter.y - BRUSH_CENTER + j;
			if (iMapX<0 || iMapZ<0 || iMapX>=m_iHeightMapSize || iMapZ>=m_iHeightMapSize) continue;

			if (m_fBrushIntensityMap[i][j]>0.0f)
			{
				float fH;
				float fSumWeight = 0.0f;
				for (k=0; k<iWSize; ++k)
					for (l=0; l<iWSize; ++l)
					{
						fH = GetApexHeight(iMapX+k-iHalfWSize, iMapZ+l-iHalfWSize);
						if (fH != -FLT_MAX)
						{
							fMapHeightDiff[i][j] += (fH*fWeight[k][l]);
							fSumWeight += fWeight[k][l];
						}
					}
				fMapHeightDiff[i][j] = (fMapHeightDiff[i][j]/fSumWeight)-GetApexHeight(iMapX,iMapZ);
			}
		}
	}

	// 버퍼에 있는 값을 m_fBrushIntensityMap 적용하여 높이값 수정
	for (i=0; i<MAX_BRUSH_SIZE; ++i)
	{
		for (j=0; j<MAX_BRUSH_SIZE; ++j)
		{
			int iMapX, iMapZ;
			iMapX = ptCenter.x - BRUSH_CENTER + i;
			iMapZ = ptCenter.y - BRUSH_CENTER + j;
			if (iMapX<0 || iMapZ<0 || iMapX>=m_iHeightMapSize || iMapZ>=m_iHeightMapSize) continue;
			SetApexHeight(iMapX, iMapZ,
				GetApexHeight(iMapX,iMapZ) + m_fBrushIntensityMap[i][j]*fMapHeightDiff[i][j]*0.2f );
		}
	}
}


//
//	UpdateBrushArea..
//
void CLyTerrain::UpdateBrushArea(POINT ptCenter)
{
	m_iBrushIndexCount = 0;
	int iBrushVerexCount = 0;
	int i, j;
	float fOffsetY = 0.05f;
	for (i=0; i<MAX_BRUSH_SIZE; ++i)
	{
		for (j=0; j<MAX_BRUSH_SIZE; ++j)
		{
			if (m_fBrushIntensityMap[i][j] == 0.0f) continue;

			float fOffsetXZ = TERRAIN_CELL_SIZE/2.0f;
			float fMapX = (float)TERRAIN_CELL_SIZE*(ptCenter.x - BRUSH_CENTER + i), fMapZ = (float)TERRAIN_CELL_SIZE*(ptCenter.y - BRUSH_CENTER + j);
			float fTmpX, fTmpZ;
			D3DCOLOR color = 0x000000ff|(DWORD(0x00000088*m_fBrushIntensityMap[i][j])<<24);

			_ASSERT(iBrushVerexCount+9 <= MAX_BRUSH_SIZE*MAX_BRUSH_SIZE*9);

			fTmpX = fMapX - fOffsetXZ;	fTmpZ = fMapZ - fOffsetXZ;
			m_vBrushVertices[iBrushVerexCount+0].Set(fTmpX, GetHeight(fTmpX,fTmpZ)+fOffsetY, fTmpZ, color);
			fTmpX = fMapX;				fTmpZ = fMapZ - fOffsetXZ;
			m_vBrushVertices[iBrushVerexCount+1].Set(fTmpX, GetHeight(fTmpX,fTmpZ)+fOffsetY, fTmpZ, color);
			fTmpX = fMapX + fOffsetXZ;	fTmpZ = fMapZ - fOffsetXZ;
			m_vBrushVertices[iBrushVerexCount+2].Set(fTmpX, GetHeight(fTmpX,fTmpZ)+fOffsetY, fTmpZ, color);
			fTmpX = fMapX - fOffsetXZ;	fTmpZ = fMapZ;
			m_vBrushVertices[iBrushVerexCount+3].Set(fTmpX, GetHeight(fTmpX,fTmpZ)+fOffsetY, fTmpZ, color);
			fTmpX = fMapX;				fTmpZ = fMapZ;
			m_vBrushVertices[iBrushVerexCount+4].Set(fTmpX, GetHeight(fTmpX,fTmpZ)+fOffsetY, fTmpZ, color);
			fTmpX = fMapX + fOffsetXZ;	fTmpZ = fMapZ;
			m_vBrushVertices[iBrushVerexCount+5].Set(fTmpX, GetHeight(fTmpX,fTmpZ)+fOffsetY, fTmpZ, color);
			fTmpX = fMapX - fOffsetXZ;	fTmpZ = fMapZ + fOffsetXZ;
			m_vBrushVertices[iBrushVerexCount+6].Set(fTmpX, GetHeight(fTmpX,fTmpZ)+fOffsetY, fTmpZ, color);
			fTmpX = fMapX;				fTmpZ = fMapZ + fOffsetXZ;
			m_vBrushVertices[iBrushVerexCount+7].Set(fTmpX, GetHeight(fTmpX,fTmpZ)+fOffsetY, fTmpZ, color);
			fTmpX = fMapX + fOffsetXZ;	fTmpZ = fMapZ + fOffsetXZ;
			m_vBrushVertices[iBrushVerexCount+8].Set(fTmpX, GetHeight(fTmpX,fTmpZ)+fOffsetY, fTmpZ, color);
			WORD wIndices[24] = {0,3,4, 0,4,1, 1,4,5, 1,5,2, 3,6,7, 3,7,4, 4,7,8, 4,8,5};
			for (int k=0; k<24; ++k)
			{
				m_wBrushIndices[m_iBrushIndexCount++] = wIndices[k]+iBrushVerexCount;
			}
			iBrushVerexCount += 9;
			ASSERT(m_iBrushIndexCount <= MAX_BRUSH_SIZE*MAX_BRUSH_SIZE*6*4);
		}
	}
}


//
//	RenderBrushArea..
//
void CLyTerrain::RenderBrushArea()
{
	if (m_iEditMode == TEM_NOT || m_iEditMode == TEM_DTEX || m_iBrushIndexCount == 0) return;

	__Matrix44 WorldMtx;	WorldMtx.Identity();	
	s_lpD3DDev->SetTransform(D3DTS_WORLD, &WorldMtx);

	// backup
	DWORD dwAlphaBlend, dwSrcBlend, dwDestBlend;
	s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlphaBlend);
	s_lpD3DDev->GetRenderState(D3DRS_SRCBLEND, &dwSrcBlend);
	s_lpD3DDev->GetRenderState(D3DRS_DESTBLEND, &dwDestBlend);

	// set state
	s_lpD3DDev->SetTexture( 0, nullptr);
	s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
	s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
	s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	s_lpD3DDev->SetFVF(FVF_XYZCOLOR);
	s_lpD3DDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, MAX_BRUSH_SIZE*MAX_BRUSH_SIZE*9,
		int(m_iBrushIndexCount/3), m_wBrushIndices, D3DFMT_INDEX16,
		m_vBrushVertices, sizeof(__VertexXyzColor));

	// restore
	s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlphaBlend);
	s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, dwSrcBlend);
	s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, dwDestBlend);
}


//
//	Import...
//	마야에서 뽑아낸 N3Scene파일로 부터 지형 읽기..
//
void CLyTerrain::Import(LPCTSTR pFileName, float fSize)
{
	int HeightMapSize;
	double NumCell = fSize / TERRAIN_CELL_SIZE;
	double Log = log(NumCell) / log(2);
	int		iLog = (int)Log;
	if((Log-(double)iLog)==0) HeightMapSize = (int)(fSize / TERRAIN_CELL_SIZE) + 1 ;
	else
	{
		HeightMapSize = (int)pow(2,iLog+1) + 1;
	}

	Init(HeightMapSize);

	CN3VMesh* pVMesh = new CN3VMesh;
	__ASSERT(pVMesh, "VMesh가 안읽허여..ㅜ.ㅜ");

	pVMesh->LoadFromFile(pFileName);

	__Vector3 vMax, vMin;
	vMax = pVMesh->Max();
	vMin = pVMesh->Min();

	float SizeX, SizeZ;
	SizeX = vMax.x - vMin.x;
	SizeZ = vMax.z - vMin.z;
	
	float ScaleX, ScaleZ;
	ScaleX = fSize / SizeX;
	ScaleZ = fSize / SizeZ;

	int NumVertices, NumFace;
	NumVertices = pVMesh->VertexCount();
	NumFace = NumVertices / 3;

	__Vector3*	pVertices = pVMesh->Vertices();
	__Vector3 vA, vB, vC;
	
	//__VertexT1 vA, vB, vC;
	CProgressBar ProgressBar;
	ProgressBar.Create("Import terrain VMesh..", 50, NumFace);
	ProgressBar.SetPos(1);
	int iPosProgress = 0;

	__Vector2 vMinXZ, vMaxXZ;
	int sx, sz, ex, ez;
	int ix, iz;

	for(ix=0;ix<m_iHeightMapSize;ix++)
	{
		for(iz=0;iz<m_iHeightMapSize;iz++)
		{
			m_ppMapData[ix][iz].fHeight = FLT_MIN;
			DTEXINFO TexInfo;
			ZeroMemory(&TexInfo, sizeof(DTEXINFO));
			TexInfo.Dir = 0;
			TexInfo.Attr.Attr = 0;
			TexInfo.Attr.Group = 0;
			TexInfo.TexIdx.TexID = -1;
			TexInfo.TexIdx.TileX = -1;
			TexInfo.TexIdx.TileY = -1;
			
			m_ppMapData[ix][iz].DTexInfo1 = TexInfo;
			m_ppMapData[ix][iz].DTexInfo2 = TexInfo;
		}
	}

	for(int i=0;i<NumFace;i++)
	{
		int index = i*3;
		vA.Set((pVertices[index].x - vMin.x)*ScaleX,	(pVertices[index].y)*ScaleX,	(pVertices[index].z - vMin.z)*ScaleZ);
		vB.Set((pVertices[index+1].x - vMin.x)*ScaleX,	(pVertices[index+1].y)*ScaleX,	(pVertices[index+1].z - vMin.z)*ScaleZ);
		vC.Set((pVertices[index+2].x - vMin.x)*ScaleX,	(pVertices[index+2].y)*ScaleX,	(pVertices[index+2].z - vMin.z)*ScaleZ);

		vMinXZ.Set(vA.x, vA.z);
		vMaxXZ.Set(vA.x, vA.z);

		if(vB.x > vMaxXZ.x) vMaxXZ.x = vB.x;
		if(vC.x > vMaxXZ.x) vMaxXZ.x = vC.x;
		if(vB.z > vMaxXZ.y) vMaxXZ.y = vB.z;
		if(vC.z > vMaxXZ.y) vMaxXZ.y = vC.z;
		
		if(vB.x < vMinXZ.x) vMinXZ.x = vB.x;
		if(vC.x < vMinXZ.x) vMinXZ.x = vC.x;		
		if(vB.z < vMinXZ.y) vMinXZ.y = vB.z;
		if(vC.z < vMinXZ.y) vMinXZ.y = vC.z;

		sx = vMinXZ.x / TERRAIN_CELL_SIZE;
		ex = (vMaxXZ.x / TERRAIN_CELL_SIZE)+1;

		sz = vMinXZ.y / TERRAIN_CELL_SIZE;
		ez = (vMaxXZ.y / TERRAIN_CELL_SIZE)+1;

		if(ex>=m_iHeightMapSize) ex = m_iHeightMapSize - 1;
		if(ez>=m_iHeightMapSize) ez = m_iHeightMapSize - 1;
		if(sx==ex && sz==ez) continue;

		for(ix=sx; ix<= ex; ix++)
		{
			for(iz=sz; iz<=ez; iz++)
			{
				// 삼각형 점 포함 테스트...
				float a, b, c;
				float x,z;
				x = (float)ix * TERRAIN_CELL_SIZE;
				z = (float)iz * TERRAIN_CELL_SIZE;

				a = ((vB.x-vA.x)*(z-vA.z)) - ((vB.z-vA.z)*(x-vA.x));
				b = ((vC.x-vB.x)*(z-vB.z)) - ((vC.z-vB.z)*(x-vB.x));
				c = ((vA.x-vC.x)*(z-vC.z)) - ((vA.z-vC.z)*(x-vC.x));

				if((a<=0 && b<=0 && c<=0) || (a>=0 && b>=0 && c>=0))
				{
					__Vector3 vAB, vAC, vAxis;
					vAB.Set(vB.x - vA.x, vB.y - vA.y, vB.z - vA.z);
					vAC.Set(vC.x - vA.x, vC.y - vA.y, vC.z - vA.z);

					vAxis.Cross(vAB, vAC);

					float y;
					if(vAxis.y == 0.0f) y = vA.y;
					else y = (vAxis.x*(vA.x-x)+vAxis.z*(vA.z-z))/vAxis.y + vA.y;

					m_ppMapData[ix][iz].fHeight = y;					
				}
			}
		}

		iPosProgress++;
		ProgressBar.SetPos(iPosProgress);
	}

//////////////////////////////////////////////////////////////////////////////////////////////////
/*
//구버전...
	CProgressBar ProgressBar;
	ProgressBar.Create("Import terrain VMesh..", 50, 1000);
	ProgressBar.SetPos(1);
	int iPosProgress = 0;
	
	for(int ix=0;ix<m_iHeightMapSize;ix++)
	{
		for(int iz=0;iz<m_iHeightMapSize;iz++)
		{
			//일단 높이값 초기화..
			m_ppMapData[ix][iz].fHeight = -FLT_MAX;

			DTEXINFO TexInfo;
			ZeroMemory(&TexInfo, sizeof(DTEXINFO));
			TexInfo.Dir = 0;
			TexInfo.Attr.Attr = 0;
			TexInfo.Attr.Group = 0;
			TexInfo.TexIdx.TexID = -1;
			TexInfo.TexIdx.TileX = -1;
			TexInfo.TexIdx.TileY = -1;
			
			m_ppMapData[ix][iz].DTexInfo1 = TexInfo;
			m_ppMapData[ix][iz].DTexInfo2 = TexInfo;

			for(int i=0;i<NumFace;i++)
			{
				int index = i*3;
				vA.Set((pVertices[index].x - vMin.x)*ScaleX,	(pVertices[index].y)*ScaleX,	(pVertices[index].z - vMin.z)*ScaleZ);
				vB.Set((pVertices[index+1].x - vMin.x)*ScaleX,	(pVertices[index+1].y)*ScaleX,	(pVertices[index+1].z - vMin.z)*ScaleZ);
				vC.Set((pVertices[index+2].x - vMin.x)*ScaleX,	(pVertices[index+2].y)*ScaleX,	(pVertices[index+2].z - vMin.z)*ScaleZ);

				// 삼각형 점 포함 테스트...
				float a, b, c;
				float x,z;
				x = (float)ix * TERRAIN_CELL_SIZE;
				z = (float)iz * TERRAIN_CELL_SIZE;

				if( (vA.x > x && vB.x > x && vC.x > x) || (vA.x < x && vB.x < x && vC.x < x) ||
					(vA.z > z && vB.z > z && vC.z > z) || (vA.z < z && vB.z < z && vC.z < z) ) continue;
				
				a = ((vB.x-vA.x)*(z-vA.z)) - ((vB.z-vA.z)*(x-vA.x));
				b = ((vC.x-vB.x)*(z-vB.z)) - ((vC.z-vB.z)*(x-vB.x));
				c = ((vA.x-vC.x)*(z-vC.z)) - ((vA.z-vC.z)*(x-vC.x));

				if((a<=0 && b<=0 && c<=0) || (a>=0 && b>=0 && c>=0))
				{
					__Vector3 vAB, vAC, vAxis;
					vAB.Set(vB.x - vA.x, vB.y - vA.y, vB.z - vA.z);
					vAC.Set(vC.x - vA.x, vC.y - vA.y, vC.z - vA.z);

					vAxis.Cross(vAB, vAC);

					float y;
					if(vAxis.y == 0.0f) y = vA.y;
					else y = (vAxis.x*(vA.x-x)+vAxis.z*(vA.z-z))/vAxis.y + vA.y;

					m_ppMapData[ix][iz].fHeight = y;
					break;
				}
			}			
			iPosProgress++;
			ProgressBar.SetPos((1000 * iPosProgress) / (m_iHeightMapSize * m_iHeightMapSize));
		}
	}
*/
//////////////////////////////////////////////////////////////////////////////////////////////////

	//라이트맵텍스쳐 수도 있고, 텍스쳐도 세팅..
	m_pRoot->SetWholeClipped();
	m_pRoot->ClearMinMaxY();
	m_pRoot->SetMinMaxY();

	delete pVMesh;
}


//
//
//
void CLyTerrain::ImportHeight(LPCTSTR pFileName)
{
	CN3VMesh* pVMesh = new CN3VMesh;
	__ASSERT(pVMesh, "VMesh가 안읽혀여..ㅜ.ㅜ");

	pVMesh->LoadFromFile(pFileName);

	__Vector3 vMax, vMin;
	vMax = pVMesh->Max();
	vMin = pVMesh->Min();

	float SizeX, SizeZ;
	SizeX = vMax.x - vMin.x;
	SizeZ = vMax.z - vMin.z;

	float ScaleX, ScaleZ;
	ScaleX = (m_iHeightMapSize-1) * TERRAIN_CELL_SIZE / SizeX;
	ScaleZ = (m_iHeightMapSize-1) * TERRAIN_CELL_SIZE / SizeZ;

	int NumVertices, NumFace;
	NumVertices = pVMesh->VertexCount();
	NumFace = NumVertices / 3;

	__Vector3*	pVertices = pVMesh->Vertices();
	__Vector3 vA, vB, vC;
	
	//__VertexT1 vA, vB, vC;
	CProgressBar ProgressBar;
	ProgressBar.Create("Import terrain VMesh..", 50, NumFace);
	ProgressBar.SetPos(1);
	int iPosProgress = 0;

	__Vector2 vMinXZ, vMaxXZ;
	int sx, sz, ex, ez;
	int ix, iz;

	for(ix=0;ix<m_iHeightMapSize;ix++)
	{
		for(iz=0;iz<m_iHeightMapSize;iz++)
		{
			m_ppMapData[ix][iz].fHeight = FLT_MIN;
		}
	}

	for(int i=0;i<NumFace;i++)
	{
		int index = i*3;
		vA.Set((pVertices[index].x - vMin.x)*ScaleX,	(pVertices[index].y)*ScaleX,	(pVertices[index].z - vMin.z)*ScaleZ);
		vB.Set((pVertices[index+1].x - vMin.x)*ScaleX,	(pVertices[index+1].y)*ScaleX,	(pVertices[index+1].z - vMin.z)*ScaleZ);
		vC.Set((pVertices[index+2].x - vMin.x)*ScaleX,	(pVertices[index+2].y)*ScaleX,	(pVertices[index+2].z - vMin.z)*ScaleZ);

		vMinXZ.Set(vA.x, vA.z);
		vMaxXZ.Set(vA.x, vA.z);

		if(vB.x > vMaxXZ.x) vMaxXZ.x = vB.x;
		if(vC.x > vMaxXZ.x) vMaxXZ.x = vC.x;
		if(vB.z > vMaxXZ.y) vMaxXZ.y = vB.z;
		if(vC.z > vMaxXZ.y) vMaxXZ.y = vC.z;
		
		if(vB.x < vMinXZ.x) vMinXZ.x = vB.x;
		if(vC.x < vMinXZ.x) vMinXZ.x = vC.x;		
		if(vB.z < vMinXZ.y) vMinXZ.y = vB.z;
		if(vC.z < vMinXZ.y) vMinXZ.y = vC.z;

		sx = vMinXZ.x / TERRAIN_CELL_SIZE;
		ex = (vMaxXZ.x / TERRAIN_CELL_SIZE)+1;

		sz = vMinXZ.y / TERRAIN_CELL_SIZE;
		ez = (vMaxXZ.y / TERRAIN_CELL_SIZE)+1;

		if(ex>=m_iHeightMapSize) ex = m_iHeightMapSize - 1;
		if(ez>=m_iHeightMapSize) ez = m_iHeightMapSize - 1;
		if(sx==ex && sz==ez) continue;

		for(ix=sx; ix<= ex; ix++)
		{
			for(iz=sz; iz<=ez; iz++)
			{
				// 삼각형 점 포함 테스트...
				float a, b, c;
				float x,z;
				x = (float)ix * TERRAIN_CELL_SIZE;
				z = (float)iz * TERRAIN_CELL_SIZE;

				a = ((vB.x-vA.x)*(z-vA.z)) - ((vB.z-vA.z)*(x-vA.x));
				b = ((vC.x-vB.x)*(z-vB.z)) - ((vC.z-vB.z)*(x-vB.x));
				c = ((vA.x-vC.x)*(z-vC.z)) - ((vA.z-vC.z)*(x-vC.x));

				if((a<=0 && b<=0 && c<=0) || (a>=0 && b>=0 && c>=0))
				{
					__Vector3 vAB, vAC, vAxis;
					vAB.Set(vB.x - vA.x, vB.y - vA.y, vB.z - vA.z);
					vAC.Set(vC.x - vA.x, vC.y - vA.y, vC.z - vA.z);

					vAxis.Cross(vAB, vAC);

					float y;
					if(vAxis.y == 0.0f) y = vA.y;
					else y = (vAxis.x*(vA.x-x)+vAxis.z*(vA.z-z))/vAxis.y + vA.y;

					m_ppMapData[ix][iz].fHeight = y;					
				}
			}
		}

		iPosProgress++;
		ProgressBar.SetPos(iPosProgress);
	}

	//라이트맵텍스쳐 수도 있고, 텍스쳐도 세팅..
	m_pRoot->SetWholeClipped();
	m_pRoot->ClearMinMaxY();
	m_pRoot->SetMinMaxY();

	delete pVMesh;
}
/*
void CLyTerrain::ImportHeight(LPCTSTR pFileName)
{
	CN3VMesh* pVMesh = new CN3VMesh;
	__ASSERT(pVMesh, "VMesh가 안읽허여..ㅜ.ㅜ");

	pVMesh->LoadFromFile(pFileName);

	__Vector3 vMax, vMin;
	vMax = pVMesh->Max();
	vMin = pVMesh->Min();

	float SizeX, SizeZ;
	SizeX = vMax.x - vMin.x;
	SizeZ = vMax.z - vMin.z;

	float ScaleX, ScaleZ;
	ScaleX = (m_iHeightMapSize-1) * TERRAIN_CELL_SIZE / SizeX;
	ScaleZ = (m_iHeightMapSize-1) * TERRAIN_CELL_SIZE / SizeZ;

	int NumVertices, NumFace;
	NumVertices = pVMesh->VertexCount();
	NumFace = NumVertices / 3;

	__Vector3*	pVertices = pVMesh->Vertices();
	__Vector3 vA, vB, vC;
	
	//__VertexT1 vA, vB, vC;
	CProgressBar ProgressBar;
	ProgressBar.Create("Import terrain VMesh..", 50, 1000);
	ProgressBar.SetPos(1);
	int iPosProgress = 0;
	
	for(int ix=0;ix<m_iHeightMapSize;ix++)
	{
		for(int iz=0;iz<m_iHeightMapSize;iz++)
		{
			//일단 높이값 초기화..
			m_ppMapData[ix][iz].fHeight = -FLT_MAX;

			for(int i=0;i<NumFace;i++)
			{
				int index = i*3;
				vA.Set((pVertices[index].x - vMin.x)*ScaleX,	(pVertices[index].y)*ScaleX,	(pVertices[index].z - vMin.z)*ScaleZ);
				vB.Set((pVertices[index+1].x - vMin.x)*ScaleX,	(pVertices[index+1].y)*ScaleX,	(pVertices[index+1].z - vMin.z)*ScaleZ);
				vC.Set((pVertices[index+2].x - vMin.x)*ScaleX,	(pVertices[index+2].y)*ScaleX,	(pVertices[index+2].z - vMin.z)*ScaleZ);

				// 삼각형 점 포함 테스트...
				float a, b, c;
				float x,z;
				x = (float)ix * TERRAIN_CELL_SIZE;
				z = (float)iz * TERRAIN_CELL_SIZE;

				if( (vA.x > x && vB.x > x && vC.x > x) || (vA.x < x && vB.x < x && vC.x < x) ||
					(vA.z > z && vB.z > z && vC.z > z) || (vA.z < z && vB.z < z && vC.z < z) ) continue;
				
				a = ((vB.x-vA.x)*(z-vA.z)) - ((vB.z-vA.z)*(x-vA.x));
				b = ((vC.x-vB.x)*(z-vB.z)) - ((vC.z-vB.z)*(x-vB.x));
				c = ((vA.x-vC.x)*(z-vC.z)) - ((vA.z-vC.z)*(x-vC.x));

				if((a<=0 && b<=0 && c<=0) || (a>=0 && b>=0 && c>=0))
				{
					__Vector3 vAB, vAC, vAxis;
					vAB.Set(vB.x - vA.x, vB.y - vA.y, vB.z - vA.z);
					vAC.Set(vC.x - vA.x, vC.y - vA.y, vC.z - vA.z);

					vAxis.Cross(vAB, vAC);

					float y;
					if(vAxis.y == 0.0f) y = vA.y;
					else y = (vAxis.x*(vA.x-x)+vAxis.z*(vA.z-z))/vAxis.y + vA.y;

					m_ppMapData[ix][iz].fHeight = y;
					break;
				}
			}			
			iPosProgress++;
			ProgressBar.SetPos((1000 * iPosProgress) / (m_iHeightMapSize * m_iHeightMapSize));
		}
	}

	//라이트맵텍스쳐 수도 있고, 텍스쳐도 세팅..
	m_pRoot->SetWholeClipped();
	m_pRoot->ClearMinMaxY();
	m_pRoot->SetMinMaxY();

	delete pVMesh;
}
//*/


//
//	ImportColorMap..
//
void CLyTerrain::ColorMapImport(LPCTSTR lpszPathName)
{
	CBitMapFile BMF;
	if(false == BMF.LoadFromFile(lpszPathName)) return;

	CProgressBar ProgressBar; // 진행 상황..
	ProgressBar.Create("Import color map..", 50, m_iNumColorMap * m_iNumColorMap);
	ProgressBar.SetStep(1);

	int x = 0, z = 0;
	for(x=0;x<m_iNumColorMap;x++)
	{
		for(z=0;z<m_iNumColorMap;z++)
		{
			ProgressBar.StepIt();

			RECT rc;
			rc.left = x*m_iColorMapTexSize;
			rc.top = (m_iNumColorMap - z - 1)*m_iColorMapTexSize;
			rc.right = rc.left + m_iColorMapTexSize;
			rc.bottom = rc.top + m_iColorMapTexSize;
			BMF.SaveRectToFile("C:\\TmpSave.BMP", rc);
			m_pColorTexture[x][z].LoadFromFile("C:\\TmpSave.BMP");
			DeleteFile("C:\\TmpSave.BMP");
		}
	}
}

void CLyTerrain::ColorMapExport(LPCTSTR lpszPathName)
{
	if(m_iNumColorMap <= 0) return;
	if(lstrlen(lpszPathName) <= 0) return;

	CBitMapFile BMF;
	if(false == BMF.Create(m_iNumColorMap * m_iColorMapTexSize, m_iNumColorMap * m_iColorMapTexSize)) return;

	CProgressBar ProgressBar; // 진행 상황..
	ProgressBar.Create("Export color map..", 50, m_iNumColorMap * m_iNumColorMap);
	ProgressBar.SetStep(1);

	int x = 0, z = 0, x2 = 0, z2 = 0;
	for(z=0;z<m_iNumColorMap;z++)
	{
		for(x=0;x<m_iNumColorMap;x++)
		{
			ProgressBar.StepIt();

			if(	m_pColorTexture[x][z].Get() == nullptr ||
				m_pColorTexture[x][z].PixelFormat() != D3DFMT_X8R8G8B8 ||
				m_pColorTexture[x][z].Width() != m_iColorMapTexSize ||
				m_pColorTexture[x][z].Height() != m_iColorMapTexSize ) 
			{
				MessageBox(::GetActiveWindow(), "Colormap's format is invalid", "Can't export colormap", MB_OK);
			}

			char*	pPixelDest = (char*)(BMF.Pixels(x * m_iColorMapTexSize, (m_iNumColorMap - z - 1) * m_iColorMapTexSize));
			int		nPitchDest = BMF.Pitch();
			D3DLOCKED_RECT lr;
			m_pColorTexture[x][z].Get()->LockRect(0, &lr, nullptr, 0);
			char*	pPixelSrc = (char*)(lr.pBits);

			for(z2 = 0; z2 < m_iColorMapTexSize; z2++)
			{
				char* pDestTmp = pPixelDest + z2*nPitchDest;
				char* pSrcTmp = pPixelSrc + z2*lr.Pitch;
				for(x2 = 0; x2 < m_iColorMapTexSize; x2++)
				{
					*(pDestTmp+0) = *(pSrcTmp+0); // Red
					*(pDestTmp+1) = *(pSrcTmp+1); // Green
					*(pDestTmp+2) = *(pSrcTmp+2); // Blue
					
					pDestTmp += 3;
					pSrcTmp += 4;
				}
			}
			
			m_pColorTexture[x][z].Get()->UnlockRect(0);
		}
	}

	BMF.SaveToFile(lpszPathName); // 비트맵으로 저장..
}

void CLyTerrain::GenerateMiniMap(LPCTSTR lpszPathName, int size)
{
	if(m_iNumColorMap <= 0) return;
	if(lstrlen(lpszPathName) <= 0) return;

	CBitMapFile BMF;
	if(false == BMF.Create(size, size)) return;

	CProgressBar ProgressBar; // 진행 상황..
	ProgressBar.Create("Generate MiniMap..", 50, m_iNumColorMap * m_iNumColorMap);
	ProgressBar.SetStep(1);

	int PatchSize = size / m_iNumColorMap;

	int x = 0, z = 0, x2 = 0, z2 = 0;
	CN3Texture TmpTex;
	for(z=0;z<m_iNumColorMap;z++)
	{
		for(x=0;x<m_iNumColorMap;x++)
		{
			ProgressBar.StepIt();
				
			LPDIRECT3DSURFACE9 lpSurface;
			m_pColorTexture[x][z].Get()->GetSurfaceLevel(0, &lpSurface);
			TmpTex.CreateFromSurface( lpSurface, m_pColorTexture[x][z].PixelFormat(), TRUE);
			lpSurface->Release();
			
			TmpTex.Convert(TmpTex.PixelFormat(), PatchSize, PatchSize);

			char*	pPixelDest = (char*)(BMF.Pixels(x * PatchSize, (m_iNumColorMap - z - 1) * PatchSize));
			int		nPitchDest = BMF.Pitch();

			D3DLOCKED_RECT lr;
			TmpTex.Get()->LockRect(0, &lr, nullptr, 0);
			char*	pPixelSrc = (char*)(lr.pBits);

			for(z2 = 0; z2 < PatchSize; z2++)
			{
				char* pDestTmp = pPixelDest + z2*nPitchDest;
				char* pSrcTmp = pPixelSrc + z2*lr.Pitch;
				for(x2 = 0; x2 < PatchSize; x2++)
				{
					*(pDestTmp+0) = *(pSrcTmp+0); // Red
					*(pDestTmp+1) = *(pSrcTmp+1); // Green
					*(pDestTmp+2) = *(pSrcTmp+2); // Blue
					
					pDestTmp += 3;
					pSrcTmp += 4;
				}
			}			
			TmpTex.Get()->UnlockRect(0);			
		}
	}
	BMF.SaveToFile(lpszPathName); // 비트맵으로 저장..
}

void  CLyTerrain::ExportHeightBMP(const char* szPathName)
{
	if(lstrlen(szPathName) <= 0) return;

	CBitMapFile BMF;
	if(false == BMF.Create(m_iHeightMapSize, m_iHeightMapSize)) return;

	CProgressBar ProgressBar; // 진행 상황..
	ProgressBar.Create("Export HeightBMP..", 50, m_iHeightMapSize * m_iHeightMapSize);
	ProgressBar.SetStep(1);

	float Min, Max;
	Min = FLT_MAX;
	Max = -FLT_MAX;

	int x = 0, z = 0;
	float Height, diff;
	for(z=0;z<m_iHeightMapSize;z++)
	{
		for(x=0;x<m_iHeightMapSize;x++)
		{
			Height = GetHeight(x*TERRAIN_CELL_SIZE,z*TERRAIN_CELL_SIZE);
			if(Height<-10000.0f || Height>10000.0f) continue;

			if(Height>Max) Max = Height;
			if(Height<Min) Min = Height;
		}
	}

	diff = Max - Min;

	unsigned char gray;
	for(z=0;z<m_iHeightMapSize;z++)
	{
		for(x=0;x<m_iHeightMapSize;x++)
		{
			ProgressBar.StepIt();
			unsigned char*	pPixelDest = (unsigned char*)(BMF.Pixels(x, m_iHeightMapSize-z-1));

			//gray = (unsigned char)((GetHeight(x*TERRAIN_CELL_SIZE, z*TERRAIN_CELL_SIZE)-Min) * 255.0f / diff);
			float Height = GetHeight(x*TERRAIN_CELL_SIZE, z*TERRAIN_CELL_SIZE);
			if(Height<-10000.0f) 
			{
				gray = 0;
			}
			if((Height-Min)>255.0f) 
			{
				gray = 255;//Max - Min;
			}
			else
			{
				gray = (unsigned char)(Height-Min);
			}
					
			pPixelDest[0] = gray;
			pPixelDest[1] = gray;
			pPixelDest[2] = gray;
		}
	}
	BMF.SaveToFile(szPathName); // 비트맵으로 저장..

	char msg[256];
	sprintf(msg, "Max : %.2f Min : %.2f", Max, Min);

	MessageBox(::GetActiveWindow(), msg, nullptr, MB_OK);
}

void  CLyTerrain::ImportHeightBMP(const char* szPathName)
{
	CBitMapFile BMF;
	BMF.LoadFromFile(szPathName);

	CProgressBar ProgressBar; // 진행 상황..
	ProgressBar.Create("Import HeightBMP..", 50, m_iHeightMapSize * m_iHeightMapSize);
	ProgressBar.SetStep(1);

	int ix, iz;
	for(ix=0;ix<m_iHeightMapSize;ix++)
	{
		for(iz=0;iz<m_iHeightMapSize;iz++)
		{
			ProgressBar.StepIt();
			unsigned char*	pPixelDest = (unsigned char*)(BMF.Pixels(ix, m_iHeightMapSize-iz-1));
			if(!pPixelDest) m_ppMapData[ix][iz].fHeight = 0.0f;
			else m_ppMapData[ix][iz].fHeight = (*pPixelDest);
		}
	}

	//라이트맵텍스쳐 수도 있고, 텍스쳐도 세팅..
	m_pRoot->SetWholeClipped();
	m_pRoot->ClearMinMaxY();
	m_pRoot->SetMinMaxY();

	m_fHeightScale = 0.5f;
}

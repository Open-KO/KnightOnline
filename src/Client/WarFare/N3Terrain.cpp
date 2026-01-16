//
// N3Terrain.cpp: implementation of the CLyTerrain class.
//	2001. 10. 22.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "N3Terrain.h"
#include "N3TerrainPatch.h"
#include "PlayerMySelf.h"
#include "GameProcedure.h"
#include "UILoading.h"

#include "N3River.h"
#include "N3Pond.h"

#include <FileIO/FileReader.h>

#include <cstdio>

constexpr float COLLISION_BOX = 100.0f;

CN3Terrain::CN3Terrain()
{
	m_Material.Init();
	m_ShadeMode       = D3DSHADE_GOURAUD;
	m_FillMode        = D3DFILL_SOLID;
	m_iLodLevel       = MIN_LOD_LEVEL;

	m_ppPatch         = nullptr;

	m_pat_CenterPos.x = m_pat_CenterPos.y = 0;
	m_pat_LBPos.x = m_pat_LBPos.y = 0;
	m_pat_PrevLBPos.x = m_pat_PrevLBPos.y = 0;

	SetRectEmpty(&m_pat_BoundRect);
	m_iNumPatch      = 0;

	m_pMapData       = nullptr;
	m_ti_MapSize     = 0;
	m_pat_MapSize    = 0;

	m_ti_CenterPos.x = m_ti_CenterPos.y = 0;
	m_ti_PrevCenterPos                  = m_ti_CenterPos;

	m_ppPatchRadius                     = nullptr;
	m_ppPatchMiddleY                    = nullptr;

	m_iNumColorMap                      = 0;

	float TileDirU[8][4]                = {
        { 0.0f, 1.0f, 0.0f, 1.0f }, //[up][LT, RT, LB, RB]
        { 0.0f, 0.0f, 1.0f, 1.0f }, //[right][ // ]
        { 1.0f, 0.0f, 1.0f, 0.0f }, //[left][ // ]
        { 1.0f, 1.0f, 0.0f, 0.0f }, //[bottom][ // ]

        { 1.0f, 0.0f, 1.0f, 0.0f }, //[up_mirr][LT, RT, LB, RB]
        { 0.0f, 0.0f, 1.0f, 1.0f }, //[right_mirr][ // ]
        { 0.0f, 1.0f, 0.0f, 1.0f }, //[left_mirr][ // ]
        { 1.0f, 1.0f, 0.0f, 0.0f }  //[bottom_mirr][ // ]
	};
	memcpy(m_fTileDirU, TileDirU, sizeof(float) * 8 * 4);

	float TileDirV[8][4] = {
		{ 0.0f, 0.0f, 1.0f, 1.0f }, //[up][ // ]
		{ 1.0f, 0.0f, 1.0f, 0.0f }, //[right][ // ]
		{ 1.0f, 1.0f, 0.0f, 0.0f }, //[left][ // ]
		{ 0.0f, 1.0f, 0.0f, 1.0f }, //[bottom][ // ]

		{ 0.0f, 0.0f, 1.0f, 1.0f }, //[up_mirr][ // ]
		{ 0.0f, 1.0f, 0.0f, 1.0f }, //[right_mirr][ // ]
		{ 1.0f, 1.0f, 0.0f, 0.0f }, //[left_mirr][ // ]
		{ 1.0f, 0.0f, 1.0f, 0.0f }  //[bottom_mirr][ // ]
	};
	memcpy(m_fTileDirV, TileDirV, sizeof(float) * 8 * 4);

	MakeDistanceTable();

	m_pRiver         = nullptr;
	m_pPond          = nullptr;
	m_pNormal        = nullptr;

	m_bAvailableTile = true;
}

CN3Terrain::~CN3Terrain()
{
	CN3Terrain::Release();
}

//
//	MakeDistanceTable
//	거리를 계산하지 말고 테이블에서 가져올 수 있게 미리 테이블 생성..
//	정수 단위 거리..
//
void CN3Terrain::MakeDistanceTable()
{
	for (int x = 0; x < DISTANCE_TABLE_SIZE; x++)
	{
		for (int z = 0; z < DISTANCE_TABLE_SIZE; z++)
		{
			double dist            = sqrt((double) ((x * x) + (z * z))) + 0.6;
			m_iDistanceTable[x][z] = (int) dist;
		}
	}
}

//
//	Release....
//
void CN3Terrain::Release()
{
	if (m_pRiver != nullptr)
	{
		m_pRiver->Release();
		delete m_pRiver;
		m_pRiver = nullptr;
	}

	delete m_pPond;
	m_pPond = nullptr;

	m_TileTex.clear();
	m_ColorMapTex.clear();

	if (m_ppPatch != nullptr)
	{
		for (int x = 0; x < m_iNumPatch; x++)
		{
			delete[] m_ppPatch[x];
			m_ppPatch[x] = nullptr;
		}

		delete[] m_ppPatch;
		m_ppPatch = nullptr;
	}

	if (m_pMapData != nullptr)
	{
		GlobalFree(m_pMapData);
		m_pMapData = nullptr;
	}

	if (m_pNormal != nullptr)
	{
		GlobalFree(m_pNormal);
		m_pNormal = nullptr;
	}

	if (m_ppPatchRadius != nullptr)
	{
		for (int x = 0; x < m_pat_MapSize; x++)
		{
			delete[] m_ppPatchRadius[x];
			m_ppPatchRadius[x] = nullptr;
		}

		delete[] m_ppPatchRadius;
		m_ppPatchRadius = nullptr;
	}

	if (m_ppPatchMiddleY != nullptr)
	{
		for (int x = 0; x < m_pat_MapSize; x++)
		{
			delete[] m_ppPatchMiddleY[x];
			m_ppPatchMiddleY[x] = nullptr;
		}

		delete[] m_ppPatchMiddleY;
		m_ppPatchMiddleY = nullptr;
	}

	for (int x = 0; x < 3; x++)
	{
		for (int z = 0; z < 3; z++)
		{
			for (auto& [_, pTex] : m_LightMapPatch[x][z])
				delete pTex;

			m_LightMapPatch[x][z].clear();
		}
	}

	CN3BaseFileAccess::Release();
}

//
//	Init...
//
void CN3Terrain::Init()
{
	Release();

	TestAvailableTile();

	m_Material.Init();
	m_ShadeMode       = D3DSHADE_GOURAUD;
	m_FillMode        = D3DFILL_SOLID;
	//m_FillMode = D3DFILL_WIREFRAME;

	m_pat_CenterPos.x = m_pat_CenterPos.y = -100;
	m_pat_LBPos.x = m_pat_LBPos.y = -100;
	m_pat_PrevLBPos.x = m_pat_PrevLBPos.y = -100;

	SetRectEmpty(&m_pat_BoundRect);
	//m_pat_Center2Side = ((int)CN3Base::s_CameraData.fFP / (PATCH_TILE_SIZE * TILE_SIZE)) + 1;
	//	m_pat_Center2Side = 17;		// CN3Base::s_CameraData.fFP = 512 라고 가정할때...
	m_pat_Center2Side = 33; // CN3Base::s_CameraData.fFP = 1024 라고 가정할때...

	m_iNumPatch       = (m_pat_Center2Side << 1) + 1;

	m_ppPatch         = new CN3TerrainPatch*[m_iNumPatch];
	for (int x = 0; x < m_iNumPatch; x++)
		m_ppPatch[x] = new CN3TerrainPatch[m_iNumPatch];

	for (int x = 0; x < m_iNumPatch; x++)
	{
		for (int z = 0; z < m_iNumPatch; z++)
			m_ppPatch[x][z].Init(this);
	}

	m_pBaseTex.LoadFromFile("Misc\\Terrain_Base.bmp");

	m_iLodLevel = MIN_LOD_LEVEL;
	SetLODLevel(3);

	m_pMapData       = nullptr;
	m_pNormal        = nullptr;
	m_ti_MapSize     = 0;
	m_pat_MapSize    = 0;

	m_ti_CenterPos.x = m_ti_CenterPos.y = -100;
	m_ti_PrevCenterPos                  = m_ti_CenterPos;

	m_TileTex.clear();
	m_ColorMapTex.clear();
	m_iNumColorMap = 0;

	m_pRiver       = new CN3River();
	m_pPond        = new CN3Pond();
}

//
//	글픽카드가 타일맵을 그릴 수 있는지 없는지 검사...
//
void CN3Terrain::TestAvailableTile()
{
	m_bAvailableTile = true;

	if (CN3Base::s_DevCaps.MaxTextureBlendStages < 3 || ((CN3Base::s_DevCaps.PrimitiveMiscCaps & D3DPMISCCAPS_BLENDOP) == 0))
	{
#ifdef _N3GAME
		CLogWriter::Write("terrain tile not supported..");
#endif
		m_bAvailableTile = false;
	}

	return;

	DWORD ColorOP[3] {}, ColorArg1[3] {}, ColorArg2[3] {};

	s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLOROP, &ColorOP[0]);
	s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLORARG1, &ColorArg1[0]);

	s_lpD3DDev->GetTextureStageState(1, D3DTSS_COLOROP, &ColorOP[1]);
	s_lpD3DDev->GetTextureStageState(1, D3DTSS_COLORARG1, &ColorArg1[1]);
	s_lpD3DDev->GetTextureStageState(1, D3DTSS_COLORARG2, &ColorArg2[1]);

	s_lpD3DDev->GetTextureStageState(2, D3DTSS_COLOROP, &ColorOP[2]);
	s_lpD3DDev->GetTextureStageState(2, D3DTSS_COLORARG1, &ColorArg1[2]);
	s_lpD3DDev->GetTextureStageState(2, D3DTSS_COLORARG2, &ColorArg2[2]);

	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

	s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_ADD);
	s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);

	s_lpD3DDev->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_MODULATE);
	s_lpD3DDev->SetTextureStageState(2, D3DTSS_COLORARG1, D3DTA_CURRENT);
	s_lpD3DDev->SetTextureStageState(2, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	DWORD dwNumPasses = 0;
	HRESULT hr        = s_lpD3DDev->ValidateDevice(&dwNumPasses);

	if (hr == D3DERR_TOOMANYOPERATIONS || hr == D3DERR_UNSUPPORTEDCOLORARG || hr == D3DERR_UNSUPPORTEDCOLOROPERATION)
		m_bAvailableTile = false;

	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, ColorOP[0]);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, ColorArg1[0]);

	s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLOROP, ColorOP[1]);
	s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLORARG1, ColorArg1[1]);
	s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLORARG2, ColorArg2[1]);

	s_lpD3DDev->SetTextureStageState(2, D3DTSS_COLOROP, ColorOP[2]);
	s_lpD3DDev->SetTextureStageState(2, D3DTSS_COLORARG1, ColorArg1[2]);
	s_lpD3DDev->SetTextureStageState(2, D3DTSS_COLORARG2, ColorArg2[2]);
}

//
//	Load...
//
bool CN3Terrain::LoadSupportedVersions(File& file)
{
	// Init will release resources and unset the filename.
	// We should preserve and restore it.
	const std::string szFNBackup = m_szFileName;

	// Fetch current offset, so we can rewind and try reading the file again.
	const int64_t originalOffset = static_cast<int64_t>(file.Offset());

	// Try supported file format versions, starting with our preferred version:
	constexpr int SupportedVersions[] { N3FORMAT_VER_1264, N3FORMAT_VER_1098 };
	for (int iFileFormatVersion : SupportedVersions)
	{
		Init();

		// Restore the filename after it was released.
		m_szFileName         = szFNBackup;

		// Attempt to load with the provided file format version.
		m_iFileFormatVersion = iFileFormatVersion;

		try
		{
			if (Load(file))
				return true;

			CLogWriter::Write("CN3Terrain: Failed to load {} for format version {} (Load() failed).", szFNBackup, iFileFormatVersion);
		}
		catch (const std::runtime_error& ex)
		{
			CLogWriter::Write("CN3Terrain: Failed to load {} for format version {} ({}).", szFNBackup, iFileFormatVersion, ex.what());
		}

		// Rewind
		file.Seek(originalOffset, SEEK_SET);
	}

	CLogWriter::Write("CN3Terrain: Failed to load {} - unsupported.", szFNBackup);
	return false;
}

bool CN3Terrain::Load(File& file)
{
	constexpr int MAX_SUPPORTED_VERSION     = 2;
	constexpr int MAX_SUPPORTED_NAME_LENGTH = 30;
	constexpr int MAX_SUPPORTED_MAP_SIZE    = 4096; // CN3ShapeMgr: MAX_CELL_MAIN * MAX_CELL_MAIN

	int iVersion                            = 0;

	if (m_iFileFormatVersion >= N3FORMAT_VER_1264)
	{
		file.Read(&iVersion, sizeof(int));

		if (iVersion < 0 || iVersion > MAX_SUPPORTED_VERSION)
			throw std::runtime_error("invalid version in header");

		int iNL = 0;
		file.Read(&iNL, sizeof(int));

		if (iNL < 0 || iNL > MAX_SUPPORTED_NAME_LENGTH)
			throw std::runtime_error("invalid name length in header");

		if (iNL > 0)
		{
			m_szName.assign(iNL, '\0');
			file.Read(&m_szName[0], iNL);
		}
	}

	file.Read(&m_ti_MapSize, sizeof(int));

	if (m_ti_MapSize <= 0 || (m_ti_MapSize - 1) > MAX_SUPPORTED_MAP_SIZE)
		throw std::runtime_error("invalid map size");

	if (((m_ti_MapSize - 1) % 4) != 0)
		throw std::runtime_error("map size must be a multiple of 4");

	CUILoading* pUILoading = CGameProcedure::s_pUILoading; // 로딩바..
	if (pUILoading != nullptr)
		pUILoading->Render("Allocating Terrain...", 0);

	m_pat_MapSize = (m_ti_MapSize - 1) / PATCH_TILE_SIZE;

	m_pMapData    = static_cast<MAPDATA*>(GlobalAlloc(GMEM_FIXED, sizeof(MAPDATA) * m_ti_MapSize * m_ti_MapSize));
	if (m_pMapData == nullptr)
		CLogWriter::Write("Terrain Error : MapData Memory Allocation Failed..-.-");

	__ASSERT(m_pMapData, "MapData Memory Allocation Failed..-.-");
	file.Read(m_pMapData, sizeof(MAPDATA) * m_ti_MapSize * m_ti_MapSize);

	m_pNormal = static_cast<__Vector3*>(GlobalAlloc(GMEM_FIXED, sizeof(__Vector3) * m_ti_MapSize * m_ti_MapSize));
	if (m_pNormal == nullptr)
		CLogWriter::Write("Terrain Error : Normal Vector Memory Allocation Failed..-.-");

	__ASSERT(m_pNormal, "Normal Vector Memory Allocation Failed..-.-");
	SetNormals();

	if (pUILoading != nullptr)
		pUILoading->Render("", 100);

	// patch middleY & radius...
	m_ppPatchRadius  = new float*[m_pat_MapSize];
	m_ppPatchMiddleY = new float*[m_pat_MapSize];
	for (int x = 0; x < m_pat_MapSize; x++)
	{
		m_ppPatchMiddleY[x] = new float[m_pat_MapSize];
		m_ppPatchRadius[x]  = new float[m_pat_MapSize];
	}

	if (pUILoading != nullptr)
		pUILoading->Render("Loading Terrain Patch Data...", 0);

	std::string szLoadingBuff;
	for (int x = 0; x < m_pat_MapSize; x++)
	{
		for (int z = 0; z < m_pat_MapSize; z++)
		{
			file.Read(&m_ppPatchMiddleY[x][z], sizeof(float));
			file.Read(&m_ppPatchRadius[x][z], sizeof(float));
		}

		int iLoading  = (x + 1) * 100 / m_pat_MapSize;
		szLoadingBuff = fmt::format("Loading Terrain Patch Data... {} %", iLoading);

		if (pUILoading != nullptr)
			pUILoading->Render(szLoadingBuff, iLoading);
	}

	// Grass attributes
	file.Seek(sizeof(uint8_t) * m_ti_MapSize * m_ti_MapSize, SEEK_CUR);

	// Grass filename
	file.Seek(MAX_PATH, SEEK_CUR);

	LoadTileInfo(file);

	// load lightmap..
	if (pUILoading != nullptr)
		pUILoading->Render("Loading Lightmap Data...", 0);

	int NumLightMap = 0;
	file.Read(&NumLightMap, sizeof(int));

	if (NumLightMap != 0)
		throw std::runtime_error("unexpected lightmap count; this is deprecated");

	if (pUILoading != nullptr)
		pUILoading->Render("Loading River Data...", 0);

	m_pRiver->Load(file); // 맵데이터 올때까지만 잠시만 막자..2002.11.15
	m_pPond->Load(file, iVersion);

	if (pUILoading != nullptr)
		pUILoading->Render("", 100);
	return true;
}

//
//
//
void CN3Terrain::SetNormals()
{
	return;
	/*
	int x,z;
	__Vector3 vNormalTmp(0.0f, 0.0f, 0.0f);
	__Vector3 V1, V2;
	for(x=0;x<m_ti_MapSize;x++)
	{
		for(z=0;z<m_ti_MapSize;z++)
		{
			if( x==0 || z==0 || x==(m_ti_MapSize-1) || z==(m_ti_MapSize-1) )
			{
				m_pNormal[z + x*m_ti_MapSize].Set(0.0f, 1.0f, 0.0f);
			}
			else
			{
				int c = x*m_ti_MapSize + z;
				if ((x+z)%2==1)
				{
					int u,v;
					for(int i=0;i<4;i++)
					{
						if(i==0) { u = x*m_ti_MapSize + (z+1); v = (x+1)*m_ti_MapSize + z; }
						else if(i==1) { u = (x+1)*m_ti_MapSize + z; v = x*m_ti_MapSize + (z-1); }
						else if(i==2) { u = x*m_ti_MapSize + (z-1); v = (x-1)*m_ti_MapSize + z; }
						else if(i==3) { u = (x-1)*m_ti_MapSize + z; v = x*m_ti_MapSize + (z+1); }
					}

					V1.Set(0.0f, m_pMapData[u].fHeight - m_pMapData[c].fHeight, 4.0f);
					V2.Set(4.0f, m_pMapData[v].fHeight - m_pMapData[c].fHeight, 0.0f);
					vNormalTmp.Cross(V1, V2);
					vNormalTmp.Normalize();



				}
			}
		}
	}
*/
}

//
//
//
const MAPDATA& CN3Terrain::GetMapData(int x, int z) const
{
	static MAPDATA empty = {};
	if (x < 0 || x >= m_ti_MapSize || z < 0 || z >= m_ti_MapSize)
		return empty;

	return m_pMapData[(x * m_ti_MapSize) + z];
}

//
//
//
void CN3Terrain::LoadTileInfo(File& file)
{
	constexpr int MAX_SUPPORTED_TILE_TEX_COUNT = 4096;
	constexpr int MAX_SUPPORTED_GTT_COUNT      = 1024;

	m_TileTex.clear();

	// 로딩바..
	CUILoading* pUILoading = CGameProcedure::s_pUILoading;
	if (pUILoading != nullptr)
		pUILoading->Render("Loading Terrain Tile Data...", 0);

	int tileTextureCount = 0;
	file.Read(&tileTextureCount, sizeof(int));
	if (tileTextureCount == 0)
		return;

	if (tileTextureCount < 0 || tileTextureCount > MAX_SUPPORTED_TILE_TEX_COUNT)
		throw std::runtime_error("invalid tile texture count");

	m_TileTex.resize(tileTextureCount);

	int NumTileTexSrc = 0;
	file.Read(&NumTileTexSrc, sizeof(int));
	if (NumTileTexSrc == 0)
		return;

	if (NumTileTexSrc < 0 || NumTileTexSrc > MAX_SUPPORTED_GTT_COUNT)
		throw std::runtime_error("invalid GTT texture count");

	char** SrcName = new char*[NumTileTexSrc];
	for (int i = 0; i < NumTileTexSrc; i++)
	{
		SrcName[i] = new char[MAX_PATH];
		file.Read(SrcName[i], MAX_PATH);
	}

	std::string szLoadingBuff;
	for (size_t i = 0; i < m_TileTex.size(); i++)
	{
		CN3Texture& tex = m_TileTex[i];

		int16_t SrcIdx = 0, TileIdx = 0;
		file.Read(&SrcIdx, sizeof(int16_t));
		file.Read(&TileIdx, sizeof(int16_t));

		if (SrcIdx < 0)
			throw std::runtime_error("invalid GTT filename index");

		if (TileIdx < 0)
			throw std::runtime_error("invalid tile texture index");

		FileReader gttFile;
		if (!gttFile.OpenExisting(SrcName[SrcIdx]))
			continue;

		tex.m_iFileFormatVersion = m_iFileFormatVersion;

		for (int j = 0; j < TileIdx; j++)
			tex.SkipFileHandle(gttFile);        // 앞에 있는 쓸때 없는 것들...

		tex.m_iLOD = s_Options.iTexLOD_Terrain; // LOD 적용후 읽기..
		tex.Load(gttFile);                      // 진짜 타일...

		// loading bar...
		size_t loadingPercentage = (i + 1) * 100 / m_TileTex.size();
		szLoadingBuff            = fmt::format("Loading Terrain Tile Data... {} %", loadingPercentage);
		if (pUILoading != nullptr)
			pUILoading->Render(szLoadingBuff, static_cast<int>(loadingPercentage));
	}

	for (int i = 0; i < NumTileTexSrc; i++)
	{
		delete[] SrcName[i];
		SrcName[i] = nullptr;
	}

	delete[] SrcName;
}

//
//	lod level 설정..
//	default는 3...
//	min = 0, max = 10..
//
bool CN3Terrain::SetLODLevel(int level)
{
	if (level == m_iLodLevel)
		return false;

	m_iLodLevel = level;

	if (m_iLodLevel < 2)
		m_iLodLevel = 2;

	for (int x = 0; x < m_iNumPatch; x++)
	{
		for (int z = 0; z < m_iNumPatch; z++)
		{
			int dist = m_iDistanceTable[std::abs(m_pat_Center2Side - x)][std::abs(m_pat_Center2Side - z)];
			if (dist <= m_iLodLevel)
				m_ppPatch[x][z].SetLevel(1);
			else if (dist <= m_iLodLevel + 3)
				m_ppPatch[x][z].SetLevel(2);
			else
				m_ppPatch[x][z].SetLevel(3);
		}
	}

	SetBlunt();
	return true;
}

//
//	SetBlunt...
//	각 패치들 그릴방법 정하기..어느면을 무디게 할것인지..
//
void CN3Terrain::SetBlunt()
{
	for (int x = 0; x < m_iNumPatch; x++)
	{
		for (int z = 0; z < m_iNumPatch; z++)
		{
			m_ppPatch[x][z].m_IsBlunt[0] = true;
			m_ppPatch[x][z].m_IsBlunt[1] = true;
			m_ppPatch[x][z].m_IsBlunt[2] = true;
			m_ppPatch[x][z].m_IsBlunt[3] = true;
			if (m_ppPatch[x][z].GetLevel() == 1)
			{
				m_ppPatch[x][z].m_IsBlunt[0] = false;
				m_ppPatch[x][z].m_IsBlunt[1] = false;
				m_ppPatch[x][z].m_IsBlunt[2] = false;
				m_ppPatch[x][z].m_IsBlunt[3] = false;
				continue;
			}

			if (m_ppPatch[x][z].GetLevel() == 2)
			{
				if (x > 0)
				{
					if (m_ppPatch[x][z].GetLevel() > m_ppPatch[x - 1][z].GetLevel())
						m_ppPatch[x][z].m_IsBlunt[0] = false;
				}
				if ((z + 1) < m_iNumPatch)
				{
					if (m_ppPatch[x][z].GetLevel() > m_ppPatch[x][z + 1].GetLevel())
						m_ppPatch[x][z].m_IsBlunt[1] = false;
				}
				if ((x + 1) < m_iNumPatch)
				{
					if (m_ppPatch[x][z].GetLevel() > m_ppPatch[x + 1][z].GetLevel())
						m_ppPatch[x][z].m_IsBlunt[2] = false;
				}
				if (z > 0)
				{
					if (m_ppPatch[x][z].GetLevel() > m_ppPatch[x][z - 1].GetLevel())
						m_ppPatch[x][z].m_IsBlunt[3] = false;
				}
				continue;
			}

			if (x > 0)
			{
				if (m_ppPatch[x][z].GetLevel() >= m_ppPatch[x - 1][z].GetLevel())
					m_ppPatch[x][z].m_IsBlunt[0] = false;
			}
			if ((z + 1) < m_iNumPatch)
			{
				if (m_ppPatch[x][z].GetLevel() >= m_ppPatch[x][z + 1].GetLevel())
					m_ppPatch[x][z].m_IsBlunt[1] = false;
			}
			if ((x + 1) < m_iNumPatch)
			{
				if (m_ppPatch[x][z].GetLevel() >= m_ppPatch[x + 1][z].GetLevel())
					m_ppPatch[x][z].m_IsBlunt[2] = false;
			}
			if (z > 0)
			{
				if (m_ppPatch[x][z].GetLevel() >= m_ppPatch[x][z - 1].GetLevel())
					m_ppPatch[x][z].m_IsBlunt[3] = false;
			}
		}
	}
}

//
//	Tick..
//
void CN3Terrain::Tick()
{
	int iLOD           = 0; // LOD 수준 계산.. 나중에 계산식을 바꾸어야 한다.
	iLOD               = (int) (3.0f * s_CameraData.fFP / 512.0f);
	bool ChangeLOD     = this->SetLODLevel(iLOD);

	m_pat_PrevLBPos    = m_pat_LBPos;
	m_ti_PrevCenterPos = m_ti_CenterPos;

	bool bMovePatch    = CheckMovePatch();
	if (bMovePatch || ChangeLOD)
		DispositionPatch();

	bool bChangeBound = CheckBound();

	if (bMovePatch || bChangeBound || ChangeLOD)
	{
		for (int x = m_pat_BoundRect.left; x <= m_pat_BoundRect.right; x++)
		{
			for (int z = m_pat_BoundRect.top; z <= m_pat_BoundRect.bottom; z++)
			{
				if (x < 0 || z < 0)
					continue;

				m_ppPatch[x][z].Tick();
			}
		}
	}

	if (m_pRiver != nullptr)
		m_pRiver->Tick();

	if (m_pPond != nullptr)
		m_pPond->Tick();
}

//
//	CheckMovePatch
//	패치단위의 이동이 이루어 졌는지...
//
bool CN3Terrain::CheckMovePatch()
{
	m_ti_CenterPos.x = Real2Tile(CN3Base::s_CameraData.vEye.x);
	m_ti_CenterPos.y = Real2Tile(CN3Base::s_CameraData.vEye.z);

	m_pat_LBPos.x    = Tile2Patch(m_ti_CenterPos.x) - m_pat_Center2Side;
	m_pat_LBPos.y    = Tile2Patch(m_ti_CenterPos.y) - m_pat_Center2Side;

	if (m_pat_PrevLBPos.x == m_pat_LBPos.x && m_pat_PrevLBPos.y == m_pat_LBPos.y)
		return false;

	return true;
}

//
//	DispositionPatch
//
void CN3Terrain::DispositionPatch()
{
	int px = 0, pz = 0;
	int cx = 0, cz = 0;
	for (int x = 0; x < m_iNumPatch; x++)
	{
		for (int z = 0; z < m_iNumPatch; z++)
		{
			px = m_pat_LBPos.x + x;
			pz = m_pat_LBPos.y + z;

			if (px < 0 || pz < 0 || px >= m_pat_MapSize || pz >= m_pat_MapSize)
				continue;

			m_ppPatch[x][z].m_ti_LBPoint.x = px * PATCH_TILE_SIZE;
			m_ppPatch[x][z].m_ti_LBPoint.y = pz * PATCH_TILE_SIZE;

			cx                             = px * PATCH_PIXEL_SIZE / COLORMAPTEX_SIZE;
			cz                             = pz * PATCH_PIXEL_SIZE / COLORMAPTEX_SIZE;
			if (cx < 0 || cz < 0 || cx >= m_iNumColorMap || cz >= m_iNumColorMap)
				m_ppPatch[x][z].m_pRefColorTex = nullptr;
			else
				m_ppPatch[x][z].m_pRefColorTex = &m_ColorMapTex[cx * m_iNumColorMap + cz];
		}
	}

	//lightmap읽어서 배치하고...
	//있던건 지우고...
	POINT PrevCenter  = m_pat_CenterPos;
	m_pat_CenterPos.x = m_pat_LBPos.x + (m_iNumPatch / 2);
	m_pat_CenterPos.y = m_pat_LBPos.y + (m_iNumPatch / 2);

	if (PrevCenter.x == m_pat_CenterPos.x)
	{
		if (PrevCenter.y == m_pat_CenterPos.y)
			SetLightMap(DIR_CM);
		else if (PrevCenter.y == (m_pat_CenterPos.y + 1))
			SetLightMap(DIR_CB);
		else if (PrevCenter.y == (m_pat_CenterPos.y - 1))
			SetLightMap(DIR_CT);
		else
			SetLightMap(DIR_WARP);
	}
	else if (PrevCenter.x == (m_pat_CenterPos.x - 1))
	{
		if (PrevCenter.y == m_pat_CenterPos.y)
			SetLightMap(DIR_RM);
		else if (PrevCenter.y == (m_pat_CenterPos.y + 1))
			SetLightMap(DIR_RB);
		else if (PrevCenter.y == (m_pat_CenterPos.y - 1))
			SetLightMap(DIR_RT);
		else
			SetLightMap(DIR_WARP);
	}
	else if (PrevCenter.x == (m_pat_CenterPos.x + 1))
	{
		if (PrevCenter.y == m_pat_CenterPos.y)
			SetLightMap(DIR_LM);
		else if (PrevCenter.y == (m_pat_CenterPos.y + 1))
			SetLightMap(DIR_LB);
		else if (PrevCenter.y == (m_pat_CenterPos.y - 1))
			SetLightMap(DIR_LT);
		else
			SetLightMap(DIR_WARP);
	}
	else
		SetLightMap(DIR_WARP);

	//m_pRiver->SetPatchPos(m_pat_LBPos.x+m_pat_Center2Side, m_pat_LBPos.y+m_pat_Center2Side);
}

//
//
//
void CN3Terrain::SetLightMap(int dir)
{
#ifndef _N3TOOL
	__TABLE_ZONE* pZoneData = CGameBase::s_pTbl_Zones.Find(CGameBase::s_pPlayer->m_InfoExt.iZoneCur);
	if (pZoneData == nullptr)
		return;

	FileReader file;
	if (!file.OpenExisting(pZoneData->szLightMapFN))
		return;

	int* Addr    = new int[m_pat_MapSize * m_pat_MapSize];
	int iVersion = 0;
	file.Read(&iVersion, sizeof(int));
	file.Read(&Addr[0], sizeof(int) * m_pat_MapSize * m_pat_MapSize);

	//DIR_LT = 0, DIR_CT = 1, DIR_RT = 2,
	//DIR_LM = 3, DIR_CM = 4, DIR_RM = 5,
	//DIR_LB = 6, DIR_CB = 7, DIR_RB = 8,
	//DIR_WARP = 9
	switch (dir)
	{
		case DIR_LT:
			SetLightMapPatch(0, 0, file, Addr);
			ReplaceLightMapPatch(1, 0, m_LightMapPatch[0][1]);
			ReplaceLightMapPatch(2, 0, m_LightMapPatch[1][1]);

			SetLightMapPatch(0, 1, file, Addr);
			ReplaceLightMapPatch(1, 1, m_LightMapPatch[0][2]);
			ReplaceLightMapPatch(2, 1, m_LightMapPatch[1][2]);

			SetLightMapPatch(0, 2, file, Addr);
			SetLightMapPatch(1, 2, file, Addr);
			SetLightMapPatch(2, 2, file, Addr);
			break;

		case DIR_CT:
			ReplaceLightMapPatch(0, 0, m_LightMapPatch[0][1]);
			ReplaceLightMapPatch(1, 0, m_LightMapPatch[1][1]);
			ReplaceLightMapPatch(2, 0, m_LightMapPatch[2][1]);

			ReplaceLightMapPatch(0, 1, m_LightMapPatch[0][2]);
			ReplaceLightMapPatch(1, 1, m_LightMapPatch[1][2]);
			ReplaceLightMapPatch(2, 1, m_LightMapPatch[2][2]);

			SetLightMapPatch(0, 2, file, Addr);
			SetLightMapPatch(1, 2, file, Addr);
			SetLightMapPatch(2, 2, file, Addr);
			break;

		case DIR_RT:
			ReplaceLightMapPatch(0, 0, m_LightMapPatch[1][1]);
			ReplaceLightMapPatch(1, 0, m_LightMapPatch[2][1]);
			SetLightMapPatch(2, 0, file, Addr);

			ReplaceLightMapPatch(0, 1, m_LightMapPatch[1][2]);
			ReplaceLightMapPatch(1, 1, m_LightMapPatch[2][2]);
			SetLightMapPatch(2, 1, file, Addr);

			SetLightMapPatch(0, 2, file, Addr);
			SetLightMapPatch(1, 2, file, Addr);
			SetLightMapPatch(2, 2, file, Addr);
			break;

		case DIR_LM:
			ReplaceLightMapPatch(2, 0, m_LightMapPatch[1][0]);
			ReplaceLightMapPatch(2, 1, m_LightMapPatch[1][1]);
			ReplaceLightMapPatch(2, 2, m_LightMapPatch[1][2]);

			ReplaceLightMapPatch(1, 0, m_LightMapPatch[0][0]);
			ReplaceLightMapPatch(1, 1, m_LightMapPatch[0][1]);
			ReplaceLightMapPatch(1, 2, m_LightMapPatch[0][2]);

			SetLightMapPatch(0, 0, file, Addr);
			SetLightMapPatch(0, 1, file, Addr);
			SetLightMapPatch(0, 2, file, Addr);
			break;

		case DIR_WARP:
			SetLightMapPatch(0, 0, file, Addr);
			SetLightMapPatch(1, 0, file, Addr);
			SetLightMapPatch(2, 0, file, Addr);

			SetLightMapPatch(0, 1, file, Addr);
			SetLightMapPatch(1, 1, file, Addr);
			SetLightMapPatch(2, 1, file, Addr);

			SetLightMapPatch(0, 2, file, Addr);
			SetLightMapPatch(1, 2, file, Addr);
			SetLightMapPatch(2, 2, file, Addr);
			break;

		case DIR_RM:
			ReplaceLightMapPatch(0, 0, m_LightMapPatch[1][0]);
			ReplaceLightMapPatch(0, 1, m_LightMapPatch[1][1]);
			ReplaceLightMapPatch(0, 2, m_LightMapPatch[1][2]);

			ReplaceLightMapPatch(1, 0, m_LightMapPatch[2][0]);
			ReplaceLightMapPatch(1, 1, m_LightMapPatch[2][1]);
			ReplaceLightMapPatch(1, 2, m_LightMapPatch[2][2]);

			SetLightMapPatch(2, 0, file, Addr);
			SetLightMapPatch(2, 1, file, Addr);
			SetLightMapPatch(2, 2, file, Addr);
			break;

		case DIR_LB:
			ReplaceLightMapPatch(2, 2, m_LightMapPatch[1][1]);
			ReplaceLightMapPatch(1, 2, m_LightMapPatch[0][1]);
			SetLightMapPatch(0, 2, file, Addr);

			ReplaceLightMapPatch(2, 1, m_LightMapPatch[1][0]);
			ReplaceLightMapPatch(1, 1, m_LightMapPatch[0][0]);
			SetLightMapPatch(0, 1, file, Addr);

			SetLightMapPatch(0, 0, file, Addr);
			SetLightMapPatch(1, 0, file, Addr);
			SetLightMapPatch(2, 0, file, Addr);
			break;

		case DIR_CB:
			ReplaceLightMapPatch(0, 2, m_LightMapPatch[0][1]);
			ReplaceLightMapPatch(1, 2, m_LightMapPatch[1][1]);
			ReplaceLightMapPatch(2, 2, m_LightMapPatch[2][1]);

			ReplaceLightMapPatch(0, 1, m_LightMapPatch[0][0]);
			ReplaceLightMapPatch(1, 1, m_LightMapPatch[1][0]);
			ReplaceLightMapPatch(2, 1, m_LightMapPatch[2][0]);

			SetLightMapPatch(0, 0, file, Addr);
			SetLightMapPatch(1, 0, file, Addr);
			SetLightMapPatch(2, 0, file, Addr);
			break;

		case DIR_RB:
			ReplaceLightMapPatch(0, 2, m_LightMapPatch[1][1]);
			ReplaceLightMapPatch(1, 2, m_LightMapPatch[2][1]);
			SetLightMapPatch(2, 2, file, Addr);

			ReplaceLightMapPatch(0, 1, m_LightMapPatch[1][0]);
			ReplaceLightMapPatch(1, 1, m_LightMapPatch[2][0]);
			SetLightMapPatch(2, 1, file, Addr);

			SetLightMapPatch(0, 0, file, Addr);
			SetLightMapPatch(1, 0, file, Addr);
			SetLightMapPatch(2, 0, file, Addr);
			break;

		default:
			break;
	}

	delete[] Addr;
#endif
}

//
//
//
void CN3Terrain::ReplaceLightMapPatch(int x, int z, stlMap_N3Tex& LightMapPatch)
{
	stlMap_N3TexIt itBegin = m_LightMapPatch[x][z].begin();
	stlMap_N3TexIt itEnd   = m_LightMapPatch[x][z].end();
	stlMap_N3TexIt it;

	for (it = itBegin; it != itEnd; it++)
	{
		CN3Texture* pTex = (*it).second;
		if (pTex)
			delete pTex;
	}
	m_LightMapPatch[x][z].clear();
	m_LightMapPatch[x][z] = LightMapPatch;
	LightMapPatch.clear();
}

//
//
//
void CN3Terrain::SetLightMapPatch(int x, int z, File& file, int* pAddr)
{
	for (auto& [_, pTex] : m_LightMapPatch[x][z])
		delete pTex;
	m_LightMapPatch[x][z].clear();

	int px = 0, pz = 0;
	px = m_pat_CenterPos.x - 1 + x;
	pz = m_pat_CenterPos.y - 1 + z;

	if (px < 0 || px >= m_pat_MapSize || pz < 0 || pz >= m_pat_MapSize)
		return;

	int jump = pAddr[px + (m_pat_MapSize * pz)];
	if (jump <= 0)
		return;

	file.Seek(jump, SEEK_SET);

	int TexCount = 0;
	file.Read(&TexCount, sizeof(int));

	int tx = 0, tz = 0;
	int rtx = 0, rtz = 0;
	for (int i = 0; i < TexCount; i++)
	{
		file.Read(&tx, sizeof(int));
		file.Read(&tz, sizeof(int));

		CN3Texture* pTex           = new CN3Texture;
		pTex->m_iFileFormatVersion = m_iFileFormatVersion;
		pTex->Load(file);

		rtx          = px * PATCH_TILE_SIZE + tx;
		rtz          = pz * PATCH_TILE_SIZE + tz;

		uint32_t key = rtx * 10000 + rtz;
		m_LightMapPatch[x][z].insert(std::make_pair(key, pTex));
	}
}

//
//
//
CN3Texture* CN3Terrain::GetLightMap(int tx, int tz)
{
	int px = 0, pz = 0;
	px  = tx / PATCH_TILE_SIZE;
	pz  = tz / PATCH_TILE_SIZE;

	px -= (m_pat_CenterPos.x - 1);
	pz -= (m_pat_CenterPos.y - 1);
	if (px < 0 || px > 2 || pz < 0 || pz > 2)
		return nullptr;

	uint32_t key      = tx * 10000 + tz;
	stlMap_N3TexIt it = m_LightMapPatch[px][pz].find(key);
	if (it != m_LightMapPatch[px][pz].end())
	{
		return (*it).second;
	}
	return nullptr;
}

//
//	CheckBounce...
//	패치단위의 가시영역 검사..
//	변했으면 return true...
//
bool CN3Terrain::CheckBound()
{
	RECT prevPatRc = m_pat_BoundRect;

	RECT rc;
	rc.left = rc.right = Real2Patch(CN3Base::s_CameraData.vEye.x);
	rc.top = rc.bottom = Real2Patch(CN3Base::s_CameraData.vEye.z);

	// 사면체의 법선 벡터와 Far 네 귀퉁이 위치 계산..
	float fS           = sinf(CN3Base::s_CameraData.fFOV / 2.0f);
	float fPL          = CN3Base::s_CameraData.fFP;
	float fAspect      = CN3Base::s_CameraData.fAspect; // 종횡비

	// Far Plane 의 네 귀퉁이 위치 계산
	__Vector3 vFPs[4]  = { __Vector3(fPL * -fS * fAspect, fPL * fS, fPL), // LeftTop
		 __Vector3(fPL * fS * fAspect, fPL * fS, fPL),                    // rightTop
		 __Vector3(fPL * fS * fAspect, fPL * -fS, fPL),                   // RightBottom
		 __Vector3(fPL * -fS * fAspect, fPL * -fS, fPL) };                // LeftBottom
	// 귀퉁이 위치에 회전 행렬을 적용한다..
	for (int i = 0; i < 4; i++)
		vFPs[i] = vFPs[i] * CN3Base::s_CameraData.mtxViewInverse;

	for (int i = 0; i < 4; i++)
	{
		POINT FarPoint;
		FarPoint.x = Real2Patch(vFPs[i].x);
		FarPoint.y = Real2Patch(vFPs[i].z);

		if (FarPoint.x < rc.left)
			rc.left = FarPoint.x;
		if (FarPoint.x > rc.right)
			rc.right = FarPoint.x;
		if (FarPoint.y < rc.top)
			rc.top = FarPoint.y;
		if (FarPoint.y > rc.bottom)
			rc.bottom = FarPoint.y;
	}

	rc.left   = rc.left - m_pat_LBPos.x - 1;
	rc.right  = rc.right - m_pat_LBPos.x + 1;
	rc.top    = rc.top - m_pat_LBPos.y - 1;
	rc.bottom = rc.bottom - m_pat_LBPos.y + 1;

	if (rc.left < 0)
		rc.left = 0;
	if (rc.left >= m_iNumPatch)
		rc.left = m_iNumPatch - 1;
	if (rc.right < 0)
		rc.right = 0;
	if (rc.right >= m_iNumPatch)
		rc.right = m_iNumPatch - 1;
	if (rc.top < 0)
		rc.top = 0;
	if (rc.top >= m_iNumPatch)
		rc.top = m_iNumPatch - 1;
	if (rc.bottom < 0)
		rc.bottom = 0;
	if (rc.bottom >= m_iNumPatch)
		rc.bottom = m_iNumPatch - 1;

	m_pat_BoundRect    = rc;

	bool bChangeRender = CheckRenderablePatch();

	if (!bChangeRender && EqualRect(&m_pat_BoundRect, &prevPatRc))
		return false;

	return true;
}

//
//
//
bool CN3Terrain::CheckRenderablePatch()
{
	bool bChange = false;
	__Vector3 CenterPoint;
	int px = 0, pz = 0;
	int x = 0, z = 0;
	for (x = m_pat_BoundRect.left; x <= m_pat_BoundRect.right; x++)
	{
		for (z = m_pat_BoundRect.top; z <= m_pat_BoundRect.bottom; z++)
		{
			if (x < 0 || z < 0)
				continue;

			BOOL PrevState              = m_ppPatch[x][z].m_bIsRender;
			m_ppPatch[x][z].m_bIsRender = TRUE;

			px                          = m_pat_LBPos.x + x;
			pz                          = m_pat_LBPos.y + z;

			if (px < 0 || pz < 0 || px >= m_pat_MapSize || pz >= m_pat_MapSize)
			{
				m_ppPatch[x][z].m_bIsRender = FALSE;
				continue;
			}

			CenterPoint.Set((float) ((m_ppPatch[x][z].m_ti_LBPoint.x + (PATCH_TILE_SIZE >> 1)) * TILE_SIZE), m_ppPatchMiddleY[px][pz],
				(float) ((m_ppPatch[x][z].m_ti_LBPoint.y + (PATCH_TILE_SIZE >> 1)) * TILE_SIZE));

			m_ppPatch[x][z].m_bIsRender = (!(CN3Base::s_CameraData.IsOutOfFrustum(CenterPoint, m_ppPatchRadius[px][pz] * 2.0f)));
			if (m_ppPatch[x][z].m_bIsRender != PrevState)
				bChange = true;
		}
	}
	return bChange;
}

//
//
//
void CN3Terrain::Render()
{
	__Matrix44 WorldMtx;
	WorldMtx.Identity();
	s_lpD3DDev->SetTransform(D3DTS_WORLD, WorldMtx.toD3D());

	__Material mtl;
	mtl.Init();
	s_lpD3DDev->SetMaterial(&mtl);

	s_lpD3DDev->SetRenderState(D3DRS_FILLMODE, m_FillMode);
	s_lpD3DDev->SetRenderState(D3DRS_SHADEMODE, m_ShadeMode);

	DWORD CullMode = 0, ZEnable = 0;
	s_lpD3DDev->GetRenderState(D3DRS_CULLMODE, &CullMode);
	s_lpD3DDev->GetRenderState(D3DRS_ZENABLE, &ZEnable);

	s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	//s_lpD3DDev->SetRenderState(D3DRS_ZBIAS, 1);

	DWORD ColorOP0 = 0, ColorOP1 = 0, ColorOP2 = 0;
	DWORD ColorArg01 = 0, ColorArg02 = 0, ColorArg11 = 0, ColorArg12 = 0, ColorArg21 = 0, ColorArg22 = 0;

	s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLOROP, &ColorOP0);
	s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLORARG1, &ColorArg01);
	s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLORARG2, &ColorArg02);
	s_lpD3DDev->GetTextureStageState(1, D3DTSS_COLOROP, &ColorOP1);
	s_lpD3DDev->GetTextureStageState(1, D3DTSS_COLORARG1, &ColorArg11);
	s_lpD3DDev->GetTextureStageState(1, D3DTSS_COLORARG2, &ColorArg12);
	s_lpD3DDev->GetTextureStageState(2, D3DTSS_COLOROP, &ColorOP2);
	s_lpD3DDev->GetTextureStageState(2, D3DTSS_COLORARG1, &ColorArg21);
	s_lpD3DDev->GetTextureStageState(2, D3DTSS_COLORARG2, &ColorArg22);

	DWORD AddressU1 = 0, AddressV1 = 0, AddressU2 = 0, AddressV2 = 0;
	// s_lpD3DDev->GetTextureStageState( 0, D3DTSS_ADDRESSU, &AddressU1 );
	// s_lpD3DDev->GetTextureStageState( 0, D3DTSS_ADDRESSV, &AddressV1 );
	// s_lpD3DDev->GetTextureStageState(1, D3DTSS_ADDRESSU, &AddressU2);
	// s_lpD3DDev->GetTextureStageState(1, D3DTSS_ADDRESSV, &AddressV2);
	s_lpD3DDev->GetSamplerState(0, D3DSAMP_ADDRESSU, &AddressU1);
	s_lpD3DDev->GetSamplerState(0, D3DSAMP_ADDRESSV, &AddressV1);
	s_lpD3DDev->GetSamplerState(1, D3DSAMP_ADDRESSU, &AddressU2);
	s_lpD3DDev->GetSamplerState(1, D3DSAMP_ADDRESSV, &AddressV2);

	// 각각의 텍스쳐들을 연결했을때 경계선을 없앨 수 있다..^^
	// s_lpD3DDev->SetTextureStageState( 0, D3DTSS_ADDRESSU,  D3DTADDRESS_MIRROR );
	// s_lpD3DDev->SetTextureStageState( 0, D3DTSS_ADDRESSV,  D3DTADDRESS_MIRROR );
	// s_lpD3DDev->SetTextureStageState( 1, D3DTSS_ADDRESSU,  D3DTADDRESS_MIRROR );
	// s_lpD3DDev->SetTextureStageState( 1, D3DTSS_ADDRESSV,  D3DTADDRESS_MIRROR );
	s_lpD3DDev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);
	s_lpD3DDev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);
	s_lpD3DDev->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);
	s_lpD3DDev->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);

	for (int x = m_pat_BoundRect.left; x <= m_pat_BoundRect.right; x++)
	{
		for (int z = m_pat_BoundRect.top; z <= m_pat_BoundRect.bottom; z++)
		{
			if (x < 0 || z < 0)
				continue;

			m_ppPatch[x][z].Render();
		}
	}

	/*
	 s_lpD3DDev->SetTextureStageState( 0, D3DTSS_ADDRESSU, AddressU1 );
	 s_lpD3DDev->SetTextureStageState( 0, D3DTSS_ADDRESSV, AddressV1 );
	 s_lpD3DDev->SetTextureStageState( 1, D3DTSS_ADDRESSU, AddressU2 );
	 s_lpD3DDev->SetTextureStageState( 1, D3DTSS_ADDRESSV, AddressV2 );
	*/
	s_lpD3DDev->SetSamplerState(0, D3DSAMP_ADDRESSU, AddressU1);
	s_lpD3DDev->SetSamplerState(0, D3DSAMP_ADDRESSV, AddressV1);
	s_lpD3DDev->SetSamplerState(1, D3DSAMP_ADDRESSU, AddressU2);
	s_lpD3DDev->SetSamplerState(1, D3DSAMP_ADDRESSV, AddressV2);

	// restor texture stage state settings...
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, ColorOP0);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, ColorArg01);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG2, ColorArg02);
	s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLOROP, ColorOP1);
	s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLORARG1, ColorArg11);
	s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLORARG2, ColorArg12);
	s_lpD3DDev->SetTextureStageState(2, D3DTSS_COLOROP, ColorOP2);
	s_lpD3DDev->SetTextureStageState(2, D3DTSS_COLORARG1, ColorArg21);
	s_lpD3DDev->SetTextureStageState(2, D3DTSS_COLORARG2, ColorArg22);
	s_lpD3DDev->SetTexture(0, nullptr);
	s_lpD3DDev->SetTexture(1, nullptr);
	s_lpD3DDev->SetTexture(2, nullptr);

	s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, CullMode);
	s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, ZEnable);

	if (m_pRiver != nullptr)
		m_pRiver->Render();

	if (m_pPond != nullptr)
		m_pPond->Render();
}

//
//	Log2(x) = l..
//	2의 승수에 대해서만 제대로 작동...(x>0)
//
inline int CN3Terrain::Log2(int x)
{
	int l = 0;
	while (x != 1)
	{
		x = x >> 1;
		l++;
	}
	return l;
}

//
//
//
float CN3Terrain::GetHeight(float x, float z)
{
	int ix = 0, iz = 0;
	ix = ((int) x) / (int) TILE_SIZE;
	iz = ((int) z) / (int) TILE_SIZE;

	if (ix < 0 || ix > (m_ti_MapSize - 2))
		return -FLT_MAX;
	if (iz < 0 || iz > (m_ti_MapSize - 2))
		return -FLT_MAX;

	float dX = 0.0f, dZ = 0.0f;
	dX      = (x - (ix * TILE_SIZE)) / TILE_SIZE;
	dZ      = (z - (iz * TILE_SIZE)) / TILE_SIZE;

	float y = 0.0f, h1 = 0.0f, h2 = 0.0f, h3 = 0.0f, h12 = 0.0f, h13 = 0.0f;

	if ((ix + iz) % 2 == 0) //사각형이 / 모양..
	{
		h1 = m_pMapData[ix * m_ti_MapSize + iz].fHeight;
		h3 = m_pMapData[(ix + 1) * m_ti_MapSize + (iz + 1)].fHeight;
		if (dZ > dX) //윗쪽 삼각형..
		{
			h2  = m_pMapData[ix * m_ti_MapSize + (iz + 1)].fHeight;

			h12 = h1 + (h2 - h1) * dZ;             // h1과 h2사이의 높이값
			h13 = h1 + (h3 - h1) * dZ;             // h1과 h3사이의 높이값

			y   = h12 + ((h13 - h12) * (dX / dZ)); // 찾고자 하는 높이값
			return y;
		}
		else                                       //아래쪽 삼각형..
		{
			if (dX == 0.0f)
				return h1;

			h2  = m_pMapData[(ix + 1) * m_ti_MapSize + iz].fHeight;

			h12 = h1 + (h2 - h1) * dX;             // h1과 h2사이의 높이값
			h13 = h1 + (h3 - h1) * dX;             // h1과 h3사이의 높이값

			y   = h12 + ((h13 - h12) * (dZ / dX)); // 찾고자 하는 높이값
			return y;
		}
	}

	else if ((ix + iz) % 2 == 1) //사각형이 역슬레쉬 모양..
	{
		h1 = m_pMapData[(ix + 1) * m_ti_MapSize + iz].fHeight;
		h3 = m_pMapData[ix * m_ti_MapSize + (iz + 1)].fHeight;

		if ((dX + dZ) > 1.0f) //윗쪽 삼각형..
		{
			if (dZ == 0.0f)
				return h1;
			h2  = m_pMapData[(ix + 1) * m_ti_MapSize + (iz + 1)].fHeight;

			h12 = h1 + (h2 - h1) * dZ;
			h13 = h1 + (h3 - h1) * dZ;

			y   = h12 + ((h13 - h12) * ((1.0f - dX) / dZ));
			return y;
		}
		else //아래쪽 삼각형..
		{
			if (dX == 1.0f)
				return h1;
			h2  = m_pMapData[ix * m_ti_MapSize + iz].fHeight;

			h12 = h2 + (h1 - h2) * dX; // h1과 h2사이의 높이값
			h13 = h3 + (h1 - h3) * dX; // h1과 h3사이의 높이값

			y   = h12 + ((h13 - h12) * (dZ / (1.0f - dX)));
			return y;
		}
	}

	return -FLT_MAX;
}

//
//	GetNormal(float x, float z, __Vector3& vNormal)
//
void CN3Terrain::GetNormal(float x, float z, __Vector3& vNormal)
{
	if (x < 10.0f || x > ((m_ti_MapSize - 1) * TILE_SIZE - 10.0f) || z < 10.0f || z > ((m_ti_MapSize - 1) * TILE_SIZE - 10.0f))
	{
		vNormal.Set(0.0f, 1.0f, 0.0f);
		return;
	}

	int ix = 0, iz = 0;
	ix       = ((int) x) / (int) TILE_SIZE;
	iz       = ((int) z) / (int) TILE_SIZE;

	float dX = 0.0f, dZ = 0.0f;
	dX = (x - ix * TILE_SIZE) / TILE_SIZE;
	dZ = (z - iz * TILE_SIZE) / TILE_SIZE;

	__Vector3 v1, v2;
	vNormal.Set(0, 1, 0);

	float Height = 0.0f;
	if ((ix + iz) % 2 == 1)
	{
		if ((dX + dZ) < 1.0f)
		{
			Height = m_pMapData[ix * m_ti_MapSize + (iz + 1)].fHeight - m_pMapData[ix * m_ti_MapSize + iz].fHeight;
			v1.Set(0, Height, TILE_SIZE);

			Height = m_pMapData[(ix + 1) * m_ti_MapSize + iz].fHeight - m_pMapData[ix * m_ti_MapSize + iz].fHeight;
			v2.Set(TILE_SIZE, Height, 0);

			vNormal.Cross(v1, v2);
			return;
		}
		else
		{
			Height = m_pMapData[(ix + 1) * m_ti_MapSize + iz].fHeight - m_pMapData[(ix + 1) * m_ti_MapSize + (iz + 1)].fHeight;
			v1.Set(0.0f, Height, (-1) * TILE_SIZE);

			Height = m_pMapData[ix * m_ti_MapSize + (iz + 1)].fHeight - m_pMapData[(ix + 1) * m_ti_MapSize + (iz + 1)].fHeight;
			v2.Set((-1) * TILE_SIZE, Height, 0.0f);

			vNormal.Cross(v1, v2);
			return;
		}
	}
	else
	{
		if (dZ > dX)
		{
			Height = m_pMapData[(ix + 1) * m_ti_MapSize + (iz + 1)].fHeight - m_pMapData[ix * m_ti_MapSize + (iz + 1)].fHeight;
			v1.Set(TILE_SIZE, Height, 0.0f);

			Height = m_pMapData[ix * m_ti_MapSize + iz].fHeight - m_pMapData[ix * m_ti_MapSize + (iz + 1)].fHeight;
			v2.Set(0.0f, Height, (-1) * TILE_SIZE);

			vNormal.Cross(v1, v2);
			return;
		}
		else
		{
			Height = m_pMapData[ix * m_ti_MapSize + iz].fHeight - m_pMapData[(ix + 1) * m_ti_MapSize + iz].fHeight;
			v1.Set((-1) * TILE_SIZE, Height, 0.0f);

			Height = m_pMapData[(ix + 1) * m_ti_MapSize + (iz + 1)].fHeight - m_pMapData[(ix + 1) * m_ti_MapSize + iz].fHeight;
			v2.Set(0.0f, Height, TILE_SIZE);

			vNormal.Cross(v1, v2);
			return;
		}
	}
	return;
}

//
//
//
bool CN3Terrain::IsInTerrain(float x, float z)
{
	if (x < 30.0f || x > ((m_ti_MapSize - 1) * TILE_SIZE - 30.0f) || z < 30.0f || z > ((m_ti_MapSize - 1) * TILE_SIZE - 30.0f))
		return false;
	return true;
}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////
//

BOOL CN3Terrain::Pick(int x, int y, __Vector3& vPick)
{
	constexpr float OFFSET_COLLISION_TERRAIN = 0.5f;

	// Compute the vector of the pick ray in screen space
	__Vector3 vTmp;
	vTmp.x             = (((2.0f * x) / (CN3Base::s_CameraData.vp.Width)) - 1) / CN3Base::s_CameraData.mtxProjection.m[0][0];
	vTmp.y             = -(((2.0f * y) / (CN3Base::s_CameraData.vp.Height)) - 1) / CN3Base::s_CameraData.mtxProjection.m[1][1];
	vTmp.z             = 1.0f;

	// Transform the screen space pick ray into 3D space
	__Matrix44* pMtxVI = &CN3Base::s_CameraData.mtxViewInverse;
	__Vector3 vDir;
	vDir.x          = vTmp.x * pMtxVI->m[0][0] + vTmp.y * pMtxVI->m[1][0] + vTmp.z * pMtxVI->m[2][0];
	vDir.y          = vTmp.x * pMtxVI->m[0][1] + vTmp.y * pMtxVI->m[1][1] + vTmp.z * pMtxVI->m[2][1];
	vDir.z          = vTmp.x * pMtxVI->m[0][2] + vTmp.y * pMtxVI->m[1][2] + vTmp.z * pMtxVI->m[2][2];
	__Vector3 vPos  = pMtxVI->Pos();

	bool bCollision = FALSE;
	__Vector3 A, B, C;
	float t = 0.0f, u = 0.0f, v = 0.0f;

	int ix = ((int) vPos.x) / (int) TILE_SIZE;
	int iz = ((int) vPos.z) / (int) TILE_SIZE;

	if ((ix + iz) % 2 == 1) // 당근.. 왼손 바인딩...
	{
		A.Set((float) ix * TILE_SIZE, GetHeight(ix * TILE_SIZE, iz * TILE_SIZE), (float) iz * TILE_SIZE);
		C.Set((float) (ix + 1) * TILE_SIZE, GetHeight((ix + 1) * TILE_SIZE, iz * TILE_SIZE), (float) iz * TILE_SIZE);
		B.Set((float) ix * TILE_SIZE, GetHeight(ix * TILE_SIZE, (iz + 1) * TILE_SIZE), (float) (iz + 1) * TILE_SIZE);
		A.y += OFFSET_COLLISION_TERRAIN;
		C.y += OFFSET_COLLISION_TERRAIN;
		B.y += OFFSET_COLLISION_TERRAIN;
	}
	if ((ix + iz) % 2 == 0)
	{
		A.Set((float) ix * TILE_SIZE, GetHeight(ix * TILE_SIZE, (iz + 1) * TILE_SIZE), (float) (iz + 1) * TILE_SIZE);
		C.Set((float) ix * TILE_SIZE, GetHeight(ix * TILE_SIZE, iz * TILE_SIZE), (float) iz * TILE_SIZE);
		B.Set((float) (ix + 1) * TILE_SIZE, GetHeight((ix + 1) * TILE_SIZE, (iz + 1) * TILE_SIZE), (float) (iz + 1) * TILE_SIZE);
		A.y += OFFSET_COLLISION_TERRAIN;
		C.y += OFFSET_COLLISION_TERRAIN;
		B.y += OFFSET_COLLISION_TERRAIN;
	}
	bCollision = ::_IntersectTriangle(vPos, vDir, A, B, C, t, u, v, &vPick);

	if (FALSE == bCollision) // 충돌점이 없을 경우....
	{
		vPick.Set(0, 0, 0);  // 일단 충돌 점은 없고..

		// 음....		!!가상!!  버텍스 버퍼와 인덱스 버퍼 만들기..
		__Vector3 AA[8] {}; // 가상 버텍스 버퍼..
		int pIndex[36] {};  // 가상 인덱스 버퍼..
		int* pIdx = pIndex;

		AA[0]     = __Vector3(vPos.x - COLLISION_BOX, vPos.y - COLLISION_BOX, vPos.z + COLLISION_BOX);
		AA[1]     = __Vector3(vPos.x + COLLISION_BOX, vPos.y - COLLISION_BOX, vPos.z + COLLISION_BOX);
		AA[2]     = __Vector3(vPos.x + COLLISION_BOX, vPos.y - COLLISION_BOX, vPos.z - COLLISION_BOX);
		AA[3]     = __Vector3(vPos.x - COLLISION_BOX, vPos.y - COLLISION_BOX, vPos.z - COLLISION_BOX);
		AA[4]     = __Vector3(vPos.x - COLLISION_BOX, vPos.y + COLLISION_BOX, vPos.z + COLLISION_BOX);
		AA[5]     = __Vector3(vPos.x + COLLISION_BOX, vPos.y + COLLISION_BOX, vPos.z + COLLISION_BOX);
		AA[6]     = __Vector3(vPos.x + COLLISION_BOX, vPos.y + COLLISION_BOX, vPos.z - COLLISION_BOX);
		AA[7]     = __Vector3(vPos.x - COLLISION_BOX, vPos.y + COLLISION_BOX, vPos.z - COLLISION_BOX);

		// 윗면.
		*pIdx++   = 0;
		*pIdx++   = 1;
		*pIdx++   = 3;
		*pIdx++   = 2;
		*pIdx++   = 3;
		*pIdx++   = 1;

		// 앞면..
		*pIdx++   = 7;
		*pIdx++   = 3;
		*pIdx++   = 6;
		*pIdx++   = 2;
		*pIdx++   = 6;
		*pIdx++   = 3;

		// 왼쪽..
		*pIdx++   = 4;
		*pIdx++   = 0;
		*pIdx++   = 7;
		*pIdx++   = 3;
		*pIdx++   = 7;
		*pIdx++   = 0;

		// 오른쪽..
		*pIdx++   = 6;
		*pIdx++   = 2;
		*pIdx++   = 5;
		*pIdx++   = 1;
		*pIdx++   = 5;
		*pIdx++   = 2;

		// 뒷면..
		*pIdx++   = 5;
		*pIdx++   = 1;
		*pIdx++   = 4;
		*pIdx++   = 0;
		*pIdx++   = 4;
		*pIdx++   = 1;

		// 밑면..
		*pIdx++   = 7;
		*pIdx++   = 6;
		*pIdx++   = 4;
		*pIdx++   = 5;
		*pIdx++   = 4;
		*pIdx++   = 6;

		float t = 0.0f, u = 0.0f, v = 0.0f;
		for (int i = 0; !bCollision && i < 36; i += 3)
			bCollision = ::_IntersectTriangle(vPos, vDir, AA[pIndex[i]], AA[pIndex[i + 1]], AA[pIndex[i + 2]], t, u, v, &vPick);
	}

	return bCollision;
}

BOOL CN3Terrain::PickWide(int x, int y, __Vector3& vPick)
{
	constexpr float COL_BOX_OFF = 2000.0f;

	// Compute the vector of the pick ray in screen space
	__Vector3 vTmp;
	vTmp.x             = (((2.0f * x) / (CN3Base::s_CameraData.vp.Width)) - 1) / CN3Base::s_CameraData.mtxProjection.m[0][0];
	vTmp.y             = -(((2.0f * y) / (CN3Base::s_CameraData.vp.Height)) - 1) / CN3Base::s_CameraData.mtxProjection.m[1][1];
	vTmp.z             = 1.0f;

	// Transform the screen space pick ray into 3D space
	__Matrix44* pMtxVI = &CN3Base::s_CameraData.mtxViewInverse;
	__Vector3 vDir;
	vDir.x            = vTmp.x * pMtxVI->m[0][0] + vTmp.y * pMtxVI->m[1][0] + vTmp.z * pMtxVI->m[2][0];
	vDir.y            = vTmp.x * pMtxVI->m[0][1] + vTmp.y * pMtxVI->m[1][1] + vTmp.z * pMtxVI->m[2][1];
	vDir.z            = vTmp.x * pMtxVI->m[0][2] + vTmp.y * pMtxVI->m[1][2] + vTmp.z * pMtxVI->m[2][2];
	__Vector3 vPos    = pMtxVI->Pos();
	__Vector3 vPosCur = vPos;

	vDir.Normalize();

	bool bCollision = FALSE;
	__Vector3 A, B, C;
	float t = 0.0f, u = 0.0f, v = 0.0f;

	while ((vPosCur.x >= 0.0f) && (vPosCur.z >= 0.0f) && (IsInTerrain(vPosCur.x, vPosCur.z)))
	{
		if (bCollision)
			return bCollision;

		int ix = ((int) vPosCur.x) / (int) TILE_SIZE;
		int iz = ((int) vPosCur.z) / (int) TILE_SIZE;

		for (int i = 0; i < 10; i++)
		{
			switch (i)
			{
				case 0:   //  0, 0
					break;
				case 1:
					ix--; // -1, 0
					break;
				case 2:   //  0, -1
					ix++;
					iz--;
					break;
				case 3: // -1, -1
					ix--;
					break;
				case 4: //  1, -1
					ix++;
					ix++;
					break;
				case 5: // -1, 1
					ix--;
					ix--;
					iz++;
					iz++;
					break;
				case 6: //  0, 1
				case 7: //  1, 1
					ix++;
					break;
				case 8: //  1, 0
					iz--;
					break;
				case 9: //  0, 0
					ix--;
					break;
				default:
					break;
			}

			if ((ix + iz) % 2 == 1) // 당근.. 왼손 바인딩...
			{
				A.Set((float) ix * TILE_SIZE, GetHeight(ix * TILE_SIZE, iz * TILE_SIZE), (float) iz * TILE_SIZE);
				C.Set((float) (ix + 1) * TILE_SIZE, GetHeight((ix + 1) * TILE_SIZE, iz * TILE_SIZE), (float) iz * TILE_SIZE);
				B.Set((float) ix * TILE_SIZE, GetHeight(ix * TILE_SIZE, (iz + 1) * TILE_SIZE), (float) (iz + 1) * TILE_SIZE);

				bCollision = ::_IntersectTriangle(vPos, vDir, A, B, C, t, u, v, &vPick);
				if (bCollision)
					break;

				A.Set((float) (ix + 1) * TILE_SIZE, GetHeight((ix + 1) * TILE_SIZE, (iz + 1) * TILE_SIZE), (float) (iz + 1) * TILE_SIZE);
				B.Set((float) (ix + 1) * TILE_SIZE, GetHeight((ix + 1) * TILE_SIZE, iz * TILE_SIZE), (float) iz * TILE_SIZE);
				C.Set((float) ix * TILE_SIZE, GetHeight(ix * TILE_SIZE, (iz + 1) * TILE_SIZE), (float) (iz + 1) * TILE_SIZE);

				bCollision = ::_IntersectTriangle(vPos, vDir, A, B, C, t, u, v, &vPick);
				if (bCollision)
					break;
			}

			if ((ix + iz) % 2 == 0)
			{
				A.Set((float) ix * TILE_SIZE, GetHeight(ix * TILE_SIZE, (iz + 1) * TILE_SIZE), (float) (iz + 1) * TILE_SIZE);
				C.Set((float) ix * TILE_SIZE, GetHeight(ix * TILE_SIZE, iz * TILE_SIZE), (float) iz * TILE_SIZE);
				B.Set((float) (ix + 1) * TILE_SIZE, GetHeight((ix + 1) * TILE_SIZE, (iz + 1) * TILE_SIZE), (float) (iz + 1) * TILE_SIZE);

				bCollision = ::_IntersectTriangle(vPos, vDir, A, B, C, t, u, v, &vPick);
				if (bCollision)
					break;

				A.Set((float) (ix + 1) * TILE_SIZE, GetHeight((ix + 1) * TILE_SIZE, iz * TILE_SIZE), (float) iz * TILE_SIZE);
				B.Set((float) ix * TILE_SIZE, GetHeight(ix * TILE_SIZE, iz * TILE_SIZE), (float) iz * TILE_SIZE);
				C.Set((float) (ix + 1) * TILE_SIZE, GetHeight((ix + 1) * TILE_SIZE, (iz + 1) * TILE_SIZE), (float) (iz + 1) * TILE_SIZE);

				bCollision = ::_IntersectTriangle(vPos, vDir, A, B, C, t, u, v, &vPick);
				if (bCollision)
					break;
			}
		}
		vPosCur += (vDir * TILE_SIZE);
		//vDir 크기가 작기 때문에 Nomalize하고 TILE_SIZE만큼 곱해서 다음 체크할 위치를 바꿔준다.
		//이렇게 하지 않으면 체크한 부분을 여러번 체크하기 때문에 부하가 커진다.
	}

	if (!bCollision)        // 충돌점이 없을 경우....
	{
		vPick.Set(0, 0, 0); // 일단 충돌 점은 없고..

		// 음....		!!가상!!  버텍스 버퍼와 인덱스 버퍼 만들기..
		__Vector3 AA[8] {}; // 가상 버텍스 버퍼..
		int pIndex[36] {};  // 가상 인덱스 버퍼..
		int* pIdx = pIndex;

		AA[0]     = __Vector3(vPos.x - COL_BOX_OFF, vPos.y - COL_BOX_OFF, vPos.z + COL_BOX_OFF);
		AA[1]     = __Vector3(vPos.x + COL_BOX_OFF, vPos.y - COL_BOX_OFF, vPos.z + COL_BOX_OFF);
		AA[2]     = __Vector3(vPos.x + COL_BOX_OFF, vPos.y - COL_BOX_OFF, vPos.z - COL_BOX_OFF);
		AA[3]     = __Vector3(vPos.x - COL_BOX_OFF, vPos.y - COL_BOX_OFF, vPos.z - COL_BOX_OFF);
		AA[4]     = __Vector3(vPos.x - COL_BOX_OFF, vPos.y + COL_BOX_OFF, vPos.z + COL_BOX_OFF);
		AA[5]     = __Vector3(vPos.x + COL_BOX_OFF, vPos.y + COL_BOX_OFF, vPos.z + COL_BOX_OFF);
		AA[6]     = __Vector3(vPos.x + COL_BOX_OFF, vPos.y + COL_BOX_OFF, vPos.z - COL_BOX_OFF);
		AA[7]     = __Vector3(vPos.x - COL_BOX_OFF, vPos.y + COL_BOX_OFF, vPos.z - COL_BOX_OFF);

		// 윗면.
		*pIdx++   = 0;
		*pIdx++   = 1;
		*pIdx++   = 3;
		*pIdx++   = 2;
		*pIdx++   = 3;
		*pIdx++   = 1;

		// 앞면..
		*pIdx++   = 7;
		*pIdx++   = 3;
		*pIdx++   = 6;
		*pIdx++   = 2;
		*pIdx++   = 6;
		*pIdx++   = 3;

		// 왼쪽..
		*pIdx++   = 4;
		*pIdx++   = 0;
		*pIdx++   = 7;
		*pIdx++   = 3;
		*pIdx++   = 7;
		*pIdx++   = 0;

		// 오른쪽..
		*pIdx++   = 6;
		*pIdx++   = 2;
		*pIdx++   = 5;
		*pIdx++   = 1;
		*pIdx++   = 5;
		*pIdx++   = 2;

		// 뒷면..
		*pIdx++   = 5;
		*pIdx++   = 1;
		*pIdx++   = 4;
		*pIdx++   = 0;
		*pIdx++   = 4;
		*pIdx++   = 1;

		// 밑면..
		*pIdx++   = 7;
		*pIdx++   = 6;
		*pIdx++   = 4;
		*pIdx++   = 5;
		*pIdx++   = 4;
		*pIdx++   = 6;

		float t = 0.0f, u = 0.0f, v = 0.0f;
		for (int i = 0; !bCollision && i < 36; i += 3)
			bCollision = ::_IntersectTriangle(vPos, vDir, AA[pIndex[i]], AA[pIndex[i + 1]], AA[pIndex[i + 2]], t, u, v, &vPick);
	}

	return bCollision;
}

bool CN3Terrain::CheckIncline(const __Vector3& vPos, const __Vector3& vDir, float fIncline)
{
	__Vector3 vNormal;
	this->GetNormal(vPos.x, vPos.z, vNormal);
	vNormal.Normalize();
	vNormal.y = 0.0f;
	if (vNormal.Magnitude() > fIncline && vNormal.Dot(vDir) <= 0.0f)
		return true;
	return false;
}

bool CN3Terrain::CheckCollisionCamera(__Vector3& vEyeResult, const __Vector3& vAt, float fNP)
{
	float fHeight = this->GetHeight(vEyeResult.x, vEyeResult.z);
	float fDelta  = fHeight - vEyeResult.y + fNP;
	if (fDelta < 0)
		return false;

	__Vector3 vDir = vAt - vEyeResult;
	vDir.Normalize();
	vDir.Set(0, 1, 0);

	vEyeResult += vDir * fDelta;
	return true;
}

bool CN3Terrain::CheckCollision(__Vector3& vPos, __Vector3& vDir, float fVelocity, __Vector3* vCol)
{
	float fHeight1 = 0.0f, fHeight2 = 0.0f;
	vDir.Normalize();

	fHeight1           = vPos.y - GetHeight(vPos.x, vPos.z);
	__Vector3 vNextPos = vPos + (vDir * (fVelocity * CN3Base::s_fSecPerFrm));
	fHeight2           = vNextPos.y - GetHeight(vNextPos.x, vNextPos.z);

	if (fHeight1 <= 0)
	{
		(*vCol)   = vPos;
		(*vCol).y = GetHeight(vPos.x, vPos.z) + 0.1f;
		return true;
	}

	if (fHeight1 * fHeight2 > 0)
		return false; // both would be positive

	/////////////////////////////////////////////
	//걍 덜 정밀하게 하려면..이케...
	//
	(*vCol)   = vPos;
	(*vCol).y = this->GetHeight(vPos.x, vPos.z) + 0.1f;
	return true;
}

bool CN3Terrain::LoadColorMap(const std::string& szFN)
{
	CUILoading* pUILoading = CGameProcedure::s_pUILoading; // 로딩바..

	m_iNumColorMap         = (m_pat_MapSize * PATCH_PIXEL_SIZE) / COLORMAPTEX_SIZE;

	m_ColorMapTex.clear();
	m_ColorMapTex.resize(m_iNumColorMap * m_iNumColorMap);

	FileReader colorMapFile;
	if (!colorMapFile.OpenExisting(szFN))
	{
#ifdef _N3GAME
		CLogWriter::Write("Failed to load ColorMap - {}", szFN);
#endif
		return false;
	}

	std::string szBuff;
	for (int x = 0; x < m_iNumColorMap; x++)
	{
		int idx = x * m_iNumColorMap;
		for (int z = 0; z < m_iNumColorMap; ++z, ++idx)
		{
			CN3Texture& tex          = m_ColorMapTex[idx];
			tex.m_iFileFormatVersion = m_iFileFormatVersion;
			tex.m_iLOD               = s_Options.iTexLOD_Terrain; // LOD 적용후 읽는다..
			tex.Load(colorMapFile);
		}

		szBuff = fmt::format("Loading colormap {} %", x * 100 / m_iNumColorMap);
		if (pUILoading != nullptr)
			pUILoading->Render(szBuff, 60 + 15 * x / m_iNumColorMap);
	}

	return true;
}

CN3Texture* CN3Terrain::GetTileTex(int x, int z)
{
	if (x < 0 || x >= m_ti_MapSize || z < 0 || z >= m_ti_MapSize)
		return nullptr;

	const MAPDATA& MapData = m_pMapData[(x * m_ti_MapSize) + z];
	if (MapData.Tex1Idx >= m_TileTex.size())
		return nullptr;

	return &m_TileTex[MapData.Tex1Idx];
}

bool CN3Terrain::GetTileTexInfo(float x, float z, TERRAINTILETEXINFO& TexInfo1, TERRAINTILETEXINFO& TexInfo2)
{
	int tx = 0, tz = 0;
	tx = (int) x / (int) TILE_SIZE;
	tz = (int) z / (int) TILE_SIZE;

	if (tx < 0 || tx >= m_ti_MapSize || tz < 0 || tz >= m_ti_MapSize)
		return false;

	const MAPDATA& MapData = m_pMapData[(tx * m_ti_MapSize) + tz];

	if (MapData.Tex1Idx >= m_TileTex.size())
	{
		TexInfo1.pTex = nullptr;
		TexInfo1.u = TexInfo1.v = 0.0f;
	}
	else
	{
		TexInfo1.pTex = &m_TileTex[MapData.Tex1Idx];
		TexInfo1.u = TexInfo1.v = 0.0f;
		//u1[0] = m_pRefTerrain->m_fTileDirU[dir1][2];
		//u1[1] = m_pRefTerrain->m_fTileDirU[dir1][0];
		//u1[2] = m_pRefTerrain->m_fTileDirU[dir1][1];
		//u1[3] = m_pRefTerrain->m_fTileDirU[dir1][3];

		//v1[0] = m_pRefTerrain->m_fTileDirV[dir1][2];
		//v1[1] = m_pRefTerrain->m_fTileDirV[dir1][0];
		//v1[2] = m_pRefTerrain->m_fTileDirV[dir1][1];
		//v1[3] = m_pRefTerrain->m_fTileDirV[dir1][3];

		//u2[0] = m_pRefTerrain->m_fTileDirU[dir2][2];
		//u2[1] = m_pRefTerrain->m_fTileDirU[dir2][0];
		//u2[2] = m_pRefTerrain->m_fTileDirU[dir2][1];
		//u2[3] = m_pRefTerrain->m_fTileDirU[dir2][3];

		//v2[0] = m_pRefTerrain->m_fTileDirV[dir2][2];
		//v2[1] = m_pRefTerrain->m_fTileDirV[dir2][0];
		//v2[2] = m_pRefTerrain->m_fTileDirV[dir2][1];
		//v2[3] = m_pRefTerrain->m_fTileDirV[dir2][3];
	}

	if (MapData.Tex2Idx >= m_TileTex.size())
	{
		TexInfo2.pTex = nullptr;
		TexInfo2.u = TexInfo2.v = 0.0f;
	}
	else
	{
		TexInfo2.pTex = &m_TileTex[MapData.Tex2Idx];
		TexInfo2.u = TexInfo2.v = 0.0f;
		//u1[0] = m_pRefTerrain->m_fTileDirU[dir1][2];
		//u1[1] = m_pRefTerrain->m_fTileDirU[dir1][0];
		//u1[2] = m_pRefTerrain->m_fTileDirU[dir1][1];
		//u1[3] = m_pRefTerrain->m_fTileDirU[dir1][3];

		//v1[0] = m_pRefTerrain->m_fTileDirV[dir1][2];
		//v1[1] = m_pRefTerrain->m_fTileDirV[dir1][0];
		//v1[2] = m_pRefTerrain->m_fTileDirV[dir1][1];
		//v1[3] = m_pRefTerrain->m_fTileDirV[dir1][3];

		//u2[0] = m_pRefTerrain->m_fTileDirU[dir2][2];
		//u2[1] = m_pRefTerrain->m_fTileDirU[dir2][0];
		//u2[2] = m_pRefTerrain->m_fTileDirU[dir2][1];
		//u2[3] = m_pRefTerrain->m_fTileDirU[dir2][3];

		//v2[0] = m_pRefTerrain->m_fTileDirV[dir2][2];
		//v2[1] = m_pRefTerrain->m_fTileDirV[dir2][0];
		//v2[2] = m_pRefTerrain->m_fTileDirV[dir2][1];
		//v2[3] = m_pRefTerrain->m_fTileDirV[dir2][3];
	}

	return true;
}

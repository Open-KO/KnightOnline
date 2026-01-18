// N3TerrainPatch.cpp: implementation of the CN3TerrainPatch class.
// 2001.10.22.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "N3TerrainPatch.h"
#include "N3Terrain.h"

#include <N3Base/N3Texture.h>

//
//	생성자
//
CN3TerrainPatch::CN3TerrainPatch()
{
	m_bIsRender    = FALSE;
	m_iLevel       = 0;
	m_CellSize     = 0;
	m_NumCell      = 0;
	m_ti_LBPoint.x = -1;
	m_ti_LBPoint.y = -1;
	m_pRefTerrain  = nullptr;
	for (int i = 0; i < 4; i++)
		m_IsBlunt[i] = true;

	m_pRefColorTex    = nullptr;

	m_VBSize[2]       = 10;
	m_VBSize[1]       = 56;
	m_VBSize[0]       = 256;

	m_pVB             = nullptr;

	m_pTileTexIndx[0] = nullptr;
	m_pTileTexIndx[1] = nullptr;
	m_pIsTileFull     = nullptr;

	// lightmap...
	m_pLightMapVB     = nullptr;
	m_pRefLightMapTex = nullptr;
	m_NumLightMapTex  = 0;
}

//
//	소멸자
//
CN3TerrainPatch::~CN3TerrainPatch()
{
	CN3TerrainPatch::Release();
}

//
//
//
void CN3TerrainPatch::Release()
{
	if (m_pVB != nullptr)
	{
		m_pVB->Release();
		m_pVB = nullptr;
	}

	if (m_pLightMapVB != nullptr)
	{
		m_pLightMapVB->Release();
		m_pLightMapVB = nullptr;
	}

	m_FanInfoList.clear();

	for (int i = 0; i < 2; i++)
	{
		delete[] m_pTileTexIndx[i];
		m_pTileTexIndx[i] = nullptr;
	}
	delete[] m_pIsTileFull;
	m_pIsTileFull = nullptr;

	if (m_pRefLightMapTex)
	{
		delete[] m_pRefLightMapTex;
		m_pRefLightMapTex = nullptr;
	}
}

//
//
//
void CN3TerrainPatch::Init()
{
	Release();

	for (int i = 0; i < 4; i++)
		m_IsBlunt[i] = true;
	m_bIsRender       = FALSE;
	m_iLevel          = 0;
	m_CellSize        = 0;
	m_NumCell         = 0;

	m_NumLightMapTex  = 0;
	m_pLightMapVB     = nullptr;
	m_pRefLightMapTex = nullptr;

	m_ti_LBPoint.x = m_ti_LBPoint.y = -1;
	m_pRefTerrain                   = nullptr;

	m_pVB                           = nullptr;
	m_pRefColorTex                  = nullptr;

	m_pTileTexIndx[0]               = nullptr;
	m_pTileTexIndx[1]               = nullptr;
	m_pIsTileFull                   = nullptr;

	m_FanInfoList.clear();
}

//
//
//
void CN3TerrainPatch::Init(CN3Terrain* pTerrain)
{
	Release();

	for (int i = 0; i < 4; i++)
		m_IsBlunt[i] = true;
	m_bIsRender       = FALSE;
	m_iLevel          = 0;
	m_CellSize        = 0;
	m_NumCell         = 0;

	m_NumLightMapTex  = 0;
	m_pLightMapVB     = nullptr;
	m_pRefLightMapTex = nullptr;

	m_ti_LBPoint.x = m_ti_LBPoint.y = -1;
	m_pRefTerrain                   = pTerrain;

	m_pVB                           = nullptr;
	m_pRefColorTex                  = nullptr;

	for (int i = 0; i < 2; i++)
	{
		m_pTileTexIndx[i] = new uint32_t[PATCH_TILE_SIZE * PATCH_TILE_SIZE];
		memset(m_pTileTexIndx[i], 0xff, sizeof(uint32_t) * PATCH_TILE_SIZE * PATCH_TILE_SIZE);
	}
	m_pIsTileFull = new bool[PATCH_TILE_SIZE * PATCH_TILE_SIZE];

	m_FanInfoList.clear();
}

//
//	Set Level....
//
void CN3TerrainPatch::SetLevel(int level)
{
	if (level == m_iLevel)
		return;

	m_iLevel   = level;

	m_CellSize = 1 << level;
	m_NumCell  = PATCH_TILE_SIZE / m_CellSize;
	m_FanInfoList.clear();

	if (m_pVB)
	{
		m_pVB->Release();
		m_pVB = nullptr;
	}
	if (m_pLightMapVB)
	{
		m_pLightMapVB->Release();
		m_pLightMapVB = nullptr;
	}
	m_NumLightMapTex = 0;
	delete[] m_pRefLightMapTex;
	m_pRefLightMapTex = nullptr;

	if (level == 1)
	{
		CN3Base::s_lpD3DDev->CreateVertexBuffer(m_VBSize[level - 1] * sizeof(__VertexT2), 0, FVF_VNT2, D3DPOOL_MANAGED, &m_pVB, nullptr);
		CN3Base::s_lpD3DDev->CreateVertexBuffer(
			m_VBSize[level - 1] * sizeof(__VertexT1), 0, FVF_VNT1, D3DPOOL_MANAGED, &m_pLightMapVB, nullptr);

		m_NumLightMapTex  = 0;
		m_pRefLightMapTex = new CN3Texture*[PATCH_TILE_SIZE * PATCH_TILE_SIZE];
	}
	else
	{
		CN3Base::s_lpD3DDev->CreateVertexBuffer(m_VBSize[level - 1] * sizeof(__VertexT1), 0, FVF_VNT1, D3DPOOL_MANAGED, &m_pVB, nullptr);
	}
}

//
//
//
void CN3TerrainPatch::Tick()
{
	if (m_ti_LBPoint.x < 0 || m_ti_LBPoint.y < 0 || m_ti_LBPoint.x >= (m_pRefTerrain->m_ti_MapSize - 1)
		|| m_ti_LBPoint.y >= (m_pRefTerrain->m_ti_MapSize - 1))
	{
		m_bIsRender = FALSE;
	}

	if (!m_bIsRender)
		return;

	int HalfCell  = m_CellSize >> 1;

	int VertexIdx = 0, FaceCount = 0, VertexCount = 0;
	int ix = 0, iz = 0;
	int tx = 0, tz = 0;
	int cx = 0, cz = 0; //cell center point...

	FANINFO FaceInfo;

	if (m_iLevel == 1)
	{
		m_FanInfoList.clear();
		m_NumLightMapTex              = 0;

		__VertexT1* pLightMapVertices = nullptr;
		__VertexT2* pVertices         = nullptr;
		if (FAILED(m_pVB->Lock(0, 0, (void**) &pVertices, 0)))
			return;

		int dir1 = 0, dir2 = 0, TileCount = 0;
		float u1[4] {}, u2[4] {}, v1[4] {}, v2[4] {};
		const size_t TileTextureCount = m_pRefTerrain->m_TileTex.size();

		///////////////////////////////////
		//unitUV
		//
		for (ix = 0; ix < PATCH_TILE_SIZE; ix++)
		{
			for (iz = 0; iz < PATCH_TILE_SIZE; iz++)
			{
				tx                           = ix + m_ti_LBPoint.x;
				tz                           = iz + m_ti_LBPoint.y;

				const MAPDATA& MapData       = m_pRefTerrain->GetMapData(tx, tz);

				dir1                         = MapData.Tex1Dir;
				dir2                         = MapData.Tex2Dir;

				m_pTileTexIndx[0][TileCount] = MapData.Tex1Idx;
				m_pTileTexIndx[1][TileCount] = MapData.Tex2Idx;
				m_pIsTileFull[TileCount]     = MapData.bIsTileFull;

				if (m_pTileTexIndx[0][TileCount] >= TileTextureCount
					// 타일이 없는 경우..컬러맵을 찍어야 돼...
					|| !m_pIsTileFull[TileCount])
				{
					u1[0] = u1[1] = UVConvert((float) (tx % UNITUV) / (float) UNITUV);
					u1[2] = u1[3] = UVConvert(u1[0] + (1.0f / (float) UNITUV));

					v1[0] = v1[3] = UVConvert((float) (UNITUV - (tz % UNITUV)) / (float) UNITUV);
					v1[1] = v1[2] = UVConvert(v1[0] - (1.0f / (float) UNITUV));

					//u1[0] =	u1[1] = (float)(tx%UNITUV) / (float)UNITUV;
					//u1[2] = u1[3] = u1[0] + (1.0f/(float)UNITUV);

					//v1[0] = v1[3] = (float)(UNITUV - (tz%UNITUV)) / (float)UNITUV;
					//v1[1] = v1[2] = v1[0] - (1.0f/(float)UNITUV);

					u2[0] = u2[1] = 0.0f;
					u2[2] = u2[3] = 1.0f;

					v2[0] = v2[3] = 0.0f;
					v2[1] = v2[2] = 1.0f;
				}
				else // 타일을 찍는 경우...
				{
					u1[0] = m_pRefTerrain->m_fTileDirU[dir1][2];
					u1[1] = m_pRefTerrain->m_fTileDirU[dir1][0];
					u1[2] = m_pRefTerrain->m_fTileDirU[dir1][1];
					u1[3] = m_pRefTerrain->m_fTileDirU[dir1][3];

					v1[0] = m_pRefTerrain->m_fTileDirV[dir1][2];
					v1[1] = m_pRefTerrain->m_fTileDirV[dir1][0];
					v1[2] = m_pRefTerrain->m_fTileDirV[dir1][1];
					v1[3] = m_pRefTerrain->m_fTileDirV[dir1][3];

					u2[0] = m_pRefTerrain->m_fTileDirU[dir2][2];
					u2[1] = m_pRefTerrain->m_fTileDirU[dir2][0];
					u2[2] = m_pRefTerrain->m_fTileDirU[dir2][1];
					u2[3] = m_pRefTerrain->m_fTileDirU[dir2][3];

					v2[0] = m_pRefTerrain->m_fTileDirV[dir2][2];
					v2[1] = m_pRefTerrain->m_fTileDirV[dir2][0];
					v2[2] = m_pRefTerrain->m_fTileDirV[dir2][1];
					v2[3] = m_pRefTerrain->m_fTileDirV[dir2][3];
				}

				if ((ix + iz) % 2 == 0)
				{
					pVertices[VertexIdx].Set((float) (tx * TILE_SIZE), m_pRefTerrain->GetMapData(tx, tz).fHeight, (float) (tz * TILE_SIZE),
						0.0f, 1.0f, 0.0f, u1[0], v1[0], u2[0], v2[0]);
					pVertices[VertexIdx + 1].Set((float) (tx * TILE_SIZE), m_pRefTerrain->GetMapData(tx, tz + 1).fHeight,
						(float) ((tz + 1) * TILE_SIZE), 0.0f, 1.0f, 0.0f, u1[1], v1[1], u2[1], v2[1]);
					pVertices[VertexIdx + 2].Set((float) ((tx + 1) * TILE_SIZE), m_pRefTerrain->GetMapData(tx + 1, tz + 1).fHeight,
						(float) ((tz + 1) * TILE_SIZE), 0.0f, 1.0f, 0.0f, u1[2], v1[2], u2[2], v2[2]);
					pVertices[VertexIdx + 3].Set((float) ((tx + 1) * TILE_SIZE), m_pRefTerrain->GetMapData(tx + 1, tz).fHeight,
						(float) (tz * TILE_SIZE), 0.0f, 1.0f, 0.0f, u1[3], v1[3], u2[3], v2[3]);
					VertexIdx += 4;
					TileCount++;

					//set lightmap vb...
					if (m_pRefTerrain->GetLightMap(tx, tz))
					{
						m_pRefLightMapTex[m_NumLightMapTex] = m_pRefTerrain->GetLightMap(tx, tz);

						if (pLightMapVertices == nullptr && FAILED(m_pLightMapVB->Lock(0, 0, (void**) &pLightMapVertices, 0)))
						{
							m_pVB->Unlock();
							return;
						}

						int VBIndx = m_NumLightMapTex * 4;
						float suv  = 1.0f / (float) LIGHTMAP_TEX_SIZE;
						float euv  = (float) (LIGHTMAP_TEX_SIZE - 1.0f) / (float) LIGHTMAP_TEX_SIZE;

						pLightMapVertices[VBIndx].Set((float) (tx * TILE_SIZE), m_pRefTerrain->GetMapData(tx, tz).fHeight,
							(float) (tz * TILE_SIZE), 0.0f, 1.0f, 0.0f, suv, euv);
						pLightMapVertices[VBIndx + 1].Set((float) (tx * TILE_SIZE), m_pRefTerrain->GetMapData(tx, tz + 1).fHeight,
							(float) ((tz + 1) * TILE_SIZE), 0.0f, 1.0f, 0.0f, suv, suv);
						pLightMapVertices[VBIndx + 2].Set((float) ((tx + 1) * TILE_SIZE), m_pRefTerrain->GetMapData(tx + 1, tz + 1).fHeight,
							(float) ((tz + 1) * TILE_SIZE), 0.0f, 1.0f, 0.0f, euv, suv);
						pLightMapVertices[VBIndx + 3].Set((float) ((tx + 1) * TILE_SIZE), m_pRefTerrain->GetMapData(tx + 1, tz).fHeight,
							(float) (tz * TILE_SIZE), 0.0f, 1.0f, 0.0f, euv, euv);

						m_NumLightMapTex++;
					}
				}
				else
				{
					pVertices[VertexIdx].Set((float) (tx * TILE_SIZE), m_pRefTerrain->GetMapData(tx, tz + 1).fHeight,
						(float) ((tz + 1) * TILE_SIZE), 0.0f, 1.0f, 0.0f, u1[1], v1[1], u2[1], v2[1]);
					pVertices[VertexIdx + 1].Set((float) ((tx + 1) * TILE_SIZE), m_pRefTerrain->GetMapData(tx + 1, tz + 1).fHeight,
						(float) ((tz + 1) * TILE_SIZE), 0.0f, 1.0f, 0.0f, u1[2], v1[2], u2[2], v2[2]);
					pVertices[VertexIdx + 2].Set((float) ((tx + 1) * TILE_SIZE), m_pRefTerrain->GetMapData(tx + 1, tz).fHeight,
						(float) (tz * TILE_SIZE), 0.0f, 1.0f, 0.0f, u1[3], v1[3], u2[3], v2[3]);
					pVertices[VertexIdx + 3].Set((float) (tx * TILE_SIZE), m_pRefTerrain->GetMapData(tx, tz).fHeight,
						(float) (tz * TILE_SIZE), 0.0f, 1.0f, 0.0f, u1[0], v1[0], u2[0], v2[0]);
					VertexIdx += 4;
					TileCount++;

					//set lightmap vb...
					if (m_pRefTerrain->GetLightMap(tx, tz))
					{
						m_pRefLightMapTex[m_NumLightMapTex] = m_pRefTerrain->GetLightMap(tx, tz);

						if (pLightMapVertices == nullptr && FAILED(m_pLightMapVB->Lock(0, 0, (void**) &pLightMapVertices, 0)))
						{
							m_pVB->Unlock();
							return;
						}

						int VBIndx = m_NumLightMapTex * 4;
						float suv  = 1.0f / (float) LIGHTMAP_TEX_SIZE;
						float euv  = (float) (LIGHTMAP_TEX_SIZE - 1.0f) / (float) LIGHTMAP_TEX_SIZE;

						pLightMapVertices[VBIndx].Set((float) (tx * TILE_SIZE), m_pRefTerrain->GetMapData(tx, tz + 1).fHeight,
							(float) ((tz + 1) * TILE_SIZE), 0.0f, 1.0f, 0.0f, suv, suv);
						pLightMapVertices[VBIndx + 1].Set((float) ((tx + 1) * TILE_SIZE), m_pRefTerrain->GetMapData(tx + 1, tz + 1).fHeight,
							(float) ((tz + 1) * TILE_SIZE), 0.0f, 1.0f, 0.0f, euv, suv);
						pLightMapVertices[VBIndx + 2].Set((float) ((tx + 1) * TILE_SIZE), m_pRefTerrain->GetMapData(tx + 1, tz).fHeight,
							(float) (tz * TILE_SIZE), 0.0f, 1.0f, 0.0f, euv, euv);
						pLightMapVertices[VBIndx + 3].Set((float) (tx * TILE_SIZE), m_pRefTerrain->GetMapData(tx, tz).fHeight,
							(float) (tz * TILE_SIZE), 0.0f, 1.0f, 0.0f, suv, euv);
						m_NumLightMapTex++;
					}
				}
			}
		}

		if (pLightMapVertices)
			m_pLightMapVB->Unlock();

		m_pVB->Unlock();
		return;
	}

	float tu = 0.0f, tv = 0.0f; //center u,v...
	if (m_iLevel == 2)
	{
		m_FanInfoList.clear();

		__VertexT1* pVertices = nullptr;
		if (FAILED(m_pVB->Lock(0, 0, (void**) &pVertices, 0)))
			return;

		float HalfUV = (float) HalfCell / (float) UNITUV;

		for (ix = 0; ix < m_NumCell; ix++)
		{
			for (iz = 0; iz < m_NumCell; iz++)
			{
				VertexCount = VertexIdx;
				FaceCount   = 0;

				cx          = ix * m_CellSize + HalfCell + m_ti_LBPoint.x;
				cz          = iz * m_CellSize + HalfCell + m_ti_LBPoint.y;

				//cell의 가운데 점...
				tx          = cx;
				tz          = cz;

				tu          = (float) (tx % UNITUV) / (float) UNITUV;
				tv          = (float) (UNITUV - (tz % UNITUV)) / (float) UNITUV;

				pVertices[VertexIdx].Set((float) (tx * TILE_SIZE), m_pRefTerrain->GetMapData(tx, tz).fHeight, (float) (tz * TILE_SIZE),
					0.0f, 1.0f, 0.0f, UVConvert(tu), UVConvert(tv));
				//tu, tv);
				VertexIdx++;

				//LB Point..
				tx = cx - HalfCell;
				tz = cz - HalfCell;
				pVertices[VertexIdx].Set((float) (tx * TILE_SIZE), m_pRefTerrain->GetMapData(tx, tz).fHeight, (float) (tz * TILE_SIZE),
					0.0f, 1.0f, 0.0f, UVConvert(tu - HalfUV), UVConvert(tv + HalfUV));
				//tu - HalfUV, tv + HalfUV);
				VertexIdx++;

				// LM Point...
				if ((ix == 0) && (m_IsBlunt[0] == false))
				{
					for (int i = 0; i < 3; i++)
					{
						tz++;
						pVertices[VertexIdx].Set((float) (tx * TILE_SIZE), m_pRefTerrain->GetMapData(tx, tz).fHeight,
							(float) (tz * TILE_SIZE), 0.0f, 1.0f, 0.0f, UVConvert(tu - HalfUV),
							UVConvert((float) (UNITUV - (tz % UNITUV)) / (float) UNITUV));
						//(tu - HalfUV), ((float)(UNITUV - (tz%UNITUV)) / (float)UNITUV));
						VertexIdx++;
						FaceCount++;
					}
				}

				//LT.......
				tz = cz + HalfCell;
				pVertices[VertexIdx].Set((float) (tx * TILE_SIZE), m_pRefTerrain->GetMapData(tx, tz).fHeight, (float) (tz * TILE_SIZE),
					0.0f, 1.0f, 0.0f, UVConvert(tu - HalfUV), UVConvert(tv - HalfUV));
				//(tu - HalfUV), (tv - HalfUV));
				VertexIdx++;
				FaceCount++;

				// MT...
				if (((iz + 1) == m_NumCell) && (m_IsBlunt[1] == false))
				{
					for (int i = 0; i < 3; i++)
					{
						tx++;
						pVertices[VertexIdx].Set((float) (tx * TILE_SIZE), m_pRefTerrain->GetMapData(tx, tz).fHeight,
							(float) (tz * TILE_SIZE), 0.0f, 1.0f, 0.0f, UVConvert((float) (tx % UNITUV) / (float) UNITUV),
							UVConvert(tv - HalfUV));
						//((float)(tx%UNITUV) / (float)UNITUV), (tv - HalfUV));
						VertexIdx++;
						FaceCount++;
					}
				}

				//RT.......
				tx = cx + HalfCell;
				pVertices[VertexIdx].Set((float) (tx * TILE_SIZE), m_pRefTerrain->GetMapData(tx, tz).fHeight, (float) (tz * TILE_SIZE),
					0.0f, 1.0f, 0.0f, UVConvert(tu + HalfUV), UVConvert(tv - HalfUV));
				//(tu + HalfUV), (tv - HalfUV));
				VertexIdx++;
				FaceCount++;

				// RM...
				if (((ix + 1) == m_NumCell) && (m_IsBlunt[2] == false))
				{
					for (int i = 0; i < 3; i++)
					{
						//tx = cx + HalfCell;
						tz--;
						pVertices[VertexIdx].Set((float) (tx * TILE_SIZE), m_pRefTerrain->GetMapData(tx, tz).fHeight,
							(float) (tz * TILE_SIZE), 0.0f, 1.0f, 0.0f, UVConvert(tu + HalfUV),
							UVConvert((float) (UNITUV - (tz % UNITUV)) / (float) UNITUV));
						//(tu + HalfUV), ((float)(UNITUV - (tz%UNITUV)) / (float)UNITUV));
						VertexIdx++;
						FaceCount++;
					}
				}

				//RB......
				//tx = cx + HalfCell;
				tz = cz - HalfCell;
				pVertices[VertexIdx].Set((float) (tx * TILE_SIZE), m_pRefTerrain->GetMapData(tx, tz).fHeight, (float) (tz * TILE_SIZE),
					0.0f, 1.0f, 0.0f, UVConvert(tu + HalfUV), UVConvert(tv + HalfUV));
				//(tu + HalfUV), (tv + HalfUV));
				VertexIdx++;
				FaceCount++;

				// MB...
				if ((iz == 0) && (m_IsBlunt[3] == false))
				{
					for (int i = 0; i < 3; i++)
					{
						tx--;
						pVertices[VertexIdx].Set((float) (tx * TILE_SIZE), m_pRefTerrain->GetMapData(tx, tz).fHeight,
							(float) (tz * TILE_SIZE), 0.0f, 1.0f, 0.0f, UVConvert((float) (tx % UNITUV) / (float) UNITUV),
							UVConvert(tv + HalfUV));
						//((float)(tx%UNITUV) / (float)UNITUV), (tv + HalfUV));
						VertexIdx++;
						FaceCount++;
					}
				}

				//LB Point..
				tx = cx - HalfCell;
				pVertices[VertexIdx].Set((float) (tx * TILE_SIZE), m_pRefTerrain->GetMapData(tx, tz).fHeight, (float) (tz * TILE_SIZE),
					0.0f, 1.0f, 0.0f, UVConvert(tu - HalfUV), UVConvert(tv + HalfUV));
				//(tu - HalfUV), (tv + HalfUV));
				VertexIdx++;
				FaceCount++;
				FaceInfo.NumFace   = FaceCount;
				FaceInfo.NumVertex = VertexIdx - VertexCount;
				m_FanInfoList.push_back(FaceInfo);
			}
		}

		m_pVB->Unlock();
		return;
	}

	if (m_iLevel > 2) // level3 이상..
	{
		m_FanInfoList.clear();

		__VertexT1* pVertices = nullptr;
		HRESULT hr            = m_pVB->Lock(0, 0, (void**) &pVertices, 0);
		if (FAILED(hr))
			return;

		float HalfUV = (float) HalfCell / (float) UNITUV;

		for (ix = 0; ix < m_NumCell; ix++)
		{
			for (iz = 0; iz < m_NumCell; iz++)
			{
				VertexCount = VertexIdx;
				FaceCount   = 0;

				cx          = ix * m_CellSize + HalfCell + m_ti_LBPoint.x;
				cz          = iz * m_CellSize + HalfCell + m_ti_LBPoint.y;

				//cell의 가운데 점...
				tx          = cx;
				tz          = cz;

				tu          = (float) (tx % UNITUV) / (float) UNITUV;
				tv          = (float) (UNITUV - (tz % UNITUV)) / (float) UNITUV;

				pVertices[VertexIdx].Set((float) (tx * TILE_SIZE), m_pRefTerrain->GetMapData(tx, tz).fHeight, (float) (tz * TILE_SIZE),
					0.0f, 1.0f, 0.0f, UVConvert(tu), UVConvert(tv));
				//(tu), (tv));
				VertexIdx++;

				//LB Point..
				tx = cx - HalfCell;
				tz = cz - HalfCell;

				pVertices[VertexIdx].Set((float) (tx * TILE_SIZE), m_pRefTerrain->GetMapData(tx, tz).fHeight, (float) (tz * TILE_SIZE),
					0.0f, 1.0f, 0.0f, UVConvert(tu - HalfUV), UVConvert(tv + HalfUV));
				//(tu - HalfUV),
				//(tv + HalfUV));
				VertexIdx++;

				// LM Point...
				if ((ix == 0) && (m_IsBlunt[0] == false))
				{
					tz += HalfCell;
					pVertices[VertexIdx].Set((float) (tx * TILE_SIZE), m_pRefTerrain->GetMapData(tx, tz).fHeight, (float) (tz * TILE_SIZE),
						0.0f, 1.0f, 0.0f, UVConvert(tu - HalfUV), UVConvert(tv));
					//(tu - HalfUV),
					//(tv));
					VertexIdx++;
					FaceCount++;
				}

				//LT.......
				//tx = cx - HalfCell;
				tz = cz + HalfCell;
				pVertices[VertexIdx].Set((float) (tx * TILE_SIZE), m_pRefTerrain->GetMapData(tx, tz).fHeight, (float) (tz * TILE_SIZE),
					0.0f, 1.0f, 0.0f, UVConvert(tu - HalfUV), UVConvert(tv - HalfUV));
				//(tu - HalfUV), (tv - HalfUV));
				VertexIdx++;
				FaceCount++;

				// MT...
				if (((iz + 1) == m_NumCell) && (m_IsBlunt[1] == false))
				{
					tx += HalfCell;
					pVertices[VertexIdx].Set((float) (tx * TILE_SIZE), m_pRefTerrain->GetMapData(tx, tz).fHeight, (float) (tz * TILE_SIZE),
						0.0f, 1.0f, 0.0f, UVConvert(tu), UVConvert(tv - HalfUV));
					//(tu), (tv - HalfUV));
					VertexIdx++;
					FaceCount++;
				}

				//RT.......
				tx = cx + HalfCell;
				pVertices[VertexIdx].Set((float) (tx * TILE_SIZE), m_pRefTerrain->GetMapData(tx, tz).fHeight, (float) (tz * TILE_SIZE),
					0.0f, 1.0f, 0.0f, UVConvert(tu + HalfUV), UVConvert(tv - HalfUV));
				//(tu + HalfUV), (tv - HalfUV));
				VertexIdx++;
				FaceCount++;

				// RM...
				if (((ix + 1) == m_NumCell) && (m_IsBlunt[2] == false))
				{
					tz -= HalfCell;
					pVertices[VertexIdx].Set((float) (tx * TILE_SIZE), m_pRefTerrain->GetMapData(tx, tz).fHeight, (float) (tz * TILE_SIZE),
						0.0f, 1.0f, 0.0f, UVConvert(tu + HalfUV), UVConvert(tv));
					//(tu + HalfUV), (tv));
					VertexIdx++;
					FaceCount++;
				}

				//RB......
				//tx = cx + HalfCell;
				tz = cz - HalfCell;
				pVertices[VertexIdx].Set((float) (tx * TILE_SIZE), m_pRefTerrain->GetMapData(tx, tz).fHeight, (float) (tz * TILE_SIZE),
					0.0f, 1.0f, 0.0f, UVConvert(tu + HalfUV), UVConvert(tv + HalfUV));
				//(tu + HalfUV), (tv + HalfUV));
				VertexIdx++;
				FaceCount++;

				// MB...
				if ((iz == 0) && (m_IsBlunt[3] == false))
				{
					tx -= HalfCell;
					pVertices[VertexIdx].Set((float) (tx * TILE_SIZE), m_pRefTerrain->GetMapData(tx, tz).fHeight, (float) (tz * TILE_SIZE),
						0.0f, 1.0f, 0.0f, UVConvert(tu), UVConvert(tv + HalfUV));
					//(tu), (tv + HalfUV));
					VertexIdx++;
					FaceCount++;
				}

				//LB Point..
				tx = cx - HalfCell;
				pVertices[VertexIdx].Set((float) (tx * TILE_SIZE), m_pRefTerrain->GetMapData(tx, tz).fHeight, (float) (tz * TILE_SIZE),
					0.0f, 1.0f, 0.0f, UVConvert(tu - HalfUV), UVConvert(tv + HalfUV));
				//(tu - HalfUV), (tv + HalfUV));
				VertexIdx++;
				FaceCount++;
				FaceInfo.NumFace   = FaceCount;
				FaceInfo.NumVertex = VertexIdx - VertexCount;
				m_FanInfoList.push_back(FaceInfo);
			}
		}

		m_pVB->Unlock();
	}
}

//
//
//
void CN3TerrainPatch::Render()
{
	if (!m_bIsRender)
		return;

	if (m_iLevel > 1)
	{
		CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
		CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);

		CN3Base::s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		CN3Base::s_lpD3DDev->SetTexture(1, nullptr);

		CN3Base::s_lpD3DDev->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
		CN3Base::s_lpD3DDev->SetTexture(2, nullptr);

		CN3Base::s_lpD3DDev->SetStreamSource(0, m_pVB, 0, sizeof(__VertexT1));
		CN3Base::s_lpD3DDev->SetFVF(FVF_VNT1);

		if (m_pRefColorTex->Get() != nullptr)
			CN3Base::s_lpD3DDev->SetTexture(0, m_pRefColorTex->Get());
		else
			CN3Base::s_lpD3DDev->SetTexture(0, nullptr);

		int vc = 0, fc = 0;
		for (const FANINFO& fi : m_FanInfoList)
		{
			fc = fi.NumFace;
			CN3Base::s_lpD3DDev->DrawPrimitive(D3DPT_TRIANGLEFAN, vc, fc);
			vc += fi.NumVertex;
#if _DEBUG
			CN3Base::s_RenderInfo.nTerrain_Polygon += fi.NumVertex; // Rendering Information 갱신..
#endif
		}
		return;
	}

	if (m_iLevel == 1)
	{
		CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

		CN3Base::s_lpD3DDev->SetStreamSource(0, m_pVB, 0, sizeof(__VertexT2));
		CN3Base::s_lpD3DDev->SetFVF(FVF_VNT2);

		const size_t TileTextureCount = m_pRefTerrain->m_TileTex.size();
		int TotalTile                 = PATCH_TILE_SIZE * PATCH_TILE_SIZE;
		for (int i = 0; i < TotalTile; i++)
		{
			CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

			CN3Base::s_lpD3DDev->SetTexture(2, nullptr);

			if (m_pTileTexIndx[0][i] >= TileTextureCount || (m_pIsTileFull[i] == false)) // 0: 컬러맵, 1:무늬 or 0:부분타일 1:NONE...
			{
				CN3Base::s_lpD3DDev->SetTexture(0, m_pRefColorTex->Get());
				CN3Base::s_lpD3DDev->SetTexture(1, m_pRefTerrain->m_pBaseTex.Get());

				CN3Base::s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
				CN3Base::s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				CN3Base::s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);

				CN3Base::s_lpD3DDev->SetTexture(2, nullptr);
				CN3Base::s_lpD3DDev->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_MODULATE);
				CN3Base::s_lpD3DDev->SetTextureStageState(2, D3DTSS_COLORARG1, D3DTA_CURRENT);
				CN3Base::s_lpD3DDev->SetTextureStageState(2, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			}
			else if (m_pTileTexIndx[0][i] < TileTextureCount)
			{
				if (m_pTileTexIndx[1][i] < TileTextureCount) //0: tile, 1: tile..
				{
					CN3Base::s_lpD3DDev->SetTexture(0, m_pRefTerrain->m_TileTex[m_pTileTexIndx[0][i]].Get());
					CN3Base::s_lpD3DDev->SetTexture(1, m_pRefTerrain->m_TileTex[m_pTileTexIndx[1][i]].Get());
					CN3Base::s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_ADD);
					CN3Base::s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
					CN3Base::s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);

					if (m_pRefTerrain->m_bAvailableTile)
					{
						CN3Base::s_lpD3DDev->SetTexture(2, nullptr);
						CN3Base::s_lpD3DDev->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_MODULATE);
						CN3Base::s_lpD3DDev->SetTextureStageState(2, D3DTSS_COLORARG1, D3DTA_CURRENT);
						CN3Base::s_lpD3DDev->SetTextureStageState(2, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
					}
				}
				else //0: tile, 1:NONE...
				{
					CN3Base::s_lpD3DDev->SetTexture(0, m_pRefTerrain->m_TileTex[m_pTileTexIndx[0][i]].Get());
					CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
					CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
					CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

					CN3Base::s_lpD3DDev->SetTexture(1, nullptr);
					CN3Base::s_lpD3DDev->SetTexture(2, nullptr);
					CN3Base::s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
					CN3Base::s_lpD3DDev->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
				}
			}

			CN3Base::s_lpD3DDev->DrawPrimitive(D3DPT_TRIANGLEFAN, (i << 2), 2);

			if ((!m_pRefTerrain->m_bAvailableTile) && m_pTileTexIndx[0][i] < TileTextureCount && m_pTileTexIndx[1][i] < TileTextureCount)
			{
				DWORD dwAlphaEnable = 0, dwSrcBlend = 0, dwDestBlend = 0;
				CN3Base::s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlphaEnable);
				CN3Base::s_lpD3DDev->GetRenderState(D3DRS_SRCBLEND, &dwSrcBlend);
				CN3Base::s_lpD3DDev->GetRenderState(D3DRS_DESTBLEND, &dwDestBlend);

				CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				CN3Base::s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
				CN3Base::s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);

				CN3Base::s_lpD3DDev->SetTexture(0, nullptr);
				CN3Base::s_lpD3DDev->SetTexture(1, nullptr);
				CN3Base::s_lpD3DDev->SetTexture(2, nullptr);
				CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
				CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
				CN3Base::s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
				CN3Base::s_lpD3DDev->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);

				CN3Base::s_lpD3DDev->DrawPrimitive(D3DPT_TRIANGLEFAN, (i << 2), 2);

				CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlphaEnable);
				CN3Base::s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, dwSrcBlend);
				CN3Base::s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, dwDestBlend);
			}
		}

#if _DEBUG
		CN3Base::s_RenderInfo.nTerrain_Tile_Polygon += TotalTile * 2; // Rendering Information 갱신..
#endif

		// Render Light Map...
		CN3Base::s_lpD3DDev->SetStreamSource(0, m_pLightMapVB, 0, sizeof(__VertexT1));
		CN3Base::s_lpD3DDev->SetFVF(FVF_VNT1);

		DWORD dwAlphaEnable = 0, dwSrcBlend = 0, dwDestBlend = 0;

		CN3Base::s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		CN3Base::s_lpD3DDev->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
		CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

		CN3Base::s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlphaEnable);
		CN3Base::s_lpD3DDev->GetRenderState(D3DRS_SRCBLEND, &dwSrcBlend);
		CN3Base::s_lpD3DDev->GetRenderState(D3DRS_DESTBLEND, &dwDestBlend);

		CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		CN3Base::s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		CN3Base::s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		for (int i = 0; i < m_NumLightMapTex; i++)
		{
			CN3Base::s_lpD3DDev->SetTexture(0, m_pRefLightMapTex[i]->Get());
			CN3Base::s_lpD3DDev->DrawPrimitive(D3DPT_TRIANGLEFAN, (i << 2), 2);
		}
		CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlphaEnable);
		CN3Base::s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, dwSrcBlend);
		CN3Base::s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, dwDestBlend);
	}
}

//
//	한픽셀씩 축소시킨 커러맵 쓸때 uv바꿔주는 함수..
//	필요없게 됐다..-.- 걍 텍스쳐 스테이지 스테이트에서 mirror쓰면 된다..ㅡ.ㅡ
//
inline float CN3TerrainPatch::UVConvert(float uv)
{
	//return ( (uv*((float)COLORMAPTEX_SIZE - 2.0f) + 1.0f) / (float)COLORMAPTEX_SIZE);
	return uv;
}

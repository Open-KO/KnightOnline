// QTNode.cpp: implementation of the CQTNode class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
//#define new DEBUG_NEW
#endif

#include "QTNode.h"
#include "LyTerrain.h"
#include <N3Base/N3Texture.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CQTNode::CQTNode()
{
	m_CenterX = m_CenterZ = 0;
	m_MinY                = FLT_MAX;
	m_MaxY                = -FLT_MAX;
	m_Level               = 0;
	m_State               = QTNODESTATE_NOTRENDER;

	int i;
	for (i = 0; i < DIR_NUM; i++)
	{
		m_pFriend[i] = nullptr;
		m_pChild[i]  = nullptr;
	}
}

//
//
//
CQTNode::~CQTNode()
{
	Release();
}

//
//	Initialize..
//
void CQTNode::Init(int level, CLyTerrain* pTerrain)
{
	if (!pTerrain)
		return;

	Release();
	m_CenterX = m_CenterZ = 0;
	m_MinY                = FLT_MAX;
	m_MaxY                = -FLT_MAX;

	m_pRefTerrain         = pTerrain;
	m_Level               = level;
	m_State               = QTNODESTATE_NOTRENDER;

	int i;

	// 자식노드만들기..
	if (level != m_pRefTerrain->m_iMaxLevel)
	{
		for (i = 0; i < DIR_NUM; i++)
		{
			m_pChild[i] = new CQTNode;
			m_pChild[i]->Init(level + 1, pTerrain);
		}
	}
	else if (level == m_pRefTerrain->m_iMaxLevel)
	{
		for (i = 0; i < DIR_NUM; i++)
		{
			m_pChild[i] = nullptr;
		}
	}
}

//
//	LinkFriend..
//	친구들 연결하기..
//	- 원래는 Init함수에 넣을려고 했었는데,
//	 친구들이 미처 만들어지기 전에 연결을
//	 시도하는 경우가 있기땜에 모든 노드들을
//	 다 생성한 다음에 친구연결을 시도해야한다.
//
void CQTNode::LinkFriend()
{
	if (m_Level == m_pRefTerrain->m_iMaxLevel)
		return;

	if (m_pChild[DIR_LEFT])
	{
		if (this->m_pFriend[DIR_LEFT])
			m_pChild[DIR_LEFT]->m_pFriend[DIR_LEFT] = this->m_pFriend[DIR_LEFT]->m_pChild[DIR_TOP];
		else
			m_pChild[DIR_LEFT]->m_pFriend[DIR_LEFT] = nullptr;

		m_pChild[DIR_LEFT]->m_pFriend[DIR_RIGHT] = this->m_pChild[DIR_TOP];

		if (this->m_pFriend[DIR_TOP])
			m_pChild[DIR_LEFT]->m_pFriend[DIR_TOP] = this->m_pFriend[DIR_TOP]->m_pChild[DIR_BOTTOM];
		else
			m_pChild[DIR_LEFT]->m_pFriend[DIR_TOP] = nullptr;

		m_pChild[DIR_LEFT]->m_pFriend[DIR_BOTTOM] = this->m_pChild[DIR_BOTTOM];

		m_pChild[DIR_LEFT]->LinkFriend();
	}
	if (m_pChild[DIR_TOP])
	{
		m_pChild[DIR_TOP]->m_pFriend[DIR_LEFT] = this->m_pChild[DIR_LEFT];

		if (this->m_pFriend[DIR_RIGHT])
			m_pChild[DIR_TOP]->m_pFriend[DIR_RIGHT] = this->m_pFriend[DIR_RIGHT]
														  ->m_pChild[DIR_LEFT];
		else
			m_pChild[DIR_TOP]->m_pFriend[DIR_RIGHT] = nullptr;

		if (this->m_pFriend[DIR_TOP])
			m_pChild[DIR_TOP]->m_pFriend[DIR_TOP] = this->m_pFriend[DIR_TOP]->m_pChild[DIR_RIGHT];
		else
			m_pChild[DIR_TOP]->m_pFriend[DIR_TOP] = nullptr;

		m_pChild[DIR_TOP]->m_pFriend[DIR_BOTTOM] = this->m_pChild[DIR_RIGHT];

		m_pChild[DIR_TOP]->LinkFriend();
	}
	if (m_pChild[DIR_RIGHT])
	{
		m_pChild[DIR_RIGHT]->m_pFriend[DIR_LEFT] = this->m_pChild[DIR_BOTTOM];

		if (this->m_pFriend[DIR_RIGHT])
			m_pChild[DIR_RIGHT]->m_pFriend[DIR_RIGHT] = this->m_pFriend[DIR_RIGHT]
															->m_pChild[DIR_BOTTOM];
		else
			m_pChild[DIR_RIGHT]->m_pFriend[DIR_RIGHT] = nullptr;

		m_pChild[DIR_RIGHT]->m_pFriend[DIR_TOP] = this->m_pChild[DIR_TOP];

		if (this->m_pFriend[DIR_BOTTOM])
			m_pChild[DIR_RIGHT]->m_pFriend[DIR_BOTTOM] = this->m_pFriend[DIR_BOTTOM]
															 ->m_pChild[DIR_TOP];
		else
			m_pChild[DIR_RIGHT]->m_pFriend[DIR_BOTTOM] = nullptr;

		m_pChild[DIR_RIGHT]->LinkFriend();
	}
	if (m_pChild[DIR_BOTTOM])
	{
		if (this->m_pFriend[DIR_LEFT])
			m_pChild[DIR_BOTTOM]->m_pFriend[DIR_LEFT] = this->m_pFriend[DIR_LEFT]
															->m_pChild[DIR_RIGHT];
		else
			m_pChild[DIR_BOTTOM]->m_pFriend[DIR_LEFT] = nullptr;

		m_pChild[DIR_BOTTOM]->m_pFriend[DIR_RIGHT] = this->m_pChild[DIR_RIGHT];

		m_pChild[DIR_BOTTOM]->m_pFriend[DIR_TOP]   = this->m_pChild[DIR_LEFT];

		if (this->m_pFriend[DIR_BOTTOM])
			m_pChild[DIR_BOTTOM]->m_pFriend[DIR_BOTTOM] = this->m_pFriend[DIR_BOTTOM]
															  ->m_pChild[DIR_LEFT];
		else
			m_pChild[DIR_BOTTOM]->m_pFriend[DIR_BOTTOM] = nullptr;

		m_pChild[DIR_BOTTOM]->LinkFriend();
	}
}

//
//	Release..
//
void CQTNode::Release()
{
	for (int i = 0; i < DIR_NUM; i++)
	{
		if (m_pChild[i])
		{
			m_pChild[i]->Release();
			delete m_pChild[i];
			m_pChild[i] = nullptr;
		}
	}
}

//
//	Center point를 설정하기..
//
void CQTNode::SetCenterPoint(int x, int z)
{
	m_CenterX = x;
	m_CenterZ = z;

	int delta = 1 << (m_pRefTerrain->m_iMaxLevel - m_Level - 1);
	int ChildX, ChildZ;

	if (m_Level != m_pRefTerrain->m_iMaxLevel)
	{
		for (int i = 0; i < DIR_NUM; i++)
		{
			if (!m_pChild[i])
				continue;

			if (i == DIR_LEFT || i == DIR_BOTTOM)
				ChildX = x - delta;
			else
				ChildX = x + delta;

			if (i == DIR_LEFT || i == DIR_TOP)
				ChildZ = z + delta;
			else
				ChildZ = z - delta;

			m_pChild[i]->SetCenterPoint(ChildX, ChildZ);
		}
	}
}

//
//	SetMinMaxY..
//
void CQTNode::SetMinMaxY()
{
	if (m_Level == m_pRefTerrain->m_iMaxLevel) //젤 마지막 노드일때..
	{
		for (int x = -1; x < 2; x++)
		{
			for (int z = -1; z < 2; z++)
			{
				if (m_MinY > m_pRefTerrain->m_ppMapData[m_CenterX + x][m_CenterZ + z].fHeight)
					m_MinY = m_pRefTerrain->m_ppMapData[m_CenterX + x][m_CenterZ + z].fHeight;
				if (m_MaxY < m_pRefTerrain->m_ppMapData[m_CenterX + x][m_CenterZ + z].fHeight)
					m_MaxY = m_pRefTerrain->m_ppMapData[m_CenterX + x][m_CenterZ + z].fHeight;
			}
		}
		return;
	}

	//젤 마지막 노드가 아닐때...
	int i;
	for (i = 0; i < DIR_NUM; i++)
	{
		if (!m_pChild[i])
			continue;
		m_pChild[i]->SetMinMaxY();

		if (m_MinY > m_pChild[i]->m_MinY)
			m_MinY = m_pChild[i]->m_MinY;
		if (m_MaxY < m_pChild[i]->m_MaxY)
			m_MaxY = m_pChild[i]->m_MaxY;
	}
}

//
//
//
void CQTNode::ClearMinMaxY()
{
	int i;
	for (i = 0; i < DIR_NUM; i++)
	{
		if (!m_pChild[i])
			continue;
		m_pChild[i]->ClearMinMaxY();
	}
	m_MinY = FLT_MAX;
	m_MaxY = -FLT_MAX;
}

//
//
//
void CQTNode::SetWholeClipped()
{
	int i;
	for (i = 0; i < DIR_NUM; i++)
	{
		if (!m_pChild[i])
			continue;
		m_pChild[i]->SetWholeClipped();
	}
	m_State = QTNODESTATE_CLIPPED;
}

//
//	tick...
//	쿼드트리의 실체...^^
//	일단 가시영역안에 드는지 검사(IntersectRect()..)를 하고..
//	가시영역안에 들면 그릴것인지 더 내려갈 것인지 검사..
//
void CQTNode::Tick()
{
	m_State = QTNODESTATE_CLIPPED; //상태 초기화..

	//가시영역안에드는지검사.
	RECT rt;
	int Range = 1 << (m_pRefTerrain->m_iMaxLevel - m_Level); // 중심점부터 가장자리까지 거리.
	SetRect(&rt, m_CenterX - Range, m_CenterZ - Range, m_CenterX + Range, m_CenterZ + Range);

	RECT rt1;
	if (IntersectRect(&rt1, &rt, &(m_pRefTerrain->m_VisibleRect)) == 0)
		return; //가시영역안에 들지 않으면 나가..

	//그릴것인지 더 내려 갈 것인지 검사..
	if (IsRender(&(m_pRefTerrain->m_EyePos)))
	{
		// 그만 내려가는 경우..
		m_pRefTerrain->m_RenderNodes.push_back(this);
		return;
	}

	//내려가는 경우..
	int i;
	for (i = 0; i < DIR_NUM; i++)
	{
		if (!m_pChild[i])
			continue;
		m_pChild[i]->Tick();
	}
}

//
//	IsRender..
//	내려가는 걸 그만두고 그릴것인지, 아니면 더 내려갈것인지 검사...
//	return true -> 그만 내려가고 Render;
//	return false -> 더 내려감...
//	여기에 들어가는게 지형 디테일 방법의 핵심...^^
//
bool CQTNode::IsRender(POINT* pEye)
{
	if (m_pRefTerrain->m_iMaxLevel == m_Level)
	{
		m_State                                                     = QTNODESTATE_RENDER;
		m_pRefTerrain->m_ppRenderInfo[m_CenterX - 1][m_CenterZ - 1] = TRUE;
		m_pRefTerrain->m_ppRenderInfo[m_CenterX - 1][m_CenterZ]     = TRUE;
		m_pRefTerrain->m_ppRenderInfo[m_CenterX - 1][m_CenterZ + 1] = TRUE;
		m_pRefTerrain->m_ppRenderInfo[m_CenterX][m_CenterZ - 1]     = TRUE;
		m_pRefTerrain->m_ppRenderInfo[m_CenterX][m_CenterZ + 1]     = TRUE;
		m_pRefTerrain->m_ppRenderInfo[m_CenterX + 1][m_CenterZ - 1] = TRUE;
		m_pRefTerrain->m_ppRenderInfo[m_CenterX + 1][m_CenterZ]     = TRUE;
		m_pRefTerrain->m_ppRenderInfo[m_CenterX + 1][m_CenterZ + 1] = TRUE;
		return true;
	}

	int DiffLevel = m_pRefTerrain->m_iMaxLevel - m_Level;

	if (DiffLevel > 4)
	{
		m_State = QTNODESTATE_NOTRENDER;
		return false;
	}

	// 높이가 어느수준이상이면 나눈다.
	float height = m_MaxY - m_MinY;
	//if( height > (1<<(DiffLevel+1)) )
	if (height > m_pRefTerrain->m_iHeightLimit)
	{
		m_State = QTNODESTATE_NOTRENDER;
		return false;
	}

	//거리에 따라..
	int Dist = Distance(pEye->x, pEye->y, m_CenterX, m_CenterZ);
	if (Dist < (m_pRefTerrain->m_iDistLimit + (1 << (DiffLevel + 1))))
	{
		m_State = QTNODESTATE_NOTRENDER;
		return false;
	}

	m_State                                                           = QTNODESTATE_RENDER;
	int half                                                          = 1 << DiffLevel;
	m_pRefTerrain->m_ppRenderInfo[m_CenterX - half][m_CenterZ - half] = TRUE;
	m_pRefTerrain->m_ppRenderInfo[m_CenterX - half][m_CenterZ + half] = TRUE;
	m_pRefTerrain->m_ppRenderInfo[m_CenterX + half][m_CenterZ + half] = TRUE;
	m_pRefTerrain->m_ppRenderInfo[m_CenterX + half][m_CenterZ - half] = TRUE;
	return true;
}

//
//	간단한 거리계산 방법..
//
inline int CQTNode::Distance(int sx, int sz, int tx, int tz)
{
	int iX   = tx - sx;
	int iZ   = tz - sz;

	iX       = std::abs(iX);
	iZ       = std::abs(iZ);

	int iMin = std::min(iX, iZ);

	return (iX + iZ - (iMin >> 1) - (iMin >> 2) + (iMin >> 4));
}

//
//
//
void CQTNode::Render()
{
	if (m_Level == m_pRefTerrain->m_iMaxLevel)
	{
		RenderMaxLevel();
		return;
	}

	RenderNormalLevel();
}

//
//	가장 디테일한 타일을 그리는 함수..
//
void CQTNode::RenderMaxLevel()
{
	HRESULT hr;
	DWORD ColorArg11, ColorArg12, ColorArg21, ColorArg22;
	DWORD ColorOP1, ColorOP2;

	hr = m_pRefTerrain->s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLOROP, &ColorOP1);
	hr = m_pRefTerrain->s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLORARG1, &ColorArg11);
	hr = m_pRefTerrain->s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLORARG2, &ColorArg12);

	hr = m_pRefTerrain->s_lpD3DDev->GetTextureStageState(1, D3DTSS_COLOROP, &ColorOP2);
	hr = m_pRefTerrain->s_lpD3DDev->GetTextureStageState(1, D3DTSS_COLORARG1, &ColorArg21);
	hr = m_pRefTerrain->s_lpD3DDev->GetTextureStageState(1, D3DTSS_COLORARG2, &ColorArg22);

	int cx, cz;
	int i;

	int dir1, dir2;
	float u1, u2, v1, v2;
	int tmpTIdx;

	__VertexT2* pVertices;
	m_pRefTerrain->m_TileVB->Lock(0, 0, (void**) &pVertices, 0);

	cx = m_CenterX - 1;
	cz = m_CenterZ - 1;

	//lightmap을 그리기 위한 준비..
	//
	CN3Texture* pRefLightMapTex[4];
	int NumLightMapUse            = 0;
	__VertexT1* pLightMapVertices = nullptr;
	//

	for (i = 0; i < 2; i++)
	{
		int idx = i << 2;
		if (i == 1)
		{
			cx++;
			cz++;
		}

		//set lightmap VB..
		if (m_pRefTerrain->m_ppIsLightMap[cx][cz])
		{
			pRefLightMapTex[NumLightMapUse] = m_pRefTerrain->m_ppLightMapTexture[cx][cz];

			float lsuv                      = 1.0f / (float) LIGHTMAP_TEX_SIZE;
			float leuv = ((float) LIGHTMAP_TEX_SIZE - 1.0f) / (float) LIGHTMAP_TEX_SIZE;

			if (!pLightMapVertices)
				m_pRefTerrain->m_LightMapVB->Lock(0, 0, (void**) &pLightMapVertices, 0);
			int VBIndx = NumLightMapUse * 4;
			pLightMapVertices[VBIndx].Set((float) (cx * TERRAIN_CELL_SIZE),
				m_pRefTerrain->m_ppMapData[cx][cz].fHeight, (float) (cz * TERRAIN_CELL_SIZE), 0.0f,
				1.0f, 0.0f, lsuv, leuv);
			pLightMapVertices[VBIndx + 1].Set((float) (cx * TERRAIN_CELL_SIZE),
				m_pRefTerrain->m_ppMapData[cx][cz + 1].fHeight,
				(float) ((cz + 1) * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f, lsuv, lsuv);
			pLightMapVertices[VBIndx + 2].Set((float) ((cx + 1) * TERRAIN_CELL_SIZE),
				m_pRefTerrain->m_ppMapData[cx + 1][cz + 1].fHeight,
				(float) ((cz + 1) * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f, leuv, lsuv);
			pLightMapVertices[VBIndx + 3].Set((float) ((cx + 1) * TERRAIN_CELL_SIZE),
				m_pRefTerrain->m_ppMapData[cx + 1][cz].fHeight, (float) (cz * TERRAIN_CELL_SIZE),
				0.0f, 1.0f, 0.0f, leuv, leuv);
			NumLightMapUse++;
		}

		tmpTIdx = m_pRefTerrain->m_ppMapData[cx][cz].DTexInfo1.TexIdx.TexID;
		if ((m_pRefTerrain->m_ppMapData[cx][cz].DTexInfo1.Attr.Group <= 0)
			|| m_pRefTerrain->GetTileTex(tmpTIdx) == nullptr) //타일이 없을때...
		{
			int NumTileInColorTex = m_pRefTerrain->m_iColorMapTexSize
									/ m_pRefTerrain->m_iColorMapPixelPerUnitDistance;
			u1 = (float) (cx % NumTileInColorTex);
			v1 = (float) (NumTileInColorTex - (cz % NumTileInColorTex));

			pVertices[idx].Set((float) (cx * TERRAIN_CELL_SIZE),
				m_pRefTerrain->m_ppMapData[cx][cz].fHeight, (float) (cz * TERRAIN_CELL_SIZE), 0.0f,
				1.0f, 0.0f, u1 / (float) NumTileInColorTex, v1 / (float) NumTileInColorTex, 0, 0);
			pVertices[idx + 1].Set((float) (cx * TERRAIN_CELL_SIZE),
				m_pRefTerrain->m_ppMapData[cx][cz + 1].fHeight,
				(float) ((cz + 1) * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
				u1 / (float) NumTileInColorTex, (v1 - 1.0f) / (float) NumTileInColorTex, 0, 0);
			pVertices[idx + 2].Set((float) ((cx + 1) * TERRAIN_CELL_SIZE),
				m_pRefTerrain->m_ppMapData[cx + 1][cz + 1].fHeight,
				(float) ((cz + 1) * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
				(u1 + 1.0f) / (float) NumTileInColorTex, (v1 - 1.0f) / (float) NumTileInColorTex, 0,
				0);
			pVertices[idx + 3].Set((float) ((cx + 1) * TERRAIN_CELL_SIZE),
				m_pRefTerrain->m_ppMapData[cx + 1][cz].fHeight, (float) (cz * TERRAIN_CELL_SIZE),
				0.0f, 1.0f, 0.0f, (u1 + 1.0f) / (float) NumTileInColorTex,
				v1 / (float) NumTileInColorTex, 0, 0);
		}

		else
		{
			tmpTIdx = m_pRefTerrain->m_ppMapData[cx][cz].DTexInfo2.TexIdx.TexID;
			if ((m_pRefTerrain->m_ppMapData[cx][cz].DTexInfo2.Attr.Group <= 0)
				&& m_pRefTerrain->GetTileTex(tmpTIdx) == nullptr
				&& m_pRefTerrain->m_ppMapData[cx][cz].DTexInfo1.Attr.Attr != DTEX_FULL)
			{
				dir1 = m_pRefTerrain->m_ppMapData[cx][cz].DTexInfo1.Dir;

				u1   = (float) m_pRefTerrain->m_ppMapData[cx][cz].DTexInfo1.TexIdx.TileX * 0.125f;
				v1   = (float) m_pRefTerrain->m_ppMapData[cx][cz].DTexInfo1.TexIdx.TileY * 0.125f;

				int NumTileInColorTex = m_pRefTerrain->m_iColorMapTexSize
										/ m_pRefTerrain->m_iColorMapPixelPerUnitDistance;
				u2 = (float) (cx % NumTileInColorTex);
				v2 = (float) (NumTileInColorTex - (cz % NumTileInColorTex));

				pVertices[idx].Set((float) (cx * TERRAIN_CELL_SIZE),
					m_pRefTerrain->m_ppMapData[cx][cz].fHeight, (float) (cz * TERRAIN_CELL_SIZE),
					0.0f, 1.0f, 0.0f,
					//u1 + m_pRefTerrain->m_fTileDirU[dir1][2], v1 + m_pRefTerrain->m_fTileDirV[dir1][2],
					u2 / (float) NumTileInColorTex, v2 / (float) NumTileInColorTex, 0, 0);
				pVertices[idx + 1].Set((float) (cx * TERRAIN_CELL_SIZE),
					m_pRefTerrain->m_ppMapData[cx][cz + 1].fHeight,
					(float) ((cz + 1) * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
					//u1 + m_pRefTerrain->m_fTileDirU[dir1][0], v1 + m_pRefTerrain->m_fTileDirV[dir1][0],
					u2 / (float) NumTileInColorTex, (v2 - 1.0f) / (float) NumTileInColorTex, 0, 0);
				pVertices[idx + 2].Set((float) ((cx + 1) * TERRAIN_CELL_SIZE),
					m_pRefTerrain->m_ppMapData[cx + 1][cz + 1].fHeight,
					(float) ((cz + 1) * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
					//u1 + m_pRefTerrain->m_fTileDirU[dir1][1], v1 + m_pRefTerrain->m_fTileDirV[dir1][1],
					(u2 + 1.0f) / (float) NumTileInColorTex,
					(v2 - 1.0f) / (float) NumTileInColorTex, 0, 0);
				pVertices[idx + 3].Set((float) ((cx + 1) * TERRAIN_CELL_SIZE),
					m_pRefTerrain->m_ppMapData[cx + 1][cz].fHeight,
					(float) (cz * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
					//u1 + m_pRefTerrain->m_fTileDirU[dir1][3], v1 + m_pRefTerrain->m_fTileDirV[dir1][3],
					(u2 + 1.0f) / (float) NumTileInColorTex, v2 / (float) NumTileInColorTex, 0, 0);
			}

			else //순수하게 타일만 찍는 경우..
			{
				dir1 = m_pRefTerrain->m_ppMapData[cx][cz].DTexInfo1.Dir;
				dir2 = m_pRefTerrain->m_ppMapData[cx][cz].DTexInfo2.Dir;

				u1   = (float) m_pRefTerrain->m_ppMapData[cx][cz].DTexInfo1.TexIdx.TileX * 0.125f;
				v1   = (float) m_pRefTerrain->m_ppMapData[cx][cz].DTexInfo1.TexIdx.TileY * 0.125f;

				u2   = (float) m_pRefTerrain->m_ppMapData[cx][cz].DTexInfo2.TexIdx.TileX * 0.125f;
				v2   = (float) m_pRefTerrain->m_ppMapData[cx][cz].DTexInfo2.TexIdx.TileY * 0.125f;

				pVertices[idx].Set((float) (cx * TERRAIN_CELL_SIZE),
					m_pRefTerrain->m_ppMapData[cx][cz].fHeight, (float) (cz * TERRAIN_CELL_SIZE),
					0.0f, 1.0f, 0.0f, u1 + m_pRefTerrain->m_fTileDirU[dir1][2],
					v1 + m_pRefTerrain->m_fTileDirV[dir1][2],
					u2 + m_pRefTerrain->m_fTileDirU[dir2][2],
					v2 + m_pRefTerrain->m_fTileDirV[dir2][2]);
				pVertices[idx + 1].Set((float) (cx * TERRAIN_CELL_SIZE),
					m_pRefTerrain->m_ppMapData[cx][cz + 1].fHeight,
					(float) ((cz + 1) * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
					u1 + m_pRefTerrain->m_fTileDirU[dir1][0],
					v1 + m_pRefTerrain->m_fTileDirV[dir1][0],
					u2 + m_pRefTerrain->m_fTileDirU[dir2][0],
					v2 + m_pRefTerrain->m_fTileDirV[dir2][0]);
				pVertices[idx + 2].Set((float) ((cx + 1) * TERRAIN_CELL_SIZE),
					m_pRefTerrain->m_ppMapData[cx + 1][cz + 1].fHeight,
					(float) ((cz + 1) * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
					u1 + m_pRefTerrain->m_fTileDirU[dir1][1],
					v1 + m_pRefTerrain->m_fTileDirV[dir1][1],
					u2 + m_pRefTerrain->m_fTileDirU[dir2][1],
					v2 + m_pRefTerrain->m_fTileDirV[dir2][1]);
				pVertices[idx + 3].Set((float) ((cx + 1) * TERRAIN_CELL_SIZE),
					m_pRefTerrain->m_ppMapData[cx + 1][cz].fHeight,
					(float) (cz * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
					u1 + m_pRefTerrain->m_fTileDirU[dir1][3],
					v1 + m_pRefTerrain->m_fTileDirV[dir1][3],
					u2 + m_pRefTerrain->m_fTileDirU[dir2][3],
					v2 + m_pRefTerrain->m_fTileDirV[dir2][3]);
			}
		}
	}

	cx = m_CenterX - 1;
	cz = m_CenterZ;
	for (i = 0; i < 2; i++)
	{
		int idx = (i << 2) + 8;
		if (i == 1)
		{
			cx++;
			cz--;
		}

		//lightmap...
		if (m_pRefTerrain->m_ppIsLightMap[cx][cz])
		{
			pRefLightMapTex[NumLightMapUse] = m_pRefTerrain->m_ppLightMapTexture[cx][cz];

			if (!pLightMapVertices)
				m_pRefTerrain->m_LightMapVB->Lock(0, 0, (void**) &pLightMapVertices, 0);
			int VBIndx = NumLightMapUse * 4;

			float lsuv = 1.0f / (float) LIGHTMAP_TEX_SIZE;
			float leuv = ((float) LIGHTMAP_TEX_SIZE - 1.0f) / (float) LIGHTMAP_TEX_SIZE;

			pLightMapVertices[VBIndx].Set((float) ((cx + 1) * TERRAIN_CELL_SIZE),
				m_pRefTerrain->m_ppMapData[cx + 1][cz].fHeight, (float) (cz * TERRAIN_CELL_SIZE),
				0.0f, 1.0f, 0.0f, leuv, leuv);
			pLightMapVertices[VBIndx + 1].Set((float) (cx * TERRAIN_CELL_SIZE),
				m_pRefTerrain->m_ppMapData[cx][cz].fHeight, (float) (cz * TERRAIN_CELL_SIZE), 0.0f,
				1.0f, 0.0f, lsuv, leuv);
			pLightMapVertices[VBIndx + 2].Set((float) (cx * TERRAIN_CELL_SIZE),
				m_pRefTerrain->m_ppMapData[cx][cz + 1].fHeight,
				(float) ((cz + 1) * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f, lsuv, lsuv);
			pLightMapVertices[VBIndx + 3].Set((float) ((cx + 1) * TERRAIN_CELL_SIZE),
				m_pRefTerrain->m_ppMapData[cx + 1][cz + 1].fHeight,
				(float) ((cz + 1) * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f, leuv, lsuv);

			NumLightMapUse++;
		}

		tmpTIdx = m_pRefTerrain->m_ppMapData[cx][cz].DTexInfo1.TexIdx.TexID;
		if ((m_pRefTerrain->m_ppMapData[cx][cz].DTexInfo1.Attr.Group <= 0)
			|| m_pRefTerrain->GetTileTex(tmpTIdx) == nullptr) //타일이 없을때...
		{
			int NumTileInColorTex = m_pRefTerrain->m_iColorMapTexSize
									/ m_pRefTerrain->m_iColorMapPixelPerUnitDistance;
			u1 = (float) (cx % NumTileInColorTex);
			v1 = (float) (NumTileInColorTex - (cz % NumTileInColorTex));

			pVertices[idx].Set((float) ((cx + 1) * TERRAIN_CELL_SIZE),
				m_pRefTerrain->m_ppMapData[cx + 1][cz].fHeight, (float) (cz * TERRAIN_CELL_SIZE),
				0.0f, 1.0f, 0.0f, (u1 + 1.0f) / (float) NumTileInColorTex,
				v1 / (float) NumTileInColorTex, 0, 0);
			pVertices[idx + 1].Set((float) (cx * TERRAIN_CELL_SIZE),
				m_pRefTerrain->m_ppMapData[cx][cz].fHeight, (float) (cz * TERRAIN_CELL_SIZE), 0.0f,
				1.0f, 0.0f, u1 / (float) NumTileInColorTex, v1 / (float) NumTileInColorTex, 0, 0);
			pVertices[idx + 2].Set((float) (cx * TERRAIN_CELL_SIZE),
				m_pRefTerrain->m_ppMapData[cx][cz + 1].fHeight,
				(float) ((cz + 1) * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
				u1 / (float) NumTileInColorTex, (v1 - 1.0f) / (float) NumTileInColorTex, 0, 0);
			pVertices[idx + 3].Set((float) ((cx + 1) * TERRAIN_CELL_SIZE),
				m_pRefTerrain->m_ppMapData[cx + 1][cz + 1].fHeight,
				(float) ((cz + 1) * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
				(u1 + 1.0f) / (float) NumTileInColorTex, (v1 - 1.0f) / (float) NumTileInColorTex, 0,
				0);
		}
		else
		{
			tmpTIdx = m_pRefTerrain->m_ppMapData[cx][cz].DTexInfo2.TexIdx.TexID;
			if ((m_pRefTerrain->m_ppMapData[cx][cz].DTexInfo2.Attr.Group <= 0)
				&& m_pRefTerrain->GetTileTex(tmpTIdx) == nullptr
				&& m_pRefTerrain->m_ppMapData[cx][cz].DTexInfo1.Attr.Attr != DTEX_FULL)
			{
				dir1 = m_pRefTerrain->m_ppMapData[cx][cz].DTexInfo1.Dir;

				u1   = (float) m_pRefTerrain->m_ppMapData[cx][cz].DTexInfo1.TexIdx.TileX * 0.125f;
				v1   = (float) m_pRefTerrain->m_ppMapData[cx][cz].DTexInfo1.TexIdx.TileY * 0.125f;

				int NumTileInColorTex = m_pRefTerrain->m_iColorMapTexSize
										/ m_pRefTerrain->m_iColorMapPixelPerUnitDistance;
				u2 = (float) (cx % NumTileInColorTex);
				v2 = (float) (NumTileInColorTex - (cz % NumTileInColorTex));

				pVertices[idx].Set((float) ((cx + 1) * TERRAIN_CELL_SIZE),
					m_pRefTerrain->m_ppMapData[cx + 1][cz].fHeight,
					(float) (cz * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
					//u1 + m_pRefTerrain->m_fTileDirU[dir1][3], v1 + m_pRefTerrain->m_fTileDirV[dir1][3],
					(u2 + 1.0f) / (float) NumTileInColorTex, v2 / (float) NumTileInColorTex, 0, 0);
				pVertices[idx + 1].Set((float) (cx * TERRAIN_CELL_SIZE),
					m_pRefTerrain->m_ppMapData[cx][cz].fHeight, (float) (cz * TERRAIN_CELL_SIZE),
					0.0f, 1.0f, 0.0f,
					//u1 + m_pRefTerrain->m_fTileDirU[dir1][2], v1 + m_pRefTerrain->m_fTileDirV[dir1][2],
					u2 / (float) NumTileInColorTex, v2 / (float) NumTileInColorTex, 0, 0);
				pVertices[idx + 2].Set((float) (cx * TERRAIN_CELL_SIZE),
					m_pRefTerrain->m_ppMapData[cx][cz + 1].fHeight,
					(float) ((cz + 1) * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
					//u1 + m_pRefTerrain->m_fTileDirU[dir1][0], v1 + m_pRefTerrain->m_fTileDirV[dir1][0],
					u2 / (float) NumTileInColorTex, (v2 - 1.0f) / (float) NumTileInColorTex, 0, 0);
				pVertices[idx + 3].Set((float) ((cx + 1) * TERRAIN_CELL_SIZE),
					m_pRefTerrain->m_ppMapData[cx + 1][cz + 1].fHeight,
					(float) ((cz + 1) * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
					//u1 + m_pRefTerrain->m_fTileDirU[dir1][1], v1 + m_pRefTerrain->m_fTileDirV[dir1][1],
					(u2 + 1.0f) / (float) NumTileInColorTex,
					(v2 - 1.0f) / (float) NumTileInColorTex, 0, 0);
			}
			else
			{
				dir1 = m_pRefTerrain->m_ppMapData[cx][cz].DTexInfo1.Dir;
				dir2 = m_pRefTerrain->m_ppMapData[cx][cz].DTexInfo2.Dir;

				u1   = (float) m_pRefTerrain->m_ppMapData[cx][cz].DTexInfo1.TexIdx.TileX * 0.125f;
				v1   = (float) m_pRefTerrain->m_ppMapData[cx][cz].DTexInfo1.TexIdx.TileY * 0.125f;

				u2   = (float) m_pRefTerrain->m_ppMapData[cx][cz].DTexInfo2.TexIdx.TileX * 0.125f;
				v2   = (float) m_pRefTerrain->m_ppMapData[cx][cz].DTexInfo2.TexIdx.TileY * 0.125f;

				pVertices[idx].Set((float) ((cx + 1) * TERRAIN_CELL_SIZE),
					m_pRefTerrain->m_ppMapData[cx + 1][cz].fHeight,
					(float) (cz * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
					u1 + m_pRefTerrain->m_fTileDirU[dir1][3],
					v1 + m_pRefTerrain->m_fTileDirV[dir1][3],
					u2 + m_pRefTerrain->m_fTileDirU[dir2][3],
					v2 + m_pRefTerrain->m_fTileDirV[dir2][3]);
				pVertices[idx + 1].Set((float) (cx * TERRAIN_CELL_SIZE),
					m_pRefTerrain->m_ppMapData[cx][cz].fHeight, (float) (cz * TERRAIN_CELL_SIZE),
					0.0f, 1.0f, 0.0f, u1 + m_pRefTerrain->m_fTileDirU[dir1][2],
					v1 + m_pRefTerrain->m_fTileDirV[dir1][2],
					u2 + m_pRefTerrain->m_fTileDirU[dir2][2],
					v2 + m_pRefTerrain->m_fTileDirV[dir2][2]);
				pVertices[idx + 2].Set((float) (cx * TERRAIN_CELL_SIZE),
					m_pRefTerrain->m_ppMapData[cx][cz + 1].fHeight,
					(float) ((cz + 1) * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
					u1 + m_pRefTerrain->m_fTileDirU[dir1][0],
					v1 + m_pRefTerrain->m_fTileDirV[dir1][0],
					u2 + m_pRefTerrain->m_fTileDirU[dir2][0],
					v2 + m_pRefTerrain->m_fTileDirV[dir2][0]);
				pVertices[idx + 3].Set((float) ((cx + 1) * TERRAIN_CELL_SIZE),
					m_pRefTerrain->m_ppMapData[cx + 1][cz + 1].fHeight,
					(float) ((cz + 1) * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
					u1 + m_pRefTerrain->m_fTileDirU[dir1][1],
					v1 + m_pRefTerrain->m_fTileDirV[dir1][1],
					u2 + m_pRefTerrain->m_fTileDirU[dir2][1],
					v2 + m_pRefTerrain->m_fTileDirV[dir2][1]);
			}
		}
	}
	m_pRefTerrain->m_TileVB->Unlock();
	if (pLightMapVertices)
		m_pRefTerrain->m_LightMapVB->Unlock();

	m_pRefTerrain->s_lpD3DDev->SetStreamSource(0, m_pRefTerrain->m_TileVB, 0, sizeof(__VertexT2));
	m_pRefTerrain->s_lpD3DDev->SetFVF(FVF_VNT2);
	/*
	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

//	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_MODULATE);
	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_ADD);
	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT);

	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_MODULATE);
	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_CURRENT);
	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
*/
	int tx, tz;

	DWORD COP2 = D3DTOP_ADD;
	for (i = 0; i < 4; i++)
	{
		COP2 = D3DTOP_ADD;
		tx   = m_CenterX;
		tz   = m_CenterZ;
		if (i == 0 || i == 2)
			tx = m_CenterX - 1;
		if (i == 0 || i == 3)
			tz = m_CenterZ - 1;

		int t1 = m_pRefTerrain->m_ppMapData[tx][tz].DTexInfo1.TexIdx.TexID;
		int t2 = m_pRefTerrain->m_ppMapData[tx][tz].DTexInfo2.TexIdx.TexID;

		if (m_pRefTerrain->m_ppMapData[tx][tz].DTexInfo1.Attr.Group > 0
			&& m_pRefTerrain->GetTileTex(t1)
			&& (m_pRefTerrain->m_ppMapData[tx][tz].DTexInfo1.Attr.Attr == DTEX_FULL
				|| m_pRefTerrain->m_ppMapData[tx][tz].DTexInfo2.Attr.Group > 0))
			hr = m_pRefTerrain->s_lpD3DDev->SetTexture(0, m_pRefTerrain->GetTileTex(t1)->Get());
		else
		{
			int ctx = tx
					  / (m_pRefTerrain->m_iColorMapTexSize
						  / m_pRefTerrain->m_iColorMapPixelPerUnitDistance);
			int ctz = tz
					  / (m_pRefTerrain->m_iColorMapTexSize
						  / m_pRefTerrain->m_iColorMapPixelPerUnitDistance);
			//int ctx = (tx * TERRAIN_CELL_SIZE) / (m_pRefTerrain->m_iColorMapTexSize * m_pRefTerrain->m_iColorMapPixelPerUnitDistance);
			//int ctz = (tz * TERRAIN_CELL_SIZE) / (m_pRefTerrain->m_iColorMapTexSize * m_pRefTerrain->m_iColorMapPixelPerUnitDistance);
			hr = m_pRefTerrain->s_lpD3DDev->SetTexture(
				0, m_pRefTerrain->m_pColorTexture[ctx][ctz].Get());
			COP2 = D3DTOP_DISABLE;
		}

		if (m_pRefTerrain->m_ppMapData[tx][tz].DTexInfo2.Attr.Group > 0
			&& m_pRefTerrain->GetTileTex(t2))
			hr = m_pRefTerrain->s_lpD3DDev->SetTexture(1, m_pRefTerrain->GetTileTex(t2)->Get());
		//else hr = m_pRefTerrain->s_lpD3DDev->SetTexture( 1, nullptr);
		else
		{
			hr   = m_pRefTerrain->s_lpD3DDev->SetTexture(1, nullptr);
			COP2 = D3DTOP_DISABLE;
			/*
			if(m_pRefTerrain->m_ppMapData[tx][tz].DTexInfo1.Attr.Attr > DTEX_FULL 
				&& m_pRefTerrain->m_ppMapData[tx][tz].DTexInfo1.Attr.Attr < DTEX_MAX)
			{
				COP2 = D3DTOP_ADD;
				int ctx = (tx * TERRAIN_CELL_SIZE) / (m_pRefTerrain->m_iColorMapTexSize * m_pRefTerrain->m_iColorMapPixelPerUnitDistance);
				int ctz = (tz * TERRAIN_CELL_SIZE) / (m_pRefTerrain->m_iColorMapTexSize * m_pRefTerrain->m_iColorMapPixelPerUnitDistance);
				hr = m_pRefTerrain->s_lpD3DDev->SetTexture( 1, m_pRefTerrain->m_pColorTexture[ctx][ctz].Get() );
			}
			*/
		}

		/*
		hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		//hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

		hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP, COP2);
		hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT);

		if(COP2 = D3DTOP_ADD)
		{
			hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_MODULATE);
			hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_CURRENT);
			hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		}
		*/
		if (D3DTOP_DISABLE == COP2)
		{
			hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState(
				0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState(
				0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState(
				0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

			hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLOROP, COP2);
		}
		else // if (D3DTOP_ADD == COP2)
		{
			hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState(
				0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState(
				0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

			hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLOROP, COP2);
			hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState(
				1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState(
				1, D3DTSS_COLORARG2, D3DTA_CURRENT);

			hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState(
				2, D3DTSS_COLOROP, D3DTOP_MODULATE);
			hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState(
				2, D3DTSS_COLORARG1, D3DTA_CURRENT);
			hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState(
				2, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			m_pRefTerrain->s_lpD3DDev->SetTexture(2, nullptr);
		}

		hr = m_pRefTerrain->s_lpD3DDev->DrawPrimitive(D3DPT_TRIANGLEFAN, (i << 2), 2);
	}

	// Render Light Map...
	m_pRefTerrain->s_lpD3DDev->SetStreamSource(
		0, m_pRefTerrain->m_LightMapVB, 0, sizeof(__VertexT1));
	m_pRefTerrain->s_lpD3DDev->SetFVF(FVF_VNT1);

	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);

	DWORD dwAlphaEnable, dwSrcBlend, dwDestBlend;

	hr = m_pRefTerrain->s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlphaEnable);
	hr = m_pRefTerrain->s_lpD3DDev->GetRenderState(D3DRS_SRCBLEND, &dwSrcBlend);
	hr = m_pRefTerrain->s_lpD3DDev->GetRenderState(D3DRS_DESTBLEND, &dwDestBlend);

	hr = m_pRefTerrain->s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	hr = m_pRefTerrain->s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	hr = m_pRefTerrain->s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	for (i = 0; i < NumLightMapUse; i++)
	{
		m_pRefTerrain->s_lpD3DDev->SetTexture(0, pRefLightMapTex[i]->Get());
		hr = m_pRefTerrain->s_lpD3DDev->DrawPrimitive(D3DPT_TRIANGLEFAN, (i << 2), 2);
	}
	//

	hr = m_pRefTerrain->s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlphaEnable);
	hr = m_pRefTerrain->s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, dwSrcBlend);
	hr = m_pRefTerrain->s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, dwDestBlend);

	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, ColorOP1);
	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, ColorArg11);
	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG2, ColorArg12);

	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLOROP, ColorOP2);
	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLORARG1, ColorArg21);
	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLORARG2, ColorArg22);
}

/*
void CQTNode::RenderMaxLevel()
{
	__VertexT2* pVertices;
	m_pRefTerrain->m_TileVB->Lock( 0, 0, (BYTE**)&pVertices, 0 );

	int cx,cz;
	int i;
	cx = m_CenterX;
	cz = m_CenterZ;

	int dir1, dir2;
	float u1, u2, v1, v2;
	for(i=0;i<2;i++)
	{
		int idx = i<<2;
		if(i==1) { cx++; cz++; }

		dir1 = m_pRefTerrain->m_ppMapData[cx-1][cz-1].DTexInfo1.Dir;
		dir2 = m_pRefTerrain->m_ppMapData[cx-1][cz-1].DTexInfo2.Dir;

		u1 = (float)m_pRefTerrain->m_ppMapData[cx-1][cz-1].DTexInfo1.TexIdx.TileX * 0.25f;
		v1 = (float)m_pRefTerrain->m_ppMapData[cx-1][cz-1].DTexInfo1.TexIdx.TileY * 0.25f;

		u2 = (float)m_pRefTerrain->m_ppMapData[cx-1][cz-1].DTexInfo2.TexIdx.TileX * 0.25f;
		v2 = (float)m_pRefTerrain->m_ppMapData[cx-1][cz-1].DTexInfo2.TexIdx.TileY * 0.25f;
		
		pVertices[idx].Set((float)(cx*TERRAIN_CELL_SIZE), m_pRefTerrain->m_ppMapData[cx][cz].fHeight,
					(float)(cz*TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
					u1 + m_pRefTerrain->m_fTileDirU[dir1][1], v1 + m_pRefTerrain->m_fTileDirV[dir1][1],
					u2 + m_pRefTerrain->m_fTileDirU[dir2][1], v2 + m_pRefTerrain->m_fTileDirV[dir2][1]);
		pVertices[idx+1].Set((float)(cx*TERRAIN_CELL_SIZE), m_pRefTerrain->m_ppMapData[cx][cz-1].fHeight,
					(float)((cz-1)*TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
					u1 + m_pRefTerrain->m_fTileDirU[dir1][3], v1 + m_pRefTerrain->m_fTileDirV[dir1][3],
					u2 + m_pRefTerrain->m_fTileDirU[dir2][3], v2 + m_pRefTerrain->m_fTileDirV[dir2][3]);
		pVertices[idx+2].Set((float)((cx-1)*TERRAIN_CELL_SIZE), m_pRefTerrain->m_ppMapData[cx-1][cz-1].fHeight,
					(float)((cz-1)*TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
					u1 + m_pRefTerrain->m_fTileDirU[dir1][2], v1 + m_pRefTerrain->m_fTileDirV[dir1][2],
					u2 + m_pRefTerrain->m_fTileDirU[dir2][2], v2 + m_pRefTerrain->m_fTileDirV[dir2][2]);
		pVertices[idx+3].Set((float)((cx-1)*TERRAIN_CELL_SIZE), m_pRefTerrain->m_ppMapData[cx-1][cz].fHeight,
					(float)(cz*TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
					u1 + m_pRefTerrain->m_fTileDirU[dir1][0], v1 + m_pRefTerrain->m_fTileDirV[dir1][0],
					u2 + m_pRefTerrain->m_fTileDirU[dir2][0], v2 + m_pRefTerrain->m_fTileDirV[dir2][0]);
	}
	cx = m_CenterX;
	cz = m_CenterZ;
	for(i=0;i<2;i++)
	{
		int idx = (i<<2) + 8;
		if(i==1) { cx++; cz--; }

		dir1 = m_pRefTerrain->m_ppMapData[cx-1][cz].DTexInfo1.Dir;
		dir2 = m_pRefTerrain->m_ppMapData[cx-1][cz].DTexInfo2.Dir;

		u1 = (float)m_pRefTerrain->m_ppMapData[cx-1][cz].DTexInfo1.TexIdx.TileX * 0.25f;
		v1 = (float)m_pRefTerrain->m_ppMapData[cx-1][cz].DTexInfo1.TexIdx.TileY * 0.25f;

		u2 = (float)m_pRefTerrain->m_ppMapData[cx-1][cz].DTexInfo2.TexIdx.TileX * 0.25f;
		v2 = (float)m_pRefTerrain->m_ppMapData[cx-1][cz].DTexInfo2.TexIdx.TileY * 0.25f;

		pVertices[idx].Set((float)(cx*TERRAIN_CELL_SIZE), m_pRefTerrain->m_ppMapData[cx][cz].fHeight,
					(float)(cz*TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f, 
					u1 + m_pRefTerrain->m_fTileDirU[dir1][3], v1 + m_pRefTerrain->m_fTileDirV[dir1][3],
					u2 + m_pRefTerrain->m_fTileDirU[dir2][3], v2 + m_pRefTerrain->m_fTileDirV[dir2][3]);
		pVertices[idx+1].Set((float)((cx-1)*TERRAIN_CELL_SIZE), m_pRefTerrain->m_ppMapData[cx-1][cz].fHeight,
					(float)(cz*TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
					u1 + m_pRefTerrain->m_fTileDirU[dir1][2], v1 + m_pRefTerrain->m_fTileDirV[dir1][2],
					u2 + m_pRefTerrain->m_fTileDirU[dir2][2], v2 + m_pRefTerrain->m_fTileDirV[dir2][2]);
		pVertices[idx+2].Set((float)((cx-1)*TERRAIN_CELL_SIZE), m_pRefTerrain->m_ppMapData[cx-1][cz+1].fHeight,
					(float)((cz+1)*TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
					u1 + m_pRefTerrain->m_fTileDirU[dir1][0], v1 + m_pRefTerrain->m_fTileDirV[dir1][0],
					u2 + m_pRefTerrain->m_fTileDirU[dir2][0], v2 + m_pRefTerrain->m_fTileDirV[dir2][0]);
		pVertices[idx+3].Set((float)(cx*TERRAIN_CELL_SIZE), m_pRefTerrain->m_ppMapData[cx][cz+1].fHeight,
					(float)((cz+1)*TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
					u1 + m_pRefTerrain->m_fTileDirU[dir1][1], v1 + m_pRefTerrain->m_fTileDirV[dir1][1],
					u2 + m_pRefTerrain->m_fTileDirU[dir2][1], v2 + m_pRefTerrain->m_fTileDirV[dir2][1]);
	}
	m_pRefTerrain->m_TileVB->Unlock();


	m_pRefTerrain->s_lpD3DDev->SetStreamSource( 0, m_pRefTerrain->m_TileVB, sizeof(__VertexT2) );
	m_pRefTerrain->s_lpD3DDev->SetVertexShader( FVF_VNT2 );

	HRESULT hr;
	DWORD ColorArg11, ColorArg12, ColorArg21, ColorArg22;
	DWORD ColorOP1, ColorOP2;

	hr = m_pRefTerrain->s_lpD3DDev->GetTextureStageState( 0, D3DTSS_COLOROP, &ColorOP1);
	hr = m_pRefTerrain->s_lpD3DDev->GetTextureStageState( 0, D3DTSS_COLORARG1, &ColorArg11);
	hr = m_pRefTerrain->s_lpD3DDev->GetTextureStageState( 0, D3DTSS_COLORARG2, &ColorArg12);

	hr = m_pRefTerrain->s_lpD3DDev->GetTextureStageState( 1, D3DTSS_COLOROP, &ColorOP2);
	hr = m_pRefTerrain->s_lpD3DDev->GetTextureStageState( 1, D3DTSS_COLORARG1, &ColorArg21);
	hr = m_pRefTerrain->s_lpD3DDev->GetTextureStageState( 1, D3DTSS_COLORARG2, &ColorArg22);

	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_MODULATE);
	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT);

	int tx, tz;	
	for(i=0;i<4;i++)
	{
		tx = m_CenterX;
		tz = m_CenterZ;
		if(i==0 || i==2) tx = m_CenterX-1;
		if(i==0 || i==3) tz = m_CenterZ-1;

		int t1 = m_pRefTerrain->m_ppMapData[tx][tz].DTexInfo1.TexIdx.TexID;
		int t2 = m_pRefTerrain->m_ppMapData[tx][tz].DTexInfo2.TexIdx.TexID;

		if(m_pRefTerrain->m_ppMapData[tx][tz].DTexInfo1.bActive && m_pRefTerrain->GetTileTex(t1)) 
			hr = m_pRefTerrain->s_lpD3DDev->SetTexture( 0, m_pRefTerrain->GetTileTex(t1)->Get());
		//else hr = m_pRefTerrain->s_lpD3DDev->SetTexture( 0, nullptr);
		else
		{
			int ctx = (tx * TERRAIN_CELL_SIZE) / (m_pRefTerrain->m_iColorMapTexSize * m_pRefTerrain->m_iColorMapPixelPerUnitDistance);
			int ctz = (tz * TERRAIN_CELL_SIZE) / (m_pRefTerrain->m_iColorMapTexSize * m_pRefTerrain->m_iColorMapPixelPerUnitDistance);
			hr = m_pRefTerrain->s_lpD3DDev->SetTexture( 0, m_pRefTerrain->m_pColorTexture[ctx][ctz].Get() );
		}

		if(m_pRefTerrain->m_ppMapData[tx][tz].DTexInfo2.bActive && m_pRefTerrain->GetTileTex(t2))
			hr = m_pRefTerrain->s_lpD3DDev->SetTexture( 1, m_pRefTerrain->GetTileTex(t2)->Get());
		else hr = m_pRefTerrain->s_lpD3DDev->SetTexture( 1, nullptr);
				
		hr = m_pRefTerrain->s_lpD3DDev->DrawPrimitive( D3DPT_TRIANGLEFAN, (i<<2), 2);

	}

	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP, ColorOP1);
	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1, ColorArg11);
	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG2, ColorArg12);

	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP, ColorOP2);
	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLORARG1, ColorArg21);
	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLORARG2, ColorArg22);
}
*/

//
//	m_pRefTerrain->m_iMaxLevel이 아닌 Level들 Render..
//
void CQTNode::RenderNormalLevel()
{
	int LeftLevel   = 0;
	int RightLevel  = 0;
	int TopLevel    = 0;
	int BottomLevel = 0;

	if (m_pFriend[DIR_LEFT])
		LeftLevel = m_pFriend[DIR_LEFT]->GetMaxLevel(DIR_RIGHT);
	if (m_pFriend[DIR_RIGHT])
		RightLevel = m_pFriend[DIR_RIGHT]->GetMaxLevel(DIR_LEFT);
	if (m_pFriend[DIR_TOP])
		TopLevel = m_pFriend[DIR_TOP]->GetMaxLevel(DIR_BOTTOM);
	if (m_pFriend[DIR_BOTTOM])
		BottomLevel = m_pFriend[DIR_BOTTOM]->GetMaxLevel(DIR_TOP);

	int VC   = 0;
	int half = 1 << (m_pRefTerrain->m_iMaxLevel - m_Level);
	__VertexT1* pVertices;
	m_pRefTerrain->m_ColorMapVB->Lock(0, 0, (void**) &pVertices, 0);

	//점만들기..
	int NumTileInColorTex = m_pRefTerrain->m_iColorMapTexSize
							/ m_pRefTerrain->m_iColorMapPixelPerUnitDistance;
	//float cx = (float)((m_CenterX % NumTileInColorTex) * m_pRefTerrain->m_iColorMapPixelPerUnitDistance);
	//float cz = (float)(((NumTileInColorTex-m_CenterZ) % NumTileInColorTex) * m_pRefTerrain->m_iColorMapPixelPerUnitDistance);
	///////////////////

	float cx = (float) (m_CenterX % NumTileInColorTex);
	float cz = (float) (NumTileInColorTex - (m_CenterZ % NumTileInColorTex));

	pVertices[VC].Set((float) (m_CenterX * TERRAIN_CELL_SIZE),
		m_pRefTerrain->m_ppMapData[m_CenterX][m_CenterZ].fHeight,
		(float) (m_CenterZ * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f, cx / (float) NumTileInColorTex,
		cz / (float) NumTileInColorTex);
	VC++;
	pVertices[VC].Set((float) ((m_CenterX - half) * TERRAIN_CELL_SIZE),
		m_pRefTerrain->m_ppMapData[m_CenterX - half][m_CenterZ - half].fHeight,
		(float) ((m_CenterZ - half) * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
		(cx - (float) half) / (float) NumTileInColorTex,
		(cz + (float) half) / (float) NumTileInColorTex);
	VC++;

	//왼쪽변..
	int interval = 1;
	int sp; //시작점..
	float UVInterval;
	if (m_Level < LeftLevel)
	{
		if (LeftLevel == m_pRefTerrain->m_iMaxLevel)
			interval = 1;
		else
			interval = 1 << (m_pRefTerrain->m_iMaxLevel - LeftLevel + 1);
		sp = m_CenterZ - half;

		for (int z = interval; z < (half << 1); z += interval)
		{
			if (m_pRefTerrain->m_ppRenderInfo[m_CenterX - half][sp + z] == FALSE)
				continue;

			pVertices[VC].Set((float) ((m_CenterX - half) * TERRAIN_CELL_SIZE),
				m_pRefTerrain->m_ppMapData[m_CenterX - half][sp + z].fHeight,
				(float) ((sp + z) * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
				(float) (cx - half) / (float) NumTileInColorTex,
				(float) (cz + half - z) / (float) NumTileInColorTex);
			VC++;
		}
	}
	pVertices[VC].Set((float) ((m_CenterX - half) * TERRAIN_CELL_SIZE),
		m_pRefTerrain->m_ppMapData[m_CenterX - half][m_CenterZ + half].fHeight,
		(float) ((m_CenterZ + half) * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
		(float) (cx - half) / (float) NumTileInColorTex,
		(float) (cz - half) / (float) NumTileInColorTex);
	VC++;

	//윗쪽변..
	if (m_Level < TopLevel)
	{
		if (TopLevel == m_pRefTerrain->m_iMaxLevel)
			interval = 1;
		else
			interval = 1 << (m_pRefTerrain->m_iMaxLevel - TopLevel + 1);
		sp = m_CenterX - half;

		for (int x = interval; x < (half << 1); x += interval)
		{
			if (m_pRefTerrain->m_ppRenderInfo[sp + x][m_CenterZ + half] == FALSE)
				continue;
			pVertices[VC].Set((float) ((sp + x) * TERRAIN_CELL_SIZE),
				m_pRefTerrain->m_ppMapData[sp + x][m_CenterZ + half].fHeight,
				(float) ((m_CenterZ + half) * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
				(float) (cx - half + x) / (float) NumTileInColorTex,
				(float) (cz - half) / (float) NumTileInColorTex);
			VC++;
		}
	}
	pVertices[VC].Set((float) ((m_CenterX + half) * TERRAIN_CELL_SIZE),
		m_pRefTerrain->m_ppMapData[m_CenterX + half][m_CenterZ + half].fHeight,
		(float) ((m_CenterZ + half) * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
		(float) (cx + half) / (float) NumTileInColorTex,
		(float) (cz - half) / (float) NumTileInColorTex);
	VC++;

	//오른쪽변..
	if (m_Level < RightLevel)
	{
		if (RightLevel == m_pRefTerrain->m_iMaxLevel)
			interval = 1;
		else
			interval = 1 << (m_pRefTerrain->m_iMaxLevel - RightLevel + 1);
		sp = m_CenterZ + half;

		for (int z = interval; z < (half << 1); z += interval)
		{
			if (m_pRefTerrain->m_ppRenderInfo[m_CenterX + half][sp - z] == FALSE)
				continue;

			pVertices[VC].Set((float) ((m_CenterX + half) * TERRAIN_CELL_SIZE),
				m_pRefTerrain->m_ppMapData[m_CenterX + half][sp - z].fHeight,
				(float) ((sp - z) * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
				(float) (cx + half) / (float) NumTileInColorTex,
				(float) (cz - half + z) / (float) NumTileInColorTex);
			VC++;
		}
	}
	pVertices[VC].Set((float) ((m_CenterX + half) * TERRAIN_CELL_SIZE),
		m_pRefTerrain->m_ppMapData[m_CenterX + half][m_CenterZ - half].fHeight,
		(float) ((m_CenterZ - half) * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
		(float) (cx + half) / (float) NumTileInColorTex,
		(float) (cz + half) / (float) NumTileInColorTex);
	VC++;

	//아랫변..
	if (m_Level < BottomLevel)
	{
		if (BottomLevel == m_pRefTerrain->m_iMaxLevel)
			interval = 1;
		else
			interval = 1 << (m_pRefTerrain->m_iMaxLevel - BottomLevel + 1);
		sp = m_CenterX + half;

		for (int x = interval; x < (half << 1); x += interval)
		{
			if (m_pRefTerrain->m_ppRenderInfo[sp - x][m_CenterZ - half] == FALSE)
				continue;

			UVInterval = (float) (x * m_pRefTerrain->m_iColorMapPixelPerUnitDistance);
			pVertices[VC].Set((float) ((sp - x) * TERRAIN_CELL_SIZE),
				m_pRefTerrain->m_ppMapData[sp - x][m_CenterZ - half].fHeight,
				(float) ((m_CenterZ - half) * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
				(float) (cx + half - x) / (float) NumTileInColorTex,
				(float) (cz + half) / (float) NumTileInColorTex);
			VC++;
		}
	}
	pVertices[VC].Set((float) ((m_CenterX - half) * TERRAIN_CELL_SIZE),
		m_pRefTerrain->m_ppMapData[m_CenterX - half][m_CenterZ - half].fHeight,
		(float) ((m_CenterZ - half) * TERRAIN_CELL_SIZE), 0.0f, 1.0f, 0.0f,
		(float) (cx - half) / (float) NumTileInColorTex,
		(float) (cz + half) / (float) NumTileInColorTex);
	VC++;

	m_pRefTerrain->m_ColorMapVB->Unlock();

	m_pRefTerrain->s_lpD3DDev->SetStreamSource(
		0, m_pRefTerrain->m_ColorMapVB, 0, sizeof(__VertexT1));
	m_pRefTerrain->s_lpD3DDev->SetFVF(FVF_VNT1);

	HRESULT hr;
	int tx, tz;
	tx = m_CenterX
		 / (m_pRefTerrain->m_iColorMapTexSize / m_pRefTerrain->m_iColorMapPixelPerUnitDistance);
	tz = m_CenterZ
		 / (m_pRefTerrain->m_iColorMapTexSize / m_pRefTerrain->m_iColorMapPixelPerUnitDistance);

	hr = m_pRefTerrain->s_lpD3DDev->SetTexture(0, m_pRefTerrain->m_pColorTexture[tx][tz].Get());

	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

	DWORD ColorArg11, ColorArg12;
	hr = m_pRefTerrain->s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLORARG1, &ColorArg11);
	hr = m_pRefTerrain->s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLORARG2, &ColorArg12);

	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	hr = m_pRefTerrain->s_lpD3DDev->SetTexture(1, nullptr);

	hr = m_pRefTerrain->s_lpD3DDev->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, VC - 2);

	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, ColorArg11);
	hr = m_pRefTerrain->s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG2, ColorArg12);
}

//
//	GetMaxLevel..
//
int CQTNode::GetMaxLevel(eDIR dir)
{
	if (m_State == QTNODESTATE_CLIPPED)
		return 0;
	if (m_State == QTNODESTATE_RENDER)
		return m_Level;
	int l1, l2;
	if (m_State == QTNODESTATE_NOTRENDER)
	{
		if (dir == DIR_RIGHT)
		{
			l1 = m_pChild[DIR_TOP]->GetMaxLevel(DIR_RIGHT);
			l2 = m_pChild[DIR_RIGHT]->GetMaxLevel(DIR_RIGHT);
			return std::max(l1, l2);
		}
		if (dir == DIR_LEFT)
		{
			l1 = m_pChild[DIR_LEFT]->GetMaxLevel(DIR_LEFT);
			l2 = m_pChild[DIR_BOTTOM]->GetMaxLevel(DIR_LEFT);
			return std::max(l1, l2);
		}
		if (dir == DIR_TOP)
		{
			l1 = m_pChild[DIR_LEFT]->GetMaxLevel(DIR_TOP);
			l2 = m_pChild[DIR_TOP]->GetMaxLevel(DIR_TOP);
			return std::max(l1, l2);
		}
		if (dir == DIR_BOTTOM)
		{
			l1 = m_pChild[DIR_BOTTOM]->GetMaxLevel(DIR_BOTTOM);
			l2 = m_pChild[DIR_RIGHT]->GetMaxLevel(DIR_BOTTOM);
			return std::max(l1, l2);
		}
	}
	return 0;
}

// N3Patch.h: interface for the CN3TerrainPatch class.
// 2001.10.22.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LYTERRAIN_H__38B10089_4193_49B7_BF2A_CD9C6CD3251A__INCLUDED_)
#define AFX_LYTERRAIN_H__38B10089_4193_49B7_BF2A_CD9C6CD3251A__INCLUDED_

#pragma once

#include "N3TerrainDef.h"

class CN3Texture;
class CN3Terrain;
class CN3TerrainPatch
{
public:
	BOOL m_bIsRender;
	POINT m_ti_LBPoint;
	CN3Terrain* m_pRefTerrain;
	bool m_IsBlunt[4]; //Left, Top, Right, Bottom...
	CN3Texture* m_pRefColorTex;

protected:
	int m_CellSize;
	int m_NumCell;
	int m_iLevel;
	LPDIRECT3DVERTEXBUFFER9 m_pVB;
	int m_VBSize[MAX_PATCH_LEVEL];

	FanInfoList m_FanInfoList;

	//for level1
	uint32_t* m_pTileTexIndx[2];
	bool* m_pIsTileFull;

	//lightmap...
	LPDIRECT3DVERTEXBUFFER9 m_pLightMapVB;
	CN3Texture** m_pRefLightMapTex;
	int m_NumLightMapTex;

	//Tile지원 안하는 카드들을 위한 설정.
	//LPDIRECT3DVERTEXBUFFER8		m_pFoolishTileVB;
	//int							m_NumFoolishTile;

public:
	inline float UVConvert(float uv);

	int GetLevel() const
	{
		return m_iLevel;
	}

	void SetLevel(int level);
	void Release();
	void Init();
	void Init(CN3Terrain* pTerrain);
	void Tick();
	void Render();

public:
	CN3TerrainPatch();
	~CN3TerrainPatch();
};

#endif // !defined(AFX_LYTERRAIN_H__38B10089_4193_49B7_BF2A_CD9C6CD3251A__INCLUDED_)

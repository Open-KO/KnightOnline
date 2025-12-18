// SoundCell.cpp: implementation of the CSoundCell class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "n3me.h"
#include "SoundCell.h"
#include <N3Base/N3Base.h>
#include "LyTerrainDef.h"
#include "LyTerrain.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSoundCell::CSoundCell()
{
	m_iVersion = 1;
	m_dwSoundGroupID = 0;
	m_pRefTerrain = nullptr;

	m_Rect.left = m_Rect.right = m_Rect.top = m_Rect.bottom = 0;
	m_BasePoint.x = m_BasePoint.y = 0;
}

CSoundCell::CSoundCell(CLyTerrain* pRefTerrain)
{
	m_iVersion = 1;
	m_dwSoundGroupID = 0;
	m_pRefTerrain = pRefTerrain;

	m_Rect.left = m_Rect.right = m_Rect.top = m_Rect.bottom = 0;
	m_BasePoint.x = m_BasePoint.y = 0;
}

CSoundCell::~CSoundCell()
{
}

void CSoundCell::AddRect(__Vector3 v)
{
	int x, z;

	x = (int)v.x / TERRAIN_CELL_SIZE;
	z = (int)v.z / TERRAIN_CELL_SIZE;

	if(m_BasePoint.x > x)
	{
		m_Rect.left = x;
		m_Rect.right = m_BasePoint.x;
	}
	else
	{
		m_Rect.left = m_BasePoint.x;
		m_Rect.right = x;
	}

	if(m_BasePoint.y > z)
	{
		m_Rect.bottom = z;
		m_Rect.top = m_BasePoint.y;
	}
	else
	{
		m_Rect.bottom = m_BasePoint.y;
		m_Rect.top = z;
	}
}

void CSoundCell::InitRect(__Vector3 v)
{
	m_BasePoint.x = m_Rect.left = m_Rect.right = (int)v.x / TERRAIN_CELL_SIZE;
	m_BasePoint.y = m_Rect.top = m_Rect.bottom = (int)v.z / TERRAIN_CELL_SIZE;
}

void CSoundCell::MakeTileVB(int x, int z, DWORD color)
{
	if(!m_pRefTerrain) return;

	if((x+z)%2==0)	// 슬래쉬 모양의 타일..
	{
		__Vector3 v;
		v.x = x*TERRAIN_CELL_SIZE;
		v.z = z*TERRAIN_CELL_SIZE;
		v.y = m_pRefTerrain->GetHeight(v.x, v.z);
		m_TileVB[0].Set(v.x, v.y, v.z, color);

		v.z = (z+1)*TERRAIN_CELL_SIZE;
		v.y = m_pRefTerrain->GetHeight(v.x, v.z);
		m_TileVB[1].Set(v.x, v.y, v.z, color);

		v.x = (x+1)*TERRAIN_CELL_SIZE;
		v.y = m_pRefTerrain->GetHeight(v.x, v.z);
		m_TileVB[2].Set(v.x, v.y, v.z, color);

		v.z = z*TERRAIN_CELL_SIZE;
		v.y = m_pRefTerrain->GetHeight(v.x, v.z);
		m_TileVB[3].Set(v.x, v.y, v.z, color);

		return;
	}
	if((x+z)%2==1)	//백슬레쉬 모양의 타일..
	{
		__Vector3 v;
		v.x = x*TERRAIN_CELL_SIZE;
		v.z = (z+1)*TERRAIN_CELL_SIZE;
		v.y = m_pRefTerrain->GetHeight(v.x, v.z);
		m_TileVB[0].Set(v.x, v.y, v.z, color);

		v.x = (x+1)*TERRAIN_CELL_SIZE;
		v.y = m_pRefTerrain->GetHeight(v.x, v.z);
		m_TileVB[1].Set(v.x, v.y, v.z, color);

		v.z = z*TERRAIN_CELL_SIZE;
		v.y = m_pRefTerrain->GetHeight(v.x, v.z);
		m_TileVB[2].Set(v.x, v.y, v.z, color);

		v.x = x*TERRAIN_CELL_SIZE;
		v.y = m_pRefTerrain->GetHeight(v.x, v.z);
		m_TileVB[3].Set(v.x, v.y, v.z, color);

		return;
	}
}

void CSoundCell::Render(DWORD color)
{
	int x, z;
	for(x=m_Rect.left; x<=m_Rect.right; x++)
	{
		for(z=m_Rect.bottom; z<=m_Rect.top; z++)
		{
			MakeTileVB(x,z, color);
			s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, m_TileVB, sizeof(__VertexXyzColor));
		}
	}
}

void CSoundCell::Load(File& file)
{
	file.Read(&m_iVersion, sizeof(int));
	file.Read(&m_dwSoundGroupID, sizeof(DWORD));
	file.Read(&m_Rect, sizeof(RECT));
}

void CSoundCell::Save(File& file)
{
	file.Write(&m_iVersion, sizeof(int));
	file.Write(&m_dwSoundGroupID, sizeof(DWORD));
	file.Write(&m_Rect, sizeof(RECT));
}

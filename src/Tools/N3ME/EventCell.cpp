// EventCell.cpp: implementation of the CEventCell class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "n3me.h"
#include "EventCell.h"
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

CEventCell::CEventCell()
{
	m_ZoneID = 0;
	m_EventID = 1;
	m_EventType = 0;

	for(int i=0;i<MAX_CON;i++)
	{
		sprintf(m_Exe[i],"0");
		sprintf(m_Con[i],"0");
	}
	sprintf(m_Name,"none");

	m_pRefTerrain = nullptr;
}

CEventCell::CEventCell(CLyTerrain* pRefTerrain)
{
	m_ZoneID = pRefTerrain->m_iZoneID;
	m_EventID = 1;
	m_EventType = 0;

	for(int i=0;i<MAX_CON;i++)
	{
		sprintf(m_Exe[i],"0");
		sprintf(m_Con[i],"0");
	}

	sprintf(m_Name,"none");

	m_pRefTerrain = pRefTerrain;
}

CEventCell::~CEventCell()
{
}

void CEventCell::AddRect(__Vector3 v)
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

void CEventCell::InitRect(__Vector3 v)
{
	m_BasePoint.x = m_Rect.left = m_Rect.right = (int)v.x / TERRAIN_CELL_SIZE;
	m_BasePoint.y = m_Rect.top = m_Rect.bottom = (int)v.z / TERRAIN_CELL_SIZE;
}

void CEventCell::MakeTileVB(int x, int z, DWORD color)
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

void CEventCell::Render(DWORD color)
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

void CEventCell::Load(File& file)
{
	file.Read(&m_ZoneID, sizeof(short));
	file.Read(m_Name, 80);
	file.Read(&m_EventID, sizeof(short));
	file.Read(&m_EventType, sizeof(short));

	for (int i = 0; i < MAX_CON; i++)
	{
		file.Read(m_Con[i], 256);
		file.Read(m_Exe[i], 256);
	}

	file.Read(&m_Rect, sizeof(RECT));
}

void CEventCell::Save(File& file)
{
	file.Write(&m_ZoneID, sizeof(short));
	file.Write(m_Name, 80);
	file.Write(&m_EventID, sizeof(short));
	file.Write(&m_EventType, sizeof(short));

	for(int i=0;i<MAX_CON;i++)
	{
		file.Write(m_Con[i], 256);
		file.Write(m_Exe[i], 256);
	}

	file.Write(&m_Rect, sizeof(RECT));
}

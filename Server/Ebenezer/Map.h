// Map.h: interface for the CMap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_3DMAP_H__986E02B6_E5A3_43CF_B1D7_A7135551933D__INCLUDED_)
#define AFX_3DMAP_H__986E02B6_E5A3_43CF_B1D7_A7135551933D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3ShapeMgr.h>
#include "Region.h"
#include "GameEvent.h"
#include <shared/STLMap.h>

typedef CSTLMap <CGameEvent>		EventArray;
typedef CSTLMap <_OBJECT_EVENT>		ObjectEventArray;
typedef CSTLMap <_REGENE_EVENT>		ObjectRegeneArray;
typedef	CSTLMap <_WARP_INFO>		WarpArray;

class CUser;
class CEbenezerDlg;

class C3DMap
{
private:
	ObjectEventArray	m_ObjectEventArray;
	ObjectRegeneArray	m_ObjectRegeneArray;

	EventArray	m_EventArray;
	CN3ShapeMgr m_N3ShapeMgr;
//	int			m_nMapSize;		// Grid Unit ex) 4m
//	float		m_fUnitDist;	// i Grid Distance

	float** m_fHeight;

	int			m_nXRegion;
	int			m_nZRegion;

public:
//
	int			m_nMapSize;		// Grid Unit ex) 4m
	float		m_fUnitDist;	// i Grid Distance
//
	void LoadWarpList(HANDLE hFile);
	void LoadRegeneEvent(HANDLE hFile);
	bool IsValidPosition(float x, float z, float y) const;

	_OBJECT_EVENT* GetObjectEvent(int objectindex)
	{
		return m_ObjectEventArray.GetData(objectindex);
	}

	_REGENE_EVENT* GetRegeneEvent(int objectindex)
	{
		return m_ObjectRegeneArray.GetData(objectindex);
	}

	void LoadObjectEvent(HANDLE hFile);
	bool LoadEvent();
	bool CheckEvent(float x, float z, CUser* pUser = nullptr);
	void RegionNpcRemove(int rx, int rz, int nid);
	void RegionNpcAdd(int rx, int rz, int nid);
	void RegionUserRemove(int rx, int rz, int uid);
	void RegionUserAdd(int rx, int rz, int uid);
	bool RegionItemRemove(int rx, int rz, int bundle_index, int itemid, int count);
	bool RegionItemAdd(int rx, int rz, _ZONE_ITEM* pItem);
	bool ObjectCollision(float x1, float z1, float y1, float x2, float z2, float y2);
	float GetHeight(float x, float y, float z);
	void LoadMapTile(HANDLE hFile);
	bool LoadMap(HANDLE hFile);
	void LoadTerrain(HANDLE hFile);

	int GetXRegionMax() const
	{
		return m_nXRegion - 1;
	}

	int GetZRegionMax() const
	{
		return m_nZRegion - 1;
	}

	C3DMap();
	virtual ~C3DMap();

	int	m_nServerNo;
	int m_nZoneNumber;
	float m_fInitX;
	float m_fInitZ;
	float m_fInitY;
	BYTE	m_bType;		// Zone Type : 1 -> common zone,  2 -> battle zone, 3 -> 24 hour open battle zone
	short	m_sMaxUser;

	CRegion** m_ppRegion;
	short** m_ppnEvent;

	WarpArray			m_WarpArray;

	CEbenezerDlg* m_pMain;

	DWORD m_wBundle;	// Zone Item Max Count
};

#endif // !defined(AFX_3DMAP_H__986E02B6_E5A3_43CF_B1D7_A7135551933D__INCLUDED_)

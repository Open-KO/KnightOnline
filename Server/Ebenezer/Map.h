#ifndef SERVER_EBENEZER_MAP_H
#define SERVER_EBENEZER_MAP_H

#pragma once

#include "Region.h"
#include "GameEvent.h"

#include <shared-server/N3ShapeMgr.h>
#include <shared-server/STLMap.h>

#include <iosfwd>

typedef CSTLMap <CGameEvent>		EventArray;
typedef CSTLMap <_OBJECT_EVENT>		ObjectEventArray;
typedef CSTLMap <_REGENE_EVENT>		ObjectRegeneArray;
typedef	CSTLMap <_WARP_INFO>		WarpArray;

class CUser;
class EbenezerApp;

class C3DMap
{
private:
	ObjectEventArray	m_ObjectEventArray;
	ObjectRegeneArray	m_ObjectRegeneArray;

	EventArray	m_EventArray;
	CN3ShapeMgr m_N3ShapeMgr;

	float**		m_fHeight;

	int			m_nXRegion;
	int			m_nZRegion;

public:
	int			m_nMapSize;		// Grid Unit ex) 4m
	float		m_fUnitDist;	// i Grid Distance

	void LoadWarpList(std::istream& fs);
	void LoadRegeneEvent(std::istream& fs);
	bool IsValidPosition(float x, float z) const;

	_OBJECT_EVENT* GetObjectEvent(int objectindex)
	{
		return m_ObjectEventArray.GetData(objectindex);
	}

	_REGENE_EVENT* GetRegeneEvent(int objectindex)
	{
		return m_ObjectRegeneArray.GetData(objectindex);
	}

	void LoadObjectEvent(std::istream& fs);
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
	void LoadMapTile(std::istream& fs);
	bool LoadMap(std::istream& fs);
	void LoadTerrain(std::istream& fs);

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
	uint8_t	m_bType;		// Zone Type : 1 -> common zone,  2 -> battle zone, 3 -> 24 hour open battle zone
	int16_t	m_sMaxUser;

	CRegion** m_ppRegion;
	int16_t** m_ppnEvent;

	WarpArray	m_WarpArray;

	EbenezerApp* m_pMain;

	uint32_t m_wBundle;	// Zone Item Max Count
};

#endif // SERVER_EBENEZER_MAP_H

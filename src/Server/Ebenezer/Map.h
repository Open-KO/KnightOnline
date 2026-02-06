#ifndef SERVER_EBENEZER_MAP_H
#define SERVER_EBENEZER_MAP_H

#pragma once

#include "Region.h"
#include "GameEvent.h"

#include <shared-server/N3ShapeMgr.h>
#include <shared-server/STLMap.h>

#include <iosfwd>

namespace Ebenezer
{

using EventArray        = CSTLMap<CGameEvent>;
using ObjectEventArray  = CSTLMap<_OBJECT_EVENT>;
using ObjectRegeneArray = CSTLMap<_REGENE_EVENT>;
using WarpArray         = CSTLMap<_WARP_INFO>;

class CUser;
class EbenezerApp;
class C3DMap
{
protected:
	ObjectEventArray m_ObjectEventArray;
	ObjectRegeneArray m_ObjectRegeneArray;

	EventArray m_EventArray;
	CN3ShapeMgr m_N3ShapeMgr;

	float** m_fHeight = nullptr;

	int m_nXRegion    = -1;
	int m_nZRegion    = -1;

public:
	int m_nMapSize    = 0;    // Grid Unit ex) 4m
	float m_fUnitDist = 0.0f; // i Grid Distance

	void LoadWarpList(File& fs);
	void LoadRegeneEvent(File& fs);
	bool IsValidPosition(float x, float z) const;

	_OBJECT_EVENT* GetObjectEvent(int objectindex)
	{
		return m_ObjectEventArray.GetData(objectindex);
	}

	_REGENE_EVENT* GetRegeneEvent(int objectindex)
	{
		return m_ObjectRegeneArray.GetData(objectindex);
	}

	void LoadObjectEvent(File& fs);
	bool LoadEvent();
	bool CheckEvent(float x, float z, CUser* pUser = nullptr);
	void RegionNpcRemove(int rx, int rz, int nid);
	bool RegionNpcAdd(int rx, int rz, int nid);
	void RegionUserRemove(int rx, int rz, int uid);
	bool RegionUserAdd(int rx, int rz, int uid);
	bool RegionItemRemove(int rx, int rz, int bundle_index, int itemid, int count);
	bool RegionItemAdd(int rx, int rz, _ZONE_ITEM* pItem);
	bool ObjectCollision(float x1, float z1, float y1, float x2, float z2, float y2);
	float GetHeight(float x, float y, float z);
	void LoadMapTile(File& fs);
	bool LoadMap(File& fs);
	void LoadTerrain(File& fs);

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

	int m_nServerNo      = 0;
	int m_nZoneNumber    = 0;
	float m_fInitX       = 0.0f;
	float m_fInitZ       = 0.0f;
	float m_fInitY       = 0.0f;

	// Zone Type : 1 -> common zone,  2 -> battle zone, 3 -> 24 hour open battle zone
	uint8_t m_bType      = 0;

	int16_t m_sMaxUser   = 150; // Max user in Battlezone!!!

	CRegion** m_ppRegion = nullptr;
	int16_t** m_ppnEvent = nullptr;

	WarpArray m_WarpArray;

	EbenezerApp* m_pMain = nullptr;

	uint32_t m_wBundle   = 1; // Zone Item Max Count
};

} // namespace Ebenezer

#endif // SERVER_EBENEZER_MAP_H

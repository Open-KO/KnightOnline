#ifndef SERVER_AISERVER_MAP_H
#define SERVER_AISERVER_MAP_H

#pragma once

#include <shared-server/N3ShapeMgr.h>
#include <shared-server/STLMap.h>

#include <MathUtils/GeometricStructs.h>

#include <iosfwd>

class CRegion;
class CNpc;
class CUser;
class AIServerApp;
class CRoomEvent;
class File;

typedef CSTLMap <_OBJECT_EVENT>		ObjectEventArray;
typedef CSTLMap <CRoomEvent>		RoomEventArray;

class CMapInfo					// 각 좌표의 정보
{
public:
	int16_t	m_sEvent = 0;		// 현좌표의 이벤트 번호
};

class MAP
{
public:
	AIServerApp*		m_pMain;
	CN3ShapeMgr			m_N3ShapeMgr;
	CMapInfo**			m_pMap;				// 타일의 정보(1셀 : 4미터)
	CRegion**			m_ppRegion;			// 64미터의 타일정보..
	_SIZE				m_sizeMap;			// 맵의 크기
	_SIZE				m_sizeRegion;		// 맵의 resion size
	int					m_nZoneNumber;		// zone number
	int					m_nServerNo;
	int					m_nMapSize;			// Grid Unit ex) 4m
	float				m_fUnitDist;		// i Grid Distance
	float**				m_fHeight;
//	int16_t				m_arDungeonBossMonster[MAX_DUNGEON_BOSS_MONSTER];
	uint8_t				m_byRoomType;		// 방의 초기화관련( 0:자동으로 초기화, 1:전쟁이벤트 관련(특정조건이 완료시 초기화)
	uint8_t				m_byRoomEvent;		// event room(0:empty, 1:use)
	uint8_t				m_byRoomStatus;		// room status(1:진행중, 2:방을 초기화중, 3:방초기화 완료)
	uint8_t				m_byInitRoomCount;	// room 초기화 시간을 제어(몬스터와 동기화를 맞추기 위해)
	ObjectEventArray	m_ObjectEventArray;
	RoomEventArray		m_arRoomEventArray;
	int16_t				m_sKarusRoom;		// karus의 성갯수
	int16_t				m_sElmoradRoom;		// elmorad의 성갯수
	
	MAP();
	virtual ~MAP();

	bool LoadMap(File& fs);
	void LoadTerrain(File& fs);
	void LoadMapTile(File& fs);
	void LoadObjectEvent(File& fs);
	bool LoadRoomEvent(int zone_number);
	bool ObjectIntersect(float x1, float z1, float y1, float x2, float z2, float y2);
	float GetHeight(float x, float z);

	void RegionNpcRemove(int rx, int rz, int nid);
	void RegionNpcAdd(int rx, int rz, int nid);
	void RegionUserRemove(int rx, int rz, int uid);
	void RegionUserAdd(int rx, int rz, int uid);
	bool RegionItemRemove(int rx, int rz, int itemid, int count, int index);
	void RegionItemAdd(int rx, int rz, int itemid, int count, int index);
	int  GetRegionUserSize(int rx, int rz);
	int  GetRegionNpcSize(int rx, int rz);

	int GetXRegionMax() const {
		return m_sizeRegion.cx - 1;
	}

	int GetZRegionMax() const {
		return m_sizeRegion.cy - 1;
	}

	int IsRoomCheck(float fx, float fz);	// 던젼에서 사용, 유저의 현재위치가 던젼의 어느 위치에 있는지를 판단
	bool IsRoomStatusCheck();

	bool IsMovable(int dest_x, int dest_y) const;
	void InitializeRoom();

	CRoomEvent* SetRoomEvent(int number);

	/// \brief Checks if a position is valid for the map
	bool IsValidPosition(float x, float z) const;

protected:
	void RemoveMapData();
};

#endif // SERVER_AISERVER_MAP_H

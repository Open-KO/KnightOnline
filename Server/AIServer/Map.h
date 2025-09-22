// MAP.h: interface for the MAP class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAP_H__BCC14687_C38F_4597_8522_2170ED077037__INCLUDED_)
#define AFX_MAP_H__BCC14687_C38F_4597_8522_2170ED077037__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3ShapeMgr.h>
#include <shared/STLMap.h>
#include "RoomEvent.h"

typedef CSTLMap <_OBJECT_EVENT>		ObjectEventArray;
typedef CSTLMap <CRoomEvent>		RoomEventArray;

class CRegion;
class CNpc;
class CUser;
class CServerDlg;
//class CRoomEvent;

class CMapInfo					// 각 좌표의 정보
{
public:
	short	m_sEvent;			// 현좌표의 이벤트 번호

	CMapInfo();
	virtual ~CMapInfo();
};

class MAP
{
public:
	CServerDlg*			m_pMain;
	CN3ShapeMgr			m_N3ShapeMgr;
	CMapInfo**			m_pMap;				// 타일의 정보(1셀 : 4미터)
	CRegion**			m_ppRegion;			// 64미터의 타일정보..
	//CRoomEvent*		m_pRoomEvent;
	CSize				m_sizeMap;			// 맵의 크기
	CSize				m_sizeRegion;		// 맵의 resion size
	int					m_nZoneNumber;		// zone number
	int					m_nServerNo;
	int					m_nMapSize;			// Grid Unit ex) 4m
	float				m_fUnitDist;		// i Grid Distance
	float**				m_fHeight;
//	short				m_arDungeonBossMonster[MAX_DUNGEON_BOSS_MONSTER];
	BYTE				m_byRoomType;		// 방의 초기화관련( 0:자동으로 초기화, 1:전쟁이벤트 관련(특정조건이 완료시 초기화)
	BYTE				m_byRoomEvent;		// event room(0:empty, 1:use)
	BYTE				m_byRoomStatus;		// room status(1:진행중, 2:방을 초기화중, 3:방초기화 완료)
	BYTE				m_byInitRoomCount;	// room 초기화 시간을 제어(몬스터와 동기화를 맞추기 위해)
	ObjectEventArray	m_ObjectEventArray;
	RoomEventArray		m_arRoomEventArray;
	short				m_sKarusRoom;		// karus의 성갯수
	short				m_sElmoradRoom;		// elmorad의 성갯수
	
	MAP();
	virtual ~MAP();

	bool LoadMap(HANDLE hFile);
	void LoadTerrain(HANDLE hFile);
	void LoadMapTile(HANDLE hFile);
	void LoadObjectEvent(HANDLE hFile);
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

#endif // !defined(AFX_MAP_H__BCC14687_C38F_4597_8522_2170ED077037__INCLUDED_)


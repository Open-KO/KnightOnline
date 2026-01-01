#ifndef SERVER_AISERVER_ROOMEVENT_H
#define SERVER_AISERVER_ROOMEVENT_H

#pragma once

#include <shared-server/STLMap.h>

#define MAX_CHECK_EVENT	5

typedef CSTLMap <int>			mapNpcArray;

struct _RoomEvent
{
	int16_t	sNumber;			// 명령어, 조건문 번호
	int16_t	sOption_1;			// option 1 (몬스터의 번호를 주로 가지고 있음)
	int16_t	sOption_2;			// option 2 (몬스터의 마리수)
};

class CNpc;
class AIServerApp;

class CRoomEvent
{
public:
	int     m_iZoneNumber;		// zone number
	int16_t	m_sRoomNumber;		// room number (0:empty room)
	uint8_t	m_byStatus;			// room status (1:init, 2:progress, 3:clear)
	uint8_t	m_byCheck;			// 조건문의 갯수
	uint8_t	m_byRoomType;		// 방의 타입(0:일반, 1:함정방, 2:,,,,)

	int		m_iInitMinX;		// room region x
	int		m_iInitMinZ;
	int		m_iInitMaxX;
	int		m_iInitMaxZ;

	int		m_iEndMinX;			// room end region x 도착지점,,
	int		m_iEndMinZ;
	int		m_iEndMaxX;
	int		m_iEndMaxZ;

	_RoomEvent  m_Logic[MAX_CHECK_EVENT];		// 조건들
	_RoomEvent  m_Exec[MAX_CHECK_EVENT];		// 실행문

	double	m_fDelayTime;						// time

	mapNpcArray	m_mapRoomNpcArray;				// room npc uid array
	AIServerApp* m_pMain;

private:
	uint8_t    m_byLogicNumber;	// 현재의 조건문 검사 번호 (조건번호는 1부터 시작됨) (m_byCheck와 m_byLogicNumber이 같다면 클리어 상태)

public:
	CRoomEvent();
	virtual ~CRoomEvent();

	void MainRoom(double currentTime);
	void InitializeRoom();

private:
	void Initialize();
	bool CheckEvent(int event_num, double currentTime);
	bool RunEvent(int event_num);

	/// \brief checks if monster counts match given type
	/// \param sid npcId
	/// \param count number of monsters to check against
	/// \param type one of:
	/// 1: Count of monsters killed
	/// 2: Count of active monsters
	/// 3: All monsters dead (count unused)
	/// 4: Reset monster (?)
	bool CheckMonsterCount(int sid, int count, int type);

	CNpc* GetNpcPtr(int sid);
	void EndEventSay(int option1, int option2);
};

#endif // SERVER_AISERVER_ROOMEVENT_H

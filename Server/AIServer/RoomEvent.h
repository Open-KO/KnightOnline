// RoomEvent.h: interface for the CRoomEvent class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ROOMEVENT_H__70001CA8_64B8_422F_B0F4_ED0F2FA95E0E__INCLUDED_)
#define AFX_ROOMEVENT_H__70001CA8_64B8_422F_B0F4_ED0F2FA95E0E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
class CServerDlg;

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
	CServerDlg* m_pMain;

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
	bool CheckMonsterCount(int sid, int count, int type);
	CNpc* GetNpcPtr(int sid);
	void EndEventSay(int option1, int option2);

};

#endif // !defined(AFX_ROOMEVENT_H__70001CA8_64B8_422F_B0F4_ED0F2FA95E0E__INCLUDED_)

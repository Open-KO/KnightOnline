// GameSocket.h: interface for the CGameSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GAMESOCKET_H__E8EB97C6_3A78_44E1_BE1D_E5AC1893D27E__INCLUDED_)
#define AFX_GAMESOCKET_H__E8EB97C6_3A78_44E1_BE1D_E5AC1893D27E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IOCPSocket2.h"
#include "Party.h"

#define WM_GAMESERVER_LOGIN			(WM_USER)+100

class CServerDlg;
class CUser;
class MAP;
//class CParty;

class CGameSocket : public CIOCPSocket2
{
public:
	CServerDlg* m_pMain;
	CParty		m_Party;
	short m_sSocketID;
public:
	CGameSocket();
	virtual ~CGameSocket();

	void Initialize();
	void Parsing(int length, char* pData);	// recv data parsing
	void CloseProcess();
	void RecvServerConnect(char* pBuf);

	bool SetUid(float x, float z, int id, int speed);
	// GameServer에서 User정보 받는 부분
	void RecvUserInfo(char* pBuf);
	void RecvUserInOut(char* pBuf);
	void RecvUserMove(char* pBuf);
	void RecvUserMoveEdge(char* pBuf);
	void RecvUserLogOut(char* pBuf);
	void RecvUserRegene(char* pBuf);
	void RecvUserSetHP(char* pBuf);
	void RecvAttackReq(char* pBuf);
	void RecvUserUpdate(char* pBuf);
	void RecvZoneChange(char* pBuf);
	void RecvMagicAttackReq(char* pBuf);
	void RecvCompressedData(char* pBuf);
	void RecvUserInfoAllData(char* pBuf);
	void RecvPartyInfoAllData(char* pBuf);
	void RecvGateOpen(char* pBuf);
	void RecvCheckAlive(char* pBuf);
	void RecvHealMagic(char* pBuf);
	void RecvTimeAndWeather(char* pBuf);
	void RecvUserFail(char* pBuf);
	void Send_UserError(short uid, short tid = 10000);
	void RecvBattleEvent(char* pBuf);
};

#endif // !defined(AFX_GAMESOCKET_H__E8EB97C6_3A78_44E1_BE1D_E5AC1893D27E__INCLUDED_)

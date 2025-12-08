#ifndef SERVER_AISERVER_GAMESOCKET_H
#define SERVER_AISERVER_GAMESOCKET_H

#pragma once

#include "Party.h"

#include <shared-server/TcpServerSocket.h>

class AIServerApp;
class CUser;
class MAP;
class CIOCPort;
class CGameSocket : public TcpServerSocket
{
public:
	AIServerApp*	m_pMain;
	CParty			m_Party;
	int16_t			_zoneNo;

public:
	CGameSocket(SocketManager* socketManager);
	~CGameSocket() override;

	void Initialize() override;
	bool PullOutCore(char*& data, int& length) override;
	int Send(char* pBuf, int length) override;
	void CloseProcess() override;

	void Parsing(int length, char* pData) override;	// recv data parsing
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
	void RecvCheckAlive();
	void RecvHealMagic(char* pBuf);
	void RecvTimeAndWeather(char* pBuf);
	void RecvUserFail(char* pBuf);
	void Send_UserError(int16_t uid, int16_t tid = 10000);
	void RecvBattleEvent(char* pBuf);
};

#endif // SERVER_AISERVER_GAMESOCKET_H

#ifndef CLIENT_WARFARE_GAMEPROCLOGIN_1298_H
#define CLIENT_WARFARE_GAMEPROCLOGIN_1298_H

#pragma once

#if !defined(LOGIN_SCENE_VERSION) || LOGIN_SCENE_VERSION == 1298

#include "GameProcedure.h"

class CUILogIn_1298;
class CGameProcLogIn_1298 : public CGameProcedure
{
public:
	CUILogIn_1298* m_pUILogIn;

	bool m_bLogIn; // 로그인 중복 방지..
	std::string m_szRegistrationSite;

	float m_fTimeUntilNextGameConnectionAttempt;

public:
	inline void ResetGameConnectionAttemptTimer()
	{
		m_fTimeUntilNextGameConnectionAttempt = 0.0f;
	}

	void MsgRecv_GameServerGroupList(Packet& pkt);
	void MsgRecv_AccountLogIn(int iCmd, Packet& pkt);
	int MsgRecv_VersionCheck(Packet& pkt) override;
	int MsgRecv_GameServerLogIn(Packet& pkt) override; // 국가 번호를 리턴한다.
	void MsgRecv_News(Packet& pkt);

	bool MsgSend_AccountLogIn(enum e_LogInClassification eLIC);
	bool MsgSend_NewsReq();

	void Release() override;
	void Init() override;
	void Tick() override;
	void Render() override;

protected:
	bool ProcessPacket(Packet& pkt) override;

public:
	void ConnectToGameServer(); // 고른 게임 서버에 접속
	CGameProcLogIn_1298();
	~CGameProcLogIn_1298() override;
};

class CGameProcLogIn : public CGameProcLogIn_1298
{
};

#endif

#endif // CLIENT_WARFARE_GAMEPROCLOGIN_1298_H

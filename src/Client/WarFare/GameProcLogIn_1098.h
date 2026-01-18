#ifndef CLIENT_WARFARE_GAMEPROCLOGIN_1098_H
#define CLIENT_WARFARE_GAMEPROCLOGIN_1098_H

#pragma once

#if defined(LOGIN_SCENE_VERSION) && LOGIN_SCENE_VERSION == 1098

#include "GameProcedure.h"

class CGameProcLogIn_1098 : public CGameProcedure
{
public:
	class CN3Chr* m_pChr;
	class CN3Texture* m_pTexBkg;
	class CUILogIn_1098* m_pUILogIn;

	class CN3Camera* m_pCamera;
	class CN3Light* m_pLights[3];

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

	bool MsgSend_AccountLogIn(enum e_LogInClassification eLIC);

	void Release() override;
	void Init() override;
	void Tick() override;
	void Render() override;

protected:
	bool ProcessPacket(Packet& pkt) override;

public:
	void ConnectToGameServer(); // 고른 게임 서버에 접속
	CGameProcLogIn_1098();
	~CGameProcLogIn_1098() override;
};

class CGameProcLogIn : public CGameProcLogIn_1098
{
};

#endif

#endif // CLIENT_WARFARE_GAMEPROCLOGIN_1098_H

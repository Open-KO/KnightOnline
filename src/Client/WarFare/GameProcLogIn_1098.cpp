#include "StdAfx.h"

#if defined(LOGIN_SCENE_VERSION) && LOGIN_SCENE_VERSION == 1098
#include "GameProcLogIn_1098.h"
#include "GameEng.h"
#include "UILogin_1098.h"
#include "PlayerMySelf.h"
#include "UIManager.h"
#include "LocalInput.h"
#include "APISocket.h"
#include "PacketDef.h"
#include "text_resources.h"

#include <N3Base/N3Camera.h>
#include <N3Base/N3Light.h>
#include <N3Base/N3Chr.h>
#include <N3Base/N3SndObj.h>
#include <N3Base/N3SndMgr.h>

using __GameServerInfo = CUILogIn_1098::__GameServerInfo;

CGameProcLogIn_1098::CGameProcLogIn_1098()
{
	m_pUILogIn = nullptr;
	m_pChr     = nullptr;
	m_pTexBkg  = nullptr;

	m_pCamera  = nullptr;
	for (int i = 0; i < 3; i++)
		m_pLights[i] = nullptr;

	m_bLogIn                              = false; // 로그인 중복 방지..
	m_fTimeUntilNextGameConnectionAttempt = 0.0f;
}

CGameProcLogIn_1098::~CGameProcLogIn_1098()
{
	delete m_pUILogIn;
	delete m_pChr;
	delete m_pTexBkg;

	delete m_pCamera;
	for (int i = 0; i < 3; i++)
		delete m_pLights[i];
}

void CGameProcLogIn_1098::Release()
{
	CGameProcedure::Release();

	delete m_pUILogIn;
	m_pUILogIn = nullptr;

	delete m_pChr;
	m_pChr = nullptr;

	delete m_pTexBkg;
	m_pTexBkg = nullptr;

	delete m_pCamera;
	m_pCamera = nullptr;

	for (int i = 0; i < 3; i++)
	{
		delete m_pLights[i];
		m_pLights[i] = nullptr;
	}
}

void CGameProcLogIn_1098::Init()
{
	CGameProcedure::Init();

	m_pTexBkg = new CN3Texture();
	m_pTexBkg->LoadFromFile("Intro\\Moon.dxt");

	m_pChr = new CN3Chr();
	m_pChr->LoadFromFile("Intro\\Intro.N3Chr");
	m_pChr->AniCurSet(0); // 루핑 에니메이션..

	m_pCamera = new CN3Camera();
	m_pCamera->EyePosSet(0.22f, 0.91f, -1.63f);
	m_pCamera->AtPosSet(-0.19f, 1.1f, 0.09f);
	m_pCamera->m_Data.fNP = 0.1f;
	m_pCamera->m_Data.fFP = 32.0f;
	m_pCamera->m_bFogUse  = false;

	for (int i = 0; i < 3; i++)
		m_pLights[i] = new CN3Light();

	m_pLights[0]->LoadFromFile("Intro\\0.N3Light");
	m_pLights[1]->LoadFromFile("Intro\\1.N3Light");
	m_pLights[2]->LoadFromFile("Intro\\2.N3Light");

	s_pEng->s_SndMgr.ReleaseStreamObj(&s_pSnd_BGM);
	s_pSnd_BGM = s_pEng->s_SndMgr.CreateStreamObj(35); //몬스터 울부짖는 26초짜리 소리..

	m_pUILogIn = new CUILogIn_1098();
	m_pUILogIn->Init(s_pUIMgr);

	__TABLE_UI_RESRC* pTbl = s_pTbl_UI.GetIndexedData(0); // 국가 기준이 없기 때문이다...
	if (pTbl != nullptr)
		m_pUILogIn->LoadFromFile(pTbl->szLogIn);

	RECT rc = m_pUILogIn->GetRegion();
	int iX  = (CN3Base::s_CameraData.vp.Width - (rc.right - rc.left)) / 2;
	int iY  = CN3Base::s_CameraData.vp.Height - (rc.bottom - rc.top);
	m_pUILogIn->SetPos(iX, iY);
	m_pUILogIn->RecalcGradePos();
	rc.left   = 0;
	rc.top    = 0;
	rc.right  = CN3Base::s_CameraData.vp.Width;
	rc.bottom = CN3Base::s_CameraData.vp.Height;
	m_pUILogIn->SetRegion(rc); // 이걸 꼭 해줘야 UI 처리가 제대로 된다..
	s_pUIMgr->SetFocusedUI((CN3UIBase*) m_pUILogIn);

	// 소켓 접속..
	char szIniPath[_MAX_PATH] {};
	lstrcpy(szIniPath, CN3Base::PathGet().c_str());
	lstrcat(szIniPath, "Server.Ini");

	char szRegistrationSite[_MAX_PATH] {};
	GetPrivateProfileString("Join", "Registration site", "", szRegistrationSite, _MAX_PATH, szIniPath);
	m_szRegistrationSite = szRegistrationSite;

	int iServerCount     = GetPrivateProfileInt("Server", "Count", 0, szIniPath);

	char szIPs[256][32] {};
	for (int i = 0; i < iServerCount; i++)
	{
		std::string key = fmt::format("IP{}", i);
		GetPrivateProfileString("Server", key.c_str(), "", szIPs[i], 32, szIniPath);
	}

	int iServer = -1;
	if (iServerCount > 0)
		iServer = rand() % iServerCount;

	if (iServer >= 0 && lstrlen(szIPs[iServer]) > 0)
	{
		const char* ip                = szIPs[iServer];
		int port                      = SOCKET_PORT_LOGIN;

		s_bNeedReportConnectionClosed = false; // Should I report that the server connection was lost?
		int iErr                      = s_pSocket->Connect(s_hWndBase, ip, port);
		s_bNeedReportConnectionClosed = true;

		if (iErr != 0)
		{
#if defined(_DEBUG)
			std::string errorMessage = fmt::format("{}:{} (errorCode: {})\n"
												   "From config: Server.ini (client)",
				ip, port, iErr);
			MessageBoxPost(errorMessage, "Failed to connect to login server", MB_OK, BEHAVIOR_EXIT);
#else
			ReportServerConnectionFailed("LogIn Server", iErr, true);
#endif
		}
		else
		{
			m_pUILogIn->FocusToID(); // 아이디 입력창에 포커스를 맞추고..

			// 게임 서버 리스트 요청..
			int iOffset = 0;
			uint8_t byBuffs[4];
			CAPISocket::MP_AddByte(byBuffs, iOffset, LS_SERVERLIST); // 커멘드.
			s_pSocket->Send(byBuffs, iOffset);                       // 보낸다
		}
	}
	else
	{
		MessageBoxPost("No server list", "LogIn Server fail", MB_OK, BEHAVIOR_EXIT); // 끝낸다.
	}

	// 게임 계정으로 들어 왔으면..
	if (LIC_KNIGHTONLINE != s_eLogInClassification)
	{
		MsgSend_AccountLogIn(s_eLogInClassification); // 로그인..
	}

	// Re-entered the scene; we can reset any existing timer.
	// The point of this delay is to prevent the user from intentionally or otherwise spamming connections
	// to the game server, in the small window where we're still on the login scene and are waiting for the
	// game server to respond.
	// Once we've changed scenes, this timer doesn't matter anymore; we can't continue to spam it.
	// Returning back to this scene, then, means we're fine to have it reset.
	ResetGameConnectionAttemptTimer();
}

void CGameProcLogIn_1098::Tick() // 프로시져 인덱스를 리턴한다. 0 이면 그대로 진행
{
	CGameProcedure::Tick();      // 키, 마우스 입력 등등..

	if (m_fTimeUntilNextGameConnectionAttempt > 0.0f)
	{
		m_fTimeUntilNextGameConnectionAttempt -= s_fSecPerFrm;
		if (m_fTimeUntilNextGameConnectionAttempt < 0.0f)
			m_fTimeUntilNextGameConnectionAttempt = 0.0f;
	}

	for (int i = 0; i < 3; i++)
		m_pLights[i]->Tick();

	m_pChr->Tick();

	static float fTmp = 0;
	if (fTmp == 0)
	{
		if (s_pSnd_BGM != nullptr)
			s_pSnd_BGM->Play(); // 음악 시작..
	}

	fTmp += CN3Base::s_fSecPerFrm;
	if (fTmp > 21.66f)
	{
		fTmp = 0;
		if (s_pSnd_BGM != nullptr)
			s_pSnd_BGM->Stop();
	}
}

void CGameProcLogIn_1098::Render()
{
	D3DCOLOR crEnv = 0x00000000;
	s_pEng->Clear(crEnv);     // 배경은 검은색
	s_lpD3DDev->BeginScene(); // 씬 렌더 ㅅ작...

							  // 카메라 잡기..
	m_pCamera->Tick();
	m_pCamera->Apply();

	for (int i = 0; i < 8; i++)
		s_lpD3DDev->LightEnable(i, FALSE);

	for (int i = 0; i < 3; i++)
		m_pLights[i]->Apply();

	////////////////////////////////////////////
	// 달그리기..
	D3DVIEWPORT9 vp;
	s_lpD3DDev->GetViewport(&vp);

	float fMW  = (m_pTexBkg->Width() * vp.Width / 1024.0f) * 1.3f;
	float fMH  = (m_pTexBkg->Height() * vp.Height / 768.0f) * 1.3f;
	float fX   = 100.0f * vp.Width / 1024.0f;
	float fY   = 50.0f * vp.Height / 768.0f;

	float fRHW = 1.0f;
	__VertexTransformed vMoon[4];
	vMoon[0].Set(fX, fY, 0, fRHW, 0xffffffff, 0.0f, 0.0f);
	vMoon[1].Set(fX + fMW, fY, 0, fRHW, 0xffffffff, 1.0f, 0.0f);
	vMoon[2].Set(fX + fMW, fY + fMH, 0, fRHW, 0xffffffff, 1.0f, 1.0f);
	vMoon[3].Set(fX, fY + fMH, 0, fRHW, 0xffffffff, 0.0f, 1.0f);

	DWORD dwZWrite;
	s_lpD3DDev->GetRenderState(D3DRS_ZWRITEENABLE, &dwZWrite);
	s_lpD3DDev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	s_lpD3DDev->SetTexture(0, m_pTexBkg->Get());
	s_lpD3DDev->SetFVF(FVF_TRANSFORMED);
	s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vMoon, sizeof(__VertexTransformed));

	s_lpD3DDev->SetRenderState(D3DRS_ZWRITEENABLE, dwZWrite);
	// 달그리기..
	////////////////////////////////////////////

	m_pChr->Render();               // 캐릭터 그리기...

	CGameProcedure::Render();       // UI 나 그밖의 기본적인 것들 렌더링..

	s_pEng->s_lpD3DDev->EndScene(); // 씬 렌더 시작...
	s_pEng->Present(CN3Base::s_hWndBase);
}

bool CGameProcLogIn_1098::MsgSend_AccountLogIn(e_LogInClassification eLIC)
{
	if (LIC_KNIGHTONLINE == eLIC)
	{
		m_pUILogIn->AccountIDGet(s_szAccount);  // 계정 기억..
		m_pUILogIn->AccountPWGet(s_szPassWord); // 비밀번호 기억..
	}

	if (s_szAccount.empty() || s_szPassWord.empty() || s_szAccount.size() >= 20 || s_szPassWord.size() >= 12)
		return false;

	m_pUILogIn->SetVisibleLogInUIs(false); // 패킷이 들어올때까지 UI 를 Disable 시킨다...
	m_pUILogIn->SetRequestedLogIn(true);
	m_bLogIn = true;                       // 로그인 시도..

	uint8_t byBuff[256];                   // 패킷 버퍼..
	int iOffset   = 0;                     // 버퍼의 오프셋..

	uint8_t byCmd = LS_LOGIN_REQ;
	if (eLIC == LIC_MGAME)
		byCmd = LS_MGAME_LOGIN;

	CAPISocket::MP_AddByte(byBuff, iOffset, byCmd);                          // 커멘드.
	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t) s_szAccount.size());  // 아이디 길이..
	CAPISocket::MP_AddString(byBuff, iOffset, s_szAccount);                  // 실제 아이디..
	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t) s_szPassWord.size()); // 패스워드 길이
	CAPISocket::MP_AddString(byBuff, iOffset, s_szPassWord);                 // 실제 패스워드

	s_pSocket->Send(byBuff, iOffset);                                        // 보낸다

	return true;
}

void CGameProcLogIn_1098::MsgRecv_GameServerGroupList(Packet& pkt)
{
	int iServerCount = pkt.read<uint8_t>(); // 서버 갯수
	for (int i = 0; i < iServerCount; i++)
	{
		int iLen = 0;
		__GameServerInfo GSI;
		iLen = pkt.read<int16_t>();
		pkt.readString(GSI.szIP, iLen);
		iLen = pkt.read<int16_t>();
		pkt.readString(GSI.szName, iLen);
		GSI.iConcurrentUserCount = pkt.read<int16_t>(); // 현재 동시 접속자수..

		m_pUILogIn->ServerInfoAdd(GSI);                 // ServerList
	}

	m_pUILogIn->ServerInfoUpdate();
}

void CGameProcLogIn_1098::MsgRecv_AccountLogIn(int iCmd, Packet& pkt)
{
	int iResult = pkt.read<uint8_t>(); // Recv - b1(0:실패 1:성공 2:ID없음 3:PW틀림 4:서버점검중)
	if (1 == iResult)                  // 접속 성공..
	{
		// 모든 메시지 박스 닫기..
		MessageBoxClose(-1);
		m_pUILogIn->OpenServerList(); // 서버 리스트 읽기..
	}
	else if (2 == iResult)            // ID 가 없어서 실패한거면..
	{
		if (iCmd == LS_LOGIN_REQ)
		{
			std::string szMsg = fmt::format_text_resource(IDS_NOACCOUNT_RETRY_MGAMEID);
			std::string szTmp = fmt::format_text_resource(IDS_CONNECT_FAIL);

			MessageBoxPost(szMsg, szTmp, MB_YESNO, BEHAVIOR_MGAME_LOGIN); // MGame ID 로 접속할거냐고 물어본다.
		}
		else
		{
			std::string szMsg = fmt::format_text_resource(IDS_NO_MGAME_ACCOUNT);
			std::string szTmp = fmt::format_text_resource(IDS_CONNECT_FAIL);

			MessageBoxPost(szMsg, szTmp, MB_OK); // MGame ID 로 접속할거냐고 물어본다.
		}
	}
	else if (3 == iResult)                       // PassWord 실패
	{
		std::string szMsg = fmt::format_text_resource(IDS_WRONG_PASSWORD);
		std::string szTmp = fmt::format_text_resource(IDS_CONNECT_FAIL);
		MessageBoxPost(szMsg, szTmp, MB_OK); // MGame ID 로 접속할거냐고 물어본다.
	}
	else if (4 == iResult)                   // 서버 점검 중??
	{
		std::string szMsg = fmt::format_text_resource(IDS_SERVER_CONNECT_FAIL);
		std::string szTmp = fmt::format_text_resource(IDS_CONNECT_FAIL);
		MessageBoxPost(szMsg, szTmp, MB_OK); // MGame ID 로 접속할거냐고 물어본다.
	}
	else if (5 == iResult)                   // 어떤 넘이 접속해 있다. 서버에게 끊어버리라고 하자..
	{
		int iLen = pkt.read<int16_t>();
		if (iLen > 0)
		{
			std::string szIP;
			pkt.readString(szIP, iLen);
			uint32_t dwPort = pkt.read<int16_t>();

			CAPISocket socketTmp;
			s_bNeedReportConnectionClosed = false; // 서버접속이 끊어진걸 보고해야 하는지..
			if (0 == socketTmp.Connect(s_hWndBase, szIP.c_str(), dwPort))
			{
				// 로그인 서버에서 받은 겜서버 주소로 접속해서 짤르라고 꼰지른다.
				int iOffset2 = 0;
				uint8_t Buff[32];
				CAPISocket::MP_AddByte(Buff, iOffset2, WIZ_KICKOUT);   // Recv s1, str1(IP) s1(port) | Send s1, str1(ID)
				CAPISocket::MP_AddShort(Buff, iOffset2, (int16_t) s_szAccount.size());
				CAPISocket::MP_AddString(Buff, iOffset2, s_szAccount); // Recv s1, str1(IP) s1(port) | Send s1, str1(ID)

				socketTmp.Send(Buff, iOffset2);
				socketTmp.Disconnect();                                // 짜른다..
			}
			s_bNeedReportConnectionClosed = true;                      // 서버접속이 끊어진걸 보고해야 하는지..

			std::string szMsg             = fmt::format_text_resource(IDS_LOGIN_ERR_ALREADY_CONNECTED_ACCOUNT);
			std::string szTmp             = fmt::format_text_resource(IDS_CONNECT_FAIL);
			MessageBoxPost(szMsg, szTmp, MB_OK); // 다시 접속 할거냐고 물어본다.
		}
	}
	else
	{
		std::string szMsg = fmt::format_text_resource(IDS_CURRENT_SERVER_ERROR);
		std::string szTmp = fmt::format_text_resource(IDS_CONNECT_FAIL);
		MessageBoxPost(szMsg, szTmp, MB_OK);  // MGame ID 로 접속할거냐고 물어본다.
	}

	if (1 != iResult)                         // 로그인 실패..
	{
		m_pUILogIn->SetVisibleLogInUIs(true); // 접속 성공..UI 조작 불가능..
		m_pUILogIn->SetRequestedLogIn(false);
		m_bLogIn = false;                     // 로그인 시도..
	}
}

int CGameProcLogIn_1098::MsgRecv_VersionCheck(Packet& pkt) // virtual
{
	int iVersion = CGameProcedure::MsgRecv_VersionCheck(pkt);
	if (iVersion == CURRENT_VERSION)
	{
		CGameProcedure::MsgSend_GameServerLogIn(); // 게임 서버에 로그인..
		m_pUILogIn->ConnectButtonSetEnable(false);
	}

	return iVersion;
}

int CGameProcLogIn_1098::MsgRecv_GameServerLogIn(Packet& pkt)   // virtual - 국가번호를 리턴한다.
{
	int iNation = CGameProcedure::MsgRecv_GameServerLogIn(pkt); // 국가 - 0 없음 0xff - 실패..

	if (0xff == iNation)
	{
		__GameServerInfo GSI;
		m_pUILogIn->ServerInfoGetCur(GSI);

		std::string szMsg = fmt::format_text_resource(IDS_FMT_GAME_SERVER_LOGIN_ERROR, GSI.szName, iNation);
		MessageBoxPost(szMsg, "", MB_OK);
		m_pUILogIn->ConnectButtonSetEnable(true); // 실패
	}
	else
	{
		if (0 == iNation)
			s_pPlayer->m_InfoBase.eNation = NATION_NOTSELECTED;
		else if (1 == iNation)
			s_pPlayer->m_InfoBase.eNation = NATION_KARUS;
		else if (2 == iNation)
			s_pPlayer->m_InfoBase.eNation = NATION_ELMORAD;
	}

	if (NATION_NOTSELECTED == s_pPlayer->m_InfoBase.eNation)
	{
		s_SndMgr.ReleaseStreamObj(&s_pSnd_BGM);

		s_pSnd_BGM = s_pEng->s_SndMgr.CreateStreamObj(ID_SOUND_BGM_EL_BATTLE);
		if (s_pSnd_BGM != nullptr)
		{
			s_pSnd_BGM->Looping(true);
			s_pSnd_BGM->Play();
		}

		ProcActiveSet((CGameProcedure*) s_pProcNationSelect);
	}
	else if (NATION_KARUS == s_pPlayer->m_InfoBase.eNation || NATION_ELMORAD == s_pPlayer->m_InfoBase.eNation)
	{
		s_SndMgr.ReleaseStreamObj(&s_pSnd_BGM);

		s_pSnd_BGM = s_SndMgr.CreateStreamObj(ID_SOUND_BGM_EL_BATTLE);
		if (s_pSnd_BGM != nullptr)
		{
			s_pSnd_BGM->Looping(true);
			s_pSnd_BGM->Play();
		}

		ProcActiveSet((CGameProcedure*) s_pProcCharacterSelect);
	}

	return iNation;
}

bool CGameProcLogIn_1098::ProcessPacket(Packet& pkt)
{
	size_t rpos = pkt.rpos();
	if (CGameProcedure::ProcessPacket(pkt))
		return true;

	pkt.rpos(rpos);

	s_pPlayer->m_InfoBase.eNation = NATION_UNKNOWN;
	int iCmd                      = pkt.read<uint8_t>(); // 커멘드 파싱..
	s_pPlayer->m_InfoBase.eNation = NATION_UNKNOWN;
	switch (iCmd)                                        // 커멘드에 다라서 분기..
	{
		case LS_SERVERLIST:                              // 접속하면 바로 보내준다..
			MsgRecv_GameServerGroupList(pkt);
			return true;

		case LS_LOGIN_REQ:   // 계정 접속 성공..
		case LS_MGAME_LOGIN: // MGame 계정 접속 성공..
			MsgRecv_AccountLogIn(iCmd, pkt);
			return true;

		case LS_NEWS:
			// act as if it's handled
			return true;
	}

	return false;
}

void CGameProcLogIn_1098::ConnectToGameServer() // 고른 게임 서버에 접속
{
	if (m_fTimeUntilNextGameConnectionAttempt > 0.0f)
		return;

	__GameServerInfo GSI;
	if (!m_pUILogIn->ServerInfoGetCur(GSI))
		return; // 서버를 고른다음..

	const char* ip                = GSI.szIP.c_str();
	int port                      = SOCKET_PORT_GAME;

	s_bNeedReportConnectionClosed = false;                                    // 서버접속이 끊어진걸 보고해야 하는지..
	int iErr                      = s_pSocket->Connect(s_hWndBase, ip, port); // 게임서버 소켓 연결
	s_bNeedReportConnectionClosed = true;                                     // 서버접속이 끊어진걸 보고해야 하는지..

	if (iErr != 0)
	{
#if defined(_DEBUG)
		std::string errorMessage = fmt::format("{}:{} (errorCode: {})\n"
											   "From config: Version.ini (server)",
			ip, port, iErr);
		MessageBoxPost(errorMessage, "Failed to connect to game server", MB_OK);
#else
		ReportServerConnectionFailed(GSI.szName, iErr, false);
#endif

		m_pUILogIn->ConnectButtonSetEnable(true);
	}
	else
	{
		s_szServer                            = GSI.szName;
		m_fTimeUntilNextGameConnectionAttempt = TIME_UNTIL_NEXT_GAME_CONNECTION_ATTEMPT;

		MsgSend_VersionCheck();
	}
}
#endif

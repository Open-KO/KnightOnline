#include "StdAfx.h"

#if !defined(LOGIN_SCENE_VERSION) || LOGIN_SCENE_VERSION == 1298
#include "GameProcLogIn_1298.h"
#include "GameEng.h"
#include "UILogin_1298.h"
#include "PlayerMySelf.h"
#include "UIManager.h"
#include "LocalInput.h"
#include "APISocket.h"
#include "PacketDef.h"
#include "text_resources.h"

#include <N3Base/N3SndObj.h>
#include <N3Base/N3SndMgr.h>

#include <ctime>

using __GameServerInfo = CUILogIn_1298::__GameServerInfo;

CGameProcLogIn_1298::CGameProcLogIn_1298()
{
	m_pUILogIn                            = nullptr;
	m_bLogIn                              = false; // 로그인 중복 방지..
	m_fTimeUntilNextGameConnectionAttempt = 0.0f;
}

CGameProcLogIn_1298::~CGameProcLogIn_1298()
{
	delete m_pUILogIn;
}

void CGameProcLogIn_1298::Release()
{
	CGameProcedure::Release();

	delete m_pUILogIn;
	m_pUILogIn = nullptr;
}

void CGameProcLogIn_1298::Init()
{
	CGameProcedure::Init();

	srand((uint32_t) time(nullptr));

	// Random elmorad or karus background
	int iRandomNation = 1 + (rand() % 2);

	m_pUILogIn        = new CUILogIn_1298();
	m_pUILogIn->Init(s_pUIMgr);

	__TABLE_UI_RESRC* pTbl = s_pTbl_UI.Find(iRandomNation);
	if (pTbl != nullptr)
		m_pUILogIn->LoadFromFile(pTbl->szLoginIntro);

	m_pUILogIn->SetPosCenter();

	RECT rc;
	rc.left   = 0;
	rc.top    = 0;
	rc.right  = static_cast<int>(s_CameraData.vp.Width);
	rc.bottom = static_cast<int>(s_CameraData.vp.Height);
	m_pUILogIn->SetRegion(rc);
	m_pUILogIn->PositionGroups();

	s_SndMgr.ReleaseStreamObj(&s_pSnd_BGM);

	std::string szFN = "Snd\\Intro_Sound.mp3";
	s_pSnd_BGM       = s_SndMgr.CreateStreamObj(szFN);

	if (s_pSnd_BGM != nullptr)
	{
		s_pSnd_BGM->Looping(true);
		s_pSnd_BGM->Play();
	}

	s_pUIMgr->SetFocusedUI(m_pUILogIn);

	// Socket connection..
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
			m_pUILogIn->FocusToID(); // Focus on the ID input box..

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

void CGameProcLogIn_1298::Tick()
{
	CGameProcedure::Tick();

	if (m_fTimeUntilNextGameConnectionAttempt > 0.0f)
	{
		m_fTimeUntilNextGameConnectionAttempt -= s_fSecPerFrm;
		if (m_fTimeUntilNextGameConnectionAttempt < 0.0f)
			m_fTimeUntilNextGameConnectionAttempt = 0.0f;
	}
}

void CGameProcLogIn_1298::Render()
{
	D3DCOLOR crEnv = 0x00000000;
	s_pEng->Clear(crEnv);     // background color black
	s_lpD3DDev->BeginScene(); // scene render start

	D3DVIEWPORT9 vp;
	s_lpD3DDev->GetViewport(&vp);

	DWORD dwZWrite = 0;
	s_lpD3DDev->GetRenderState(D3DRS_ZWRITEENABLE, &dwZWrite);
	s_lpD3DDev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	s_lpD3DDev->SetFVF(FVF_TRANSFORMED);
	s_lpD3DDev->SetRenderState(D3DRS_ZWRITEENABLE, dwZWrite);
	CGameProcedure::Render(); // Render UI and other basic elements.

	s_lpD3DDev->EndScene();   // Starting scene rendering...

	s_pEng->Present(CN3Base::s_hWndBase);
}

bool CGameProcLogIn_1298::MsgSend_AccountLogIn(e_LogInClassification eLIC)
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

bool CGameProcLogIn_1298::MsgSend_NewsReq()
{
	uint8_t byBuff[2];
	int iOffset = 0;

	CAPISocket::MP_AddByte(byBuff, iOffset, LS_NEWS);
	s_pSocket->Send(byBuff, iOffset);

	return true;
}

void CGameProcLogIn_1298::MsgRecv_News(Packet& pkt)
{
	uint16_t strLen = pkt.read<uint16_t>();
	std::string strLabel;
	pkt.readString(strLabel, strLen);

	if (strLabel != "Login Notice")
		return;

	// check limits
	uint16_t wSize = pkt.read<uint16_t>();
	if (wSize == 0 || wSize >= 4096)
		return;

	// read content
	std::string strContent;
	pkt.readString(strContent, wSize);

	m_pUILogIn->AddNews(strContent);
}

void CGameProcLogIn_1298::MsgRecv_GameServerGroupList(Packet& pkt)
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

void CGameProcLogIn_1298::MsgRecv_AccountLogIn(int iCmd, Packet& pkt)
{
	// Recv - b1 (0: Failure, 1: Success, 2: ID Not Found, 3: Incorrect Password,
	// 4: Server Under Maintenance)
	int iResult = pkt.read<uint8_t>();

	// Connection successful
	if (1 == iResult)
	{
		// request news from server
		MsgSend_NewsReq();

		// Close all message boxes..
		MessageBoxClose(-1);

		m_pUILogIn->OpenNews();
	}
	// ID not found
	else if (2 == iResult)
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

			auto socketTmp  = std::make_unique<CAPISocket>();
			if (socketTmp != nullptr)
			{
				s_bNeedReportConnectionClosed = false; // 서버접속이 끊어진걸 보고해야 하는지..
				if (0 == socketTmp->Connect(s_hWndBase, szIP, dwPort))
				{
					// 로그인 서버에서 받은 겜서버 주소로 접속해서 짤르라고 꼰지른다.
					int iOffset2 = 0;
					uint8_t Buff[32];
					CAPISocket::MP_AddByte(Buff, iOffset2, WIZ_KICKOUT);   // Recv s1, str1(IP) s1(port) | Send s1, str1(ID)
					CAPISocket::MP_AddShort(Buff, iOffset2, (int16_t) s_szAccount.size());
					CAPISocket::MP_AddString(Buff, iOffset2, s_szAccount); // Recv s1, str1(IP) s1(port) | Send s1, str1(ID)

					socketTmp->Send(Buff, iOffset2);
					socketTmp->Disconnect();                               // 짜른다..
				}
				s_bNeedReportConnectionClosed = true;                      // 서버접속이 끊어진걸 보고해야 하는지..
			}

			std::string szMsg = fmt::format_text_resource(IDS_LOGIN_ERR_ALREADY_CONNECTED_ACCOUNT);
			std::string szTmp = fmt::format_text_resource(IDS_CONNECT_FAIL);
			MessageBoxPost(szMsg, szTmp, MB_OK); // 다시 접속 할거냐고 물어본다.
		}
	}
	else
	{
		std::string szMsg = fmt::format_text_resource(IDS_CURRENT_SERVER_ERROR);
		std::string szTmp = fmt::format_text_resource(IDS_CONNECT_FAIL);
		MessageBoxPost(szMsg, szTmp, MB_OK); // MGame ID 로 접속할거냐고 물어본다.
	}

	// 로그인 실패..
	if (1 != iResult)
	{
		m_pUILogIn->SetVisibleLogInUIs(true); // 접속 성공..UI 조작 불가능..
		m_pUILogIn->SetRequestedLogIn(false);
		m_bLogIn = false;                     // 로그인 시도..
	}
}

int CGameProcLogIn_1298::MsgRecv_VersionCheck(Packet& pkt) // virtual
{
	int iVersion = CGameProcedure::MsgRecv_VersionCheck(pkt);
	if (iVersion == CURRENT_VERSION)
	{
		CGameProcedure::MsgSend_GameServerLogIn(); // 게임 서버에 로그인..
		m_pUILogIn->ConnectButtonSetEnable(false);
	}

	return iVersion;
}

int CGameProcLogIn_1298::MsgRecv_GameServerLogIn(Packet& pkt)   // virtual - 국가번호를 리턴한다.
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
		ProcActiveSet((CGameProcedure*) s_pProcNationSelect);
	}
	else if (NATION_KARUS == s_pPlayer->m_InfoBase.eNation || NATION_ELMORAD == s_pPlayer->m_InfoBase.eNation)
	{
		ProcActiveSet((CGameProcedure*) s_pProcCharacterSelect);
	}

	return iNation;
}

bool CGameProcLogIn_1298::ProcessPacket(Packet& pkt)
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
			MsgRecv_News(pkt);
			return true;

		default:
			break;
	}

	return false;
}

void CGameProcLogIn_1298::ConnectToGameServer() // 고른 게임 서버에 접속
{
	if (m_fTimeUntilNextGameConnectionAttempt > 0.0f)
		return;

	__GameServerInfo GSI;
	if (!m_pUILogIn->ServerInfoGetCur(GSI))
		return; // 서버를 고른다음..

	int port                      = SOCKET_PORT_GAME;

	s_bNeedReportConnectionClosed = false;                                          // 서버접속이 끊어진걸 보고해야 하는지..
	int iErr                      = s_pSocket->Connect(s_hWndBase, GSI.szIP, port); // 게임서버 소켓 연결
	s_bNeedReportConnectionClosed = true;                                           // 서버접속이 끊어진걸 보고해야 하는지..

	if (iErr != 0)
	{
#if defined(_DEBUG)
		std::string errorMessage = fmt::format("{}:{} (errorCode: {})\n"
											   "From config: Version.ini (server)",
			GSI.szIP, port, iErr);
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

// GameProcedure.h: interface for the CGameProcedure class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GameProcedure_H__0BEC53F2_1282_402C_9A28_FB98CC131F64__INCLUDED_)
#define AFX_GameProcedure_H__0BEC53F2_1282_402C_9A28_FB98CC131F64__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum e_LogInClassification { LIC_KNIGHTONLINE = 0, LIC_MGAME = 1, LIC_DAUM = 2 };

#include "GameDef.h"
#include "GameBase.h"

#include "N3WorldManager.h"

#define		UI_POST_WND_CHAT		"Wnd_Chat"
#define		UI_POST_WND_HOTKEY		"Wnd_Hotkey"
#define		UI_POST_WND_PARTY		"Wnd_Party"
#define		UI_POST_WND_HELP		"Wnd_Help"
#define		UI_POST_WND_INFO		"Wnd_Msg"

const int MAX_MSG_BOX = 4;

struct __WndInfo
{
	char			szName[16];
	bool			bVisible;
	POINT			ptPosition;

	__WndInfo()	{ memset(this, 0, sizeof(__WndInfo));	}
};

class CHotkeyData 
{
public:
	int	row;
	int column;
	int iID;

	CHotkeyData() {	memset(this, 0, sizeof(CHotkeyData)); }
	CHotkeyData(int rw, int cl, int id) 
	{
		CHotkeyData();
		row = rw;	column = cl;	iID = id;
	}
};

class CGameProcedure : public CGameBase
{
public:
	static class CN3SndObjStream*	s_pSnd_BGM;			// 메인 배경음악 포인터..
	static class CLocalInput*		s_pLocalInput;		// 마우스와 키보드 입력 객체 .. Direct Input 을 썼다.
	static class CAPISocket*		s_pSocket;			// 메인 소켓 객체
	static class CAPISocket*		s_pSocketSub;		// 서브 소켓 객체..
	static class CGameEng*			s_pEng;				// 3D Wrapper Engine
	static class CN3FXMgr*			s_pFX;
	
	static class CUIManager*		s_pUIMgr;					// UI Manager
	static class CUILoading*		s_pUILoading;				// 로딩바..
	static class CUIMessageBoxManager*	s_pMsgBoxMgr;			// MessageBox Manager

	static class CGameProcLogIn*			s_pProcLogIn;
	static class CGameProcNationSelect*		s_pProcNationSelect;
	static class CGameProcCharacterCreate*	s_pProcCharacterCreate;
	static class CGameProcCharacterSelect*	s_pProcCharacterSelect;
	static class CGameProcMain*				s_pProcMain;
	static class CGameProcOption*			s_pProcOption;

	static class CGameProcedure*			s_pProcPrev;
	static class CGameProcedure*			s_pProcActive;

	static class CGameCursor*				s_pGameCursor;

	static HCURSOR s_hCursorNormal;
	static HCURSOR s_hCursorNormal1;
	static HCURSOR s_hCursorClick;
	static HCURSOR s_hCursorClick1;
	static HCURSOR s_hCursorAttack;
	static HCURSOR s_hCursorPreRepair;
	static HCURSOR s_hCursorNowRepair;

	static e_LogInClassification	s_eLogInClassification; // 접속한 서비스.. MGame, Daum, KnightOnLine ....
	static std::string				s_szAccount; // 계정 문자열..
	static std::string				s_szPassWord; // 계정 비번..
	static std::string				s_szServer; // 서버이름 문자열..
	static int						s_iChrSelectIndex; // 이계정의 몇번째 캐릭인지...??

	static bool		m_bCursorLocked;
	static HCURSOR	m_hPrevGameCursor;
	static HWND		s_hWndSubSocket; // 서브 소켓용 윈도우 핸들..

	static bool		s_bNeedReportConnectionClosed; // 서버접속이 끊어진걸 보고해야 하는지..
	static bool		s_bWindowed; // 창모드 실행??
	static bool		s_bKeyPress;	//키가 눌려졌을때 ui에서 해당하는 조작된적이 있다면
	static bool		s_bKeyPressed;	//키가 올라갔을때 ui에서 해당하는 조작된적이 있다면

	static bool		s_bIsRestarting;

	// NOTE: adding boolean to check if window has focus or not
	static bool		s_bIsWindowInFocus;

public:
	static std::string MessageBoxPost(const std::string& szMsg, const std::string& szTitle, int iStyle, e_Behavior eBehavior = BEHAVIOR_NOTHING);
	static void MessageBoxClose(const std::string& szMsg);
	static void MessageBoxClose(int iMsgBoxIndex);

	static void ProcActiveSet(CGameProcedure* pProc);
	static void ReConnect();
	
	static void ReportServerConnectionFailed(const std::string& szServerName, int iErrCode, bool bNeedQuitGame);
	static void ReportServerConnectionClosed(bool bNeedQuitGame);
	static void ReportDebugStringAndSendToServer(const std::string& szDebug);

	virtual int		MsgRecv_VersionCheck(Packet& pkt); // 암호화 키도 같이 받는다..
	virtual int		MsgRecv_GameServerLogIn(Packet& pkt); // virtual - 국가번호를 리턴한다.
	virtual bool	MsgRecv_CharacterSelect(Packet& pkt);

	static void		MsgSend_GameServerLogIn();
	void			MsgSend_VersionCheck();
	virtual void	MsgSend_CharacterSelect();
	void			MsgRecv_CompressedPacket(Packet& pkt);


	
	virtual void Release(); // 리소스 풀어주기..
	virtual void Init(); // 필요한 요소들을 초기화 및 로딩
	virtual void Tick();  // 프로시져 인덱스를 리턴한다. 0 이면 그대로 진행
	virtual void Render();
	virtual void ProcessUIKeyInput(bool bEnable = true);

	static bool	IsUIKeyOperated();
	static void	StaticMemberInit(HINSTANCE hInstance, HWND hWndMain);
	static void StaticMemberRelease();

	static void TickActive();
	static void RenderActive();

	static bool RegPutSetting( const char *ValueName, void *pValueData, long length );
	static bool RegGetSetting( const char *ValueName, void *pValueData, long length );

	static void	UIPostData_Read(const std::string& szKey, class CN3UIBase* pUI, int iDefaultX, int iDefaultY);
	static void	UIPostData_Write(const std::string& szKey, CN3UIBase* pUI);

	static bool CaptureScreenAndSaveToFile(const std::string& szFN);

	static void	SetGameCursor(HCURSOR hCursor, bool bLocked = false);
	static void	SetGameCursor(e_Cursor eCursor, bool bLocked = false);
	static void RestoreGameCursor();

protected:
	virtual bool ProcessPacket(Packet& pkt);

private:
	static std::string GetStrRegKeySetting();

public:
	void LoadingUIChange(int iVictoryNation);
	CGameProcedure();
	virtual ~CGameProcedure();
};


#endif // !defined(AFX_GameProcedure_H__0BEC53F2_1282_402C_9A28_FB98CC131F64__INCLUDED_)

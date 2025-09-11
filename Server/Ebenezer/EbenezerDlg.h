// EbenezerDlg.h : header file
//

#if !defined(AFX_EBENEZERDLG_H__655A21EF_E029_42C0_890A_68DA7F542428__INCLUDED_)
#define AFX_EBENEZERDLG_H__655A21EF_E029_42C0_890A_68DA7F542428__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Iocport.h"
#include "Map.h"
#include "Define.h"
#include "GameDefine.h"
#include "AISocket.h"
#include "Npc.h"
#include "SharedMem.h"
#include "Knights.h"
#include "KnightsManager.h"
#include "EVENT.h"
#include "UdpSocket.h"

#include <shared/Ini.h>
#include <shared/logger.h>
#include <shared/STLMap.h>

#include <vector>

#include "resource.h"

namespace recordset_loader
{
	struct Error;
}

class EbenezerLogger : public logger::Logger
{
public:
	EbenezerLogger()
		: Logger(logger::Ebenezer)
	{
	}

	void SetupExtraLoggers(CIni& ini,
		std::shared_ptr<spdlog::details::thread_pool> threadPool,
		const std::string& baseDir) override;
};

/////////////////////////////////////////////////////////////////////////////
// CEbenezerDlg dialog

typedef std::vector <C3DMap*>				ZoneArray;
typedef std::vector <model::LevelUp*>		LevelUpTableArray;
typedef CSTLMap <model::Coefficient>		CoefficientTableMap;
typedef CSTLMap <model::Item>				ItemTableMap;
typedef CSTLMap <model::Magic>				MagicTableMap;
typedef CSTLMap <model::MagicType1>			MagicType1TableMap;
typedef CSTLMap <model::MagicType2>			MagicType2TableMap;
typedef CSTLMap <model::MagicType3>			MagicType3TableMap;
typedef CSTLMap	<model::MagicType4>			MagicType4TableMap;
typedef CSTLMap <model::MagicType5>			MagicType5TableMap;
typedef CSTLMap <model::MagicType7>			MagicType7TableMap;
typedef CSTLMap <model::MagicType8>			MagicType8TableMap;
typedef CSTLMap <CNpc>						NpcMap;
typedef CSTLMap <CAISocket>					AISocketMap;
typedef CSTLMap <_PARTY_GROUP>				PartyMap;
typedef CSTLMap <CKnights>					KnightsMap;
typedef CSTLMap <_ZONE_SERVERINFO>			ServerMap;
typedef CSTLMap <model::Home>				HomeTableMap;
typedef CSTLMap <model::ServerResource>		ServerResourceTableMap;
typedef CSTLMap <model::StartPosition>		StartPositionTableMap;
typedef	CSTLMap	<EVENT>						EventMap;

enum class NameType
{
	Account		= 1,
	Character	= 2
};

class CUser;
class CEbenezerDlg : public CDialog
{
// Construction
public:
	static inline CEbenezerDlg* GetInstance() {
		return s_pInstance;
	}

	C3DMap* GetMapByID(int iZoneID) const;
	C3DMap* GetMapByIndex(int iZoneIndex) const;
	void FlySanta();
	void BattleZoneCurrentUsers();
	BOOL LoadKnightsRankTable();
	void Send_CommandChat(char* pBuf, int len, int nation, CUser* pExceptUser = nullptr);
	BOOL LoadBattleTable();
	void Send_UDP_All(char* pBuf, int len, int group_type = 0);
	void KickOutZoneUsers(short zone);
	int64_t GenerateItemSerial();
	void KickOutAllUsers();
	void CheckAliveUser();
	int GetKnightsGrade(int nPoints);
	void WritePacketLog();
	void MarketBBSSellDelete(short index);
	void MarketBBSBuyDelete(short index);
	void MarketBBSTimeCheck();
	int  GetKnightsAllMembers(int knightsindex, char* temp_buff, int& buff_index, int type = 0);
	BOOL LoadAllKnightsUserData();
	BOOL LoadAllKnights();
	BOOL LoadStartPositionTable();
	BOOL LoadServerResourceTable();
	BOOL LoadHomeTable();
	void Announcement(BYTE type, int nation = 0, int chat_type = 8);
	void ResetBattleZone();
	void BanishLosers();
	void BattleZoneVictoryCheck();
	void BattleZoneOpenTimer();
	void BattleZoneOpen(int nType);	// 0:open 1:close
	void AliveUserCheck();
	void WithdrawUserOut();
	BOOL LoadMagicType8();
	BOOL LoadMagicType4();
	BOOL LoadMagicType5();
	BOOL LoadMagicType7();
	BOOL LoadMagicType3();
	BOOL LoadMagicType2();
	BOOL LoadMagicType1();
	void KillUser(const char* strbuff);
	void Send_PartyMember(int party, char* pBuf, int len);
	void Send_KnightsMember(int index, char* pBuf, int len, int zone = 100);
	BOOL AISocketConnect(int zone, bool flag, std::string* errorReason = nullptr);
	int GetAIServerPort() const;
	int GetRegionNpcIn(C3DMap* pMap, int region_x, int region_z, char* buff, int& t_count);
	BOOL LoadNoticeData();
	int GetZoneIndex(int zonenumber);
	int GetRegionNpcList(C3DMap* pMap, int region_x, int region_z, char* nid_buff, int& t_count, int nType = 0); // Region All Npcs nid Packaging Function
	void RegionNpcInfoForMe(CUser* pSendUser, int nType = 0);	// 9 Regions All Npcs nid Packaging Function
	int GetRegionUserList(C3DMap* pMap, int region_x, int region_z, char* buff, int& t_count); // Region All Users uid Packaging Function
	int GetRegionUserIn(C3DMap* pMap, int region_x, int region_z, char* buff, int& t_count);	// Region All Users USERINOUT Packet Packaging Function
	void RegionUserInOutForMe(CUser* pSendUser);	// 9 Regions All Users uid Packaging Function
	BOOL LoadLevelUpTable();
	void SetGameTime();
	void UpdateWeather();
	void UpdateGameTime();
	void LoadConfig();
	void Send_NearRegion(char* pBuf, int len, int zone, int region_x, int region_z, float curx, float curz, CUser* pExceptUser = nullptr);
	void Send_FilterUnitRegion(C3DMap* pMap, char* pBuf, int len, int x, int z, float ref_x, float ref_z, CUser* pExceptUser = nullptr);
	void Send_UnitRegion(C3DMap* pMap, char* pBuf, int len, int x, int z, CUser* pExceptUser = nullptr, bool bDirect = true);
	BOOL LoadCoefficientTable();
	BOOL LoadMagicTable();
	BOOL LoadItemTable();
	void ReportTableLoadError(const recordset_loader::Error& err, const char* source);
	BOOL MapFileLoad();
	void UserAcceptThread();
	// sungyong 2001.11.06
	BOOL AIServerConnect();
	void SyncTest(int nType);
	void SyncRegionTest(C3DMap* pMap, int rx, int rz, FILE* pfile, int nType);
	void SendAllUserInfo();
	void SendCompressedData();
	void DeleteAllNpcList(int flag = 0);
	CNpc* GetNpcPtr(int sid, int cur_zone);
	// ~sungyong 2001.11.06
	BOOL InitializeMMF();
	void UserInOutForMe(CUser* pSendUser);	// 9 Regions All Users USERINOUT Packet Packaging Function
	void NpcInOutForMe(CUser* pSendUser);	// 9 Regions All Npcs NPCINOUT Packet Packaging Function
	void Send_Region(char* pBuf, int len, int zone, int x, int z, CUser* pExceptUser = nullptr, bool bDirect = true);	// zone == real zone number
	void Send_All(char* pBuf, int len, CUser* pExceptUser = nullptr, int nation = 0);	// pointer != nullptr don`t send to that user pointer
	void Send_AIServer(int zone, char* pBuf, int len);
	static CUser* GetUserPtr(const char* userid, NameType type);

	/// \brief adds a message to the application's output box and updates scrollbar position
	/// \see _outputList
	void AddOutputMessage(const std::string& msg);

	/// \brief adds a message to the application's output box and updates scrollbar position
	/// \see _outputList
	void AddOutputMessage(const std::wstring& msg);

	CEbenezerDlg(CWnd* pParent = nullptr);	// standard constructor
	~CEbenezerDlg();

	static CEbenezerDlg* s_pInstance;
	static CIOCPort	m_Iocport;

	CSharedMemQueue	m_LoggerSendQueue;
	CSharedMemQueue	m_LoggerRecvQueue;
	CSharedMemQueue m_ItemLoggerSendQ;

	HANDLE	m_hReadQueueThread;
	HANDLE	m_hMMFile;
	char*	m_lpMMFile;
	BOOL	m_bMMFCreate;
	DWORD	m_ServerOffset;

	char	m_ppNotice[20][128];
	char	m_AIServerIP[20];

	AISocketMap				m_AISocketMap;
	NpcMap					m_NpcMap;
	ZoneArray				m_ZoneArray;
	ItemTableMap			m_ItemTableMap;
	MagicTableMap			m_MagicTableMap;
	MagicType1TableMap		m_MagicType1TableMap;
	MagicType2TableMap		m_MagicType2TableMap;
	MagicType3TableMap		m_MagicType3TableMap;
	MagicType4TableMap		m_MagicType4TableMap;
	MagicType5TableMap		m_MagicType5TableMap;
	MagicType7TableMap		m_MagicType7TableMap;
	MagicType8TableMap		m_MagicType8TableMap;
	CoefficientTableMap		m_CoefficientTableMap;		// 공식 계산 계수데이타 테이블
	LevelUpTableArray		m_LevelUpTableArray;
	PartyMap				m_PartyMap;
	KnightsMap				m_KnightsMap;
	HomeTableMap			m_HomeTableMap;
	ServerResourceTableMap	m_ServerResourceTableMap;
	StartPositionTableMap	m_StartPositionTableMap;
	EventMap				m_EventMap;

	CKnightsManager			m_KnightsManager;

	short	m_sPartyIndex;
	short	m_sZoneCount;							// AI Server 재접속시 사용
	short	m_sSocketCount;							// AI Server 재접속시 사용
	// sungyong 2002.05.23
	short   m_sSendSocket;
	BOOL	m_bFirstServerFlag;		// 서버가 처음시작한 후 게임서버가 붙은 경우에는 1, 붙지 않은 경우 0
	BOOL	m_bServerCheckFlag;
	BOOL	m_bPointCheckFlag;		// AI서버와 재접전에 NPC포인터 참조막기 (TRUE:포인터 참조, FALSE:포인터 참조 못함)
	short   m_sReSocketCount;		// GameServer와 재접시 필요
	float   m_fReConnectStart;	// 처음 소켓이 도착한 시간
	short   m_sErrorSocketCount;  // 이상소켓 감시용
	// ~sungyong 2002.05.23

	int m_iPacketCount;		// packet check
	int m_iSendPacketCount;		// packet check
	int m_iRecvPacketCount;		// packet check

	int m_nYear, m_nMonth, m_nDate, m_nHour, m_nMin, m_nWeather, m_nAmount;
	int m_nCastleCapture;

	// ~Yookozuna 2002.06.12
	BYTE    m_byBattleOpen, m_byOldBattleOpen;					// 0:전쟁중이 아님, 1:전쟁중(국가간전쟁), 2:눈싸움전쟁
	BYTE	m_bVictory, m_byOldVictory;
	BYTE	m_bKarusFlag, m_bElmoradFlag;
	BYTE    m_byKarusOpenFlag, m_byElmoradOpenFlag, m_byBanishFlag, m_byBattleSave;
	short   m_sDiscount;	// 능력치와 포인트 초기화 할인 (0:할인없음, 1:할인(50%) )
	short	m_sKarusDead, m_sElmoradDead, m_sBanishDelay, m_sKarusCount, m_sElmoradCount;
	int		m_nBattleZoneOpenWeek, m_nBattleZoneOpenHourStart, m_nBattleZoneOpenHourEnd;
	char	m_strKarusCaptain[MAX_ID_SIZE + 1];
	char	m_strElmoradCaptain[MAX_ID_SIZE + 1];

	// ~Yookozuna 2002.07.17
	BYTE	m_bMaxRegenePoint;

	// ~Yookozuna 2002.09.21 - Today is Chusok :( 
	short	m_sBuyID[MAX_BBS_POST];
	char	m_strBuyTitle[MAX_BBS_POST][MAX_BBS_TITLE];
	char	m_strBuyMessage[MAX_BBS_POST][MAX_BBS_MESSAGE];
	int		m_iBuyPrice[MAX_BBS_POST];
	float	m_fBuyStartTime[MAX_BBS_POST];

	short	m_sSellID[MAX_BBS_POST];
	char	m_strSellTitle[MAX_BBS_POST][MAX_BBS_TITLE];
	char	m_strSellMessage[MAX_BBS_POST][MAX_BBS_MESSAGE];
	int		m_iSellPrice[MAX_BBS_POST];
	float	m_fSellStartTime[MAX_BBS_POST];

	// ~Yookozuna 2002.11.26 - 비러머글 남는 공지 --;
	BOOL	m_bPermanentChatMode;
	BOOL	m_bPermanentChatFlag;
	char	m_strPermanentChat[1024];

	// ~Yookozuna 2002.12.11 - 갓댐 산타 클로스 --;
	BOOL	m_bSanta;

	// 패킷 압축에 필요 변수   -------------only from ai server
	int					m_CompCount;
	char				m_CompBuf[10240];
	int					m_iCompIndex;
	// ~패킷 압축에 필요 변수   -------------

	// zone server info
	int					m_nServerIndex;
	int					m_nServerNo, m_nServerGroupNo;
	int					m_nServerGroup;	// server의 번호(0:서버군이 없다, 1:서버1군, 2:서버2군)
	ServerMap			m_ServerArray;
	ServerMap			m_ServerGroupArray;
	CUdpSocket*			m_pUdpSocket;

// Dialog Data
	//{{AFX_DATA(CEbenezerDlg)
	enum { IDD = IDD_EBENEZER_DIALOG };
	CEdit	m_AnnounceEdit;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEbenezerDlg)
	virtual BOOL DestroyWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL
// Implementation

	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CEbenezerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
private:
	CIni	m_Ini;
	EbenezerLogger _logger;

	/// \brief output message box for the application
	CListBox _outputList;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EBENEZERDLG_H__655A21EF_E029_42C0_890A_68DA7F542428__INCLUDED_)

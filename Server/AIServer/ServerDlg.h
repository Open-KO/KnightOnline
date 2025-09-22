// ServerDlg.h : header file
//

#if !defined(AFX_SERVERDLG_H__7E2A41F8_68A3_4C94_8A6E_7C80636869D3__INCLUDED_)
#define AFX_SERVERDLG_H__7E2A41F8_68A3_4C94_8A6E_7C80636869D3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IOCPort.h"
#include "GameSocket.h"

#include "MAP.h"
#include "NpcItem.h"
#include "Pathfind.h"
#include "User.h"
#include "Npc.h"
#include "NpcThread.h"
#include "Server.h"
#include "Party.h"

#include "Extern.h"			// 전역 객체

#include "resource.h"

#include <shared/logger.h>
#include <shared/STLMap.h>

#include <vector>
#include <list>

namespace recordset_loader
{
	struct Error;
}

class AIServerLogger : public logger::Logger
{
public:
	AIServerLogger()
		: Logger(logger::AIServer)
	{
	}

	void SetupExtraLoggers(CIni& ini,
		std::shared_ptr<spdlog::details::thread_pool> threadPool,
		const std::string& baseDir) override;
};

/////////////////////////////////////////////////////////////////////////////
// CServerDlg dialog

typedef std::vector <CNpcThread*>			NpcThreadArray;
typedef CSTLMap <model::Npc>				NpcTableMap;
typedef CSTLMap <CNpc>						NpcMap;
typedef CSTLMap <model::Magic>				MagicTableMap;
typedef CSTLMap <model::MagicType1>			MagicType1TableMap;
typedef CSTLMap <model::MagicType2>			MagicType2TableMap;
typedef CSTLMap <model::MagicType3>			MagicType3TableMap;
typedef CSTLMap	<model::MagicType4>			MagicType4TableMap;
typedef CSTLMap	<model::MagicType7>			MagicType7TableMap;
typedef CSTLMap <_PARTY_GROUP>				PartyMap;
typedef CSTLMap <model::MakeWeapon>			MakeWeaponTableMap;
typedef CSTLMap <model::MakeItemGradeCode>	MakeGradeItemCodeTableMap;
typedef CSTLMap <model::MakeItemRareCode>	MakeItemRareCodeTableMap;
typedef std::list <int>						ZoneNpcInfoList;
typedef std::vector <MAP*>					ZoneArray;

/*
	 ** Repent AI Server 작업시 참고 사항 **
	1. 3개의 함수 추가
		int GetSpeed(BYTE bySpeed);
		int GetAttackSpeed(BYTE bySpeed);
		int GetCatsSpeed(BYTE bySpeed);
*/

class CServerDlg : public CDialog
{
// Construction
public:
	void GameServerAcceptThread();
	bool AddObjectEventNpc(_OBJECT_EVENT* pEvent, int zone_number);
	void AllNpcInfo();			// ~sungyong 2002.05.23
	CUser* GetUserPtr(int nid);
	CUser* GetActiveUserPtr(int index);
	CNpc* GetNpcPtr(const char* pNpcName);
	CNpc* GetEventNpcPtr();
	bool   SetSummonNpcData(CNpc* pNpc, int zone_id, float fx, float fz);
	int    MonsterSummon(const char* pNpcName, int zone_id, float fx, float fz);
	int GetZoneIndex(int zoneId) const;
	int GetServerNumber(int zoneId) const;
	void CloseSocket(int zonenumber);

	void CheckAliveTest();
	void DeleteUserList(int uid);
	void DeleteAllUserList(int zone);
	void SendCompressedData(int nZone);			// 패킷을 압축해서 보낸다..
	int Send(char* pData, int length, int nZone = 0);
	void SendSystemMsg(const std::string_view msg, int zone, int type = 0, int who = 0);
	void ResetBattleZone();
	MAP* GetMapByIndex(int iZoneIndex) const;
	MAP* GetMapByID(int iZoneID) const;

	/// \brief adds a message to the application's output box and updates scrollbar position
	/// \see _outputList
	void AddOutputMessage(const std::string& msg);

	/// \brief adds a message to the application's output box and updates scrollbar position
	/// \see _outputList
	void AddOutputMessage(const std::wstring& msg);

	CServerDlg(CWnd* pParent = nullptr);	// standard constructor
	~CServerDlg();

	static inline CServerDlg* GetInstance() {
		return s_pInstance;
	}

// Dialog Data
	//{{AFX_DATA(CServerDlg)
	enum { IDD = IDD_SERVER_DIALOG };
	CString	m_strStatus;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CServerDlg)
	virtual BOOL DestroyWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

public:
	NpcMap						m_NpcMap;
	NpcTableMap					m_MonTableMap;
	NpcTableMap					m_NpcTableMap;
	NpcThreadArray				m_NpcThreadArray;
	NpcThreadArray				m_EventNpcThreadArray;	// Event Npc Logic
	PartyMap					m_PartyMap;
	ZoneNpcInfoList				m_ZoneNpcList;
	MagicTableMap				m_MagicTableMap;
	MagicType1TableMap			m_MagicType1TableMap;
	MagicType2TableMap			m_MagicType2TableMap;
	MagicType3TableMap			m_MagicType3TableMap;
	MagicType4TableMap			m_MagicType4TableMap;
	MagicType7TableMap			m_MagicType7TableMap;
	MakeWeaponTableMap			m_MakeWeaponTableMap;
	MakeWeaponTableMap			m_MakeDefensiveTableMap;
	MakeGradeItemCodeTableMap	m_MakeGradeItemArray;
	MakeItemRareCodeTableMap	m_MakeItemRareCodeTableMap;
	ZoneArray					m_ZoneArray;

	CWinThread* m_pZoneEventThread;		// zone

	CUser* m_pUser[MAX_USER];

	// class 객체
	CNpcItem				m_NpcItem;

	// 전역 객체 변수
	//bool			m_bNpcExit;
	long			m_TotalNPC;			// DB에있는 총 수
	long			m_CurrentNPCError;	// 세팅에서 실패한 수
	long			m_CurrentNPC;		// 현재 게임상에서 실제로 셋팅된 수
	short			m_sTotalMap;		// Zone 수 
	short			m_sMapEventNpc;		// Map에서 읽어들이는 event npc 수

	// sungyong 2002.05.23
	bool			m_bFirstServerFlag;		// 서버가 처음시작한 후 게임서버가 붙은 경우에는 1, 붙지 않은 경우 0
	short m_sSocketCount;		// GameServer와 처음접시 필요
	short m_sReSocketCount;		// GameServer와 재접시 필요
	float m_fReConnectStart;	// 처음 소켓이 도착한 시간
	short m_sErrorSocketCount;  // 이상소켓 감시용
	// ~sungyong 2002.05.23
	BYTE  m_byBattleEvent;				   // 전쟁 이벤트 관련 플래그( 1:전쟁중이 아님, 0:전쟁중)
	short m_sKillKarusNpc, m_sKillElmoNpc; // 전쟁동안에 죽은 npc숫자

	int m_iYear, m_iMonth, m_iDate, m_iHour, m_iMin, m_iWeather, m_iAmount;
	BYTE	m_byNight;			// 밤인지,, 낮인지를 판단... 1:낮, 2:밤
	BYTE    m_byTestMode;

	CIOCPort m_Iocport;

	static CServerDlg* s_pInstance;

// Implementation
protected:
	void DefaultInit();

	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CServerDlg)
	virtual BOOL OnInitDialog();

	/// \brief attempts to listen on the port associated with m_byZone
	/// \see m_byZone
	/// \returns true when successful, otherwise false
	bool ListenByZone();

	/// \brief fetches the listen port associated with m_byZone
	/// \see m_byZone
	/// \returns the associated listen port or -1 if invalid
	int GetListenPortByZone() const;
	
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	afx_msg LRESULT OnGameServerLogin(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	// 패킷 압축에 필요 변수   -------------
	int					m_CompCount;
	char				m_CompBuf[10240];
	int					m_iCompIndex;
	// ~패킷 압축에 필요 변수   -------------

	BYTE				m_byZone;

	AIServerLogger		_logger;
	
	/// \brief output message box for the application
	CListBox _outputList;

	void ResumeAI();
	bool LoadNpcPosTable(std::vector<model::NpcPos*>& rows);
	bool CreateNpcThread();
	void ReportTableLoadError(const recordset_loader::Error& err, const char* source);
	bool GetMagicTableData();
	bool GetMagicType1Data();
	bool GetMagicType2Data();
	bool GetMagicType3Data();
	bool GetMagicType4Data();
	bool GetMagicType7Data();
	bool GetMonsterTableData();
	bool GetNpcTableData();
	bool GetNpcItemTable();
	bool GetMakeWeaponItemTableData();
	bool GetMakeDefensiveItemTableData();
	bool GetMakeGradeItemTableData();
	bool GetMakeRareItemTableData();
	bool MapFileLoad();
	void GetServerInfoIni();

	void SyncTest();
	void RegionCheck();		// region안에 들어오는 유저 체크 (스레드에서 FindEnermy()함수의 부하를 줄이기 위한 꽁수)
	void TestCode();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERVERDLG_H__7E2A41F8_68A3_4C94_8A6E_7C80636869D3__INCLUDED_)

// ServerDlg.h : header file
//

#if !defined(AFX_SERVERDLG_H__7E2A41F8_68A3_4C94_8A6E_7C80636869D3__INCLUDED_)
#define AFX_SERVERDLG_H__7E2A41F8_68A3_4C94_8A6E_7C80636869D3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable : 4786)

#include "IOCPort.h"
#include "GameSocket.h"

#include "MAP.h"
#include "NpcTable.h"
#include "NpcItem.h"
#include "Pathfind.h"
#include "User.h"
#include "Npc.h"
#include "NpcThread.h"
#include "Server.h"
#include "Compress.h"
#include "Party.h"

#include "extern.h"			// 전역 객체

#include <shared/STLMap.h>
#include <vector>
#include <list>

/////////////////////////////////////////////////////////////////////////////
// CServerDlg dialog

typedef std::vector <CNpcThread*>			NpcThreadArray;
typedef CSTLMap <CNpcTable>					NpcTableArray;
typedef CSTLMap <CNpc>						NpcArray;
typedef CSTLMap <_MAGIC_TABLE>				MagictableArray;
typedef CSTLMap <_MAGIC_TYPE1>				Magictype1Array;
typedef CSTLMap <_MAGIC_TYPE2>				Magictype2Array;
typedef CSTLMap <_MAGIC_TYPE3>				Magictype3Array;
typedef CSTLMap	<_MAGIC_TYPE4>				Magictype4Array;
typedef CSTLMap <_PARTY_GROUP>				PartyArray;
typedef CSTLMap <_MAKE_WEAPON>				MakeWeaponItemTableArray;
typedef CSTLMap <_MAKE_ITEM_GRADE_CODE>		MakeGradeItemTableArray;
typedef CSTLMap <_MAKE_ITEM_LARE_CODE>		MakeLareItemTableArray;
typedef std::list <int>						ZoneNpcInfoList;
typedef std::vector <MAP*>				ZoneArray;

/*
	 ** Repent AI Server 작업시 참고 사항 **
	1. 3개의 함수 추가
		int GetSpeed(BYTE bySpeed);
		int GetAttackSpeed(BYTE bySpeed);
		int GetCatsSpeed(BYTE bySpeed);
*/

class CServerDlg : public CDialog
{
private:
	void ResumeAI();
	BOOL CreateNpcThread();
	BOOL GetMagicTableData();
	BOOL GetMagicType1Data();
	BOOL GetMagicType2Data();
	BOOL GetMagicType3Data();
	BOOL GetMagicType4Data();
	BOOL GetMonsterTableData();
	BOOL GetNpcTableData();
	BOOL GetNpcItemTable();
	BOOL GetMakeWeaponItemTableData();
	BOOL GetMakeDefensiveItemTableData();
	BOOL GetMakeGradeItemTableData();
	BOOL GetMakeLareItemTableData();
	BOOL MapFileLoad();
	void GetServerInfoIni();

	void SyncTest();
	void RegionCheck();		// region안에 들어오는 유저 체크 (스레드에서 FindEnermy()함수의 부하를 줄이기 위한 꽁수)
	void TestCode();
// Construction
public:
	void GameServerAcceptThread();
	BOOL AddObjectEventNpc(_OBJECT_EVENT* pEvent, int zone_number);
	void AllNpcInfo();			// ~sungyong 2002.05.23
	CUser* GetUserPtr(int nid);
	CUser* GetActiveUserPtr(int index);
	CNpc* GetNpcPtr(TCHAR* pNpcName);
	CNpc* GetEventNpcPtr();
	BOOL   SetSummonNpcData(CNpc* pNpc, int zone, float fx, float fz);
	int    MonsterSummon(TCHAR* pNpcName, int zone, float fx, float fz);
	int GetZoneIndex(int zonenumber);
	int GetServerNumber(int zonenumber);
	void ClostSocket(int zonenumber);

	void CheckAliveTest();
	void DeleteUserList(int uid);
	void DeleteAllUserList(int zone);
	void SendCompressedData(int nZone);			// 패킷을 압축해서 보낸다..
	int Send(char* pData, int length, int nZone = 0);
	void SendSystemMsg(char* pMsg, int zone, int type = 0, int who = 0);
	void ResetBattleZone();

	CServerDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CServerDlg)
	enum {
		IDD = IDD_SERVER_DIALOG
	};
	CListBox	m_StatusList;
	CString	m_strStatus;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CServerDlg)
public:
	virtual BOOL DestroyWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

public:
//	ZoneArray			m_arZone;
	NpcArray			m_arNpc;
	NpcTableArray		m_arMonTable;
	NpcTableArray		m_arNpcTable;
	NpcThreadArray		m_arNpcThread;
	NpcThreadArray		m_arEventNpcThread;	// Event Npc Logic
	PartyArray			m_arParty;
	ZoneNpcInfoList		m_ZoneNpcList;
	MagictableArray		m_MagictableArray;
	Magictype1Array		m_Magictype1Array;
	Magictype2Array		m_Magictype2Array;
	Magictype3Array		m_Magictype3Array;
	Magictype4Array		m_Magictype4Array;
	MakeWeaponItemTableArray	m_MakeWeaponItemArray;
	MakeWeaponItemTableArray	m_MakeDefensiveItemArray;
	MakeGradeItemTableArray  m_MakeGradeItemArray;
	MakeLareItemTableArray  m_MakeLareItemArray;
	ZoneArray g_arZone;

	CWinThread* m_pZoneEventThread;		// zone

	CUser* m_pUser[MAX_USER];

	// class 객체
	CNpcItem				m_NpcItem;

	CFile					m_UserLogFile;
	CFile					m_ItemLogFile;

	// 전역 객체 변수
	//BOOL			m_bNpcExit;
	long			m_TotalNPC;			// DB에있는 총 수
	long			m_CurrentNPCError;	// 세팅에서 실패한 수
	long			m_CurrentNPC;		// 현재 게임상에서 실제로 셋팅된 수
	short			m_sTotalMap;		// Zone 수 
	short			m_sMapEventNpc;		// Map에서 읽어들이는 event npc 수

	// sungyong 2002.05.23
	BOOL			m_bFirstServerFlag;		// 서버가 처음시작한 후 게임서버가 붙은 경우에는 1, 붙지 않은 경우 0
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

private:
	// 패킷 압축에 필요 변수   -------------
	CCompressManager	m_CompMng;
	int					m_CompCount;
	TCHAR				m_CompBuf[10240];
	int					m_iCompIndex;
	// ~패킷 압축에 필요 변수   -------------

	BYTE				m_byZone;


// Implementation
protected:
	void DefaultInit();


//	CGameSocket m_GameSocket;

	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CServerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	afx_msg LRESULT OnGameServerLogin(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERVERDLG_H__7E2A41F8_68A3_4C94_8A6E_7C80636869D3__INCLUDED_)

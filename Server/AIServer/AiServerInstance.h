// AiServerInstance.h : header file
//
#pragma once

#include "AISocketManager.h"

#include "MAP.h"
#include "NpcItem.h"
#include "Npc.h"

#include "Extern.h"			// 전역 객체

#include <shared/Thread.h>

#include <shared-server/logger.h>
#include <shared-server/STLMap.h>

#include <vector>
#include <list>

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
// AiServerInstance dialog

class CNpcThread;
class ZoneEventThread;

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
typedef CSTLMap <model::MakeItemGroup>		MakeItemGroupMap;
typedef CSTLMap <model::MakeWeapon>			MakeWeaponTableMap;
typedef CSTLMap <model::MakeItemGradeCode>	MakeGradeItemCodeTableMap;
typedef CSTLMap <model::MakeItemRareCode>	MakeItemRareCodeTableMap;
typedef std::list <int>						ZoneNpcInfoList;
typedef std::vector <MAP*>					ZoneArray;

class TimerThread;
class AiServerInstance : public Thread
{
// Construction
public:
	static AiServerInstance* instance()
	{
		return s_instance;
	}

	void GameServerAcceptThread();
	bool AddObjectEventNpc(_OBJECT_EVENT* pEvent, int zone_number);
	void AllNpcInfo();
	CUser* GetUserPtr(int nid);
	int GetZoneIndex(int zoneId) const;
	int GetServerNumber(int zoneId) const;

	void CheckAliveTest();
	void DeleteUserList(int uid);
	void DeleteAllUserList(int zone);
	void SendCompressedData(int nZone);			// 패킷을 압축해서 보낸다..
	int Send(const char* pData, int length, int nZone = 0);
	void SendSystemMsg(const std::string_view msg, int zone, int type = 0, int who = 0);
	void ResetBattleZone();
	MAP* GetMapByIndex(int iZoneIndex) const;
	MAP* GetMapByID(int iZoneID) const;

	AiServerInstance(AIServerLogger& logger);
	~AiServerInstance();

	NpcMap						m_NpcMap;
	NpcTableMap					m_MonTableMap;
	NpcTableMap					m_NpcTableMap;
	NpcThreadArray				m_NpcThreadArray;
	PartyMap					m_PartyMap;
	ZoneNpcInfoList				m_ZoneNpcList;
	MagicTableMap				m_MagicTableMap;
	MagicType1TableMap			m_MagicType1TableMap;
	MagicType2TableMap			m_MagicType2TableMap;
	MagicType3TableMap			m_MagicType3TableMap;
	MagicType4TableMap			m_MagicType4TableMap;
	MagicType7TableMap			m_MagicType7TableMap;
	MakeItemGroupMap			m_MakeItemGroupTableMap;
	MakeWeaponTableMap			m_MakeWeaponTableMap;
	MakeWeaponTableMap			m_MakeDefensiveTableMap;
	MakeGradeItemCodeTableMap	m_MakeGradeItemArray;
	MakeItemRareCodeTableMap	m_MakeItemRareCodeTableMap;
	ZoneArray					m_ZoneArray;

	ZoneEventThread*			m_pZoneEventThread;		// zone

	CUser*			m_pUser[MAX_USER];

	// class 객체
	CNpcItem		m_NpcItem;

	// 전역 객체 변수
	long			m_TotalNPC;			// DB에있는 총 수
	long			m_CurrentNPCError;	// 세팅에서 실패한 수
	std::atomic<long>	m_CurrentNPC;		// 현재 게임상에서 실제로 셋팅된 수
	int16_t			m_sTotalMap;		// Zone 수 
	int16_t			m_sMapEventNpc;		// Map에서 읽어들이는 event npc 수

	// sungyong 2002.05.23
	bool			m_bFirstServerFlag;	// 서버가 처음시작한 후 게임서버가 붙은 경우에는 1, 붙지 않은 경우 0
	int16_t			m_sSocketCount;		// GameServer와 처음접시 필요
	int16_t			m_sReSocketCount;	// GameServer와 재접시 필요
	double			m_fReConnectStart;	// 처음 소켓이 도착한 시간
	int16_t			m_sErrorSocketCount;  // 이상소켓 감시용
	// ~sungyong 2002.05.23
	uint8_t			m_byBattleEvent;	// 전쟁 이벤트 관련 플래그( 1:전쟁중이 아님, 0:전쟁중)
	int16_t			m_sKillKarusNpc, m_sKillElmoNpc; // 전쟁동안에 죽은 npc숫자

	int				m_iYear, m_iMonth, m_iDate, m_iHour, m_iMin, m_iWeather, m_iAmount;
	uint8_t			m_byNight;			// 밤인지,, 낮인지를 판단... 1:낮, 2:밤
	uint8_t			m_byTestMode;

	AISocketManager	_socketManager;

protected:
	/// \brief Loads config, database caches, then starts sockets and thread pools.
	/// \returns true when successful, false otherwise
	bool OnStart();

	/// \brief The main thread loop for the server instance
	void thread_loop() override;

	/// \brief attempts to listen on the port associated with m_byZone
	/// \see m_byZone
	/// \returns true when successful, otherwise false
	bool ListenByZone();

	/// \brief fetches the listen port associated with m_byZone
	/// \see m_byZone
	/// \returns the associated listen port or -1 if invalid
	int GetListenPortByZone() const;

private:
	// 패킷 압축에 필요 변수   -------------
	int					m_CompCount;
	char				m_CompBuf[10240];
	int					m_iCompIndex;
	// ~패킷 압축에 필요 변수   -------------

	uint8_t				m_byZone;

	AIServerLogger&		_logger;

	std::unique_ptr<TimerThread>	_checkAliveThread;

	static AiServerInstance* s_instance;

	void StartNpcThreads();
	bool LoadNpcPosTable(std::vector<model::NpcPos*>& rows);
	bool CreateNpcThread();
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
	bool GetMakeItemGroupTableData();
	bool MapFileLoad();
	void GetServerInfoIni();

	void SyncTest();
	void RegionCheck();		// region안에 들어오는 유저 체크 (스레드에서 FindEnermy()함수의 부하를 줄이기 위한 꽁수)
	void TestCode();
};

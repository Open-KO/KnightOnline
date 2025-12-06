#pragma once

#include "AISocketManager.h"

#include "MAP.h"
#include "NpcItem.h"
#include "Npc.h"

#include "Extern.h"			// 전역 객체

#include <shared-server/AppThread.h>
#include <shared-server/STLMap.h>

#include <vector>
#include <list>

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
typedef CSTLMap <model::ZoneInfo>			ZoneInfoTableMap;
typedef std::list <int>						ZoneNpcInfoList;
typedef std::vector <MAP*>					ZoneArray;

class AIServerLogger;
class TimerThread;
class AIServerApp : public AppThread
{
public:
	static AIServerApp* instance()
	{
		return static_cast<AIServerApp*>(s_instance);
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

	AIServerApp(AIServerLogger& logger);
	~AIServerApp();

	NpcMap						_npcMap;
	NpcTableMap					_monTableMap;
	NpcTableMap					_npcTableMap;
	NpcThreadArray				_npcThreads;
	PartyMap					_partyMap;
	MagicTableMap				_magicTableMap;
	MagicType1TableMap			_magicType1TableMap;
	MagicType2TableMap			_magicType2TableMap;
	MagicType3TableMap			_magicType3TableMap;
	MagicType4TableMap			_magicType4TableMap;
	MagicType7TableMap			_magicType7TableMap;
	MakeItemGroupMap			_makeItemGroupTableMap;
	MakeWeaponTableMap			_makeWeaponTableMap;
	MakeWeaponTableMap			_makeDefensiveTableMap;
	MakeGradeItemCodeTableMap	_makeGradeItemCodeTableMap;
	MakeItemRareCodeTableMap	_makeItemRareCodeTableMap;
	ZoneArray					_zones;
	ZoneInfoTableMap			_zoneInfoTableMap;

	ZoneEventThread*			_zoneEventThread;

	CUser*						_users[MAX_USER];

	// class 객체
	CNpcItem					_npcItem;

	// 전역 객체 변수
	long						_totalNpcCount;		// DB에있는 총 수
	std::atomic<long>			_loadedNpcCount;	// 현재 게임상에서 실제로 셋팅된 수
	int16_t						_mapCount;			// Zone 수 
	int16_t						_mapEventNpcCount;	// Map에서 읽어들이는 event npc 수

	// sungyong 2002.05.23
	bool						_firstServerFlag;	// 서버가 처음시작한 후 게임서버가 붙은 경우에는 1, 붙지 않은 경우 0
	int16_t						_socketCount;		// GameServer와 처음접시 필요
	int16_t						_reconnectSocketCount;	// GameServer와 재접시 필요
	double						_reconnectStartTime;	// 처음 소켓이 도착한 시간
	int16_t						_aliveSocketCount;  // 이상소켓 감시용
	// ~sungyong 2002.05.23
	uint8_t						_battleEventType;	// 전쟁 이벤트 관련 플래그( 1:전쟁중이 아님, 0:전쟁중)
	int16_t						_battleNpcsKilledByKarus, _battleNpcsKilledByElmorad; // 전쟁동안에 죽은 npc숫자

	int							_year, _month, _dayOfMonth, _hour, _minute;
	int							_weatherType, _weatherAmount;
	uint8_t						_nightMode;			// 밤인지,, 낮인지를 판단... 1:낮, 2:밤
	uint8_t						_testMode;

	AISocketManager				_socketManager;

private:
	// 패킷 압축에 필요 변수   -------------
	int							_compressedPacketCount;
	char						_compressedPacketBuffer[10240];
	int							_compressedPacketIndex;
	// ~패킷 압축에 필요 변수   -------------

	uint8_t						_serverZoneType;

	std::unique_ptr<TimerThread>	_checkAliveThread;

protected:
	/// \brief Loads config, database caches, then starts sockets and thread pools.
	/// \returns true when successful, false otherwise
	bool OnStart() override;

	/// \brief attempts to listen on the port associated with _serverZoneType
	/// \see _serverZoneType
	/// \returns true when successful, otherwise false
	bool ListenByServerZoneType();

	/// \brief fetches the listen port associated with _serverZoneType
	/// \see _serverZoneType
	/// \returns the associated listen port or -1 if invalid
	int GetListenPortByServerZoneType() const;

private:
	void StartNpcThreads();
	bool LoadNpcPosTable(std::vector<model::NpcPos*>& rows);
	bool CreateNpcThread();
	bool GetMagicTableData();
	bool GetMagicType1Data();
	bool GetMagicType2Data();
	bool GetMagicType3Data();
	bool GetMagicType4Data();
	bool GetMagicType7Data();
	bool GetZoneInfoTableData();
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

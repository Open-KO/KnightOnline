#ifndef SERVER_AISERVER_AISERVERAPP_H
#define SERVER_AISERVER_AISERVERAPP_H

#pragma once

#include <shared-server/AppThread.h>
#include <shared-server/STLMap.h>

#include "AISocketManager.h"

#include "Map.h"
#include "NpcItem.h"
#include "Npc.h"

#include "Extern.h" // 전역 객체

#include <list>
#include <vector>

class TimerThread;

namespace AIServer
{

class CNpcThread;
class ZoneEventThread;

using NpcThreadArray            = std::vector<CNpcThread*>;
using NpcTableMap               = CSTLMap<model::Npc>;
using NpcMap                    = CSTLMap<CNpc>;
using MagicTableMap             = CSTLMap<model::Magic>;
using MagicType1TableMap        = CSTLMap<model::MagicType1>;
using MagicType2TableMap        = CSTLMap<model::MagicType2>;
using MagicType3TableMap        = CSTLMap<model::MagicType3>;
using MagicType4TableMap        = CSTLMap<model::MagicType4>;
using MagicType7TableMap        = CSTLMap<model::MagicType7>;
using PartyMap                  = CSTLMap<_PARTY_GROUP>;
using MakeItemGroupMap          = CSTLMap<model::MakeItemGroup>;
using MakeWeaponTableMap        = CSTLMap<model::MakeWeapon>;
using MakeGradeItemCodeTableMap = CSTLMap<model::MakeItemGradeCode>;
using MakeItemRareCodeTableMap  = CSTLMap<model::MakeItemRareCode>;
using ZoneInfoTableMap          = CSTLMap<model::ZoneInfo>;
using ZoneNpcInfoList           = std::list<int>;
using ZoneArray                 = std::vector<MAP*>;

class AIServerLogger;
class AIServerApp : public AppThread
{
public:
	static AIServerApp* instance()
	{
		return static_cast<AIServerApp*>(s_instance);
	}

	std::shared_ptr<spdlog::logger>& itemLogger()
	{
		return _itemLogger;
	}

	std::shared_ptr<spdlog::logger>& userLogger()
	{
		return _userLogger;
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
	void SendCompressedData(int nZone); // 패킷을 압축해서 보낸다..
	int Send(const char* pData, int length, int nZone = 0);
	void SendSystemMsg(const std::string_view msg, int zone, int type = 0, int who = 0);
	void ResetBattleZone();
	MAP* GetMapByIndex(int iZoneIndex) const;
	MAP* GetMapByID(int iZoneID) const;

	AIServerApp(AIServerLogger& logger);
	~AIServerApp() override;

	NpcMap _npcMap;
	NpcTableMap _monTableMap;
	NpcTableMap _npcTableMap;
	NpcThreadArray _npcThreads;
	PartyMap _partyMap;
	MagicTableMap _magicTableMap;
	MagicType1TableMap _magicType1TableMap;
	MagicType2TableMap _magicType2TableMap;
	MagicType3TableMap _magicType3TableMap;
	MagicType4TableMap _magicType4TableMap;
	MagicType7TableMap _magicType7TableMap;
	MakeItemGroupMap _makeItemGroupTableMap;
	MakeWeaponTableMap _makeWeaponTableMap;
	MakeWeaponTableMap _makeDefensiveTableMap;
	MakeGradeItemCodeTableMap _makeGradeItemCodeTableMap;
	MakeItemRareCodeTableMap _makeItemRareCodeTableMap;
	ZoneArray _zones;
	ZoneInfoTableMap _zoneInfoTableMap;

	ZoneEventThread* _zoneEventThread = nullptr;

	CUser* _users[MAX_USER]           = {};

	// class 객체
	CNpcItem _npcItem;

	// 전역 객체 변수
	long _totalNpcCount                = 0; // DB에있는 총 수
	std::atomic<long> _loadedNpcCount  = 0; // 현재 게임상에서 실제로 셋팅된 수
	int16_t _mapCount                  = 0; // Zone 수
	int16_t _mapEventNpcCount          = 0; // Map에서 읽어들이는 event npc 수

	// 서버가 처음시작한 후 게임서버가 붙은 경우에는 1, 붙지 않은 경우 0
	bool _firstServerFlag              = false;
	int16_t _socketCount               = 0;   // GameServer와 처음접시 필요
	int16_t _reconnectSocketCount      = 0;   // GameServer와 재접시 필요
	double _reconnectStartTime         = 0.0; // 처음 소켓이 도착한 시간
	int16_t _aliveSocketCount          = 0;   // 이상소켓 감시용

	// 전쟁 이벤트 관련 플래그( 1:전쟁중이 아님, 0:전쟁중)
	uint8_t _battleEventType           = BATTLEZONE_CLOSE;

	// 전쟁동안에 죽은 npc숫자
	int16_t _battleNpcsKilledByKarus   = 0;
	int16_t _battleNpcsKilledByElmorad = 0;

	int _year                          = 0;
	int _month                         = 0;
	int _dayOfMonth                    = 0;
	int _hour                          = 0;
	int _minute                        = 0;
	int _weatherType                   = 0;
	int _weatherAmount                 = 0;
	uint8_t _nightMode                 = 1; // 밤인지,, 낮인지를 판단... 1:낮, 2:밤
	uint8_t _testMode                  = 0;

	AISocketManager _serverSocketManager;

private:
	// 패킷 압축에 필요 변수   -------------
	int _compressedPacketCount = 0;
	char _compressedPacketBuffer[10240];
	int _compressedPacketIndex = 0;
	// ~패킷 압축에 필요 변수   -------------

	uint8_t _serverZoneType    = KARUS_ZONE;

	std::unique_ptr<TimerThread> _checkAliveThread;

	std::filesystem::path _mapDir;
	std::string _overrideMapDir;

	std::filesystem::path _eventDir;
	std::string _overrideEventDir;

	std::shared_ptr<spdlog::logger> _itemLogger;
	std::shared_ptr<spdlog::logger> _userLogger;

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

	/// \brief Sets up the command-line arg parser, binding args for parsing.
	void SetupCommandLineArgParser(argparse::ArgumentParser& parser) override;

	/// \brief Processes any parsed command-line args as needed by the app.
	/// \returns true on success, false on failure
	bool ProcessCommandLineArgs(const argparse::ArgumentParser& parser) override;

	/// \returns The application's ini config path.
	std::filesystem::path ConfigPath() const override;

	/// \brief Loads application-specific config from the loaded application ini file (`iniFile`).
	/// \param iniFile The loaded application ini file.
	/// \returns true when successful, false otherwise
	bool LoadConfig(CIni& iniFile) override;

	void SyncTest();

	// region안에 들어오는 유저 체크 (스레드에서 FindEnermy()함수의 부하를 줄이기 위한 꽁수)
	void RegionCheck();
};

} // namespace AIServer

#endif // SERVER_AISERVER_AISERVERAPP_H

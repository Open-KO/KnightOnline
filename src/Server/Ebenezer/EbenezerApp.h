#ifndef SERVER_EBENEZER_EBENEZERAPP_H
#define SERVER_EBENEZER_EBENEZERAPP_H

#pragma once

#include "EbenezerSocketManager.h"
#include "Map.h"
#include "Define.h"
#include "GameDefine.h"
#include "AISocket.h"
#include "Npc.h"
#include "Knights.h"
#include "KnightsManager.h"
#include "KnightsSiegeWar.h"
#include "EVENT.h"
#include "UdpSocket.h"

#include <shared/Ini.h>
#include <shared-server/AppThread.h>
#include <shared-server/SharedMemoryBlock.h>
#include <shared-server/SharedMemoryQueue.h>
#include <shared-server/STLMap.h>
#include <shared-server/TcpClientSocketManager.h>

#include <unordered_map>
#include <vector>
#include <queue>

class ReadQueueThread;
class TimerThread;

namespace Ebenezer
{

using ZoneArray              = std::vector<C3DMap*>;
using LevelUpTableArray      = std::vector<model::LevelUp*>;
using CoefficientTableMap    = CSTLMap<model::Coefficient>;
using ItemTableMap           = CSTLMap<model::Item>;
using MagicTableMap          = CSTLMap<model::Magic>;
using MagicType1TableMap     = CSTLMap<model::MagicType1>;
using MagicType2TableMap     = CSTLMap<model::MagicType2>;
using MagicType3TableMap     = CSTLMap<model::MagicType3>;
using MagicType4TableMap     = CSTLMap<model::MagicType4>;
using MagicType5TableMap     = CSTLMap<model::MagicType5>;
using MagicType7TableMap     = CSTLMap<model::MagicType7>;
using MagicType8TableMap     = CSTLMap<model::MagicType8>;
using NpcMap                 = CSTLMap<CNpc>;
using AISocketMap            = std::unordered_map<int, std::shared_ptr<TcpClientSocket>>;
using PartyMap               = CSTLMap<_PARTY_GROUP>;
using KnightsMap             = CSTLMap<CKnights>;
using ServerMap              = CSTLMap<_ZONE_SERVERINFO>;
using HomeTableMap           = CSTLMap<model::Home>;
using ServerResourceTableMap = CSTLMap<model::ServerResource>;
using StartPositionTableMap  = CSTLMap<model::StartPosition>;
using EventMap               = CSTLMap<EVENT>;

using EventTriggerMap        = std::unordered_map<uint32_t, int32_t>;

enum class NameType : uint8_t
{
	Account   = 1,
	Character = 2
};

class CUser;
class EbenezerLogger;
class EbenezerApp : public AppThread
{
public:
	static EbenezerApp* instance()
	{
		return static_cast<EbenezerApp*>(s_instance);
	}

	std::shared_ptr<spdlog::logger> regionLogger()
	{
		return _regionLogger;
	}

	std::shared_ptr<spdlog::logger> eventLogger()
	{
		return _eventLogger;
	}

	void GameTimeTick();
	void SendSMQHeartbeat();
	uint32_t GetEventTriggerKey(uint8_t byNpcType, uint16_t sTrapNumber) const;
	int32_t GetEventTrigger(uint8_t byNpcType, uint16_t sTrapNumber) const;
	bool LoadEventTriggerTable();
	C3DMap* GetMapByID(int iZoneID) const;
	C3DMap* GetMapByIndex(int iZoneIndex) const;
	void FlySanta();
	void BattleZoneCurrentUsers();
	bool LoadKnightsRankTable();
	void Send_CommandChat(char* pBuf, int len, int nation, CUser* pExceptUser = nullptr);
	bool LoadBattleTable();
	void Send_UDP_All(char* pBuf, int len, int group_type = 0);
	void KickOutZoneUsers(int16_t zone);
	int64_t GenerateItemSerial();
	void KickOutAllUsers();
	void CheckAliveUser();
	int GetKnightsGrade(int nPoints);
	void WritePacketLog();
	void MarketBBSSellDelete(int16_t index);
	void MarketBBSBuyDelete(int16_t index);
	void MarketBBSTimeCheck();
	int GetKnightsAllMembers(int knightsindex, char* temp_buff, int& buff_index, int type = 0);
	bool LoadKnightsSiegeWarfareTable();
	bool LoadAllKnightsUserData();
	bool LoadAllKnights();
	bool LoadStartPositionTable();
	bool LoadServerResourceTable();
	bool LoadHomeTable();
	void Announcement(uint8_t type, int nation = 0, int chat_type = 8);
	void ResetBattleZone();
	void BanishLosers();
	void BattleZoneVictoryCheck();
	void BattleZoneOpenTimer();
	void BattleZoneOpen(int nType); // 0:open 1:close
	void AliveUserCheck();
	void WithdrawUserOut();
	bool LoadMagicType8();
	bool LoadMagicType4();
	bool LoadMagicType5();
	bool LoadMagicType7();
	bool LoadMagicType3();
	bool LoadMagicType2();
	bool LoadMagicType1();
	void KillUser(const char* strbuff);
	void Send_PartyMember(int party, char* pBuf, int len);
	void Send_KnightsMember(int index, char* pBuf, int len, int zone = 100);
	bool AISocketConnect(int zone, bool flag);
	int GetAIServerPort() const;
	int GetRegionNpcIn(C3DMap* pMap, int region_x, int region_z, char* buff, int& t_count);
	bool LoadNoticeData();
	bool HandleCommand(const std::string& command) override;
	int GetZoneIndex(int zonenumber) const;
	int GetRegionNpcList(C3DMap* pMap, int region_x, int region_z, char* nid_buff, int& t_count,
		int nType = 0);                   // Region All Npcs nid Packaging Function
	void RegionNpcInfoForMe(
		CUser* pSendUser, int nType = 0); // 9 Regions All Npcs nid Packaging Function
	int GetRegionUserList(C3DMap* pMap, int region_x, int region_z, char* buff,
		int& t_count);                    // Region All Users uid Packaging Function
	int GetRegionUserIn(C3DMap* pMap, int region_x, int region_z, char* buff,
		int& t_count);                    // Region All Users USERINOUT Packet Packaging Function
	void RegionUserInOutForMe(CUser* pSendUser); // 9 Regions All Users uid Packaging Function
	bool LoadLevelUpTable();
	void SetGameTime();
	void UpdateWeather();
	void UpdateGameTime();
	void Send_NearRegion(char* pBuf, int len, int zone, int region_x, int region_z, float curx,
		float curz, CUser* pExceptUser = nullptr);
	void Send_FilterUnitRegion(C3DMap* pMap, char* pBuf, int len, int x, int z, float ref_x,
		float ref_z, CUser* pExceptUser = nullptr);
	void Send_UnitRegion(C3DMap* pMap, char* pBuf, int len, int x, int z,
		CUser* pExceptUser = nullptr, bool bDirect = true);
	bool LoadCoefficientTable();
	bool LoadMagicTable();
	bool LoadItemTable();
	bool MapFileLoad();
	void UserAcceptThread();
	// sungyong 2001.11.06
	bool AIServerConnect();
	void SendAllUserInfo();
	void SendCompressedData();
	void DeleteAllNpcList(int flag = 0);
	CNpc* GetNpcPtr(int sid, int cur_zone);
	CKnights* GetKnightsPtr(int16_t clanId);
	// ~sungyong 2001.11.06
	bool InitializeMMF();
	void UserInOutForMe(
		CUser* pSendUser);                // 9 Regions All Users USERINOUT Packet Packaging Function
	void NpcInOutForMe(CUser* pSendUser); // 9 Regions All Npcs NPCINOUT Packet Packaging Function
	void Send_Region(char* pBuf, int len, int zone, int x, int z, CUser* pExceptUser = nullptr,
		bool bDirect = true);             // zone == real zone number
	void Send_All(char* pBuf, int len, CUser* pExceptUser = nullptr,
		int nation = 0);                  // pointer != nullptr don`t send to that user pointer
	void Send_AIServer(int zone, char* pBuf, int len);

	std::shared_ptr<CUser> GetUserPtr(const char* userid, NameType type);

	inline std::shared_ptr<CUser> GetUserPtr(int socketId) const
	{
		return _serverSocketManager.GetUser(socketId);
	}

	inline std::shared_ptr<CUser> GetUserPtrUnchecked(int socketId) const
	{
		return _serverSocketManager.GetUserUnchecked(socketId);
	}

	inline int GetUserSocketCount() const
	{
		return _serverSocketManager.GetSocketCount();
	}

	inline bool IsValidUserId(int socketId) const
	{
		return _serverSocketManager.IsValidSocketId(socketId);
	}

	EbenezerApp(EbenezerLogger& logger);
	~EbenezerApp() override;

	EbenezerSocketManager _serverSocketManager;
	TcpClientSocketManager _aiSocketManager;

	SharedMemoryQueue m_LoggerSendQueue;
	SharedMemoryQueue m_LoggerRecvQueue;
	SharedMemoryQueue m_ItemLoggerSendQ;

	SharedMemoryBlock m_UserDataBlock;

	uint32_t m_ServerOffset;

	char m_ppNotice[20][128];
	std::string m_AIServerIP;

	AISocketMap _aiSocketMap;
	std::mutex _aiSocketMutex;

	NpcMap m_NpcMap;
	ZoneArray m_ZoneArray;
	ItemTableMap m_ItemTableMap;
	MagicTableMap m_MagicTableMap;
	MagicType1TableMap m_MagicType1TableMap;
	MagicType2TableMap m_MagicType2TableMap;
	MagicType3TableMap m_MagicType3TableMap;
	MagicType4TableMap m_MagicType4TableMap;
	MagicType5TableMap m_MagicType5TableMap;
	MagicType7TableMap m_MagicType7TableMap;
	MagicType8TableMap m_MagicType8TableMap;
	CoefficientTableMap m_CoefficientTableMap; // 공식 계산 계수데이타 테이블
	LevelUpTableArray m_LevelUpTableArray;
	PartyMap m_PartyMap;
	KnightsMap m_KnightsMap;
	HomeTableMap m_HomeTableMap;
	ServerResourceTableMap m_ServerResourceTableMap;
	StartPositionTableMap m_StartPositionTableMap;
	EventMap m_EventMap;
	EventTriggerMap m_EventTriggerMap;

	CKnightsManager m_KnightsManager;
	CKnightsSiegeWar m_KnightsSiegeWar;

	int16_t m_sPartyIndex;
	int16_t m_sZoneCount;   // AI Server 재접속시 사용
	int16_t m_sSocketCount; // AI Server 재접속시 사용
	// sungyong 2002.05.23
	int16_t m_sSendSocket;
	bool m_bFirstServerFlag; // 서버가 처음시작한 후 게임서버가 붙은 경우에는 1, 붙지 않은 경우 0
	bool m_bServerCheckFlag;
	bool
		m_bPointCheckFlag; // AI서버와 재접전에 NPC포인터 참조막기 (true:포인터 참조, false:포인터 참조 못함)
	int16_t m_sReSocketCount;    // GameServer와 재접시 필요
	double m_fReConnectStart;    // 처음 소켓이 도착한 시간
	int16_t m_sErrorSocketCount; // 이상소켓 감시용
	// ~sungyong 2002.05.23

	int m_iPacketCount;     // packet check
	int m_iSendPacketCount; // packet check
	int m_iRecvPacketCount; // packet check

	int m_nYear, m_nMonth, m_nDate, m_nHour, m_nMin, m_nWeather, m_nAmount;
	int m_nCastleCapture;

	// ~Yookozuna 2002.06.12
	uint8_t m_byBattleOpen,
		m_byOldBattleOpen; // 0:전쟁중이 아님, 1:전쟁중(국가간전쟁), 2:눈싸움전쟁
	uint8_t m_bVictory, m_byOldVictory;
	uint8_t m_bKarusFlag, m_bElmoradFlag;
	uint8_t m_byKarusOpenFlag, m_byElmoradOpenFlag, m_byBanishFlag, m_byBattleSave;
	int16_t m_sDiscount; // 능력치와 포인트 초기화 할인 (0:할인없음, 1:할인(50%) )
	int16_t m_sKarusDead, m_sElmoradDead, m_sBanishDelay, m_sKarusCount, m_sElmoradCount;

	uint8_t _karusInvasionMonumentLastCapturedNation[INVASION_MONUMENT_COUNT];
	uint8_t _elmoradInvasionMonumentLastCapturedNation[INVASION_MONUMENT_COUNT];

	int m_nBattleZoneOpenWeek, m_nBattleZoneOpenHourStart, m_nBattleZoneOpenHourEnd;
	char m_strKarusCaptain[MAX_ID_SIZE + 1];
	char m_strElmoradCaptain[MAX_ID_SIZE + 1];

	// ~Yookozuna 2002.07.17
	uint8_t m_bMaxRegenePoint;

	// ~Yookozuna 2002.09.21 - Today is Chusok :(
	int16_t m_sBuyID[MAX_BBS_POST];
	char m_strBuyTitle[MAX_BBS_POST][MAX_BBS_TITLE];
	char m_strBuyMessage[MAX_BBS_POST][MAX_BBS_MESSAGE];
	int m_iBuyPrice[MAX_BBS_POST];
	double m_fBuyStartTime[MAX_BBS_POST];

	int16_t m_sSellID[MAX_BBS_POST];
	char m_strSellTitle[MAX_BBS_POST][MAX_BBS_TITLE];
	char m_strSellMessage[MAX_BBS_POST][MAX_BBS_MESSAGE];
	int m_iSellPrice[MAX_BBS_POST];
	double m_fSellStartTime[MAX_BBS_POST];

	// ~Yookozuna 2002.11.26 - 비러머글 남는 공지 --;
	bool m_bPermanentChatMode;
	bool m_bPermanentChatFlag;
	char m_strPermanentChat[1024];

	// ~Yookozuna 2002.12.11 - 갓댐 산타 클로스 --;
	uint8_t m_bySanta;

	e_BeefRoastVictory _beefRoastVictoryType;

	uint8_t _monsterChallengeActiveType;
	uint8_t _monsterChallengeState; // TODO: enum this
	int16_t _monsterChallengePlayerCount;

	// 패킷 압축에 필요 변수   -------------only from ai server
	int m_CompCount;
	char m_CompBuf[10240];
	int m_iCompIndex;
	// ~패킷 압축에 필요 변수   -------------

	// zone server info
	int m_nServerIndex;
	int m_nServerNo, m_nServerGroupNo;
	int m_nServerGroup; // server의 번호(0:서버군이 없다, 1:서버1군, 2:서버2군)
	ServerMap m_ServerArray;
	ServerMap m_ServerGroupArray;
	CUdpSocket* m_pUdpSocket;

protected:
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

	bool OnStart() override;

private:
	std::unique_ptr<TimerThread> _gameTimeThread;
	std::unique_ptr<TimerThread> _smqHeartbeatThread;
	std::unique_ptr<TimerThread> _aliveTimeThread;
	std::unique_ptr<TimerThread> _marketBBSTimeThread;
	std::unique_ptr<TimerThread> _packetCheckThread;

	std::unique_ptr<ReadQueueThread> _readQueueThread;

	std::mutex _serialMutex;

	std::filesystem::path _mapDir;
	std::string _overrideMapDir;

	std::filesystem::path _questsDir;
	std::string _overrideQuestsDir;

	std::shared_ptr<spdlog::logger> _regionLogger;
	std::shared_ptr<spdlog::logger> _eventLogger;
};

} // namespace Ebenezer

#endif // SERVER_EBENEZER_EBENEZERAPP_H

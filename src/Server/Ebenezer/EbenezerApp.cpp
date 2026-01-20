#include "pch.h"
#include "EbenezerApp.h"
#include "EbenezerLogger.h"
#include "EbenezerReadQueueThread.h"
#include "OperationMessage.h"
#include "User.h"
#include "db_resources.h"

#include <argparse/argparse.hpp>
#include <FileIO/FileReader.h>

#include <shared/crc32.h>
#include <shared/DateTime.h>
#include <shared/lzf.h>
#include <shared/packets.h>
#include <shared/StringUtils.h>
#include <shared/TimerThread.h>

#include <db-library/ConnectionManager.h>
#include <db-library/RecordSetLoader_STLMap.h>
#include <db-library/RecordSetLoader_Vector.h>

#include <Ebenezer/binder/EbenezerBinder.h>

#include <fstream>

using namespace db;
using namespace std::chrono_literals;

namespace Ebenezer
{

constexpr int MAX_SMQ_SEND_QUEUE_RETRY_COUNT = 50;

constexpr int NUM_FLAG_VICTORY               = 4;
constexpr int AWARD_GOLD                     = 5000;

std::recursive_mutex g_region_mutex;

uint16_t g_increase_serial = 1;
bool g_serverdown_flag     = false;

EbenezerApp::EbenezerApp(EbenezerLogger& logger) :
	AppThread(logger), _aiSocketManager(SOCKET_BUFF_SIZE, SOCKET_BUFF_SIZE),
	m_LoggerSendQueue(MAX_SMQ_SEND_QUEUE_RETRY_COUNT),
	m_ItemLoggerSendQ(MAX_SMQ_SEND_QUEUE_RETRY_COUNT)
{
	// Ebenezer is the only server that had built in command line support, so we'll
	// default _enableTelnet to on.
	_enableTelnet     = true;
	_telnetPort       = 2324;

	m_nYear           = 0;
	m_nMonth          = 0;
	m_nDate           = 0;
	m_nHour           = 0;
	m_nMin            = 0;
	m_nWeather        = 0;
	m_nAmount         = 0;
	m_sPartyIndex     = 0;

	m_nCastleCapture  = 0;

	m_bKarusFlag      = 0;
	m_bElmoradFlag    = 0;

	m_byKarusOpenFlag = m_byElmoradOpenFlag = 0;
	m_byBanishFlag                          = 0;
	m_sBanishDelay                          = 0;

	m_sKarusDead                            = 0;
	m_sElmoradDead                          = 0;

	for (int i = 0; i < INVASION_MONUMENT_COUNT; i++)
	{
		_karusInvasionMonumentLastCapturedNation[i]   = 0;
		_elmoradInvasionMonumentLastCapturedNation[i] = 0;
	}

	m_bVictory            = 0;
	m_byOldVictory        = 0;
	m_byBattleSave        = 0;
	m_sKarusCount         = 0;
	m_sElmoradCount       = 0;

	m_nBattleZoneOpenWeek = m_nBattleZoneOpenHourStart = m_nBattleZoneOpenHourEnd = 0;

	m_byBattleOpen                                                                = NO_BATTLE;
	m_byOldBattleOpen                                                             = NO_BATTLE;
	m_bFirstServerFlag                                                            = false;
	m_bPointCheckFlag                                                             = false;

	m_nServerIndex                                                                = 0;
	m_nServerNo                                                                   = 0;
	m_nServerGroupNo                                                              = 0;
	m_nServerGroup                                                                = 0;
	m_iPacketCount                                                                = 0;
	m_iSendPacketCount                                                            = 0;
	m_iRecvPacketCount                                                            = 0;
	m_sDiscount                                                                   = 0;

	m_pUdpSocket                                                                  = nullptr;

	for (int h = 0; h < MAX_BBS_POST; h++)
	{
		m_sBuyID[h] = -1;
		memset(m_strBuyTitle[h], 0, sizeof(m_strBuyTitle[h]));
		memset(m_strBuyMessage[h], 0, sizeof(m_strBuyMessage[h]));
		m_iBuyPrice[h]     = 0;
		m_fBuyStartTime[h] = 0.0;

		m_sSellID[h]       = -1;
		memset(m_strSellTitle[h], 0, sizeof(m_strSellTitle[h]));
		memset(m_strSellMessage[h], 0, sizeof(m_strSellMessage[h]));
		m_iSellPrice[h]     = 0;
		m_fSellStartTime[h] = 0.0;
	}

	memset(m_ppNotice, 0, sizeof(m_ppNotice));

	m_bPermanentChatMode = false; // 비러머글 남는 공지 --;
	m_bPermanentChatFlag = false;
	memset(m_strPermanentChat, 0, sizeof(m_strPermanentChat));

	memset(m_strKarusCaptain, 0, sizeof(m_strKarusCaptain));
	memset(m_strElmoradCaptain, 0, sizeof(m_strElmoradCaptain));

	m_bySanta                    = 0; // 갓댐 산타!!! >.<

	_beefRoastVictoryType        = BEEF_ROAST_VICTORY_PENDING_RESTART_AFTER_VICTORY;

	_monsterChallengeActiveType  = 0;
	_monsterChallengeState       = 0;
	_monsterChallengePlayerCount = 0;

	ConnectionManager::Create();

	_gameTimeThread = std::make_unique<TimerThread>(
		6s, std::bind(&EbenezerApp::GameTimeTick, this));

	_smqHeartbeatThread = std::make_unique<TimerThread>(
		10s, std::bind(&EbenezerApp::SendSMQHeartbeat, this));

	_aliveTimeThread = std::make_unique<TimerThread>(
		34s, // NOTE: oddly specific time which they preserved in newer builds
		std::bind(&EbenezerApp::CheckAliveUser, this));

	_marketBBSTimeThread = std::make_unique<TimerThread>(
		5min, std::bind(&EbenezerApp::MarketBBSTimeCheck, this));

	_packetCheckThread = std::make_unique<TimerThread>(
		6min, std::bind(&EbenezerApp::WritePacketLog, this));

	_readQueueThread = std::make_unique<EbenezerReadQueueThread>();
}

EbenezerApp::~EbenezerApp()
{
	spdlog::info("EbenezerApp::~EbenezerApp: Shutting down, releasing resources.");

	// wait for all of these threads to be fully shut down.
	spdlog::info("EbenezerApp::~EbenezerApp: Waiting for worker threads to fully shut down.");

	if (_gameTimeThread != nullptr)
	{
		spdlog::info("EbenezerApp::~EbenezerApp: Shutting down game time thread...");

		_gameTimeThread->shutdown();

		spdlog::info("EbenezerApp::~EbenezerApp: game time thread stopped.");
	}

	if (_smqHeartbeatThread != nullptr)
	{
		spdlog::info(
			"EbenezerApp::~EbenezerApp: Shutting down shared memory queue heartbeat thread...");

		_smqHeartbeatThread->shutdown();

		spdlog::info("EbenezerApp::~EbenezerApp: shared memory queue heartbeat stopped.");
	}

	if (_aliveTimeThread != nullptr)
	{
		spdlog::info("EbenezerApp::~EbenezerApp: Shutting down alive time thread...");

		_aliveTimeThread->shutdown();

		spdlog::info("EbenezerApp::~EbenezerApp: alive time thread stopped.");
	}

	if (_marketBBSTimeThread != nullptr)
	{
		spdlog::info("EbenezerApp::~EbenezerApp: Shutting down market BBS time thread...");

		_marketBBSTimeThread->shutdown();

		spdlog::info("EbenezerApp::~EbenezerApp: market BBS time thread stopped.");
	}

	if (_packetCheckThread != nullptr)
	{
		spdlog::info("EbenezerApp::~EbenezerApp: Shutting down packet check thread...");

		_packetCheckThread->shutdown();

		spdlog::info("EbenezerApp::~EbenezerApp: packet check thread stopped.");
	}

	if (_readQueueThread != nullptr)
	{
		spdlog::info("EbenezerApp::~EbenezerApp: Shutting down ReadQueueThread...");

		_readQueueThread->shutdown();

		spdlog::info("EbenezerApp::~EbenezerApp: ReadQueueThread stopped.");
	}

	delete m_pUdpSocket;
	m_pUdpSocket = nullptr;

	_serverSocketManager.Shutdown();

	spdlog::info("EbenezerApp::~EbenezerApp: Ebenezer socket manager stopped.");

	// We don't manage these, these are managed by _aiSocketManager.
	// We should only empty the map.
	_aiSocketMap.clear();
	_aiSocketManager.Shutdown();

	spdlog::info("EbenezerApp::~EbenezerApp: AI socket manager stopped.");

	for (C3DMap* pMap : m_ZoneArray)
		delete pMap;
	m_ZoneArray.clear();

	for (model::LevelUp* pLevelUp : m_LevelUpTableArray)
		delete pLevelUp;
	m_LevelUpTableArray.clear();

	spdlog::info("EbenezerApp::~EbenezerApp: All resources safely released.");

	ConnectionManager::Destroy();
}

bool EbenezerApp::OnStart()
{
	srand(static_cast<uint32_t>(time(nullptr)));

	// Compress Init
	memset(m_CompBuf, 0, sizeof(m_CompBuf)); // 압축할 데이터를 모으는 버퍼
	m_iCompIndex             = 0;            // 압축할 데이터의 길이
	m_CompCount              = 0;            // 압축할 데이터의 개수

	m_sZoneCount             = 0;
	m_sSocketCount           = 0;
	m_sErrorSocketCount      = 0;
	m_KnightsManager.m_pMain = this;
	// sungyong 2002.05.23
	m_sSendSocket            = 0;
	m_bFirstServerFlag       = false;
	m_bServerCheckFlag       = false;
	m_sReSocketCount         = 0;
	m_fReConnectStart        = 0.0;
	// sungyong~ 2002.05.23

	_regionLogger            = spdlog::get(std::string(logger::EbenezerRegion));
	_eventLogger             = spdlog::get(std::string(logger::EbenezerEvent));

	_serverSocketManager.Init(MAX_USER, 4);
	_serverSocketManager.AllocateSockets<CUser>();

	_aiSocketManager.Init(CLIENT_SOCKSIZE, 1);
	_aiSocketManager.AllocateSockets<CAISocket>();

	_ZONE_SERVERINFO* pInfo = m_ServerArray.GetData(m_nServerNo);
	if (pInfo == nullptr)
	{
		spdlog::error("No Listen Port!!");
		return false;
	}

	if (!_serverSocketManager.Listen(pInfo->sPort))
	{
		spdlog::error("FAIL TO CREATE LISTEN STATE");
		return false;
	}

	spdlog::info("Listening on 0.0.0.0:{} - not accepting user connections yet", pInfo->sPort);

	if (!InitializeMMF())
	{
		spdlog::error("Main Shared Memory Initialize Fail");
		return false;
	}

	if (!m_LoggerSendQueue.OpenOrCreate(SMQ_LOGGERSEND))
	{
		spdlog::error("SMQ Send Shared Memory Initialize Fail");
		return false;
	}

	if (!m_LoggerRecvQueue.OpenOrCreate(SMQ_LOGGERRECV))
	{
		spdlog::error("SMQ Recv Shared Memory Initialize Fail");
		return false;
	}

	if (!m_ItemLoggerSendQ.OpenOrCreate(SMQ_ITEMLOGGER))
	{
		spdlog::error("SMQ ItemLog Shared Memory Initialize Fail");
		return false;
	}

	spdlog::info("EbenezerApp::OnInitDialog: loading ITEM table");
	if (!LoadItemTable())
	{
		spdlog::error("EbenezerApp::OnInitDialog: failed to cache ITEM table, closing");
		return false;
	}

	spdlog::info("EbenezerApp::OnInitDialog: loading MAGIC table");
	if (!LoadMagicTable())
	{
		spdlog::error("EbenezerApp::OnInitDialog: failed to cache MAGIC table, closing");
		return false;
	}

	spdlog::info("EbenezerApp::OnInitDialog: loading MAGIC_TYPE1 table");
	if (!LoadMagicType1())
	{
		spdlog::error("EbenezerApp::OnInitDialog: failed to cache MAGIC_TYPE1 table, closing");
		return false;
	}

	spdlog::info("EbenezerApp::OnInitDialog: loading MAGIC_TYPE2 table");
	if (!LoadMagicType2())
	{
		spdlog::error("EbenezerApp::OnInitDialog: failed to cache MAGIC_TYPE2 table, closing");
		return false;
	}

	spdlog::info("EbenezerApp::OnInitDialog: loading MAGIC_TYPE3 table");
	if (!LoadMagicType3())
	{
		spdlog::error("EbenezerApp::OnInitDialog: failed to cache MAGIC_TYPE3 table, closing");
		return false;
	}

	spdlog::info("EbenezerApp::OnInitDialog: loading MAGIC_TYPE4 table");
	if (!LoadMagicType4())
	{
		spdlog::error("EbenezerApp::OnInitDialog: failed to cache MAGIC_TYPE4 table, closing");
		return false;
	}

	spdlog::info("EbenezerApp::OnInitDialog: loading MAGIC_TYPE5 table");
	if (!LoadMagicType5())
	{
		spdlog::error("EbenezerApp::OnInitDialog: failed to cache MAGIC_TYPE5 table, closing");
		return false;
	}

	spdlog::info("EbenezerApp::OnInitDialog: loading MAGIC_TYPE7 table");
	if (!LoadMagicType7())
	{
		spdlog::error("EbenezerApp::OnInitDialog: failed to cache MAGIC_TYPE7 table, closing");
		return false;
	}

	spdlog::info("EbenezerApp::OnInitDialog: loading MAGIC_TYPE8 table");
	if (!LoadMagicType8())
	{
		spdlog::error("EbenezerApp::OnInitDialog: failed to cache MAGIC_TYPE8 table, closing");
		return false;
	}

	spdlog::info("EbenezerApp::OnInitDialog: loading COEFFICIENT table");
	if (!LoadCoefficientTable())
	{
		spdlog::error("EbenezerApp::OnInitDialog: failed to cache COEFFICIENT table, closing");
		return false;
	}

	spdlog::info("EbenezerApp::OnInitDialog: loading LEVEL_UP table");
	if (!LoadLevelUpTable())
	{
		spdlog::error("EbenezerApp::OnInitDialog: failed to cache LEVEL_UP table, closing");
		return false;
	}

	spdlog::info("EbenezerApp::OnInitDialog: loading KNIGHTS table");
	if (!LoadAllKnights())
	{
		spdlog::error("EbenezerApp::OnInitDialog: failed to cache KNIGHTS table, closing");
		return false;
	}

	spdlog::info("EbenezerApp::OnInitDialog: loading KNIGHTS_USER table");
	if (!LoadAllKnightsUserData())
	{
		spdlog::error("EbenezerApp::OnInitDialog: failed to cache KNIGHTS_USER table, closing");
		return false;
	}

	spdlog::info("EbenezerApp::OnInitDialog: loading KNIGHTS_SIEGE_WARFARE table");
	if (!LoadKnightsSiegeWarfareTable())
	{
		spdlog::error(
			"EbenezerApp::OnInitDialog: failed to cache KNIGHTS_SIEGE_WARFARE table, closing");
		return false;
	}

	spdlog::info("EbenezerApp::OnInitDialog: loading HOME table");
	if (!LoadHomeTable())
	{
		spdlog::error("EbenezerApp::OnInitDialog: failed to cache HOME table, closing");
		return false;
	}

	spdlog::info("EbenezerApp::OnInitDialog: loading START_POSITION table");
	if (!LoadStartPositionTable())
	{
		spdlog::error("EbenezerApp::OnInitDialog: failed to cache START_POSITION table, closing");
		return false;
	}

	spdlog::info("EbenezerApp::OnInitDialog: loading BATTLE table");
	if (!LoadBattleTable())
	{
		spdlog::error("EbenezerApp::OnInitDialog: failed to cache BATTLE table, closing");
		return false;
	}

	spdlog::info("EbenezerApp::OnInitDialog: loading SERVER_RESOURCE table");
	if (!LoadServerResourceTable())
	{
		spdlog::error("EbenezerApp::OnInitDialog: failed to cache SERVER_RESOURCE, closing");
		return false;
	}

	spdlog::info("EbenezerApp::OnInitDialog: loading EVENT_TRIGGER table");
	if (!LoadEventTriggerTable())
	{
		spdlog::error("EbenezerApp::OnInitDialog: failed to cache EVENT_TRIGGER, closing");
		return false;
	}

	spdlog::info("EbenezerApp::OnInitDialog: loading maps");
	if (!MapFileLoad())
	{
		spdlog::error("EbenezerApp::OnInitDialog: failed to load maps, closing");
		return false;
	}

	LoadNoticeData();

	m_pUdpSocket = new CUdpSocket(this);
	if (!m_pUdpSocket->CreateSocket())
	{
		spdlog::error("EbenezerApp::OnInitDialog: failed to create UDP socket");
		return false;
	}

	if (!AIServerConnect())
	{
		spdlog::error("EbenezerApp::OnInitDialog: failed to connect to the AIServer");
#ifndef _DEBUG
		return false;
#endif
	}

#ifdef _DEBUG
	// meant to be called when AI finishes connecting, to allow users to connect.
	// we allow user connections early in debug builds so we do not need to wait
	// for a full server load to log in
	UserAcceptThread();
#endif

	_gameTimeThread->start();
	_smqHeartbeatThread->start();
	_aliveTimeThread->start();
	_marketBBSTimeThread->start();
	_packetCheckThread->start();

	_readQueueThread->start();

	spdlog::info("EbenezerApp::OnInitDialog: successfully initialized");
	return true;
}

void EbenezerApp::UserAcceptThread()
{
	_serverSocketManager.StartAccept();
	spdlog::info("Accepting user connections");
}

std::shared_ptr<CUser> EbenezerApp::GetUserPtr(const char* userid, NameType type)
{
	int socketCount = GetUserSocketCount();

	if (type == NameType::Account)
	{
		for (int i = 0; i < socketCount; i++)
		{
			auto pUser = GetUserPtrUnchecked(i);
			if (pUser != nullptr && strnicmp(pUser->m_strAccountID, userid, MAX_ID_SIZE) == 0)
				return pUser;
		}
	}
	else // if (type == NameType::Character)
	{
		for (int i = 0; i < socketCount; i++)
		{
			auto pUser = GetUserPtrUnchecked(i);
			if (pUser != nullptr && strnicmp(pUser->m_pUserData->m_id, userid, MAX_ID_SIZE) == 0)
				return pUser;
		}
	}

	return nullptr;
}

// sungyong 2002.05.22
bool EbenezerApp::AIServerConnect()
{
	for (int i = 0; i < MAX_AI_SOCKET; i++)
	{
		if (!AISocketConnect(i, false))
			return false;
	}

	return true;
}

bool EbenezerApp::AISocketConnect(int zone, bool flag)
{
	std::shared_ptr<TcpClientSocket> tcpClientSocket;
	int sendIndex = 0;
	char pBuf[128] {};

	//if( m_nServerNo == 3 ) return false;
	int port = GetAIServerPort();
	if (port < 0)
	{
		spdlog::error(
			"EbenezerApp::AISocketConnect: Invalid port, unsupported m_nServerNo {} (zone {})",
			m_nServerNo, zone);

		return false;
	}

	std::unique_lock<std::mutex> lock(_aiSocketMutex);

	auto itr = _aiSocketMap.find(zone);
	if (itr != _aiSocketMap.end() && itr->second != nullptr)
	{
		// Already connected.
		if (itr->second->GetState() != CONNECTION_STATE_DISCONNECTED)
			return true;

		// Already have a socket, try to reconnect.
		tcpClientSocket = itr->second;
	}
	else
	{
		tcpClientSocket = _aiSocketManager.AcquireSocket();
		if (tcpClientSocket == nullptr)
		{
			spdlog::error("EbenezerApp::AISocketConnect: Failed to acquire new AI server socket "
						  "(zone {}) ({}:{})",
				zone, m_AIServerIP, port);

			return false;
		}

		_aiSocketMap[zone] = tcpClientSocket;
	}

	if (!tcpClientSocket->Connect(m_AIServerIP.c_str(), port))
	{
		spdlog::error(
			"EbenezerApp::AISocketConnect: Failed to connect to AI server (zone {}) ({}:{})", zone,
			m_AIServerIP, port);

		return false;
	}

	lock.unlock();

	SetByte(pBuf, AI_SERVER_CONNECT, sendIndex);
	SetByte(pBuf, zone, sendIndex);

	// 재접속
	if (flag)
		SetByte(pBuf, 1, sendIndex);
	// 처음 접속..
	else
		SetByte(pBuf, 0, sendIndex);

	tcpClientSocket->Send(pBuf, sendIndex);

	// 해야할일 :이 부분 처리.....
	//SendAllUserInfo();
	//m_sSocketCount = zone;

	spdlog::debug("EbenezerApp::AISocketConnect: connected to zone {}", zone);
	return true;
}
// ~sungyong 2002.05.22

int EbenezerApp::GetAIServerPort() const
{
	switch (m_nServerNo)
	{
		case SERVER_ZONE_KARUS:
			return AI_KARUS_SOCKET_PORT;

		case SERVER_ZONE_ELMORAD:
			return AI_ELMO_SOCKET_PORT;

		case SERVER_ZONE_BATTLE:
			return AI_BATTLE_SOCKET_PORT;

		default:
			return -1;
	}
}

void EbenezerApp::Send_All(char* pBuf, int len, CUser* pExceptUser, int nation)
{
	int socketCount = GetUserSocketCount();
	for (int i = 0; i < socketCount; i++)
	{
		auto pUser = GetUserPtrUnchecked(i);
		if (pUser == nullptr || pUser.get() == pExceptUser)
			continue;

		if (pUser->GetState() == CONNECTION_STATE_GAMESTART)
		{
			if (nation == 0 || nation == pUser->m_pUserData->m_bNation)
				pUser->Send(pBuf, len);
		}
	}
}

void EbenezerApp::Send_Region(
	char* pBuf, int len, int zone, int x, int z, CUser* pExceptUser, bool bDirect)
{
	C3DMap* pMap = GetMapByID(zone);
	if (pMap == nullptr)
		return;

	Send_UnitRegion(pMap, pBuf, len, x, z, pExceptUser, bDirect);
	Send_UnitRegion(pMap, pBuf, len, x - 1, z - 1, pExceptUser, bDirect); // NW
	Send_UnitRegion(pMap, pBuf, len, x, z - 1, pExceptUser, bDirect);     // N
	Send_UnitRegion(pMap, pBuf, len, x + 1, z - 1, pExceptUser, bDirect); // NE
	Send_UnitRegion(pMap, pBuf, len, x - 1, z, pExceptUser, bDirect);     // W
	Send_UnitRegion(pMap, pBuf, len, x + 1, z, pExceptUser, bDirect);     // E
	Send_UnitRegion(pMap, pBuf, len, x - 1, z + 1, pExceptUser, bDirect); // SW
	Send_UnitRegion(pMap, pBuf, len, x, z + 1, pExceptUser, bDirect);     // S
	Send_UnitRegion(pMap, pBuf, len, x + 1, z + 1, pExceptUser, bDirect); // SE
}

void EbenezerApp::Send_UnitRegion(
	C3DMap* pMap, char* pBuf, int len, int x, int z, CUser* pExceptUser, bool bDirect)
{
	if (pMap == nullptr)
		return;

	if (x < 0 || z < 0 || x > pMap->GetXRegionMax() || z > pMap->GetZRegionMax())
		return;

	std::lock_guard<std::recursive_mutex> lock(g_region_mutex);

	for (const auto& [_, pUid] : pMap->m_ppRegion[x][z].m_RegionUserArray)
	{
		int uid    = *pUid;

		auto pUser = GetUserPtr(uid);
		if (pUser.get() == pExceptUser)
			continue;

		if (pUser != nullptr && (pUser->GetState() == CONNECTION_STATE_GAMESTART))
		{
			if (bDirect)
				pUser->Send(pBuf, len);
			else
				pUser->RegionPacketAdd(pBuf, len);
		}
	}
}

void EbenezerApp::Send_NearRegion(char* pBuf, int len, int zone, int region_x, int region_z,
	float curx, float curz, CUser* pExceptUser)
{
	C3DMap* pMap = GetMapByID(zone);
	if (pMap == nullptr)
		return;

	int left_border = region_x * VIEW_DISTANCE, top_border = region_z * VIEW_DISTANCE;

	Send_FilterUnitRegion(pMap, pBuf, len, region_x, region_z, curx, curz, pExceptUser);

	// RIGHT
	if (((curx - left_border) > (VIEW_DISTANCE / 2.0f)))
	{
		// BOTTOM
		if (((curz - top_border) > (VIEW_DISTANCE / 2.0f)))
		{
			Send_FilterUnitRegion(pMap, pBuf, len, region_x + 1, region_z, curx, curz, pExceptUser);
			Send_FilterUnitRegion(pMap, pBuf, len, region_x, region_z + 1, curx, curz, pExceptUser);
			Send_FilterUnitRegion(
				pMap, pBuf, len, region_x + 1, region_z + 1, curx, curz, pExceptUser);
		}
		// TOP
		else
		{
			Send_FilterUnitRegion(pMap, pBuf, len, region_x + 1, region_z, curx, curz, pExceptUser);
			Send_FilterUnitRegion(pMap, pBuf, len, region_x, region_z - 1, curx, curz, pExceptUser);
			Send_FilterUnitRegion(
				pMap, pBuf, len, region_x + 1, region_z - 1, curx, curz, pExceptUser);
		}
	}
	// LEFT
	else
	{
		// BOTTOM
		if (((curz - top_border) > (VIEW_DISTANCE / 2.0f)))
		{
			Send_FilterUnitRegion(pMap, pBuf, len, region_x - 1, region_z, curx, curz, pExceptUser);
			Send_FilterUnitRegion(pMap, pBuf, len, region_x, region_z + 1, curx, curz, pExceptUser);
			Send_FilterUnitRegion(
				pMap, pBuf, len, region_x - 1, region_z + 1, curx, curz, pExceptUser);
		}
		// TOP
		else
		{
			Send_FilterUnitRegion(pMap, pBuf, len, region_x - 1, region_z, curx, curz, pExceptUser);
			Send_FilterUnitRegion(pMap, pBuf, len, region_x, region_z - 1, curx, curz, pExceptUser);
			Send_FilterUnitRegion(
				pMap, pBuf, len, region_x - 1, region_z - 1, curx, curz, pExceptUser);
		}
	}
}

void EbenezerApp::Send_FilterUnitRegion(
	C3DMap* pMap, char* pBuf, int len, int x, int z, float ref_x, float ref_z, CUser* pExceptUser)
{
	if (pMap == nullptr)
		return;

	if (x < 0 || z < 0 || x > pMap->GetXRegionMax() || z > pMap->GetZRegionMax())
		return;

	std::lock_guard<std::recursive_mutex> lock(g_region_mutex);

	for (const auto& [_, pUid] : pMap->m_ppRegion[x][z].m_RegionUserArray)
	{
		int uid    = *pUid;

		auto pUser = GetUserPtr(uid);
		if (pUser.get() == pExceptUser)
			continue;

		if (pUser != nullptr && pUser->GetState() == CONNECTION_STATE_GAMESTART)
		{
			double fDist = sqrt(pow((pUser->m_pUserData->m_curx - ref_x), 2)
								+ pow((pUser->m_pUserData->m_curz - ref_z), 2));
			if (fDist < 32)
				pUser->RegionPacketAdd(pBuf, len);
		}
	}
}

void EbenezerApp::Send_PartyMember(int party, char* pBuf, int len)
{
	if (party < 0)
		return;

	_PARTY_GROUP* pParty = m_PartyMap.GetData(party);
	if (pParty == nullptr)
		return;

	for (int i = 0; i < 8; i++)
	{
		auto pUser = GetUserPtr(pParty->uid[i]);
		if (pUser != nullptr)
			pUser->Send(pBuf, len);
	}
}

void EbenezerApp::Send_KnightsMember(int index, char* pBuf, int len, int zone)
{
	if (index <= 0)
		return;

	CKnights* pKnights = m_KnightsMap.GetData(index);
	if (pKnights == nullptr)
		return;

	int socketCount = GetUserSocketCount();
	for (int i = 0; i < socketCount; i++)
	{
		auto pUser = GetUserPtrUnchecked(i);
		if (pUser == nullptr)
			continue;

		if (pUser->m_pUserData->m_bKnights != index)
			continue;

		if (zone != 100 && pUser->m_pUserData->m_bZone != zone)
			continue;

		pUser->Send(pBuf, len);
	}
}

// sungyong 2002.05.22
void EbenezerApp::Send_AIServer(int /*zone*/, char* pBuf, int len)
{
	for (int i = 0; i < MAX_AI_SOCKET; i++)
	{
		std::unique_lock<std::mutex> lock(_aiSocketMutex);

		auto itr = _aiSocketMap.find(i);
		if (itr == _aiSocketMap.end())
			continue;

		auto tcpSocket = itr->second;
		if (tcpSocket == nullptr)
		{
			m_sSendSocket++;
			if (m_sSendSocket >= MAX_AI_SOCKET)
				m_sSendSocket = 0;

			continue;
		}

		if (i == m_sSendSocket)
		{
			lock.unlock();

			int send_size = tcpSocket->Send(pBuf, len);
			// int old_send_socket = m_sSendSocket;
			m_sSendSocket++;
			if (m_sSendSocket >= MAX_AI_SOCKET)
				m_sSendSocket = 0;
			if (send_size == 0)
				continue;

			//TRACE(_T(" <--- Send_AIServer : length = %d, socket = %d \n"), send_size, old_send_socket);
			return;
		}
	}
}
// ~sungyong 2002.05.22

bool EbenezerApp::InitializeMMF()
{
	int socketCount   = GetUserSocketCount();
	uint32_t filesize = socketCount * ALLOCATED_USER_DATA_BLOCK;

	char* memory      = m_UserDataBlock.OpenOrCreate("KNIGHT_DB", filesize);
	if (memory == nullptr)
		return false;

	spdlog::info("EbenezerApp::InitializeMMF: shared memory created successfully");

	memset(memory, 0, filesize);

	for (int i = 0; i < socketCount; i++)
	{
		auto pUser = _serverSocketManager.GetInactiveUserUnchecked(i);
		if (pUser == nullptr)
		{
			spdlog::error("EbenezerApp::InitializeMMF: invalid user pointer for userId={}", i);
			return false;
		}

		pUser->m_pUserData = reinterpret_cast<_USER_DATA*>(
			memory + static_cast<ptrdiff_t>(i * ALLOCATED_USER_DATA_BLOCK));
	}

	return true;
}

bool EbenezerApp::MapFileLoad()
{
	using ModelType = model::ZoneInfo;

	bool loaded     = false;

	recordset_loader::Base<ModelType> loader;
	loader.SetProcessFetchCallback(
		[&](db::ModelRecordSet<ModelType>& recordset)
		{
			m_ZoneArray.reserve(20);

			do
			{
				ModelType row {};
				recordset.get_ref(row);

				std::filesystem::path mapPath = _mapDir / row.Name;

				// NOTE: spdlog is a C++11 library that doesn't support std::filesystem or std::u8string
				// This just ensures the path is always explicitly UTF-8 in a cross-platform way.
				std::u8string filenameUtf8    = mapPath.u8string();
				std::string filename(filenameUtf8.begin(), filenameUtf8.end());

				FileReader file;
				if (!file.OpenExisting(mapPath))
				{
					spdlog::error("EbenezerApp::MapFileLoad: File Open Fail - {}", filename);
					return;
				}

				C3DMap* pMap        = new C3DMap();

				pMap->m_nServerNo   = row.ServerId;
				pMap->m_nZoneNumber = row.ZoneId;
				pMap->m_fInitX      = (float) (row.InitX / 100.0);
				pMap->m_fInitZ      = (float) (row.InitZ / 100.0);
				pMap->m_fInitY      = (float) (row.InitY / 100.0);
				pMap->m_bType       = row.Type;

				if (!pMap->LoadMap(file))
				{
					spdlog::error("EbenezerApp::MapFileLoad: Map Load Fail - {}", filename);
					delete pMap;
					return;
				}

				file.Close();

				m_ZoneArray.push_back(pMap);

				// 스트립트를 읽어 들인다.
				EVENT* pEvent = new EVENT;
				if (!pEvent->LoadEvent(row.ZoneId, _questsDir))
				{
					delete pEvent;
					continue;
				}

				if (!m_EventMap.PutData(pEvent->m_Zone, pEvent))
					delete pEvent;
			}
			while (recordset.next());

			loaded = true;
		});

	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("EbenezerApp::MapFileLoad: load failed - {}", loader.GetError().Message);
		return false;
	}

	return loaded;
}

bool EbenezerApp::LoadItemTable()
{
	recordset_loader::STLMap loader(m_ItemTableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("EbenezerApp::LoadItemTable: load failed - {}", loader.GetError().Message);
		return false;
	}

	return true;
}

bool EbenezerApp::LoadMagicTable()
{
	recordset_loader::STLMap loader(m_MagicTableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("EbenezerApp::LoadMagicTable: load failed - {}", loader.GetError().Message);
		return false;
	}

	return true;
}

bool EbenezerApp::LoadMagicType1()
{
	recordset_loader::STLMap loader(m_MagicType1TableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("EbenezerApp::LoadMagicType1: load failed - {}", loader.GetError().Message);
		return false;
	}

	return true;
}

bool EbenezerApp::LoadMagicType2()
{
	recordset_loader::STLMap loader(m_MagicType2TableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("EbenezerApp::LoadMagicType2: load failed - {}", loader.GetError().Message);
		return false;
	}

	return true;
}

bool EbenezerApp::LoadMagicType3()
{
	recordset_loader::STLMap loader(m_MagicType3TableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("EbenezerApp::LoadMagicType3: load failed - {}", loader.GetError().Message);
		return false;
	}

	return true;
}

bool EbenezerApp::LoadMagicType4()
{
	recordset_loader::STLMap loader(m_MagicType4TableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("EbenezerApp::LoadMagicType4: load failed - {}", loader.GetError().Message);
		return false;
	}

	return true;
}

bool EbenezerApp::LoadMagicType5()
{
	recordset_loader::STLMap loader(m_MagicType5TableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("EbenezerApp::LoadMagicType5: load failed - {}", loader.GetError().Message);
		return false;
	}

	return true;
}

bool EbenezerApp::LoadMagicType7()
{
	recordset_loader::STLMap loader(m_MagicType7TableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("EbenezerApp::LoadMagicType7: load failed - {}", loader.GetError().Message);
		return false;
	}

	return true;
}

bool EbenezerApp::LoadMagicType8()
{
	recordset_loader::STLMap loader(m_MagicType8TableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("EbenezerApp::LoadMagicType8: load failed - {}", loader.GetError().Message);
		return false;
	}

	return true;
}

bool EbenezerApp::LoadCoefficientTable()
{
	recordset_loader::STLMap loader(m_CoefficientTableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error(
			"EbenezerApp::LoadCoefficientTable: load failed - {}", loader.GetError().Message);
		return false;
	}

	return true;
}

bool EbenezerApp::LoadLevelUpTable()
{
	recordset_loader::Vector<model::LevelUp> loader(m_LevelUpTableArray);
	if (!loader.Load_ForbidEmpty(true))
	{
		spdlog::error("EbenezerApp::LoadLevelUpTable: load failed - {}", loader.GetError().Message);
		return false;
	}

	return true;
}

void EbenezerApp::SetupCommandLineArgParser(argparse::ArgumentParser& parser)
{
	AppThread::SetupCommandLineArgParser(parser);

	parser.add_argument("--map-dir")
		.help("location of directory containing server map data files (.SMDs)")
		.store_into(_overrideMapDir);

	parser.add_argument("--quests-dir")
		.help("location of directory containing the Ebenezer server specific quest scripts (.EVTs)")
		.store_into(_overrideQuestsDir);
}

bool EbenezerApp::ProcessCommandLineArgs(const argparse::ArgumentParser& parser)
{
	if (!AppThread::ProcessCommandLineArgs(parser))
		return false;

	std::error_code ec;
	if (!_overrideMapDir.empty() && !std::filesystem::exists(_overrideMapDir, ec))
	{
		spdlog::error("Supplied map directory (--map-dir) doesn't exist or is inaccessible: {}",
			_overrideMapDir);
		return false;
	}

	if (!_overrideQuestsDir.empty() && !std::filesystem::exists(_overrideQuestsDir, ec))
	{
		spdlog::error(
			"Supplied quests directory (--quests-dir) doesn't exist or is inaccessible: {}",
			_overrideQuestsDir);
		return false;
	}

	return true;
}

std::filesystem::path EbenezerApp::ConfigPath() const
{
	return "gameserver.ini";
}

bool EbenezerApp::LoadConfig(CIni& iniFile)
{
	int serverCount = 0, sgroup_count = 0;
	std::string key;

	// Load paths from config
	std::string mapDir    = iniFile.GetString("PATH", "MAP_DIR", "");
	std::string questsDir = iniFile.GetString("PATH", "QUESTS_DIR", "");

	std::error_code ec;

	// Map directory supplied from command-line.
	// Replace it in the config -- but only if it's not explicitly been set already.
	// We should always use the map directory passed from command-line over the INI.
	if (!_overrideMapDir.empty())
	{
		if (mapDir.empty())
			iniFile.SetString("PATH", "MAP_DIR", _overrideMapDir);

		_mapDir = _overrideMapDir;
	}
	// No command-line override is present, but it is configured in the INI.
	// We should use that.
	else if (!mapDir.empty())
	{
		_mapDir = mapDir;

		if (!std::filesystem::exists(_mapDir, ec))
		{
			spdlog::error("Configured map directory doesn't exist or is inaccessible: {}", mapDir);
			return false;
		}
	}
	// Fallback to the default (don't save this).
	else
	{
		_mapDir = DEFAULT_MAP_DIR;
	}

	// Resolve the path to strip the relative references (to be nice).
	if (std::filesystem::exists(_mapDir, ec))
		_mapDir = std::filesystem::canonical(_mapDir, ec);

	// Quests (.EVT) directory supplied from command-line.
	// Replace it in the config -- but only if it's not explicitly been set already.
	// We should always use the map directory passed from command-line over the INI.
	if (!_overrideQuestsDir.empty())
	{
		if (questsDir.empty())
			iniFile.SetString("PATH", "QUESTS_DIR", _overrideQuestsDir);

		_questsDir = _overrideQuestsDir;
	}
	// No command-line override is present, but it is configured in the INI.
	// We should use that.
	else if (!questsDir.empty())
	{
		_questsDir = questsDir;

		if (!std::filesystem::exists(_questsDir, ec))
		{
			spdlog::error(
				"Configured quests directory doesn't exist or is inaccessible: {}", questsDir);
			return false;
		}
	}
	// Fallback to the default (don't save this).
	else
	{
		_questsDir = DEFAULT_QUESTS_DIR;
	}

	// Resolve the path to strip the relative references (to be nice).
	if (std::filesystem::exists(_questsDir, ec))
		_questsDir = std::filesystem::canonical(_questsDir);

	m_nYear                    = iniFile.GetInt("TIMER", "YEAR", 1);
	m_nMonth                   = iniFile.GetInt("TIMER", "MONTH", 1);
	m_nDate                    = iniFile.GetInt("TIMER", "DATE", 1);
	m_nHour                    = iniFile.GetInt("TIMER", "HOUR", 1);
	m_nWeather                 = iniFile.GetInt("TIMER", "WEATHER", 1);

	//	m_nBattleZoneOpenWeek  = iniFile.GetInt("BATTLE", "WEEK", 3);
	m_nBattleZoneOpenWeek      = iniFile.GetInt("BATTLE", "WEEK", 5);
	m_nBattleZoneOpenHourStart = iniFile.GetInt("BATTLE", "START_TIME", 20);
	m_nBattleZoneOpenHourEnd   = iniFile.GetInt("BATTLE", "END_TIME", 0);

	std::string datasourceName = iniFile.GetString("ODBC", "GAME_DSN", "KN_online");
	std::string datasourceUser = iniFile.GetString("ODBC", "GAME_UID", "knight");
	std::string datasourcePass = iniFile.GetString("ODBC", "GAME_PWD", "knight");

	ConnectionManager::SetDatasourceConfig(
		modelUtil::DbType::GAME, datasourceName, datasourceUser, datasourcePass);

	m_AIServerIP     = iniFile.GetString("AI_SERVER", "IP", "127.0.0.1");

	// NOTE: officially this is required to be explicitly set, so it defaults to 0 and fails.
	m_nServerIndex   = iniFile.GetInt("SG_INFO", "SERVER_INDEX", 1);

	m_nCastleCapture = iniFile.GetInt("CASTLE", "NATION", 1);
	m_nServerNo      = iniFile.GetInt("ZONE_INFO", "MY_INFO", 1);
	m_nServerGroup   = iniFile.GetInt("ZONE_INFO", "SERVER_NUM", 0);
	serverCount      = iniFile.GetInt("ZONE_INFO", "SERVER_COUNT", 1);
	if (serverCount < 1)
	{
		spdlog::error("EbenezerApp::LoadConfig: invalid SERVER_COUNT={}, must be 1+", serverCount);
		return false;
	}

	for (int i = 0; i < serverCount; i++)
	{
		_ZONE_SERVERINFO* pInfo = new _ZONE_SERVERINFO;

		key                     = fmt::format("SERVER_{:02}", i);
		pInfo->sServerNo        = iniFile.GetInt("ZONE_INFO", key, 1);

		key                     = fmt::format("SERVER_IP_{:02}", i);
		pInfo->strServerIP      = iniFile.GetString("ZONE_INFO", key, "127.0.0.1");

		pInfo->sPort            = _LISTEN_PORT + pInfo->sServerNo;

		m_ServerArray.PutData(pInfo->sServerNo, pInfo);
	}

	if (m_nServerGroup != 0)
	{
		m_nServerGroupNo = iniFile.GetInt("SG_INFO", "GMY_INFO", 1);
		sgroup_count     = iniFile.GetInt("SG_INFO", "GSERVER_COUNT", 1);
		if (sgroup_count < 1)
		{
			spdlog::error(
				"EbenezerApp::LoadConfig: Invalid server group count={}, must be 1+", sgroup_count);
			return false;
		}

		for (int i = 0; i < sgroup_count; i++)
		{
			_ZONE_SERVERINFO* pInfo = new _ZONE_SERVERINFO;

			key                     = fmt::format("GSERVER_{:02}", i);
			pInfo->sServerNo        = iniFile.GetInt("SG_INFO", key, 1);

			key                     = fmt::format("GSERVER_IP_{:02}", i);
			pInfo->strServerIP      = iniFile.GetString("SG_INFO", key, "127.0.0.1");

			pInfo->sPort            = _LISTEN_PORT + pInfo->sServerNo;

			m_ServerGroupArray.PutData(pInfo->sServerNo, pInfo);
		}
	}

	return true;
}

void EbenezerApp::UpdateGameTime()
{
	bool bKnights = false;

	m_nMin++;

	BattleZoneOpenTimer(); // Check if it's time for the BattleZone to open or end.

	if (m_nMin == 60)
	{
		m_nHour++;
		m_nMin = 0;

		UpdateWeather();
		SetGameTime();

		//  갓댐 산타!! >.<
		if (m_bySanta != 0)
			FlySanta();
		//
	}

	if (m_nHour == 24)
	{
		m_nDate++;
		m_nHour  = 0;
		bKnights = true;
	}

	if (m_nDate == 31)
	{
		m_nMonth++;
		m_nDate = 1;
	}

	if (m_nMonth == 13)
	{
		m_nYear++;
		m_nMonth = 1;
	}

	// ai status check packet...
	m_sErrorSocketCount++;

	int sendIndex = 0;
	char pSendBuf[256] {};
	//SetByte(pSendBuf, AG_CHECK_ALIVE_REQ, sendIndex);
	//Send_AIServer(1000, pSendBuf, sendIndex);

	// 시간과 날씨 정보를 보낸다..
	memset(pSendBuf, 0, sizeof(pSendBuf));
	sendIndex = 0;
	SetByte(pSendBuf, AG_TIME_WEATHER, sendIndex);
	SetShort(pSendBuf, m_nYear, sendIndex); // time info
	SetShort(pSendBuf, m_nMonth, sendIndex);
	SetShort(pSendBuf, m_nDate, sendIndex);
	SetShort(pSendBuf, m_nHour, sendIndex);
	SetShort(pSendBuf, m_nMin, sendIndex);
	SetByte(pSendBuf, (uint8_t) m_nWeather, sendIndex); // weather info
	SetShort(pSendBuf, m_nAmount, sendIndex);
	Send_AIServer(1000, pSendBuf, sendIndex);

	if (bKnights)
	{
		memset(pSendBuf, 0, sizeof(pSendBuf));
		sendIndex = 0;
		SetByte(pSendBuf, WIZ_KNIGHTS_PROCESS, sendIndex);
		SetByte(pSendBuf, DB_KNIGHTS_ALLLIST_REQ, sendIndex);
		SetByte(pSendBuf, m_nServerNo, sendIndex);
		m_LoggerSendQueue.PutData(pSendBuf, sendIndex);
	}
}

void EbenezerApp::UpdateWeather()
{
	int weather = 0, result = 0, sendIndex = 0;
	char sendBuffer[256] {};

	result = myrand(0, 100);

	//	if (result < 5)
	if (result < 2)
		weather = WEATHER_SNOW;
	//	else if (result < 15)
	else if (result < 7)
		weather = WEATHER_RAIN;
	else
		weather = WEATHER_FINE;

	m_nAmount = myrand(0, 100);

	// WEATHER_FINE 일때 m_nAmount 는 안개 정도 표시
	if (weather == WEATHER_FINE)
	{
		if (m_nAmount > 70)
			m_nAmount = m_nAmount / 2;
		else
			m_nAmount = 0;
	}

	m_nWeather = weather;

	SetByte(sendBuffer, WIZ_WEATHER, sendIndex);
	SetByte(sendBuffer, (uint8_t) m_nWeather, sendIndex);
	SetShort(sendBuffer, m_nAmount, sendIndex);
	Send_All(sendBuffer, sendIndex);
}

void EbenezerApp::SetGameTime()
{
	CIni& iniFile = IniFile();
	iniFile.SetInt("TIMER", "YEAR", m_nYear);
	iniFile.SetInt("TIMER", "MONTH", m_nMonth);
	iniFile.SetInt("TIMER", "DATE", m_nDate);
	iniFile.SetInt("TIMER", "HOUR", m_nHour);
	iniFile.SetInt("TIMER", "WEATHER", m_nWeather);
	iniFile.Save();
}

void EbenezerApp::UserInOutForMe(CUser* pSendUser)
{
	int sendIndex = 0, buff_index = 0, t_count = 0, prevIndex = 0;
	C3DMap* pMap = nullptr;
	int region_x = -1, region_z = -1;
	char buff[16384] {}, sendBuffer[49152] {};

	if (pSendUser == nullptr)
		return;

	pMap = GetMapByIndex(pSendUser->m_iZoneIndex);
	if (pMap == nullptr)
		return;

	sendIndex  = 3;                    // packet command 와 user_count 를 나중에 셋팅한다...
	region_x   = pSendUser->m_RegionX;
	region_z   = pSendUser->m_RegionZ; // CENTER
	buff_index = GetRegionUserIn(pMap, region_x, region_z, buff, t_count);
	SetString(sendBuffer, buff, buff_index, sendIndex);

	memset(buff, 0, sizeof(buff));
	region_x   = pSendUser->m_RegionX - 1;
	region_z   = pSendUser->m_RegionZ - 1; // NORTH WEST
	buff_index = GetRegionUserIn(pMap, region_x, region_z, buff, t_count);
	SetString(sendBuffer, buff, buff_index, sendIndex);

	memset(buff, 0, sizeof(buff));
	region_x   = pSendUser->m_RegionX;
	region_z   = pSendUser->m_RegionZ - 1; // NORTH
	buff_index = GetRegionUserIn(pMap, region_x, region_z, buff, t_count);
	prevIndex  = buff_index + sendIndex;

	if (prevIndex >= 49152)
	{
		spdlog::error("EbenezerApp::UserInOutForMe: buffer overflow [prevIndex={}, line={}]",
			prevIndex, __LINE__);
		return;
	}

	SetString(sendBuffer, buff, buff_index, sendIndex);

	memset(buff, 0, sizeof(buff));
	region_x   = pSendUser->m_RegionX + 1;
	region_z   = pSendUser->m_RegionZ - 1; // NORTH EAST
	buff_index = GetRegionUserIn(pMap, region_x, region_z, buff, t_count);
	prevIndex  = buff_index + sendIndex;

	if (prevIndex >= 49152)
	{
		spdlog::error("EbenezerApp::UserInOutForMe: buffer overflow [prevIndex={}, line={}]",
			prevIndex, __LINE__);
		return;
	}

	SetString(sendBuffer, buff, buff_index, sendIndex);

	memset(buff, 0, sizeof(buff));
	region_x   = pSendUser->m_RegionX - 1;
	region_z   = pSendUser->m_RegionZ; // WEST
	buff_index = GetRegionUserIn(pMap, region_x, region_z, buff, t_count);
	prevIndex  = buff_index + sendIndex;

	if (prevIndex >= 49152)
	{
		spdlog::error("EbenezerApp::UserInOutForMe: buffer overflow [prevIndex={}, line={}]",
			prevIndex, __LINE__);
		return;
	}

	SetString(sendBuffer, buff, buff_index, sendIndex);

	memset(buff, 0, sizeof(buff));
	region_x   = pSendUser->m_RegionX + 1;
	region_z   = pSendUser->m_RegionZ; // EAST
	buff_index = GetRegionUserIn(pMap, region_x, region_z, buff, t_count);
	prevIndex  = buff_index + sendIndex;
	if (prevIndex >= 49152)
	{
		spdlog::error("EbenezerApp::UserInOutForMe: buffer overflow [prevIndex={}, line={}]",
			prevIndex, __LINE__);
		return;
	}

	SetString(sendBuffer, buff, buff_index, sendIndex);

	memset(buff, 0, sizeof(buff));
	region_x   = pSendUser->m_RegionX - 1;
	region_z   = pSendUser->m_RegionZ + 1; // SOUTH WEST
	buff_index = GetRegionUserIn(pMap, region_x, region_z, buff, t_count);
	prevIndex  = buff_index + sendIndex;
	if (prevIndex >= 49152)
	{
		spdlog::error("EbenezerApp::UserInOutForMe: buffer overflow [prevIndex={}, line={}]",
			prevIndex, __LINE__);
		return;
	}

	SetString(sendBuffer, buff, buff_index, sendIndex);

	memset(buff, 0, sizeof(buff));
	region_x   = pSendUser->m_RegionX;
	region_z   = pSendUser->m_RegionZ + 1; // SOUTH
	buff_index = GetRegionUserIn(pMap, region_x, region_z, buff, t_count);
	prevIndex  = buff_index + sendIndex;

	if (prevIndex >= 49152)
	{
		spdlog::error("EbenezerApp::UserInOutForMe: buffer overflow [prevIndex={}, line={}]",
			prevIndex, __LINE__);
		return;
	}

	SetString(sendBuffer, buff, buff_index, sendIndex);
	memset(buff, 0, sizeof(buff));
	region_x   = pSendUser->m_RegionX + 1;
	region_z   = pSendUser->m_RegionZ + 1; // SOUTH EAST
	buff_index = GetRegionUserIn(pMap, region_x, region_z, buff, t_count);
	prevIndex  = buff_index + sendIndex;
	if (prevIndex >= 49152)
	{
		spdlog::error("EbenezerApp::UserInOutForMe: buffer overflow [prevIndex={}, line={}]",
			prevIndex, __LINE__);
		return;
	}

	SetString(sendBuffer, buff, buff_index, sendIndex);

	int temp_index = 0;
	SetByte(sendBuffer, WIZ_REQ_USERIN, temp_index);
	SetShort(sendBuffer, t_count, temp_index);

	pSendUser->SendCompressingPacket(sendBuffer, sendIndex);
}

void EbenezerApp::RegionUserInOutForMe(CUser* pSendUser)
{
	int buff_index = 0;
	C3DMap* pMap   = nullptr;
	int region_x = -1, region_z = -1, userCount = 0, uid_sendindex = 0;
	char uid_buff[2048] {}, sendBuffer[16384] {};

	if (pSendUser == nullptr)
		return;

	pMap = GetMapByIndex(pSendUser->m_iZoneIndex);
	if (pMap == nullptr)
		return;

	uid_sendindex = 3;                    // packet command 와 user_count 는 나중에 셋팅한다...

	region_x      = pSendUser->m_RegionX;
	region_z      = pSendUser->m_RegionZ; // CENTER
	buff_index    = GetRegionUserList(pMap, region_x, region_z, uid_buff, userCount);
	SetString(sendBuffer, uid_buff, buff_index, uid_sendindex);
	memset(uid_buff, 0, sizeof(uid_buff));

	region_x   = pSendUser->m_RegionX - 1;
	region_z   = pSendUser->m_RegionZ - 1; // NORTH WEST
	buff_index = GetRegionUserList(pMap, region_x, region_z, uid_buff, userCount);
	SetString(sendBuffer, uid_buff, buff_index, uid_sendindex);
	memset(uid_buff, 0, sizeof(uid_buff));

	region_x   = pSendUser->m_RegionX;
	region_z   = pSendUser->m_RegionZ - 1; // NORTH
	buff_index = GetRegionUserList(pMap, region_x, region_z, uid_buff, userCount);
	SetString(sendBuffer, uid_buff, buff_index, uid_sendindex);
	memset(uid_buff, 0, sizeof(uid_buff));

	region_x   = pSendUser->m_RegionX + 1;
	region_z   = pSendUser->m_RegionZ - 1; // NORTH EAST
	buff_index = GetRegionUserList(pMap, region_x, region_z, uid_buff, userCount);
	SetString(sendBuffer, uid_buff, buff_index, uid_sendindex);
	memset(uid_buff, 0, sizeof(uid_buff));

	region_x   = pSendUser->m_RegionX - 1;
	region_z   = pSendUser->m_RegionZ; // WEST
	buff_index = GetRegionUserList(pMap, region_x, region_z, uid_buff, userCount);
	SetString(sendBuffer, uid_buff, buff_index, uid_sendindex);
	memset(uid_buff, 0, sizeof(uid_buff));

	region_x   = pSendUser->m_RegionX + 1;
	region_z   = pSendUser->m_RegionZ; // EAST
	buff_index = GetRegionUserList(pMap, region_x, region_z, uid_buff, userCount);
	SetString(sendBuffer, uid_buff, buff_index, uid_sendindex);
	memset(uid_buff, 0, sizeof(uid_buff));

	region_x   = pSendUser->m_RegionX - 1;
	region_z   = pSendUser->m_RegionZ + 1; // SOUTH WEST
	buff_index = GetRegionUserList(pMap, region_x, region_z, uid_buff, userCount);
	SetString(sendBuffer, uid_buff, buff_index, uid_sendindex);
	memset(uid_buff, 0, sizeof(uid_buff));

	region_x   = pSendUser->m_RegionX;
	region_z   = pSendUser->m_RegionZ + 1; // SOUTH
	buff_index = GetRegionUserList(pMap, region_x, region_z, uid_buff, userCount);
	SetString(sendBuffer, uid_buff, buff_index, uid_sendindex);
	memset(uid_buff, 0, sizeof(uid_buff));

	region_x   = pSendUser->m_RegionX + 1;
	region_z   = pSendUser->m_RegionZ + 1; // SOUTH EAST
	buff_index = GetRegionUserList(pMap, region_x, region_z, uid_buff, userCount);
	SetString(sendBuffer, uid_buff, buff_index, uid_sendindex);

	int temp_index = 0;
	SetByte(sendBuffer, WIZ_REGIONCHANGE, temp_index);
	SetShort(sendBuffer, userCount, temp_index);

	pSendUser->Send(sendBuffer, uid_sendindex);

	if (userCount > 500)
		spdlog::debug("EbenezerApp::RegionUserInOutForMe: userCount={}", userCount);
}

int EbenezerApp::GetRegionUserIn(C3DMap* pMap, int region_x, int region_z, char* buff, int& t_count)
{
	if (pMap == nullptr)
		return 0;

	if (region_x < 0 || region_z < 0 || region_x > pMap->GetXRegionMax()
		|| region_z > pMap->GetZRegionMax())
		return 0;

	int buff_index = 0;

	std::lock_guard<std::recursive_mutex> lock(g_region_mutex);

	for (const auto& [_, pUid] : pMap->m_ppRegion[region_x][region_z].m_RegionUserArray)
	{
		int uid    = *pUid;

		auto pUser = GetUserPtr(uid);
		if (pUser == nullptr)
			continue;

		if (pUser->m_RegionX != region_x || pUser->m_RegionZ != region_z)
			continue;

		if (pUser->GetState() != CONNECTION_STATE_GAMESTART)
			continue;

		SetShort(buff, pUser->GetSocketID(), buff_index);
		pUser->GetUserInfo(buff, buff_index);

		++t_count;
	}

	return buff_index;
}

int EbenezerApp::GetRegionUserList(
	C3DMap* pMap, int region_x, int region_z, char* buff, int& t_count)
{
	if (pMap == nullptr)
		return 0;

	if (region_x < 0 || region_z < 0 || region_x > pMap->GetXRegionMax()
		|| region_z > pMap->GetZRegionMax())
		return 0;

	int buff_index = 0;

	std::lock_guard<std::recursive_mutex> lock(g_region_mutex);

	for (const auto& [_, pUid] : pMap->m_ppRegion[region_x][region_z].m_RegionUserArray)
	{
		int uid    = *pUid;

		auto pUser = GetUserPtr(uid);
		if (pUser != nullptr && pUser->GetState() == CONNECTION_STATE_GAMESTART)
		{
			SetShort(buff, pUser->GetSocketID(), buff_index);
			t_count++;
		}
	}

	return buff_index;
}

void EbenezerApp::NpcInOutForMe(CUser* pSendUser)
{
	int sendIndex = 0, buff_index = 0, t_count = 0;
	C3DMap* pMap = nullptr;
	int region_x = -1, region_z = -1;
	char buff[8192] {}, sendBuffer[32768] {};

	if (pSendUser == nullptr)
		return;

	pMap = GetMapByIndex(pSendUser->m_iZoneIndex);
	if (pMap == nullptr)
		return;

	sendIndex  = 3;                    // packet command 와 user_count 를 나중에 셋팅한다...
	region_x   = pSendUser->m_RegionX;
	region_z   = pSendUser->m_RegionZ; // CENTER
	buff_index = GetRegionNpcIn(pMap, region_x, region_z, buff, t_count);
	SetString(sendBuffer, buff, buff_index, sendIndex);

	memset(buff, 0, sizeof(buff));
	region_x   = pSendUser->m_RegionX - 1;
	region_z   = pSendUser->m_RegionZ - 1; // NORTH WEST
	buff_index = GetRegionNpcIn(pMap, region_x, region_z, buff, t_count);
	SetString(sendBuffer, buff, buff_index, sendIndex);

	memset(buff, 0, sizeof(buff));
	region_x   = pSendUser->m_RegionX;
	region_z   = pSendUser->m_RegionZ - 1; // NORTH
	buff_index = GetRegionNpcIn(pMap, region_x, region_z, buff, t_count);
	SetString(sendBuffer, buff, buff_index, sendIndex);

	memset(buff, 0, sizeof(buff));
	region_x   = pSendUser->m_RegionX + 1;
	region_z   = pSendUser->m_RegionZ - 1; // NORTH EAST
	buff_index = GetRegionNpcIn(pMap, region_x, region_z, buff, t_count);
	SetString(sendBuffer, buff, buff_index, sendIndex);

	memset(buff, 0, sizeof(buff));
	region_x   = pSendUser->m_RegionX - 1;
	region_z   = pSendUser->m_RegionZ; // WEST
	buff_index = GetRegionNpcIn(pMap, region_x, region_z, buff, t_count);
	SetString(sendBuffer, buff, buff_index, sendIndex);

	memset(buff, 0, sizeof(buff));
	region_x   = pSendUser->m_RegionX + 1;
	region_z   = pSendUser->m_RegionZ; // EAST
	buff_index = GetRegionNpcIn(pMap, region_x, region_z, buff, t_count);
	SetString(sendBuffer, buff, buff_index, sendIndex);

	memset(buff, 0, sizeof(buff));
	region_x   = pSendUser->m_RegionX - 1;
	region_z   = pSendUser->m_RegionZ + 1; // SOUTH WEST
	buff_index = GetRegionNpcIn(pMap, region_x, region_z, buff, t_count);
	SetString(sendBuffer, buff, buff_index, sendIndex);

	memset(buff, 0, sizeof(buff));
	region_x   = pSendUser->m_RegionX;
	region_z   = pSendUser->m_RegionZ + 1; // SOUTH
	buff_index = GetRegionNpcIn(pMap, region_x, region_z, buff, t_count);
	SetString(sendBuffer, buff, buff_index, sendIndex);

	memset(buff, 0, sizeof(buff));
	region_x   = pSendUser->m_RegionX + 1;
	region_z   = pSendUser->m_RegionZ + 1; // SOUTH EAST
	buff_index = GetRegionNpcIn(pMap, region_x, region_z, buff, t_count);
	SetString(sendBuffer, buff, buff_index, sendIndex);

	int temp_index = 0;
	SetByte(sendBuffer, WIZ_REQ_NPCIN, temp_index);
	SetShort(sendBuffer, t_count, temp_index);

	pSendUser->SendCompressingPacket(sendBuffer, sendIndex);
}

int EbenezerApp::GetRegionNpcIn(C3DMap* pMap, int region_x, int region_z, char* buff, int& t_count)
{
	// 포인터 참조하면 안됨
	if (!m_bPointCheckFlag)
		return 0;

	if (pMap == nullptr)
		return 0;

	if (region_x < 0 || region_z < 0 || region_x > pMap->GetXRegionMax()
		|| region_z > pMap->GetZRegionMax())
		return 0;

	int buff_index = 0;

	std::lock_guard<std::recursive_mutex> lock(g_region_mutex);

	for (const auto& [_, pNid] : pMap->m_ppRegion[region_x][region_z].m_RegionNpcArray)
	{
		int nid = *pNid;
		if (nid < 0)
			continue;

		CNpc* pNpc = m_NpcMap.GetData(nid);
		if (pNpc == nullptr)
			continue;

		if (pNpc->m_sRegion_X != region_x || pNpc->m_sRegion_Z != region_z)
			continue;

		SetShort(buff, pNpc->m_sNid, buff_index);
		pNpc->GetNpcInfo(buff, buff_index);

		t_count++;
	}

	return buff_index;
}

void EbenezerApp::RegionNpcInfoForMe(CUser* pSendUser, int nType)
{
	int buff_index = 0;
	C3DMap* pMap   = nullptr;
	int region_x = -1, region_z = -1, npcCount = 0, nid_sendindex = 0;
	char nid_buff[1024] {}, sendBuffer[8192] {};

	if (pSendUser == nullptr)
		return;

	pMap = GetMapByIndex(pSendUser->m_iZoneIndex);
	if (pMap == nullptr)
		return;

	nid_sendindex = 3; // packet command 와 user_count 는 나중에 셋팅한다...

	// test
	if (nType == 1)
	{
		_regionLogger->info("RegionNpcInfoForMe start: charId={} x={} z={}",
			pSendUser->m_pUserData->m_id, pSendUser->m_RegionX, pSendUser->m_RegionZ);
	}

	region_x   = pSendUser->m_RegionX;
	region_z   = pSendUser->m_RegionZ; // CENTER
	buff_index = GetRegionNpcList(pMap, region_x, region_z, nid_buff, npcCount, nType);
	SetString(sendBuffer, nid_buff, buff_index, nid_sendindex);

	memset(nid_buff, 0, sizeof(nid_buff));
	region_x   = pSendUser->m_RegionX - 1;
	region_z   = pSendUser->m_RegionZ - 1; // NORTH WEST
	buff_index = GetRegionNpcList(pMap, region_x, region_z, nid_buff, npcCount, nType);
	SetString(sendBuffer, nid_buff, buff_index, nid_sendindex);

	memset(nid_buff, 0, sizeof(nid_buff));
	region_x   = pSendUser->m_RegionX;
	region_z   = pSendUser->m_RegionZ - 1; // NORTH
	buff_index = GetRegionNpcList(pMap, region_x, region_z, nid_buff, npcCount, nType);
	SetString(sendBuffer, nid_buff, buff_index, nid_sendindex);

	memset(nid_buff, 0, sizeof(nid_buff));
	region_x   = pSendUser->m_RegionX + 1;
	region_z   = pSendUser->m_RegionZ - 1; // NORTH EAST
	buff_index = GetRegionNpcList(pMap, region_x, region_z, nid_buff, npcCount, nType);
	SetString(sendBuffer, nid_buff, buff_index, nid_sendindex);

	memset(nid_buff, 0, sizeof(nid_buff));
	region_x   = pSendUser->m_RegionX - 1;
	region_z   = pSendUser->m_RegionZ; // WEST
	buff_index = GetRegionNpcList(pMap, region_x, region_z, nid_buff, npcCount, nType);
	SetString(sendBuffer, nid_buff, buff_index, nid_sendindex);

	memset(nid_buff, 0, sizeof(nid_buff));
	region_x   = pSendUser->m_RegionX + 1;
	region_z   = pSendUser->m_RegionZ; // EAST
	buff_index = GetRegionNpcList(pMap, region_x, region_z, nid_buff, npcCount, nType);
	SetString(sendBuffer, nid_buff, buff_index, nid_sendindex);

	memset(nid_buff, 0, sizeof(nid_buff));
	region_x   = pSendUser->m_RegionX - 1;
	region_z   = pSendUser->m_RegionZ + 1; // SOUTH WEST
	buff_index = GetRegionNpcList(pMap, region_x, region_z, nid_buff, npcCount, nType);
	SetString(sendBuffer, nid_buff, buff_index, nid_sendindex);

	memset(nid_buff, 0, sizeof(nid_buff));
	region_x   = pSendUser->m_RegionX;
	region_z   = pSendUser->m_RegionZ + 1; // SOUTH
	buff_index = GetRegionNpcList(pMap, region_x, region_z, nid_buff, npcCount, nType);
	SetString(sendBuffer, nid_buff, buff_index, nid_sendindex);

	memset(nid_buff, 0, sizeof(nid_buff));
	region_x   = pSendUser->m_RegionX + 1;
	region_z   = pSendUser->m_RegionZ + 1; // SOUTH EAST
	buff_index = GetRegionNpcList(pMap, region_x, region_z, nid_buff, npcCount, nType);
	SetString(sendBuffer, nid_buff, buff_index, nid_sendindex);

	int temp_index = 0;

	// test
	if (nType == 1)
	{
		SetByte(sendBuffer, WIZ_TEST_PACKET, temp_index);
		_regionLogger->info("RegionNpcInfoForMe end: charId={} x={} z={} count={}",
			pSendUser->m_pUserData->m_id, pSendUser->m_RegionX, pSendUser->m_RegionZ, npcCount);
	}
	else
	{
		SetByte(sendBuffer, WIZ_NPC_REGION, temp_index);
	}

	SetShort(sendBuffer, npcCount, temp_index);

	pSendUser->Send(sendBuffer, nid_sendindex);

	if (npcCount > 500)
		spdlog::debug("EbenezerApp::RegionNpcInfoForMe: npcCount={}", npcCount);
}

int EbenezerApp::GetRegionNpcList(
	C3DMap* pMap, int region_x, int region_z, char* nid_buff, int& t_count, int nType)
{
	// 포인터 참조하면 안됨
	if (!m_bPointCheckFlag)
		return 0;

	if (pMap == nullptr)
		return 0;

	if (region_x < 0 || region_z < 0 || region_x > pMap->GetXRegionMax()
		|| region_z > pMap->GetZRegionMax())
		return 0;

	int buff_index = 0;

	if (nType == 1)
		_regionLogger->info("GetRegionNpcList: x={} z={}", region_x, region_z);

	std::lock_guard<std::recursive_mutex> lock(g_region_mutex);

	for (const auto& [_, pNid] : pMap->m_ppRegion[region_x][region_z].m_RegionNpcArray)
	{
		int npcId = *pNid;
		if (npcId < 0)
			continue;

		CNpc* pNpc = m_NpcMap.GetData(npcId);
		//if( pNpc && (pNpc->m_NpcState == NPC_LIVE ) ) {  // 수정할 것,,
		if (pNpc != nullptr)
		{
			SetShort(nid_buff, pNpc->m_sNid, buff_index);
			t_count++;

			if (nType == 1)
				_regionLogger->info("GetRegionNpcList: serial={}", pNpc->m_sNid);
		}
		else if (nType == 1)
		{
			_regionLogger->error("GetRegionNpcList: not found: npcId={}", npcId);
		}
	}

	return buff_index;
}

int EbenezerApp::GetZoneIndex(int zonenumber) const
{
	for (size_t i = 0; i < m_ZoneArray.size(); i++)
	{
		C3DMap* pMap = m_ZoneArray[i];
		if (pMap != nullptr && zonenumber == pMap->m_nZoneNumber)
			return static_cast<int>(i);
	}

	return -1;
}

bool EbenezerApp::HandleCommand(const std::string& command)
{
	if (AppThread::HandleCommand(command))
		return true;

	OperationMessage opMessage(this, nullptr);
	if (opMessage.Process(command))
		return true;

	std::string finalstr;

	int sendIndex = 0;
	char sendBuffer[1024] {};
	SetByte(sendBuffer, WIZ_CHAT, sendIndex);

	if (m_bPermanentChatFlag)
	{
		finalstr = fmt::format("- {} -", command);
		if (finalstr.length() >= sizeof(m_strPermanentChat))
		{
			spdlog::warn("Supplied permanent chat notice too long: {}", finalstr);
			return false;
		}

		SetByte(sendBuffer, PERMANENT_CHAT, sendIndex);
		strcpy_safe(m_strPermanentChat, finalstr);
		m_bPermanentChatFlag = false;
	}
	else
	{
		finalstr = fmt::format_db_resource(IDP_ANNOUNCEMENT, command);
		SetByte(sendBuffer, PUBLIC_CHAT, sendIndex);
	}

	SetByte(sendBuffer, 0x01, sendIndex); // nation
	SetShort(sendBuffer, -1, sendIndex);  // sid
	SetByte(sendBuffer, 0, sendIndex);    // sender name length
	SetString2(sendBuffer, finalstr, sendIndex);
	Send_All(sendBuffer, sendIndex);

	sendIndex = 0;
	memset(sendBuffer, 0, 1024);
	SetByte(sendBuffer, STS_CHAT, sendIndex);
	SetString2(sendBuffer, finalstr, sendIndex);

	for (const auto& [_, pInfo] : m_ServerArray)
	{
		if (pInfo != nullptr && pInfo->sServerNo != m_nServerNo)
			m_pUdpSocket->SendUDPPacket(pInfo->strServerIP, sendBuffer, sendIndex);
	}

	return true;
}

bool EbenezerApp::LoadNoticeData()
{
	std::filesystem::path NoticePath = "Notice.txt";
	std::string line;
	int count = 0;

	std::ifstream file(NoticePath, std::ios::in);
	if (!file)
	{
		spdlog::warn("EbenezerApp::LoadNoticeData: failed to open Notice.txt");
		return false;
	}

	bool loadedNotice = true;
	while (std::getline(file, line))
	{
		if (count > 19)
		{
			spdlog::error("EbenezerApp::LoadNoticeData: notice count overflow [count={}]", count);
			loadedNotice = false;
			break;
		}

		if (strcpy_safe(m_ppNotice[count], line))
		{
			spdlog::error("EbenezerApp::LoadNoticeData: notice line {} is too long", count + 1);
			loadedNotice = false;
			break;
		}

		count++;
	}

	file.close();

	return loadedNotice;
}

void EbenezerApp::SendAllUserInfo()
{
	int sendIndex = 0;
	char sendBuffer[2048] {};

	SetByte(sendBuffer, AG_SERVER_INFO, sendIndex);
	SetByte(sendBuffer, SERVER_INFO_START, sendIndex);
	Send_AIServer(1000, sendBuffer, sendIndex);

	int count = 0;
	sendIndex = 2;
	memset(sendBuffer, 0, sizeof(sendBuffer));
	int send_count  = 0;
	int send_tot    = 0;
	int tot         = 20;

	int socketCount = GetUserSocketCount();
	for (int i = 0; i < socketCount; i++)
	{
		auto pUser = GetUserPtrUnchecked(i);
		if (pUser != nullptr)
		{
			pUser->SendUserInfo(sendBuffer, sendIndex);
			count++;
			if (count == tot)
			{
				SetByte(sendBuffer, AG_USER_INFO_ALL, send_count);
				SetByte(sendBuffer, (uint8_t) count, send_count);
				m_CompCount++;
				memset(m_CompBuf, 0, sizeof(m_CompBuf));
				memcpy(m_CompBuf, sendBuffer, sendIndex);
				m_iCompIndex = sendIndex;
				SendCompressedData();
				sendIndex  = 2;
				send_count = 0;
				count      = 0;
				send_tot++;
				spdlog::trace(
					"EbenezerApp::SendAllUserInfo: send_count={} count={}", send_tot, count);
				memset(sendBuffer, 0, sizeof(sendBuffer));
				//Sleep(320);
			}
		}
	}

	if (count != 0 && count < (tot - 1))
	{
		send_count = 0;
		SetByte(sendBuffer, AG_USER_INFO_ALL, send_count);
		SetByte(sendBuffer, (uint8_t) count, send_count);
		Send_AIServer(1000, sendBuffer, sendIndex);
		send_tot++;
		spdlog::trace("EbenezerApp::SendAllUserInfo: send_count={} count={}", send_tot, count);
		//Sleep(1);
	}

	// 파티에 대한 정보도 보내도록 한다....
	std::unique_lock<std::recursive_mutex> lock(g_region_mutex);

	for (int i = 0; i < m_PartyMap.GetSize(); i++)
	{
		_PARTY_GROUP* pParty = m_PartyMap.GetData(i);
		if (pParty == nullptr)
			continue;

		sendIndex = 0;
		memset(sendBuffer, 0, sizeof(sendBuffer));
		SetByte(sendBuffer, AG_PARTY_INFO_ALL, sendIndex);
		SetShort(sendBuffer, i, sendIndex); // 파티 번호
		//if( i == pParty->wIndex )
		for (int j = 0; j < 8; j++)
		{
			SetShort(sendBuffer, pParty->uid[j], sendIndex); // 유저 번호
			//SetShort(sendBuffer, pParty->sHp[j], sendIndex );				// HP
			//SetByte(sendBuffer, pParty->bLevel[j], sendIndex );				// Level
			//SetShort(sendBuffer, pParty->sClass[j], sendIndex );			// Class
		}

		Send_AIServer(1000, sendBuffer, sendIndex);
	}
	lock.unlock();

	sendIndex = 0;
	memset(sendBuffer, 0, sizeof(sendBuffer));
	SetByte(sendBuffer, AG_SERVER_INFO, sendIndex);
	SetByte(sendBuffer, SERVER_INFO_END, sendIndex);
	Send_AIServer(1000, sendBuffer, sendIndex);

	spdlog::trace("EbenezerApp::SendAllUserInfo: completed");
}

void EbenezerApp::SendCompressedData()
{
	if (m_CompCount <= 0 || m_iCompIndex <= 0)
	{
		m_CompCount  = 0;
		m_iCompIndex = 0;
		return;
	}

	char sendBuffer[32000] {};
	uint8_t comp_buff[32000] {};
	int sendIndex              = 0;
	unsigned int comp_data_len = 0;
	uint32_t crc_value         = 0;

	comp_data_len = lzf_compress(m_CompBuf, m_iCompIndex, comp_buff, sizeof(comp_buff));

	assert(comp_data_len != 0 && comp_data_len <= sizeof(comp_buff));

	if (comp_data_len == 0 || comp_data_len > sizeof(comp_buff))
	{
		spdlog::error("EbenezerApp::SendCompressedData: Failed to compress AI packet");
		return;
	}

	crc_value = crc32(reinterpret_cast<uint8_t*>(m_CompBuf), m_iCompIndex);

	SetByte(sendBuffer, AG_COMPRESSED_DATA, sendIndex);
	SetShort(sendBuffer, (int16_t) comp_data_len, sendIndex);
	SetShort(sendBuffer, (int16_t) m_iCompIndex, sendIndex);
	SetDWORD(sendBuffer, crc_value, sendIndex);
	SetShort(sendBuffer, (int16_t) m_CompCount, sendIndex);
	SetString(sendBuffer, reinterpret_cast<const char*>(comp_buff), comp_data_len, sendIndex);

	Send_AIServer(1000, sendBuffer, sendIndex);

	m_CompCount  = 0;
	m_iCompIndex = 0;
}

// sungyong 2002. 05. 23
void EbenezerApp::DeleteAllNpcList(int /*flag*/)
{
	if (!m_bServerCheckFlag)
		return;

	if (m_bPointCheckFlag)
	{
		m_bPointCheckFlag = false;
		spdlog::error("EbenezerApp::DeleteAllNpcList: pointCheckFlag set");
		return;
	}

	spdlog::debug("EbenezerApp::DeleteAllNpcList: start");

	// region Npc Array Delete
	for (C3DMap* pMap : m_ZoneArray)
	{
		if (pMap == nullptr)
			continue;

		for (int i = 0; i < pMap->GetXRegionMax(); i++)
		{
			for (int j = 0; j < pMap->GetZRegionMax(); j++)
				pMap->m_ppRegion[i][j].m_RegionNpcArray.DeleteAllData();
		}
	}

	m_NpcMap.DeleteAllData();
	m_bServerCheckFlag = false;

	spdlog::debug("EbenezerApp::DeleteAllNpcList: end");
}
// ~sungyong 2002. 05. 23

void EbenezerApp::KillUser(const char* strbuff)
{
	if (strlen(strbuff) <= 0 || strlen(strbuff) > MAX_ID_SIZE)
		return;

	auto pUser = GetUserPtr(strbuff, NameType::Character);
	if (pUser != nullptr)
		pUser->Close();
}

CNpc* EbenezerApp::GetNpcPtr(int sid, int cur_zone)
{
	// 포인터 참조하면 안됨
	if (!m_bPointCheckFlag)
		return nullptr;

	for (const auto& [_, pNpc] : m_NpcMap)
	{
		if (pNpc == nullptr)
			continue;

		if (pNpc->m_sCurZone != cur_zone)
			continue;

		if (pNpc->m_sPid == sid)
			return pNpc;
	}

	return nullptr;
}

CKnights* EbenezerApp::GetKnightsPtr(int16_t clanId)
{
	if (clanId <= 0)
		return nullptr;

	return m_KnightsMap.GetData(clanId);
}

void EbenezerApp::WithdrawUserOut()
{
	int socketCount = GetUserSocketCount();
	for (int i = 0; i < socketCount; i++)
	{
		auto pUser = GetUserPtrUnchecked(i);
		if (pUser != nullptr && pUser->m_pUserData->m_bZone == pUser->m_pUserData->m_bNation)
		{
			C3DMap* pMap = GetMapByID(pUser->m_pUserData->m_bNation);
			if (pMap != nullptr)
				pUser->ZoneChange(pMap->m_nZoneNumber, pMap->m_fInitX, pMap->m_fInitZ);
		}
	}
}

void EbenezerApp::AliveUserCheck()
{
	double currentTime = TimeGet();

	int socketCount    = GetUserSocketCount();
	for (int i = 0; i < socketCount; i++)
	{
		auto pUser = GetUserPtrUnchecked(i);
		if (pUser == nullptr)
			continue;

		if (pUser->GetState() != CONNECTION_STATE_GAMESTART)
			continue;

		/*
		if ((currentTime - pUser->m_fHPLastTime) > 300)
			pUser->Close();
*/
		for (int k = 0; k < MAX_TYPE3_REPEAT; k++)
		{
			if ((currentTime - pUser->m_fHPLastTime[k]) > 300)
			{
				pUser->Close();
				break;
			}
		}
	}
}

/////// BATTLEZONE RELATED by Yookozuna 2002.6.18 /////////////////
void EbenezerApp::BattleZoneOpenTimer()
{
	// DateTime cur = DateTime::GetNow();

	// sungyong modify
	// int nWeek = cur.GetDayOfWeek();
	// int nTime = cur.GetHour();
	char sendBuffer[128] {};
	int sendIndex = 0, loser_nation = 0;

	/*	if( m_byBattleOpen == NO_BATTLE )	{	// When Battlezone is closed, open it!
		if( nWeek == m_nBattleZoneOpenWeek && nTime == m_nBattleZoneOpenHourStart )	{	// 수요일, 20시에 전쟁존 open
			TRACE(_T("전쟁 자동 시작 - week=%d, time=%d\n"), nWeek, nTime);
			BattleZoneOpen(BATTLEZONE_OPEN);
//			KickOutZoneUsers(ZONE_FRONTIER);	// Kick out users in frontier zone.
		}
	}
	else {	  // When Battlezone is open, close it!
		if( nWeek == (m_nBattleZoneOpenWeek+1) && nTime == m_nBattleZoneOpenHourEnd )	{	// 목요일, 0시에 전쟁존 close
			TRACE(_T("전쟁 자동 종료 - week=%d, time=%d\n"), nWeek, nTime);
			m_byBanishFlag = 1;
		}
	}	*/

	if (m_byBattleOpen == NATION_BATTLE)
		BattleZoneCurrentUsers();

	if (m_byBanishFlag == 1)
	{
		if (m_sBanishDelay == 0)
		{
			m_byBattleOpen      = NO_BATTLE;
			m_byKarusOpenFlag   = 0; // 카루스 땅으로 넘어갈 수 없도록
			m_byElmoradOpenFlag = 0; // 엘모 땅으로 넘어갈 수 없도록

			memset(m_strKarusCaptain, 0, sizeof(m_strKarusCaptain));
			memset(m_strElmoradCaptain, 0, sizeof(m_strElmoradCaptain));

			// original: 전쟁 종료 0단계
			spdlog::debug("EbenezerApp::BattleZoneOpenTimer: war ended, stage 0");

			if (m_nServerNo == SERVER_ZONE_KARUS)
			{
				memset(sendBuffer, 0, sizeof(sendBuffer));
				sendIndex = 0;
				SetByte(sendBuffer, UDP_BATTLE_EVENT_PACKET, sendIndex);
				SetByte(sendBuffer, BATTLE_EVENT_KILL_USER, sendIndex);
				SetByte(sendBuffer, 1, sendIndex); // karus의 정보 전송
				SetShort(sendBuffer, m_sKarusDead, sendIndex);
				SetShort(sendBuffer, m_sElmoradDead, sendIndex);
				Send_UDP_All(sendBuffer, sendIndex);
			}
		}

		m_sBanishDelay++;

		if (m_sBanishDelay == 3)
		{
			// 눈싸움 전쟁
			if (m_byOldBattleOpen == SNOW_BATTLE)
			{
				if (m_sKarusDead > m_sElmoradDead)
				{
					m_bVictory   = NATION_ELMORAD;
					loser_nation = NATION_KARUS;
				}
				else if (m_sKarusDead < m_sElmoradDead)
				{
					m_bVictory   = NATION_KARUS;
					loser_nation = NATION_ELMORAD;
				}
				else if (m_sKarusDead == m_sElmoradDead)
				{
					m_bVictory = 0;
				}
			}

			if (m_bVictory == 0)
			{
				BattleZoneOpen(BATTLEZONE_CLOSE);
			}
			else if (m_bVictory)
			{
				if (m_bVictory == NATION_KARUS)
					loser_nation = NATION_ELMORAD;
				else if (m_bVictory == NATION_ELMORAD)
					loser_nation = NATION_KARUS;

				Announcement(DECLARE_WINNER, m_bVictory);
				Announcement(DECLARE_LOSER, loser_nation);
			}
			spdlog::debug(
				"EbenezerApp::BattleZoneOpenTimer: War ended, stage 1: m_bVictory={}", m_bVictory);
		}
		else if (m_sBanishDelay == 8)
		{
			Announcement(DECLARE_BAN);
		}
		else if (m_sBanishDelay == 10)
		{
			spdlog::debug("EbenezerApp::BattleZoneOpenTimer: War ended, stage 2: All users return "
						  "to their own nation");
			BanishLosers();
		}
		else if (m_sBanishDelay == 20)
		{
			// original: 전쟁 종료 3단계 - 초기화 해주세여
			spdlog::debug(
				"EbenezerApp::BattleZoneOpenTimer: War ended, stage 3: resetting battlezone");
			SetByte(sendBuffer, AG_BATTLE_EVENT, sendIndex);
			SetByte(sendBuffer, BATTLE_EVENT_OPEN, sendIndex);
			SetByte(sendBuffer, BATTLEZONE_CLOSE, sendIndex);
			Send_AIServer(1000, sendBuffer, sendIndex);
			ResetBattleZone();
		}
	}

	// ~
}

void EbenezerApp::BattleZoneOpen(int nType)
{
	int sendIndex = 0;
	char sendBuffer[1024] {};

	// Open battlezone.
	if (nType == BATTLEZONE_OPEN)
	{
		m_byBattleOpen    = NATION_BATTLE;
		m_byOldBattleOpen = NATION_BATTLE;
	}
	// Open snow battlezone.
	else if (nType == SNOW_BATTLEZONE_OPEN)
	{
		m_byBattleOpen    = SNOW_BATTLE;
		m_byOldBattleOpen = SNOW_BATTLE;
	}
	// battle close
	else if (nType == BATTLEZONE_CLOSE)
	{
		m_byBattleOpen = NO_BATTLE;
		Announcement(BATTLEZONE_CLOSE);
	}
	else
	{
		return;
	}

	Announcement(nType); // Send an announcement out that the battlezone is open/closed.
						 //
	KickOutZoneUsers(ZONE_FRONTIER);
	//
	memset(sendBuffer, 0, sizeof(sendBuffer));
	SetByte(sendBuffer, AG_BATTLE_EVENT, sendIndex); // Send packet to AI server.
	SetByte(sendBuffer, BATTLE_EVENT_OPEN, sendIndex);
	SetByte(sendBuffer, nType, sendIndex);
	Send_AIServer(1000, sendBuffer, sendIndex);
}

void EbenezerApp::BattleZoneVictoryCheck()
{
	// WINNER DECLARATION PROCEDURE !!!
	if (m_bKarusFlag >= NUM_FLAG_VICTORY)
		m_bVictory = NATION_KARUS;
	else if (m_bElmoradFlag >= NUM_FLAG_VICTORY)
		m_bVictory = NATION_ELMORAD;
	else
		return;

	Announcement(DECLARE_WINNER);

	// GOLD DISTRIBUTION PROCEDURE FOR WINNERS !!!
	int socketCount = GetUserSocketCount();
	for (int i = 0; i < socketCount; i++)
	{
		auto pTUser = GetUserPtrUnchecked(i);
		if (pTUser == nullptr)
			continue;

		if (pTUser->m_pUserData->m_bNation == m_bVictory
			// Zone Check!
			&& pTUser->m_pUserData->m_bZone == pTUser->m_pUserData->m_bNation)
			pTUser->m_pUserData->m_iGold += AWARD_GOLD; // Target is in the area.
	}
}

void EbenezerApp::BanishLosers()
{
	// EVACUATION PROCEDURE FOR LOSERS !!!
	int socketCount = GetUserSocketCount();
	for (int i = 0; i < socketCount; i++)
	{
		auto pTUser = GetUserPtrUnchecked(i);
		if (pTUser == nullptr)
			continue;

		if (pTUser->m_pUserData->m_bFame == KNIGHTS_DUTY_CAPTAIN)
		{
			pTUser->m_pUserData->m_bFame = KNIGHTS_DUTY_CHIEF;

			char sendBuffer[256] {};
			int sendIndex = 0;
			SetByte(sendBuffer, WIZ_AUTHORITY_CHANGE, sendIndex);
			SetByte(sendBuffer, COMMAND_AUTHORITY, sendIndex);
			SetShort(sendBuffer, pTUser->GetSocketID(), sendIndex);
			SetByte(sendBuffer, pTUser->m_pUserData->m_bFame, sendIndex);
			pTUser->Send(sendBuffer, sendIndex);
		}

		if (pTUser->m_pUserData->m_bZone != pTUser->m_pUserData->m_bNation)
			pTUser->KickOutZoneUser(true);
	}
}

void EbenezerApp::ResetBattleZone()
{
	m_bVictory          = 0;
	m_byBanishFlag      = 0;
	m_sBanishDelay      = 0;
	m_bKarusFlag        = 0;
	m_bElmoradFlag      = 0;
	m_byKarusOpenFlag   = 0;
	m_byElmoradOpenFlag = 0;
	m_byBattleOpen      = NO_BATTLE;
	m_byOldBattleOpen   = NO_BATTLE;
	m_sKarusDead        = 0;
	m_sElmoradDead      = 0;
	m_byBattleSave      = 0;
	m_sKarusCount       = 0;
	m_sElmoradCount     = 0;
	// REMEMBER TO MAKE ALL FLAGS AND LEVERS NEUTRAL AGAIN!!!!!!!!!!
}

void EbenezerApp::Announcement(uint8_t type, int nation, int chat_type)
{
	int sendIndex = 0;
	char sendBuffer[1024] {};

	std::string chatstr;

	switch (type)
	{
		case BATTLEZONE_OPEN:
		case SNOW_BATTLEZONE_OPEN:
			chatstr = fmt::format_db_resource(IDP_BATTLEZONE_OPEN);
			break;

		case DECLARE_WINNER:
			if (m_bVictory == NATION_KARUS)
				chatstr = fmt::format_db_resource(IDP_KARUS_VICTORY, m_sElmoradDead, m_sKarusDead);
			else if (m_bVictory == NATION_ELMORAD)
				chatstr = fmt::format_db_resource(
					IDP_ELMORAD_VICTORY, m_sKarusDead, m_sElmoradDead);
			else
				return;
			break;

		case DECLARE_LOSER:
			if (m_bVictory == NATION_KARUS)
				chatstr = fmt::format_db_resource(IDS_ELMORAD_LOSER, m_sKarusDead, m_sElmoradDead);
			else if (m_bVictory == NATION_ELMORAD)
				chatstr = fmt::format_db_resource(IDS_KARUS_LOSER, m_sElmoradDead, m_sKarusDead);
			else
				return;
			break;

		case DECLARE_BAN:
			chatstr = fmt::format_db_resource(IDS_BANISH_USER);
			break;

		case BATTLEZONE_CLOSE:
			chatstr = fmt::format_db_resource(IDS_BATTLE_CLOSE);
			break;

		case KARUS_CAPTAIN_NOTIFY:
			chatstr = fmt::format_db_resource(IDS_KARUS_CAPTAIN, m_strKarusCaptain);
			break;

		case ELMORAD_CAPTAIN_NOTIFY:
			chatstr = fmt::format_db_resource(IDS_ELMO_CAPTAIN, m_strElmoradCaptain);
			break;

		case KARUS_CAPTAIN_DEPRIVE_NOTIFY:
			chatstr = fmt::format_db_resource(IDS_KARUS_CAPTAIN_DEPRIVE, m_strKarusCaptain);
			break;

		case ELMORAD_CAPTAIN_DEPRIVE_NOTIFY:
			chatstr = fmt::format_db_resource(IDS_ELMO_CAPTAIN_DEPRIVE, m_strElmoradCaptain);
			break;

		default:
			spdlog::error(
				"EbenezerApp::Announcement: Unsupported announcement type [type={}]", type);
			return;
	}

	chatstr = fmt::format_db_resource(IDP_ANNOUNCEMENT, chatstr);
	SetByte(sendBuffer, WIZ_CHAT, sendIndex);
	SetByte(sendBuffer, chat_type, sendIndex);
	SetByte(sendBuffer, 1, sendIndex);
	SetShort(sendBuffer, -1, sendIndex);
	SetByte(sendBuffer, 0, sendIndex); // sender name length
	SetString2(sendBuffer, chatstr, sendIndex);

	int socketCount = GetUserSocketCount();
	for (int i = 0; i < socketCount; i++)
	{
		auto pUser = GetUserPtrUnchecked(i);
		if (pUser == nullptr)
			continue;

		if (pUser->GetState() == CONNECTION_STATE_GAMESTART)
		{
			if (nation == 0 || nation == pUser->m_pUserData->m_bNation)
				pUser->Send(sendBuffer, sendIndex);
		}
	}
}

bool EbenezerApp::LoadStartPositionTable()
{
	recordset_loader::STLMap loader(m_StartPositionTableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error(
			"EbenezerApp::LoadStartPositionTable: load failed - {}", loader.GetError().Message);
		return false;
	}

	return true;
}

bool EbenezerApp::LoadServerResourceTable()
{
	ServerResourceTableMap tableMap;

	recordset_loader::STLMap loader(tableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error(
			"EbenezerApp::LoadServerResourceTable: load failed - {}", loader.GetError().Message);
		return false;
	}

	// NOTE: Not a name, but still falls under the same umbrella - this won't be an issue with DBs not padding these.
#if defined(DB_COMPAT_PADDED_NAMES)
	for (auto& [_, serverResource] : tableMap)
		rtrim(serverResource->Resource);
#endif

	m_ServerResourceTableMap.Swap(tableMap);
	return true;
}

bool EbenezerApp::LoadHomeTable()
{
	recordset_loader::STLMap loader(m_HomeTableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("EbenezerApp::LoadHomeTable: load failed - {}", loader.GetError().Message);
		return false;
	}

	return true;
}

bool EbenezerApp::LoadAllKnights()
{
	using ModelType = model::Knights;

	recordset_loader::Base<ModelType> loader;
	loader.SetProcessFetchCallback(
		[this](db::ModelRecordSet<ModelType>& recordset)
		{
			do
			{
				ModelType row {};
				recordset.get_ref(row);

				CKnights* pKnights = new CKnights();
				pKnights->InitializeValue();

				pKnights->m_sIndex   = row.ID;
				pKnights->m_byFlag   = row.Flag;
				pKnights->m_byNation = row.Nation;

#if defined(DB_COMPAT_PADDED_NAMES)
				rtrim(row.Name);
#endif
				if (strcpy_safe(pKnights->m_strName, row.Name))
				{
					spdlog::warn(
						"EbenezerApp::LoadAllKnights: IDName too long, truncating - knightsId={}",
						pKnights->m_sIndex);
				}

#if defined(DB_COMPAT_PADDED_NAMES)
				rtrim(row.Chief);
#endif

				if (strcpy_safe(pKnights->m_strChief, row.Chief) != 0)
				{
					spdlog::warn(
						"EbenezerApp::LoadAllKnights: Chief too long, truncating - knightsId={}",
						pKnights->m_sIndex);
				}

				if (row.ViceChief1.has_value())
				{
#if defined(DB_COMPAT_PADDED_NAMES)
					rtrim(*row.ViceChief1);
#endif

					if (strcpy_safe(pKnights->m_strViceChief_1, row.ViceChief1.value()) != 0)
					{
						spdlog::warn("EbenezerApp::LoadAllKnights: ViceChief_1 too long, "
									 "truncating - knightsId={}",
							pKnights->m_sIndex);
					}
				}

				if (row.ViceChief2.has_value())
				{
#if defined(DB_COMPAT_PADDED_NAMES)
					rtrim(*row.ViceChief2);
#endif

					if (strcpy_safe(pKnights->m_strViceChief_2, row.ViceChief2.value()) != 0)
					{
						spdlog::warn("EbenezerApp::LoadAllKnights: ViceChief_2 too long, "
									 "truncating - knightsId={}",
							pKnights->m_sIndex);
					}
				}

				if (row.ViceChief3.has_value())
				{
#if defined(DB_COMPAT_PADDED_NAMES)
					rtrim(*row.ViceChief3);
#endif

					if (strcpy_safe(pKnights->m_strViceChief_3, row.ViceChief3.value()) != 0)
					{
						spdlog::warn("EbenezerApp::LoadAllKnights: ViceChief_3 too long, "
									 "truncating - knightsId={}",
							pKnights->m_sIndex);
					}
				}

				pKnights->m_sMembers         = row.Members;
				pKnights->m_nMoney           = row.Gold;
				pKnights->m_sAllianceKnights = row.AllianceKnights;
				pKnights->m_sMarkVersion     = row.MarkVersion;
				pKnights->m_sCape            = row.Cape;
				pKnights->m_sDomination      = row.Domination;
				pKnights->m_nPoints          = row.Points;
				pKnights->m_byGrade          = GetKnightsGrade(row.Points);
				pKnights->m_byRanking        = row.Ranking;

				for (int i = 0; i < MAX_CLAN; i++)
				{
					pKnights->m_arKnightsUser[i].byUsed = 0;
					strcpy_safe(pKnights->m_arKnightsUser[i].strUserName, "");
				}

				if (!m_KnightsMap.PutData(pKnights->m_sIndex, pKnights))
				{
					spdlog::error(
						"EbenezerApp::LoadAllKnights: failed to put into KnightsMap [knightsId={}]",
						pKnights->m_sIndex);
					delete pKnights;
				}
			}
			while (recordset.next());
		});

	if (!loader.Load_AllowEmpty())
	{
		spdlog::error("EbenezerApp::LoadAllKnights: load failed - {}", loader.GetError().Message);
		return false;
	}

	return true;
}

bool EbenezerApp::LoadAllKnightsUserData()
{
	using ModelType = model::KnightsUser;

	recordset_loader::Base<ModelType> loader;
	loader.SetProcessFetchCallback(
		[this](db::ModelRecordSet<ModelType>& recordset)
		{
			do
			{
				ModelType row {};
				recordset.get_ref(row);

#if defined(DB_COMPAT_PADDED_NAMES)
				rtrim(row.UserId);
#endif
				m_KnightsManager.AddKnightsUser(row.KnightsId, row.UserId.c_str());
			}
			while (recordset.next());
		});

	if (!loader.Load_AllowEmpty())
	{
		spdlog::error(
			"EbenezerApp::LoadAllKnightsUserData: load failed - {}", loader.GetError().Message);
		return false;
	}

	return true;
}

bool EbenezerApp::LoadKnightsSiegeWarfareTable()
{
	using ModelType = model::KnightsSiegeWarfare;

	recordset_loader::Base<ModelType> loader;
	loader.SetProcessFetchCallback(
		[this](db::ModelRecordSet<ModelType>& recordset)
		{
			do
			{
				ModelType row {};
				recordset.get_ref(row);
				m_KnightsSiegeWar._castleIndex   = row.CastleIndex;
				m_KnightsSiegeWar._masterKnights = row.MasterKnights;
			}
			while (recordset.next());
		});

	if (!loader.Load_AllowEmpty())
	{
		spdlog::error("EbenezerApp::LoadKnightsSiegeWarfareTable: load failed - {}",
			loader.GetError().Message);
		return false;
	}

	return true;
}

int EbenezerApp::GetKnightsAllMembers(int knightsindex, char* temp_buff, int& buff_index, int type)
{
	if (knightsindex <= 0)
		return 0;

	int count = 0;
	if (type == 0)
	{
		int socketCount = GetUserSocketCount();
		for (int i = 0; i < socketCount; i++)
		{
			auto pUser = GetUserPtrUnchecked(i);
			if (pUser == nullptr)
				continue;

			// 같은 소속의 클랜..
			if (pUser->m_pUserData->m_bKnights == knightsindex)
			{
				SetString2(temp_buff, pUser->m_pUserData->m_id, buff_index);
				SetByte(temp_buff, pUser->m_pUserData->m_bFame, buff_index);
				SetByte(temp_buff, pUser->m_pUserData->m_bLevel, buff_index);
				SetShort(temp_buff, pUser->m_pUserData->m_sClass, buff_index);
				SetByte(temp_buff, 1, buff_index);
				count++;
			}
		}
	}
	else if (type == 1)
	{
		CKnights* pKnights = m_KnightsMap.GetData(knightsindex);
		if (pKnights == nullptr)
			return 0;

		for (int i = 0; i < MAX_CLAN; i++)
		{
			if (pKnights->m_arKnightsUser[i].byUsed == 1)
			{
				// 접속중인 회원
				auto pUser = GetUserPtr(
					pKnights->m_arKnightsUser[i].strUserName, NameType::Character);
				if (pUser != nullptr)
				{
					if (pUser->m_pUserData->m_bKnights == knightsindex)
					{
						SetString2(temp_buff, pUser->m_pUserData->m_id, buff_index);
						SetByte(temp_buff, pUser->m_pUserData->m_bFame, buff_index);
						SetByte(temp_buff, pUser->m_pUserData->m_bLevel, buff_index);
						SetShort(temp_buff, pUser->m_pUserData->m_sClass, buff_index);
						SetByte(temp_buff, 1, buff_index);
						count++;
					}
					// 다른존에서 탈퇴나 추방된 유저이므로 메모리에서 삭제
					else
					{
						m_KnightsManager.RemoveKnightsUser(knightsindex, pUser->m_pUserData->m_id);
					}
				}
				// 비접속중인 회원
				else
				{
					SetString2(temp_buff, pKnights->m_arKnightsUser[i].strUserName, buff_index);
					SetByte(temp_buff, 0, buff_index);
					SetByte(temp_buff, 0, buff_index);
					SetShort(temp_buff, 0, buff_index);
					SetByte(temp_buff, 0, buff_index);
					count++;
				}
			}
		}
	}

	return count;
}

void EbenezerApp::MarketBBSTimeCheck()
{
	double currentTime = TimeGet();

	for (int i = 0; i < MAX_BBS_POST; i++)
	{
		// BUY!!!
		if (m_sBuyID[i] != -1)
		{
			auto pUser = GetUserPtr(m_sBuyID[i]);
			if (pUser == nullptr)
			{
				MarketBBSBuyDelete(i);
				continue;
			}

			if (m_fBuyStartTime[i] + BBS_CHECK_TIME < currentTime)
				MarketBBSBuyDelete(i);
		}

		// SELL!!!
		if (m_sSellID[i] != -1)
		{
			auto pUser = GetUserPtr(m_sSellID[i]);
			if (pUser == nullptr)
			{
				MarketBBSSellDelete(i);
				continue;
			}

			if (m_fSellStartTime[i] + BBS_CHECK_TIME < currentTime)
				MarketBBSSellDelete(i);
		}
	}
}

void EbenezerApp::MarketBBSBuyDelete(int16_t index)
{
	m_sBuyID[index] = -1;
	memset(m_strBuyTitle[index], 0, sizeof(m_strBuyTitle[index]));
	memset(m_strBuyMessage[index], 0, sizeof(m_strBuyMessage[index]));
	m_iBuyPrice[index]     = 0;
	m_fBuyStartTime[index] = 0.0;
}

void EbenezerApp::MarketBBSSellDelete(int16_t index)
{
	m_sSellID[index] = -1;
	memset(m_strSellTitle[index], 0, sizeof(m_strSellTitle[index]));
	memset(m_strSellMessage[index], 0, sizeof(m_strSellMessage[index]));
	m_iSellPrice[index]     = 0;
	m_fSellStartTime[index] = 0.0;
}

void EbenezerApp::WritePacketLog()
{
	spdlog::info("EbenezerApp::WritePacketLog: send={} realsend={} recv={}", m_iPacketCount,
		m_iSendPacketCount, m_iRecvPacketCount);
}

int EbenezerApp::GetKnightsGrade(int nPoints)
{
	// 클랜등급 = 클랜원 국가 기여도의 총합 / 24
	int nClanPoints = nPoints / 24;

	int nGrade      = 5;
	if (nClanPoints >= 0 && nClanPoints < 2000)
		nGrade = 5;
	else if (nClanPoints >= 2000 && nClanPoints < 5000)
		nGrade = 4;
	else if (nClanPoints >= 5000 && nClanPoints < 10000)
		nGrade = 3;
	else if (nClanPoints >= 10000 && nClanPoints < 20000)
		nGrade = 2;
	else if (nClanPoints >= 20000)
		nGrade = 1;

	return nGrade;
}

void EbenezerApp::CheckAliveUser()
{
	int socketCount = GetUserSocketCount();
	for (int i = 0; i < socketCount; i++)
	{
		auto pUser = GetUserPtrUnchecked(i);
		if (pUser == nullptr)
			continue;

		if (pUser->GetState() == CONNECTION_STATE_GAMESTART)
		{
			if (pUser->m_sAliveCount > 3)
			{
				pUser->Close();
				spdlog::debug("EbenezerApp::CheckAliveUser: User Alive Close [userId={} charId={}]",
					pUser->GetSocketID(), pUser->m_pUserData->m_id);
			}

			pUser->m_sAliveCount++;
		}
	}
}

void EbenezerApp::KickOutAllUsers()
{
	int socketCount = GetUserSocketCount();
	for (int i = 0; i < socketCount; i++)
	{
		auto pUser = GetUserPtrUnchecked(i);
		if (pUser == nullptr)
			continue;

		pUser->Close();
		std::this_thread::sleep_for(1s);
	}
}

int64_t EbenezerApp::GenerateItemSerial()
{
	MYINT64 serial;
	MYSHORT increase;
	serial.i   = 0;
	increase.w = 0;

	DateTime t = DateTime::GetNow();

	std::lock_guard<std::mutex> lock(_serialMutex);

	increase.w  = g_increase_serial++;

	serial.b[7] = (uint8_t) m_nServerNo;
	serial.b[6] = (uint8_t) (t.GetYear() % 100);
	serial.b[5] = (uint8_t) t.GetMonth();
	serial.b[4] = (uint8_t) t.GetDay();
	serial.b[3] = (uint8_t) t.GetHour();
	serial.b[2] = (uint8_t) t.GetMinute();
	serial.b[1] = increase.b[1];
	serial.b[0] = increase.b[0];

	//	TRACE(_T("Generate Item Serial : %I64d\n"), serial.i);
	return serial.i;
}

void EbenezerApp::KickOutZoneUsers(int16_t zone)
{
	int socketCount = GetUserSocketCount();
	for (int i = 0; i < socketCount; i++)
	{
		auto pTUser = GetUserPtrUnchecked(i);
		if (pTUser == nullptr)
			continue;

		// Only kick out users in requested zone.
		if (pTUser->m_pUserData->m_bZone == zone)
		{
			C3DMap* pMap = GetMapByID(pTUser->m_pUserData->m_bNation);
			if (pMap != nullptr)
				pTUser->ZoneChange(pMap->m_nZoneNumber, pMap->m_fInitX,
					pMap->m_fInitZ); // Move user to native zone.
		}
	}
}

void EbenezerApp::Send_UDP_All(char* pBuf, int len, int group_type)
{
	if (group_type == 0)
	{
		for (const auto& [_, pInfo] : m_ServerArray)
		{
			if (pInfo != nullptr && pInfo->sServerNo != m_nServerNo)
				m_pUdpSocket->SendUDPPacket(pInfo->strServerIP, pBuf, len);
		}
	}
	else
	{
		for (const auto& [_, pInfo] : m_ServerGroupArray)
		{
			if (pInfo != nullptr && pInfo->sServerNo != m_nServerGroupNo)
				m_pUdpSocket->SendUDPPacket(pInfo->strServerIP, pBuf, len);
		}
	}
}

bool EbenezerApp::LoadBattleTable()
{
	using ModelType = model::Battle;

	recordset_loader::Base<ModelType> loader;
	loader.SetProcessFetchCallback(
		[this](db::ModelRecordSet<ModelType>& recordset)
		{
			do
			{
				ModelType row {};
				recordset.get_ref(row);

				m_byOldVictory = row.Nation;
			}
			while (recordset.next());
		});

	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("EbenezerApp::LoadBattleTable: load failed - {}", loader.GetError().Message);
		return false;
	}

	return true;
}

void EbenezerApp::Send_CommandChat(char* pBuf, int len, int nation, CUser* /*pExceptUser*/)
{
	int socketCount = GetUserSocketCount();
	for (int i = 0; i < socketCount; i++)
	{
		auto pUser = GetUserPtrUnchecked(i);
		if (pUser == nullptr)
			continue;

		// if (pUser == pExceptUser)
		//	continue;

		if (pUser->GetState() == CONNECTION_STATE_GAMESTART
			&& pUser->m_pUserData->m_bNation == nation)
			pUser->Send(pBuf, len);
	}
}

bool EbenezerApp::LoadKnightsRankTable()
{
	using ModelType = model::KnightsRating;

	std::string strKarusCaptain[5], strElmoCaptain[5];

	recordset_loader::Base<ModelType> loader;
	loader.SetProcessFetchCallback(
		[&](db::ModelRecordSet<ModelType>& recordset)
		{
			int nKarusRank = 0, nElmoRank = 0, sendIndex = 0;
			char sendBuffer[1024] {};

			do
			{
				ModelType row {};
				recordset.get_ref(row);

				CKnights* pKnights = m_KnightsMap.GetData(row.Index);
				if (pKnights == nullptr)
					continue;

#if defined(DB_COMPAT_PADDED_NAMES)
				rtrim(row.Name);
#endif

				if (pKnights->m_byNation == NATION_KARUS)
				{
					//if (nKarusRank == 5 || nFindKarus == 1)
					if (nKarusRank == 5)
						continue; // 5위까지 클랜장이 없으면 대장은 없음

					//nKarusRank++;

					auto pUser = GetUserPtr(pKnights->m_strChief, NameType::Character);
					if (pUser == nullptr)
						continue;

					if (pUser->m_pUserData->m_bZone != ZONE_BATTLE)
						continue;

					if (pUser->m_pUserData->m_bKnights == row.Index)
					{
						pUser->m_pUserData->m_bFame = KNIGHTS_DUTY_CAPTAIN;
						strKarusCaptain[nKarusRank] = fmt::format(
							"[{}][{}]", row.Name, pUser->m_pUserData->m_id);
						nKarusRank++;

						memset(sendBuffer, 0, sizeof(sendBuffer));
						sendIndex = 0;
						SetByte(sendBuffer, WIZ_AUTHORITY_CHANGE, sendIndex);
						SetByte(sendBuffer, COMMAND_AUTHORITY, sendIndex);
						SetShort(sendBuffer, pUser->GetSocketID(), sendIndex);
						SetByte(sendBuffer, pUser->m_pUserData->m_bFame, sendIndex);
						//pUser->Send( sendBuffer, sendIndex );
						Send_Region(sendBuffer, sendIndex, pUser->m_pUserData->m_bZone,
							pUser->m_RegionX, pUser->m_RegionZ);

						//strcpy( m_strKarusCaptain, pUser->m_pUserData->m_id );
						//Announcement( KARUS_CAPTAIN_NOTIFY, NATION_KARUS );
						//TRACE(_T("Karus Captain - %hs, rank=%d, index=%d\n"), pUser->m_pUserData->m_id, row.Rank, row.Index);
					}
				}
				else if (pKnights->m_byNation == NATION_ELMORAD)
				{
					//if (nElmoRank == 5 || nFindElmo == 1)
					if (nElmoRank == 5)
						continue; // 5위까지 클랜장이 없으면 대장은 없음

					//nElmoRank++;

					auto pUser = GetUserPtr(pKnights->m_strChief, NameType::Character);
					if (pUser == nullptr)
						continue;

					if (pUser->m_pUserData->m_bZone != ZONE_BATTLE)
						continue;

					if (pUser->m_pUserData->m_bKnights == row.Index)
					{
						pUser->m_pUserData->m_bFame = KNIGHTS_DUTY_CAPTAIN;
						strElmoCaptain[nElmoRank]   = fmt::format(
                            "[{}][{}]", row.Name, pUser->m_pUserData->m_id);
						nElmoRank++;

						memset(sendBuffer, 0, sizeof(sendBuffer));
						sendIndex = 0;
						SetByte(sendBuffer, WIZ_AUTHORITY_CHANGE, sendIndex);
						SetByte(sendBuffer, COMMAND_AUTHORITY, sendIndex);
						SetShort(sendBuffer, pUser->GetSocketID(), sendIndex);
						SetByte(sendBuffer, pUser->m_pUserData->m_bFame, sendIndex);
						//pUser->Send( sendBuffer, sendIndex );
						Send_Region(sendBuffer, sendIndex, pUser->m_pUserData->m_bZone,
							pUser->m_RegionX, pUser->m_RegionZ);

						//strcpy( m_strElmoradCaptain, pUser->m_pUserData->m_id );
						//Announcement( ELMORAD_CAPTAIN_NOTIFY, NATION_ELMORAD );
						//TRACE(_T("Elmo Captain - %hs, rank=%d, index=%d\n"), pUser->m_pUserData->m_id, row.Rank, row.Index);
					}
				}
			}
			while (recordset.next());
		});

	if (!loader.Load_AllowEmpty())
	{
		spdlog::error(
			"EbenezerApp::LoadKnightsRankTable: load failed - {}", loader.GetError().Message);
		return false;
	}

	std::string strKarusCaptainName = fmt::format_db_resource(IDS_KARUS_CAPTAIN, strKarusCaptain[0],
		strKarusCaptain[1], strKarusCaptain[2], strKarusCaptain[3], strKarusCaptain[4]);

	std::string strElmoCaptainName  = fmt::format_db_resource(IDS_ELMO_CAPTAIN, strElmoCaptain[0],
		 strElmoCaptain[1], strElmoCaptain[2], strElmoCaptain[3], strElmoCaptain[4]);

	spdlog::trace("EbenezerApp::LoadKnightsRankTable: success");

	int sendIndex = 0, temp_index = 0;
	char sendBuffer[1024] {}, temp_buff[1024] {};

	SetByte(sendBuffer, WIZ_CHAT, sendIndex);
	SetByte(sendBuffer, WAR_SYSTEM_CHAT, sendIndex);
	SetByte(sendBuffer, 1, sendIndex);
	SetShort(sendBuffer, -1, sendIndex);
	SetByte(sendBuffer, 0, sendIndex); // sender name length
	SetString2(sendBuffer, strKarusCaptainName, sendIndex);

	SetByte(temp_buff, WIZ_CHAT, temp_index);
	SetByte(temp_buff, WAR_SYSTEM_CHAT, temp_index);
	SetByte(temp_buff, 1, temp_index);
	SetShort(temp_buff, -1, temp_index);
	SetByte(temp_buff, 0, sendIndex); // sender name length
	SetString2(temp_buff, strElmoCaptainName, temp_index);

	int socketCount = GetUserSocketCount();
	for (int i = 0; i < socketCount; i++)
	{
		auto pUser = GetUserPtrUnchecked(i);
		if (pUser == nullptr)
			continue;

		if (pUser->GetState() == CONNECTION_STATE_GAMESTART)
		{
			if (pUser->m_pUserData->m_bNation == NATION_KARUS)
				pUser->Send(sendBuffer, sendIndex);
			else if (pUser->m_pUserData->m_bNation == NATION_ELMORAD)
				pUser->Send(temp_buff, temp_index);
		}
	}

	return true;
}

void EbenezerApp::BattleZoneCurrentUsers()
{
	C3DMap* pMap = GetMapByID(ZONE_BATTLE);
	if (pMap == nullptr)
		return;

	// 현재의 서버가 배틀존 서버가 아니라면 리턴
	if (m_nServerNo != pMap->m_nServerNo)
		return;

	char sendBuffer[128] {};
	int nKarusMan = 0, nElmoradMan = 0, sendIndex = 0;

	int socketCount = GetUserSocketCount();
	for (int i = 0; i < socketCount; i++)
	{
		auto pUser = GetUserPtrUnchecked(i);
		if (pUser == nullptr)
			continue;

		if (pUser->m_pUserData->m_bZone == ZONE_BATTLE)
		{
			if (pUser->m_pUserData->m_bNation == NATION_KARUS)
				nKarusMan++;
			else if (pUser->m_pUserData->m_bNation == NATION_ELMORAD)
				nElmoradMan++;
		}
	}

	m_sKarusCount   = nKarusMan;
	m_sElmoradCount = nElmoradMan;

	//TRACE(_T("---> BattleZoneCurrentUsers - karus=%d, elmorad=%d\n"), m_sKarusCount, m_sElmoradCount);

	SetByte(sendBuffer, UDP_BATTLEZONE_CURRENT_USERS, sendIndex);
	SetShort(sendBuffer, m_sKarusCount, sendIndex);
	SetShort(sendBuffer, m_sElmoradCount, sendIndex);
	Send_UDP_All(sendBuffer, sendIndex);
}

void EbenezerApp::FlySanta()
{
	int sendIndex = 0;
	char sendBuffer[128] {};

	SetByte(sendBuffer, WIZ_SANTA, sendIndex);
	SetByte(sendBuffer, m_bySanta, sendIndex);
	Send_All(sendBuffer, sendIndex);
}

C3DMap* EbenezerApp::GetMapByIndex(int iZoneIndex) const
{
	if (iZoneIndex < 0 || iZoneIndex >= static_cast<int>(m_ZoneArray.size()))
		return nullptr;

	return m_ZoneArray[iZoneIndex];
}

C3DMap* EbenezerApp::GetMapByID(int iZoneID) const
{
	for (C3DMap* pMap : m_ZoneArray)
	{
		if (pMap != nullptr && pMap->m_nZoneNumber == iZoneID)
			return pMap;
	}

	return nullptr;
}

bool EbenezerApp::LoadEventTriggerTable()
{
	using ModelType = model::EventTrigger;

	EventTriggerMap localMap;

	recordset_loader::Base<ModelType> loader;
	loader.SetProcessFetchCallback(
		[&](db::ModelRecordSet<ModelType>& recordset)
		{
			do
			{
				ModelType row {};
				recordset.get_ref(row);

				uint32_t key  = GetEventTriggerKey(row.NpcType, row.NpcId);

				bool inserted = localMap.insert(std::make_pair(key, row.TriggerNumber)).second;
				if (!inserted)
				{
					spdlog::error("EbenezerApp::LoadEventTriggerTable: failed to insert into "
								  "EventTriggerMap [NpcType={} NpcId={}]",
						row.NpcType, row.NpcId);
				}
			}
			while (recordset.next());
		});

	if (!loader.Load_AllowEmpty())
	{
		spdlog::error(
			"EbenezerApp::LoadEventTriggerTable: load failed - {}", loader.GetError().Message);
		return false;
	}

	m_EventTriggerMap.swap(localMap);
	return true;
}

uint32_t EbenezerApp::GetEventTriggerKey(uint8_t byNpcType, uint16_t sTrapNumber) const
{
	// This is just a more efficient way of packing both components into a single key
	// instead of using and comparing std::pair<> in our map.
	return (static_cast<uint32_t>(byNpcType) << 16) | sTrapNumber;
}

int32_t EbenezerApp::GetEventTrigger(uint8_t byNpcType, uint16_t sTrapNumber) const
{
	uint32_t key = GetEventTriggerKey(byNpcType, sTrapNumber);

	auto itr     = m_EventTriggerMap.find(key);
	if (itr == m_EventTriggerMap.end())
		return -1;

	return itr->second;
}

void EbenezerApp::SendSMQHeartbeat()
{
	char sendBuffer[4];
	int sendIndex = 0;
	SetByte(sendBuffer, DB_HEARTBEAT, sendIndex);
	m_LoggerSendQueue.PutData(sendBuffer, sendIndex);
}

void EbenezerApp::GameTimeTick()
{
	UpdateGameTime();

	// AIServer Socket Alive Check Routine
	if (m_bFirstServerFlag)
	{
		int count = 0;
		for (int i = 0; i < MAX_AI_SOCKET; i++)
		{
			std::unique_lock<std::mutex> lock(_aiSocketMutex);

			auto itr = _aiSocketMap.find(i);
			if (itr != _aiSocketMap.end() && itr->second != nullptr
				&& itr->second->GetState() != CONNECTION_STATE_DISCONNECTED)
			{
				++count;
				continue;
			}

			lock.unlock();
			AISocketConnect(i, true);
		}

		if (count <= 0)
			DeleteAllNpcList();
	}
	// sungyong~ 2002.05.23
}

} // namespace Ebenezer

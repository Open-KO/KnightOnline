#include "pch.h"
#include "AIServerApp.h"
#include "AIServerLogger.h"
#include "GameSocket.h"
#include "NpcThread.h"
#include "Region.h"
#include "RoomEvent.h"
#include "ZoneEventThread.h"

#include <argparse/argparse.hpp>
#include <FileIO/FileReader.h>

#include <shared/crc32.h>
#include <shared/globals.h>
#include <shared/Ini.h>
#include <shared/lzf.h>
#include <shared/StringUtils.h>
#include <shared/TimerThread.h>

#include <spdlog/spdlog.h>

#include <db-library/ConnectionManager.h>
#include <db-library/RecordSetLoader_STLMap.h>
#include <db-library/RecordSetLoader_Vector.h>

#include <AIServer/binder/AIServerBinder.h>

#include <cmath>

using namespace std::chrono_literals;
using namespace db;

namespace AIServer
{

std::mutex g_user_mutex;
std::mutex g_region_mutex;

AIServerApp::AIServerApp(AIServerLogger& logger) : AppThread(logger)
{
	_telnetPort = 2323;
	memset(_compressedPacketBuffer, 0, sizeof(_compressedPacketBuffer));

	ConnectionManager::Create();

	_checkAliveThread = std::make_unique<TimerThread>(
		10s, std::bind(&AIServerApp::CheckAliveTest, this));
}

AIServerApp::~AIServerApp()
{
	spdlog::info("AIServerApp::~AIServerApp: Shutting down, releasing resources.");
	_serverSocketManager.Shutdown();
	spdlog::info("AIServerApp::~AIServerApp: AI socket manager stopped.");

	// wait for all of these threads to be fully shut down.
	spdlog::info("AIServerApp::~AIServerApp: Waiting for worker threads to fully shut down.");

	if (_checkAliveThread != nullptr)
	{
		spdlog::info("AIServerApp::~AIServerApp: Shutting down CheckAliveThread...");

		_checkAliveThread->shutdown();

		spdlog::info("AIServerApp::~AIServerApp: CheckAliveThread stopped.");
	}

	if (!_npcThreads.empty())
	{
		spdlog::info(
			"AIServerApp::~AIServerApp: Shutting down {} NPC threads...", _npcThreads.size());

		for (CNpcThread* npcThread : _npcThreads)
			npcThread->shutdown(false);

		for (CNpcThread* npcThread : _npcThreads)
			npcThread->join();

		spdlog::info("AIServerApp::~AIServerApp: NPC threads stopped.");
	}

	if (_zoneEventThread != nullptr)
	{
		spdlog::info("AIServerApp::~AIServerApp: Shutting down ZoneEventThread...");

		_zoneEventThread->shutdown();

		spdlog::info("AIServerApp::~AIServerApp: ZoneEventThread stopped.");
	}

	spdlog::info("AIServerApp::~AIServerApp: All worker threads stopped, freeing caches.");

	for (MAP* map : _zones)
		delete map;
	_zones.clear();

	delete _zoneEventThread;
	_zoneEventThread = nullptr;

	for (CNpcThread* npcThread : _npcThreads)
		delete npcThread;
	_npcThreads.clear();

	if (_npcItem.m_ppItem != nullptr)
	{
		for (int i = 0; i < _npcItem.m_nRow; i++)
		{
			delete[] _npcItem.m_ppItem[i];
			_npcItem.m_ppItem[i] = nullptr;
		}

		delete[] _npcItem.m_ppItem;
		_npcItem.m_ppItem = nullptr;
	}

	for (int i = 0; i < MAX_USER; i++)
	{
		delete _users[i];
		_users[i] = nullptr;
	}

	spdlog::info("AIServerApp::~AIServerApp: All resources safely released.");

	ConnectionManager::Destroy();
}

bool AIServerApp::OnStart()
{
	//----------------------------------------------------------------------
	//	Sets a random number starting point.
	//----------------------------------------------------------------------
	srand((unsigned int) time(nullptr));
	for (int i = 0; i < 10; i++)
		myrand(1, 10000); // don't delete

	// Compress Init
	memset(
		_compressedPacketBuffer, 0, sizeof(_compressedPacketBuffer)); // 압축할 데이터를 모으는 버퍼
	_compressedPacketIndex = 0;                                       // 압축할 데이터의 길이
	_compressedPacketCount = 0;                                       // 압축할 데이터의 개수

	_socketCount           = 0;
	_aliveSocketCount      = 0;
	_mapEventNpcCount      = 0;
	_reconnectSocketCount  = 0;
	_reconnectStartTime    = 0.0;
	_firstServerFlag       = false;
	_testMode              = NOW_TEST_MODE;

	// User Point Init
	for (int i = 0; i < MAX_USER; i++)
		_users[i] = nullptr;

	// Server start message
	spdlog::info("AIServerApp::OnStart: starting...");

	_itemLogger = spdlog::get(std::string(logger::AIServerItem));
	_userLogger = spdlog::get(std::string(logger::AIServerUser));

	//----------------------------------------------------------------------
	//	DB part initialize
	//----------------------------------------------------------------------
	if (_serverZoneType == UNIFY_ZONE)
		spdlog::info("AIServerApp::OnStart: Server Zone: UNIFY");
	else if (_serverZoneType == KARUS_ZONE)
		spdlog::info("AIServerApp::OnStart: Server Zone: KARUS");
	else if (_serverZoneType == ELMORAD_ZONE)
		spdlog::info("AIServerApp::OnStart: Server Zone: ELMORAD");
	else if (_serverZoneType == BATTLE_ZONE)
		spdlog::info("AIServerApp::OnStart: Server Zone: BATTLE");

	//----------------------------------------------------------------------
	//	Communication Part Init ...
	//----------------------------------------------------------------------
	spdlog::info("AIServerApp::OnStart: initializing sockets");
	_serverSocketManager.Init(MAX_SOCKET, 1);
	_serverSocketManager.AllocateSockets<CGameSocket>();

	//----------------------------------------------------------------------
	//	Load Magic Table
	//----------------------------------------------------------------------
	if (!GetMagicTableData())
	{
		spdlog::error("AIServerApp::OnStart: failed to load MAGIC, closing server");
		return false;
	}

	if (!GetMagicType1Data())
	{
		spdlog::error("AIServerApp::OnStart: failed to load MAGIC_TYPE1, closing server");
		return false;
	}

	if (!GetMagicType2Data())
	{
		spdlog::error("AIServerApp::OnStart: failed to load MAGIC_TYPE2, closing server");
		return false;
	}

	if (!GetMagicType3Data())
	{
		spdlog::error("AIServerApp::OnStart: failed to load MAGIC_TYPE3, closing server");
		return false;
	}

	if (!GetMagicType4Data())
	{
		spdlog::error("AIServerApp::OnStart: failed to load MAGIC_TYPE4, closing server");
		return false;
	}

	if (!GetMagicType7Data())
	{
		spdlog::error("AIServerApp::OnStart: failed to load MAGIC_TYPE7, closing server");
		return false;
	}

	//----------------------------------------------------------------------
	//	Load NPC Item Table
	//----------------------------------------------------------------------
	if (!GetNpcItemTable())
	{
		spdlog::error("AIServerApp::OnStart: failed to load K_MONSTER_ITEM, closing server");
		return false;
	}

	if (!GetMakeWeaponItemTableData())
	{
		spdlog::error("AIServerApp::OnStart: failed to load MAKE_WEAPON, closing server");
		return false;
	}

	if (!GetMakeDefensiveItemTableData())
	{
		spdlog::error("AIServerApp::OnStart: failed to load MAKE_DEFENSIVE, closing server");
		return false;
	}

	if (!GetMakeGradeItemTableData())
	{
		spdlog::error("AIServerApp::OnStart: failed to load MAKE_ITEM_GRADECODE, closing server");
		return false;
	}

	if (!GetMakeRareItemTableData())
	{
		spdlog::error("AIServerApp::OnStart: failed to load MAKE_ITEM_LARECODE, closing server");
		return false;
	}

	if (!GetMakeItemGroupTableData())
	{
		spdlog::error("AIServerApp::OnStart: failed to load MAKE_ITEM_GROUP, closing server");
		return false;
	}

	if (!GetZoneInfoTableData())
	{
		spdlog::error("AIServerApp::OnStart: failed to load ZONE_INFO, closing server");
		return false;
	}

	//----------------------------------------------------------------------
	//	Load NPC Data & Activate NPC
	//----------------------------------------------------------------------

	// Monster 특성치 테이블 Load
	if (!GetMonsterTableData())
	{
		spdlog::error("AIServerApp::OnStart: failed to load K_MONSTER, closing server");
		return false;
	}

	// NPC 특성치 테이블 Load
	if (!GetNpcTableData())
	{
		spdlog::error("AIServerApp::OnStart: failed to load K_NPC, closing server");
		return false;
	}

	//----------------------------------------------------------------------
	//	Load Zone & Event...
	//----------------------------------------------------------------------
	if (!MapFileLoad())
	{
		spdlog::error("AIServerApp::OnStart: failed to load maps, closing server");
		return false;
	}

	if (!CreateNpcThread())
	{
		spdlog::error("AIServerApp::OnStart: CreateNpcThread failed, closing server");
		return false;
	}

	//----------------------------------------------------------------------
	//	Load NPC DN Table
	//----------------------------------------------------------------------

	//----------------------------------------------------------------------
	//	Start Accepting...
	//----------------------------------------------------------------------
	if (!ListenByServerZoneType())
		return false;

	//----------------------------------------------------------------------
	//	Start NPC THREAD
	//----------------------------------------------------------------------
	StartNpcThreads();

	_checkAliveThread->start();

	spdlog::info("AIServerApp::OnStart: AIServer successfully initialized");

	return true;
}

bool AIServerApp::ListenByServerZoneType()
{
	int port = GetListenPortByServerZoneType();
	if (port < 0)
	{
		spdlog::error(
			"AIServerApp::ListenByServerZoneType: failed to associate listen port for zone {}",
			_serverZoneType);
		return false;
	}

	if (!_serverSocketManager.Listen(port))
	{
		spdlog::error("AIServerApp::ListenByServerZoneType: failed to listen on port {}", port);
		return false;
	}

	spdlog::info("AIServerApp::ListenByServerZoneType: Listening on 0.0.0.0:{}", port);
	return true;
}

int AIServerApp::GetListenPortByServerZoneType() const
{
	switch (_serverZoneType)
	{
		case KARUS_ZONE:
		case UNIFY_ZONE:
			return AI_KARUS_SOCKET_PORT;

		case ELMORAD_ZONE:
			return AI_ELMO_SOCKET_PORT;

		case BATTLE_ZONE:
			return AI_BATTLE_SOCKET_PORT;

		default:
			return -1;
	}
}

//	Magic Table 을 읽는다.
bool AIServerApp::GetMagicTableData()
{
	recordset_loader::STLMap loader(_magicTableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error(
			"AIServerApp::GetMagicTableData: load failed - {}", loader.GetError().Message);
		return false;
	}

	spdlog::info("AIServerApp::GetMagicTableData: MAGIC loaded");
	return true;
}

bool AIServerApp::GetMakeWeaponItemTableData()
{
	recordset_loader::STLMap loader(_makeWeaponTableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error(
			"AIServerApp::GetMakeWeaponItemTableData: load failed - {}", loader.GetError().Message);
		return false;
	}

	spdlog::info("AIServerApp::GetMakeWeaponItemTableData: MAKE_WEAPON loaded");
	return true;
}

bool AIServerApp::GetMakeDefensiveItemTableData()
{
	recordset_loader::STLMap<MakeWeaponTableMap, model::MakeDefensive> loader(
		_makeDefensiveTableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("AIServerApp::GetMakeDefensiveItemTableData: load failed - {}",
			loader.GetError().Message);
		return false;
	}

	spdlog::info("AIServerApp::GetMakeDefensiveItemTableData: MAKE_DEFENSIVE loaded");
	return true;
}

bool AIServerApp::GetMakeGradeItemTableData()
{
	recordset_loader::STLMap loader(_makeGradeItemCodeTableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error(
			"AIServerApp::GetMakeGradeItemTableData: load failed - {}", loader.GetError().Message);
		return false;
	}

	spdlog::info("AIServerApp::GetMakeGradeItemTableData: MAKE_ITEM_GRADECODE loaded");
	return true;
}

bool AIServerApp::GetMakeRareItemTableData()
{
	recordset_loader::STLMap loader(_makeItemRareCodeTableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error(
			"AIServerApp::GetMakeRareItemTableData: load failed - {}", loader.GetError().Message);
		return false;
	}

	spdlog::info("AIServerApp::GetMakeRareItemTableData: MAKE_ITEM_LARECODE loaded");
	return true;
}

bool AIServerApp::GetMakeItemGroupTableData()
{
	recordset_loader::STLMap loader(_makeItemGroupTableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error(
			"AIServerApp::GetMakeItemGroupTableData: load failed - {}", loader.GetError().Message);
		return false;
	}

	spdlog::info("AIServerApp::GetMakeItemGroupTableData: MAKE_ITEM_GROUP loaded");
	return true;
}

/////////////////////////////////////////////////////////////////////////
//	NPC Item Table 을 읽는다.
//
bool AIServerApp::GetNpcItemTable()
{
	using ModelType = model::MonsterItem;

	std::vector<ModelType*> rows;

	recordset_loader::Vector<ModelType> loader(rows);
	if (!loader.Load_ForbidEmpty(true))
	{
		spdlog::error("AIServerApp::GetNpcItemTable: load failed - {}", loader.GetError().Message);
		return false;
	}

	_npcItem.m_nField = loader.GetColumnCount();
	_npcItem.m_nRow   = static_cast<int>(rows.size());

	if (rows.empty())
		return false;

	_npcItem.m_ppItem = new int*[_npcItem.m_nRow];
	for (int i = 0; i < _npcItem.m_nRow; i++)
		_npcItem.m_ppItem[i] = new int[_npcItem.m_nField];

	for (size_t i = 0; i < rows.size(); i++)
	{
		ModelType* row           = rows[i];

		_npcItem.m_ppItem[i][0]  = row->MonsterId;
		_npcItem.m_ppItem[i][1]  = row->ItemId[0];
		_npcItem.m_ppItem[i][2]  = row->DropChance[0];
		_npcItem.m_ppItem[i][3]  = row->ItemId[1];
		_npcItem.m_ppItem[i][4]  = row->DropChance[1];
		_npcItem.m_ppItem[i][5]  = row->ItemId[2];
		_npcItem.m_ppItem[i][6]  = row->DropChance[2];
		_npcItem.m_ppItem[i][7]  = row->ItemId[3];
		_npcItem.m_ppItem[i][8]  = row->DropChance[3];
		_npcItem.m_ppItem[i][9]  = row->ItemId[4];
		_npcItem.m_ppItem[i][10] = row->DropChance[4];

		delete row;
	}

	rows.clear();

	spdlog::info("AIServerApp::GetNpcItemTable: K_MONSTER_ITEM loaded");
	return true;
}

bool AIServerApp::GetZoneInfoTableData()
{
	recordset_loader::STLMap loader(_zoneInfoTableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error(
			"AIServerApp::GetZoneInfoTableData: load failed - {}", loader.GetError().Message);
		return false;
	}

	spdlog::info("AIServerApp::GetZoneInfoTableData: ZONE_INFO loaded");
	return true;
}

//	Monster Table Data 를 읽는다.
bool AIServerApp::GetMonsterTableData()
{
	NpcTableMap tableMap;
	recordset_loader::STLMap<NpcTableMap, model::Monster> loader(tableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error(
			"AIServerApp::GetMonsterTableData: load failed - {}", loader.GetError().Message);
		return false;
	}

#if defined(DB_COMPAT_PADDED_NAMES)
	for (const auto& [_, row] : tableMap)
	{
		if (row->Name.has_value())
			rtrim(*row->Name);
	}
#endif

	_monTableMap.Swap(tableMap);

	spdlog::info("AIServerApp::GetMonsterTableData: K_MONSTER loaded");
	return true;
}

//	NPC Table Data 를 읽는다. (경비병 & NPC)
bool AIServerApp::GetNpcTableData()
{
	NpcTableMap tableMap;
	recordset_loader::STLMap loader(tableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("AIServerApp::GetNpcTableData: load failed - {}", loader.GetError().Message);
		return false;
	}

#if defined(DB_COMPAT_PADDED_NAMES)
	for (const auto& [_, row] : tableMap)
	{
		if (row->Name.has_value())
			rtrim(*row->Name);
	}
#endif

	_npcTableMap.Swap(tableMap);

	spdlog::info("AIServerApp::GetNpcTableData: K_NPC loaded");
	return true;
}

//	Npc Thread 를 만든다.
bool AIServerApp::CreateNpcThread()
{
	_totalNpcCount  = 0; // DB에 있는 수
	_loadedNpcCount = 0;

	std::vector<model::NpcPos*> rows;
	if (!LoadNpcPosTable(rows))
	{
		spdlog::error("AIServerApp::CreateNpcThread: K_NPCPOS load failed");
		return false;
	}

	for (model::NpcPos* row : rows)
		delete row;
	rows.clear();

	int step               = 0;
	int nThreadNumber      = 0;
	CNpcThread* pNpcThread = nullptr;

	for (auto& [_, pNpc] : _npcMap)
	{
		if (step == 0)
			pNpcThread = new CNpcThread();

		pNpcThread->m_pNpc[step]                  = pNpc;
		pNpcThread->m_pNpc[step]->m_sThreadNumber = nThreadNumber;
		pNpcThread->m_pNpc[step]->Init();

		++step;

		if (step == NPC_NUM)
		{
			pNpcThread->m_sThreadNumber = nThreadNumber++;
			_npcThreads.push_back(pNpcThread);
			step = 0;
		}
	}

	if (step != 0)
	{
		pNpcThread->m_sThreadNumber = nThreadNumber++;
		_npcThreads.push_back(pNpcThread);
	}

	if (_zoneEventThread == nullptr)
		_zoneEventThread = new ZoneEventThread();

	spdlog::info("AIServerApp::CreateNpcThread: Monsters/NPCs loaded: {}", _totalNpcCount);
	return true;
}

bool AIServerApp::LoadNpcPosTable(std::vector<model::NpcPos*>& rows)
{
	CRoomEvent* pRoom = nullptr;

	recordset_loader::Vector<model::NpcPos> loader(rows);
	if (!loader.Load_ForbidEmpty(true))
	{
		spdlog::error("AIServerApp::LoadNpcPosTable: failed - {}", loader.GetError().Message);
		return false;
	}

	int nSerial = _mapEventNpcCount;

	for (model::NpcPos* row : rows)
	{
		bool bMoveNext  = true;
		int nPathSerial = 1;
		int nNpcCount   = 0;

		do
		{
			int nMonsterNumber = row->NumNpc;
			int nServerNum     = GetServerNumber(row->ZoneId);

			if (_serverZoneType == nServerNum || _serverZoneType == UNIFY_ZONE)
			{
				for (int j = 0; j < nMonsterNumber; j++)
				{
					CNpc* pNpc             = new CNpc();
					pNpc->m_sNid           = nSerial++;            // 서버 내에서의 고유 번호
					pNpc->m_sSid           = (int16_t) row->NpcId; // MONSTER(NPC) Serial ID

					pNpc->m_byMoveType     = row->ActType;
					pNpc->m_byInitMoveType = row->ActType;
					pNpc->m_byDirection    = row->Direction;

					model::Npc* pNpcTable  = nullptr;

					if (row->ActType >= 0 && row->ActType < 100)
					{
						pNpcTable = _monTableMap.GetData(pNpc->m_sSid);
					}
					else if (row->ActType >= 100)
					{
						pNpc->m_byMoveType = row->ActType - 100;
						//pNpc->m_byInitMoveType = row->ActType - 100;

						pNpcTable          = _npcTableMap.GetData(pNpc->m_sSid);
					}

					pNpc->m_byBattlePos = 0;

					if (pNpc->m_byMoveType >= 2)
					{
						pNpc->m_byBattlePos = myrand(1, 3);
						pNpc->m_byPathCount = nPathSerial++;
					}

					pNpc->InitPos();

					if (pNpcTable == nullptr)
					{
						spdlog::error(
							"AIServerApp::LoadNpcPosTable: npc not found [serial={}, npcId={}]",
							pNpc->m_sNid, pNpc->m_sSid);
						break;
					}

					if (bMoveNext)
					{
						bMoveNext = false;
						nNpcCount = row->NumNpc;
					}

					pNpc->Load(pNpcTable, true);

					//////// MONSTER POS ////////////////////////////////////////
					pNpc->m_sCurZone = row->ZoneId;

					float fRandom_X = 0.0f, fRandom_Z = 0.0f;

					// map에 몬스터의 위치를 랜덤하게 위치시킨다.. (테스트 용 : 수정-DB에서 읽어오는데로 몬 위치 결정)
					int nRandom = abs(row->LeftX - row->RightX);
					if (nRandom <= 1)
					{
						fRandom_X = (float) row->LeftX;
					}
					else
					{
						if (row->LeftX < row->RightX)
							fRandom_X = (float) myrand(row->LeftX, row->RightX);
						else
							fRandom_X = (float) myrand(row->RightX, row->LeftX);
					}

					nRandom = abs(row->TopZ - row->BottomZ);
					if (nRandom <= 1)
					{
						fRandom_Z = (float) row->TopZ;
					}
					else
					{
						if (row->TopZ < row->BottomZ)
							fRandom_Z = (float) myrand(row->TopZ, row->BottomZ);
						else
							fRandom_Z = (float) myrand(row->BottomZ, row->TopZ);
					}

					pNpc->m_fCurX = fRandom_X;
					pNpc->m_fCurY = 0;
					pNpc->m_fCurZ = fRandom_Z;

					if (row->RespawnTime < 15)
					{
						spdlog::warn("AIServerApp::LoadNpcPosTable: RegTime below minimum value of "
									 "15s [npcId={}, serial={}, npcName={}, RegTime={}]",
							pNpc->m_sSid, pNpc->m_sNid + NPC_BAND, pNpc->m_strName,
							row->RespawnTime);
						// TODO: Set this to 15 in separate ticket and comment on it deviating from official behavior
						row->RespawnTime = 30;
					}

					pNpc->m_sRegenTime    = row->RespawnTime * 1000; // 초(DB)단위-> 밀리세컨드로

					pNpc->m_sMaxPathCount = row->PathPointCount;

					if (pNpc->m_byMoveType == 2 || pNpc->m_byMoveType == 3)
					{
						if (row->PathPointCount == 0 || !row->Path.has_value())
						{
							spdlog::error("AIServerApp::LoadNpcPosTable: NPC expects path to be "
										  "set [zoneId={} serial={}, npcId={}, npcName={}, "
										  "moveType={}, pathCount={}]",
								row->ZoneId, pNpc->m_sNid + NPC_BAND, pNpc->m_sSid, pNpc->m_strName,
								pNpc->m_byMoveType, pNpc->m_sMaxPathCount);
							return false;
						}
					}

					int index = 0;

					if (row->PathPointCount != 0 && row->Path.has_value())
					{
						// The path is a series of points (x,z), each in the form ("%04d%04d", x, z)
						// As such, we expect there to be at least 8 characters per point.
						constexpr size_t CharactersPerPoint = 8;

						const std::string& path             = *row->Path;
						if ((row->PathPointCount * CharactersPerPoint) > path.size())
						{
							spdlog::error("LoadNpcPosTable: NPC expects a larger path for this "
										  "PathPointCount [zoneId={} serial={} npcId={} npcName={} "
										  "moveType={}, pathCount={}]",
								row->ZoneId, row->PathPointCount, pNpc->m_sNid + NPC_BAND,
								pNpc->m_sSid, pNpc->m_strName, pNpc->m_byMoveType,
								pNpc->m_sMaxPathCount);
							return false;
						}

						for (int l = 0; l < row->PathPointCount; l++)
						{
							char szX[5] {}, szZ[5] {};
							GetString(szX, path.c_str(), 4, index);
							GetString(szZ, path.c_str(), 4, index);
							pNpc->m_PathList.pPattenPos[l].x = atoi(szX);
							pNpc->m_PathList.pPattenPos[l].z = atoi(szZ);
							//	TRACE(_T(" l=%d, x=%d, z=%d\n"), l, pNpc->m_PathList.pPattenPos[l].x, pNpc->m_PathList.pPattenPos[l].z);
						}
					}

					pNpc->m_nInitMinX = pNpc->m_nLimitMinX = row->LeftX;
					pNpc->m_nInitMinY = pNpc->m_nLimitMinZ = row->TopZ;
					pNpc->m_nInitMaxX = pNpc->m_nLimitMaxX = row->RightX;
					pNpc->m_nInitMaxY = pNpc->m_nLimitMaxZ = row->BottomZ;
					// dungeon work
					pNpc->m_byDungeonFamily                = row->DungeonFamily;
					pNpc->m_bySpecialType                  = row->SpecialType;
					pNpc->m_byRegenType                    = row->RegenType;
					pNpc->m_byTrapNumber                   = row->TrapNumber;

					if (pNpc->m_byDungeonFamily > 0)
					{
						pNpc->m_nLimitMinX = row->LimitMinX;
						pNpc->m_nLimitMinZ = row->LimitMinZ;
						pNpc->m_nLimitMaxX = row->LimitMaxX;
						pNpc->m_nLimitMaxZ = row->LimitMaxZ;
					}

					pNpc->m_ZoneIndex = -1;

					MAP* pMap         = nullptr;
					for (size_t i = 0; i < _zones.size(); i++)
					{
						if (_zones[i]->m_nZoneNumber == pNpc->m_sCurZone)
						{
							pNpc->m_ZoneIndex = static_cast<int16_t>(i);
							pMap              = _zones[i];
							break;
						}
					}

					if (pMap == nullptr)
					{
						spdlog::error("AIServerApp::LoadNpcPosTable: NPC invalid zone [npcId:{}, "
									  "npcZoneId:{}]",
							pNpc->m_sSid, pNpc->m_sCurZone);
						return false;
					}

					//pNpc->Init();
					//_npcMap.Add(pNpc);
					if (!_npcMap.PutData(pNpc->m_sNid, pNpc))
					{
						spdlog::error("AIServerApp::LoadNpcPosTable: Npc PutData Fail [serial={}]",
							pNpc->m_sNid);
						delete pNpc;
						pNpc = nullptr;
					}

					if (pNpc != nullptr && pMap->m_byRoomEvent > 0 && pNpc->m_byDungeonFamily > 0)
					{
						pRoom = pMap->m_arRoomEventArray.GetData(pNpc->m_byDungeonFamily);
						if (pRoom == nullptr)
						{
							spdlog::error(
								"AIServerApp::LoadNpcPosTable: No RoomEvent for NPC dungeonFamily: "
								"serial={}, npcId={}, npcName={}, dungeonFamily={}, zoneId={}",
								pNpc->m_sNid + NPC_BAND, pNpc->m_sSid, pNpc->m_strName,
								pNpc->m_byDungeonFamily, pNpc->m_ZoneIndex);
							return false;
						}

						int* pInt = new int;
						*pInt     = pNpc->m_sNid;
						if (!pRoom->m_mapRoomNpcArray.PutData(pNpc->m_sNid, pInt))
						{
							delete pInt;
							spdlog::error("AIServerApp::LoadNpcPosTable: MapRoomNpcArray.PutData "
										  "failed for NPC: [serial={}, npcId={}]",
								pNpc->m_sNid, pNpc->m_sSid);
						}
					}

					_totalNpcCount = nSerial;

					if (--nNpcCount > 0)
						continue;

					bMoveNext = true;
					nNpcCount = 0;
				}
			}
		}
		while (!bMoveNext);
	}

	return true;
}

//	NPC Thread 들을 작동시킨다.
void AIServerApp::StartNpcThreads()
{
	for (CNpcThread* npcThread : _npcThreads)
		npcThread->start();

	_zoneEventThread->start();
}

void AIServerApp::DeleteUserList(int uid)
{
	if (uid < 0 || uid >= MAX_USER)
	{
		spdlog::error("AIServerApp::DeleteUserList: userId invalid: {}", uid);
		return;
	}

	std::string characterName;

	std::unique_lock<std::mutex> lock(g_user_mutex);

	CUser* pUser = _users[uid];
	if (pUser == nullptr)
	{
		lock.unlock();
		spdlog::error("AIServerApp::DeleteUserList: userId not found: {}", uid);
		return;
	}

	if (pUser->m_iUserId != uid)
	{
		lock.unlock();
		spdlog::warn("AIServerApp::DeleteUserList: userId mismatch : userId={} pUserId={}", uid,
			pUser->m_iUserId);
		return;
	}

	characterName  = pUser->m_strUserID;

	pUser->m_lUsed = 1;
	delete _users[uid];
	_users[uid] = nullptr;

	lock.unlock();
	spdlog::debug(
		"AIServerApp::DeleteUserList: User Logout: userId={}, charId={}", uid, characterName);
}

bool AIServerApp::MapFileLoad()
{
	using ModelType = model::ZoneInfo;

	bool loaded     = false;

	_mapCount       = 0;

	recordset_loader::Base<ModelType> loader;
	loader.SetProcessFetchCallback(
		[&](db::ModelRecordSet<ModelType>& recordset)
		{
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
					spdlog::error("AIServerApp::MapFileLoad: Failed to open file: {}", filename);
					return;
				}

				MAP* pMap           = new MAP();
				pMap->m_nServerNo   = row.ServerId;
				pMap->m_nZoneNumber = row.ZoneId;

				if (!pMap->LoadMap(file))
				{
					spdlog::error(
						"AIServerApp::MapFileLoad: Failed to load map file: {}", filename);
					delete pMap;
					return;
				}

				file.Close();

				// dungeon work
				if (row.RoomEvent > 0)
				{
					if (!pMap->LoadRoomEvent(row.RoomEvent, _eventDir))
					{
						spdlog::error(
							"AIServerApp::MapFileLoad: LoadRoomEvent failed: {}", filename);
						delete pMap;
						return;
					}

					pMap->m_byRoomEvent = 1;
				}

				_zones.push_back(pMap);
				++_mapCount;
			}
			while (recordset.next());

			loaded = true;
		});

	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error(
			"AIServerApp::MapFileLoad: load failed (ZONE_INFO) - {}", loader.GetError().Message);
		return false;
	}

	return loaded;
}

// sungyong 2002.05.23
// game server에 모든 npc정보를 전송..
void AIServerApp::AllNpcInfo()
{
	// server alive check
	CNpc* pNpc    = nullptr;
	int nZone     = 0;
	int size      = _npcMap.GetSize();

	int sendIndex = 0;
	int count = 0, send_count = 0, send_tot = 0;
	char sendBuffer[2048] {};

	for (MAP* pMap : _zones)
	{
		if (pMap == nullptr)
			continue;

		nZone = pMap->m_nZoneNumber;

		memset(sendBuffer, 0, sizeof(sendBuffer));
		sendIndex = 0;
		SetByte(sendBuffer, AG_SERVER_INFO, sendIndex);
		SetByte(sendBuffer, SERVER_INFO_START, sendIndex);
		SetByte(sendBuffer, nZone, sendIndex);
		Send(sendBuffer, sendIndex, nZone);

		sendIndex              = 2;
		count                  = 0;
		send_count             = 0;
		_compressedPacketCount = 0;
		_compressedPacketIndex = 0;
		memset(sendBuffer, 0, sizeof(sendBuffer));

		spdlog::debug("AIServerApp::AllNpcInfo: start for zoneIndex={}", nZone);

		for (int i = 0; i < size; i++)
		{
			pNpc = _npcMap.GetData(i);
			if (pNpc == nullptr)
			{
				spdlog::warn("AIServerApp::AllNpcInfo: NpcMap[{}] is null", i);
				continue;
			}

			if (pNpc->m_sCurZone != nZone)
				continue;

			pNpc->SendNpcInfoAll(sendBuffer, sendIndex, count);
			count++;

			if (count == NPC_NUM)
			{
				SetByte(sendBuffer, NPC_INFO_ALL, send_count);
				SetByte(sendBuffer, (uint8_t) count, send_count);
				_compressedPacketCount++;
				//memcpy(_compressedPacketBuffer+_compressedPacketIndex, sendBuffer, sendIndex);
				memset(_compressedPacketBuffer, 0, sizeof(_compressedPacketBuffer));
				memcpy(_compressedPacketBuffer, sendBuffer, sendIndex);
				_compressedPacketIndex = sendIndex;
				SendCompressedData(nZone);
				sendIndex  = 2;
				send_count = 0;
				count      = 0;
				send_tot++;
				spdlog::trace("AIServerApp::AllNpcInfo: send_count={}, count={}, zone={}", send_tot,
					count, nZone);
				memset(sendBuffer, 0, sizeof(sendBuffer));
				std::this_thread::sleep_for(50ms);
			}
		}

		//if( count != 0 )	TRACE(_T("--> AllNpcInfo - send_count=%d, count=%d, zone=%d\n"), send_tot, count, nZone);
		if (count != 0 && count < NPC_NUM)
		{
			send_count = 0;
			SetByte(sendBuffer, NPC_INFO_ALL, send_count);
			SetByte(sendBuffer, (uint8_t) count, send_count);
			Send(sendBuffer, sendIndex, nZone);
			send_tot++;
			spdlog::trace("AIServerApp::AllNpcInfo: send_count={}, count={}, zone={}", send_tot,
				count, nZone);
			std::this_thread::sleep_for(50ms);
		}

		sendIndex = 0;
		memset(sendBuffer, 0, sizeof(sendBuffer));
		SetByte(sendBuffer, AG_SERVER_INFO, sendIndex);
		SetByte(sendBuffer, SERVER_INFO_END, sendIndex);
		SetByte(sendBuffer, nZone, sendIndex);
		SetShort(sendBuffer, (int16_t) _totalNpcCount, sendIndex);
		Send(sendBuffer, sendIndex, nZone);

		spdlog::debug("AIServerApp::AllNpcInfo: end for zoneId={}", nZone);
	}

	std::this_thread::sleep_for(1s);
}
// ~sungyong 2002.05.23

CUser* AIServerApp::GetUserPtr(int nid)
{
	CUser* pUser = nullptr;

	if (nid < 0 || nid >= MAX_USER)
	{
		if (nid != -1)
			spdlog::error("AIServerApp::GetUserPtr: User Array Overflow [{}]", nid);

		return nullptr;
	}

	/*	if( !m_ppUserActive[nid] )
		return nullptr;

	if( m_ppUserActive[nid]->m_lUsed == 1 ) return nullptr;	// 포인터 사용을 허락치 않음.. (logout중)

	pUser = (CUser*)m_ppUserActive[nid];
*/
	pUser = _users[nid];
	if (pUser == nullptr)
		return nullptr;

	// 포인터 사용을 허락치 않음.. (logout중)
	if (pUser->m_lUsed == 1)
		return nullptr;

	if (pUser->m_iUserId < 0 || pUser->m_iUserId >= MAX_USER)
		return nullptr;

	if (pUser->m_iUserId == nid)
		return pUser;

	return nullptr;
}

// sungyong 2002.05.23
void AIServerApp::CheckAliveTest()
{
	int sendIndex = 0;
	char sendBuffer[256] {};

	SetByte(sendBuffer, AG_CHECK_ALIVE_REQ, sendIndex);

	int size = 0, count = 0;
	int socketCount = _serverSocketManager.GetSocketCount();
	for (int i = 0; i < socketCount; i++)
	{
		auto socket = _serverSocketManager.GetSocketUnchecked(i);
		if (socket == nullptr)
			continue;

		size = socket->Send(sendBuffer, sendIndex);
		if (size > 0)
		{
			if (++_aliveSocketCount == MAX_AI_SOCKET)
				spdlog::debug("AIServerApp::CheckAliveTest: all Ebenezer sockets are connected");

			count++;
		}
		//TRACE(_T("size = %d, socket_num = %d, i=%d \n"), size, socket->m_sSocketID, i);
	}

	if (count <= 0)
		DeleteAllUserList(9999);

	RegionCheck();
}

void AIServerApp::DeleteAllUserList(int zone)
{
	if (zone < 0)
		return;

	// 모든 소켓이 끊어진 상태...
	if (zone == 9999 && _firstServerFlag)
	{
		spdlog::debug("AIServerApp::DeleteAllUserList: start");

		{
			std::lock_guard<std::mutex> lock(g_region_mutex);

			for (MAP* pMap : _zones)
			{
				if (pMap == nullptr)
					continue;

				for (int i = 0; i < pMap->m_sizeRegion.cx; i++)
				{
					for (int j = 0; j < pMap->m_sizeRegion.cy; j++)
						pMap->m_ppRegion[i][j].m_RegionUserArray.DeleteAllData();
				}
			}
		}

		{
			std::lock_guard<std::mutex> lock(g_user_mutex);

			// User Array Delete
			for (int i = 0; i < MAX_USER; i++)
			{
				delete _users[i];
				_users[i] = nullptr;
			}
		}

		// Party Array Delete
		{
			std::lock_guard<std::mutex> lock(g_region_mutex);
			_partyMap.DeleteAllData();
		}

		_firstServerFlag = false;
		spdlog::debug("AIServerApp::DeleteAllUserList: end");
	}
	else if (zone != 9999)
	{
		spdlog::info("AIServerApp::DeleteAllUserList: ebenezer zone {} disconnected", zone);
	}
}
// ~sungyong 2002.05.23

void AIServerApp::SendCompressedData(int nZone)
{
	if (_compressedPacketCount <= 0 || _compressedPacketIndex <= 0)
	{
		_compressedPacketCount = 0;
		_compressedPacketIndex = 0;
		spdlog::error("AIServerApp::SendCompressData: count={}, index={}", _compressedPacketCount,
			_compressedPacketIndex);
		return;
	}

	int sendIndex              = 0;
	unsigned int comp_data_len = 0;
	uint32_t crc_value         = 0;

	char sendBuffer[32000] {};
	uint8_t comp_buff[32000] {};

	comp_data_len = lzf_compress(
		_compressedPacketBuffer, _compressedPacketIndex, comp_buff, sizeof(comp_buff));

	assert(comp_data_len != 0 && comp_data_len <= sizeof(comp_buff));

	if (comp_data_len == 0 || comp_data_len > sizeof(comp_buff))
	{
		spdlog::error("AIServerApp::SendCompressedData: Failed to compress packet");
		return;
	}

	crc_value = crc32(reinterpret_cast<uint8_t*>(_compressedPacketBuffer), _compressedPacketIndex);

	SetByte(sendBuffer, AG_COMPRESSED_DATA, sendIndex);
	SetShort(sendBuffer, (int16_t) comp_data_len, sendIndex);
	SetShort(sendBuffer, (int16_t) _compressedPacketIndex, sendIndex);
	SetDWORD(sendBuffer, crc_value, sendIndex);
	SetShort(sendBuffer, (int16_t) _compressedPacketCount, sendIndex);
	SetString(sendBuffer, reinterpret_cast<const char*>(comp_buff), comp_data_len, sendIndex);

	Send(sendBuffer, sendIndex, nZone);

	_compressedPacketCount = 0;
	_compressedPacketIndex = 0;
}

// sungyong 2002.05.23
int AIServerApp::Send(const char* pData, int length, int nZone)
{
	// Not connected to any servers.
	// No point queueing updates, the server will be fully synced upon connection.
	if (!_firstServerFlag)
		return 0;

	if (length <= 0 || length > static_cast<int>(sizeof(_SEND_DATA::pBuf)))
		return 0;

	_SEND_DATA* pNewData = new _SEND_DATA;
	if (pNewData == nullptr)
		return 0;

	pNewData->sCurZone = nZone;
	pNewData->sLength  = length;
	memcpy(pNewData->pBuf, pData, length);

	_serverSocketManager.QueueSendData(pNewData);

	return 0;
}
// ~sungyong 2002.05.23

void AIServerApp::GameServerAcceptThread()
{
	_serverSocketManager.StartAccept();
}

void AIServerApp::SyncTest()
{
	spdlog::info("AIServerApp::SyncTest: begin");

	int sendIndex = 0;
	char sendBuffer[256] {};

	SetByte(sendBuffer, AG_CHECK_ALIVE_REQ, sendIndex);

	int socketCount = _serverSocketManager.GetSocketCount();
	for (int i = 0; i < socketCount; i++)
	{
		auto socket = _serverSocketManager.GetSocketUnchecked(i);
		if (socket == nullptr)
			continue;

		int size = socket->Send(sendBuffer, sendIndex);
		spdlog::info("AIServerApp::SyncTest: size={}, zoneNo={}", size, socket->_zoneNo);
	}
}

bool AIServerApp::GetMagicType1Data()
{
	recordset_loader::STLMap loader(_magicType1TableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error(
			"AIServerApp::GetMagicType1Data: load failed - {}", loader.GetError().Message);
		return false;
	}

	spdlog::info("AIServerApp::GetMagicType1Data: MAGIC_TYPE1 loaded");
	return true;
}

bool AIServerApp::GetMagicType2Data()
{
	recordset_loader::STLMap loader(_magicType2TableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error(
			"AIServerApp::GetMagicType2Data: load failed - {}", loader.GetError().Message);
		return false;
	}

	spdlog::info("AIServerApp::GetMagicType2Data: MAGIC_TYPE2 loaded");
	return true;
}

bool AIServerApp::GetMagicType3Data()
{
	recordset_loader::STLMap loader(_magicType3TableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error(
			"AIServerApp::GetMagicType3Data: load failed - {}", loader.GetError().Message);
		return false;
	}

	spdlog::info("AIServerApp::GetMagicType3Data: MAGIC_TYPE3 loaded");
	return true;
}

bool AIServerApp::GetMagicType4Data()
{
	recordset_loader::STLMap loader(_magicType4TableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error(
			"AIServerApp::GetMagicType4Data: load failed - {}", loader.GetError().Message);
		return false;
	}

	spdlog::info("AIServerApp::GetMagicType4Data: MAGIC_TYPE4 loaded");
	return true;
}

bool AIServerApp::GetMagicType7Data()
{
	recordset_loader::STLMap loader(_magicType7TableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error(
			"AIServerApp::GetMagicType7Data: load failed - {}", loader.GetError().Message);
		return false;
	}

	spdlog::info("AIServerApp::GetMagicType7Data: MAGIC_TYPE7 loaded");
	return true;
}

void AIServerApp::RegionCheck()
{
	for (MAP* pMap : _zones)
	{
		if (pMap == nullptr)
			continue;

		int total_user = 0;
		for (int i = 0; i < pMap->m_sizeRegion.cx; i++)
		{
			for (int j = 0; j < pMap->m_sizeRegion.cy; j++)
			{
				{
					std::lock_guard<std::mutex> lock(g_user_mutex);
					total_user = pMap->m_ppRegion[i][j].m_RegionUserArray.GetSize();
				}

				if (total_user > 0)
					pMap->m_ppRegion[i][j].m_byMoving = 1;
				else
					pMap->m_ppRegion[i][j].m_byMoving = 0;
			}
		}
	}
}

bool AIServerApp::AddObjectEventNpc(_OBJECT_EVENT* pEvent, int zone_number)
{
	model::Npc* pNpcTable = nullptr;
	int nServerNum        = GetServerNumber(zone_number);
	if (_serverZoneType != nServerNum)
		return false;

	pNpcTable = _npcTableMap.GetData(pEvent->sIndex);
	if (pNpcTable == nullptr)
	{
		spdlog::error("AIServerApp::AddObjectEventNpc error: eventId={} zoneId={}", pEvent->sIndex,
			zone_number);
		return false;
	}

	CNpc* pNpc             = new CNpc();

	pNpc->m_sNid           = _mapEventNpcCount++;      // 서버 내에서의 고유 번호
	pNpc->m_sSid           = (int16_t) pEvent->sIndex; // MONSTER(NPC) Serial ID

	pNpc->m_byMoveType     = 100;
	pNpc->m_byInitMoveType = 100;

	pNpc->m_byMoveType     = 0;
	pNpc->m_byInitMoveType = 0;

	pNpc->m_byBattlePos    = 0;

	pNpc->m_fSecForMetor   = 4.0f; // 초당 갈 수 있는 거리..

	pNpc->Load(pNpcTable, false);

	//////// MONSTER POS ////////////////////////////////////////

	pNpc->m_sCurZone      = zone_number;

	pNpc->m_byGateOpen    = static_cast<uint8_t>(pEvent->sStatus);
	pNpc->m_fCurX         = pEvent->fPosX;
	pNpc->m_fCurY         = pEvent->fPosY;
	pNpc->m_fCurZ         = pEvent->fPosZ;

	pNpc->m_nInitMinX     = static_cast<int>(pEvent->fPosX - 1);
	pNpc->m_nInitMinY     = static_cast<int>(pEvent->fPosZ - 1);
	pNpc->m_nInitMaxX     = static_cast<int>(pEvent->fPosX + 1);
	pNpc->m_nInitMaxY     = static_cast<int>(pEvent->fPosZ + 1);

	pNpc->m_sRegenTime    = 10000 * 1000; // 초(DB)단위-> 밀리세컨드로
	//pNpc->m_sRegenTime		= 30 * 1000;	// 초(DB)단위-> 밀리세컨드로
	pNpc->m_sMaxPathCount = 0;

	pNpc->m_ZoneIndex     = -1;
	pNpc->m_byObjectType  = SPECIAL_OBJECT;
	pNpc->m_bFirstLive    = 1; // 처음 살아난 경우로 해줘야 한다..
	//pNpc->m_ZoneIndex = GetZoneIndex(pNpc->m_sCurZone);
	/*
	if(pNpc->m_ZoneIndex == -1)	{
		return false;
	}	*/

	//pNpc->Init();
	if (!_npcMap.PutData(pNpc->m_sNid, pNpc))
	{
		spdlog::warn("AIServerApp::AddObjectEventNpc: Npc PutData Fail [serial={}]", pNpc->m_sNid);
		delete pNpc;
		pNpc = nullptr;
	}

	_totalNpcCount = _mapEventNpcCount;

	return true;
}

int AIServerApp::GetZoneIndex(int zoneId) const
{
	for (size_t i = 0; i < _zones.size(); i++)
	{
		MAP* pMap = _zones[i];
		if (pMap != nullptr && pMap->m_nZoneNumber == zoneId)
			return static_cast<int>(i);
	}

	spdlog::error("AIServerApp::GetZoneIndex: zoneId={} not found", zoneId);
	return -1;
}

int AIServerApp::GetServerNumber(int zoneId) const
{
	const model::ZoneInfo* zoneInfo = _zoneInfoTableMap.GetData(zoneId);
	if (zoneInfo == nullptr)
	{
		spdlog::error("AIServerApp::GetServerNumber: zoneId={} not found", zoneId);
		return -1;
	}

	return zoneInfo->ServerId;
}

void AIServerApp::SetupCommandLineArgParser(argparse::ArgumentParser& parser)
{
	AppThread::SetupCommandLineArgParser(parser);

	parser.add_argument("--map-dir")
		.help("location of directory containing server map data files (.SMDs)")
		.store_into(_overrideMapDir);

	parser.add_argument("--event-dir")
		.help("location of directory containing the AI server specific event scripts (.EVTs)")
		.store_into(_overrideEventDir);
}

bool AIServerApp::ProcessCommandLineArgs(const argparse::ArgumentParser& parser)
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

	if (!_overrideEventDir.empty() && !std::filesystem::exists(_overrideEventDir, ec))
	{
		spdlog::error("Supplied EVT directory (--event-dir) doesn't exist or is inaccessible: {}",
			_overrideEventDir);
		return false;
	}

	return true;
}

std::filesystem::path AIServerApp::ConfigPath() const
{
	return "server.ini";
}

bool AIServerApp::LoadConfig(CIni& iniFile)
{
	_serverZoneType            = iniFile.GetInt("SERVER", "ZONE", 1);

	std::string datasourceName = iniFile.GetString("ODBC", "GAME_DSN", "KN_online");
	std::string datasourceUser = iniFile.GetString("ODBC", "GAME_UID", "knight");
	std::string datasourcePass = iniFile.GetString("ODBC", "GAME_PWD", "knight");

	ConnectionManager::SetDatasourceConfig(
		modelUtil::DbType::GAME, datasourceName, datasourceUser, datasourcePass);

	// Load paths from config
	std::string mapDir   = iniFile.GetString("PATH", "MAP_DIR", "");
	std::string eventDir = iniFile.GetString("PATH", "EVENT_DIR", "");

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

	// EVT directory supplied from command-line.
	// Replace it in the config -- but only if it's not explicitly been set already.
	// We should always use the map directory passed from command-line over the INI.
	if (!_overrideEventDir.empty())
	{
		if (eventDir.empty())
			iniFile.SetString("PATH", "EVENT_DIR", _overrideEventDir);

		_eventDir = _overrideEventDir;
	}
	// No command-line override is present, but it is configured in the INI.
	// We should use that.
	else if (!eventDir.empty())
	{
		_eventDir = eventDir;

		if (!std::filesystem::exists(_eventDir, ec))
		{
			spdlog::error(
				"Configured EVT directory doesn't exist or is inaccessible: {}", eventDir);
			return false;
		}
	}
	// Fallback to the default (don't save this).
	else
	{
		// NOTE: We have the AI server EVTs in the MAP dir by default.
		// We only separate Ebenezer's EVTs into QUESTS.
		_eventDir = DEFAULT_MAP_DIR;
	}

	// Resolve the path to strip the relative references (to be nice).
	if (std::filesystem::exists(_eventDir, ec))
		_eventDir = std::filesystem::canonical(_eventDir, ec);

	return true;
}

void AIServerApp::SendSystemMsg(const std::string_view msg, int zone, int type, int who)
{
	int sendIndex = 0;
	char buff[256] {};

	SetByte(buff, AG_SYSTEM_MSG, sendIndex);
	SetByte(buff, type, sendIndex); // 채팅형식
	SetShort(buff, who, sendIndex); // 누구에게
	SetString2(buff, msg, sendIndex);
	Send(buff, sendIndex, zone);

	spdlog::info(
		"AIServerApp::SendSystemMsg: zoneId={} type={} who={} msg={}", zone, type, who, msg);
}

void AIServerApp::ResetBattleZone()
{
	spdlog::debug("AIServerApp::ResetBattleZone: start");

	for (MAP* pMap : _zones)
	{
		if (pMap == nullptr)
			continue;

		// 현재의 존이 던젼담당하는 존이 아니면 리턴..
		if (pMap->m_byRoomEvent == 0)
			continue;

		// 전체방이 클리어 되었다면
		// if (pMap->IsRoomStatusCheck())
		//	continue;

		pMap->InitializeRoom();
	}

	spdlog::debug("AIServerApp::ResetBattleZone: end");
}

MAP* AIServerApp::GetMapByIndex(int iZoneIndex) const
{
	if (iZoneIndex < 0 || iZoneIndex >= static_cast<int>(_zones.size()))
	{
		spdlog::error("AIServerApp::GetMapByIndex: zoneIndex={} out of bounds", iZoneIndex);
		return nullptr;
	}

	return _zones[iZoneIndex];
}

MAP* AIServerApp::GetMapByID(int iZoneID) const
{
	for (MAP* pMap : _zones)
	{
		if (pMap != nullptr && pMap->m_nZoneNumber == iZoneID)
			return pMap;
	}

	spdlog::error("AIServerApp::GetMapByID: no map found for zoneId={}", iZoneID);
	return nullptr;
}

} // namespace AIServer

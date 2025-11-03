// AiServerInstance.cpp : implementation file
//

#include "pch.h"
#include "AiServerInstance.h"
#include "GameSocket.h"
#include "NpcThread.h"
#include "Region.h"
#include "ZoneEventThread.h"

#include <shared/crc32.h>
#include <shared/lzf.h>
#include <shared/globals.h>
#include <shared/Ini.h>
#include <shared/StringConversion.h>
#include <shared/StringUtils.h>

#include <spdlog/spdlog.h>

#include <db-library/ConnectionManager.h>

#include <math.h>
#include <fstream>
#include <db-library/RecordSetLoader_STLMap.h>
#include <db-library/RecordsetLoader_Vector.h>
#include <shared/TimerThread.h>

using namespace std::chrono_literals;

bool g_bNpcExit = false;
ZoneArray m_ZoneArray;

std::mutex g_user_mutex;
std::mutex g_region_mutex;

import AIServerBinder;

using namespace db;

AiServerInstance* AiServerInstance::s_instance = nullptr;

AiServerInstance::AiServerInstance(AIServerLogger& logger)
	: _logger(logger)
{
	assert(s_instance == nullptr);
	s_instance = this;

	m_iYear = 0;
	m_iMonth = 0;
	m_iDate = 0;
	m_iHour = 0;
	m_iMin = 0;
	m_iWeather = 0;
	m_iAmount = 0;
	m_byNight = 1;
	m_byZone = KARUS_ZONE;
	m_byBattleEvent = BATTLEZONE_CLOSE;
	m_sKillKarusNpc = 0;
	m_sKillElmoNpc = 0;
	m_pZoneEventThread = nullptr;
	m_byTestMode = 0;
	//m_ppUserActive = nullptr;
	//m_ppUserInActive = nullptr;

	ConnectionManager::Create();

	_checkAliveThread = std::make_unique<TimerThread>(
		10s,
		std::bind(&AiServerInstance::CheckAliveTest, this));
}

AiServerInstance::~AiServerInstance()
{
	spdlog::info("AiServerInstance::~AiServerInstance: Graceful shutdown triggered, releasing resources.");
	_socketManager.Shutdown();
	spdlog::info("AiServerInstance::~AiServerInstance: SocketManager stopped.");

	// wait for all of these threads to be fully shut down.
	spdlog::info("AiServerInstance::~AiServerInstance: Waiting for worker threads to fully shut down.");

	if (_checkAliveThread != nullptr)
	{
		spdlog::info("AiServerInstance::~AiServerInstance: Shutting down CheckAliveThread...");

		_checkAliveThread->shutdown();

		spdlog::info("AiServerInstance::~AiServerInstance: CheckAliveThread stopped.");
	}

	if (!m_NpcThreadArray.empty())
	{
		spdlog::info("AiServerInstance::~AiServerInstance: Shutting down {} NPC threads...", m_NpcThreadArray.size());

		for (CNpcThread* npcThread : m_NpcThreadArray)
			npcThread->shutdown(false);

		for (CNpcThread* npcThread : m_NpcThreadArray)
			npcThread->join();

		spdlog::info("AiServerInstance::~AiServerInstance: NPC threads stopped.");
	}

	if (m_pZoneEventThread != nullptr)
	{
		spdlog::info("AiServerInstance::~AiServerInstance: Shutting down ZoneEventThread...");

		m_pZoneEventThread->shutdown();

		spdlog::info("AiServerInstance::~AiServerInstance: ZoneEventThread stopped.");
	}

	spdlog::info("AiServerInstance::~AiServerInstance: All worker threads stopped, freeing caches.");

	for (MAP* map : m_ZoneArray)
		delete map;
	m_ZoneArray.clear();

	delete m_pZoneEventThread;
	m_pZoneEventThread = nullptr;

	for (CNpcThread* npcThread : m_NpcThreadArray)
		delete npcThread;
	m_NpcThreadArray.clear();

	if (m_NpcItem.m_ppItem != nullptr)
	{
		for (int i = 0; i < m_NpcItem.m_nRow; i++)
		{
			delete[] m_NpcItem.m_ppItem[i];
			m_NpcItem.m_ppItem[i] = nullptr;
		}

		delete[] m_NpcItem.m_ppItem;
		m_NpcItem.m_ppItem = nullptr;
	}

	for (int i = 0; i < MAX_USER; i++)
	{
		delete m_pUser[i];
		m_pUser[i] = nullptr;
	}

	while (!m_ZoneNpcList.empty())
		m_ZoneNpcList.pop_front();

	spdlog::info("AiServerInstance::~AiServerInstance: All resources safely released.");

	ConnectionManager::Destroy();

	assert(s_instance != nullptr);
	s_instance = nullptr;
}

bool AiServerInstance::OnStart()
{
	// load config
	GetServerInfoIni();

	// TestCode
	TestCode();

	//----------------------------------------------------------------------
	//	Sets a random number starting point.
	//----------------------------------------------------------------------
	srand((unsigned int) time(nullptr));
	for (int i = 0; i < 10; i++)
		myrand(1, 10000);	// don't delete

	// Compress Init
	memset(m_CompBuf, 0, sizeof(m_CompBuf));	// 압축할 데이터를 모으는 버퍼
	m_iCompIndex = 0;							// 압축할 데이터의 길이
	m_CompCount = 0;							// 압축할 데이터의 개수

	m_sSocketCount = 0;
	m_sErrorSocketCount = 0;
	m_sMapEventNpc = 0;
	m_sReSocketCount = 0;
	m_fReConnectStart = 0.0;
	m_bFirstServerFlag = false;
	m_byTestMode = NOW_TEST_MODE;

	// User Point Init
	for (int i = 0; i < MAX_USER; i++)
		m_pUser[i] = nullptr;

	// Server Start message
	spdlog::info("AiServerInstance::OnStart: starting...");

	//----------------------------------------------------------------------
	//	DB part initialize
	//----------------------------------------------------------------------
	if (m_byZone == UNIFY_ZONE)
		spdlog::info("AiServerInstance::OnStart: Server Zone: UNIFY");
	else if (m_byZone == KARUS_ZONE)
		spdlog::info("AiServerInstance::OnStart: Server Zone: KARUS");
	else if (m_byZone == ELMORAD_ZONE)
		spdlog::info("AiServerInstance::OnStart: Server Zone: ELMORAD");
	else if (m_byZone == BATTLE_ZONE)
		spdlog::info("AiServerInstance::OnStart: Server Zone: BATTLE");
	
	//----------------------------------------------------------------------
	//	Communication Part Init ...
	//----------------------------------------------------------------------
	spdlog::info("AiServerInstance::OnStart: initializing sockets");
	_socketManager.Init(MAX_SOCKET, 0, 1);
	_socketManager.AllocateServerSockets<CGameSocket>();

	//----------------------------------------------------------------------
	//	Load Magic Table
	//----------------------------------------------------------------------
	if (!GetMagicTableData())
	{
		spdlog::error("AiServerInstance::OnStart: failed to load MAGIC, closing server");
		return false;
	}

	if (!GetMagicType1Data())
	{
		spdlog::error("AiServerInstance::OnStart: failed to load MAGIC_TYPE1, closing server");
		return false;
	}

	if (!GetMagicType2Data())
	{
		spdlog::error("AiServerInstance::OnStart: failed to load MAGIC_TYPE2, closing server");
		return false;
	}

	if (!GetMagicType3Data())
	{
		spdlog::error("AiServerInstance::OnStart: failed to load MAGIC_TYPE3, closing server");
		return false;
	}

	if (!GetMagicType4Data())
	{
		spdlog::error("AiServerInstance::OnStart: failed to load MAGIC_TYPE4, closing server");
		return false;
	}

	if (!GetMagicType7Data())
	{
		spdlog::error("AiServerInstance::OnStart: failed to load MAGIC_TYPE7, closing server");
		return false;
	}

	//----------------------------------------------------------------------
	//	Load NPC Item Table
	//----------------------------------------------------------------------
	if (!GetNpcItemTable())
	{
		spdlog::error("AiServerInstance::OnStart: failed to load K_MONSTER_ITEM, closing server");
		return false;
	}

	if (!GetMakeWeaponItemTableData())
	{
		spdlog::error("AiServerInstance::OnStart: failed to load MAKE_WEAPON, closing server");
		return false;
	}

	if (!GetMakeDefensiveItemTableData())
	{
		spdlog::error("AiServerInstance::OnStart: failed to load MAKE_DEFENSIVE, closing server");
		return false;
	}

	if (!GetMakeGradeItemTableData())
	{
		spdlog::error("AiServerInstance::OnStart: failed to load MAKE_ITEM_GRADECODE, closing server");
		return false;
	}

	if (!GetMakeRareItemTableData())
	{
		spdlog::error("AiServerInstance::OnStart: failed to load MAKE_ITEM_LARECODE, closing server");
		return false;
	}

	if (!GetMakeItemGroupTableData())
	{
		spdlog::error("AiServerInstance::OnStart: failed to load MAKE_ITEM_GROUP, closing server");
		return false;
	}

	//----------------------------------------------------------------------
	//	Load NPC Chat Table
	//----------------------------------------------------------------------

	//----------------------------------------------------------------------
	//	Load NPC Data & Activate NPC
	//----------------------------------------------------------------------

	// Monster 특성치 테이블 Load
	if (!GetMonsterTableData())
	{
		spdlog::error("AiServerInstance::OnStart: failed to load K_MONSTER, closing server");
		return false;
	}

	// NPC 특성치 테이블 Load
	if (!GetNpcTableData())
	{
		spdlog::error("AiServerInstance::OnStart: failed to load K_NPC, closing server");
		return false;
	}

	//----------------------------------------------------------------------
	//	Load Zone & Event...
	//----------------------------------------------------------------------
	if (!MapFileLoad())
	{
		spdlog::error("AiServerInstance::OnStart: failed to load maps, closing server");
		return false;
	}

	if (!CreateNpcThread())
	{
		spdlog::error("AiServerInstance::OnStart: CreateNpcThread failed, closing server");
		return false;
	}

	//----------------------------------------------------------------------
	//	Load NPC DN Table
	//----------------------------------------------------------------------

	//----------------------------------------------------------------------
	//	Start NPC THREAD
	//----------------------------------------------------------------------
	StartNpcThreads();

	//----------------------------------------------------------------------
	//	Start Accepting...
	//----------------------------------------------------------------------
	if (!ListenByZone())
	{
		return false;
	}

	_checkAliveThread->start();

	//::ResumeThread( _socketManager.m_hAcceptThread );

	spdlog::info("AiServerInstance::OnStart: AIServer successfully initialized");

	return true;
}

void AiServerInstance::thread_loop()
{
	if (!OnStart())
		return;
		
	while (_canTick)
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_cv.wait(lock);
	}
}

/// \brief attempts to listen on the port associated with m_byZone
/// \see m_byZone
/// \returns true when successful, otherwise false
bool AiServerInstance::ListenByZone()
{
	int port = GetListenPortByZone();
	if (port < 0)
	{
		spdlog::error("AiServerInstance::ListenByZone: failed to associate listen port for zone {}", m_byZone);
		return false;
	}

	if (!_socketManager.Listen(port))
	{
		spdlog::error("AiServerInstance::ListenByZone: failed to listen on port {}", port);
		return false;
	}

	spdlog::info("AiServerInstance::ListenByZone: Listening on 0.0.0.0:{}", port);
	return true;
}

/// \brief fetches the listen port associated with m_byZone
/// \see m_byZone
/// \returns the associated listen port or -1 if invalid
int AiServerInstance::GetListenPortByZone() const
{
	switch (m_byZone)
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
bool AiServerInstance::GetMagicTableData()
{
	recordset_loader::STLMap loader(m_MagicTableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("AiServerInstance::GetMagicTableData: load failed - {}",
			loader.GetError().Message);
		return false;
	}
	
	spdlog::info("AiServerInstance::GetMagicTableData: MAGIC loaded");
	return true;
}

bool AiServerInstance::GetMakeWeaponItemTableData()
{
	recordset_loader::STLMap loader(m_MakeWeaponTableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("AiServerInstance::GetMakeWeaponItemTableData: load failed - {}",
			loader.GetError().Message);
		return false;
	}

	spdlog::info("AiServerInstance::GetMakeWeaponItemTableData: MAKE_WEAPON loaded");
	return true;
}

bool AiServerInstance::GetMakeDefensiveItemTableData()
{
	recordset_loader::STLMap<MakeWeaponTableMap, model::MakeDefensive> loader(
		m_MakeDefensiveTableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("AiServerInstance::GetMakeDefensiveItemTableData: load failed - {}",
			loader.GetError().Message);
		return false;
	}

	spdlog::info("AiServerInstance::GetMakeDefensiveItemTableData: MAKE_DEFENSIVE loaded");
	return true;
}

bool AiServerInstance::GetMakeGradeItemTableData()
{
	recordset_loader::STLMap loader(m_MakeGradeItemArray);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("AiServerInstance::GetMakeGradeItemTableData: load failed - {}",
			loader.GetError().Message);
		return false;
	}

	spdlog::info("AiServerInstance::GetMakeGradeItemTableData: MAKE_ITEM_GRADECODE loaded");
	return true;
}

bool AiServerInstance::GetMakeRareItemTableData()
{
	recordset_loader::STLMap loader(m_MakeItemRareCodeTableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("AiServerInstance::GetMakeRareItemTableData: load failed - {}",
			loader.GetError().Message);
		return false;
	}

	spdlog::info("AiServerInstance::GetMakeRareItemTableData: MAKE_ITEM_LARECODE loaded");
	return true;
}

bool AiServerInstance::GetMakeItemGroupTableData()
{
	recordset_loader::STLMap loader(m_MakeItemGroupTableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("AiServerInstance::GetMakeItemGroupTableData: load failed - {}",
			loader.GetError().Message);
		return false;
	}

	spdlog::info("AiServerInstance::GetMakeItemGroupTableData: MAKE_ITEM_GROUP loaded");
	return true;
}

/////////////////////////////////////////////////////////////////////////
//	NPC Item Table 을 읽는다.
//
bool AiServerInstance::GetNpcItemTable()
{
	using ModelType = model::MonsterItem;

	std::vector<ModelType*> rows;

	recordset_loader::Vector<ModelType> loader(rows);
	if (!loader.Load_ForbidEmpty(true))
	{
		spdlog::error("AiServerInstance::GetNpcItemTable: load failed - {}",
			loader.GetError().Message);
		return false;
	}

	m_NpcItem.m_nField = loader.GetColumnCount();
	m_NpcItem.m_nRow = static_cast<int>(rows.size());

	if (rows.empty())
		return false;

	m_NpcItem.m_ppItem = new int* [m_NpcItem.m_nRow];
	for (int i = 0; i < m_NpcItem.m_nRow; i++)
		m_NpcItem.m_ppItem[i] = new int[m_NpcItem.m_nField];

	for (size_t i = 0; i < rows.size(); i++)
	{
		ModelType* row = rows[i];

		m_NpcItem.m_ppItem[i][0] = row->MonsterId;
		m_NpcItem.m_ppItem[i][1] = row->ItemId1;
		m_NpcItem.m_ppItem[i][2] = row->DropChance1;
		m_NpcItem.m_ppItem[i][3] = row->ItemId2;
		m_NpcItem.m_ppItem[i][4] = row->DropChance2;
		m_NpcItem.m_ppItem[i][5] = row->ItemId3;
		m_NpcItem.m_ppItem[i][6] = row->DropChance3;
		m_NpcItem.m_ppItem[i][7] = row->ItemId4;
		m_NpcItem.m_ppItem[i][8] = row->DropChance4;
		m_NpcItem.m_ppItem[i][9] = row->ItemId5;
		m_NpcItem.m_ppItem[i][10] = row->DropChance5;

		delete row;
	}

	rows.clear();

	spdlog::info("AiServerInstance::GetNpcItemTable: K_MONSTER_ITEM loaded");
	return true;
}

//	Monster Table Data 를 읽는다.
bool AiServerInstance::GetMonsterTableData()
{
	NpcTableMap tableMap;
	recordset_loader::STLMap<
		NpcTableMap,
		model::Monster> loader(tableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("AiServerInstance::GetMonsterTableData: load failed - {}",
			loader.GetError().Message);
		return false;
	}

#if defined(DB_COMPAT_PADDED_NAMES)
	for (const auto& [_, row] : tableMap)
	{
		if (row->Name.has_value())
			rtrim(*row->Name);
	}
#endif

	m_MonTableMap.Swap(tableMap);

	spdlog::info("AiServerInstance::GetMonsterTableData: K_MONSTER loaded");
	return true;
}

//	NPC Table Data 를 읽는다. (경비병 & NPC)
bool AiServerInstance::GetNpcTableData()
{
	NpcTableMap tableMap;
	recordset_loader::STLMap loader(tableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("AiServerInstance::GetNpcTableData: load failed - {}",
			loader.GetError().Message);
		return false;
	}

#if defined(DB_COMPAT_PADDED_NAMES)
	for (const auto& [_, row] : tableMap)
	{
		if (row->Name.has_value())
			rtrim(*row->Name);
	}
#endif

	m_NpcTableMap.Swap(tableMap);

	spdlog::info("AiServerInstance::GetNpcTableData: K_NPC loaded");
	return true;
}

//	Npc Thread 를 만든다.
bool AiServerInstance::CreateNpcThread()
{
	m_TotalNPC = 0;			// DB에 있는 수
	m_CurrentNPC = 0;
	m_CurrentNPCError = 0;

	std::vector<model::NpcPos*> rows;
	if (!LoadNpcPosTable(rows))
	{
		spdlog::error("AiServerInstance::CreateNpcThread: K_NPCPOS load failed");
		return false;
	}

	for (model::NpcPos* row : rows)
		delete row;
	rows.clear();

	int step = 0;
	int nThreadNumber = 0;
	CNpcThread* pNpcThread = nullptr;

	for (auto& [_, pNpc] : m_NpcMap)
	{
		if (step == 0)
			pNpcThread = new CNpcThread();

		pNpcThread->m_pNpc[step] = pNpc;
		pNpcThread->m_pNpc[step]->m_sThreadNumber = nThreadNumber;
		pNpcThread->m_pNpc[step]->Init();

		++step;

		if (step == NPC_NUM)
		{
			pNpcThread->m_sThreadNumber = nThreadNumber++;
			m_NpcThreadArray.push_back(pNpcThread);
			step = 0;
		}
	}

	if (step != 0)
	{
		pNpcThread->m_sThreadNumber = nThreadNumber++;
		m_NpcThreadArray.push_back(pNpcThread);
	}

	if (m_pZoneEventThread == nullptr)
		m_pZoneEventThread = new ZoneEventThread();
	
	spdlog::info("AiServerInstance::CreateNpcThread: Monsters/NPCs loaded: {}", m_TotalNPC);
	return true;
}

bool AiServerInstance::LoadNpcPosTable(std::vector<model::NpcPos*>& rows)
{
	CRoomEvent* pRoom = nullptr;

	recordset_loader::Vector<model::NpcPos> loader(rows);
	if (!loader.Load_ForbidEmpty(true))
	{
		spdlog::error("AiServerInstance::LoadNpcPosTable: failed - {}",
			loader.GetError().Message);
		return false;
	}

	int nSerial = m_sMapEventNpc;

	for (model::NpcPos* row : rows)
	{
		bool bMoveNext = true;
		int nPathSerial = 1;
		int nNpcCount = 0;

		do
		{
			int nMonsterNumber = row->NumNpc;
			int nServerNum = GetServerNumber(row->ZoneId);

			if (m_byZone == nServerNum
				|| m_byZone == UNIFY_ZONE)
			{
				for (int j = 0; j < nMonsterNumber; j++)
				{
					CNpc* pNpc = new CNpc();
					pNpc->m_sNid = nSerial++;						// 서버 내에서의 고유 번호
					pNpc->m_sSid = (int16_t) row->NpcId;				// MONSTER(NPC) Serial ID

					pNpc->m_byMoveType = row->ActType;
					pNpc->m_byInitMoveType = row->ActType;
					pNpc->m_byDirection = row->Direction;

					model::Npc* pNpcTable = nullptr;

					if (row->ActType >= 0
						&& row->ActType < 100)
					{
						pNpcTable = m_MonTableMap.GetData(pNpc->m_sSid);
					}
					else if (row->ActType >= 100)
					{
						pNpc->m_byMoveType = row->ActType - 100;
						//pNpc->m_byInitMoveType = row->ActType - 100;

						pNpcTable = m_NpcTableMap.GetData(pNpc->m_sSid);
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
						spdlog::error("AiServerInstance::LoadNpcPosTable: npc not found [serial={}, npcId={}]",
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
						spdlog::warn("AiServerInstance::LoadNpcPosTable: RegTime below minimum value of 15s [npcId={}, serial={}, npcName={}, RegTime={}]",
							pNpc->m_sSid, pNpc->m_sNid + NPC_BAND, pNpc->m_strName, row->RespawnTime);
						// TODO: Set this to 15 in separate ticket and comment on it deviating from official behavior
						row->RespawnTime = 30;
					}

					pNpc->m_sRegenTime = row->RespawnTime * 1000;	// 초(DB)단위-> 밀리세컨드로

					pNpc->m_sMaxPathCount = row->PathPointCount;

					if (pNpc->m_byMoveType == 2
						|| pNpc->m_byMoveType == 3)
					{
						if (row->PathPointCount == 0
							|| !row->Path.has_value())
						{
							spdlog::error(
								"AiServerInstance::LoadNpcPosTable: NPC expects path to be set [zoneId={} serial={}, npcId={}, npcName={}, moveType={}, pathCount={}]",
								row->ZoneId, pNpc->m_sNid + NPC_BAND, pNpc->m_sSid, pNpc->m_strName, pNpc->m_byMoveType, pNpc->m_sMaxPathCount);
							return false;
						}
					}

					int index = 0;

					if (row->PathPointCount != 0
						&& row->Path.has_value())
					{
						// The path is a series of points (x,z), each in the form ("%04d%04d", x, z)
						// As such, we expect there to be at least 8 characters per point.
						constexpr size_t CharactersPerPoint = 8;

						const std::string& path = *row->Path;
						if ((row->PathPointCount * CharactersPerPoint) > path.size())
						{
							spdlog::error(
								"LoadNpcPosTable: NPC expects a larger path for this PathPointCount [zoneId={} serial={} npcId={} npcName={} moveType={}, pathCount={}]",
								row->ZoneId, row->PathPointCount, pNpc->m_sNid + NPC_BAND, pNpc->m_sSid, pNpc->m_strName, pNpc->m_byMoveType, pNpc->m_sMaxPathCount);
							return false;
						}

						for (int l = 0; l < row->PathPointCount; l++)
						{
							char szX[5] = {}, szZ[5] = {};
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
					pNpc->m_byDungeonFamily = row->DungeonFamily;
					pNpc->m_bySpecialType = row->SpecialType;
					pNpc->m_byRegenType = row->RegenType;
					pNpc->m_byTrapNumber = row->TrapNumber;

					if (pNpc->m_byDungeonFamily > 0)
					{
						pNpc->m_nLimitMinX = row->LimitMinX;
						pNpc->m_nLimitMinZ = row->LimitMinZ;
						pNpc->m_nLimitMaxX = row->LimitMaxX;
						pNpc->m_nLimitMaxZ = row->LimitMaxZ;
					}

					pNpc->m_ZoneIndex = -1;

					MAP* pMap = nullptr;
					for (size_t i = 0; i < m_ZoneArray.size(); i++)
					{
						if (m_ZoneArray[i]->m_nZoneNumber == pNpc->m_sCurZone)
						{
							pNpc->m_ZoneIndex = static_cast<int16_t>(i);
							pMap = m_ZoneArray[i];
							break;
						}
					}

					if (pMap == nullptr)
					{
						spdlog::error("AiServerInstance::LoadNpcPosTable: NPC invalid zone [npcId:{}, npcZoneId:{}]",
							pNpc->m_sSid, pNpc->m_sCurZone);
						return false;
					}

					//pNpc->Init();
					//m_NpcMap.Add(pNpc);
					if (!m_NpcMap.PutData(pNpc->m_sNid, pNpc))
					{
						spdlog::error("AiServerInstance::LoadNpcPosTable: Npc PutData Fail [serial={}]",
							pNpc->m_sNid);
						delete pNpc;
						pNpc = nullptr;
					}

					if (pNpc != nullptr
						&& pMap->m_byRoomEvent > 0
						&& pNpc->m_byDungeonFamily > 0)
					{
						pRoom = pMap->m_arRoomEventArray.GetData(pNpc->m_byDungeonFamily);
						if (pRoom == nullptr)
						{
							spdlog::error("AiServerInstance::LoadNpcPosTable: No RoomEvent for NPC dungeonFamily: serial={}, npcId={}, npcName={}, dungeonFamily={}, zoneId={}",
								pNpc->m_sNid + NPC_BAND, pNpc->m_sSid, pNpc->m_strName, pNpc->m_byDungeonFamily, pNpc->m_ZoneIndex);
							return false;
						}

						int* pInt = new int;
						*pInt = pNpc->m_sNid;
						if (!pRoom->m_mapRoomNpcArray.PutData(pNpc->m_sNid, pInt))
						{
							delete pInt;
							spdlog::error("AiServerInstance::LoadNpcPosTable: MapRoomNpcArray.PutData failed for NPC: [serial={}, npcId={}]", pNpc->m_sNid, pNpc->m_sSid);
						}
					}

					m_TotalNPC = nSerial;

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
void AiServerInstance::StartNpcThreads()
{
	for (CNpcThread* npcThread : m_NpcThreadArray)
		npcThread->start();

	m_pZoneEventThread->start();
}

void AiServerInstance::DeleteUserList(int uid)
{
	if (uid < 0
		|| uid >= MAX_USER)
	{
		spdlog::error("AiServerInstance::DeleteUserList: userId invalid: {}", uid);
		return;
	}

	std::string characterName;

	std::unique_lock<std::mutex> lock(g_user_mutex);

	CUser* pUser = m_pUser[uid];
	if (pUser == nullptr)
	{
		lock.unlock();
		spdlog::error("AiServerInstance::DeleteUserList: userId not found: {}", uid);
		return;
	}

	if (pUser->m_iUserId != uid)
	{
		lock.unlock();
		spdlog::warn("AiServerInstance::DeleteUserList: userId mismatch : userId={} pUserId={}", uid, pUser->m_iUserId);
		return;
	}

	characterName = pUser->m_strUserID;

	pUser->m_lUsed = 1;
	delete m_pUser[uid];
	m_pUser[uid] = nullptr;

	lock.unlock();
	spdlog::debug("AiServerInstance::DeleteUserList: User Logout: userId={}, charId={}", uid, characterName);
}

bool AiServerInstance::MapFileLoad()
{
	using ModelType = model::ZoneInfo;

	bool loaded = false;

	m_sTotalMap = 0;

	recordset_loader::Base<ModelType> loader;
	loader.SetProcessFetchCallback([&](db::ModelRecordSet<ModelType>& recordset)
	{
		// Build the base MAP directory
		std::filesystem::path mapDir(GetProgPath());
		mapDir /= MAP_DIR;

		// Resolve it to strip the relative references to be nice.
		if (std::filesystem::exists(mapDir))
			mapDir = std::filesystem::canonical(mapDir);

		do
		{
			ModelType row = {};
			recordset.get_ref(row);

			std::filesystem::path mapPath
				= mapDir / row.Name;

			std::ifstream file(mapPath, std::ios::in | std::ios::binary);
			if (!file)
			{
				spdlog::error("AiServerInstance::MapFileLoad: Failed to open file: {}",
					mapPath.string());
				return;
			}

			MAP* pMap = new MAP();
			pMap->m_nServerNo = row.ServerId;
			pMap->m_nZoneNumber = row.ZoneId;

			if (!pMap->LoadMap(file))
			{
				spdlog::error("AiServerInstance::MapFileLoad: Failed to load map file: {}",
					mapPath.string());
				delete pMap;
				return;
			}

			file.close();

			// dungeon work
			if (row.RoomEvent > 0)
			{
				if (!pMap->LoadRoomEvent(row.RoomEvent))
				{
					spdlog::error("AiServerInstance::MapFileLoad: LoadRoomEvent failed: {}",
						mapPath.string());
					delete pMap;
					return;
				}

				pMap->m_byRoomEvent = 1;
			}

			m_ZoneArray.push_back(pMap);
			++m_sTotalMap;
		}
		while (recordset.next());

		loaded = true;
	});

	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("AiServerInstance::MapFileLoad: load failed (ZONE_INFO) - {}",
			loader.GetError().Message);
		return false;
	}

	return loaded;
}

// sungyong 2002.05.23
// game server에 모든 npc정보를 전송..
void AiServerInstance::AllNpcInfo()
{
	// server alive check
	CNpc* pNpc = nullptr;
	int nZone = 0;
	int size = m_NpcMap.GetSize();

	int send_index = 0, zone_index = 0, packet_size = 0;
	int count = 0, send_count = 0, send_tot = 0;
	char send_buff[2048] = {};

	for (MAP* pMap : m_ZoneArray)
	{
		if (pMap == nullptr)
			continue;

		nZone = pMap->m_nZoneNumber;

		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;
		SetByte(send_buff, AG_SERVER_INFO, send_index);
		SetByte(send_buff, SERVER_INFO_START, send_index);
		SetByte(send_buff, nZone, send_index);
		packet_size = Send(send_buff, send_index, nZone);

		zone_index = GetZoneIndex(nZone);
		send_index = 2;
		count = 0;
		send_count = 0;
		m_CompCount = 0;
		m_iCompIndex = 0;
		memset(send_buff, 0, sizeof(send_buff));

		spdlog::debug("AiServerInstance::AllNpcInfo: start for zoneIndex={}", nZone);

		for (int i = 0; i < size; i++)
		{
			pNpc = m_NpcMap.GetData(i);
			if (pNpc == nullptr)
			{
				spdlog::warn("AiServerInstance::AllNpcInfo: NpcMap[{}] is null", i);
				continue;
			}

			if (pNpc->m_sCurZone != nZone)
				continue;

			pNpc->SendNpcInfoAll(send_buff, send_index, count);
			count++;

			if (count == NPC_NUM)
			{
				SetByte(send_buff, NPC_INFO_ALL, send_count);
				SetByte(send_buff, (uint8_t) count, send_count);
				m_CompCount++;
				//memcpy(m_CompBuf+m_iCompIndex, send_buff, send_index);
				memset(m_CompBuf, 0, sizeof(m_CompBuf));
				memcpy(m_CompBuf, send_buff, send_index);
				m_iCompIndex = send_index;
				SendCompressedData(nZone);
				send_index = 2;
				send_count = 0;
				count = 0;
				send_tot++;
				//TRACE(_T("AllNpcInfo - send_count=%d, count=%d, zone=%d\n"), send_tot, count, nZone);
				memset(send_buff, 0, sizeof(send_buff));
				Sleep(50);
			}
		}

		//if( count != 0 )	TRACE(_T("--> AllNpcInfo - send_count=%d, count=%d, zone=%d\n"), send_tot, count, nZone);
		if (count != 0
			&& count < NPC_NUM)
		{
			send_count = 0;
			SetByte(send_buff, NPC_INFO_ALL, send_count);
			SetByte(send_buff, (uint8_t) count, send_count);
			Send(send_buff, send_index, nZone);
			send_tot++;
			//TRACE(_T("AllNpcInfo - send_count=%d, count=%d, zone=%d\n"), send_tot, count, nZone);
			Sleep(50);
		}

		send_index = 0;
		memset(send_buff, 0, sizeof(send_buff));
		SetByte(send_buff, AG_SERVER_INFO, send_index);
		SetByte(send_buff, SERVER_INFO_END, send_index);
		SetByte(send_buff, nZone, send_index);
		SetShort(send_buff, (int16_t) m_TotalNPC, send_index);
		packet_size = Send(send_buff, send_index, nZone);

		spdlog::debug("AiServerInstance::AllNpcInfo: end for zoneId={}", nZone);
	}

	Sleep(1000);
}
// ~sungyong 2002.05.23

CUser* AiServerInstance::GetUserPtr(int nid)
{
	CUser* pUser = nullptr;

	if (nid < 0
		|| nid >= MAX_USER)
	{
		if (nid != -1)
			spdlog::error("AiServerInstance::GetUserPtr: User Array Overflow [{}]", nid);

		return nullptr;
	}

/*	if( !m_ppUserActive[nid] )
		return nullptr;

	if( m_ppUserActive[nid]->m_lUsed == 1 ) return nullptr;	// 포인터 사용을 허락치 않음.. (logout중)

	pUser = (CUser*)m_ppUserActive[nid];
*/
	pUser = m_pUser[nid];
	if (pUser == nullptr)
		return nullptr;

	// 포인터 사용을 허락치 않음.. (logout중)
	if (pUser->m_lUsed == 1)
		return nullptr;

	if (pUser->m_iUserId < 0
		|| pUser->m_iUserId >= MAX_USER)
		return nullptr;

	if (pUser->m_iUserId == nid)
		return pUser;

	return nullptr;
}

// sungyong 2002.05.23
void AiServerInstance::CheckAliveTest()
{
	int send_index = 0;
	char send_buff[256] = {};
	int iErrorCode = 0;

	SetByte(send_buff, AG_CHECK_ALIVE_REQ, send_index);

	CGameSocket* pSocket = nullptr;
	int size = 0, count = 0;
	int socketCount = _socketManager.GetServerSocketCount();
	for (int i = 0; i < socketCount; i++)
	{
		pSocket = _socketManager.GetServerSocketUnchecked(i);
		if (pSocket == nullptr)
			continue;

		size = pSocket->Send(send_buff, send_index);
		if (size > 0)
		{
			if (++m_sErrorSocketCount == 10)
				spdlog::debug("AiServerInstance::CheckAliveTest: all ebenezer sockets are connected");

			count++;
		}
		//TRACE(_T("size = %d, socket_num = %d, i=%d \n"), size, pSocket->m_sSocketID, i);
	}

	if (count <= 0)
		DeleteAllUserList(9999);

	RegionCheck();
}

void AiServerInstance::DeleteAllUserList(int zone)
{
	if (zone < 0)
		return;

	// 모든 소켓이 끊어진 상태...
	if (zone == 9999
		&& m_bFirstServerFlag)
	{
		spdlog::debug("AiServerInstance::DeleteAllUserList: start");

		{
			std::lock_guard<std::mutex> lock(g_region_mutex);

			for (MAP* pMap : m_ZoneArray)
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
				delete m_pUser[i];
				m_pUser[i] = nullptr;
			}
		}

		// Party Array Delete 
		{
			std::lock_guard<std::mutex> lock(g_region_mutex);
			m_PartyMap.DeleteAllData();
		}

		m_bFirstServerFlag = false;
		spdlog::debug("AiServerInstance::DeleteAllUserList: end");
	}
	else if (zone != 9999)
	{
		spdlog::info("AiServerInstance::DeleteAllUserList: ebenezer zone {} disconnected", zone);
	}
}
// ~sungyong 2002.05.23

void AiServerInstance::SendCompressedData(int nZone)
{
	if (m_CompCount <= 0
		|| m_iCompIndex <= 0)
	{
		m_CompCount = 0;
		m_iCompIndex = 0;
		spdlog::error("AiServerInstance::SendCompressData: count={}, index={}",
			m_CompCount, m_iCompIndex);
		return;
	}

	int send_index = 0;
	char send_buff[32000] = {};
	uint8_t comp_buff[32000] = {};
	unsigned int comp_data_len = 0;
	uint32_t crc_value = 0;

	comp_data_len = lzf_compress(m_CompBuf, m_iCompIndex, comp_buff, sizeof(comp_buff));

	assert(comp_data_len != 0 && comp_data_len <= sizeof(comp_buff));

	if (comp_data_len == 0
		|| comp_data_len > sizeof(comp_buff))
	{
		spdlog::error("AiServerInstance::SendCompressedData: Failed to compress packet");
		return;
	}

	crc_value = crc32(reinterpret_cast<uint8_t*>(m_CompBuf), m_iCompIndex);

	SetByte(send_buff, AG_COMPRESSED_DATA, send_index);
	SetShort(send_buff, (int16_t) comp_data_len, send_index);
	SetShort(send_buff, (int16_t) m_iCompIndex, send_index);
	SetDWORD(send_buff, crc_value, send_index);
	SetShort(send_buff, (int16_t) m_CompCount, send_index);
	SetString(send_buff, reinterpret_cast<const char*>(comp_buff), comp_data_len, send_index);

	Send(send_buff, send_index, nZone);

	m_CompCount = 0;
	m_iCompIndex = 0;
}

// sungyong 2002.05.23
int AiServerInstance::Send(const char* pData, int length, int nZone)
{
	// Not connected to any servers.
	// No point queueing updates, the server will be fully synced upon connection.
	if (!m_bFirstServerFlag)
		return 0;

	if (length <= 0
		|| length > sizeof(_SEND_DATA::pBuf))
		return 0;

	_SEND_DATA* pNewData = new _SEND_DATA;
	if (pNewData == nullptr)
		return 0;

	pNewData->sCurZone = nZone;
	pNewData->sLength = length;
	memcpy(pNewData->pBuf, pData, length);

	_socketManager.QueueSendData(pNewData);

	return 0;
}
// ~sungyong 2002.05.23

void AiServerInstance::GameServerAcceptThread()
{
	_socketManager.StartAccept();
}

void AiServerInstance::SyncTest()
{
	spdlog::info("AiServerInstance::SyncTest: begin");

	int send_index = 0;
	char send_buff[256] = {};
	int iErrorCode = 0;

	SetByte(send_buff, AG_CHECK_ALIVE_REQ, send_index);

	CGameSocket* pSocket = nullptr;
	int size = 0, socketCount = _socketManager.GetServerSocketCount();

	for (int i = 0; i < socketCount; i++)
	{
		pSocket = _socketManager.GetServerSocketUnchecked(i);
		if (pSocket == nullptr)
			continue;

		size = pSocket->Send(send_buff, send_index);

		spdlog::info("AiServerInstance::SyncTest: size={}, zoneNo={}", size, pSocket->_zoneNo);
	}
}

void AiServerInstance::TestCode()
{
	//InitTrigonometricFunction();

	int random = 0, count_1 = 0, count_2 = 0, count_3 = 0;

	// TestCoding
	for (int i = 0; i < 100; i++)
	{
		random = myrand(1, 3);
		if (random == 1)
			count_1++;
		else if (random == 2)
			count_2++;
		else if (random == 3)
			count_3++;
	}

	//TRACE(_T("$$$ random test == 1=%d, 2=%d, 3=%d,, %d,%hs $$$\n"), count_1, count_2, count_3, __FILE__, __LINE__);
}

bool AiServerInstance::GetMagicType1Data()
{
	recordset_loader::STLMap loader(m_MagicType1TableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("AiServerInstance::GetMagicType1Data: load failed - {}",
			loader.GetError().Message);
		return false;
	}

	spdlog::info("AiServerInstance::GetMagicType1Data: MAGIC_TYPE1 loaded");
	return true;
}

bool AiServerInstance::GetMagicType2Data()
{
	recordset_loader::STLMap loader(m_MagicType2TableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("AiServerInstance::GetMagicType2Data: load failed - {}",
			loader.GetError().Message);
		return false;
	}

	spdlog::info("AiServerInstance::GetMagicType2Data: MAGIC_TYPE2 loaded");
	return true;
}

bool AiServerInstance::GetMagicType3Data()
{
	recordset_loader::STLMap loader(m_MagicType3TableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("AiServerInstance::GetMagicType3Data: load failed - {}",
			loader.GetError().Message);
		return false;
	}

	spdlog::info("AiServerInstance::GetMagicType3Data: MAGIC_TYPE3 loaded");
	return true;
}

bool AiServerInstance::GetMagicType4Data()
{
	recordset_loader::STLMap loader(m_MagicType4TableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("AiServerInstance::GetMagicType4Data: load failed - {}",
			loader.GetError().Message);
		return false;
	}

	spdlog::info("AiServerInstance::GetMagicType4Data: MAGIC_TYPE4 loaded");
	return true;
}

bool AiServerInstance::GetMagicType7Data()
{
	recordset_loader::STLMap loader(m_MagicType7TableMap);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("AiServerInstance::GetMagicType7Data: load failed - {}",
			loader.GetError().Message);
		return false;
	}

	spdlog::info("AiServerInstance::GetMagicType7Data: MAGIC_TYPE7 loaded");
	return true;
}

void AiServerInstance::RegionCheck()
{
	for (MAP* pMap : m_ZoneArray)
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

bool AiServerInstance::AddObjectEventNpc(_OBJECT_EVENT* pEvent, int zone_number)
{
	int i = 0, j = 0, objectid = 0;
	model::Npc* pNpcTable = nullptr;
	bool bFindNpcTable = false;
	int offset = 0;
	int nServerNum = 0;
	nServerNum = GetServerNumber(zone_number);
	//if(m_byZone != zone_number)	 return false;
	//if(m_byZone != UNIFY_ZONE)	{
	//	if(m_byZone != nServerNum)	 return false;
	//}

	//if( zone_number > 201 )	return false;	// test
	pNpcTable = m_NpcTableMap.GetData(pEvent->sIndex);
	if (pNpcTable == nullptr)
	{
		bFindNpcTable = false;
		spdlog::error("AiServerInstance::AddObjectEventNpc error: eventId={} zoneId={}",
			pEvent->sIndex, zone_number);
		return false;
	}

	bFindNpcTable = true;

	CNpc* pNpc = new CNpc();

	pNpc->m_sNid = m_sMapEventNpc++;				// 서버 내에서의 고유 번호
	pNpc->m_sSid = (int16_t) pEvent->sIndex;			// MONSTER(NPC) Serial ID

	pNpc->m_byMoveType = 100;
	pNpc->m_byInitMoveType = 100;
	bFindNpcTable = false;

	pNpc->m_byMoveType = 0;
	pNpc->m_byInitMoveType = 0;

	pNpc->m_byBattlePos = 0;

	pNpc->m_fSecForMetor = 4.0f;					// 초당 갈 수 있는 거리..

	pNpc->Load(pNpcTable, false);

	//////// MONSTER POS ////////////////////////////////////////

	pNpc->m_sCurZone = zone_number;

	pNpc->m_byGateOpen = static_cast<uint8_t>(pEvent->sStatus);
	pNpc->m_fCurX = pEvent->fPosX;
	pNpc->m_fCurY = pEvent->fPosY;
	pNpc->m_fCurZ = pEvent->fPosZ;

	pNpc->m_nInitMinX = static_cast<int>(pEvent->fPosX - 1);
	pNpc->m_nInitMinY = static_cast<int>(pEvent->fPosZ - 1);
	pNpc->m_nInitMaxX = static_cast<int>(pEvent->fPosX + 1);
	pNpc->m_nInitMaxY = static_cast<int>(pEvent->fPosZ + 1);

	pNpc->m_sRegenTime = 10000 * 1000;	// 초(DB)단위-> 밀리세컨드로
	//pNpc->m_sRegenTime		= 30 * 1000;	// 초(DB)단위-> 밀리세컨드로
	pNpc->m_sMaxPathCount = 0;

	pNpc->m_ZoneIndex = -1;
	pNpc->m_byObjectType = SPECIAL_OBJECT;
	pNpc->m_bFirstLive = 1;		// 처음 살아난 경우로 해줘야 한다..
	//pNpc->m_ZoneIndex = GetZoneIndex(pNpc->m_sCurZone);
/*
	if(pNpc->m_ZoneIndex == -1)	{
		AfxMessageBox("Invaild zone Index!!");
		return false;
	}	*/

	//pNpc->Init();
	if (!m_NpcMap.PutData(pNpc->m_sNid, pNpc))
	{
		spdlog::warn("AiServerInstance::AddObjectEventNpc: Npc PutData Fail [serial={}]",
			pNpc->m_sNid);
		delete pNpc;
		pNpc = nullptr;
	}

	m_TotalNPC = m_sMapEventNpc;

	return true;
}

int AiServerInstance::GetZoneIndex(int zoneId) const
{
	for (size_t i = 0; i < m_ZoneArray.size(); i++)
	{
		MAP* pMap = m_ZoneArray[i];
		if (pMap != nullptr
			&& pMap->m_nZoneNumber == zoneId)
			return i;
	}

	spdlog::error("AiServerInstance::GetZoneIndex: zoneId={} not found", zoneId);
	return -1;
}

int AiServerInstance::GetServerNumber(int zoneId) const
{
	for (MAP* pMap : m_ZoneArray)
	{
		if (pMap != nullptr
			&& pMap->m_nZoneNumber == zoneId)
			return pMap->m_nServerNo;
	}

	spdlog::error("AiServerInstance::GetServerNumber: zoneId={} not found", zoneId);
	return -1;
}

void AiServerInstance::GetServerInfoIni()
{
	std::string exePath = GetProgPath();
	std::filesystem::path iniPath(exePath);
	iniPath /= L"server.ini";
	
	CIni inifile;
	inifile.Load(iniPath);

	// logger setup
	_logger.Setup(inifile, exePath);
	
	m_byZone = inifile.GetInt("SERVER", "ZONE", 1);

	std::string datasourceName = inifile.GetString("ODBC", "GAME_DSN", "KN_online");
	std::string datasourceUser = inifile.GetString("ODBC", "GAME_UID", "knight");
	std::string datasourcePass = inifile.GetString("ODBC", "GAME_PWD", "knight");

	ConnectionManager::SetDatasourceConfig(
		modelUtil::DbType::GAME,
		datasourceName, datasourceUser, datasourcePass);

	// Trigger a save to flush defaults to file.
	inifile.Save();
}

void AIServerLogger::SetupExtraLoggers(CIni& ini,
	std::shared_ptr<spdlog::details::thread_pool> threadPool,
	const std::string& baseDir)
{
	SetupExtraLogger(ini, threadPool, baseDir, logger::AIServerItem, ini::ITEM_LOG_FILE);
	SetupExtraLogger(ini, threadPool, baseDir, logger::AIServerUser, ini::USER_LOG_FILE);
}

void AiServerInstance::SendSystemMsg(const std::string_view msg, int zone, int type, int who)
{
	int send_index = 0;
	char buff[256] = {};

	SetByte(buff, AG_SYSTEM_MSG, send_index);
	SetByte(buff, type, send_index);				// 채팅형식
	SetShort(buff, who, send_index);				// 누구에게
	SetString2(buff, msg, send_index);

	Send(buff, send_index, zone);
	spdlog::info("AiServerInstance::SendSystemMsg: zoneId={} type={} who={} msg={}",
		zone, type, who, msg);
}

void AiServerInstance::ResetBattleZone()
{
	spdlog::debug("AiServerInstance::ResetBattleZone: start");

	for (MAP* pMap : m_ZoneArray)
	{
		if (pMap== nullptr)
			continue;

		// 현재의 존이 던젼담당하는 존이 아니면 리턴..
		if (pMap->m_byRoomEvent == 0)
			continue;

		// 전체방이 클리어 되었다면
		// if (pMap->IsRoomStatusCheck())
		//	continue;

		pMap->InitializeRoom();
	}

	spdlog::debug("AiServerInstance::ResetBattleZone: end");
}

MAP* AiServerInstance::GetMapByIndex(int iZoneIndex) const
{
	if (iZoneIndex < 0
		|| iZoneIndex >= static_cast<int>(m_ZoneArray.size()))
	{
		spdlog::error("AiServerInstance::GetMapByIndex: zoneIndex={} out of bounds", iZoneIndex);
		return nullptr;
	}

	return m_ZoneArray[iZoneIndex];
}

MAP* AiServerInstance::GetMapByID(int iZoneID) const
{
	for (MAP* pMap : m_ZoneArray)
	{
		if (pMap != nullptr
			&& pMap->m_nZoneNumber == iZoneID)
			return pMap;
	}
	spdlog::error("AiServerInstance::GetMapByID: no map found for zoneId={}", iZoneID);
	return nullptr;
}

#include "pch.h"
#include "AujardApp.h"
#include "AujardReadQueueThread.h"

#include <db-library/ConnectionManager.h>
#include <db-library/RecordSetLoader_STLMap.h>

#include <shared/Ini.h>
#include <shared/StringUtils.h>
#include <shared/TimerThread.h>

#include <spdlog/spdlog.h>

using namespace std::chrono_literals;

// Minimum time without a heartbeat to consider saving player data.
// This will only save periodically.
static constexpr auto SECONDS_SINCE_LAST_HEARTBEAT_TO_SAVE = 30s;

constexpr int MAX_SMQ_SEND_QUEUE_RETRY_COUNT = 50;

#include <Aujard/binder/AujardBinder.h>
#include <Aujard/model/AujardModel.h>
namespace model = aujard_model;

AujardApp::AujardApp(logger::Logger& logger)
	: AppThread(logger),
	LoggerSendQueue(MAX_SMQ_SEND_QUEUE_RETRY_COUNT)
{
	_sendPacketCount = 0;
	_packetCount = 0;
	_recvPacketCount = 0;

	db::ConnectionManager::DefaultConnectionTimeout = DB_PROCESS_TIMEOUT;
	db::ConnectionManager::Create();

	_dbPoolCheckThread = std::make_unique<TimerThread>(
		1min,
		std::bind(&db::ConnectionManager::ExpireUnusedPoolConnections));

	_heartbeatCheckThread = std::make_unique<TimerThread>(
		40s,
		std::bind(&AujardApp::CheckHeartbeat, this));

	_concurrentCheckThread = std::make_unique<TimerThread>(
		5min,
		std::bind(&AujardApp::ConCurrentUserCount, this));

	_packetCheckThread = std::make_unique<TimerThread>(
		2min,
		std::bind(&AujardApp::WritePacketLog, this));

	_readQueueThread = std::make_unique<AujardReadQueueThread>();

	_smqOpenThread = std::make_unique<TimerThread>(
		5s,
		std::bind(&AujardApp::AttemptOpenSharedMemoryThreadTick, this));
}

AujardApp::~AujardApp()
{
	spdlog::info("AujardApp::~AujardApp: Shutting down, releasing resources.");

	spdlog::info("AujardApp::~AujardApp: Waiting for worker threads to fully shut down.");

	if (_readQueueThread != nullptr)
	{
		spdlog::info("AujardApp::~AujardApp: Shutting down ReadQueueThread...");

		_readQueueThread->shutdown();

		spdlog::info("AujardApp::~AujardApp: ReadQueueThread stopped.");
	}

	if (_dbPoolCheckThread != nullptr)
	{
		spdlog::info("AujardApp::~AujardApp: Shutting down DB pool check thread...");

		_dbPoolCheckThread->shutdown();

		spdlog::info("AujardApp::~AujardApp: DB pool check thread stopped.");
	}

	if (_heartbeatCheckThread != nullptr)
	{
		spdlog::info("AujardApp::~AujardApp: Shutting down heartbeat check thread...");

		_heartbeatCheckThread->shutdown();

		spdlog::info("AujardApp::~AujardApp: heartbeat check thread stopped.");
	}

	if (_concurrentCheckThread != nullptr)
	{
		spdlog::info("AujardApp::~AujardApp: Shutting down concurrent check thread...");

		_concurrentCheckThread->shutdown();

		spdlog::info("AujardApp::~AujardApp: concurrent check thread stopped.");
	}

	if (_packetCheckThread != nullptr)
	{
		spdlog::info("AujardApp::~AujardApp: Shutting down packet check thread...");

		_packetCheckThread->shutdown();

		spdlog::info("AujardApp::~AujardApp: packet check thread stopped.");
	}

	spdlog::info("AujardApp::~AujardApp: All resources safely released.");

	db::ConnectionManager::Destroy();
}

/// \returns The application's ini config path.
std::filesystem::path AujardApp::ConfigPath() const
{
	return GetProgPath() / "Aujard.ini";
}

/// \brief Loads application-specific config from the loaded application ini file (`iniFile`).
/// \param iniFile The loaded application ini file.
/// \returns true when successful, false otherwise
bool AujardApp::LoadConfig(CIni& iniFile)
{
	// TODO: This should be Knight_Account
	// TODO: This should only fetch the once.
	// The above won't be necessary after stored procedures are replaced, so it can be replaced then.
	std::string datasourceName, datasourceUser, datasourcePass;

	// TODO: This should be Knight_Account
	datasourceName = iniFile.GetString(ini::ODBC, ini::ACCOUNT_DSN, "KN_online");
	datasourceUser = iniFile.GetString(ini::ODBC, ini::ACCOUNT_UID, "knight");
	datasourcePass = iniFile.GetString(ini::ODBC, ini::ACCOUNT_PWD, "knight");

	db::ConnectionManager::SetDatasourceConfig(
		modelUtil::DbType::ACCOUNT,
		datasourceName, datasourceUser, datasourcePass);

	datasourceName = iniFile.GetString(ini::ODBC, ini::GAME_DSN, "KN_online");
	datasourceUser = iniFile.GetString(ini::ODBC, ini::GAME_UID, "knight");
	datasourcePass = iniFile.GetString(ini::ODBC, ini::GAME_PWD, "knight");

	db::ConnectionManager::SetDatasourceConfig(
		modelUtil::DbType::GAME,
		datasourceName, datasourceUser, datasourcePass);

	_serverId = iniFile.GetInt(ini::ZONE_INFO, ini::GROUP_INFO, 1);
	_zoneId = iniFile.GetInt(ini::ZONE_INFO, ini::ZONE_INFO, 1);

	return true;
}

/// \brief Initializes the server, loading caches, attempting to load shared memory etc.
/// \returns true when successful, false otherwise
bool AujardApp::OnStart()
{
	if (!_dbAgent.InitDatabase())
		return false;

	if (!LoadItemTable())
		return false;

	// Attempt to open all shared memory queues & blocks first.
	// If it fails (memory not yet available), we'll run the _smqOpenThread to periodically check
	// until it can finally be opened.
	if (!AttemptOpenSharedMemory())
	{
		spdlog::info("AujardApp::OnStart: shared memory unavailable, waiting for memory to become available");
		_smqOpenThread->start();
	}
	else
	{
		OnSharedMemoryOpened();
	}

	return true;
}

/// \brief Attempts to open all shared memory queues & blocks that we've yet to open.
bool AujardApp::AttemptOpenSharedMemory()
{
	bool openedAll = true;

	if (!LoggerRecvQueue.IsOpen()
		&& !LoggerRecvQueue.Open(SMQ_LOGGERSEND))
		openedAll = false;

	if (!LoggerSendQueue.IsOpen()
		&& !LoggerSendQueue.Open(SMQ_LOGGERRECV))
		openedAll = false;

	// NOTE: This looks unsafe, but the server isn't started up until all of this finishes,
	//       so nothing else is using _dbAgent.UserData yet.
	if (_dbAgent.UserData.empty()
		&& !InitSharedMemory())
		openedAll = false;

	return openedAll;
}

/// \brief Thread tick attempting to open all shared memory queues & blocks that we've yet to open.
/// \see AttemptOpenSharedMemory
void AujardApp::AttemptOpenSharedMemoryThreadTick()
{
	if (!AttemptOpenSharedMemory())
		return;

	// Shared memory is open, this thread doesn't need to exist anymore.
	_smqOpenThread->shutdown(false);

	// Run the server
	OnSharedMemoryOpened();
}

/// \brief Finishes server initialization and starts processing threads.
void AujardApp::OnSharedMemoryOpened()
{
	_dbPoolCheckThread->start();
	_heartbeatCheckThread->start();
	_concurrentCheckThread->start();
	_packetCheckThread->start();
	_readQueueThread->start();

	spdlog::info("AujardApp::OnSharedMemoryOpened: server started, processing requests");
}

/// \brief initializes shared memory with other server applications
bool AujardApp::InitSharedMemory()
{
	char* memory = _userDataBlock.Open("KNIGHT_DB");
	if (memory == nullptr)
		return false;

	spdlog::info("AujardApp::InitSharedMemory: shared memory loaded successfully");

	_dbAgent.UserData.reserve(MAX_USER);

	for (int i = 0; i < MAX_USER; i++)
	{
		_USER_DATA* pUser = reinterpret_cast<_USER_DATA*>(memory + i * ALLOCATED_USER_DATA_BLOCK);
		_dbAgent.UserData.push_back(pUser);
	}

	return true;
}

/// \brief loads information needed from the ITEM table to a cache map
bool AujardApp::LoadItemTable()
{
	recordset_loader::STLMap loader(ItemArray);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("AujardApp::LoadItemTable: failed: {}",
			loader.GetError().Message);
		return false;
	}

	return true;
}

/// \brief loads and sends data after a character is selected
void AujardApp::SelectCharacter(const char* buffer)
{
	int index = 0, userId = -1, sendIndex = 0, idLen1 = 0, idLen2 = 0, tempUserId = -1,
		packetIndex = 0;
	uint8_t init = 0x01;
	char sendBuff[256] = {},
		accountId[MAX_ID_SIZE + 1] = {},
		charId[MAX_ID_SIZE + 1] = {};

	_USER_DATA* user = nullptr;

	userId = GetShort(buffer, index);
	idLen1 = GetShort(buffer, index);
	GetString(accountId, buffer, idLen1, index);
	idLen2 = GetShort(buffer, index);
	GetString(charId, buffer, idLen2, index);
	init = GetByte(buffer, index);
	packetIndex = GetDWORD(buffer, index);

	spdlog::debug("AujardApp::SelectCharacter: acctId={}, charId={}, index={}",
		accountId, charId, packetIndex);
	
	_recvPacketCount++;		// packet count

	if (userId < 0
		|| userId >= MAX_USER)
		goto fail_return;

	if (strlen(accountId) == 0)
		goto fail_return;

	if (strlen(charId) == 0)
		goto fail_return;

	if (GetUserPtr(charId, tempUserId) != nullptr)
	{
		SetShort(sendBuff, tempUserId, sendIndex);
		SetShort(sendBuff, idLen1, sendIndex);
		SetString(sendBuff, accountId, idLen1, sendIndex);
		SetShort(sendBuff, idLen2, sendIndex);
		SetString(sendBuff, charId, idLen2, sendIndex);
		UserLogOut(sendBuff);
		return;
	}

	if (!_dbAgent.LoadUserData(accountId, charId, userId))
		goto fail_return;

	if (!_dbAgent.LoadWarehouseData(accountId, userId))
		goto fail_return;

	user = _dbAgent.UserData[userId];
	if (user == nullptr)
		goto fail_return;

	if (strcpy_safe(user->m_Accountid, accountId) != 0)
	{
		spdlog::error("AujardApp::SelectCharacter: accountId too long (len: {}, val: {})",
			std::strlen(accountId), accountId);
		// it didn't fail here before and we don't currently return anything upstream
		// if this exposes any problems we'll have to decide how to handle it then
	}

	SetByte(sendBuff, WIZ_SEL_CHAR, sendIndex);
	SetShort(sendBuff, userId, sendIndex);
	SetByte(sendBuff, 0x01, sendIndex);
	SetByte(sendBuff, init, sendIndex);

	_packetCount++;	

	if (LoggerSendQueue.PutData(sendBuff, sendIndex) != SMQ_OK)
	{
		spdlog::error("AujardApp::SelectCharacter: Packet Drop: WIZ_SEL_CHAR");
		return;
	}

	++_sendPacketCount;
	return;

fail_return:
	sendIndex = 0;
	SetByte(sendBuff, WIZ_SEL_CHAR, sendIndex);
	SetShort(sendBuff, userId, sendIndex);
	SetByte(sendBuff, 0x00, sendIndex);

	LoggerSendQueue.PutData(sendBuff, sendIndex);
}

/// \brief Handles a WIZ_LOGOUT request when logging out of the game
/// \details Updates USERDATA and WAREHOUSE as part of logging out, then resets the UserData entry for re-use
/// \see WIZ_LOGOUT
void AujardApp::UserLogOut(const char* buffer)
{
	int index = 0, userId = -1, accountIdLen = 0,
		charIdLen = 0, sendIndex = 0;
	char charId[MAX_ID_SIZE + 1] = {},
		accountId[MAX_ID_SIZE + 1] = {},
		sendBuff[256] = {};

	userId = GetShort(buffer, index);
	accountIdLen = GetShort(buffer, index);
	GetString(accountId, buffer, accountIdLen, index);
	charIdLen = GetShort(buffer, index);
	GetString(charId, buffer, charIdLen, index);

	if (userId < 0
		|| userId >= MAX_USER)
		return;

	if (strlen(charId) == 0)
		return;

	HandleUserLogout(userId, UPDATE_LOGOUT);

	SetByte(sendBuff, WIZ_LOGOUT, sendIndex);
	SetShort(sendBuff, userId, sendIndex);

	if (LoggerSendQueue.PutData(sendBuff, sendIndex) != SMQ_OK)
		spdlog::error("AujardApp::UserLogOut: Packet Drop: WIZ_LOGOUT");
}

/// \brief handles user logout functions
/// \param userId user index for UserData
/// \param saveType one of: UPDATE_LOGOUT, UPDATE_ALL_SAVE
/// \param forceLogout should be set to true in panic situations
/// \see UserLogOut(), AllSaveRoutine(), HandleUserUpdate()
bool AujardApp::HandleUserLogout(int userId, uint8_t saveType, bool forceLogout)
{
	bool logoutResult = false;

	// make sure UserData[userId] value is valid
	_USER_DATA* pUser = _dbAgent.UserData[userId];
	if (pUser == nullptr || std::strlen(pUser->m_id) == 0)
	{
		spdlog::error("AujardApp::HandleUserLogout: Invalid logout: UserData[{}] is not in use", userId);
		return false;
	}

	// only call AccountLogout for non-zone change logouts
	if (pUser->m_bLogout != 2 || forceLogout)
		logoutResult = _dbAgent.AccountLogout(pUser->m_Accountid);
	else
		logoutResult = true;

	// update UserData (USERDATA/WAREHOUSE)
	bool userdataSuccess = HandleUserUpdate(userId, *pUser, saveType);
	
	// Log results
	bool success = userdataSuccess && logoutResult;
	if (!success)
	{
		spdlog::error("AujardApp::HandleUserLogout: Invalid Logout: {}, {} (UserData: {}, Logout: {})",
			pUser->m_Accountid, pUser->m_id, userdataSuccess, logoutResult);
		return false;
	}

	spdlog::debug("AujardApp::HandleUserLogout: Logout: {}, {} (UserData: {}, Logout: {})",
	pUser->m_Accountid, pUser->m_id, userdataSuccess, logoutResult);

	// reset the object stored in UserData[userId] before returning
	// this will reset data like accountId/charId, so logging must
	// occur beforehand
	_dbAgent.ResetUserData(userId);
	
	return success;
}

/// \brief handles user update functions and retry logic
/// \param userId user index for UserData
/// \param user reference to user object
/// \param saveType one of: UPDATE_LOGOUT, UPDATE_ALL_SAVE, UPDATE_PACKET_SAVE
/// \see UserDataSave(), HandleUserLogout()
bool AujardApp::HandleUserUpdate(int userId, const _USER_DATA& user, uint8_t saveType)
{
	auto sleepTime = 10ms;
	int updateWarehouseResult = 0, updateUserResult = 0,
		retryCount = 0, maxRetry = 10;

	// attempt updates
	updateUserResult = _dbAgent.UpdateUser(user.m_id, userId, saveType);

	std::this_thread::sleep_for(sleepTime);

	updateWarehouseResult = _dbAgent.UpdateWarehouseData(user.m_Accountid, userId, saveType);

	// TODO:  Seems like the following two loops could/should just be combined
	// retry handling for update user/warehouse
	for (retryCount = 0; !updateWarehouseResult || !updateUserResult; retryCount++)
	{
		if (retryCount >= maxRetry)
		{
			spdlog::error("AujardApp::HandleUserUpdate: UserData Save Error: [accountId={} charId={} (W:{},U:{})]",
				user.m_Accountid, user.m_id, updateWarehouseResult, updateUserResult);
			break;
		}
		// only retry the calls that fail - they're both updating using UserData[userId]->dwTime, so they should sync fine
		if (!updateUserResult)
		{
			updateUserResult = _dbAgent.UpdateUser(user.m_id, userId, saveType);
		}

		std::this_thread::sleep_for(sleepTime);

		if (!updateWarehouseResult)
		{
			updateWarehouseResult = _dbAgent.UpdateWarehouseData(user.m_Accountid, userId, saveType);
		}
	}
	
	// Verify saved data/timestamp
	updateWarehouseResult = _dbAgent.CheckUserData(user.m_Accountid, user.m_id, 1, user.m_dwTime, user.m_iBank);
	updateUserResult = _dbAgent.CheckUserData(user.m_Accountid, user.m_id, 2, user.m_dwTime, user.m_iExp);
	for (retryCount = 0; !updateWarehouseResult || !updateUserResult; retryCount++)
	{
		if (retryCount >= maxRetry)
		{
			if (!updateWarehouseResult)
			{
				spdlog::error("AujardApp::HandleUserUpdate: Warehouse Save Check Error [accountId={} charId={} (W:{})]",
					user.m_Accountid, user.m_id, updateWarehouseResult);
			}

			if (!updateUserResult)
			{
				spdlog::error("AujardApp::HandleUserUpdate: UserData Save Check Error: [accountId={} charId={} (U:{})]",
					user.m_Accountid, user.m_id, updateUserResult);
			}
			break;
		}

		if (!updateWarehouseResult)
		{
			_dbAgent.UpdateWarehouseData(user.m_Accountid, userId, saveType);
			updateWarehouseResult = _dbAgent.CheckUserData(user.m_Accountid, user.m_id, 1, user.m_dwTime, user.m_iBank);
		}

		std::this_thread::sleep_for(sleepTime);

		if (!updateUserResult)
		{
			_dbAgent.UpdateUser(user.m_id, userId, saveType);
			updateUserResult = _dbAgent.CheckUserData(user.m_Accountid, user.m_id, 2, user.m_dwTime, user.m_iExp);
		}
	}

	return static_cast<bool>(updateUserResult) && static_cast<bool>(updateWarehouseResult);
}

/// \brief handles a WIZ_LOGIN request to a selected game server
/// \see WIZ_LOGIN
void AujardApp::AccountLogIn(const char* buffer)
{
	int index = 0, userId = -1, accountIdLen = 0,
		passwordLen = 0, sendIndex = 0;
	int nation = -1;

	char accountId[MAX_ID_SIZE + 1] = {},
		password[MAX_PW_SIZE + 1] = {},
		sendBuff[256] = {};

	userId = GetShort(buffer, index);
	accountIdLen = GetShort(buffer, index);
	GetString(accountId, buffer, accountIdLen, index);
	passwordLen = GetShort(buffer, index);
	GetString(password, buffer, passwordLen, index);

	nation = _dbAgent.AccountLogInReq(accountId, password);

	SetByte(sendBuff, WIZ_LOGIN, sendIndex);
	SetShort(sendBuff, userId, sendIndex);
	SetByte(sendBuff, nation, sendIndex);

	if (LoggerSendQueue.PutData(sendBuff, sendIndex) != SMQ_OK)
		spdlog::error("AujardApp::AccountLogIn: Packet Drop: WIZ_LOGIN");
}

/// \brief handles a WIZ_SEL_NATION request to a selected game server
/// \see WIZ_SEL_NATION
void AujardApp::SelectNation(const char* buffer)
{
	int index = 0, userId = -1, accountIdLen = 0, sendIndex = 0;
	int nation = -1;
	char accountId[MAX_ID_SIZE + 1] = {},
		sendBuff[256] = {};

	userId = GetShort(buffer, index);
	accountIdLen = GetShort(buffer, index);
	GetString(accountId, buffer, accountIdLen, index);
	nation = GetByte(buffer, index);

	bool result = _dbAgent.NationSelect(accountId, nation);

	SetByte(sendBuff, WIZ_SEL_NATION, sendIndex);
	SetShort(sendBuff, userId, sendIndex);

	if (result)
		SetByte(sendBuff, nation, sendIndex);
	else
		SetByte(sendBuff, 0x00, sendIndex);

	if (LoggerSendQueue.PutData(sendBuff, sendIndex) != SMQ_OK)
		spdlog::error("AujardApp::SelectNation: Packet Drop: WIZ_SEL_NATION");
}

/// \brief handles a WIZ_NEW_CHAR request
/// \see WIZ_NEW_CHAR
void AujardApp::CreateNewChar(const char* buffer)
{
	int index = 0, userId = -1, accountIdLen = 0, charIdLen = 0, sendIndex = 0,
		result = 0, charIndex = 0, race = 0, Class = 0, hair = 0,
		face = 0, str = 0, sta = 0, dex = 0, intel = 0, cha = 0;
	char accountId[MAX_ID_SIZE + 1] = {},
		charId[MAX_ID_SIZE + 1] = {},
		sendBuff[256] = {};

	userId = GetShort(buffer, index);
	accountIdLen = GetShort(buffer, index);
	GetString(accountId, buffer, accountIdLen, index);
	charIndex = GetByte(buffer, index);
	charIdLen = GetShort(buffer, index);
	GetString(charId, buffer, charIdLen, index);
	race = GetByte(buffer, index);
	Class = GetShort(buffer, index);
	face = GetByte(buffer, index);
	hair = GetByte(buffer, index);
	str = GetByte(buffer, index);
	sta = GetByte(buffer, index);
	dex = GetByte(buffer, index);
	intel = GetByte(buffer, index);
	cha = GetByte(buffer, index);

	result = _dbAgent.CreateNewChar(accountId, charIndex, charId, race, Class, hair, face, str, sta, dex, intel, cha);

	SetByte(sendBuff, WIZ_NEW_CHAR, sendIndex);
	SetShort(sendBuff, userId, sendIndex);
	SetByte(sendBuff, result, sendIndex);

	if (LoggerSendQueue.PutData(sendBuff, sendIndex) != SMQ_OK)
		spdlog::error("AujardApp::CreateNewChar: Packet Drop: WIZ_NEW_CHAR");
}

/// \brief handles a WIZ_DEL_CHAR request
/// \todo not implemented, always returns an error to the client
/// \see WIZ_DEL_CHAR
void AujardApp::DeleteChar(const char* buffer)
{
	int index = 0, userId = -1, accountIdLen = 0, charIdLen = 0,
		sendIndex = 0, result = 0, charIndex = 0, socNoLen = 0;
	char accountId[MAX_ID_SIZE + 1] = {},
		charId[MAX_ID_SIZE + 1] = {},
		socNo[15] = {},
		sendBuff[256] = {};

	userId = GetShort(buffer, index);
	accountIdLen = GetShort(buffer, index);
	GetString(accountId, buffer, accountIdLen, index);
	charIndex = GetByte(buffer, index);
	charIdLen = GetShort(buffer, index);
	GetString(charId, buffer, charIdLen, index);
	socNoLen = GetShort(buffer, index);
	GetString(socNo, buffer, socNoLen, index);

	// Not implemented.  Allow result to default to 0.
	//result = _dbAgent.DeleteChar(charindex, accountid, charid, socno);

	spdlog::trace("AujardApp::DeleteChar: [charId={}, socNo={}]", charId, socNo);

	SetByte(sendBuff, WIZ_DEL_CHAR, sendIndex);
	SetShort(sendBuff, userId, sendIndex);
	SetByte(sendBuff, result, sendIndex);
	if (result > 0)
		SetByte(sendBuff, charIndex, sendIndex);
	else
		SetByte(sendBuff, 0xFF, sendIndex);

	if (LoggerSendQueue.PutData(sendBuff, sendIndex) != SMQ_OK)
		spdlog::error("AujardApp::DeleteChar: Packet Drop: WIZ_DEL_CHAR");
}

/// \brief handles a WIZ_ALLCHAR_INFO_REQ request
/// \details Loads all character information and sends it to the client
/// \see WIZ_ALLCHAR_INFO_REQ
void AujardApp::AllCharInfoReq(const char* buffer)
{
	int index = 0, userId = 0, accountIdLen = 0, sendIndex = 0,
		charBuffIndex = 0;
	char accountId[MAX_ID_SIZE + 1] = {},
		sendBuff[1024] = {},
		charBuff[1024] = {},
		charId1[MAX_ID_SIZE + 1] = {},
		charId2[MAX_ID_SIZE + 1] = {},
		charId3[MAX_ID_SIZE + 1] = {};

	userId = GetShort(buffer, index);
	accountIdLen = GetShort(buffer, index);
	GetString(accountId, buffer, accountIdLen, index);

	SetByte(charBuff, 0x01, charBuffIndex);	// result

	_dbAgent.GetAllCharID(accountId, charId1, charId2, charId3);
	_dbAgent.LoadCharInfo(charId1, charBuff, charBuffIndex);
	_dbAgent.LoadCharInfo(charId2, charBuff, charBuffIndex);
	_dbAgent.LoadCharInfo(charId3, charBuff, charBuffIndex);

	SetByte(sendBuff, WIZ_ALLCHAR_INFO_REQ, sendIndex);
	SetShort(sendBuff, userId, sendIndex);
	SetShort(sendBuff, charBuffIndex, sendIndex);
	SetString(sendBuff, charBuff, charBuffIndex, sendIndex);

	if (LoggerSendQueue.PutData(sendBuff, sendIndex) != SMQ_OK)
		spdlog::error("AujardApp::AllCharInfoReq: Packet Drop: WIZ_ALLCHAR_INFO_REQ");
}

/// \brief handling for when CheckHeartbeat detects no heatbeat from Ebenezer for a time
/// \details Logs ebenezer outage, attempts to save all UserData, and resets all UserData[userId] objects
/// \see CheckHeartbeat()
void AujardApp::AllSaveRoutine()
{
	// TODO:  100ms seems excessive
	auto sleepTime = 100ms;
	bool allUsersSaved = true;

	// log the disconnect
	spdlog::error("AujardApp::AllSaveRoutine: Ebenezer disconnected. Saving users...");
	
	for (int userId = 0; userId < static_cast<int>(_dbAgent.UserData.size()); userId++)
	{
		_USER_DATA* pUser = _dbAgent.UserData[userId];
		if (pUser == nullptr)
		{
			spdlog::debug("AujardApp::AllSaveRoutine: userId skipped for invalid data: {}", userId);
			continue;
		}

		if (strlen(pUser->m_id) == 0)
			continue;

		if (HandleUserLogout(userId, UPDATE_ALL_SAVE, true))
		{
			spdlog::debug("AujardApp::AllSaveRoutine: Character saved: {}", pUser->m_id);
		}
		else
		{
			allUsersSaved = false;
			spdlog::error("AujardApp::AllSaveRoutine: failed to save character: {}", pUser->m_id);
		}

		std::this_thread::sleep_for(sleepTime);
	}

	if (allUsersSaved)
		spdlog::info("AujardApp::AllSaveRoutine: Ebenezer disconnect: all users saved successfully");
	else
		spdlog::error("AujardApp::AllSaveRoutine: Ebenezer disconnect: not all users saved");
}

/// \brief Called every 5min by _concurrentCheckThread
/// \see _concurrentCheckThread
void AujardApp::ConCurrentUserCount()
{
	int usercount = 0;

	for (int userId = 0; userId < MAX_USER; userId++)
	{
		_USER_DATA* pUser = _dbAgent.UserData[userId];
		if (pUser == nullptr)
			continue;

		if (strlen(pUser->m_id) == 0)
			continue;

		usercount++;
	}

	spdlog::trace("AujardApp::ConCurrentUserCount: [serverId={} zoneId={} userCount={}]",
		_serverId, _zoneId, usercount);

	_dbAgent.UpdateConCurrentUserCount(_serverId, _zoneId, usercount);
}

/// \brief handles a WIZ_DATASAVE request
/// \see WIZ_DATASAVE
/// \see HandleUserUpdate()
void AujardApp::UserDataSave(const char* buffer)
{
	int index = 0, userId = -1, accountIdLen = 0, charIdLen = 0;
	char accountId[MAX_ID_SIZE + 1] = {},
		charId[MAX_ID_SIZE + 1] = {};

	userId = GetShort(buffer, index);
	accountIdLen = GetShort(buffer, index);
	GetString(accountId, buffer, accountIdLen, index);
	charIdLen = GetShort(buffer, index);
	GetString(charId, buffer, charIdLen, index);

	if (userId < 0
		|| userId >= MAX_USER
		|| strlen(accountId) == 0
		|| strlen(charId) == 0)
		return;
	
	_USER_DATA* pUser = _dbAgent.UserData[userId];
	if (pUser == nullptr)
		return;

	bool userdataSuccess = HandleUserUpdate(userId, *pUser, UPDATE_PACKET_SAVE);
	if (!userdataSuccess)
	{
		spdlog::error("AujardApp::UserDataSave: failed for UserData[{}] [accountId={} charId={}]",
			userId, accountId, charId);
	}
}

/// \brief attempts to find a UserData record for charId
/// \param charId
/// \param[out] userId UserData index of the user, if found
/// \return pointer to UserData[userId] object if found, nullptr otherwise
_USER_DATA* AujardApp::GetUserPtr(const char* charId, int& userId)
{
	for (int i = 0; i < MAX_USER; i++)
	{
		_USER_DATA* pUser = _dbAgent.UserData[i];
		if (!pUser)
			continue;

		if (strnicmp(charId, pUser->m_id, MAX_ID_SIZE) == 0)
		{
			userId = i;
			return pUser;
		}
	}

	return nullptr;
}

/// \brief handles WIZ_KNIGHTS_PROCESS and WIZ_CLAN_PROCESS requests
/// \detail calls the appropriate method for the subprocess op-code
/// \see WIZ_KNIGHTS_PROCESS WIZ_CLAN_PROCESS
/// \see "Knights Packet sub define" section in Define.h
void AujardApp::KnightsPacket(const char* buffer)
{
	int index = 0, nation = 0;
	uint8_t command = GetByte(buffer, index);
	switch (command)
	{
		case KNIGHTS_CREATE:
			CreateKnights(buffer + index);
			break;

		case KNIGHTS_JOIN:
			JoinKnights(buffer + index);
			break;

		case KNIGHTS_WITHDRAW:
			WithdrawKnights(buffer + index);
			break;

		case KNIGHTS_REMOVE:
		case KNIGHTS_ADMIT:
		case KNIGHTS_REJECT:
		case KNIGHTS_CHIEF:
		case KNIGHTS_VICECHIEF:
		case KNIGHTS_OFFICER:
		case KNIGHTS_PUNISH:
			ModifyKnightsMember(buffer + index, command);
			break;

		case KNIGHTS_DESTROY:
			DestroyKnights(buffer + index);
			break;

		case KNIGHTS_MEMBER_REQ:
			AllKnightsMember(buffer + index);
			break;

		case KNIGHTS_STASH:
			break;

		case KNIGHTS_LIST_REQ:
			KnightsList(buffer + index);
			break;

		case KNIGHTS_ALLLIST_REQ:
			nation = GetByte(buffer, index);
			_dbAgent.LoadKnightsAllList(nation);
			break;

		default:
			spdlog::error("AujardApp::KnightsPacket: Invalid WIZ_KNIGHTS_PROCESS command code received: {:X}",
				command);
	}
}

/// \brief attempts to create a knights clan
/// \see KnightsPacket(), KNIGHTS_CREATE
void AujardApp::CreateKnights(const char* buffer)
{
	int index = 0, sendIndex = 0, nameLen = 0, chiefNameLen = 0, knightsId = 0,
		userId = -1, nation = 0, result = 0, community = 0;
	char knightsName[MAX_ID_SIZE + 1] = {},
		chiefName[MAX_ID_SIZE + 1] = {},
		sendBuff[256] = {};

	userId = GetShort(buffer, index);
	community = GetByte(buffer, index);
	knightsId = GetShort(buffer, index);
	nation = GetByte(buffer, index);
	nameLen = GetShort(buffer, index);
	GetString(knightsName, buffer, nameLen, index);
	chiefNameLen = GetShort(buffer, index);
	GetString(chiefName, buffer, chiefNameLen, index);

	if (userId < 0
		|| userId >= MAX_USER)
		return;

	result = _dbAgent.CreateKnights(knightsId, nation, knightsName, chiefName, community);

	spdlog::trace("AujardApp::CreateKnights: userId={}, knightsId={}, result={}",
		userId, knightsId, result);

	SetByte(sendBuff, KNIGHTS_CREATE, sendIndex);
	SetShort(sendBuff, userId, sendIndex);
	SetByte(sendBuff, result, sendIndex);
	SetByte(sendBuff, community, sendIndex);
	SetShort(sendBuff, knightsId, sendIndex);
	SetByte(sendBuff, nation, sendIndex);
	SetShort(sendBuff, nameLen, sendIndex);
	SetString(sendBuff, knightsName, nameLen, sendIndex);
	SetShort(sendBuff, chiefNameLen, sendIndex);
	SetString(sendBuff, chiefName, chiefNameLen, sendIndex);

	if (LoggerSendQueue.PutData(sendBuff, sendIndex) != SMQ_OK)
		spdlog::error("AujardApp::CreateKnights: Packet Drop: KNIGHTS_CREATE");
}

/// \brief attempts to add a character to a knights clan
/// \see KnightsPacket(), KNIGHTS_JOIN
void AujardApp::JoinKnights(const char* buffer)
{
	int index = 0, sendIndex = 0, knightsId = 0, userId = -1;
	uint8_t result = 0;
	char sendBuff[256] = {};

	userId = GetShort(buffer, index);
	knightsId = GetShort(buffer, index);

	if (userId < 0
		|| userId >= MAX_USER)
		return;

	_USER_DATA* pUser = _dbAgent.UserData[userId];
	if (pUser == nullptr)
		return;

	result = _dbAgent.UpdateKnights(KNIGHTS_JOIN, pUser->m_id, knightsId, 0);

	spdlog::trace("AujardApp::JoinKnights: userId={}, charId={}, knightsId={}, result={}",
		userId, pUser->m_id, knightsId, result);

	SetByte(sendBuff, KNIGHTS_JOIN, sendIndex);
	SetShort(sendBuff, userId, sendIndex);
	SetByte(sendBuff, result, sendIndex);
	SetShort(sendBuff, knightsId, sendIndex);

	if (LoggerSendQueue.PutData(sendBuff, sendIndex) != SMQ_OK)
		spdlog::error("AujardApp::JoinKnights: Packet Drop: KNIGHTS_JOIN");
}

/// \brief attempt to remove a character from a knights clan
/// \see KnightsPacket(), KNIGHTS_WITHDRAW
void AujardApp::WithdrawKnights(const char* buffer)
{
	int index = 0, sendIndex = 0, knightsId = 0, userId = -1;
	uint8_t result = 0;
	char sendBuff[256] = {};

	userId = GetShort(buffer, index);
	knightsId = GetShort(buffer, index);

	if (userId < 0
		|| userId >= MAX_USER)
		return;

	_USER_DATA* pUser = _dbAgent.UserData[userId];
	if (pUser == nullptr)
		return;

	result = _dbAgent.UpdateKnights(KNIGHTS_WITHDRAW, pUser->m_id, knightsId, 0);
	spdlog::trace("AujardApp::WithdrawKnights: userId={}, knightsId={}, result={}",
		userId, knightsId, result);

	SetByte(sendBuff, KNIGHTS_WITHDRAW, sendIndex);
	SetShort(sendBuff, userId, sendIndex);
	SetByte(sendBuff, result, sendIndex);
	SetShort(sendBuff, knightsId, sendIndex);

	if (LoggerSendQueue.PutData(sendBuff, sendIndex) != SMQ_OK)
		spdlog::error("AujardApp::WithdrawKnights: Packet Drop: KNIGHTS_WITHDRAW");
}

/// \brief attempts to modify a knights character
/// \see KnightsPacket(), KNIGHTS_REMOVE, KNIGHTS_ADMIT, KNIGHTS_REJECT, KNIGHTS_CHIEF,
/// KNIGHTS_VICECHIEF, KNIGHTS_OFFICER, KNIGHTS_PUNISH
void AujardApp::ModifyKnightsMember(const char* buffer, uint8_t command)
{
	int index = 0, sendIndex = 0, knightsId = 0, userId = -1, charIdLen = 0,
		removeFlag = 0;
	uint8_t result = 0;
	char sendBuff[256] = {},
		charId[MAX_ID_SIZE + 1] = {};

	userId = GetShort(buffer, index);
	knightsId = GetShort(buffer, index);
	charIdLen = GetShort(buffer, index);
	GetString(charId, buffer, charIdLen, index);
	removeFlag = GetByte(buffer, index);

	if (userId < 0
		|| userId >= MAX_USER)
		return;

/*	if( remove_flag == 0 && command == KNIGHTS_REMOVE )	{		// 없는 유저 추방시에는 디비에서만 처리한다
		result = m_DBAgent.UpdateKnights( command, userid, knightindex, remove_flag );
		TRACE(_T("ModifyKnights - command=%d, nid=%d, index=%d, result=%d \n"), command, uid, knightindex, result);
		return;
	}	*/

	result = _dbAgent.UpdateKnights(command, charId, knightsId, removeFlag);
	spdlog::trace("AujardApp::ModifyKnights: command={}, userId={}, knightsId={}, result={}",
		command, userId, knightsId, result);

	//SetByte(sendBuff, WIZ_KNIGHTS_PROCESS, sendBuff);
	SetByte(sendBuff, command, sendIndex);
	SetShort(sendBuff, userId, sendIndex);
	SetByte(sendBuff, result, sendIndex);
	SetShort(sendBuff, knightsId, sendIndex);
	SetShort(sendBuff, charIdLen, sendIndex);
	SetString(sendBuff, charId, charIdLen, sendIndex);
	SetByte(sendBuff, removeFlag, sendIndex);

	if (LoggerSendQueue.PutData(sendBuff, sendIndex) != SMQ_OK)
	{
		std::string cmdStr;
		switch (command)
		{
		case KNIGHTS_REMOVE:
			cmdStr = "KNIGHTS_REMOVE";
			break;
		case KNIGHTS_ADMIT:
			cmdStr = "KNIGHTS_ADMIT";
			break;
		case KNIGHTS_REJECT:
			cmdStr = "KNIGHTS_REJECT";
			break;
		case KNIGHTS_CHIEF:
			cmdStr = "KNIGHTS_CHIEF";
			break;
		case KNIGHTS_VICECHIEF:
			cmdStr = "KNIGHTS_VICECHIEF";
			break;
		case KNIGHTS_OFFICER:
			cmdStr = "KNIGHTS_OFFICER";
			break;
		case KNIGHTS_PUNISH:
			cmdStr = "KNIGHTS_PUNISH";
			break;
		default:
			cmdStr = "ModifyKnightsMember";
		}

		spdlog::error("AujardApp::ModifyKnightsMember: Packet Drop: {}", cmdStr);
	}
}

/// \brief attempts to disband a knights clan
/// \see KnightsPacket(), KNIGHTS_DESTROY
void AujardApp::DestroyKnights(const char* buffer)
{
	int index = 0, sendIndex = 0, knightsId = 0, userId = -1;
	uint8_t result = 0;
	char sendBuff[256] = {};

	userId = GetShort(buffer, index);
	knightsId = GetShort(buffer, index);
	if (userId < 0
		|| userId >= MAX_USER)
		return;

	result = _dbAgent.DeleteKnights(knightsId);
	spdlog::trace("AujardApp::DestroyKnights: userId={}, knightsId={}, result={}",
	userId, knightsId, result);

	SetByte(sendBuff, KNIGHTS_DESTROY, sendIndex);
	SetShort(sendBuff, userId, sendIndex);
	SetByte(sendBuff, result, sendIndex);
	SetShort(sendBuff, knightsId, sendIndex);

	if (LoggerSendQueue.PutData(sendBuff, sendIndex) != SMQ_OK)
		spdlog::error("AujardApp::DestroyKnights: Packet Drop: KNIGHTS_DESTROY");
}

/// \brief attempts to return a list of all knights members
/// \see KnightsPacket(), KNIGHTS_MEMBER_REQ
void AujardApp::AllKnightsMember(const char* buffer)
{
	int index = 0, sendIndex = 0, knightsId = 0, userId = -1,
		dbIndex = 0, count = 0;
	char sendBuff[2048] = {},
		dbBuff[2048] = {};

	userId = GetShort(buffer, index);
	knightsId = GetShort(buffer, index);
	//int page = GetShort( pBuf, index );

	if (userId < 0
		|| userId >= MAX_USER)
		return;

	//if( page < 0 )  return;

	count = _dbAgent.LoadKnightsAllMembers(knightsId, 0, dbBuff, dbIndex);
	//count = m_DBAgent.LoadKnightsAllMembers( knightindex, page*10, temp_buff, buff_index );

	SetByte(sendBuff, KNIGHTS_MEMBER_REQ, sendIndex);
	SetShort(sendBuff, userId, sendIndex);
	SetByte(sendBuff, 0x00, sendIndex);		// Success
	SetShort(sendBuff, 4 + dbIndex, sendIndex);	// total packet size -> int16_t(*3) + buff_index 
	//SetShort( send_buff, page, send_index );
	SetShort(sendBuff, count, sendIndex);
	SetString(sendBuff, dbBuff, dbIndex, sendIndex);

	if (LoggerSendQueue.PutData(sendBuff, sendIndex) != SMQ_OK)
		spdlog::error("AujardApp::AllKnightsMember: Packet Drop: KNIGHTS_MEMBER_REQ");
}

/// \brief attempts to retrieve metadata for a knights clan
/// \see KnightsPacket(), KNIGHTS_LIST_REQ
void AujardApp::KnightsList(const char* buffer)
{
	int index = 0, sendIndex = 0, knightsId = 0, userId = -1,
		dbIndex = 0;
	char sendBuff[256] = {},
		dbBuff[256] = {};

	userId = GetShort(buffer, index);
	knightsId = GetShort(buffer, index);
	if (userId < 0
		|| userId >= MAX_USER)
		return;

	_dbAgent.LoadKnightsInfo(knightsId, dbBuff, dbIndex);

	SetByte(sendBuff, KNIGHTS_LIST_REQ, sendIndex);
	SetShort(sendBuff, userId, sendIndex);
	SetByte(sendBuff, 0x00, sendIndex);
	SetString(sendBuff, dbBuff, dbIndex, sendIndex);

	if (LoggerSendQueue.PutData(sendBuff, sendIndex) != SMQ_OK)
		spdlog::error("AujardApp::KnightsList: Packet Drop: KNIGHTS_LIST_REQ");
}

/// \brief handles WIZ_LOGIN_INFO requests, updating CURRENTUSER for a user
/// \see WIZ_LOGIN_INFO
void AujardApp::SetLogInInfo(const char* buffer)
{
	int index = 0, accountIdLen = 0, charIdLen = 0, serverId = 0, serverIpLen = 0,
	clientIpLen = 0, userId = -1, sendIndex = 0;
	char accountId[MAX_ID_SIZE + 1] = {},
		serverIp[20] = {},
		clientIp[20] = {},
		charId[MAX_ID_SIZE + 1] = {},
		sendBuff[256] = {};

	userId = GetShort(buffer, index);
	accountIdLen = GetShort(buffer, index);
	GetString(accountId, buffer, accountIdLen, index);
	charIdLen = GetShort(buffer, index);
	GetString(charId, buffer, charIdLen, index);
	serverIpLen = GetShort(buffer, index);
	GetString(serverIp, buffer, serverIpLen, index);
	serverId = GetShort(buffer, index);
	clientIpLen = GetShort(buffer, index);
	GetString(clientIp, buffer, clientIpLen, index);
	
	// init: 0x01 to insert, 0x02 to update CURRENTUSER
	uint8_t init = GetByte(buffer, index);

	if (!_dbAgent.SetLogInInfo(accountId, charId, serverIp, serverId, clientIp, init))
	{
		SetByte(sendBuff, WIZ_LOGIN_INFO, sendIndex);
		SetShort(sendBuff, userId, sendIndex);
		SetByte(sendBuff, 0x00, sendIndex);							// FAIL

		if (LoggerSendQueue.PutData(sendBuff, sendIndex) != SMQ_OK)
		{
			spdlog::error("AujardApp::SetLoginInfo: Packet Drop: WIZ_LOGIN_INFO [accountId={}, charId={}, init={}]",
				accountId, charId, init);
		}
	}
}

/// \brief handles WIZ_KICKOUT requests
/// \see WIZ_KICKOUT
void AujardApp::UserKickOut(const char* buffer)
{
	int index = 0, accountIdLen = 0;
	char accountId[MAX_ID_SIZE + 1] = {};

	accountIdLen = GetShort(buffer, index);
	GetString(accountId, buffer, accountIdLen, index);

	_dbAgent.AccountLogout(accountId);
}

/// \brief writes a packet summary line to the log file
void AujardApp::WritePacketLog()
{
	spdlog::info("AujardApp::WritePacketLog: Packet Count: recv={}, send={}, realsend={}",
		_recvPacketCount, _packetCount, _sendPacketCount);
}

/// \brief handles WIZ_BATTLE_EVENT requests
/// \details contains which nation won the war and which charId killed the commander
/// \see WIZ_BATTLE_EVENT
void AujardApp::BattleEventResult(const char* data)
{
	int _type = 0, result = 0, charIdLen = 0, index = 0;
	char charId[MAX_ID_SIZE + 1] = {};

	_type = GetByte(data, index);
	result = GetByte(data, index);
	charIdLen = GetByte(data, index);
	if (charIdLen > 0
		&& charIdLen < MAX_ID_SIZE + 1)
	{
		GetString(charId, data, charIdLen, index);
		spdlog::info("AujardApp::BattleEventResult : The user who killed the enemy commander is {}, _type={}, nation={}",
			charId, _type, result);
		_dbAgent.UpdateBattleEvent(charId, result);
	}
}

/// \brief handles DB_COUPON_EVENT requests
/// \todo related stored procedures are not implemented
/// \see DB_COUPON_EVENT
void AujardApp::CouponEvent(const char* data)
{
	int index = 0, sendIndex = 0;
	char strAccountName[MAX_ID_SIZE + 1] = {},
		strCharName[MAX_ID_SIZE + 1] = {},
		strCouponID[MAX_ID_SIZE + 1] = {},
		sendBuff[256] = {};

	int nType = GetByte(data, index);
	if (nType == CHECK_COUPON_EVENT)
	{
		int nSid = GetShort(data, index);
		int nLen = GetShort(data, index);
		GetString(strAccountName, data, nLen, index);
		int nEventNum = GetDWORD(data, index);
		// 비러머글 대사문 >.<
		int nMessageNum = GetDWORD(data, index);
		// TODO: Not implemented. Allow nResult to default to 0
		// int nResult = _dbAgent.CheckCouponEvent(strAccountName);
		int nResult = 0;

		SetByte(sendBuff, DB_COUPON_EVENT, sendIndex);
		SetShort(sendBuff, nSid, sendIndex);
		SetByte(sendBuff, nResult, sendIndex);
		SetDWORD(sendBuff, nEventNum, sendIndex);
		// 비러머글 대사문 >.<
		SetDWORD(sendBuff, nMessageNum, sendIndex);
		//

		if (LoggerSendQueue.PutData(sendBuff, sendIndex) != SMQ_OK)
			spdlog::error("AujardApp::CouponEvent: Packet Drop: DB_COUPON_EVENT");
	}
	else if (nType == UPDATE_COUPON_EVENT)
	{
		/*int nSid =*/ GetShort(data, index);
		int nLen = GetShort(data, index);
		GetString(strAccountName, data, nLen, index);
		int nCharLen = GetShort(data, index);
		GetString(strCharName, data, nCharLen, index);
		int nCouponLen = GetShort(data, index);
		GetString(strCouponID, data, nCouponLen, index);
		/*int nItemID =*/ GetDWORD(data, index);
		/*int nItemCount =*/ GetDWORD(data, index);

		// TODO: not implemented.  Allow nResult to default to 0
		// int nResult = _dbAgent.UpdateCouponEvent(strAccountName, strCharName, strCouponID, nItemID, nItemCount);
	}
}

void AujardApp::CheckHeartbeat()
{
	if (_heartbeatReceivedTime == 0)
		return;

	time_t currentTime = time(nullptr);
	time_t secondsSinceLastHeartbeat = currentTime - _heartbeatReceivedTime;

	if (secondsSinceLastHeartbeat > SECONDS_SINCE_LAST_HEARTBEAT_TO_SAVE.count())
		AllSaveRoutine();
}

void AujardApp::HeartbeatReceived()
{
	_heartbeatReceivedTime = time(nullptr);
}

#include "pch.h"
#include "AujardApp.h"
#include "AujardReadQueueThread.h"

#include <Aujard/binder/AujardBinder.h>
#include <Aujard/model/AujardModel.h>

#include <db-library/ConnectionManager.h>
#include <db-library/RecordSetLoader_STLMap.h>

#include <shared/Ini.h>
#include <shared/StringUtils.h>
#include <shared/TimerThread.h>

#include <spdlog/spdlog.h>

using namespace std::chrono_literals;

namespace Aujard
{

// Minimum time without a heartbeat to consider saving player data.
// This will only save periodically.
static constexpr auto SECONDS_SINCE_LAST_HEARTBEAT_TO_SAVE = 30s;

constexpr int MAX_SMQ_SEND_QUEUE_RETRY_COUNT               = 50;

namespace model                                            = aujard_model;

AujardApp::AujardApp(logger::Logger& logger) :
	AppThread(logger), LoggerSendQueue(MAX_SMQ_SEND_QUEUE_RETRY_COUNT)
{
	_telnetPort                                     = 2325;

	db::ConnectionManager::DefaultConnectionTimeout = DB_PROCESS_TIMEOUT;
	db::ConnectionManager::Create();

	_dbPoolCheckThread = std::make_unique<TimerThread>(
		1min, std::bind(&db::ConnectionManager::ExpireUnusedPoolConnections));

	_heartbeatCheckThread = std::make_unique<TimerThread>(
		40s, std::bind(&AujardApp::CheckHeartbeat, this));

	_concurrentCheckThread = std::make_unique<TimerThread>(
		5min, std::bind(&AujardApp::ConCurrentUserCount, this));

	_packetCheckThread = std::make_unique<TimerThread>(
		2min, std::bind(&AujardApp::WritePacketLog, this));

	_readQueueThread = std::make_unique<AujardReadQueueThread>();

	_smqOpenThread   = std::make_unique<TimerThread>(
        5s, std::bind(&AujardApp::AttemptOpenSharedMemoryThreadTick, this));
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

std::filesystem::path AujardApp::ConfigPath() const
{
	return "Aujard.ini";
}

bool AujardApp::LoadConfig(CIni& iniFile)
{
	// TODO: This should be Knight_Account. This should only fetch the once.
	// The above won't be necessary after stored procedures are replaced, so it can be replaced then.
	std::string datasourceName, datasourceUser, datasourcePass;

	// TODO: This should be Knight_Account
	datasourceName = iniFile.GetString(ini::ODBC, ini::ACCOUNT_DSN, "KN_online");
	datasourceUser = iniFile.GetString(ini::ODBC, ini::ACCOUNT_UID, "knight");
	datasourcePass = iniFile.GetString(ini::ODBC, ini::ACCOUNT_PWD, "knight");

	db::ConnectionManager::SetDatasourceConfig(
		modelUtil::DbType::ACCOUNT, datasourceName, datasourceUser, datasourcePass);

	datasourceName = iniFile.GetString(ini::ODBC, ini::GAME_DSN, "KN_online");
	datasourceUser = iniFile.GetString(ini::ODBC, ini::GAME_UID, "knight");
	datasourcePass = iniFile.GetString(ini::ODBC, ini::GAME_PWD, "knight");

	db::ConnectionManager::SetDatasourceConfig(
		modelUtil::DbType::GAME, datasourceName, datasourceUser, datasourcePass);

	_serverId = iniFile.GetInt(ini::ZONE_INFO, ini::GROUP_INFO, 1);
	_zoneId   = iniFile.GetInt(ini::ZONE_INFO, ini::ZONE_INFO, 1);

	return true;
}

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
		spdlog::info("AujardApp::OnStart: shared memory unavailable, waiting for memory to become "
					 "available");
		_smqOpenThread->start();
	}
	else
	{
		OnSharedMemoryOpened();
	}

	return true;
}

bool AujardApp::AttemptOpenSharedMemory()
{
	bool openedAll = true;

	if (!LoggerRecvQueue.IsOpen() && !LoggerRecvQueue.Open(SMQ_LOGGERSEND))
		openedAll = false;

	if (!LoggerSendQueue.IsOpen() && !LoggerSendQueue.Open(SMQ_LOGGERRECV))
		openedAll = false;

	// NOTE: This looks unsafe, but the server isn't started up until all of this finishes,
	//       so nothing else is using _dbAgent.UserData yet.
	if (_dbAgent.UserData.empty() && !InitSharedMemory())
		openedAll = false;

	return openedAll;
}

void AujardApp::AttemptOpenSharedMemoryThreadTick()
{
	if (!AttemptOpenSharedMemory())
		return;

	// Shared memory is open, this thread doesn't need to exist anymore.
	_smqOpenThread->shutdown(false);

	// Run the server
	OnSharedMemoryOpened();
}

void AujardApp::OnSharedMemoryOpened()
{
	_dbPoolCheckThread->start();
	_heartbeatCheckThread->start();
	_concurrentCheckThread->start();
	_packetCheckThread->start();
	_readQueueThread->start();

	spdlog::info("AujardApp::OnSharedMemoryOpened: server started, processing requests");
}

bool AujardApp::InitSharedMemory()
{
	char* memory = _userDataBlock.Open("KNIGHT_DB");
	if (memory == nullptr)
		return false;

	spdlog::info("AujardApp::InitSharedMemory: shared memory loaded successfully");

	_dbAgent.UserData.reserve(MAX_USER);

	for (int i = 0; i < MAX_USER; i++)
	{
		_USER_DATA* pUser = reinterpret_cast<_USER_DATA*>(
			memory + static_cast<ptrdiff_t>(i * ALLOCATED_USER_DATA_BLOCK));
		_dbAgent.UserData.push_back(pUser);
	}

	return true;
}

bool AujardApp::LoadItemTable()
{
	recordset_loader::STLMap loader(ItemArray);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("AujardApp::LoadItemTable: failed: {}", loader.GetError().Message);
		return false;
	}

	return true;
}

void AujardApp::SelectCharacter(const char* buffer)
{
	int index = 0, userId = -1, sendIndex = 0, idLen1 = 0, idLen2 = 0, tempUserId = -1,
		packetIndex = 0;
	uint8_t init    = 0x01;
	char sendBuffer[256] {}, accountId[MAX_ID_SIZE + 1] {}, charId[MAX_ID_SIZE + 1] {};

	_USER_DATA* user = nullptr;

	userId           = GetShort(buffer, index);
	idLen1           = GetShort(buffer, index);
	GetString(accountId, buffer, idLen1, index);
	idLen2 = GetShort(buffer, index);
	GetString(charId, buffer, idLen2, index);
	init        = GetByte(buffer, index);
	packetIndex = GetDWORD(buffer, index);

	spdlog::debug("AujardApp::SelectCharacter: acctId={}, charId={}, index={}", accountId, charId,
		packetIndex);

	_recvPacketCount++; // packet count

	if (userId < 0 || userId >= MAX_USER || strlen(accountId) == 0 || strlen(charId) == 0)
	{
		SendSelectCharacterFailed(userId);
		return;
	}

	if (GetUserPtr(charId, tempUserId) != nullptr)
	{
		SetShort(sendBuffer, tempUserId, sendIndex);
		SetShort(sendBuffer, idLen1, sendIndex);
		SetString(sendBuffer, accountId, idLen1, sendIndex);
		SetShort(sendBuffer, idLen2, sendIndex);
		SetString(sendBuffer, charId, idLen2, sendIndex);
		UserLogOut(sendBuffer);
		return;
	}

	if (!_dbAgent.LoadUserData(accountId, charId, userId)
		|| !_dbAgent.LoadWarehouseData(accountId, userId))
	{
		SendSelectCharacterFailed(userId);
		return;
	}

	user = _dbAgent.UserData[userId];
	if (user == nullptr)
	{
		SendSelectCharacterFailed(userId);
		return;
	}

	if (strcpy_safe(user->m_Accountid, accountId) != 0)
	{
		spdlog::error("AujardApp::SelectCharacter: accountId too long (len: {}, val: {})",
			std::strlen(accountId), accountId);
		// it didn't fail here before and we don't currently return anything upstream
		// if this exposes any problems we'll have to decide how to handle it then
	}

	SetByte(sendBuffer, WIZ_SEL_CHAR, sendIndex);
	SetShort(sendBuffer, userId, sendIndex);
	SetByte(sendBuffer, 0x01, sendIndex);
	SetByte(sendBuffer, init, sendIndex);

	_packetCount++;

	if (LoggerSendQueue.PutData(sendBuffer, sendIndex) != SMQ_OK)
	{
		spdlog::error("AujardApp::SelectCharacter: Packet Drop: WIZ_SEL_CHAR");
		return;
	}

	++_sendPacketCount;
}

void AujardApp::SendSelectCharacterFailed(int sessionId)
{
	char sendBuffer[8] {};
	int sendIndex = 0;
	SetByte(sendBuffer, WIZ_SEL_CHAR, sendIndex);
	SetShort(sendBuffer, sessionId, sendIndex);
	SetByte(sendBuffer, 0, sendIndex);
	LoggerSendQueue.PutData(sendBuffer, sendIndex);
}

void AujardApp::UserLogOut(const char* buffer)
{
	int index = 0, userId = -1, accountIdLen = 0, charIdLen = 0, sendIndex = 0;
	char charId[MAX_ID_SIZE + 1] {}, accountId[MAX_ID_SIZE + 1] {}, sendBuffer[256] {};

	userId       = GetShort(buffer, index);
	accountIdLen = GetShort(buffer, index);
	GetString(accountId, buffer, accountIdLen, index);
	charIdLen = GetShort(buffer, index);
	GetString(charId, buffer, charIdLen, index);

	if (userId < 0 || userId >= MAX_USER)
		return;

	if (strlen(charId) == 0)
		return;

	HandleUserLogout(userId, UPDATE_LOGOUT);

	SetByte(sendBuffer, WIZ_LOGOUT, sendIndex);
	SetShort(sendBuffer, userId, sendIndex);

	if (LoggerSendQueue.PutData(sendBuffer, sendIndex) != SMQ_OK)
		spdlog::error("AujardApp::UserLogOut: Packet Drop: WIZ_LOGOUT");
}

bool AujardApp::HandleUserLogout(int userId, uint8_t saveType, bool forceLogout)
{
	bool logoutResult = false;

	// make sure UserData[userId] value is valid
	_USER_DATA* pUser = _dbAgent.UserData[userId];
	if (pUser == nullptr || std::strlen(pUser->m_id) == 0)
	{
		spdlog::error(
			"AujardApp::HandleUserLogout: Invalid logout: UserData[{}] is not in use", userId);
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
	bool success         = userdataSuccess && logoutResult;
	if (!success)
	{
		spdlog::error(
			"AujardApp::HandleUserLogout: Invalid Logout: {}, {} (UserData: {}, Logout: {})",
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

bool AujardApp::HandleUserUpdate(int userId, const _USER_DATA& user, uint8_t saveType)
{
	auto sleepTime            = 10ms;
	int updateWarehouseResult = 0, updateUserResult = 0, retryCount = 0, maxRetry = 10;

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
			spdlog::error("AujardApp::HandleUserUpdate: UserData Save Error: [accountId={} "
						  "charId={} (W:{},U:{})]",
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
			updateWarehouseResult = _dbAgent.UpdateWarehouseData(
				user.m_Accountid, userId, saveType);
		}
	}

	// Verify saved data/timestamp
	updateWarehouseResult = _dbAgent.CheckUserData(
		user.m_Accountid, user.m_id, 1, user.m_dwTime, user.m_iBank);
	updateUserResult = _dbAgent.CheckUserData(
		user.m_Accountid, user.m_id, 2, user.m_dwTime, user.m_iExp);
	for (retryCount = 0; !updateWarehouseResult || !updateUserResult; retryCount++)
	{
		if (retryCount >= maxRetry)
		{
			if (!updateWarehouseResult)
			{
				spdlog::error("AujardApp::HandleUserUpdate: Warehouse Save Check Error "
							  "[accountId={} charId={} (W:{})]",
					user.m_Accountid, user.m_id, updateWarehouseResult);
			}

			if (!updateUserResult)
			{
				spdlog::error("AujardApp::HandleUserUpdate: UserData Save Check Error: "
							  "[accountId={} charId={} (U:{})]",
					user.m_Accountid, user.m_id, updateUserResult);
			}
			break;
		}

		if (!updateWarehouseResult)
		{
			_dbAgent.UpdateWarehouseData(user.m_Accountid, userId, saveType);
			updateWarehouseResult = _dbAgent.CheckUserData(
				user.m_Accountid, user.m_id, 1, user.m_dwTime, user.m_iBank);
		}

		std::this_thread::sleep_for(sleepTime);

		if (!updateUserResult)
		{
			_dbAgent.UpdateUser(user.m_id, userId, saveType);
			updateUserResult = _dbAgent.CheckUserData(
				user.m_Accountid, user.m_id, 2, user.m_dwTime, user.m_iExp);
		}
	}

	return static_cast<bool>(updateUserResult) && static_cast<bool>(updateWarehouseResult);
}

void AujardApp::AccountLogIn(const char* buffer)
{
	int index = 0, userId = -1, accountIdLen = 0, passwordLen = 0, sendIndex = 0, nation = -1;
	char accountId[MAX_ID_SIZE + 1] {}, password[MAX_PW_SIZE + 1] {}, sendBuffer[256] {};

	userId       = GetShort(buffer, index);
	accountIdLen = GetShort(buffer, index);
	GetString(accountId, buffer, accountIdLen, index);
	passwordLen = GetShort(buffer, index);
	GetString(password, buffer, passwordLen, index);

	nation = _dbAgent.AccountLogInReq(accountId, password);

	SetByte(sendBuffer, WIZ_LOGIN, sendIndex);
	SetShort(sendBuffer, userId, sendIndex);
	SetByte(sendBuffer, nation, sendIndex);

	if (LoggerSendQueue.PutData(sendBuffer, sendIndex) != SMQ_OK)
		spdlog::error("AujardApp::AccountLogIn: Packet Drop: WIZ_LOGIN");
}

void AujardApp::SelectNation(const char* buffer)
{
	int index = 0, userId = -1, accountIdLen = 0, sendIndex = 0, nation = -1;
	char accountId[MAX_ID_SIZE + 1] {}, sendBuffer[256] {};

	userId       = GetShort(buffer, index);
	accountIdLen = GetShort(buffer, index);
	GetString(accountId, buffer, accountIdLen, index);
	nation      = GetByte(buffer, index);

	bool result = _dbAgent.NationSelect(accountId, nation);

	SetByte(sendBuffer, WIZ_SEL_NATION, sendIndex);
	SetShort(sendBuffer, userId, sendIndex);

	if (result)
		SetByte(sendBuffer, nation, sendIndex);
	else
		SetByte(sendBuffer, 0x00, sendIndex);

	if (LoggerSendQueue.PutData(sendBuffer, sendIndex) != SMQ_OK)
		spdlog::error("AujardApp::SelectNation: Packet Drop: WIZ_SEL_NATION");
}

void AujardApp::CreateNewChar(const char* buffer)
{
	int index = 0, userId = -1, accountIdLen = 0, charIdLen = 0, sendIndex = 0, result = 0,
		charIndex = 0, race = 0, Class = 0, hair = 0, face = 0, str = 0, sta = 0, dex = 0,
		intel = 0, cha = 0;
	char accountId[MAX_ID_SIZE + 1] {}, charId[MAX_ID_SIZE + 1] {}, sendBuffer[256] {};

	userId       = GetShort(buffer, index);
	accountIdLen = GetShort(buffer, index);
	GetString(accountId, buffer, accountIdLen, index);
	charIndex = GetByte(buffer, index);
	charIdLen = GetShort(buffer, index);
	GetString(charId, buffer, charIdLen, index);
	race   = GetByte(buffer, index);
	Class  = GetShort(buffer, index);
	face   = GetByte(buffer, index);
	hair   = GetByte(buffer, index);
	str    = GetByte(buffer, index);
	sta    = GetByte(buffer, index);
	dex    = GetByte(buffer, index);
	intel  = GetByte(buffer, index);
	cha    = GetByte(buffer, index);

	result = _dbAgent.CreateNewChar(
		accountId, charIndex, charId, race, Class, hair, face, str, sta, dex, intel, cha);

	SetByte(sendBuffer, WIZ_NEW_CHAR, sendIndex);
	SetShort(sendBuffer, userId, sendIndex);
	SetByte(sendBuffer, result, sendIndex);

	if (LoggerSendQueue.PutData(sendBuffer, sendIndex) != SMQ_OK)
		spdlog::error("AujardApp::CreateNewChar: Packet Drop: WIZ_NEW_CHAR");
}

void AujardApp::DeleteChar(const char* buffer)
{
	int index = 0, userId = -1, accountIdLen = 0, charIdLen = 0, sendIndex = 0, result = 0,
		charIndex = 0, socNoLen = 0;
	char accountId[MAX_ID_SIZE + 1] {}, charId[MAX_ID_SIZE + 1] {}, socNo[15] {},
		sendBuffer[256] {};

	userId       = GetShort(buffer, index);
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

	SetByte(sendBuffer, WIZ_DEL_CHAR, sendIndex);
	SetShort(sendBuffer, userId, sendIndex);
	SetByte(sendBuffer, result, sendIndex);
	if (result > 0)
		SetByte(sendBuffer, charIndex, sendIndex);
	else
		SetByte(sendBuffer, 0xFF, sendIndex);

	if (LoggerSendQueue.PutData(sendBuffer, sendIndex) != SMQ_OK)
		spdlog::error("AujardApp::DeleteChar: Packet Drop: WIZ_DEL_CHAR");
}

void AujardApp::AllCharInfoReq(const char* buffer)
{
	int index = 0, userId = 0, accountIdLen = 0, sendIndex = 0, charBuffIndex = 0;
	char accountId[MAX_ID_SIZE + 1] {}, sendBuffer[1024] {}, charBuff[1024] {};
	char charId1[MAX_ID_SIZE + 1] {}, charId2[MAX_ID_SIZE + 1] {}, charId3[MAX_ID_SIZE + 1] {};

	userId       = GetShort(buffer, index);
	accountIdLen = GetShort(buffer, index);
	GetString(accountId, buffer, accountIdLen, index);

	SetByte(charBuff, 0x01, charBuffIndex); // result

	_dbAgent.GetAllCharID(accountId, charId1, charId2, charId3);
	_dbAgent.LoadCharInfo(charId1, charBuff, charBuffIndex);
	_dbAgent.LoadCharInfo(charId2, charBuff, charBuffIndex);
	_dbAgent.LoadCharInfo(charId3, charBuff, charBuffIndex);

	SetByte(sendBuffer, WIZ_ALLCHAR_INFO_REQ, sendIndex);
	SetShort(sendBuffer, userId, sendIndex);
	SetShort(sendBuffer, charBuffIndex, sendIndex);
	SetString(sendBuffer, charBuff, charBuffIndex, sendIndex);

	if (LoggerSendQueue.PutData(sendBuffer, sendIndex) != SMQ_OK)
		spdlog::error("AujardApp::AllCharInfoReq: Packet Drop: WIZ_ALLCHAR_INFO_REQ");
}

void AujardApp::AllSaveRoutine()
{
	// TODO:  100ms seems excessive
	auto sleepTime     = 100ms;
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
		spdlog::info(
			"AujardApp::AllSaveRoutine: Ebenezer disconnect: all users saved successfully");
	else
		spdlog::error("AujardApp::AllSaveRoutine: Ebenezer disconnect: not all users saved");
}

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

	spdlog::trace("AujardApp::ConCurrentUserCount: [serverId={} zoneId={} userCount={}]", _serverId,
		_zoneId, usercount);

	_dbAgent.UpdateConCurrentUserCount(_serverId, _zoneId, usercount);
}

void AujardApp::UserDataSave(const char* buffer)
{
	int index = 0, userId = -1, accountIdLen = 0, charIdLen = 0;
	char accountId[MAX_ID_SIZE + 1] {}, charId[MAX_ID_SIZE + 1] {};

	userId       = GetShort(buffer, index);
	accountIdLen = GetShort(buffer, index);
	GetString(accountId, buffer, accountIdLen, index);
	charIdLen = GetShort(buffer, index);
	GetString(charId, buffer, charIdLen, index);

	if (userId < 0 || userId >= MAX_USER || strlen(accountId) == 0 || strlen(charId) == 0)
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

void AujardApp::KnightsPacket(const char* buffer)
{
	int index = 0, nation = 0;
	uint8_t command = GetByte(buffer, index);
	switch (command)
	{
		case DB_KNIGHTS_CREATE:
			CreateKnights(buffer + index);
			break;

		case DB_KNIGHTS_JOIN:
			JoinKnights(buffer + index);
			break;

		case DB_KNIGHTS_WITHDRAW:
			WithdrawKnights(buffer + index);
			break;

		case DB_KNIGHTS_REMOVE:
		case DB_KNIGHTS_ADMIT:
		case DB_KNIGHTS_REJECT:
		case DB_KNIGHTS_CHIEF:
		case DB_KNIGHTS_VICECHIEF:
		case DB_KNIGHTS_OFFICER:
		case DB_KNIGHTS_PUNISH:
			ModifyKnightsMember(buffer + index, command);
			break;

		case DB_KNIGHTS_DESTROY:
			DestroyKnights(buffer + index);
			break;

		case DB_KNIGHTS_MEMBER_REQ:
			AllKnightsMember(buffer + index);
			break;

		case DB_KNIGHTS_STASH:
			break;

		case DB_KNIGHTS_LIST_REQ:
			KnightsList(buffer + index);
			break;

		case DB_KNIGHTS_ALLLIST_REQ:
			nation = GetByte(buffer, index);
			_dbAgent.LoadKnightsAllList(nation);
			break;

		default:
			spdlog::error(
				"AujardApp::KnightsPacket: Invalid WIZ_KNIGHTS_PROCESS command code received: {:X}",
				command);
	}
}

void AujardApp::CreateKnights(const char* buffer)
{
	int index = 0, sendIndex = 0, nameLen = 0, chiefNameLen = 0, knightsId = 0, userId = -1,
		nation = 0, result = 0, community = 0;
	char knightsName[MAX_ID_SIZE + 1] {}, chiefName[MAX_ID_SIZE + 1] {}, sendBuffer[256] {};

	userId    = GetShort(buffer, index);
	community = GetByte(buffer, index);
	knightsId = GetShort(buffer, index);
	nation    = GetByte(buffer, index);
	nameLen   = GetShort(buffer, index);
	GetString(knightsName, buffer, nameLen, index);
	chiefNameLen = GetShort(buffer, index);
	GetString(chiefName, buffer, chiefNameLen, index);

	if (userId < 0 || userId >= MAX_USER)
		return;

	result = _dbAgent.CreateKnights(knightsId, nation, knightsName, chiefName, community);

	spdlog::trace(
		"AujardApp::CreateKnights: userId={}, knightsId={}, result={}", userId, knightsId, result);

	SetByte(sendBuffer, DB_KNIGHTS_CREATE, sendIndex);
	SetShort(sendBuffer, userId, sendIndex);
	SetByte(sendBuffer, result, sendIndex);
	SetByte(sendBuffer, community, sendIndex);
	SetShort(sendBuffer, knightsId, sendIndex);
	SetByte(sendBuffer, nation, sendIndex);
	SetShort(sendBuffer, nameLen, sendIndex);
	SetString(sendBuffer, knightsName, nameLen, sendIndex);
	SetShort(sendBuffer, chiefNameLen, sendIndex);
	SetString(sendBuffer, chiefName, chiefNameLen, sendIndex);

	if (LoggerSendQueue.PutData(sendBuffer, sendIndex) != SMQ_OK)
		spdlog::error("AujardApp::CreateKnights: Packet Drop: DB_KNIGHTS_CREATE");
}

void AujardApp::JoinKnights(const char* buffer)
{
	int index = 0, sendIndex = 0, knightsId = 0, userId = -1;
	uint8_t result = 0;
	char sendBuffer[256] {};

	userId    = GetShort(buffer, index);
	knightsId = GetShort(buffer, index);

	if (userId < 0 || userId >= MAX_USER)
		return;

	_USER_DATA* pUser = _dbAgent.UserData[userId];
	if (pUser == nullptr)
		return;

	result = _dbAgent.UpdateKnights(DB_KNIGHTS_JOIN, pUser->m_id, knightsId, 0);

	spdlog::trace("AujardApp::JoinKnights: userId={}, charId={}, knightsId={}, result={}", userId,
		pUser->m_id, knightsId, result);

	SetByte(sendBuffer, DB_KNIGHTS_JOIN, sendIndex);
	SetShort(sendBuffer, userId, sendIndex);
	SetByte(sendBuffer, result, sendIndex);
	SetShort(sendBuffer, knightsId, sendIndex);

	if (LoggerSendQueue.PutData(sendBuffer, sendIndex) != SMQ_OK)
		spdlog::error("AujardApp::JoinKnights: Packet Drop: DB_KNIGHTS_JOIN");
}

void AujardApp::WithdrawKnights(const char* buffer)
{
	int index = 0, sendIndex = 0, knightsId = 0, userId = -1;
	uint8_t result = 0;
	char sendBuffer[256] {};

	userId    = GetShort(buffer, index);
	knightsId = GetShort(buffer, index);

	if (userId < 0 || userId >= MAX_USER)
		return;

	_USER_DATA* pUser = _dbAgent.UserData[userId];
	if (pUser == nullptr)
		return;

	result = _dbAgent.UpdateKnights(DB_KNIGHTS_WITHDRAW, pUser->m_id, knightsId, 0);
	spdlog::trace("AujardApp::WithdrawKnights: userId={}, knightsId={}, result={}", userId,
		knightsId, result);

	SetByte(sendBuffer, DB_KNIGHTS_WITHDRAW, sendIndex);
	SetShort(sendBuffer, userId, sendIndex);
	SetByte(sendBuffer, result, sendIndex);
	SetShort(sendBuffer, knightsId, sendIndex);

	if (LoggerSendQueue.PutData(sendBuffer, sendIndex) != SMQ_OK)
		spdlog::error("AujardApp::WithdrawKnights: Packet Drop: DB_KNIGHTS_WITHDRAW");
}

void AujardApp::ModifyKnightsMember(const char* buffer, uint8_t command)
{
	int index = 0, sendIndex = 0, knightsId = 0, userId = -1, charIdLen = 0, removeFlag = 0;
	uint8_t result = 0;
	char sendBuffer[256] {}, charId[MAX_ID_SIZE + 1] {};

	userId    = GetShort(buffer, index);
	knightsId = GetShort(buffer, index);
	charIdLen = GetShort(buffer, index);
	GetString(charId, buffer, charIdLen, index);
	removeFlag = GetByte(buffer, index);

	if (userId < 0 || userId >= MAX_USER)
		return;

	/*	if( remove_flag == 0 && command == DB_KNIGHTS_REMOVE )	{		// 없는 유저 추방시에는 디비에서만 처리한다
		result = m_DBAgent.UpdateKnights( command, userid, knightindex, remove_flag );
		TRACE(_T("ModifyKnights - command=%d, nid=%d, index=%d, result=%d \n"), command, uid, knightindex, result);
		return;
	}	*/

	result = _dbAgent.UpdateKnights(command, charId, knightsId, removeFlag);
	spdlog::trace("AujardApp::ModifyKnights: command={}, userId={}, knightsId={}, result={}",
		command, userId, knightsId, result);

	//SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendBuffer);
	SetByte(sendBuffer, command, sendIndex);
	SetShort(sendBuffer, userId, sendIndex);
	SetByte(sendBuffer, result, sendIndex);
	SetShort(sendBuffer, knightsId, sendIndex);
	SetShort(sendBuffer, charIdLen, sendIndex);
	SetString(sendBuffer, charId, charIdLen, sendIndex);
	SetByte(sendBuffer, removeFlag, sendIndex);

	if (LoggerSendQueue.PutData(sendBuffer, sendIndex) != SMQ_OK)
	{
		std::string cmdStr;
		switch (command)
		{
			case DB_KNIGHTS_REMOVE:
				cmdStr = "DB_KNIGHTS_REMOVE";
				break;
			case DB_KNIGHTS_ADMIT:
				cmdStr = "DB_KNIGHTS_ADMIT";
				break;
			case DB_KNIGHTS_REJECT:
				cmdStr = "DB_KNIGHTS_REJECT";
				break;
			case DB_KNIGHTS_CHIEF:
				cmdStr = "DB_KNIGHTS_CHIEF";
				break;
			case DB_KNIGHTS_VICECHIEF:
				cmdStr = "DB_KNIGHTS_VICECHIEF";
				break;
			case DB_KNIGHTS_OFFICER:
				cmdStr = "DB_KNIGHTS_OFFICER";
				break;
			case DB_KNIGHTS_PUNISH:
				cmdStr = "DB_KNIGHTS_PUNISH";
				break;
			default:
				cmdStr = "ModifyKnightsMember";
		}

		spdlog::error("AujardApp::ModifyKnightsMember: Packet Drop: {}", cmdStr);
	}
}

void AujardApp::DestroyKnights(const char* buffer)
{
	int index = 0, sendIndex = 0, knightsId = 0, userId = -1;
	uint8_t result = 0;
	char sendBuffer[256] {};

	userId    = GetShort(buffer, index);
	knightsId = GetShort(buffer, index);
	if (userId < 0 || userId >= MAX_USER)
		return;

	result = _dbAgent.DeleteKnights(knightsId);
	spdlog::trace(
		"AujardApp::DestroyKnights: userId={}, knightsId={}, result={}", userId, knightsId, result);

	SetByte(sendBuffer, DB_KNIGHTS_DESTROY, sendIndex);
	SetShort(sendBuffer, userId, sendIndex);
	SetByte(sendBuffer, result, sendIndex);
	SetShort(sendBuffer, knightsId, sendIndex);

	if (LoggerSendQueue.PutData(sendBuffer, sendIndex) != SMQ_OK)
		spdlog::error("AujardApp::DestroyKnights: Packet Drop: DB_KNIGHTS_DESTROY");
}

void AujardApp::AllKnightsMember(const char* buffer)
{
	int index = 0, sendIndex = 0, knightsId = 0, userId = -1, dbIndex = 0, count = 0;
	char sendBuffer[2048] {}, dbBuff[2048] {};

	userId    = GetShort(buffer, index);
	knightsId = GetShort(buffer, index);
	//int page = GetShort( pBuf, index );

	if (userId < 0 || userId >= MAX_USER)
		return;

	//if( page < 0 )  return;

	count = _dbAgent.LoadKnightsAllMembers(knightsId, 0, dbBuff, dbIndex);
	//count = m_DBAgent.LoadKnightsAllMembers( knightindex, page*10, temp_buff, buff_index );

	SetByte(sendBuffer, DB_KNIGHTS_MEMBER_REQ, sendIndex);
	SetShort(sendBuffer, userId, sendIndex);
	SetByte(sendBuffer, 0x00, sendIndex);         // Success
	SetShort(sendBuffer, 4 + dbIndex, sendIndex); // total packet size -> int16_t(*3) + buff_index
	//SetShort( send_buff, page, sendIndex );
	SetShort(sendBuffer, count, sendIndex);
	SetString(sendBuffer, dbBuff, dbIndex, sendIndex);

	if (LoggerSendQueue.PutData(sendBuffer, sendIndex) != SMQ_OK)
		spdlog::error("AujardApp::AllKnightsMember: Packet Drop: DB_KNIGHTS_MEMBER_REQ");
}

void AujardApp::KnightsList(const char* buffer)
{
	int index = 0, sendIndex = 0, knightsId = 0, userId = -1, dbIndex = 0;
	char sendBuffer[256] {}, dbBuff[256] {};

	userId    = GetShort(buffer, index);
	knightsId = GetShort(buffer, index);
	if (userId < 0 || userId >= MAX_USER)
		return;

	_dbAgent.LoadKnightsInfo(knightsId, dbBuff, dbIndex);

	SetByte(sendBuffer, DB_KNIGHTS_LIST_REQ, sendIndex);
	SetShort(sendBuffer, userId, sendIndex);
	SetByte(sendBuffer, 0x00, sendIndex);
	SetString(sendBuffer, dbBuff, dbIndex, sendIndex);

	if (LoggerSendQueue.PutData(sendBuffer, sendIndex) != SMQ_OK)
		spdlog::error("AujardApp::KnightsList: Packet Drop: DB_KNIGHTS_LIST_REQ");
}

void AujardApp::SetLogInInfo(const char* buffer)
{
	int index = 0, accountIdLen = 0, charIdLen = 0, serverId = 0, serverIpLen = 0, clientIpLen = 0,
		userId = -1, sendIndex = 0;
	char accountId[MAX_ID_SIZE + 1] {}, serverIp[20] {}, clientIp[20] {};
	char charId[MAX_ID_SIZE + 1] {}, sendBuffer[256] {};

	userId       = GetShort(buffer, index);
	accountIdLen = GetShort(buffer, index);
	GetString(accountId, buffer, accountIdLen, index);
	charIdLen = GetShort(buffer, index);
	GetString(charId, buffer, charIdLen, index);
	serverIpLen = GetShort(buffer, index);
	GetString(serverIp, buffer, serverIpLen, index);
	serverId    = GetShort(buffer, index);
	clientIpLen = GetShort(buffer, index);
	GetString(clientIp, buffer, clientIpLen, index);

	// init: 0x01 to insert, 0x02 to update CURRENTUSER
	uint8_t init = GetByte(buffer, index);

	if (!_dbAgent.SetLogInInfo(accountId, charId, serverIp, serverId, clientIp, init))
	{
		SetByte(sendBuffer, WIZ_LOGIN_INFO, sendIndex);
		SetShort(sendBuffer, userId, sendIndex);
		SetByte(sendBuffer, 0x00, sendIndex); // FAIL

		if (LoggerSendQueue.PutData(sendBuffer, sendIndex) != SMQ_OK)
		{
			spdlog::error("AujardApp::SetLoginInfo: Packet Drop: WIZ_LOGIN_INFO [accountId={}, "
						  "charId={}, init={}]",
				accountId, charId, init);
		}
	}
}

void AujardApp::UserKickOut(const char* buffer)
{
	int index = 0, accountIdLen = 0;
	char accountId[MAX_ID_SIZE + 1] {};

	accountIdLen = GetShort(buffer, index);
	GetString(accountId, buffer, accountIdLen, index);

	_dbAgent.AccountLogout(accountId);
}

void AujardApp::WritePacketLog()
{
	spdlog::info("AujardApp::WritePacketLog: Packet Count: recv={}, send={}, realsend={}",
		_recvPacketCount, _packetCount, _sendPacketCount);
}

void AujardApp::BattleEventResult(const char* data)
{
	int _type = 0, result = 0, charIdLen = 0, index = 0;
	char charId[MAX_ID_SIZE + 1] {};

	_type     = GetByte(data, index);
	result    = GetByte(data, index);
	charIdLen = GetByte(data, index);
	if (charIdLen > 0 && charIdLen < MAX_ID_SIZE + 1)
	{
		GetString(charId, data, charIdLen, index);
		spdlog::info("AujardApp::BattleEventResult : The user who killed the enemy commander is "
					 "{}, _type={}, nation={}",
			charId, _type, result);
		_dbAgent.UpdateBattleEvent(charId, result);
	}
}

void AujardApp::CouponEvent(const char* data)
{
	int index = 0, sendIndex = 0;
	char strAccountName[MAX_ID_SIZE + 1] {}, strCharName[MAX_ID_SIZE + 1] {};
	char strCouponID[MAX_ID_SIZE + 1] {}, sendBuffer[256] {};

	int nType = GetByte(data, index);
	if (nType == CHECK_COUPON_EVENT)
	{
		int nSid = GetShort(data, index);
		int nLen = GetShort(data, index);
		GetString(strAccountName, data, nLen, index);
		int nEventNum   = GetDWORD(data, index);
		// 비러머글 대사문 >.<
		int nMessageNum = GetDWORD(data, index);
		// TODO: Not implemented. Allow nResult to default to 0
		// int nResult = _dbAgent.CheckCouponEvent(strAccountName);
		int nResult     = 0;

		SetByte(sendBuffer, DB_COUPON_EVENT, sendIndex);
		SetShort(sendBuffer, nSid, sendIndex);
		SetByte(sendBuffer, nResult, sendIndex);
		SetDWORD(sendBuffer, nEventNum, sendIndex);
		// 비러머글 대사문 >.<
		SetDWORD(sendBuffer, nMessageNum, sendIndex);
		//

		if (LoggerSendQueue.PutData(sendBuffer, sendIndex) != SMQ_OK)
			spdlog::error("AujardApp::CouponEvent: Packet Drop: DB_COUPON_EVENT");
	}
	else if (nType == UPDATE_COUPON_EVENT)
	{
		/*int nSid =*/GetShort(data, index);
		int nLen = GetShort(data, index);
		GetString(strAccountName, data, nLen, index);
		int nCharLen = GetShort(data, index);
		GetString(strCharName, data, nCharLen, index);
		int nCouponLen = GetShort(data, index);
		GetString(strCouponID, data, nCouponLen, index);
		/*int nItemID =*/GetDWORD(data, index);
		/*int nItemCount =*/GetDWORD(data, index);

		// TODO: not implemented.  Allow nResult to default to 0
		// int nResult = _dbAgent.UpdateCouponEvent(strAccountName, strCharName, strCouponID, nItemID, nItemCount);
	}
}

void AujardApp::CheckHeartbeat()
{
	if (_heartbeatReceivedTime == 0)
		return;

	time_t currentTime               = time(nullptr);
	time_t secondsSinceLastHeartbeat = currentTime - _heartbeatReceivedTime;

	if (secondsSinceLastHeartbeat > SECONDS_SINCE_LAST_HEARTBEAT_TO_SAVE.count())
		AllSaveRoutine();
}

void AujardApp::HeartbeatReceived()
{
	_heartbeatReceivedTime = time(nullptr);
}

} // namespace Aujard

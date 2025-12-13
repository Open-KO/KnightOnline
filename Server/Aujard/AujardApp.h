#pragma once

#include "DBAgent.h"
#include "Define.h"

#include <shared-server/AppThread.h>
#include <shared-server/logger.h>
#include <shared-server/SharedMemoryBlock.h>
#include <shared-server/SharedMemoryQueue.h>
#include <shared-server/STLMap.h>

using ItemtableArray = CSTLMap<model::Item>;

class ReadQueueThread;
class TimerThread;
class AujardApp : public AppThread
{
public:
	static AujardApp* instance()
	{
		return static_cast<AujardApp*>(s_instance);
	}

	AujardApp(logger::Logger& logger);
	~AujardApp();

	/// \brief handles DB_HEARTBEAT requests
	/// \see DB_HEARTBEAT
	void HeartbeatReceived();

	/// \brief checks the time since the last received heartbeat and saves if
	//  enough time has passed.
	void CheckHeartbeat();

	/// \brief handles DB_COUPON_EVENT requests
	/// \todo related stored procedures are not implemented
	/// \see DB_COUPON_EVENT
	void CouponEvent(const char* data);
	
	/// \brief handles WIZ_BATTLE_EVENT requests
    /// \details contains which nation won the war and which charId killed the commander
    /// \see WIZ_BATTLE_EVENT
	void BattleEventResult(const char* data);

	/// \brief writes a packet summary line to the log file
	void WritePacketLog();

	/// \brief handles WIZ_KICKOUT requests
	/// \see WIZ_KICKOUT
	void UserKickOut(const char* buffer);

	/// \brief handles WIZ_LOGIN_INFO requests, updating CURRENTUSER for a user
	/// \see WIZ_LOGIN_INFO
	void SetLogInInfo(const char* buffer);

	/// \brief attempts to retrieve metadata for a knights clan
	/// \see KnightsPacket(), KNIGHTS_LIST_REQ
	void KnightsList(const char* buffer);

	/// \brief Called every 5min by _concurrentCheckThread
	/// \see _concurrentCheckThread
	void ConCurrentUserCount();

	/// \brief attempts to return a list of all knights members
	/// \see KnightsPacket(), KNIGHTS_MEMBER_REQ
	void AllKnightsMember(const char* buffer);

	/// \brief attempts to disband a knights clan
	/// \see KnightsPacket(), KNIGHTS_DESTROY
	void DestroyKnights(const char* buffer);

	/// \brief attempts to modify a knights character
	/// \see KnightsPacket(), KNIGHTS_REMOVE, KNIGHTS_ADMIT, KNIGHTS_REJECT, KNIGHTS_CHIEF,
	/// KNIGHTS_VICECHIEF, KNIGHTS_OFFICER, KNIGHTS_PUNISH
	void ModifyKnightsMember(const char* buffer, uint8_t command);

	/// \brief attempt to remove a character from a knights clan
	/// \see KnightsPacket(), KNIGHTS_WITHDRAW
	void WithdrawKnights(const char* buffer);

	/// \brief attempts to add a character to a knights clan
	/// \see KnightsPacket(), KNIGHTS_JOIN
	void JoinKnights(const char* buffer);

	/// \brief attempts to create a knights clan
	/// \see KnightsPacket(), KNIGHTS_CREATE
	void CreateKnights(const char* buffer);

	/// \brief handles WIZ_KNIGHTS_PROCESS and WIZ_CLAN_PROCESS requests
	/// \detail calls the appropriate method for the subprocess op-code
	/// \see "Knights Packet sub define" section in Define.h
	void KnightsPacket(const char* buffer);

	/// \brief attempts to find a UserData record for charId
	/// \param charId
	/// \param[out] userId UserData index of the user, if found
	/// \return pointer to UserData[userId] object if found, nullptr otherwise
	_USER_DATA* GetUserPtr(const char* charId, int& userId);
	
	/// \brief handles a WIZ_ALLCHAR_INFO_REQ request
	/// \details Loads all character information and sends it to the client
	/// \see WIZ_ALLCHAR_INFO_REQ
	void AllCharInfoReq(const char* buffer);

	/// \brief handles a WIZ_LOGIN request to a selected game server
	/// \see WIZ_LOGIN
	void AccountLogIn(const char* buffer);

	/// \brief handles a WIZ_DEL_CHAR request
	/// \todo not implemented, always returns an error to the client
	/// \see WIZ_DEL_CHAR
	void DeleteChar(const char* buffer);

	/// \brief handles a WIZ_NEW_CHAR request
	/// \see WIZ_NEW_CHAR
	void CreateNewChar(const char* buffer);

	/// \brief handles a WIZ_SEL_NATION request to a selected game server
	/// \see WIZ_SEL_NATION
	void SelectNation(const char* buffer);

	/// \brief loads information needed from the ITEM table to a cache map
	bool LoadItemTable();
	
	/// \brief handles a WIZ_DATASAVE request
	/// \see WIZ_DATASAVE
	/// \see HandleUserUpdate()
	void UserDataSave(const char* buffer);
	
	/// \brief Handles a WIZ_LOGOUT request when logging out of the game
	/// \details Updates USERDATA and WAREHOUSE as part of logging out, then resets the UserData entry for re-use
	/// \see WIZ_LOGOUT, HandleUserLogout()
	void UserLogOut(const char* buffer);
	
	/// \brief handling for when OnTimer fails a PROCESS_CHECK with ebenezer
	/// \details Logs ebenezer outage, attempts to save all UserData, and resets all UserData[userId] objects
	/// \see OnTimer(), HandleUserLogout()
	void AllSaveRoutine();

	/// \brief initializes shared memory with other server applications
	bool InitSharedMemory();

	/// \brief loads and sends data after a character is selected
	void SelectCharacter(const char* buffer);

	SharedMemoryQueue	LoggerSendQueue;
	SharedMemoryQueue	LoggerRecvQueue;

	ItemtableArray		ItemArray;

protected:
	CDBAgent			_dbAgent;
	SharedMemoryBlock	_userDataBlock;

	int					_serverId;
	int					_zoneId;

	int					_packetCount;		// packet의 수를 체크
	int					_sendPacketCount;	// packet의 수를 체크
	int					_recvPacketCount;	// packet의 수를 체크

protected:
	/// \brief handles user logout functions
	/// \param userId user index for UserData
	/// \param saveType one of: UPDATE_LOGOUT, UPDATE_ALL_SAVE
	/// \param forceLogout should be set to true in panic situations
	/// \see UserLogOut(), AllSaveRoutine(), HandleUserUpdate()
	bool HandleUserLogout(int userId, uint8_t saveType, bool forceLogout = false);

	/// \brief handles user update functions and retry logic
	/// \param userId user index for UserData
	/// \param user reference to user object
	/// \param saveType one of: UPDATE_LOGOUT, UPDATE_ALL_SAVE, UPDATE_PACKET_SAVE
	/// \see UserDataSave(), HandleUserLogout()
	bool HandleUserUpdate(int userId, const _USER_DATA& user, uint8_t saveType);

	/// \returns The application's ini config path.
	std::filesystem::path ConfigPath() const override;

	/// \brief Loads application-specific config from the loaded application ini file (`iniFile`).
	/// \param iniFile The loaded application ini file.
	/// \returns true when successful, false otherwise
	bool LoadConfig(CIni& iniFile) override;

	/// \brief Initializes the server, loading caches, attempting to load shared memory etc.
	/// \returns true when successful, false otherwise
	bool OnStart() override;

	/// \brief Attempts to open all shared memory queues & blocks that we've yet to open.
	bool AttemptOpenSharedMemory();

	/// \brief Thread tick attempting to open all shared memory queues & blocks that we've yet to open.
	/// \see AttemptOpenSharedMemory
	void AttemptOpenSharedMemoryThreadTick();

	/// \brief Finishes server initialization and starts processing threads.
	void OnSharedMemoryOpened();

private:
	time_t								_heartbeatReceivedTime;
	std::unique_ptr<TimerThread>		_dbPoolCheckThread;
	std::unique_ptr<TimerThread>		_heartbeatCheckThread;
	std::unique_ptr<TimerThread>		_concurrentCheckThread;
	std::unique_ptr<TimerThread>		_packetCheckThread;
	std::unique_ptr<TimerThread>		_smqOpenThread;

	std::unique_ptr<ReadQueueThread>	_readQueueThread;
};

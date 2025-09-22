// _dbAgent.h: interface for the CDBAgent class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DBAGENT_H__C7580B9A_10FF_46DE_93C2_B0C9BA9D0422__INCLUDED_)
#define AFX_DBAGENT_H__C7580B9A_10FF_46DE_93C2_B0C9BA9D0422__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <memory>

#include "Define.h"
#include <vector>

#include <db-library/fwd.h>

typedef std::vector <_USER_DATA*>			UserDataArray;

class CAujardDlg;
class CDBAgent
{
public:
	
	/// \brief updates which nation won the war and which charId killed the commander
	/// \returns true if update successful, otherwise false
	bool UpdateBattleEvent(const char* charId, int nation);

	/// \brief loads knights ranking data by nation to handle KNIGHTS_ALLLIST_REQ
	/// \param nation 1 = karus, 2 = elmorad, 3 = battlezone
	/// \see KNIGHTS_ALLLIST_REQ
	/// \note most other functions in this file are db-ops only.  This does db-ops
	/// and socket IO.  Should likely be separated into separate functions
	void LoadKnightsAllList(int nation);

	/// \brief verifies that USERDATA or WAREHOUSE are in sync with the provided input
	/// \param accountId
	/// \param charId
	/// \param checkType 0 for USERDATA, 1 for WAREHOUSE
	/// \param compareData is WAREHOUSE.nMoney when checkType is 1, otherwise USERDATA.Exp
	/// \returns true when input data matches database record, false otherwise
	bool CheckUserData(const char* accountId, const char* charId, int checkType, int userUpdateTime, int compareData);

	/// \brief removes the CURRENTUSER record for accountId
	/// \returns true on success, otherwise false
	bool AccountLogout(const char* accountId, int logoutCode = 0);

	/// \brief sets CURRENTUSER record for a given accountId
	/// \param accountId
	/// \param charId
	/// \param serverIp
	/// \param serverId
	/// \param clientIp
	/// \param init 0x01 to insert, 0x02 to update CURRENTUSER
	/// \returns true when CURRENTUSER successfully updated, otherwise false
	bool SetLogInInfo(const char* accountId, const char* charId, const char* serverIp, int serverId, const char* clientIp, BYTE init);

	/// \brief loads knights clan metadata into the supplied buffer
	/// \param knightsId
	/// \param[out] buffOut buffer to write knights data to
	/// \param[out] buffIndex
	/// \returns true if data was successfully loaded to the buffer, otherwise false
	bool LoadKnightsInfo(int knightsId, char* buffOut, int& buffIndex);

	/// \brief attempts to update warehouse data from UserData[userId]
	/// \param accountId
	/// \param userId
	/// \param updateType one of UPDATE_LOGOUT, UPDATE_ALL_SAVE, UPDATE_PACKET_SAVE
	/// \returns true for success, otherwise false
	bool UpdateWarehouseData(const char* accountId, int userId, int updateType);

	/// \brief attempts to load warehouse data for an account into UserData[userId]
	/// \returns true if successful, otherwise false
	bool LoadWarehouseData(const char* accountId, int userId);

	/// \brief updates concurrent zone count
	/// \returns true if successful, false otherwise
	bool UpdateConCurrentUserCount(int serverId, int zoneId, int userCount);

	/// \brief loads knight member information into buffOut
	/// \param knightsId
	/// \param start likely made for pagination, but unused by callers (always 0)
	/// \param[out] buffOut buffer to write member information to
	/// \param[out] buffIndex
	/// \returns rowCount - start
	int LoadKnightsAllMembers(int knightsId, int start, char* buffOut, int& buffIndex);

	/// \brief attemps to delete a knights clan from the database
	/// \return 0 for success, 7 if not found
	int DeleteKnights(int knightsId);
	
	/// \brief attempts to update a knights clan
	/// \returns 0 on success, 2 for charId not found or db error, 7 for not found, 8 for capacity error
	int UpdateKnights(int type, char* charId, int knightsId, int domination);

	/// \brief attempts to create a knights clan
	/// \returns 0 for success, 3 for name already in use, 6 for db error
	int CreateKnights(int knightsId, int nation, char* name, char* chief, int flag = 1);

	/// \brief loads all character names for an account
	/// \param accountId
	/// \param[out] charId1
	/// \param[out] charId2
	/// \param[out] charId3
	/// \returns true if charIds were successfully loaded, false otherwise
	bool GetAllCharID(const char* accountId, char* charId1, char* charId2, char* charId3);

	/// \brief loads character information needed in WIZ_ALLCHAR_INFO_REQ
	/// \param[out] charId will trime whitespace
	/// \param[out] buff buffer to write character info to
	/// \param[out] buffIndex
	/// /// \see AllCharInfoReq(), WIZ_ALLCHAR_INFO_REQ
	bool LoadCharInfo(char* charId, char* buff, int& buffIndex);

	/// \brief ensures that database records are created in ACCOUNT_CHAR
	/// and WAREHOUSE prior to logging into the game
	/// \returns true if records are properly set, false otherwise
	bool NationSelect(char* accountId, int nation);

	/// \brief attempts to create a new character
	/// \returns NEW_CHAR_SUCCESS on success, or one of the following on error:
	/// NEW_CHAR_ERROR, NEW_CHAR_NO_FREE_SLOT, NEW_CHAR_INVALID_RACE, NEW_CHAR_NAME_IN_USE, NEW_CHAR_SYNC_ERROR
	int CreateNewChar(char* accountId, int index, char* charId, int race, int Class, int hair, int face, int str, int sta, int dex, int intel, int cha);

	/// \brief attempts to login a character to the game server
	/// \returns -1 for failure, 0 for unselected nation, 1 for karus, 2 for elmorad
	int	AccountLogInReq(char* accountId, char* password);	// return Nation value

	/// \brief updates the database with the data from UserData[userId]
	/// \param charId
	/// \param userId
	/// \param updateType one of UPDATE_PACKET_SAVE, UPDATE_LOGOUT, UPDATE_ALL_SAVE
	/// \see UPDATE_PACKET_SAVE, UPDATE_LOGOUT, UPDATE_ALL_SAVE
	/// \returns true when database successfully updated, false otherwise
	bool UpdateUser(const char* charId, int userId, int updateType);

	/// \brief populates UserData[userId] from the database
	bool LoadUserData(const char* accountId, const char* charId, int userId);

	// stored procs not implemented:
	//bool UpdateCouponEvent(const char* accountId, char* charId, char* cpid, int itemId, int count);
	//bool CheckCouponEvent(const char* accountId);
	//bool DeleteChar(int index, char* id, char* charId, char* socno);
	
	/// \brief attempts connections with db::ConnectionManager to needed dbTypes
	/// \returns true is successful, false otherwise
	bool InitDatabase();

	/// \brief resets a UserData[userId] record.  Called after logout actions
	/// \see UserData
	void ResetUserData(int userId);
	
	CDBAgent();
	virtual ~CDBAgent();

	UserDataArray	UserData;
	
private:
	/// \brief reference back to the main AujardDlg instance
	CAujardDlg* _main;
};

#endif // !defined(AFX_DBAGENT_H__C7580B9A_10FF_46DE_93C2_B0C9BA9D0422__INCLUDED_)

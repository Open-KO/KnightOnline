#ifndef SERVER_VERSIONMANAGER_DBPROCESS_H
#define SERVER_VERSIONMANAGER_DBPROCESS_H

#pragma once

#include <memory>

#include <db-library/fwd.h>
#include "Define.h"

/// \brief Handles database operations for VersionManager
class CDBProcess
{
public:
	/// \brief calls LoadPremiumServiceUser and writes how many days of premium remain
	/// to premiumDaysRemaining
	/// \param accountId
	/// \param[out] premiumDaysRemaining output value of remaining premium days
	/// \return true on success, false on failure
	bool LoadPremiumServiceUser(const char* accountId, int16_t* premiumDaysRemaining);

	/// \brief Checks to see if a user is present in CURRENTUSER for a particular server
	/// writes to serverIp and serverId
	/// \param accountId
	/// \param[out] serverIp output of the server IP the user is connected to
	/// \param[out] serverId output of the serverId the user is connected to
	/// \return true on success, false on failure
	bool IsCurrentUser(const char* accountId, std::string& serverIp, int& serverId);

	/// \brief Deletes Version table entry tied to the specified key
	/// \return true on success, false on failure
	bool DeleteVersion(int version);

	/// \brief attempts to create a new Version table record
	/// \returns true on success, false on failure
	bool InsertVersion(int version, const char* fileName, const char* compressName, int historyVersion);

	/// \brief attempts a connection with db::ConnectionManager to the ACCOUNT dbType
	/// \throws nanodbc::database_error
	/// \returns true is successful, false otherwise
	bool InitDatabase();

	/// \brief Attempts account authentication with a given accountId and password
	/// \returns AUTH_OK on success, AUTH_NOT_FOUND on failure, AUTH_BANNED for banned accounts
	int AccountLogin(const char* accountId, const char* password);

	/// \brief loads the VERSION table into versionList
	/// \return true on success, false on failure
	bool LoadVersionList(VersionInfoList* versionList);

	/// \brief updates the server's concurrent user counts
	/// \return true on success, false on failure
	bool LoadUserCountList();

	CDBProcess();
	virtual ~CDBProcess();
};

#endif // SERVER_VERSIONMANAGER_DBPROCESS_H

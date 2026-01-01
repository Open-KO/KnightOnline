// DBProcess.cpp: implementation of the CDBProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "DBProcess.h"
#include "Define.h"
#include "VersionManagerApp.h"

#include <db-library/Connection.h>
#include <db-library/Exceptions.h>
#include <db-library/RecordSetLoader_STLMap.h>
#include <db-library/StoredProc.h>
#include <db-library/utils.h>

#include <nanodbc/nanodbc.h>
#include <spdlog/spdlog.h>

#include <VersionManager/binder/VersionManagerBinder.h>
#include <StoredProc/StoredProc.h>

CDBProcess::CDBProcess()
{
}

CDBProcess::~CDBProcess()
{
}

bool CDBProcess::InitDatabase() noexcept(false)
{
	try
	{
		auto conn = db::ConnectionManager::CreatePoolConnection(modelUtil::DbType::ACCOUNT, DB_PROCESS_TIMEOUT);
		if (conn == nullptr)
			return false;
	}
	catch (const nanodbc::database_error& dbErr)
	{
		db::utils::LogDatabaseError(dbErr, "DBProcess::InitDatabase()");
		return false;
	}

	return true;
}

bool CDBProcess::LoadVersionList(VersionInfoList* versionList)
{
	recordset_loader::STLMap loader(*versionList);
	if (!loader.Load_ForbidEmpty())
	{
		spdlog::error("DBProcess::LoadVersionList: failed: {}",
			loader.GetError().Message);
		return false;
	}

	return true;
}

int CDBProcess::AccountLogin(const char* accountId, const char* password)
{
	// TODO: Restore this, but it should be handled ideally in its own database, or a separate stored procedure.
	// As we're currently using a singular database (and we expect people to be using our database), and we have
	// no means of syncing this currently, we'll temporarily hack this to fetch and handle basic auth logic
	// without a procedure.
	db::SqlBuilder<model::TbUser> sql;
	sql.IsWherePK = true;
	
	try
	{
		db::ModelRecordSet<model::TbUser> recordSet;

		auto stmt = recordSet.prepare(sql);
		if (stmt == nullptr)
		{
			throw db::ApplicationError("DBProcess::AccountLogin: statement could not be allocated");
		}

		stmt->bind(0, accountId);
		recordSet.execute();

		if (!recordSet.next())
			return AUTH_NOT_FOUND;

		model::TbUser user = {};
		recordSet.get_ref(user);

		if (user.Password != password)
		{
			// Use AUTH_NOT_FOUND here instead of AUTH_INVALID_PW
			// to ensure attackers have no way of identifying real accounts to bruteforce passwords on.
			return AUTH_NOT_FOUND;
		}

		if (user.Authority == AUTHORITY_BLOCK_USER)
		{
			return AUTH_BANNED;
		}
	}
	catch (const nanodbc::database_error& dbErr)
	{
		db::utils::LogDatabaseError(dbErr, "DBProcess::AccountLogin()");
		return AUTH_FAILED;
	}
	
	return AUTH_OK;
}

bool CDBProcess::InsertVersion(int version, const char* fileName, const char* compressName, int historyVersion)
{
	using ModelType = model::Version;

	db::SqlBuilder<ModelType> sql;
	std::string insert = sql.InsertString();
	try
	{
		auto conn = db::ConnectionManager::CreatePoolConnection(ModelType::DbType(), DB_PROCESS_TIMEOUT);
		if (conn == nullptr)
			return false;

		nanodbc::statement stmt = conn->CreateStatement(insert);
		stmt.bind(0, &version);
		stmt.bind(1, fileName);
		stmt.bind(2, compressName);
		stmt.bind(3, &historyVersion);

		nanodbc::result result = stmt.execute();
		if (result.affected_rows() > 0)
			return true;
	}
	catch (const nanodbc::database_error& dbErr)
	{
		db::utils::LogDatabaseError(dbErr, "DBProcess::InsertVersion()");
		return false;
	}
	
	return false;
}

bool CDBProcess::DeleteVersion(int version)
{
	using ModelType = model::Version;

	db::SqlBuilder<ModelType> sql;
	std::string deleteQuery = sql.DeleteByIdString();
	try
	{
		auto conn = db::ConnectionManager::CreatePoolConnection(ModelType::DbType(), DB_PROCESS_TIMEOUT);
		if (conn == nullptr)
			return false;

		nanodbc::statement stmt = conn->CreateStatement(deleteQuery);
		stmt.bind(0, &version);

		nanodbc::result result = stmt.execute();
		if (result.affected_rows() > 0)
			return true;
	}
	catch (const nanodbc::database_error& dbErr)
	{
		db::utils::LogDatabaseError(dbErr, "DBProcess::DeleteVersion()");
	}
	
	return false;
}

bool CDBProcess::LoadUserCountList()
{
	VersionManagerApp* appInstance = VersionManagerApp::instance();
	try
	{
		db::ModelRecordSet<model::Concurrent> recordSet;
		recordSet.open();

		while (recordSet.next())
		{
			model::Concurrent concurrent = recordSet.get();

			int serverId = concurrent.ServerId - 1;
			if (serverId >= static_cast<int>(appInstance->ServerList.size()))
				continue;

			appInstance->ServerList[serverId]->sUserCount = concurrent.Zone1Count + concurrent.Zone2Count + concurrent.Zone3Count;
		}

		return true;
	}
	catch (const nanodbc::database_error& dbErr)
	{
		db::utils::LogDatabaseError(dbErr, "DBProcess::LoadUserCountList()");
	}
	
	return false;
}

bool CDBProcess::IsCurrentUser(const char* accountId, std::string& serverIp, int& serverId)
{
	db::SqlBuilder<model::CurrentUser> sql;
	sql.IsWherePK = true;
	try
	{
		db::ModelRecordSet<model::CurrentUser> recordSet;

		auto stmt = recordSet.prepare(sql);
		if (stmt == nullptr)
			throw db::ApplicationError("DBProcess::IsCurrentUser: statement could not be allocated");

		stmt->bind(0, accountId);
		recordSet.execute();

		if (!recordSet.next())
			return false;

		model::CurrentUser user = recordSet.get();
		serverId = user.ServerId;
		serverIp = std::move(user.ServerIP);

		return true;
	}
	catch (const nanodbc::database_error& dbErr)
	{
		db::utils::LogDatabaseError(dbErr, "DBProcess::IsCurrentUser()");
	}

	return false;
}

bool CDBProcess::LoadPremiumServiceUser(const char* accountId, int16_t* premiumDaysRemaining)
{
	int32_t premiumType = 0, // NOTE: we don't need this in the login server
		daysRemaining = 0;
	try
	{
		db::StoredProc<storedProc::LoadPremiumServiceUser> premium;
		premium.execute(accountId, &premiumType, &daysRemaining);
	}
	catch (const nanodbc::database_error& dbErr)
	{
		db::utils::LogDatabaseError(dbErr, "DBProcess::LoadPremiumServiceUser()");
		return false;
	}

	*premiumDaysRemaining = static_cast<int16_t>(daysRemaining);
	return true;
}

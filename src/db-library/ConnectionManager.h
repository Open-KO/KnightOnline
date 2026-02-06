#ifndef DBLIBRARY_CONNECTIONMANAGER_H
#define DBLIBRARY_CONNECTIONMANAGER_H

#pragma once

#include <ModelUtil/ModelUtil.h>

#include <cassert>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>

namespace nanodbc
{
class connection;
}

namespace db
{

class Connection;
struct DatasourceConfig;
class PoolConnection;

/// \brief manages connections to the database via nanodbc
class ConnectionManager
{
	friend class PoolConnection;

public:
	static void Create();
	static void Destroy();

	/// \brief Fetches the associated previously stored database config using the code-generated dbType
	static void SetDatasourceConfig(modelUtil::DbType dbType, const std::string_view datasourceName,
		const std::string_view datasourceUserName, const std::string_view datasourcePassword);

	/// \brief Fetches the associated previously stored database config using the code-generated dbType
	static std::shared_ptr<const DatasourceConfig> GetDatasourceConfig(modelUtil::DbType dbType);

	/// \brief Attempts a connection to the database using the code-generated dbType
	/// \throws DatasourceConfigNotFoundException
	/// \throws nanodbc::database_error
	static std::shared_ptr<Connection> CreateConnection(
		modelUtil::DbType dbType, long timeout = -1) noexcept(false);

	/// \brief Fetches an established pool connection using the code-generated dbType, or otherwise attempt to establish a new
	/// pool connection using the code-generated dbType if one is not available.
	/// \throws DatasourceConfigNotFoundException
	/// \throws nanodbc::database_error
	static std::shared_ptr<PoolConnection> CreatePoolConnection(
		modelUtil::DbType dbType, long timeout = -1) noexcept(false);

	/// \brief Returns the current ODBC connection string for a given DbType, or empty string if there is none
	static std::string GetOdbcConnectionString(modelUtil::DbType dbType);

	/// \brief Returns the current ODBC connection string for a given config, or empty string if there is none
	static std::string GetOdbcConnectionString(const DatasourceConfig* config);

	// \brief Disconnects and removes pooled connections that haven't been used in some time.
	// Will always leave at least 1 connection available.
	static void ExpireUnusedPoolConnections();

protected:
	static inline ConnectionManager& GetInstance()
	{
		assert(s_instance != nullptr);
		return *s_instance;
	}

	ConnectionManager();

	/// \brief fetch the associated previously stored database config using the code-generated dbType
	void SetDatasourceConfigImpl(modelUtil::DbType dbType, const std::string_view datasourceName,
		const std::string_view datasourceUserName, const std::string_view datasourcePassword);

	/// \brief fetch the associated previously stored database config using the code-generated dbType
	std::shared_ptr<const DatasourceConfig> GetDatasourceConfigImpl(modelUtil::DbType dbType) const;

	/// \brief attempt a connection to the database using the code-generated dbType
	/// \throws DatasourceConfigNotFoundException
	/// \throws nanodbc::database_error
	std::shared_ptr<Connection> CreateConnectionImpl(
		modelUtil::DbType dbType, long timeout) noexcept(false);

	/// \brief fetch an established pool connection using the code-generated dbType, or otherwise attempt to establish a new
	/// pool connection using the code-generated dbType if one is not available.
	/// \throws DatasourceConfigNotFoundException
	/// \throws nanodbc::database_error
	std::shared_ptr<PoolConnection> CreatePoolConnectionImpl(
		modelUtil::DbType dbType, long timeout) noexcept(false);

	// \brief Disconnects and removes pooled connections that haven't been used in some time.
	// Will always leave at least 1 connection available.
	void ExpireUnusedPoolConnectionsImpl();

	~ConnectionManager();

private:
	/// \brief pulls an established connection using the code-generated dbType from the dbType pool, if available.
	/// \throws DatasourceConfigNotFoundException
	/// \throws nanodbc::database_error
	std::shared_ptr<Connection> PullPoolConnection(modelUtil::DbType dbType) noexcept(false);

	/// \brief restores an established connection using the code-generated dbType to the dbType pool.
	void RestorePoolConnection(
		modelUtil::DbType dbType, const std::shared_ptr<Connection>& conn) noexcept(false);

protected:
	static ConnectionManager* s_instance;

	std::unordered_map<modelUtil::DbType, std::shared_ptr<const DatasourceConfig>> _configMap;
	mutable std::mutex _configLock;

	struct ConnectionInfo
	{
		std::shared_ptr<Connection> Conn;
		time_t LastUsedTime = {};
	};

	std::unordered_multimap<modelUtil::DbType, ConnectionInfo> _connectionPoolMap;
	mutable std::mutex _connectionPoolLock;

public:
	static long DefaultConnectionTimeout;
	static time_t PoolConnectionTimeout;
};

} // namespace db

#endif // DBLIBRARY_CONNECTIONMANAGER_H

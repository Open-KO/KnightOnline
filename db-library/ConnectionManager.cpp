#include "pch.h"
#include "ConnectionManager.h"
#include "Connection.h"
#include "DatasourceConfig.h"
#include "Exceptions.h"
#include "PoolConnection.h"

namespace db
{
	ConnectionManager* ConnectionManager::s_instance = nullptr;
	long ConnectionManager::DefaultConnectionTimeout = 0;
	time_t ConnectionManager::PoolConnectionTimeout = 600;

	void ConnectionManager::Create()
	{
		if (s_instance == nullptr)
			s_instance = new ConnectionManager();
	}

	void ConnectionManager::Destroy()
	{
		delete s_instance;
		s_instance = nullptr;
	}

	ConnectionManager::ConnectionManager()
	{
		assert(s_instance == nullptr);
		s_instance = this;
	}

	/// \brief Fetches the associated previously stored database config using the code-generated dbType
	void ConnectionManager::SetDatasourceConfig(
		modelUtil::DbType dbType,
		const std::string_view datasourceName,
		const std::string_view datasourceUserName,
		const std::string_view datasourcePassword)
	{
		GetInstance().SetDatasourceConfigImpl(
			dbType,
			datasourceName,
			datasourceUserName,
			datasourcePassword);
	}

	/// \brief Fetches the associated previously stored database config using the code-generated dbType
	std::shared_ptr<const DatasourceConfig>
		ConnectionManager::GetDatasourceConfig(modelUtil::DbType dbType)
	{
		return GetInstance().GetDatasourceConfigImpl(dbType);
	}

	/// \brief Attempts a connection to the database using the code-generated dbType
	/// \throws std::runtime_error
	/// \throws nanodbc::database_error
	std::shared_ptr<Connection> ConnectionManager::CreateConnection(modelUtil::DbType dbType, long timeout) noexcept(false)
	{
		return GetInstance().CreateConnectionImpl(dbType, timeout);
	}

	/// \brief Fetches an established pool connection using the code-generated dbType, or otherwise attempt to establish a new
	/// pool connection using the code-generated dbType if one is not available.
	/// \throws DatasourceConfigNotFoundException
	/// \throws nanodbc::database_error
	std::shared_ptr<PoolConnection> ConnectionManager::CreatePoolConnection(modelUtil::DbType dbType, long timeout) noexcept(false)
	{
		return GetInstance().CreatePoolConnectionImpl(dbType, timeout);
	}

	/// \brief Returns the current ODBC connection string for a given DbType, or empty string if there is none
	std::string ConnectionManager::GetOdbcConnectionString(modelUtil::DbType dbType)
	{
		std::string out;

		auto config = GetDatasourceConfig(dbType);
		if (config != nullptr)
		{
			out = "ODBC;DSN=" + config->DatasourceName + ";UID=" + config->DatasourceUsername + ";PWD=" + config->DatasourcePassword;
		}
		
		return out;
	}

	// \brief Disconnects and removes pooled connections that haven't been used in some time.
	// Will always leave at least 1 connection available.
	void ConnectionManager::ExpireUnusedPoolConnections()
	{
		GetInstance().ExpireUnusedPoolConnectionsImpl();
	}

	void ConnectionManager::SetDatasourceConfigImpl(
		modelUtil::DbType dbType,
		const std::string_view datasourceName,
		const std::string_view datasourceUserName,
		const std::string_view datasourcePassword)
	{
		std::lock_guard<std::mutex> lock(_configLock);

		auto itr = _configMap.find(dbType);
		if (itr == _configMap.end())
		{
			auto config = std::make_shared<const DatasourceConfig>(
				datasourceName,
				datasourceUserName,
				datasourcePassword);

			_configMap.insert({ dbType, config });
		}
		else
		{
			auto config = std::const_pointer_cast<DatasourceConfig>(itr->second);

			config->DatasourceName = datasourceName,
			config->DatasourceUsername = datasourceUserName,
			config->DatasourcePassword = datasourcePassword;
		}
	}

	/// \brief fetch the associated previously stored database config using the code-generated dbType
	std::shared_ptr<const DatasourceConfig>
	ConnectionManager::GetDatasourceConfigImpl(modelUtil::DbType dbType) const
	{
		std::lock_guard<std::mutex> lock(_configLock);

		auto itr = _configMap.find(dbType);
		if (itr == _configMap.end())
			return nullptr;

		return itr->second;
	}

	/// \brief attempt a connection to the database using the code-generated dbType
	/// \throws std::runtime_error
	/// \throws nanodbc::database_error
	std::shared_ptr<Connection> ConnectionManager::CreateConnectionImpl(modelUtil::DbType dbType, long timeout) noexcept(false)
	{
		auto config = GetDatasourceConfig(dbType);
		if (config == nullptr)
			throw DatasourceConfigNotFoundException("Invalid database type: " + modelUtil::DbTypeString(dbType));

		// If not supplied (we default the arg to -1), use the configured default connection timeout instead.
		if (timeout == -1)
			timeout = DefaultConnectionTimeout;

		auto nanoconn = std::make_shared<nanodbc::connection>(
				config->DatasourceName,
				config->DatasourceUsername,
				config->DatasourcePassword,
				timeout);
		
		return std::make_shared<Connection>(nanoconn, config, timeout);
	}

	/// \brief fetch an established pool connection using the code-generated dbType, or otherwise attempt to establish a new
	/// pool connection using the code-generated dbType if one is not available.
	/// \throws DatasourceConfigNotFoundException
	/// \throws nanodbc::database_error
	std::shared_ptr<PoolConnection> ConnectionManager::CreatePoolConnectionImpl(modelUtil::DbType dbType, long timeout) noexcept(false)
	{
		auto conn = PullPoolConnection(dbType);

		// We pulled an existing connection; return a wrapper to manage it.
		if (conn == nullptr)
		{
			// No available connection - spin up a new connection.
			conn = CreateConnectionImpl(dbType, timeout);

			// Failed to allocate.
			if (conn == nullptr)
				return nullptr;
		}

		return std::make_shared<PoolConnection>(dbType, conn);
	}

	// \brief Disconnects and removes pooled connections that haven't been used in some time.
	// Will always leave at least 1 connection available.
	void ConnectionManager::ExpireUnusedPoolConnectionsImpl()
	{
		time_t nowTime = time(nullptr);

		std::lock_guard<std::mutex> lock(_connectionPoolLock);
		for (auto itr = _connectionPoolMap.begin(); itr != _connectionPoolMap.end(); )
		{
			modelUtil::DbType dbType = itr->first;

			// We should always keep 1 (the first) available.
			auto nextItr = std::next(itr);
			while (nextItr != _connectionPoolMap.end()
				&& nextItr->first == dbType)
			{
				ConnectionInfo& connInfo = nextItr->second;

				// Expire any others that haven't been used in the specified time.
				if ((nowTime - connInfo.LastUsedTime) >= PoolConnectionTimeout)
					nextItr = _connectionPoolMap.erase(nextItr);
				else
					++nextItr;
			}

			itr = nextItr;
		}

	}

	/// \brief pulls an established connection using the code-generated dbType from the dbType pool, if available.
	/// \throws DatasourceConfigNotFoundException
	/// \throws nanodbc::database_error
	std::shared_ptr<Connection> ConnectionManager::PullPoolConnection(modelUtil::DbType dbType) noexcept(false)
	{
		std::lock_guard<std::mutex> lock(_connectionPoolLock);

		auto itr = _connectionPoolMap.find(dbType);
		if (itr == _connectionPoolMap.end())
			return nullptr;

		ConnectionInfo& connInfo = itr->second;
		auto conn = connInfo.Conn;
		_connectionPoolMap.erase(itr);
		return conn;
	}

	/// \brief restores an established connection using the code-generated dbType to the dbType pool.
	void ConnectionManager::RestorePoolConnection(modelUtil::DbType dbType, const std::shared_ptr<Connection>& conn) noexcept(false)
	{
		ConnectionInfo connInfo =
		{
			.Conn = conn,
			.LastUsedTime = time(nullptr)
		};

		std::lock_guard<std::mutex> lock(_connectionPoolLock);
		_connectionPoolMap.insert(std::make_pair(dbType, std::move(connInfo)));
	}

	ConnectionManager::~ConnectionManager()
	{
		assert(s_instance != nullptr);
		s_instance = nullptr;
	}
}

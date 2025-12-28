#include "pch.h"
#include "Connection.h"
#include "DatasourceConfig.h"
#include "Exceptions.h"
#include "utils.h"
#include <spdlog/spdlog.h>

namespace db
{

	Connection::Connection(
		const std::shared_ptr<nanodbc::connection>& conn,
		const std::shared_ptr<const DatasourceConfig>& config,
		long timeout)
		: _conn(conn), _config(config), _timeout(timeout)
	{
	}

	/// \brief Checks to see if the connection is disconnected, and attempts
	/// to reconnect if appropriate.
	/// \returns -1 for failed connection, 0 for no-op, 1 for successful reconnect
	int8_t Connection::Reconnect() noexcept(false)
	{
		if (_conn == nullptr)
			return -1;

		// only calling connect if we're currently disconnected/timed out
		// calling connect while connected would likely refresh a timeout, however
		// it also deallocs/allocs resources and may cause unknown side effects
		if (!_conn->connected())
		{
			// this can throw an exception
			_conn->connect(_config->DatasourceName, _config->DatasourceUsername, _config->DatasourcePassword, _timeout);
			return 1;
		}

		return 0;
	}

	/// \brief checks if the managed connection is disconnected and attempts to reconnect if it is
	/// \throws nanodbc::database_error
	void Connection::ReconnectIfDisconnected() noexcept(false)
	{
		try
		{
			int8_t result = Reconnect();
			if (result == -1)
			{
				throw ApplicationError("Failed to connect. This usually means the connection is null");
			}

			// reconnect was necessary and successful
			if (result == 1)
			{
				spdlog::info("ReconnectIfDisconnected(): reconnect successful");
			}
		}
		catch (const nanodbc::database_error& dbErr)
		{
			utils::LogDatabaseError(dbErr, "ReconnectIfDisconnected()");
			throw;
		}
	}

	/// \brief Creates a new statement for the current database connection.
	/// \throws nanodbc::database_error
	nanodbc::statement Connection::CreateStatement() noexcept(false)
	{
		ReconnectIfDisconnected();
		return nanodbc::statement(*_conn);
	}

	/// \brief Creates a new statement for the current database connection, using the given query.
	/// \throws nanodbc::database_error
	nanodbc::statement Connection::CreateStatement(const std::string& query) noexcept(false)
	{
		ReconnectIfDisconnected();
		return nanodbc::statement(*_conn, query);
	}

	Connection::operator std::shared_ptr<nanodbc::connection> () const
	{
		return _conn;
	}

	Connection::operator nanodbc::connection* () const
	{
		return _conn.get();
	}

	Connection::operator nanodbc::connection& () const
	{
		return *_conn;
	}

}

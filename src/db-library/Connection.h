#ifndef DBLIBRARY_CONNECTION_H
#define DBLIBRARY_CONNECTION_H

#pragma once

#include <memory> // std::shared_ptr<>
#include <nanodbc/nanodbc.h>

namespace db
{

struct DatasourceConfig;
class Connection
{
public:
	Connection(const std::shared_ptr<nanodbc::connection>& conn,
		const std::shared_ptr<const DatasourceConfig>& config, long timeout);

	/// \brief Checks to see if the connection is disconnected, and attempts
	/// to reconnect if appropriate.
	/// \returns -1 for failed connection, 0 for no-op, 1 for successful reconnect
	int8_t Reconnect() noexcept(false);

	/// \brief Checks if the managed connection is disconnected and attempts to reconnect if it is
	/// \throws nanodbc::database_error
	void ReconnectIfDisconnected() noexcept(false);

	/// \brief Creates a new statement for the current database connection.
	/// \throws nanodbc::database_error
	nanodbc::statement CreateStatement() noexcept(false);

	/// \brief Creates a new statement for the current database connection, using the given query.
	/// \throws nanodbc::database_error
	nanodbc::statement CreateStatement(const std::string& query) noexcept(false);

	operator std::shared_ptr<nanodbc::connection>() const;
	operator nanodbc::connection*() const;
	operator nanodbc::connection&() const;

protected:
	std::shared_ptr<nanodbc::connection> _conn;
	std::shared_ptr<const DatasourceConfig> _config;
	long _timeout;
};

} // namespace db

#endif // DBLIBRARY_CONNECTION_H

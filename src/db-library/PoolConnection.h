#ifndef DBLIBRARY_POOLCONNECTION_H
#define DBLIBRARY_POOLCONNECTION_H

#pragma once

#include <memory>
#include "Connection.h"

#include <ModelUtil/ModelUtil.h>

namespace db
{

	class PoolConnection
	{
	public:
		PoolConnection(modelUtil::DbType dbType, const std::shared_ptr<Connection>& conn);
		~PoolConnection();

		/// \brief checks if the managed connection is disconnected and attempts to reconnect if it is
		/// \throws nanodbc::database_error
		void ReconnectIfDisconnected() noexcept(false);

		/// \brief Creates a new statement for the current database connection.
		/// \throws nanodbc::database_error
		nanodbc::statement CreateStatement() noexcept(false);

		/// \brief Creates a new statement for the current database connection, using the given query.
		/// \throws nanodbc::database_error
		nanodbc::statement CreateStatement(const std::string& query) noexcept(false);

		operator std::shared_ptr<Connection> () const;
		operator Connection* () const;
		operator std::shared_ptr<nanodbc::connection>() const;
		operator nanodbc::connection* () const;
		operator nanodbc::connection& () const;

	protected:
		modelUtil::DbType _dbType;
		std::shared_ptr<Connection> _conn;
	};

}

#endif // DBLIBRARY_POOLCONNECTION_H

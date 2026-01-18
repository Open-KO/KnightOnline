#include "pch.h"
#include "PoolConnection.h"
#include "ConnectionManager.h"

namespace db
{

PoolConnection::PoolConnection(modelUtil::DbType dbType, const std::shared_ptr<Connection>& conn) :
	_dbType(dbType), _conn(conn)
{
}

PoolConnection::~PoolConnection()
{
	ConnectionManager::GetInstance().RestorePoolConnection(_dbType, _conn);
}

void PoolConnection::ReconnectIfDisconnected() noexcept(false)
{
	_conn->ReconnectIfDisconnected();
}

nanodbc::statement PoolConnection::CreateStatement() noexcept(false)
{
	return _conn->CreateStatement();
}

nanodbc::statement PoolConnection::CreateStatement(const std::string& query) noexcept(false)
{
	return _conn->CreateStatement(query);
}

PoolConnection::operator std::shared_ptr<Connection>() const
{
	return _conn;
}

PoolConnection::operator Connection*() const
{
	return _conn.get();
}

PoolConnection::operator std::shared_ptr<nanodbc::connection>() const
{
	return *_conn;
}

PoolConnection::operator nanodbc::connection*() const
{
	return *_conn;
}

PoolConnection::operator nanodbc::connection&() const
{
	return *_conn;
}

} // namespace db

#include "pch.h"
#include "TelnetClientThread.h"
#include "AppThread.h"
#include "TelnetThread.h"

#include <db-library/Connection.h>
#include <db-library/Exceptions.h>
#include <db-library/RecordSetLoader_STLMap.h>
#include <db-library/utils.h>
#include <db-models/Full/binder/FullBinder.h>
#include <db-models/Full/model/FullModel.h>

#include <nanodbc/nanodbc.h>
#include <spdlog/spdlog.h>

#include <chrono>

using namespace std::chrono_literals;

TelnetClientThread::TelnetClientThread(asio::ip::tcp::socket&& rawSocket, uint32_t socketId) :
	_clientSocket(std::move(rawSocket)), _socketId(socketId)
{
	asio::error_code ec;

	asio::ip::tcp::endpoint endpoint = _clientSocket.remote_endpoint(ec);
	if (ec)
	{
		spdlog::warn("TelnetClientThread::TelnetClientThread: failed remote IP lookup. socketId={}",
			_socketId);
	}
	else
	{
		_remoteIp = endpoint.address().to_string();
	}
}

TelnetClientThread::~TelnetClientThread()
{
	spdlog::debug("TelnetClientThread::~TelnetClientThread()");
}

void TelnetClientThread::Disconnect()
{
	if (_clientSocket.is_open())
	{
		std::error_code ec;
		_clientSocket.shutdown(asio::ip::tcp::socket::shutdown_both);
		_clientSocket.close(ec);
	}

	shutdown(false);
}

void TelnetClientThread::thread_loop()
{
	try
	{
		WriteLine("User:");
		std::string accountId;
		do
		{
			accountId = ReadLine();
		}
		while (accountId.empty());

		WriteLine("Password:");
		std::string password;
		do
		{
			password = ReadLine();
		}
		while (password.empty());

		if (!Authenticate(accountId, password))
		{
			Disconnect();
			return;
		}

		_accountId = std::move(accountId);

		spdlog::info("TelnetClientThread::thread_loop: {} authenticated [remoteIp={}]", _accountId,
			_remoteIp);
		WriteLine("Authenticated.  Accepting commands. \"quit\" to close connection.");

		while (CanTick())
		{
			if (!_clientSocket.is_open())
			{
				spdlog::info("TelnetClientThread::thread_loop: socket longer open [accountId={} "
							 "remoteIp={}]",
					_accountId, _remoteIp);

				shutdown(false);
				continue;
			}

			std::string input = ReadLine();
			if (input.empty())
				continue;

			spdlog::info("TelnetClientThread::thread_loop: input: {} [accountId={} remoteIp={}]",
				input, _accountId, _remoteIp);

			if (input == "quit")
			{
				spdlog::info("TelnetClientThread::thread_loop: socket closed by request "
							 "[accountId={} remoteIp={}]",
					accountId, _remoteIp);

				Disconnect();
				break;
			}

			if (input == "healthcheck")
			{
				HealthCheck();
				continue;
			}

			AppThread* appThread = AppThread::instance();
			if (appThread == nullptr)
			{
				Disconnect();
				continue;
			}

			if (appThread->HandleCommand(input))
				WriteLine(fmt::format("Input command accepted: {}", input));
			else
				WriteLine(fmt::format("Input command failed: {}", input));
		}
	}
	catch (const std::exception& e)
	{
		spdlog::error("TelnetClientThread::thread_loop: {} [accountId={} remoteIp={}]", e.what(),
			_accountId, _remoteIp);
	}
}

void TelnetClientThread::WriteLine(const std::string& line)
{
	asio::write(_clientSocket, asio::buffer(line + "\r\n"));
}

std::string TelnetClientThread::ReadLine()
{
	constexpr char Delimiter = '\n';

	std::string line;
	asio::streambuf buffer;
	std::promise<size_t> promise;

	asio::async_read_until(_clientSocket, buffer, Delimiter,
		[&](const asio::error_code& ec, std::size_t bytesRead)
		{
			if (ec)
				promise.set_exception(std::make_exception_ptr(asio::system_error(ec)));
			else
				promise.set_value(bytesRead);
		});

	auto future      = promise.get_future();
	size_t bytesRead = future.get();
	if (bytesRead == 0)
		throw std::runtime_error("socket disconnected");

	std::istream is(&buffer);
	std::getline(is, line, Delimiter);

	// strip any trailing \r
	if (!line.empty() && line.back() == '\r')
		line.pop_back();

	return line;
}

bool TelnetClientThread::Authenticate(const std::string& accountId, const std::string& password)
{
	full_model::TbUser user {};

	try
	{
		db::SqlBuilder<full_model::TbUser> sql;
		sql.IsWherePK = true;

		db::ModelRecordSet<full_model::TbUser> recordSet;

		auto stmt = recordSet.prepare(sql);
		if (stmt == nullptr)
		{
			throw db::ApplicationError(
				"TelnetClientThread::Authenticate: statement could not be allocated");
		}

		stmt->bind(0, accountId.c_str());
		recordSet.execute();

		if (!recordSet.next())
		{
			spdlog::warn(
				"TelnetClientThread::Authenticate: failed authentication attempt for user {} - "
				"missing account [remoteIp={}]",
				accountId, _remoteIp);
			return false;
		}

		recordSet.get_ref(user);
	}
	catch (const nanodbc::database_error& dbErr)
	{
		db::utils::LogDatabaseError(dbErr, "TelnetClientThread::Authenticate()");
		return false;
	}

	if (user.Password != password)
	{
		spdlog::warn(
			"TelnetClientThread::Authenticate: failed authentication attempt for user {} - "
			"incorrect password [remoteIp={}]",
			accountId, _remoteIp);
		return false;
	}

	if (user.Authority != AUTHORITY_MANAGER)
	{
		spdlog::warn(
			"TelnetClientThread::Authenticate: failed authentication attempt for user {} - "
			"not AUTHORITY_MANAGER [remoteIp={}]",
			accountId, _remoteIp);
		return false;
	}

	return true;
}

void TelnetClientThread::HealthCheck()
{
	AppThread* appThread = AppThread::instance();
	if (appThread == nullptr)
	{
		WriteLine("status: STOPPED");
		return;
	}

	switch (appThread->GetAppStatus())
	{
		case AppStatus::INITIALIZING:
			WriteLine("status: INITIALIZING");
			break;

		case AppStatus::STARTING:
			WriteLine("status: STARTING");
			break;

		case AppStatus::READY:
			WriteLine("status: READY");
			break;

		case AppStatus::STOPPING:
			WriteLine("status: STOPPING");
			break;
	}
}

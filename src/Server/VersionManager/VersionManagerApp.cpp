#include "pch.h"
#include "VersionManagerApp.h"
#include "User.h"

#include <db-library/ConnectionManager.h>
#include <db-library/RecordSetLoader.h>
#include <shared/Ini.h>
#include <shared/TimerThread.h>

#include <spdlog/spdlog.h>

#include <VersionManager/binder/VersionManagerBinder.h>

using namespace std::chrono_literals;

namespace VersionManager
{

VersionManagerApp::VersionManagerApp(logger::Logger& logger) :
	AppThread(logger), _serverSocketManager(SOCKET_BUFF_SIZE, SOCKET_BUFF_SIZE)
{
	_telnetPort                                     = 2326;
	db::ConnectionManager::DefaultConnectionTimeout = DB_PROCESS_TIMEOUT;
	db::ConnectionManager::Create();

	_dbPoolCheckThread = std::make_unique<TimerThread>(
		1min, std::bind(&db::ConnectionManager::ExpireUnusedPoolConnections));
}

VersionManagerApp::~VersionManagerApp()
{
	spdlog::info("VersionManagerApp::~VersionManagerApp: Shutting down, releasing resources.");
	_serverSocketManager.Shutdown();
	spdlog::info("VersionManagerApp::~VersionManagerApp: TcpServerSocketManager stopped.");

	spdlog::info(
		"VersionManagerApp::~VersionManagerApp: Waiting for worker threads to fully shut down.");

	if (_dbPoolCheckThread != nullptr)
	{
		spdlog::info("VersionManagerApp::~VersionManagerApp: Shutting down CheckAliveThread...");

		_dbPoolCheckThread->shutdown();

		spdlog::info("VersionManagerApp::~VersionManagerApp: DB pool check thread stopped.");
	}

	spdlog::info(
		"VersionManagerApp::~VersionManagerApp: All worker threads stopped, freeing caches.");

	for (_SERVER_INFO* pInfo : ServerList)
		delete pInfo;
	ServerList.clear();

	spdlog::info("VersionManagerApp::~VersionManagerApp: All resources safely released.");

	db::ConnectionManager::Destroy();
}

bool VersionManagerApp::OnStart()
{
	_serverSocketManager.Init(MAX_USER, 1);
	_serverSocketManager.AllocateSockets<CUser>();

	spdlog::info("Version Manager initialized");

	// print the ODBC connection string
	// TODO: modelUtil::DbType::ACCOUNT;  Currently all models are assigned to GAME
	spdlog::debug(db::ConnectionManager::GetOdbcConnectionString(modelUtil::DbType::GAME));

	if (!DbProcess.InitDatabase())
	{
		spdlog::error("Database Connection Fail!!");
		return false;
	}

	if (!LoadVersionList())
	{
		spdlog::error("Load Version List Fail!!");
		return false;
	}

	if (!_serverSocketManager.Listen(_LISTEN_PORT))
	{
		spdlog::error("FAIL TO CREATE LISTEN STATE");
		return false;
	}

	_serverSocketManager.StartAccept();

	spdlog::info("Listening on 0.0.0.0:{}", _LISTEN_PORT);

	_dbPoolCheckThread->start();

	return true;
}

std::filesystem::path VersionManagerApp::ConfigPath() const
{
	return "Version.ini";
}

bool VersionManagerApp::LoadConfig(CIni& iniFile)
{
	// ftp config
	_ftpUrl                    = iniFile.GetString(ini::DOWNLOAD, ini::URL, "127.0.0.1");
	_ftpPath                   = iniFile.GetString(ini::DOWNLOAD, ini::PATH, "/");

	// TODO: KN_online should be Knight_Account
	std::string datasourceName = iniFile.GetString(ini::ODBC, ini::DSN, "KN_online");
	std::string datasourceUser = iniFile.GetString(ini::ODBC, ini::UID, "knight");
	std::string datasourcePass = iniFile.GetString(ini::ODBC, ini::PWD, "knight");

	db::ConnectionManager::SetDatasourceConfig(
		modelUtil::DbType::ACCOUNT, datasourceName, datasourceUser, datasourcePass);

	// TODO: Remove this - currently all models are assigned to GAME
	db::ConnectionManager::SetDatasourceConfig(
		modelUtil::DbType::GAME, datasourceName, datasourceUser, datasourcePass);

	int serverCount = iniFile.GetInt(ini::SERVER_LIST, ini::COUNT, 1);

	if (_ftpUrl.empty())
	{
		spdlog::error("VersionManagerApp::LoadConfig: The FTP URL must be set.");
		return false;
	}

	if (_ftpPath.empty())
	{
		spdlog::error("VersionManagerApp::LoadConfig: The FTP path must be set.");
		return false;
	}

	if (datasourceName.empty()
		// TODO: Should we not validate UID/Pass length?  Would that allow Windows Auth?
		|| datasourceUser.empty() || datasourcePass.empty())
	{
		spdlog::error("VersionManagerApp::LoadConfig: Datasource config must be set.");
		return false;
	}

	if (serverCount <= 0)
	{
		spdlog::error(
			"VersionManagerApp::LoadConfig: At least 1 server must exist in the server list.");
		return false;
	}

	std::string key;
	ServerList.reserve(serverCount);

	for (int i = 0; i < serverCount; i++)
	{
		_SERVER_INFO* pInfo  = new _SERVER_INFO;

		key                  = fmt::format("SERVER_{:02}", i);
		pInfo->strServerIP   = iniFile.GetString(ini::SERVER_LIST, key, "127.0.0.1");

		key                  = fmt::format("NAME_{:02}", i);
		pInfo->strServerName = iniFile.GetString(ini::SERVER_LIST, key, "TEST|Server 1");

		key                  = fmt::format("ID_{:02}", i);
		pInfo->sServerID     = static_cast<int16_t>(iniFile.GetInt(ini::SERVER_LIST, key, 1));

		key                  = fmt::format("USER_LIMIT_{:02}", i);
		pInfo->sUserLimit = static_cast<int16_t>(iniFile.GetInt(ini::SERVER_LIST, key, MAX_USER));

		ServerList.push_back(pInfo);
	}

	// Read news from INI (max 3 blocks)
	std::stringstream ss;
	std::string title, message;

	News.Size = 0;
	for (int i = 0; i < MAX_NEWS_COUNT; i++)
	{
		key   = fmt::format("TITLE_{:02}", i);
		title = iniFile.GetString("NEWS", key, "");
		if (title.empty())
			continue;

		key     = fmt::format("MESSAGE_{:02}", i);
		message = iniFile.GetString("NEWS", key, "");
		if (message.empty())
			continue;

		ss << title;
		ss.write(NEWS_MESSAGE_START, sizeof(NEWS_MESSAGE_START));
		ss << message;
		ss.write(NEWS_MESSAGE_END, sizeof(NEWS_MESSAGE_END));
	}

	const std::string newsContent = ss.str();
	if (!newsContent.empty())
	{
		if (newsContent.size() > sizeof(News.Content))
		{
			spdlog::error("VersionManagerApp::LoadConfig: News too long");
			return false;
		}

		memcpy(&News.Content, newsContent.c_str(), newsContent.size());
		News.Size = static_cast<int16_t>(newsContent.size());
	}

	return true;
}

bool VersionManagerApp::LoadVersionList()
{
	VersionInfoList versionList;
	if (!DbProcess.LoadVersionList(&versionList))
		return false;

	int lastVersion = 0;

	for (const auto& [_, pInfo] : versionList)
	{
		if (lastVersion < pInfo->Number)
			lastVersion = pInfo->Number;
	}

	if (lastVersion != _lastVersion)
		spdlog::info("Latest Version: {}", lastVersion);

	_lastVersion = lastVersion;

	VersionList.Swap(versionList);
	return true;
}

} // namespace VersionManager

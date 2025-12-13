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

VersionManagerApp::VersionManagerApp(logger::Logger& logger)
	: AppThread(logger),
	_socketManager(SOCKET_BUFF_SIZE, SOCKET_BUFF_SIZE)
{
	memset(_ftpUrl, 0, sizeof(_ftpUrl));
	memset(_ftpPath, 0, sizeof(_ftpPath));
	_lastVersion = 0;

	db::ConnectionManager::DefaultConnectionTimeout = DB_PROCESS_TIMEOUT;
	db::ConnectionManager::Create();

	_dbPoolCheckThread = std::make_unique<TimerThread>(
		1min,
		std::bind(&db::ConnectionManager::ExpireUnusedPoolConnections));
}

VersionManagerApp::~VersionManagerApp()
{
	spdlog::info("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
	spdlog::info("🛑 Shutting down Version Manager Server...");
	spdlog::info("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
	
	spdlog::info("  → Stopping Socket Manager...");
	_socketManager.Shutdown();
	spdlog::info("  ✓ Socket Manager stopped successfully");

	spdlog::info("  → Waiting for worker threads to shutdown...");

	if (_dbPoolCheckThread != nullptr)
	{
		spdlog::info("  → Stopping DB Pool Check Thread...");
		_dbPoolCheckThread->shutdown();
		spdlog::info("  ✓ DB Pool Check Thread stopped");
	}

	spdlog::info("  → Freeing server list cache...");
	for (_SERVER_INFO* pInfo : ServerList)
		delete pInfo;
	ServerList.clear();
	spdlog::info("  ✓ Server list cache cleared");

	spdlog::info("  → Destroying database connection manager...");
	db::ConnectionManager::Destroy();
	spdlog::info("  ✓ Database connections closed");

	spdlog::info("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
	spdlog::info("✓ All resources safely released. Server shutdown complete.");
	spdlog::info("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
}

bool VersionManagerApp::OnStart()
{
	spdlog::info("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
	spdlog::info("🚀 Initializing Version Manager Server...");
	spdlog::info("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");

	spdlog::info("  → Initializing Socket Manager (Max Users: {})...", MAX_USER);
	_socketManager.Init(MAX_USER, 0, 1);
	_socketManager.AllocateServerSockets<CUser>();
	spdlog::info("  ✓ Socket Manager initialized");

	// print the ODBC connection string
	// TODO: modelUtil::DbType::ACCOUNT;  Currently all models are assigned to GAME
	spdlog::debug("  → ODBC Connection: {}", 
		db::ConnectionManager::GetOdbcConnectionString(modelUtil::DbType::GAME));

	spdlog::info("  → Connecting to database...");
	if (!DbProcess.InitDatabase())
	{
		spdlog::error("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
		spdlog::error("❌ FATAL ERROR: Database connection failed!");
		spdlog::error("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
		return false;
	}
	spdlog::info("  ✓ Database connection established");

	spdlog::info("  → Loading version list from database...");
	if (!LoadVersionList())
	{
		spdlog::error("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
		spdlog::error("❌ FATAL ERROR: Failed to load version list!");
		spdlog::error("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
		return false;
	}
	spdlog::info("  ✓ Version list loaded successfully");

	spdlog::info("  → Starting network listener on port {}...", _LISTEN_PORT);
	if (!_socketManager.Listen(_LISTEN_PORT))
	{
		spdlog::error("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
		spdlog::error("❌ FATAL ERROR: Failed to create listen socket!");
		spdlog::error("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
		return false;
	}

	_socketManager.StartAccept();
	spdlog::info("  ✓ Network listener started");

	spdlog::info("  → Starting DB pool check thread...");
	_dbPoolCheckThread->start();
	spdlog::info("  ✓ DB pool check thread started");

	spdlog::info("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
	spdlog::info("✅ Server is now ONLINE and listening on 0.0.0.0:{}", _LISTEN_PORT);
	spdlog::info("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
	spdlog::info("");

	return true;
}

/// \returns The application's ini config path.
std::filesystem::path VersionManagerApp::ConfigPath() const
{
	return GetProgPath() / "Version.ini";
}

bool VersionManagerApp::LoadConfig(CIni& iniFile)
{
	spdlog::info("  → Loading configuration from: {}", ConfigPath().string());
	
	// ftp config
	iniFile.GetString(ini::DOWNLOAD, ini::URL, "127.0.0.1", _ftpUrl, sizeof(_ftpUrl));
	iniFile.GetString(ini::DOWNLOAD, ini::PATH, "/", _ftpPath, sizeof(_ftpPath));
	
	// TODO: KN_online should be Knight_Account
	std::string datasourceName = iniFile.GetString(ini::ODBC, ini::DSN, "KN_online");
	std::string datasourceUser = iniFile.GetString(ini::ODBC, ini::UID, "knight");
	std::string datasourcePass = iniFile.GetString(ini::ODBC, ini::PWD, "knight");

	db::ConnectionManager::SetDatasourceConfig(
		modelUtil::DbType::ACCOUNT,
		datasourceName, datasourceUser, datasourcePass);

	// TODO: Remove this - currently all models are assigned to GAME
	db::ConnectionManager::SetDatasourceConfig(
		modelUtil::DbType::GAME,
		datasourceName, datasourceUser, datasourcePass);

	int serverCount = iniFile.GetInt(ini::SERVER_LIST, ini::COUNT, 1);

	if (strlen(_ftpUrl) == 0)
	{
		spdlog::error("  ❌ Configuration Error: FTP URL must be set in [DOWNLOAD] section");
		return false;
	}

	if (strlen(_ftpPath) == 0)
	{
		spdlog::error("  ❌ Configuration Error: FTP path must be set in [DOWNLOAD] section");
		return false;
	}

	if (datasourceName.empty()
		// TODO: Should we not validate UID/Pass length?  Would that allow Windows Auth?
		|| datasourceUser.empty()
		|| datasourcePass.empty())
	{
		spdlog::error("  ❌ Configuration Error: Database credentials must be set in [ODBC] section");
		return false;
	}

	if (serverCount <= 0)
	{
		spdlog::error("  ❌ Configuration Error: At least 1 server must exist in server list");
		return false;
	}

	char key[32] = {};
	ServerList.reserve(serverCount);

	for (int i = 0; i < serverCount; i++)
	{
		_SERVER_INFO* pInfo = new _SERVER_INFO;

		snprintf(key, sizeof(key), "SERVER_%02d", i);
		iniFile.GetString(ini::SERVER_LIST, key, "127.0.0.1", pInfo->strServerIP, sizeof(pInfo->strServerIP));

		snprintf(key, sizeof(key), "NAME_%02d", i);
		iniFile.GetString(ini::SERVER_LIST, key, "TEST|Server 1", pInfo->strServerName, sizeof(pInfo->strServerName));

		snprintf(key, sizeof(key), "ID_%02d", i);
		pInfo->sServerID = static_cast<int16_t>(iniFile.GetInt(ini::SERVER_LIST, key, 1));

		snprintf(key, sizeof(key), "USER_LIMIT_%02d", i);
		pInfo->sUserLimit = static_cast<int16_t>(iniFile.GetInt(ini::SERVER_LIST, key, MAX_USER));

		ServerList.push_back(pInfo);
	}
	spdlog::info("  ✓ Loaded {} server(s) from configuration", serverCount);

	// Read news from INI (max 3 blocks)
	std::stringstream ss;
	std::string title, message;

	News.Size = 0;
	for (int i = 0; i < MAX_NEWS_COUNT; i++)
	{
		snprintf(key, sizeof(key), "TITLE_%02d", i);
		title = iniFile.GetString("NEWS", key, "");
		if (title.empty())
			continue;

		snprintf(key, sizeof(key), "MESSAGE_%02d", i);
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
			spdlog::error("  ❌ Configuration Error: News content exceeds maximum size ({} bytes)", 
				sizeof(News.Content));
			return false;
		}

		memcpy(&News.Content, newsContent.c_str(), newsContent.size());
		News.Size = static_cast<int16_t>(newsContent.size());
		spdlog::info("  ✓ News content loaded ({} bytes)", News.Size);
	}

	spdlog::info("  ✓ Configuration loaded successfully");
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
		spdlog::info("  📦 Latest Client Version: {}", lastVersion);

	_lastVersion = lastVersion;

	VersionList.Swap(versionList);
	return true;
}

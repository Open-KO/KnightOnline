#ifndef SERVER_VERSIONMANAGER_VERSIONMANAGERAPP_H
#define SERVER_VERSIONMANAGER_VERSIONMANAGERAPP_H

#pragma once

#include "Define.h"
#include "DBProcess.h"

#include <shared-server/AppThread.h>
#include <shared-server/TcpServerSocketManager.h>

#include <vector>
#include <string>

class TimerThread;

namespace VersionManager
{

class VersionManagerApp : public AppThread
{
	using ServerInfoList = std::vector<_SERVER_INFO*>;

public:
	static VersionManagerApp* instance()
	{
		return static_cast<VersionManagerApp*>(s_instance);
	}

	const std::string& FtpUrl() const
	{
		return _ftpUrl;
	}

	const std::string& FtpPath() const
	{
		return _ftpPath;
	}

	int LastVersion() const
	{
		return _lastVersion;
	}

	VersionManagerApp(logger::Logger& logger);
	~VersionManagerApp() override;
	bool LoadVersionList();

	TcpServerSocketManager _serverSocketManager;

	VersionInfoList VersionList;
	ServerInfoList ServerList;
	_NEWS News;
	CDBProcess DbProcess;

protected:
	/// \returns The application's ini config path.
	std::filesystem::path ConfigPath() const override;

	/// \brief Loads application-specific config from the loaded application ini file (`iniFile`).
	/// \param iniFile The loaded application ini file.
	/// \returns true when successful, false otherwise
	bool LoadConfig(CIni& iniFile) override;

	/// \brief Loads config, database caches, then starts sockets and thread pools.
	/// \returns true when successful, false otherwise
	bool OnStart() override;

protected:
	std::string _ftpUrl;
	std::string _ftpPath;

	int _lastVersion = 0;

	std::unique_ptr<TimerThread> _dbPoolCheckThread;
};

} // namespace VersionManager

#endif // SERVER_VERSIONMANAGER_VERSIONMANAGERAPP_H

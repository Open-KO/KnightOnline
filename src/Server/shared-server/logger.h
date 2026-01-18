#ifndef SERVER_SHAREDSERVER_LOGGER_H
#define SERVER_SHAREDSERVER_LOGGER_H

#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

#include <spdlog/sinks/stdout_color_sinks.h>

// forward declarations
class CIni;

namespace spdlog::details
{
class thread_pool;
}

namespace logger
{

class Logger
{
	// setup defaults
	static constexpr uint16_t MessageQueueSize = 8192;
	static constexpr uint8_t ThreadPoolSize    = 1;

public:
	const std::string& AppName() const
	{
		return _appName;
	}

	/// \param appName application name (VersionManager, Aujard, AIServer, Ebenezer)
	Logger(const std::string_view appName);

	/// \brief Sets up spdlog from an ini file using standardized server settings
	/// \param ini server application's ini file (already loaded)
	/// \param baseDir base directory to store logs folder under
	void Setup(CIni& ini, const std::filesystem::path& baseDir);

	virtual void SetupExtraLoggers(CIni& ini,
		std::shared_ptr<spdlog::details::thread_pool> threadPool,
		const std::filesystem::path& baseDir);

	void SetupExtraLogger(CIni& ini, std::shared_ptr<spdlog::details::thread_pool> threadPool,
		const std::filesystem::path& baseDir, const std::string_view appName,
		const std::string_view logFileConfigProp);

	virtual ~Logger();

protected:
	std::string _appName;
	std::string _defaultLogPath;

	std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> _consoleLogger;
};

//
// Application names used by our loggers
//

// AI Server
static constexpr std::string_view AIServer        = "AIServer";
static constexpr std::string_view AIServerItem    = "AIServerItem";
static constexpr std::string_view AIServerUser    = "AIServerUser";

// Ebenezer
static constexpr std::string_view Ebenezer        = "Ebenezer";
static constexpr std::string_view EbenezerEvent   = "EbenezerEvent";
static constexpr std::string_view EbenezerRegion  = "EbenezerRegion";

// Aujard
static constexpr std::string_view Aujard          = "Aujard";

// Version Manager
static constexpr std::string_view VersionManager  = "VersionManager";

// Item Manager
static constexpr std::string_view ItemManager     = "ItemManager";
static constexpr std::string_view ItemManagerItem = "ItemManagerItem";
static constexpr std::string_view ItemManagerExp  = "ItemManagerExp";
} // namespace logger

namespace ini
{
// LOGGER section
static constexpr std::string_view LOGGER                      = "LOGGER";
static constexpr std::string_view LEVEL                       = "LEVEL";
static constexpr std::string_view PATTERN                     = "PATTERN";
static constexpr std::string_view CONSOLE_PATTERN             = "CONSOLE_PATTERN";
static constexpr std::string_view FILE                        = "FILE";
static constexpr std::string_view ITEM_LOG_FILE               = "ITEM_LOG_FILE";
static constexpr std::string_view USER_LOG_FILE               = "USER_LOG_FILE";
static constexpr std::string_view REGION_LOG_FILE             = "REGION_LOG_FILE";
static constexpr std::string_view EVENT_LOG_FILE              = "EVENT_LOG_FILE";
static constexpr std::string_view EXP_LOG_FILE                = "EXP_LOG_FILE";

/// \brief default logger line prefix ([12:59:59][AppName][  level] log line...)
static constexpr std::string_view DEFAULT_LOG_PATTERN         = "[%H:%M:%S][%n][%7l] %v";

/// \brief default console logger line prefix ([12:59:59][AppName][  level] log line...)
static constexpr std::string_view DEFAULT_CONSOLE_LOG_PATTERN = "[%H:%M:%S][%n]%^[%7l] %$%v";
} // namespace ini

#endif // SERVER_SHAREDSERVER_LOGGER_H

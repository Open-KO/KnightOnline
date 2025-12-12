#ifndef SERVER_SHAREDSERVER_LOGGER_H
#define SERVER_SHAREDSERVER_LOGGER_H

#pragma once

#include <filesystem>
#include <memory>
#include <string>

// forward declarations
class CIni;

namespace spdlog::details
{
	class thread_pool;
}

namespace ftxui
{
	class sink_mt;
}

namespace logger
{

	class Logger
	{
		// setup defaults
		static constexpr uint16_t MessageQueueSize = 8192;
		static constexpr uint8_t ThreadPoolSize = 1;

	public:
		std::shared_ptr<ftxui::sink_mt> FxtuiSink()
		{
			return _fxtuiSink;
		}

		const std::string& AppName() const
		{
			return _appName;
		}

		/// \param appName application name (VersionManager, Aujard, AIServer, Ebenezer)
		Logger(const std::string& appName);

		/// \brief Sets up spdlog from an ini file using standardized server settings
		/// \param ini server application's ini file (already loaded)
		/// \param baseDir base directory to store logs folder under
		void Setup(CIni& ini, const std::filesystem::path& baseDir);

		virtual void SetupExtraLoggers(CIni& ini,
			std::shared_ptr<spdlog::details::thread_pool> threadPool,
			const std::filesystem::path& baseDir);

		void SetupExtraLogger(CIni& ini,
			std::shared_ptr<spdlog::details::thread_pool> threadPool,
			const std::filesystem::path& baseDir,
			const std::string& appName, const std::string& logFileConfigProp);

		virtual ~Logger();

	protected:
		std::string _appName;
		std::string _defaultLogPath;

		std::shared_ptr<ftxui::sink_mt> _fxtuiSink;
	};

	//
	// Application names used by our loggers
	//

	// AI Server
	static constexpr char AIServer[] = "AIServer";
	static constexpr char AIServerItem[] = "AIServerItem";
	static constexpr char AIServerUser[] = "AIServerUser";

	// Ebenezer
	static constexpr char Ebenezer[] = "Ebenezer";
	static constexpr char EbenezerEvent[] = "EbenezerEvent";
	static constexpr char EbenezerRegion[] = "EbenezerRegion";

	// Aujard
	static constexpr char Aujard[] = "Aujard";

	// Version Manager
	static constexpr char VersionManager[] = "VersionManager";

	// Item Manager
	static constexpr char ItemManager[] = "ItemManager";
	static constexpr char ItemManagerItem[] = "ItemManagerItem";
	static constexpr char ItemManagerExp[] = "ItemManagerExp";
}

namespace ini
{
	// LOGGER section
	static constexpr char LOGGER[] = "LOGGER";
	static constexpr char LEVEL[] = "LEVEL";
	static constexpr char PATTERN[] = "PATTERN";
	static constexpr char CONSOLE_PATTERN[] = "CONSOLE_PATTERN";
	static constexpr char FILE[] = "FILE";
	static constexpr char ITEM_LOG_FILE[] = "ITEM_LOG_FILE";
	static constexpr char USER_LOG_FILE[] = "USER_LOG_FILE";
	static constexpr char REGION_LOG_FILE[] = "REGION_LOG_FILE";
	static constexpr char EVENT_LOG_FILE[] = "EVENT_LOG_FILE";
	static constexpr char EXP_LOG_FILE[] = "EXP_LOG_FILE";

	/// \brief default logger line prefix ([12:59:59][AppName][  level] log line...)
	static constexpr char DEFAULT_LOG_PATTERN[] = "[%H:%M:%S][%n][%7l] %v";

	/// \brief default console logger line prefix ([12:59:59][AppName][  level] log line...)
	static constexpr char DEFAULT_CONSOLE_LOG_PATTERN[] = "[%H:%M:%S][%n][%^%7l%$] %v";
}

#endif // SERVER_SHAREDSERVER_LOGGER_H

#include "stdafx.h"
#include "logger.h"

#include <shared/Ini.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/common.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async.h>
#include <spdlog/async_logger.h>

#include <filesystem>

logger::Logger::Logger(const std::string& appName)
	: _appName(appName)
{
	_defaultLogPath = fmt::format("logs/{}.log", appName);
}

/// \brief Sets up spdlog from an ini file using standardized server settings
/// \param ini server application's ini file (already loaded)
/// \param baseDir base directory to store logs folder under
void logger::Logger::Setup(CIni& ini, const std::string& baseDir)
{
	// setup file logger
	std::string fileName = ini.GetString(ini::LOGGER, ini::FILE, _defaultLogPath);

	std::filesystem::path configuredLogPath(fileName);
	if (configuredLogPath.is_relative())
		configuredLogPath = std::filesystem::path(baseDir) / fileName;

	std::u8string utf8String = configuredLogPath.u8string();
	fileName.assign(utf8String.begin(), utf8String.end());

	auto fileLogger = std::make_shared<spdlog::sinks::daily_file_format_sink_mt>(fileName, 0, 0);

	// setup console logger
	auto consoleLogger = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

	std::string logPattern = ini.GetString(ini::LOGGER, ini::PATTERN, ini::DEFAULT_LOG_PATTERN);
	fileLogger->set_pattern(logPattern);

	std::string consoleLogPattern = ini.GetString(ini::LOGGER, ini::CONSOLE_PATTERN, ini::DEFAULT_CONSOLE_LOG_PATTERN);
	consoleLogger->set_pattern(consoleLogPattern);

	spdlog::init_thread_pool(MessageQueueSize, ThreadPoolSize);

	// setup multi-sink async logger as default (combines file+console logger)
	spdlog::sinks_init_list sinks = { fileLogger, consoleLogger };

	auto threadPool = spdlog::thread_pool();
	auto appLogger = std::make_shared<spdlog::async_logger>(
		_appName, sinks.begin(), sinks.end(), threadPool, spdlog::async_overflow_policy::block);
	spdlog::set_default_logger(appLogger);

	// set default logger level and pattern
	// we default to debug level logging
	int logLevel = ini.GetInt(ini::LOGGER, ini::LEVEL, spdlog::level::debug);
	spdlog::set_level(static_cast<spdlog::level::level_enum>(logLevel));

	// add any extra loggers
	SetupExtraLoggers(ini, threadPool, baseDir);

	// periodically flush all *registered* loggers every 3 seconds:
	// warning: only use if all your loggers are thread-safe ("_mt" loggers)
	spdlog::flush_every(std::chrono::seconds(3));

	spdlog::info("{} logger configured", _appName);
}

void logger::Logger::SetupExtraLoggers(CIni& ini,
	std::shared_ptr<spdlog::details::thread_pool> threadPool,
	const std::string& baseDir)
{
	/* do nothing - consumers will implement this */
	ini;
	threadPool;
	baseDir;
}

void logger::Logger::SetupExtraLogger(CIni& ini,
	std::shared_ptr<spdlog::details::thread_pool> threadPool,
	const std::string& baseDir,
	const std::string& appName, const std::string& logFileConfigProp)
{
	// setup file logger
	std::string fileName = ini.GetString(ini::LOGGER, logFileConfigProp, _defaultLogPath);

	std::filesystem::path configuredLogPath(fileName);
	if (configuredLogPath.is_relative())
		configuredLogPath = std::filesystem::path(baseDir) / fileName;

	std::u8string utf8String = configuredLogPath.u8string();
	fileName.assign(utf8String.begin(), utf8String.end());

	auto fileLogger = std::make_shared<spdlog::sinks::daily_file_format_sink_mt>(fileName, 0, 0);

	// setup console logger
	auto consoleLogger = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

	std::string logPattern = ini.GetString(ini::LOGGER, ini::PATTERN, ini::DEFAULT_LOG_PATTERN);
	fileLogger->set_pattern(logPattern);

	std::string consoleLogPattern = ini.GetString(ini::LOGGER, ini::CONSOLE_PATTERN, ini::DEFAULT_CONSOLE_LOG_PATTERN);
	consoleLogger->set_pattern(consoleLogPattern);

	// setup multi-sink async logger as default (combines file+console logger)
	spdlog::sinks_init_list sinks = { fileLogger, consoleLogger };
	auto extraLogger = std::make_shared<spdlog::async_logger>(
		appName, sinks.begin(), sinks.end(), threadPool, spdlog::async_overflow_policy::block);

	// set default logger level and pattern
	// we default to debug level logging
	int logLevel = ini.GetInt(ini::LOGGER, ini::LEVEL, spdlog::level::debug);
	extraLogger->set_level(static_cast<spdlog::level::level_enum>(logLevel));

	// register the logger
	spdlog::register_logger(extraLogger);
	spdlog::get(appName)->info("{} logger configured", appName);
}

logger::Logger::~Logger()
{
}

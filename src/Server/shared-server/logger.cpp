#include "pch.h"
#include "logger.h"

#include <shared/Ini.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/common.h>
#include <spdlog/async.h>
#include <spdlog/async_logger.h>

logger::Logger::Logger(const std::string_view appName) : _appName(appName)
{
	_defaultLogPath = fmt::format("logs/{}.log", appName);
}

void logger::Logger::Setup(CIni& ini, const std::filesystem::path& baseDir)
{
	// setup file logger
	std::string fileName = ini.GetString(ini::LOGGER, ini::FILE, _defaultLogPath);

	std::filesystem::path configuredLogPath(fileName);
	if (configuredLogPath.is_relative())
		configuredLogPath = baseDir / fileName;

	std::u8string utf8String = configuredLogPath.u8string();
	fileName.assign(utf8String.begin(), utf8String.end());

	auto fileLogger = std::make_shared<spdlog::sinks::daily_file_format_sink_mt>(fileName, 0, 0);

	std::string logPattern = ini.GetString(ini::LOGGER, ini::PATTERN, ini::DEFAULT_LOG_PATTERN);
	fileLogger->set_pattern(logPattern);

	// setup console logger
	_consoleLogger                = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

	std::string consoleLogPattern = ini.GetString(
		ini::LOGGER, ini::CONSOLE_PATTERN, ini::DEFAULT_CONSOLE_LOG_PATTERN);
	_consoleLogger->set_pattern(consoleLogPattern);

	spdlog::init_thread_pool(MessageQueueSize, ThreadPoolSize);

	// setup multi-sink async logger as default (combines file + console logger)
	spdlog::sinks_init_list sinks = { fileLogger, _consoleLogger };

	auto threadPool               = spdlog::thread_pool();
	auto appLogger                = std::make_shared<spdlog::async_logger>(
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

void logger::Logger::SetupExtraLoggers(CIni& /*ini*/,
	// NOLINTNEXTLINE(performance-unnecessary-value-param)
	std::shared_ptr<spdlog::details::thread_pool> /*threadPool*/,
	const std::filesystem::path& /*baseDir*/)
{
	/* do nothing - consumers will implement this */
}

void logger::Logger::SetupExtraLogger(CIni& ini,
	// NOLINTNEXTLINE(performance-unnecessary-value-param): threadPool
	std::shared_ptr<spdlog::details::thread_pool> threadPool, const std::filesystem::path& baseDir,
	const std::string_view appName, const std::string_view logFileConfigProp)
{
	// setup file logger
	std::string fileName = ini.GetString(ini::LOGGER, logFileConfigProp, _defaultLogPath);

	std::filesystem::path configuredLogPath(fileName);
	if (configuredLogPath.is_relative())
		configuredLogPath = baseDir / fileName;

	std::u8string utf8String = configuredLogPath.u8string();
	fileName.assign(utf8String.begin(), utf8String.end());

	auto fileLogger = std::make_shared<spdlog::sinks::daily_file_format_sink_mt>(fileName, 0, 0);

	std::string logPattern = ini.GetString(ini::LOGGER, ini::PATTERN, ini::DEFAULT_LOG_PATTERN);
	fileLogger->set_pattern(logPattern);

	// setup multi-sink async logger as default (combines file + console logger)
	spdlog::sinks_init_list sinks = { fileLogger, _consoleLogger };
	auto extraLogger = std::make_shared<spdlog::async_logger>(std::string(appName), sinks.begin(),
		sinks.end(), threadPool, spdlog::async_overflow_policy::block);

	// set default logger level and pattern
	// we default to debug level logging
	int logLevel     = ini.GetInt(ini::LOGGER, ini::LEVEL, spdlog::level::debug);
	extraLogger->set_level(static_cast<spdlog::level::level_enum>(logLevel));

	// register the logger
	spdlog::register_logger(extraLogger);
	extraLogger->info("{} logger configured", appName);
}

logger::Logger::~Logger()
{
}

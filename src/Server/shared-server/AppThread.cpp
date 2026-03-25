#include "pch.h"
#include "AppThread.h"
#include "TelnetThread.h"
#include "utilities.h"

#include <shared/Ini.h>

#include <argparse/argparse.hpp>
#include <spdlog/spdlog.h>

#include <csignal>
#include <string>
#include <vector>

AppThread* AppThread::s_instance = nullptr;
bool AppThread::s_shutdown       = false;

AppThread::AppThread(logger::Logger& logger) : _logger(logger)
{
	assert(s_instance == nullptr);
	s_instance = this;

	_iniFile   = new CIni();
}

AppThread::~AppThread()
{
	delete _iniFile;

	if (_telnetThread != nullptr)
	{
		spdlog::debug("AppThread::~AppThread: Shutting down telnet thread.");
		_telnetThread->shutdown();
		spdlog::debug("AppThread::~AppThread: Telnet thread shut down.");
	}

	assert(s_instance != nullptr);
	s_instance = nullptr;
}

std::filesystem::path AppThread::LogBaseDir() const
{
	return std::filesystem::current_path();
}

void AppThread::before_shutdown()
{
	_appStatus = AppStatus::STOPPING;
}

bool AppThread::parse_commandline(int argc, char* argv[])
{
	argparse::ArgumentParser parser(_logger.AppName());

	SetupCommandLineArgParser(parser);

	try
	{
		parser.parse_args(argc, argv);
		return ProcessCommandLineArgs(parser);
	}
	catch (const std::exception& ex)
	{
		spdlog::error("AppThread::parse_commandline: {}", ex.what());
		return false;
	}
}

void AppThread::SetupCommandLineArgParser(argparse::ArgumentParser& parser)
{
	parser.add_argument("--headless")
		.help("run in headless mode, without the ftxui terminal UI for input")
		.flag()
		.store_into(_headless);
	parser.add_argument("--enable-telnet")
		.help("enables the telnet command server, overriding config")
		.flag()
		.store_into(_overrideEnableTelnet);
	parser.add_argument("--telnet-address")
		.help("sets the address for the telnet command server to listen on, overriding config")
		.store_into(_overrideTelnetAddress);
}

bool AppThread::ProcessCommandLineArgs(const argparse::ArgumentParser& /*parser*/)
{
	/* for implementation, only if needed by the app - bound args won't need this */
	return true;
}

void AppThread::thread_loop()
{
	CIni& iniFile         = IniFile();
	bool configFileLoaded = iniFile.Load(ConfigPath());

	// Setup the logger
	_logger.Setup(iniFile, LogBaseDir());

	if (!configFileLoaded)
	{
		std::u8string filenameUtf8 = iniFile.GetPath().u8string();
		std::string filename(filenameUtf8.begin(), filenameUtf8.end());

		spdlog::warn(
			"AppThread::thread_loop: {} does not exist, will use configured defaults.", filename);
	}

	_exitCode = thread_loop_impl(iniFile);
}

int AppThread::thread_loop_impl(CIni& iniFile)
{
	int exitCode = EXIT_SUCCESS;
	if (!StartupImpl(iniFile))
	{
		exitCode = EXIT_FAILURE;
		shutdown(false);
	}

	while (CanTick())
	{
		std::unique_lock<std::mutex> lock(ThreadMutex());
		ThreadCondition().wait(lock);
	}

	return exitCode;
}

bool AppThread::LoadConfig(CIni& /*iniFile*/)
{
	return true;
}

bool AppThread::StartupImpl(CIni& iniFile)
{
	try
	{
		// Load application-specific config
		if (!LoadConfig(iniFile))
		{
			spdlog::error("AppThread::StartupImpl: LoadConfig() failed.");
			return false;
		}

		// Load Telnet config
		_enableTelnet         = iniFile.GetBool("TELNET", "ENABLED", _enableTelnet);
		_telnetAddress        = iniFile.GetString("TELNET", "IP", _telnetAddress);
		_telnetPort           = iniFile.GetInt("TELNET", "PORT", _telnetPort);
		std::string whitelist = iniFile.GetString("TELNET", "WHITELIST", "127.0.0.1");

		if (!_overrideTelnetAddress.empty())
			_telnetAddress = _overrideTelnetAddress;

		// Trigger a save to flush defaults to file.
		iniFile.Save();

		// Start the Telnet Server, if enabled
		if (_enableTelnet || _overrideEnableTelnet)
		{
			std::unordered_set<std::string> addressWhiteList;
			std::stringstream ss(whitelist);
			std::string value;
			while (std::getline(ss, value, ','))
				addressWhiteList.insert(value);

			_telnetThread = new TelnetThread(
				_telnetAddress, _telnetPort, std::move(addressWhiteList));

			spdlog::info("AppThread::StartupImpl: Telnet thread starting on {}:{}", _telnetAddress,
				_telnetPort);
			_telnetThread->start();
		}

		// Load application-specific startup logic
		_appStatus = AppStatus::STARTING;

		if (!OnStart())
		{
			spdlog::error("AppThread::StartupImpl: OnStart() failed.");
			return false;
		}

		_appStatus = AppStatus::READY;
		return true;
	}
	catch (const std::exception& ex)
	{
		spdlog::error("AppThread::StartupImpl: unhandled exception - {}", ex.what());
		return false;
	}
}

void AppThread::ParseCommand(const std::string& command)
{
	if (command.empty())
		return;

	if (HandleCommand(command))
		spdlog::info("Command handled: {}", command);
	else
		spdlog::warn("Command not handled: {}", command);
}

bool AppThread::HandleCommand(const std::string& command)
{
	if (command == "/exit")
	{
		shutdown(false);
		return true;
	}

	return false;
}

void AppThread::catchInterruptSignals()
{
	// catch interrupt signals for graceful shutdowns.
	signal(SIGINT, signalHandler);
	signal(SIGABRT, signalHandler);
	signal(SIGTERM, signalHandler);
}

void AppThread::signalHandler(int signalNumber)
{
	spdlog::info("AppThread::signalHandler: Caught {}", signalNumber);

	switch (signalNumber)
	{
		case SIGINT:
		case SIGABRT:
		case SIGTERM:
			// Shutdown the application thread
			if (!s_shutdown && s_instance != nullptr)
			{
				s_instance->shutdown(false);
				s_shutdown = true;
			}
			break;

		default:
			break;
	}

	signal(signalNumber, signalHandler);
}

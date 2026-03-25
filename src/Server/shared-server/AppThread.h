#ifndef SERVER_SHAREDSERVER_APPTHREAD_H
#define SERVER_SHAREDSERVER_APPTHREAD_H

#pragma once

#include <shared/Thread.h>
#include "logger.h"

#include <memory>

namespace argparse
{
class ArgumentParser;
}

/// \brief Possible application states.  Used for health checks
enum class AppStatus : uint8_t
{
	INITIALIZING, ///< Initial state for an application, has not started loading resources
	STARTING,     ///< Loading resources, not ready for connections
	READY,        ///< Resources loaded, ready for connection
	STOPPING      ///< Shutdown underway
};

class CIni;
class TelnetThread;

class AppThread : public Thread
{
public:
	static AppThread* instance()
	{
		return s_instance;
	}

	int ExitCode() const
	{
		return _exitCode;
	}

	CIni& IniFile()
	{
		return *_iniFile;
	}

	/// \brief Retrieves the application's current status
	AppStatus GetAppStatus() const
	{
		return _appStatus;
	}

	AppThread(logger::Logger& logger);
	~AppThread() override;

	/// \returns The base directory for logs. By default this is the working directory.
	virtual std::filesystem::path LogBaseDir() const;

	/// \returns The application's ini config path.
	virtual std::filesystem::path ConfigPath() const = 0;

	/// \brief Sets the application status to STOPPING when shutdown is triggered
	void before_shutdown() override;

private:
	/// \brief Sets up the parser & parses the command-line args, dispatching it to the app
	bool parse_commandline(int argc, char* argv[]);

protected:
	/// \brief Sets up the command-line arg parser, binding args for parsing.
	virtual void SetupCommandLineArgParser(argparse::ArgumentParser& parser);

	/// \brief Processes any parsed command-line args as needed by the app.
	/// \returns true on success, false on failure
	virtual bool ProcessCommandLineArgs(const argparse::ArgumentParser& parser);

	/// \brief The main thread loop for the server instance
	void thread_loop() override;

private:
	/// \brief Main thread loop implementation logic.
	/// \param iniFile The loaded application ini file.
	/// \returns Exit code.
	int thread_loop_impl(CIni& iniFile);

protected:
	/// \brief Loads application-specific config from the loaded application ini file (`iniFile`).
	/// \param iniFile The loaded application ini file.
	/// \returns true when successful, false otherwise
	virtual bool LoadConfig(CIni& iniFile);

private:
	/// \brief Wraps the main startup logic so that error handling can be shared by the caller.
	/// \param iniFile The loaded application ini file.
	/// \returns true when successful, false otherwise
	bool StartupImpl(CIni& iniFile);

public:
	/// \brief Initializes the server, loading caches, socket managers, etc.
	/// \returns true when successful, false otherwise
	virtual bool OnStart() = 0;

	/// \brief Parses command input from the console and dispatches command handling via HandleCommand.
	/// \see HandleCommand
	virtual void ParseCommand(const std::string& command);

	/// \brief Handles command processing.
	/// \returns true when handled, false otherwise
	virtual bool HandleCommand(const std::string& command);

	template <typename AppThreadType, typename LoggerType>
	static int main(int argc, char* argv[], LoggerType& logger)
	{
		// catch interrupt signals for graceful shutdowns.
		catchInterruptSignals();

		// Logger config/setup is handled by the server instance.
		// We just instantiate it early for signal handling.
		auto appThread = std::make_unique<AppThreadType>(logger);
		if (appThread == nullptr)
			return 1;

		if (!appThread->parse_commandline(argc, argv))
			return 1;

		appThread->start();

		// We keep the main() thread alive to catch interrupt signals and call shutdown
		appThread->join();

		return appThread->ExitCode();
	}

private:
	static void catchInterruptSignals();
	static void signalHandler(int signalNumber);

	CIni* _iniFile              = nullptr;
	TelnetThread* _telnetThread = nullptr;

protected:
	logger::Logger& _logger;

	int _exitCode                      = 0; // EXIT_SUCCESS
	bool _headless                     = true;

	/// \brief Indicates if telnet was enabled through config or default constructor override
	bool _enableTelnet                 = false;

	/// \brief Indicates if --enable-telnet was passed as a command line argument. Overrides _enableTelnet
	bool _overrideEnableTelnet         = false;

	/// \brief Listen address for the Telnet server
	std::string _telnetAddress         = "127.0.0.1";

	/// \brief Listen address for the Telnet server. Overrides _telnetAddress
	std::string _overrideTelnetAddress = {};

	/// \brief Listen port for the Telnet server
	uint16_t _telnetPort               = 2323;

	/// \brief Application status used for health checks
	AppStatus _appStatus               = AppStatus::INITIALIZING;

	static AppThread* s_instance;
	static bool s_shutdown;
};

#endif // SERVER_SHAREDSERVER_APPTHREAD_H

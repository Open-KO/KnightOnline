#ifndef SERVER_SHAREDSERVER_APPTHREAD_H
#define SERVER_SHAREDSERVER_APPTHREAD_H

#pragma once

#include <shared/Thread.h>
#include "logger.h"

#include <ftxui/component/event.hpp>

class CIni;
class AppThread : public Thread
{
public:
	int ExitCode() const
	{
		return _exitCode;
	}

	CIni& IniFile()
	{
		return *_iniFile;
	}

	AppThread(logger::Logger& logger);
	~AppThread();

	/// \returns The base directory for logs. By default this is the working directory.
	virtual std::filesystem::path LogBaseDir() const;

	/// \returns The application's ini config path.
	virtual std::filesystem::path ConfigPath() const = 0;

	/// \brief The main thread loop for the server instance
	void thread_loop() override;

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

	/// \brief Handles input events from the UI
	/// \returns true when handled, false otherwise
	virtual bool HandleInputEvent(const ftxui::Event& event);

	/// \brief Parses command input from the console and dispatches command handling via HandleCommand.
	/// \see HandleCommand
	virtual void ParseCommand(const std::string& command);

	/// \brief Handles command processing.
	/// \returns true when handled, false otherwise
	virtual bool HandleCommand(const std::string& command);

	template <typename AppThreadType, typename LoggerType>
	static int main(LoggerType& logger)
	{
		// catch interrupt signals for graceful shutdowns.
		catchInterruptSignals();

		// Logger config/setup is handled by the server instance.
		// We just instantiate it early for signal handling.
		AppThreadType appThread(logger);
		appThread.start();

		// We keep the main() thread alive to catch interrupt signals and call shutdown
		appThread.join();

		return appThread.ExitCode();
	}

private:
	static void catchInterruptSignals();
	static void signalHandler(int signalNumber);

private:
	CIni* _iniFile;

protected:
	logger::Logger&		_logger;
	int					_exitCode;

	static AppThread*	s_instance;
	static bool			s_shutdown;
};

#endif // SERVER_SHAREDSERVER_APPTHREAD_H

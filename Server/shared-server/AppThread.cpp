#include "pch.h"
#include "AppThread.h"
#include "ftxui_sink_mt.h"
#include "utilities.h"

#include <shared/Ini.h>

#include <spdlog/spdlog.h>

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/loop.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <signal.h>
#include <stdlib.h>
#include <string>
#include <vector>

AppThread* AppThread::s_instance = nullptr;
bool AppThread::s_shutdown = false;

AppThread::AppThread(logger::Logger& logger)
	: _logger(logger), _exitCode(EXIT_SUCCESS)
{
	assert(s_instance == nullptr);
	s_instance = this;

	_iniFile = new CIni();
}

AppThread::~AppThread()
{
	delete _iniFile;

	assert(s_instance != nullptr);
	s_instance = nullptr;
}

std::filesystem::path AppThread::LogBaseDir() const
{
	return GetProgPath();
}

/// \brief The main thread loop for the server instance
void AppThread::thread_loop()
{
	int exitCode = EXIT_SUCCESS;

	CIni& iniFile = IniFile();
	bool configFileLoaded = iniFile.Load(ConfigPath());

	// Setup the logger
	_logger.Setup(iniFile, LogBaseDir());

	if (!configFileLoaded)
	{
		std::u8string filenameUtf8 = iniFile.GetPath().u8string();
		std::string filename(filenameUtf8.begin(), filenameUtf8.end());

		spdlog::warn("AppThread::thread_loop: {} does not exist, will use configured defaults.",
			filename);
	}

	using namespace ftxui;

	bool isServerLoaded = false;
	auto fxtuiSink = _logger.fxtuiSink();

	std::string inputText;
	Elements logElements;

	int focusedLineNumber = 0;
	bool autoScroll = true;
	int elementCount = 0, lastElementIndex = 0;

	auto input = Input(&inputText, "Enter command...");

	input |= CatchEvent([&](Event event)
	{
		if (event == Event::Return
			&& isServerLoaded)
		{
			ParseCommand(inputText);
			inputText.clear();
			return true;
		}

		return false;
	});

	auto renderer = Renderer(input, [&]
	{
		{
			std::lock_guard<std::mutex> lock(fxtuiSink->lock());
			logElements = fxtuiSink->log_buffer(); // this is intentionally a copy, but it's a container of shared pointers
		}

		// clamping
		int oldElementCount = elementCount;
		elementCount = static_cast<int>(logElements.size());
		lastElementIndex = std::max(0, elementCount - 1);
		focusedLineNumber = std::clamp(focusedLineNumber, 0, lastElementIndex);

		float scrollPosition = 0.0f;
		if (elementCount > 0)
			scrollPosition = std::clamp(static_cast<float>(focusedLineNumber) / static_cast<float>(elementCount), 0.0f, 1.0f);

		// Auto-scroll to bottom when new lines are added
		if (autoScroll && oldElementCount != elementCount)
		{
			focusedLineNumber = lastElementIndex;
			scrollPosition = 1.0f;
	 	}

		// render
		auto logDisplay = vbox(logElements)
			| focusPositionRelative(0, scrollPosition)
			| vscroll_indicator
			| frame
			| flex;

		auto inputBox = hbox({
			text(" Command: ") | bold,
			input->Render() | flex
		}) | border;

		if (!isServerLoaded)
			return logDisplay;

		return vbox({
			logDisplay,
			inputBox
		});
	});

	constexpr int PageSize	= 10;
	constexpr int WheelSize	= 3;

	renderer |= CatchEvent([&](Event event)
	{
		// Keyboard events
		if (event == Event::ArrowUp)
		{
			--focusedLineNumber;
			return true;
		}

		if (event == Event::ArrowDown)
		{
			++focusedLineNumber;

			if (focusedLineNumber >= lastElementIndex)
				autoScroll = true;
			return true;
		}

		if (event == Event::PageUp)
		{
			focusedLineNumber -= PageSize;
			return true;
		}

		if (event == Event::PageDown)
		{
			focusedLineNumber += PageSize;

			if (focusedLineNumber >= lastElementIndex)
				autoScroll = true;
			return true;
		}

		if (event == Event::Home)
		{
			focusedLineNumber = 0;
			return true;
		}

		if (event == Event::End)
		{
			focusedLineNumber = std::max(0, elementCount - 1);
			autoScroll = true;
			return true;
		}

		// Mouse events
		if (event.is_mouse())
		{
			if (event.mouse().button == Mouse::WheelUp)
			{
				focusedLineNumber -= WheelSize;
				return true;
			}
			
			if (event.mouse().button == Mouse::WheelDown)
			{
				focusedLineNumber += WheelSize;

				if (focusedLineNumber >= lastElementIndex)
					autoScroll = true;
				return true;
			}
		}

		return HandleInputEvent(event);
	});

	auto screen = ScreenInteractive::Fullscreen();

	std::thread uiThread([&]
	{
		fxtuiSink->set_screen(&screen);
		screen.Loop(renderer);
		fxtuiSink->set_screen(nullptr);

		shutdown(false);
	});

	if (StartupImpl(iniFile))
	{
		isServerLoaded = true;
	}
	else
	{
		exitCode = EXIT_FAILURE;
		shutdown(false);
	}

	while (_canTick)
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_cv.wait(lock);
	}

	screen.Exit();

	if (uiThread.joinable())
		uiThread.join();

	_exitCode = exitCode;
}

/// \brief Loads application-specific config from the loaded application ini file (`iniFile`).
/// \param iniFile The loaded application ini file.
/// \returns true when successful, false otherwise
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

		// Trigger a save to flush defaults to file.
		iniFile.Save();

		// Load application-specific startup logic
		if (!OnStart())
		{
			spdlog::error("AppThread::StartupImpl: OnStart() failed.");
			return false;
		}

		return true;
	}
	catch (const std::exception& ex)
	{
		spdlog::error("AppThread::StartupImpl: unhandled exception - {}", ex.what());
		return false;
	}
}

bool AppThread::HandleInputEvent(const ftxui::Event& /*event*/)
{
	return false;
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
	if (command == "/clear")
	{
		auto fxtuiSink = _logger.fxtuiSink();
		if (fxtuiSink != nullptr)
		{
			std::lock_guard<std::mutex> lock(fxtuiSink->lock());
			fxtuiSink->log_buffer().clear();
		}

		spdlog::info("Logs cleared");
		return true;
	}
	
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
			if (!s_shutdown
				&& s_instance != nullptr)
			{
				s_instance->shutdown(false);
				s_shutdown = true;
			}
			break;
	}

	signal(signalNumber, signalHandler);
}

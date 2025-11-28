#include "pch.h"
#include "AppThread.h"

#include <signal.h>
#include <stdlib.h>
#include <spdlog/spdlog.h>

AppThread* AppThread::s_instance = nullptr;
bool AppThread::s_shutdown = false;

AppThread::AppThread(logger::Logger& logger)
	: _logger(logger), _exitCode(EXIT_SUCCESS)
{
	assert(s_instance == nullptr);
	s_instance = this;
}

AppThread::~AppThread()
{
	assert(s_instance != nullptr);
	s_instance = nullptr;
}


#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <vector>
#include <string>

#include "ftxui_sink_mt.h"

/// \brief The main thread loop for the server instance
void AppThread::thread_loop()
{
	if (!OnStart())
	{
		_exitCode = EXIT_FAILURE;
		return;
	}

	using namespace ftxui;

	auto consoleLogger = _logger.consoleLogger();
    
    std::string inputText;

	int scrollPosition = 0;
	bool autoScroll = true; // TODO: ensure this gets reset

    auto input = Input(&inputText, "Enter command...");
    
    input |= CatchEvent([&](Event event)
	{
        if (event != Event::Return)
			return false;

		if (inputText.empty())
			return false;

		spdlog::info("Command: {}", inputText);

		if (inputText == "clear")
		{
			{
				std::lock_guard<std::mutex> lock(consoleLogger->lock());
				consoleLogger->log_buffer().clear();
			}

			spdlog::info("Logs cleared");
		}
		else if (inputText == "test")
		{
			for (int i = 0; i < 10; i++)
				spdlog::info("Test {}", i + 1);
		}

		inputText.clear();
		return true;
    });
    
    auto renderer = Renderer(input, [&]
	{
		Elements logElements;
		size_t entryCount = 0;

		{
			std::lock_guard<std::mutex> lock(consoleLogger->lock());
			entryCount = consoleLogger->log_buffer().size();
			for (const ColoredLog& log : consoleLogger->log_buffer())
			{
				auto logLine = hbox({
					text(log.Prefix),
					text(log.LevelText) | color(log.color()),
					text(log.Suffix)
				});

				logElements.push_back(logLine);
			}
		}

		// Auto-scroll to bottom
		if (autoScroll
			&& entryCount > 0)
		{
			// TODO: 
			scrollPosition = std::max(0, static_cast<int>(entryCount) - 20);
		}

		auto logDisplay = vbox(logElements)
			| focusPositionRelative(0, static_cast<float>(scrollPosition))
			| vscroll_indicator
			| frame
			| flex;

		auto inputBox = hbox({
			text(" Command: ") | bold,
			input->Render() | flex
		}) | border;

		return vbox({
			logDisplay,
			inputBox
		});
    });

	constexpr int PageSize	= 10;
	constexpr int WheelSize	= 3;

	renderer |= CatchEvent([&](Event event)
	{
		if (event == Event::ArrowUp)
		{
			scrollPosition = std::max(0, scrollPosition - 1);
			autoScroll = false;
			return true;
		}

		if (event == Event::ArrowDown)
		{
			++scrollPosition;
			autoScroll = false;
			return true;
		}

		if (event == Event::PageUp)
		{
			scrollPosition = std::max(0, scrollPosition - PageSize);
			autoScroll = false;
			return true;
		}

		if (event == Event::PageDown)
		{
			scrollPosition += PageSize;
			autoScroll = false;
			return true;
		}

		if (event.is_mouse()
			&& event.mouse().button == Mouse::WheelUp)
		{
			scrollPosition = std::max(0, scrollPosition - WheelSize);
			autoScroll = false;
			return true;
		}

		if (event.is_mouse()
			&& event.mouse().button == Mouse::WheelDown)
		{
			scrollPosition += WheelSize;
			autoScroll = false;
			return true;
		}

		return false;
	});

	auto screen = ScreenInteractive::Fullscreen();

	consoleLogger->set_screen(&screen);

	// TODO: Couple this to our main loop. This is just for testing.
	screen.Loop(renderer);

	consoleLogger->set_screen(nullptr);

#if 0
	while (_canTick)
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_cv.wait(lock);
	}
#endif

	_exitCode = EXIT_SUCCESS;
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

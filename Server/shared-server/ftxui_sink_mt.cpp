#include "pch.h"
#include "ftxui_sink_mt.h"

#include <ftxui/screen/color.hpp>
#include <ftxui/component/screen_interactive.hpp>

namespace ftxui
{
	static Color spdlog_level_to_fxtui_color(spdlog::level::level_enum level);

	sink_mt::sink_mt()
	{
		_screen = nullptr;
		_useStdout = false;
		_backlogSize = DEFAULT_BACKLOG_SIZE;
	}

	void sink_mt::set_screen(ScreenInteractive* screen)
	{
		auto previousScreen = _screen;
		_screen = screen;

		if (previousScreen != nullptr
			&& screen == nullptr)
			_useStdout = true;
	}

	void sink_mt::set_backlog_size(size_t backlogSize)
	{
		_backlogSize = backlogSize;

		if (backlogSize != 0)
		{
			std::lock_guard<std::mutex> lock(_logBufferMutex);

			if (_logBuffer.size() > backlogSize)
			{
				size_t entriesOverLimit = _logBuffer.size() - backlogSize;

				// Remove the first (oldest) entries from the buffer.
				_logBuffer.erase(_logBuffer.begin(), _logBuffer.begin() + entriesOverLimit);
			}
		}
	}

	void sink_mt::sink_it_(const spdlog::details::log_msg& msg)
	{
		if (_useStdout)
			return;

		msg.color_range_start = 0;
		msg.color_range_end = 0;

		spdlog::memory_buf_t formatted;
		formatter_->format(msg, formatted);

		std::string logStr = fmt::to_string(formatted);

		// Remove any potential trailing newlines
		if (!logStr.empty())
		{
			if (logStr.back() == '\n')
				logStr.pop_back();

			if (!logStr.empty()
				&& logStr.back() == '\r')
				logStr.pop_back();
		}

		std::string_view textBeforeColor(logStr.data(), msg.color_range_start);
		std::string_view textColored(logStr.data() + msg.color_range_start, msg.color_range_end - msg.color_range_start);
		std::string_view textAfterColor(logStr.data() + msg.color_range_end, logStr.length() - msg.color_range_end);

		auto logLine = hbox({
			text(std::string(textBeforeColor)),
			text(std::string(textColored)) | color(spdlog_level_to_fxtui_color(msg.level)),
			text(std::string(textAfterColor))
		});

		{
			std::lock_guard<std::mutex> lock(_logBufferMutex);

			if (_backlogSize != 0
				&& _logBuffer.size() >= _backlogSize)
				_logBuffer.erase(_logBuffer.begin());

			_logBuffer.push_back(logLine);
		}

		// Trigger UI refresh when new log is added
		if (_screen != nullptr)
			_screen->Post(Event::Custom);
	}

	void sink_mt::flush_()
	{
	}

	static Color spdlog_level_to_fxtui_color(spdlog::level::level_enum level)
	{
		switch (level)
		{
			case spdlog::level::trace:
				return Color::White;

			case spdlog::level::debug:
				return Color::Cyan;

			case spdlog::level::info:
				return Color::Green;

			case spdlog::level::warn:
				return Color::Yellow;

			case spdlog::level::err:
				return Color::Red;

			case spdlog::level::critical:
				return Color::RedLight;

			default:
				return Color::Default;
		}
	}
}

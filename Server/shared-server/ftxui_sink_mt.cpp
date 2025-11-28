#include "pch.h"
#include "ftxui_sink_mt.h"

#include <ftxui/component/screen_interactive.hpp>

namespace ftxui
{
	sink_mt::sink_mt()
		: _screen(nullptr)
	{
	}

	void sink_mt::set_screen(ScreenInteractive* screen)
	{
		_screen = screen;
	}

	void sink_mt::sink_it_(const spdlog::details::log_msg& msg)
	{
		spdlog::memory_buf_t formatted;
		formatter_->format(msg, formatted);

		std::string logStr = fmt::to_string(formatted);

		// Remove any potential trailing newlines
		if (!logStr.empty()
			&& logStr.back() == '\n')
			logStr.pop_back();

		// TODO: Not this. But for now let's just hackily parse a string like:
		// "[12:34:56] [info] message"
		std::string prefix, levelText, suffix;

		// Find the second opening bracket (where level starts)
		size_t first_bracket = logStr.find('[');
		if (first_bracket == std::string::npos)
			return;

		size_t second_bracket = logStr.find('[', first_bracket + 1);
		if (second_bracket == std::string::npos)
			return;

		size_t level_end = logStr.find(']', second_bracket);
		if (level_end == std::string::npos)
			return;

		prefix = logStr.substr(0, second_bracket + 1);
		levelText = logStr.substr(second_bracket + 1, level_end - second_bracket - 1);
		suffix = logStr.substr(level_end);

		{
			std::lock_guard<std::mutex> lock(_logBufferMutex);
			_logBuffer.push_back({ prefix, levelText, suffix, msg.level });
		}

		// Trigger UI refresh when new log is added
		if (_screen != nullptr)
			_screen->Post(Event::Custom);
	}

	void sink_mt::flush_()
	{
	}

	Color ColoredLog::color() const
	{
		switch (Level)
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

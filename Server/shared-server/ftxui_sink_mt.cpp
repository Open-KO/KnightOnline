#include "pch.h"
#include "ftxui_sink_mt.h"

#include <ftxui/component/screen_interactive.hpp>

namespace ftxui
{
	sink_mt::sink_mt()
	{
		_screen = nullptr;
		_useStdout = false;
		_bufferSize = DEFAULT_BUFFER_SIZE;
	}

	void sink_mt::set_screen(ScreenInteractive* screen)
	{
		auto previousScreen = _screen;
		_screen = screen;

		if (previousScreen != nullptr
			&& screen == nullptr)
			_useStdout = true;
	}

	void sink_mt::set_buffer_size(size_t bufferSize)
	{
		_bufferSize = bufferSize;

		if (_bufferSize != 0)
		{
			std::lock_guard<std::mutex> lock(_logBufferMutex);

			if (_logBuffer.size() > _bufferSize)
			{
				size_t entriesOverLimit = _logBuffer.size() - _bufferSize;

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

		{
			std::lock_guard<std::mutex> lock(_logBufferMutex);

			if (_bufferSize != 0
				&& _logBuffer.size() >= _bufferSize)
				_logBuffer.pop_front();

			_logBuffer.push_back({ logStr, msg.color_range_start, msg.color_range_end, msg.level });
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

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
		_useConsoleSink = true;
		_storeLogBuffer = true;
		_backlogSize = DEFAULT_BACKLOG_SIZE;
	}

	void sink_mt::set_screen(ScreenInteractive* screen)
	{
		auto previousScreen = _screen;

		_screen = screen;

		if (previousScreen != nullptr
			&& screen == nullptr)
		{
			disable_log_buffer();
		}
		else if (screen != nullptr)
		{
			_useConsoleSink = false;
			_storeLogBuffer = true;
		}
	}

	void sink_mt::disable_log_buffer()
	{
		using namespace spdlog;

		std::lock_guard<std::mutex> lock(_logBufferMutex);

		while (!_replayLogBuffer.empty())
		{
			const ReplayLog& replayLog = _replayLogBuffer.front();

			details::log_msg msg;
			msg.logger_name	= replayLog.logger_name;
			msg.level		= replayLog.level;
			msg.time		= replayLog.time;
			msg.thread_id	= replayLog.thread_id;
			msg.payload		= replayLog.payload;
			msg.source.line	= replayLog.source_line;

			if (!replayLog.source_filename.empty())
				msg.source.filename = replayLog.source_filename.c_str();

			if (!replayLog.source_funcname.empty())
				msg.source.funcname = replayLog.source_funcname.c_str();

			_consoleSink->log(msg);
			_replayLogBuffer.pop_front();
		}

		_useConsoleSink = true;
		_storeLogBuffer = false;
	}

	void sink_mt::set_backlog_size(size_t backlogSize)
	{
		_backlogSize = backlogSize;

		if (backlogSize != 0)
		{
			std::lock_guard<std::mutex> lock(_logBufferMutex);

			_logBuffer.reserve(backlogSize);

			if (_logBuffer.size() > backlogSize)
			{
				size_t entriesOverLimit = _logBuffer.size() - backlogSize;

				// Remove the first (oldest) entries from the buffer.
				_logBuffer.erase(_logBuffer.begin(), _logBuffer.begin() + entriesOverLimit);
			}

			if (_replayLogBuffer.size() > backlogSize)
			{
				size_t entriesOverLimit = _replayLogBuffer.size() - backlogSize;

				// Remove the first (oldest) entries from the buffer.
				_replayLogBuffer.erase(_replayLogBuffer.begin(), _replayLogBuffer.begin() + entriesOverLimit);
			}
		}
	}

	void sink_mt::set_console_sink(std::shared_ptr<spdlog::sinks::sink> consoleSink)
	{
		_consoleSink = std::move(consoleSink);
	}

	void sink_mt::sink_it_(const spdlog::details::log_msg& msg)
	{
		if (_useConsoleSink)
		{
			auto consoleSink = _consoleSink;
			if (consoleSink != nullptr)
				consoleSink->log(msg);
		}

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
			paragraph(std::string(textAfterColor))
		});

		ReplayLog replayLog;
		replayLog.level				= msg.level;
		replayLog.time				= msg.time;
		replayLog.thread_id			= msg.thread_id;
		replayLog.source_line		= msg.source.line;

		if (msg.source.filename != nullptr)
			replayLog.source_filename = msg.source.filename;

		if (msg.source.funcname != nullptr)
			replayLog.source_funcname = msg.source.funcname;

		replayLog.logger_name.assign(msg.logger_name.begin(), msg.logger_name.end());
		replayLog.payload.assign(msg.payload.begin(), msg.payload.end());

		{
			std::lock_guard<std::mutex> lock(_logBufferMutex);

			if (_backlogSize != 0)
			{
				if (_logBuffer.size() >= _backlogSize)
					_logBuffer.erase(_logBuffer.begin());

				if (_replayLogBuffer.size() >= _backlogSize)
					_replayLogBuffer.pop_front();
			}

			if (_storeLogBuffer)
				_logBuffer.push_back(logLine);

			// No need to replay entries that have already been written to the console.
			if (!_useConsoleSink)
				_replayLogBuffer.push_back(std::move(replayLog));
		}

		// Trigger UI refresh when new log is added
		if (_screen != nullptr)
			_screen->Post(Event::Custom);
	}

	void sink_mt::flush_()
	{
		if (_useConsoleSink)
		{
			auto consoleSink = _consoleSink;
			if (consoleSink != nullptr)
				consoleSink->flush();
		}
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
				return Color::YellowLight;

			case spdlog::level::err:
				return Color::Red;

			case spdlog::level::critical:
				return Color::RedLight;

			default:
				return Color::Default;
		}
	}
}

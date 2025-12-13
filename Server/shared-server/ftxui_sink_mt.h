#ifndef SERVER_SHAREDSERVER_FTXUI_SINK_MT_H
#define SERVER_SHAREDSERVER_FTXUI_SINK_MT_H

#pragma once

#include <spdlog/sinks/base_sink.h>
#include <ftxui/dom/elements.hpp>

#include <deque>

namespace ftxui
{
	struct ReplayLog
	{
		std::string						logger_name;
		spdlog::level::level_enum		level;
		spdlog::log_clock::time_point	time;
		size_t							thread_id;
		std::string						source_filename;
		int								source_line;
		std::string						source_funcname;
		std::string						payload;
	};

	class ScreenInteractive;
	class sink_mt : public spdlog::sinks::base_sink<std::mutex>
	{
	public:
		// Use a smaller buffer size 
#if defined(_DEBUG)
		static constexpr size_t DEFAULT_BACKLOG_SIZE = 100;
#else
		static constexpr size_t DEFAULT_BACKLOG_SIZE = 1000;
#endif

		sink_mt();
		void set_screen(ftxui::ScreenInteractive* screen);
		void disable_log_buffer();
		void set_backlog_size(size_t backlogSize);
		void set_console_sink(std::shared_ptr<spdlog::sinks::sink> consoleSink);

	protected:
		void sink_it_(const spdlog::details::log_msg& msg) override;
		void flush_() override;

	public:
		Elements& log_buffer()
		{
			return _logBuffer;
		}

		std::mutex& lock()
		{
			return _logBufferMutex;
		}

	private:
		Elements								_logBuffer;
		std::deque<ReplayLog>					_replayLogBuffer;
		std::mutex								_logBufferMutex;
		ftxui::ScreenInteractive*				_screen;
		bool									_useConsoleSink;
		bool									_storeLogBuffer;
		size_t									_backlogSize;
		std::shared_ptr<spdlog::sinks::sink>	_consoleSink;
	};
}

#endif // SERVER_SHAREDSERVER_FTXUI_SINK_MT_H

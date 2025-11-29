#pragma once

#include <spdlog/sinks/base_sink.h>
#include <ftxui/dom/elements.hpp>

#include <deque>

namespace ftxui
{
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
		void set_backlog_size(size_t backlogSize);

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
		Elements					_logBuffer;
		std::mutex					_logBufferMutex;
		ftxui::ScreenInteractive*	_screen;
		bool						_useStdout;
		size_t						_backlogSize;
	};
}

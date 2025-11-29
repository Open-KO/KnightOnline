#pragma once

#include <spdlog/sinks/base_sink.h>
#include <ftxui/screen/color.hpp>

#include <deque>

namespace ftxui
{
	struct ColoredLog
	{
		std::string Prefix;		// "[12:34:56] ["
		std::string LevelText;	// "info"
		std::string Suffix;		// "] message text"
		spdlog::level::level_enum Level;

		Color color() const;
	};

	class ScreenInteractive;
	class sink_mt : public spdlog::sinks::base_sink<std::mutex>
	{
	public:
		static constexpr size_t DEFAULT_BUFFER_SIZE = 1000;

		sink_mt();
		void set_screen(ftxui::ScreenInteractive* screen);
		void set_buffer_size(size_t bufferSize);
		void dump_to_console();

	protected:
		void sink_it_(const spdlog::details::log_msg& msg) override;
		void flush_() override;

	public:
		std::deque<ColoredLog>& log_buffer()
		{
			return _logBuffer;
		}

		std::mutex& lock()
		{
			return _logBufferMutex;
		}

	private:
		std::deque<ColoredLog>		_logBuffer;
		std::mutex					_logBufferMutex;
		ftxui::ScreenInteractive*	_screen;
		bool						_useStdout;
		size_t						_bufferSize;
	};
}

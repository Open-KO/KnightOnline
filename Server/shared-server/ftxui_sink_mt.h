#pragma once

#include <spdlog/sinks/base_sink.h>
#include <ftxui/screen/color.hpp>

namespace ftxui
{
	struct ColoredLog
	{
		std::string Prefix;		// "[12:34:56] ["
		std::string LevelText;  // "info"
		std::string Suffix;		// "] message text"
		spdlog::level::level_enum Level;

		Color color() const;
	};

	class ScreenInteractive;
	class sink_mt : public spdlog::sinks::base_sink<std::mutex>
	{
	public:
		sink_mt();
		void set_screen(ftxui::ScreenInteractive* screen);

	protected:
		void sink_it_(const spdlog::details::log_msg& msg) override;
		void flush_() override;

	public:
		std::vector<ColoredLog>& log_buffer()
		{
			return _logBuffer;
		}

		std::mutex& lock()
		{
			return _logBufferMutex;
		}

	private:
		std::vector<ColoredLog> _logBuffer;
		std::mutex _logBufferMutex;
		ftxui::ScreenInteractive* _screen;
	};
}

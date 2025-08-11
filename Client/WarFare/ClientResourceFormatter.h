#pragma once

// TODO: Replace CLogWriter's implementation
// #include <spdlog/spdlog.h>
#include <N3Base/LogWriter.h>

#include <spdlog/fmt/bundled/format.h>
#include <spdlog/fmt/bundled/printf.h>

#include <string>
#include <string_view>

namespace fmt
{
	namespace resource_helper
	{
		bool get_from_texts(uint32_t resourceId, std::string& fmtStr);
	}

	template <typename... Args>
	inline std::string format_text_resource(uint32_t resourceId, Args&&... args)
	{
		std::string fmtStr;

		// NOTE: Let the implementation error accordingly
		if (!resource_helper::get_from_texts(resourceId, fmtStr))
		{
			// In debug builds, we should still show useful text to highlight the problem.
			// Release builds should mimic official behaviour by returning an empty string.
#if defined(_DEBUG)
			return std::to_string(resourceId);
#else
			return {};
#endif
		}

		if constexpr (sizeof...(Args) == 0)
		{
			return fmtStr;
		}
		else
		{
			try
			{
				return fmt::sprintf(fmtStr, std::forward<Args>(args)...);
			}
			catch (const fmt::format_error&)
			{
#if defined(_N3GAME)
				CLogWriter::Write("format_text({}) failed - invalid args for format string.",
					resourceId);
#endif
			}

			// In debug builds, we should still show useful text to highlight the problem.
			// Release builds should mimic official behaviour by returning an empty string.
#if defined(_DEBUG)
			return std::to_string(resourceId);
#else
			return {};
#endif
		}
	}

	inline std::string format_text_resource(uint32_t resourceId, const std::string_view str)
	{
		return format_text_resource(resourceId, str);
	}
}

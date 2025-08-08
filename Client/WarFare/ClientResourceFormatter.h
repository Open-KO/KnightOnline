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
		if (!resource_helper::get_from_texts(resourceId, fmtStr))
		{
			CLogWriter::Write("format_text_resource(%u) failed - resource missing in Texts TBL.",
				resourceId);
			return std::to_string(resourceId);
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
				CLogWriter::Write("format_text(%u) failed - invalid args for format string.",
					resourceId);
			}

			return std::to_string(resourceId);
		}
	}

	inline std::string format_text_resource(uint32_t resourceId, const std::string_view str)
	{
		return format_text_resource(resourceId, str);
	}
}

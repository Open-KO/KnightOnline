#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/format.h>
#include <spdlog/fmt/bundled/printf.h>

#include <string>
#include <string_view>

namespace fmt
{
	namespace resource_helper
	{
		bool get_from_db(uint32_t resourceId, std::string& fmtStr);
	}

	template <typename... Args>
	inline std::string format_db_resource(uint32_t resourceId, Args&&... args)
	{
		std::string fmtStr;

		// NOTE: Let the implementation error accordingly
		if (!resource_helper::get_from_db(resourceId, fmtStr))
			return std::to_string(resourceId);

		try
		{
			return fmt::sprintf(fmtStr, std::forward<Args>(args)...);
		}
		catch (const fmt::format_error&)
		{
			spdlog::error("format_db_resource({}) failed - invalid args for format string.",
				resourceId);
		}

		return std::to_string(resourceId);
	}

	inline std::string format_db_resource(uint32_t resourceId, const std::string_view str)
	{
		return format_db_resource(resourceId, str);
	}
}

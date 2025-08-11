#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/format.h>
#include <spdlog/fmt/bundled/printf.h>

#include <string>
#include <string_view>

#include <atlconv.h>

namespace fmt
{
	namespace resource_helper
	{
		inline bool get_from_win32(uint32_t resourceId, std::string& fmtStr, const int outputCodePage = 949)
		{
			static wchar_t buffer[512];
			buffer[0] = '\0';

			// LoadStringW() returns 0 if the resource isn't found.
			if (::LoadStringW(nullptr, resourceId, buffer, _countof(buffer)) == 0)
				return false;

			fmtStr = CW2A(buffer, outputCodePage);
			return true;
		}
	}

	template <typename... Args>
	inline std::string format_win32_resource(uint32_t resourceId, Args&&... args)
	{
		std::string fmtStr;
		if (!resource_helper::get_from_win32(resourceId, fmtStr))
		{
			spdlog::error("get_from_win32({}) failed - resource not found.",
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
				spdlog::error("format_win32_resource({}) failed - invalid args for format string.",
					resourceId);
			}

			return std::to_string(resourceId);
		}
	}

	inline std::string format_win32_resource(uint32_t resourceId, const std::string_view str)
	{
		return format_win32_resource(resourceId, str);
	}
}

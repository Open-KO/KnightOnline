#ifndef CLIENT_WARFARE_CLIENTRESOURCEFORMATTER_H
#define CLIENT_WARFARE_CLIENTRESOURCEFORMATTER_H

#pragma once

// TODO: Replace CLogWriter's implementation
// #include <spdlog/spdlog.h>
#include <N3Base/LogWriter.h>

#include <spdlog/fmt/bundled/format.h>
#include <spdlog/fmt/bundled/printf.h>

#include <cassert>
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
		return {};

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
			assert(!"format_text_resource(): Invalid format string");

#if defined(_N3GAME)
			CLogWriter::Write("format_text_resource({}) failed - invalid args for format string.", resourceId);
#endif
		}

		return {};
	}
}

} // namespace fmt

#endif // CLIENT_WARFARE_CLIENTRESOURCEFORMATTER_H

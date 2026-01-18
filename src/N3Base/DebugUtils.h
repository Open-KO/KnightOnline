#ifndef CLIENT_N3BASE_DEBUGUTILS_H
#define CLIENT_N3BASE_DEBUGUTILS_H

#pragma once

#if defined(_DEBUG) || defined(DEBUG)
#include <cassert>
#include <string_view>

#include <spdlog/fmt/bundled/format.h>

void DebugStringToOutput(const std::string_view logMessage);

template <typename... Args>
static inline void FormattedDebugString(fmt::format_string<Args...> fmt, Args&&... args)
{
	DebugStringToOutput(fmt::format(fmt, std::forward<Args>(args)...));
}

#ifndef _N3TOOL
#define ASSERT assert
#define TRACE  FormattedDebugString
#endif

//	Ensure both typically used debug defines behave as intended
#ifndef DEBUG
#define DEBUG
#endif

#ifndef _DEBUG
#define _DEBUG
#endif

#else

#ifndef _N3TOOL
#define ASSERT(...)
#define TRACE(...)
#endif

#endif

#endif // CLIENT_N3BASE_DEBUGUTILS_H

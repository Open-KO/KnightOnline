#pragma once

#if defined(_DEBUG) || defined(DEBUG)
void FormattedDebugString(const char* fmt, ...);

#	include <cassert>

#	define ASSERT assert
#	define TRACE FormattedDebugString

//	Ensure both typically used debug defines behave as intended
#	ifndef DEBUG
#		define DEBUG
#	endif

#	ifndef _DEBUG
#		define _DEBUG
#	endif

#else
#	define ASSERT(...)
#	define TRACE(...)
#endif

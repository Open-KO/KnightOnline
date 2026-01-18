#ifndef SERVER_SHAREDSERVER_MY_3DSTRUCT_H
#define SERVER_SHAREDSERVER_MY_3DSTRUCT_H

#pragma once

#include <spdlog/spdlog.h>

#include <MathUtils/MathUtils.h>

#ifndef _DEBUG
#define __ASSERT(expr, msg) (void) 0
#else
#define __ASSERT(expr, msg) ASSERT_IMPL(#expr, expr, __FILE__, __LINE__, msg)

static inline void ASSERT_IMPL(const char* expressionString, bool expressionResult,
	const char* file, int line, const char* expressionMessage)
{
	if (expressionResult)
		return;

	spdlog::error(
		"Assertion failed: {}({}) - {} ({})", file, line, expressionMessage, expressionString);
}
#endif

#endif // SERVER_SHAREDSERVER_MY_3DSTRUCT_H

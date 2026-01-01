#ifndef SERVER_SHAREDSERVER_MY_3DSTRUCT_H
#define SERVER_SHAREDSERVER_MY_3DSTRUCT_H

#pragma once

#include <spdlog/spdlog.h>

#include <MathUtils/MathUtils.h>

#ifndef _DEBUG
#define __ASSERT(expression, expressionMessage) (void)0
#else
#define __ASSERT(expression, expressionMessage) ASSERT_IMPL(#expression, expression, __FILE__, __LINE__, expressionMessage)

static inline void ASSERT_IMPL(const char* expressionString, bool expressionResult, const char* file, int line, const char* expressionMessage)
{
	if (!expressionResult)
		spdlog::error("Assertion failed: {}({}) - {} ({})", file, line, expressionMessage, expressionString);
}
#endif

#endif // SERVER_SHAREDSERVER_MY_3DSTRUCT_H

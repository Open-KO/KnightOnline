#ifndef SERVER_EBENEZER_LOGIC_ELSE_H
#define SERVER_EBENEZER_LOGIC_ELSE_H

#pragma once

#include "Define.h"

namespace Ebenezer
{

class LOGIC_ELSE
{
public:
	bool Parse_and(const char* line, const std::string& filename, int lineNumber);
	void Init();
	uint8_t m_LogicElse;
	bool m_bAnd;
	int m_LogicElseInt[MAX_LOGIC_ELSE_INT];

	LOGIC_ELSE();
	virtual ~LOGIC_ELSE();
};

} // namespace Ebenezer

#endif // SERVER_EBENEZER_LOGIC_ELSE_H

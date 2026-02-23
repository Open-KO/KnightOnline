#ifndef SERVER_EBENEZER_EXEC_H
#define SERVER_EBENEZER_EXEC_H

#pragma once

#include "Define.h"

namespace Ebenezer
{

class EXEC
{
public:
	void Init();

	uint8_t m_Exec              = 0;
	int m_ExecInt[MAX_EXEC_INT] = {};

	bool Parse(const char* line, const std::string& filename, int lineNumber);
	EXEC();
	virtual ~EXEC();
};

} // namespace Ebenezer

#endif // SERVER_EBENEZER_EXEC_H

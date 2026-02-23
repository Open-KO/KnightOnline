#ifndef SERVER_EBENEZER_EVENT_DATA_H
#define SERVER_EBENEZER_EVENT_DATA_H

#pragma once

#include "EXEC.h"
#include "LOGIC_ELSE.h"

#include <list>
#include <vector>

namespace Ebenezer
{

using ExecArray      = std::list<EXEC*>;
using LogicElseArray = std::list<LOGIC_ELSE*>;

class EVENT_DATA
{
public:
	int m_EventNum = 0;
	ExecArray m_arExec;
	LogicElseArray m_arLogicElse;

	std::vector<std::string> _unhandledOpcodes;

	EVENT_DATA();
	virtual ~EVENT_DATA();
};

} // namespace Ebenezer

#endif // SERVER_EBENEZER_EVENT_DATA_H

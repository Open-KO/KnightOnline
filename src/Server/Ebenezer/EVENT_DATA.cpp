#include "pch.h"
#include "Define.h"
#include "EVENT_DATA.h"
#include "EXEC.h"
#include "LOGIC_ELSE.h"

EVENT_DATA::EVENT_DATA()
{
}

EVENT_DATA::~EVENT_DATA()
{
	while (!m_arExec.empty())
	{
		delete m_arExec.front();
		m_arExec.pop_front();
	}
	m_arExec.clear();

	while (!m_arLogicElse.empty())
	{
		delete m_arLogicElse.front();
		m_arLogicElse.pop_front();
	}
	m_arLogicElse.clear();
}

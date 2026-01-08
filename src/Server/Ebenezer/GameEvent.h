#ifndef SERVER_EBENEZER_GAMEEVENT_H
#define SERVER_EBENEZER_GAMEEVENT_H

#pragma once

namespace Ebenezer
{

class CUser;
class CGameEvent
{
public:
	void RunEvent(CUser* pUser = nullptr);
	CGameEvent();
	virtual ~CGameEvent();

	int16_t m_sIndex = 0;
	uint8_t m_bType  = 0;

	int m_iExec[5]   = {};
};

} // namespace Ebenezer

#endif // SERVER_EBENEZER_GAMEEVENT_H

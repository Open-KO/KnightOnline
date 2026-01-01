#ifndef SERVER_EBENEZER_GAMEEVENT_H
#define SERVER_EBENEZER_GAMEEVENT_H

#pragma once

class CUser;
class CGameEvent
{
public:
	void RunEvent(CUser* pUser = nullptr);
	CGameEvent();
	virtual ~CGameEvent();

	int16_t	m_sIndex;
	uint8_t	m_bType;

	int		m_iExec[5];
};

#endif // SERVER_EBENEZER_GAMEEVENT_H

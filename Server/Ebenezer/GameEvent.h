// GameEvent.h: interface for the CGameEvent class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GAMEEVENT_H__1D5848E3_70D7_4278_9A89_A2A94AA20757__INCLUDED_)
#define AFX_GAMEEVENT_H__1D5848E3_70D7_4278_9A89_A2A94AA20757__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CUser;
class CGameEvent
{
public:
	void RunEvent(CUser* pUser = nullptr);
	CGameEvent();
	virtual ~CGameEvent();

	short	m_sIndex;
	BYTE	m_bType;

	int		m_iExec[5];
};

#endif // !defined(AFX_GAMEEVENT_H__1D5848E3_70D7_4278_9A89_A2A94AA20757__INCLUDED_)

// EventManager.h: interface for the CEventManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EVENTMANAGER_H__7FB7F7FD_67B8_4F22_AE3E_C41581C16561__INCLUDED_)
#define AFX_EVENTMANAGER_H__7FB7F7FD_67B8_4F22_AE3E_C41581C16561__INCLUDED_

#pragma once

#include <N3Base/N3Base.h>

class CEventCell
{
public:
	int16_t m_sEventType;
	RECT m_Rect;

	void Load(File& file);
	CEventCell();
	virtual ~CEventCell();
};

typedef std::list<CEventCell*> EventList;
typedef EventList::iterator EventItor;

class CEventManager
{
private:
	EventList m_lstEvents;
	int16_t m_sEventType;
	RECT m_rcEvent;

public:
	void Behavior(int16_t sEventType, int16_t sPreEventType);
	bool PtInRect(int x, int z, RECT rc);
	int16_t SetPos(float fX, float fZ);
	void Release();
	bool LoadFromFile(const char* szFileName);
	CEventManager();
	virtual ~CEventManager();
};

#endif // !defined(AFX_EVENTMANAGER_H__7FB7F7FD_67B8_4F22_AE3E_C41581C16561__INCLUDED_)

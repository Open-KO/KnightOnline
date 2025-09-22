// EVENT.h: interface for the EVENT class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EVENT_H__7514FC23_511B_11D3_BE41_00105A6B97E2__INCLUDED_)
#define AFX_EVENT_H__7514FC23_511B_11D3_BE41_00105A6B97E2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EVENT_DATA.h"
#include <shared/STLMap.h>

typedef CSTLMap <EVENT_DATA>				EventDataArray;

class EVENT
{
public:
	void DeleteAll();
	void Parsing(char* pBuf);
	void Init();
	bool LoadEvent(int zone);
	int m_Zone;

	EventDataArray m_arEvent;

	EVENT();
	virtual ~EVENT();

};

#endif // !defined(AFX_EVENT_H__7514FC23_511B_11D3_BE41_00105A6B97E2__INCLUDED_)

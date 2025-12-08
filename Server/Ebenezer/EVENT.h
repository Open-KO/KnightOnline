#pragma once

#include "EVENT_DATA.h"
#include <shared-server/STLMap.h>

typedef CSTLMap <EVENT_DATA>				EventDataArray;

class EVENT
{
public:
	void DeleteAll();
	void Init();
	bool LoadEvent(int zone);
	int m_Zone;

	EventDataArray m_arEvent;

	EVENT();
	virtual ~EVENT();
};

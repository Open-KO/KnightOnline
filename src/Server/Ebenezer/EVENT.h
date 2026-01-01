#ifndef SERVER_EBENEZER_EVENT_H
#define SERVER_EBENEZER_EVENT_H

#pragma once

#include "EVENT_DATA.h"
#include <shared-server/STLMap.h>

typedef CSTLMap <EVENT_DATA>				EventDataArray;

class EVENT
{
public:
	void DeleteAll();
	void Init();
	bool LoadEvent(int zone, const std::filesystem::path& questsDir);
	int m_Zone;

	EventDataArray m_arEvent;

	EVENT();
	virtual ~EVENT();
};

#endif // SERVER_EBENEZER_EVENT_H

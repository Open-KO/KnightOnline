#ifndef SERVER_EBENEZER_EVENT_H
#define SERVER_EBENEZER_EVENT_H

#pragma once

#include "EVENT_DATA.h"
#include <shared-server/STLMap.h>

namespace Ebenezer
{

using EventDataArray = CSTLMap<EVENT_DATA>;

class EVENT
{
public:
	void DeleteAll();
	void Init();
	bool LoadEvent(int zone, const std::filesystem::path& questsDir);

protected:
	bool LoadEventImpl(int zone, const std::filesystem::path& questsDir);

public:
	int m_Zone = 0;
	EventDataArray m_arEvent;

	EVENT();
	virtual ~EVENT();
};

} // namespace Ebenezer

#endif // SERVER_EBENEZER_EVENT_H

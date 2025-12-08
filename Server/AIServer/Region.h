#ifndef SERVER_AISERVER_REGION_H
#define SERVER_AISERVER_REGION_H

#pragma once

#include <shared-server/STLMap.h>

class CUser;
class CNpc;

typedef CSTLMap <int>			ZoneUserArray;
typedef CSTLMap <int>			ZoneNpcArray;

class CRegion
{
public:
	ZoneUserArray	m_RegionUserArray;
	ZoneNpcArray	m_RegionNpcArray;
	uint8_t	m_byMoving;			// move : 1, not moving : 0

protected:
	int		m_nIndex;
};

#endif // SERVER_AISERVER_REGION_H

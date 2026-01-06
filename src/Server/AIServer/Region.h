#ifndef SERVER_AISERVER_REGION_H
#define SERVER_AISERVER_REGION_H

#pragma once

#include <shared-server/STLMap.h>

namespace AIServer
{

using ZoneUserArray = CSTLMap<int>;
using ZoneNpcArray  = CSTLMap<int>;

class CRegion
{
public:
	ZoneUserArray m_RegionUserArray = {};
	ZoneNpcArray m_RegionNpcArray   = {};
	uint8_t m_byMoving              = 0; // move : 1, not moving : 0
};

} // namespace AIServer

#endif // SERVER_AISERVER_REGION_H

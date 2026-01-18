#ifndef SERVER_EBENEZER_REGION_H
#define SERVER_EBENEZER_REGION_H

#pragma once

#include "Define.h"
#include "GameDefine.h"

#include <shared-server/STLMap.h>

namespace Ebenezer
{

using ZoneItemArray = CSTLMap<_ZONE_ITEM>;
using ZoneUserArray = CSTLMap<int>;
using ZoneNpcArray  = CSTLMap<int>;

class CRegion
{
public:
	ZoneItemArray m_RegionItemArray;
	ZoneUserArray m_RegionUserArray;
	ZoneNpcArray m_RegionNpcArray;
};

} // namespace Ebenezer

#endif // SERVER_EBENEZER_REGION_H

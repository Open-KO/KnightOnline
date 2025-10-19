// Region.h: interface for the CRegion class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REGION_H__98237635_8ED6_4B3A_8AB8_933055C4995F__INCLUDED_)
#define AFX_REGION_H__98237635_8ED6_4B3A_8AB8_933055C4995F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Define.h"
#include "GameDefine.h"
#include <shared-server/STLMap.h>

typedef CSTLMap <_ZONE_ITEM>	ZoneItemArray;
typedef CSTLMap <int>			ZoneUserArray;
typedef CSTLMap <int>			ZoneNpcArray;

class CRegion
{
public:
	CRegion();
	virtual ~CRegion();

	ZoneItemArray	m_RegionItemArray;
	ZoneUserArray	m_RegionUserArray;
	ZoneNpcArray	m_RegionNpcArray;

};

#endif // !defined(AFX_REGION_H__98237635_8ED6_4B3A_8AB8_933055C4995F__INCLUDED_)

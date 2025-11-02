// Region.cpp: implementation of the CRegion class.
//
//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "Region.h"
#include "Npc.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRegion::CRegion()
{
}

CRegion::~CRegion()
{
	if (!m_RegionUserArray.IsEmpty())
		m_RegionUserArray.DeleteAllData();

	if (!m_RegionNpcArray.IsEmpty())
		m_RegionNpcArray.DeleteAllData();
}

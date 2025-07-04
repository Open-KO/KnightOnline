// N3WorldBase.cpp: implementation of the CN3WorldBase class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "N3WorldBase.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CN3WorldBase::CN3WorldBase()
{
	m_byTariff = 0;
	m_zoneFlags = 0;
	m_zoneType = ZONE_ABILITY_NEUTRAL;
	m_byMinLevel = m_byMaxLevel = 0;
}

CN3WorldBase::~CN3WorldBase()
{

}

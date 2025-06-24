// NpcPosTable.cpp: implementation of the CNpcPosTable class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Server.h"
#include "NpcPosTable.h"
#include "ini.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNpcPosTable::CNpcPosTable()
{

}

CNpcPosTable::~CNpcPosTable()
{

}

void CNpcPosTable::Initialize()
{
	CIni ini;
	ini.SetPath("mon_pos1.ini");
	
	m_sSid = ini.GetProfileInt("MAIN", "sSid", 0);
	strcpy(m_strName, ini.GetProfileString("MAIN", "strName", "monster"));
	m_byType = ini.GetProfileInt("MAIN", "type", 0);
	m_sZone = ini.GetProfileInt("MAIN", "zone", 0);
	m_sX = ini.GetProfileInt("MAIN", "xpos", 0);
	m_sY = ini.GetProfileInt("MAIN", "ypos", 0);
	m_sZ = ini.GetProfileInt("MAIN", "zpos", 0);
	m_sMinX = ini.GetProfileInt("MAIN", "minx", 0);
	m_sMinY = ini.GetProfileInt("MAIN", "miny", 0);
	m_sMaxX = ini.GetProfileInt("MAIN", "maxx", 0);
	m_sMaxY = ini.GetProfileInt("MAIN", "maxy", 0);
	m_sArea = ini.GetProfileInt("MAIN", "area", 0);
	m_sRegenTime = ini.GetProfileInt("MAIN", "regentime", 2000);
	m_sCount = ini.GetProfileInt("MAIN", "count", 0);
}

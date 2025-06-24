// NpcPosTable.h: interface for the CNpcPosTable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NPCPOSTABLE_H__8BBA6053_8411_456C_9ADC_72C5215FA82B__INCLUDED_)
#define AFX_NPCPOSTABLE_H__8BBA6053_8411_456C_9ADC_72C5215FA82B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CNpcPosTable  
{
public:
	short	m_sSid;				// MONSTER(NPC) Serial ID
	TCHAR	m_strName[20];		// MONSTER(NPC) Name
	BYTE	m_byType;			// MONSTER(NPC) type
	int		m_sZone;			// MONSTER(NPC) zone
	float	m_sX;				// MONSTER(NPC) x position
	float	m_sY;				// MONSTER(NPC) y position
	float	m_sZ;				// MONSTER(NPC) z position
	int		m_sArea;			// MONSTER(NPC) area
	int		m_sMinX;			// 몬스터의 움직임 영역..
	int		m_sMinY;
	int		m_sMaxX;
	int		m_sMaxY;
	long	m_sRegenTime;		// 
	int		m_sCount;			// MONSTER(NPC) count

public:
	void Initialize();
	CNpcPosTable();
	virtual ~CNpcPosTable();

};

#endif // !defined(AFX_NPCPOSTABLE_H__8BBA6053_8411_456C_9ADC_72C5215FA82B__INCLUDED_)

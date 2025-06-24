// NpcInfoTable.h: interface for the CNpcInfoTable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NPCINFOTABLE_H__5F61A01D_DBD7_40CE_8728_03E662E44182__INCLUDED_)
#define AFX_NPCINFOTABLE_H__5F61A01D_DBD7_40CE_8728_03E662E44182__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CNpcInfoTable  
{
public:
	short	m_sSid;				// MONSTER(NPC) Serial ID
	TCHAR	m_strName[20];		// MONSTER(NPC) Name

	short	m_sMaxHP;			// 최대 HP
	short	m_sMaxMP;			// 최대 PP
	
	BYTE	m_byClass;			// 무기계열

	int		m_iExp;				// 경험치

	short	m_sDefense;		// 방어값
	BYTE	m_byAttackRange;			// 사정거리

	int		m_sAI;				// 인공지능 인덱스
	int		m_sAttackDelay;		// 공격딜레이

	BYTE	m_bySearchRange;	// 적 탐지 범위
	int		m_sSpeed;			// 이동속도	

	int		m_sStandTime;		// 서있는 시간
	BYTE	m_tNpcType;			// NPC Type
								// 0 : Monster
								// 1 : Normal NPC

	int		m_sFamilyType;		// 몹들사이에서 가족관계를 결정한다.
	BYTE	m_tItemPer;			// 아이템이 떨어질 확률
	BYTE	m_tDnPer;			// 돈이 떨어질확률

public:
	void Initialize();
	CNpcInfoTable();
	virtual ~CNpcInfoTable();

};

#endif // !defined(AFX_NPCINFOTABLE_H__5F61A01D_DBD7_40CE_8728_03E662E44182__INCLUDED_)

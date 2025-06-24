// NpcTable.h: interface for the CNpcTable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NPCTABLE_H__07FF861B_C3CB_4EE8_B64F_E9796B110101__INCLUDED_)
#define AFX_NPCTABLE_H__07FF861B_C3CB_4EE8_B64F_E9796B110101__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CNpcTable
{
public:
	short	m_sSid;							// MONSTER(NPC) Serial ID
	char	m_strName[MAX_ID_SIZE + 1];		// MONSTER(NPC) Name
	short	m_sPid;							// MONSTER(NPC) Picture ID
	short   m_sSize;						// 캐릭터의 비율(100 퍼센트 기준)
	int 	m_iWeapon_1;		// 무기계열(오른손)
	int 	m_iWeapon_2;		// 무기계열(왼손)
	BYTE	m_byGroup;			// 소속집단
	BYTE	m_byActType;		// 행동패턴
	BYTE	m_tNpcType;			// NPC Type
								// 0 : Monster
								// 1 : Normal NPC
	BYTE	m_byFamilyType;		// 몹들사이에서 가족관계를 결정한다.
	BYTE	m_byRank;			// 작위
	BYTE	m_byTitle;			// 지위
	int 	m_iSellingGroup;	// 아이템 그룹(물건매매 담당 NPC의 경우만)
	short	m_sLevel;			// level
	int		m_iExp;				// 경험치
	int		m_iLoyalty;			// loyalty

	int 	m_iMaxHP;			// 최대 HP
	short	m_sMaxMP;			// 최대 MP

	short	m_sAttack;			// 공격값
	short	m_sDefense;			// 방어값
	short	m_sHitRate;			// 타격성공률
	short   m_sEvadeRate;		// 회피성공률
	short	m_sDamage;			// 기본 데미지
	short	m_sAttackDelay;		// 공격딜레이
	short	m_sSpeed;			// 이동속도	
	BYTE    m_bySpeed_1;		// 기본 이동 타입
	BYTE    m_bySpeed_2;		// 뛰는 이동 타입..
	short	m_sStandTime;		// 서있는 시간
	int		m_iMagic1;			// 사용마법 1
	int		m_iMagic2;			// 사용마법 2
	int		m_iMagic3;			// 사용마법 3	

	BYTE	m_byFireR;			// 화염 저항력
	BYTE	m_byColdR;			// 냉기 저항력
	BYTE	m_byLightningR;		// 전기 저항력
	BYTE	m_byMagicR;			// 마법 저항력
	BYTE	m_byDiseaseR;		// 저주 저항력
	BYTE	m_byPoisonR;		// 독 저항력
	BYTE	m_byLightR;			// 빛 저항력
	short	m_sBulk;			// 몬스터의 크기 (실제 길이 * 100)

	BYTE	m_bySearchRange;	// 적 탐지 범위
	BYTE	m_byAttackRange;	// 사정거리
	BYTE	m_byTracingRange;	// 추격거리

	short	m_sAI;				// 인공지능 인덱스


	BYTE	m_tItemPer;			// 아이템이 떨어질 확률
	BYTE	m_tDnPer;			// 돈이 떨어질확률

	int		m_iMoney;			// 떨어지는 돈
	int		m_iItem;			// 떨어지는 아이템
	BYTE	m_byDirectAttack;	// 공격방법( 0 : 직접, 1:롱공격(간접공격), 2:직,간접공격 )
	BYTE	m_byMagicAttack;	// 마법공격( 0:마법공격 없음, 1:마법공격, 2:독공격, 3:힐링)

public:
	CNpcTable();
	virtual ~CNpcTable();

	void Initialize();

};

#endif // !defined(AFX_NPCTABLE_H__07FF861B_C3CB_4EE8_B64F_E9796B110101__INCLUDED_)

// NpcTable.cpp: implementation of the CNpcTable class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Server.h"
#include "NpcTable.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNpcTable::CNpcTable()
{

}

CNpcTable::~CNpcTable()
{

}

void CNpcTable::Initialize()
{
	m_sSid = -1;				// MONSTER(NPC) Serial ID
	memset(m_strName, 0, sizeof(m_strName));		// MONSTER(NPC) Name
	m_sPid = 0;
	m_sSize = 0;
	m_iWeapon_1 = 0;		// 무기계열
	m_iWeapon_2 = 0;		// 무기계열
	m_byGroup = 0;			// 소속집단
	m_byActType = 0;		// 행동패턴
	m_byRank = 0;			// 작위
	m_byTitle = 0;			// 지위
	m_iSellingGroup = 0;	// 아이템 그룹(물건매매 담당 NPC의 경우만)
	m_sLevel = 0;			// level
	m_iExp = 0;				// 경험치
	m_iLoyalty = 0;			// loyalty
	m_iMaxHP = 0;			// 최대 HP
	m_sMaxMP = 0;			// 최대 MP
	m_sAttack = 0;			// 공격값
	m_sDefense = 0;			// 방어값
	m_sHitRate = 0;			// 타격성공률
	m_sEvadeRate = 0;		// 회피성공률
	m_sDamage = 0;			// 기본 데미지
	m_sAttackDelay = 0;		// 공격딜레이
	m_sSpeed = 0;			// 이동속도	
	m_sStandTime = 0;		// 서있는 시간
	m_iMagic1 = 0;			// 사용마법 1
	m_iMagic2 = 0;			// 사용마법 2
	m_iMagic3 = 0;			// 사용마법 3	
	m_byFireR = 0;			// 화염 저항력
	m_byColdR = 0;			// 냉기 저항력
	m_byLightR = 0;			// 빛 저항력
	m_byMagicR = 0;			// 마법 저항력
	m_byDiseaseR = 0;		// 저주 저항력
	m_byPoisonR = 0;		// 독 저항력
	m_sBulk = 0;
	m_bySearchRange = 0;	// 적 탐지 범위
	m_byAttackRange = 0;	// 사정거리
	m_byTracingRange = 0;
	m_sAI = 0;				// 인공지능 인덱스
	m_tNpcType = 0;			// NPC Type
						// 0 : Monster
						// 1 : Normal NPC
	m_byFamilyType = 0;		// 몹들사이에서 가족관계를 결정한다.
	m_tItemPer = 0;			// 아이템이 떨어질 확률
	m_tDnPer = 0;			// 돈이 떨어질확률
	m_iMoney = 0;			// 떨어지는 돈
	m_iItem = 0;			// 떨어지는 아이템
	m_byDirectAttack = 0;	// 공격방법( 0 : 직접, 1:롱공격(간접공격), 2:직,간접공격 )
	m_byMagicAttack = 0;	// 마법공격( 0:마법공격 없음, 1:마법공격, 2:독공격, 3:힐링)
}

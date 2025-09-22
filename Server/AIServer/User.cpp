// User.cpp: implementation of the CUser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Server.h"
#include "User.h"
#include "Serverdlg.h"
#include "define.h"
#include "Region.h"
#include "GameSocket.h"
#include <spdlog/spdlog.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#include "extern.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/*
	 ** Repent AI Server 작업시 참고 사항 **
	1. Initialize() 수정
	2. SendAttackSuccess() 수정
	3. GetDamage() 수정
*/

#define MORAL_GOOD		0x01
#define MORAL_BAD		0x02
#define MORAL_NEUTRAL	0x03

// 운영자 아이디 넣기..
/*const char* g_pszOPID[] =
{
	//"여우야2",
	//"난강해",
	//"이쁜여우2"
	//"Morpheus"
//	"맨순",
//	"민순"
};*/

float surround_fx[8] = { 0.0f, -0.7071f, -1.0f, -0.7083f,  0.0f,  0.7059f,  1.0000f, 0.7083f };
float surround_fz[8] = { 1.0f,  0.7071f,  0.0f, -0.7059f, -1.0f, -0.7083f, -0.0017f, 0.7059f };

extern CRITICAL_SECTION g_region_critical;
extern CRITICAL_SECTION g_LogFileWrite;

CUser::CUser()
{
}

CUser::~CUser()
{
}

void CUser::Initialize()
{
	m_pMain = (CServerDlg*) AfxGetApp()->GetMainWnd();
	m_pIocport = (CIOCPort*) &m_pMain->m_Iocport;

	m_MagicProcess.m_pMain = m_pMain;
	m_MagicProcess.m_pSrcUser = this;

	memset(m_strUserID, 0, sizeof(m_strUserID));// 캐릭터의 이름
	m_iUserId = -1;								// User의 번호
	m_bLive = USER_DEAD;						// 죽었니? 살았니?
	m_curx = 0.0f;								// 현재 X 좌표
	m_cury = 0.0f;								// 현재 Y 좌표
	m_curz = 0.0f;								// 현재 Z 좌표
	m_fWill_x = 0.0f;
	m_fWill_y = 0.0f;
	m_fWill_z = 0.0f;
	m_curZone = -1;								// 현재 존
	m_sZoneIndex = -1;
	m_bNation = 0;								// 소속국가
	m_sLevel = 0;								// 레벨
	m_sHP = 0;									// HP
	m_sMP = 0;									// MP
	m_sSP = 0;									// SP
	m_sMaxHP = 0;								// MaxHP
	m_sMaxMP = 0;								// MaxMP
	m_sMaxSP = 0;								// MaxSP
	m_state = 0;								// User의 상태
	m_sRegionX = 0;								// 현재 영역 X 좌표
	m_sRegionZ = 0;								// 현재 영역 Z 좌표
	m_sOldRegionX = 0;
	m_sOldRegionZ = 0;
	m_bResHp = 0;								// 회복량
	m_bResMp = 0;
	m_bResSta = 0;
	m_sHitDamage = 0;							// Hit
	m_sAC = 0;
	m_sItemAC = 0;
	m_fHitrate = 0.0f;							// 타격 성공률
	m_fAvoidrate = 0;							// 회피 성공률
	m_bLogOut = false;							// Logout 중인가?
	m_byNowParty = 0;
	m_sPartyTotalLevel = 0;
	m_byPartyTotalMan = 0;
	m_sPartyNumber = -1;
	m_byIsOP = 0;
	m_lUsed = 0;
	InitNpcAttack();

	m_bMagicTypeLeftHand = 0;
	m_bMagicTypeRightHand = 0;
	m_sMagicAmountLeftHand = 0;
	m_sMagicAmountRightHand = 0;
}

void CUser::Attack(int sid, int tid)
{
	CNpc* pNpc = m_pMain->m_NpcMap.GetData(tid - NPC_BAND);
	if (pNpc == nullptr)
		return;

	if (pNpc->m_NpcState == NPC_DEAD)
		return;

	if (pNpc->m_iHP == 0)
		return;

	// 경비병이면 타겟을 해당 유저로
/*	if (pNpc->m_tNpcType == NPCTYPE_GUARD)
	{
		pNpc->m_Target.id = m_iUserId + USER_BAND;
		pNpc->m_Target.x = m_curx;
		pNpc->m_Target.y = m_cury;
		pNpc->m_Target.failCount = 0;
		pNpc->Attack(m_pIocport);
	//	return;
	}	*/

	int nDefence = 0, nFinalDamage = 0;
	// NPC 방어값 
	nDefence = pNpc->GetDefense();

	// 명중이면 //Damage 처리 ----------------------------------------------------------------//
	nFinalDamage = GetDamage(tid);

	if (m_byIsOP == AUTHORITY_MANAGER)
		nFinalDamage = USER_DAMAGE_OVERRIDE_GM;
	else if (m_byIsOP == AUTHORITY_LIMITED_MANAGER)
		nFinalDamage = USER_DAMAGE_OVERRIDE_LIMITED_GM;
	else if (m_pMain->m_byTestMode)
		nFinalDamage = USER_DAMAGE_OVERRIDE_TEST_MODE;	// sungyong test

	// Calculate Target HP	 -------------------------------------------------------//
	short sOldNpcHP = pNpc->m_iHP;

	// Npc가 죽은 경우,,
	if (!pNpc->SetDamage(0, nFinalDamage, m_strUserID, m_iUserId + USER_BAND, m_pIocport))
	{
		pNpc->SendExpToUserList(); // 경험치 분배!!
		pNpc->SendDead(m_pIocport);
		SendAttackSuccess(tid, ATTACK_TARGET_DEAD, nFinalDamage, pNpc->m_iHP);

	//	CheckMaxValue(m_dwXP, 1);		// 몹이 죽을때만 1 증가!	
	//	SendXP();
	}
	// 공격 결과 전송
	else
	{
		SendAttackSuccess(tid, ATTACK_SUCCESS, nFinalDamage, pNpc->m_iHP);
	}

	//	m_dwLastAttackTime = GetTickCount();
}

void CUser::SendAttackSuccess(int tuid, BYTE result, short sDamage, int nHP, short sAttack_type)
{
	int send_index = 0;
	int sid = -1, tid = -1;
	BYTE type, bResult;
	char buff[256] = {};
	float rx = 0.0f, ry = 0.0f, rz = 0.0f;

	type = 0x01;
	bResult = result;
	sid = m_iUserId + USER_BAND;
	tid = tuid;

	SetByte(buff, AG_ATTACK_RESULT, send_index);
	SetByte(buff, type, send_index);
	SetByte(buff, bResult, send_index);
	SetShort(buff, sid, send_index);
	SetShort(buff, tid, send_index);
	SetShort(buff, sDamage, send_index);
	SetDWORD(buff, nHP, send_index);
	SetByte(buff, sAttack_type, send_index);

	//TRACE(_T("User - SendAttackSuccess() : [sid=%d, tid=%d, result=%d], damage=%d, hp = %d\n"), sid, tid, bResult, sDamage, sHP);

	SendAll(buff, send_index);   // thread 에서 send
}

void CUser::SendMagicAttackResult(int tuid, BYTE result, short sDamage, short sHP)
{
	int send_index = 0;
	int sid = -1, tid = -1;
	BYTE type, bResult;
	char buff[256] = {};
	float rx = 0.0f, ry = 0.0f, rz = 0.0f;

	type = 0x01;
	bResult = result;
	sid = m_iUserId + USER_BAND;
	tid = tuid;

	SetByte(buff, AG_MAGIC_ATTACK_RESULT, send_index);
	SetByte(buff, type, send_index);
	SetByte(buff, bResult, send_index);
	SetShort(buff, sid, send_index);
	SetShort(buff, tid, send_index);
	SetShort(buff, sDamage, send_index);
	SetShort(buff, sHP, send_index);

	//TRACE(_T("User - SendAttackSuccess() : [sid=%d, tid=%d, result=%d], damage=%d, hp = %d\n"), sid, tid, bResult, sDamage, sHP);

	SendAll(buff, send_index);   // thread 에서 send
}

// sungyong 2002.05.22
void CUser::SendAll(const char* pBuf, int nLength)
{
	if (nLength <= 0
		|| nLength > sizeof(SEND_DATA::pBuf))
		return;

	if (m_iUserId < 0
		|| m_iUserId >= MAX_USER)
	{
		spdlog::error("User::SendAll: userId out of bounds [userId={} charId={}]",
			m_iUserId, m_strUserID);
		return;
	}

	if (m_pIocport == nullptr)
		return;

	MAP* pMap = m_pMain->GetMapByIndex(m_sZoneIndex);
	if (pMap == nullptr)
		return;

	SEND_DATA* pNewData = nullptr;
	pNewData = new SEND_DATA;
	if (pNewData == nullptr)
		return;

	pNewData->sCurZone = m_curZone;
	pNewData->sLength = nLength;
	::CopyMemory(pNewData->pBuf, pBuf, nLength);

	EnterCriticalSection(&m_pIocport->m_critSendData);
	m_pIocport->m_SendDataList.push_back(pNewData);
	LeaveCriticalSection(&m_pIocport->m_critSendData);

	PostQueuedCompletionStatus(m_pIocport->m_hSendIOCP, 0, 0, nullptr);
}
// ~sungyong 2002.05.22

//	Damage 계산, 만약 m_sHP 가 0 이하이면 사망처리
void CUser::SetDamage(int damage, int tid)
{
	if (damage <= 0)
		return;

	if (m_bLive == USER_DEAD)
		return;

	short sHP = m_sHP;

	m_sHP -= (short) damage;

	//TRACE(_T("User - SetDamage() : old=%d, damage=%d, curHP = %d, id=%hs, uid=%d\n"), sHP, damage, m_sHP, m_strUserID, m_iUserId);

	if (m_sHP <= 0)
	{
		m_sHP = 0;
		Dead(tid, damage);
	}

	//SendHP();
	// 버디중이면 다른 버디원에게 날린다.
}

void CUser::Dead(int tid, int nDamage)
{
	if (m_bLive == USER_DEAD)
		return;

	// 이 부분에서 update를 해야 함,,  게임서버에서,, 처리하도록,,,
	m_sHP = 0;
	m_bLive = USER_DEAD;

	InitNpcAttack();

	// region에서 삭제...
	MAP* pMap = m_pMain->GetMapByIndex(m_sZoneIndex);
	if (pMap == nullptr)
	{
		spdlog::error("User::Dead: map not found [userId={} charId={} zoneId={}]",
			m_iUserId, m_strUserID, m_sZoneIndex);
		return;
	}

	// map에 region에서 나의 정보 삭제..
	if (m_sRegionX < 0
		|| m_sRegionZ < 0
		|| m_sRegionX > pMap->GetXRegionMax()
		|| m_sRegionZ > pMap->GetZRegionMax())
	{
		spdlog::error("User::Dead: out of region bounds [userId={} charId={} x={} z={}]",
			m_iUserId, m_strUserID, m_sRegionX, m_sRegionZ);
		return;
	}

	//pMap->m_ppRegion[m_sRegionX][m_sRegionZ].DeleteUser(m_iUserId);
	pMap->RegionUserRemove(m_sRegionX, m_sRegionZ, m_iUserId);
	//TRACE(_T("*** User Dead()-> User(%hs, %d)를 Region에 삭제,, region_x=%d, y=%d\n"), m_strUserID, m_iUserId, m_sRegionX, m_sRegionZ);

	m_sRegionX = -1;
	m_sRegionZ = -1;

	int send_index = 0;
	int sid = -1, targid = -1;
	BYTE type, result;
	char buff[256] = {};
	memset(buff, 0, sizeof(buff));
	spdlog::debug("User::Dead: userId={} charId={}", m_iUserId, m_strUserID);

	float rx = 0.0f, ry = 0.0f, rz = 0.0f;

	type = 0x02;
	result = ATTACK_TARGET_DEAD;
	sid = tid;
	targid = m_iUserId + USER_BAND;

	SetByte(buff, AG_ATTACK_RESULT, send_index);
	SetByte(buff, type, send_index);
	SetByte(buff, result, send_index);
	SetShort(buff, sid, send_index);
	SetShort(buff, targid, send_index);
	SetShort(buff, nDamage, send_index);
	SetDWORD(buff, m_sHP, send_index);
	//SetShort( buff, m_sMaxHP, send_index );

	//TRACE(_T("Npc - SendAttackSuccess()-User Dead : [sid=%d, tid=%d, result=%d], damage=%d, hp = %d\n"), sid, targid, result, nDamage, m_sHP);

	if (tid > 0)
		SendAll(buff, send_index);   // thread 에서 send

/*	SetByte(buff, AG_DEAD, send_index );
	SetShort(buff, m_iUserId, send_index );
	Setfloat(buff, m_curx, send_index);
	Setfloat(buff, m_curz, send_index);

	SendAll(buff, send_index);   // thread 에서 send	*/
}

void CUser::SendHP()
{
	if (m_bLive == USER_DEAD)
		return;

	// HP 변동량을 게임서버로...
	int send_index = 0;
	char buff[256] = {};

	SetByte(buff, AG_USER_SET_HP, send_index);
	SetShort(buff, m_iUserId, send_index);
	SetDWORD(buff, m_sHP, send_index);

	SendAll(buff, send_index);
}

void CUser::SetExp(int iNpcExp, int iLoyalty, int iLevel)
{
	int nExp = 0;
	int nLoyalty = 0;
	int nLevel = 0;
	double TempValue = 0;
	nLevel = iLevel - m_sLevel;

	if (nLevel <= -14)
	{
		//TRACE(_T("$$ User - SetExp Level Fail : %hs, exp=%d, loyalty=%d, mylevel=%d, level=%d $$\n"), m_strUserID, iNpcExp, iLoyalty, m_sLevel, iLevel);
		//return;
		TempValue = iNpcExp * 0.2;
		nExp = (int) TempValue;
		if (TempValue > nExp)
			++nExp;

		TempValue = iLoyalty * 0.2;
		nLoyalty = (int) TempValue;
		if (TempValue > nLoyalty)
			++nLoyalty;
	}
	else if (nLevel <= -8
		&& nLevel >= -13)
	{
		TempValue = iNpcExp * 0.5;
		nExp = (int) TempValue;
		if (TempValue > nExp)
			++nExp;

		TempValue = iLoyalty * 0.5;
		nLoyalty = (int) TempValue;
		if (TempValue > nLoyalty)
			++nLoyalty;
	}
	else if (nLevel <= -2
		&& nLevel >= -7)
	{
		TempValue = iNpcExp * 0.8;
		nExp = (int) TempValue;
		if (TempValue > nExp)
			++nExp;

		TempValue = iLoyalty * 0.8;
		nLoyalty = (int) TempValue;
		if (TempValue > nLoyalty)
			++nLoyalty;
	}
	else if (nLevel >= -1) // && nLevel < 2)
	{
		nExp = iNpcExp;
		nLoyalty = iLoyalty;
	}
	/* else if (nLevel >= 2
		&& nLevel < 5)
	{
		TempValue = iNpcExp * 1.2;
		nExp = (int) TempValue;
		if (TempValue > nExp)
			++nExp;

		TempValue = iLoyalty * 1.2;
		nLoyalty = (int) TempValue;
		if (TempValue > nLoyalty)
			++nLoyalty;
	}
	else if (nLevel >= 5
		&& nLevel < 8)
	{
		TempValue = iNpcExp * 1.5;
		nExp = (int) TempValue;
		if (TempValue > nExp)
			++nExp;

		TempValue = iLoyalty * 1.5;
		nLoyalty = (int) TempValue;
		if (TempValue > nLoyalty)
			++nLoyalty;
	}
	else if (nLevel >= 8)
	{
		nExp = iNpcExp * 2;
		nLoyalty = iLoyalty * 2;
	}*/

	//TRACE(_T("$$ User - SetExp Level : %hs, exp=%d->%d, loy=%d->%d, mylevel=%d, monlevel=%d $$\n"), m_strUserID, iNpcExp, nExp, iLoyalty, nLoyalty, m_sLevel, iLevel);

	SendExp(nExp, nLoyalty);
}

void CUser::SetPartyExp(int iNpcExp, int iLoyalty, int iPartyLevel, int iMan)
{
	int nExp = 0;
	int nLoyalty = 0;
	int nPercent = 0, nLevelPercent = 0, nExpPercent = 0;
	double TempValue = 0;

	TempValue = (double) iPartyLevel / 100.0;
	nExpPercent = iNpcExp * TempValue;

	//TRACE(_T("$$ User - SetPartyExp Level : %hs, exp=%d->%d, loy=%d->%d, mylevel=%d, iPartyLevel=%d $$\n"), m_strUserID, iNpcExp, nExpPercent, iLoyalty, nLoyalty, m_sLevel, iPartyLevel);

	SendExp(iNpcExp, iLoyalty);
}

//  경험치를 보낸다. (레벨업일때 관련 수치를 준다)
void CUser::SendExp(int iExp, int iLoyalty, int tType)
{
	int send_index = 0;
	char buff[256] = {};

	SetByte(buff, AG_USER_EXP, send_index);
	SetShort(buff, m_iUserId, send_index);
	SetShort(buff, iExp, send_index);
	SetShort(buff, iLoyalty, send_index);

	//TRACE(_T("$$ User - SendExp : %hs, exp=%d, loyalty=%d $$\n"), m_strUserID, iExp, iLoyalty);

	SendAll(buff, send_index);
}

short CUser::GetDamage(int tid, int magicid)
{
	short damage = 0;
	float Attack = 0, Avoid = 0;
	short Hit = 0, HitB = 0;
	short Ac = 0;
	int random = 0;
	BYTE result = FAIL;

	model::Magic* pTable = nullptr;
	model::MagicType1* pType1 = nullptr;
	model::MagicType2* pType2 = nullptr;

	if (tid < NPC_BAND
		|| tid > INVALID_BAND)
		return damage;

	CNpc* pNpc = m_pMain->m_NpcMap.GetData(tid - NPC_BAND);
	if (pNpc == nullptr)
		return damage;

	if (pNpc->m_tNpcType == NPC_ARTIFACT
		|| pNpc->m_tNpcType == NPC_PHOENIX_GATE
		|| pNpc->m_tNpcType == NPC_GATE_LEVER
		|| pNpc->m_tNpcType == NPC_SPECIAL_GATE)
		return damage;

	Attack = (float) m_fHitrate;						// 공격민첩
	Avoid = (float) pNpc->m_sEvadeRate;					// 방어민첩	
	Hit = m_sHitDamage;									// 공격자 Hit 
//	Ac = (short) (pNpc->m_sDefense) + pNpc->m_sLevel;	// 방어자 Ac 2002.07.06
	Ac = (short) (pNpc->m_sDefense);					// 방어자 Ac 
	HitB = (int) ((Hit * 200) / (Ac + 240));			// 새로운 공격식의 B

	// Skill Hit.
	if (magicid > 0)
	{
		// Get main magic table.
		pTable = m_pMain->m_MagicTableMap.GetData(magicid);
		if (pTable == nullptr)
			return -1;

		// SKILL HIT!
		if (pTable->Type1 == 1)
		{
			// Get magic skill table type 1.
			pType1 = m_pMain->m_MagicType1TableMap.GetData(magicid);
			if (!pType1)
				return -1;

			// Non-relative hit.
			if (pType1->Type)
			{
				random = myrand(0, 100);
				if (pType1->HitRateMod <= random)
					result = FAIL;
				else
					result = SUCCESS;
			}
			// Relative hit.
			else
			{
				result = GetHitRate((Attack / Avoid) * (pType1->HitRateMod / 100.0f));
			}
/*
			// Non-relative hit.
			if (pType1->Type)
			{
				Hit = m_sHitDamage * (pType1->DamageMod / 100.0f);
			}
			else
			{
//				Hit = (m_sHitDamage - pNpc->m_sDefense)
				Hit = HitB * (pType1->DamageMod / 100.0f);
			}
*/
			Hit = HitB * (pType1->DamageMod / 100.0f);
		}
		// ARROW HIT!
		else if (pTable->Type1 == 2)
		{
			// Get magic skill table type 2.
			pType2 = m_pMain->m_MagicType2TableMap.GetData(magicid);
			if (pType2 == nullptr)
				return -1;

			// Non-relative/Penetration hit.
			if (pType2->HitType == 1
				|| pType2->HitType == 2)
			{
				random = myrand(0, 100);
				if (pType2->HitRateMod <= random)
					result = FAIL;
				else
					result = SUCCESS;

				//result = SUCCESS;
			}
			// Relative hit/Arc hit.
			else
			{
				result = GetHitRate((Attack / Avoid) * (pType2->HitRateMod / 100.0f));
			}

			if (pType2->HitType == 1
				/*|| pType2->HitType == 2*/)
			{
				Hit = m_sHitDamage * (pType2->DamageMod / 100.0f);
			}
			else
			{
//				Hit = (m_sHitDamage - pNpc->m_sDefense) * (pType2->DamageMod / 100.0f);
				Hit = HitB * (pType2->DamageMod / 100.0f);
			}
		}
	}
	// Normal Hit.
	else
	{
		result = GetHitRate(Attack / Avoid);		// 타격비 구하기
	}

	switch (result)
	{
		case GREAT_SUCCESS:
		case SUCCESS:
		case NORMAL:
			// 스킬 공격
			if (magicid > 0)
			{
				damage = (short) Hit;
				random = myrand(0, damage);
//				damage = (short) ((0.85f * (float) Hit) + 0.3f * (float) random);
				if (pTable->Type1 == 1)
				{
					damage = (short) ((float) Hit + 0.3f * (float) random + 0.99);
				}
				else
				{
					damage = (short) ((float) (Hit * 0.6f) + 1.0f * (float) random + 0.99);
				}
			}
			//일반 공격	
			else
			{
				damage = (short) (HitB);
				random = myrand(0, damage);
				damage = (short) ((0.85f * (float) HitB) + 0.3f * (float) random);
			}
			break;

		case FAIL:  // 사장님 요구 
			damage = 0;
			break;
	}

	damage = GetMagicDamage(damage, tid);	// 2. Magical item damage....

	//return 3000;
	return damage;
}

short CUser::GetMagicDamage(int damage, short tid)
{
	short total_r = 0, temp_damage = 0;

	CNpc* pNpc = m_pMain->m_NpcMap.GetData(tid - NPC_BAND);
	if (pNpc == nullptr)
		return damage;

	// RIGHT HAND!!! by Yookozuna
	if (m_bMagicTypeRightHand > 4
		&& m_bMagicTypeRightHand < 8)
		temp_damage = damage * m_sMagicAmountRightHand / 100;

	// RIGHT HAND!!!
	switch (m_bMagicTypeRightHand)
	{
		case 1:	// Fire Damage
			total_r = pNpc->m_sFireR;
			break;

		case 2:	// Ice Damage
			total_r = pNpc->m_sColdR;
			break;

		case 3:	// Lightning Damage
			total_r = pNpc->m_sLightningR;
			break;

		case 4:	// Poison Damage
			total_r = pNpc->m_sPoisonR;
			break;

		case 5:	// HP Drain						
			break;

		case 6:	// MP Damage		
			pNpc->MSpChange(2, -temp_damage);
			break;

		case 7:	// MP Drain				
			break;
	}

	if (m_bMagicTypeRightHand > 0
		&& m_bMagicTypeRightHand < 5)
	{
		temp_damage = m_sMagicAmountRightHand - m_sMagicAmountRightHand * total_r / 200;
		damage += temp_damage;
	}

	// Reset all temporary data.
	total_r = 0;
	temp_damage = 0;

	// LEFT HAND!!! by Yookozuna
	if (m_bMagicTypeLeftHand > 4
		&& m_bMagicTypeLeftHand < 8)
		temp_damage = damage * m_sMagicAmountLeftHand / 100;

	// LEFT HAND!!!
	switch (m_bMagicTypeLeftHand)
	{
		case 1:	// Fire Damage
			total_r = pNpc->m_sFireR;
			break;

		case 2:	// Ice Damage
			total_r = pNpc->m_sColdR;
			break;

		case 3:	// Lightning Damage
			total_r = pNpc->m_sLightningR;
			break;

		case 4:	// Poison Damage
			total_r = pNpc->m_sPoisonR;
			break;

		case 5:	// HP Drain					
			break;

		case 6:	// MP Damage		
			pNpc->MSpChange(2, -temp_damage);
			break;

		case 7:	// MP Drain		
			break;
	}

	if (m_bMagicTypeLeftHand > 0
		&& m_bMagicTypeLeftHand < 5)
	{
		if (total_r > 200)
			total_r = 200;

		temp_damage = m_sMagicAmountLeftHand - m_sMagicAmountLeftHand * total_r / 200;
		damage += temp_damage;
	}

	return damage;
}

BYTE CUser::GetHitRate(float rate)
{
	BYTE result = FAIL;
	int random = myrand(1, 10000);

	if (rate >= 5.0f)
	{
		if (random >= 1
			&& random <= 3500)
			result = GREAT_SUCCESS;
		else if (random >= 3501
			&& random <= 7500)
			result = SUCCESS;
		else if (random >= 7501
			&& random <= 9800)
			result = NORMAL;
	}
	else if (rate >= 3.0f)
	{
		if (random >= 1
			&& random <= 2500)
			result = GREAT_SUCCESS;
		else if (random >= 2501
			&& random <= 6000)
			result = SUCCESS;
		else if (random >= 6001
			&& random <= 9600)
			result = NORMAL;
	}
	else if (rate >= 2.0f)
	{
		if (random >= 1
			&& random <= 2000)
			result = GREAT_SUCCESS;
		else if (random >= 2001
			&& random <= 5000)
			result = SUCCESS;
		else if (random >= 5001
			&& random <= 9400)
			result = NORMAL;
	}
	else if (rate >= 1.25f)
	{
		if (random >= 1
			&& random <= 1500)
			result = GREAT_SUCCESS;
		else if (random >= 1501
			&& random <= 4000)
			result = SUCCESS;
		else if (random >= 4001
			&& random <= 9200)
			result = NORMAL;
	}
	else if (rate >= 0.8f)
	{
		if (random >= 1
			&& random <= 1000)
			result = GREAT_SUCCESS;
		else if (random >= 1001
			&& random <= 3000)
			result = SUCCESS;
		else if (random >= 3001
			&& random <= 9000)
			result = NORMAL;
	}
	else if (rate >= 0.5f)
	{
		if (random >= 1
			&& random <= 800)
			result = GREAT_SUCCESS;
		else if (random >= 801
			&& random <= 2500)
			result = SUCCESS;
		else if (random >= 2501
			&& random <= 8000)
			result = NORMAL;
	}
	else if (rate >= 0.33f)
	{
		if (random >= 1
			&& random <= 600)
			result = GREAT_SUCCESS;
		else if (random >= 601
			&& random <= 2000)
			result = SUCCESS;
		else if (random >= 2001
			&& random <= 7000)
			result = NORMAL;
	}
	else if (rate >= 0.2f)
	{
		if (random >= 1
			&& random <= 400)
			result = GREAT_SUCCESS;
		else if (random >= 401
			&& random <= 1500)
			result = SUCCESS;
		else if (random >= 1501
			&& random <= 6000)
			result = NORMAL;
	}
	else
	{
		if (random >= 1
			&& random <= 200)
			result = GREAT_SUCCESS;
		else if (random >= 201
			&& random <= 1000)
			result = SUCCESS;
		else if (random >= 1001
			&& random <= 5000)
			result = NORMAL;
	}

	return result;
}


void CUser::SendSystemMsg(const std::string_view msg, BYTE type, int nWho)
{
	int send_index = 0;
	char buff[1024] = {};

	SetByte(buff, AG_SYSTEM_MSG, send_index);
	SetByte(buff, type, send_index);				// 채팅형식
	SetShort(buff, nWho, send_index);				// 누구에게
	SetShort(buff, m_iUserId, send_index);
	SetString2(buff, msg, send_index);

	SendAll(buff, send_index);
}

void CUser::InitNpcAttack()
{
	for (int i = 0; i < 8; i++)
		m_sSurroundNpcNumber[i] = -1;
}

int CUser::IsSurroundCheck(float fX, float fY, float fZ, int NpcID)
{
	int nDir = 0;
	__Vector3 vNpc, vUser, vDis;
	vNpc.Set(fX, fY, fZ);
	float fDX, fDZ;
	float fDis = 0.0f, fCurDis = 1000.0f;
	bool bFlag = false;
	for (int i = 0; i < 8; i++)
	{
		//if(m_sSurroundNpcNumber[i] != -1) continue;
		if (m_sSurroundNpcNumber[i] == NpcID)
		{
			if (bFlag)
			{
				m_sSurroundNpcNumber[i] = -1;
			}
			else
			{
				m_sSurroundNpcNumber[i] = NpcID;
				nDir = i + 1;
				bFlag = true;
			}
			//return nDir;
		}

		if (m_sSurroundNpcNumber[i] == -1
			&& !bFlag)
		{
			fDX = m_curx + surround_fx[i];
			fDZ = m_curz + surround_fz[i];
			vUser.Set(fDX, 0.0f, fDZ);
			vDis = vUser - vNpc;
			fDis = vDis.Magnitude();

			if (fDis < fCurDis)
			{
				nDir = i + 1;
				fCurDis = fDis;
			}
		}
	}


/*	TRACE(_T("User-Sur : [0=%d,1=%d,2=%d,3=%d,4=%d,5=%d,6=%d,7=%d]\n"), m_sSurroundNpcNumber[0],
		m_sSurroundNpcNumber[1], m_sSurroundNpcNumber[2], m_sSurroundNpcNumber[3], m_sSurroundNpcNumber[4],
		m_sSurroundNpcNumber[5],m_sSurroundNpcNumber[6], m_sSurroundNpcNumber[7]);
	*/
	if (nDir != 0)
		m_sSurroundNpcNumber[nDir - 1] = NpcID;

	return nDir;
}

bool CUser::IsOpIDCheck(const char* szName)
{
/*	int nSize = sizeof(g_pszOPID)/sizeof(char*);
	CString szCheck = szName;
	CString szCheck2;

	szCheck.MakeLower();

	for (int i=0; i< nSize; i++)
	{
		szCheck2 = g_pszOPID[i];
		szCheck2.MakeLower();

		if(szCheck.Find(szCheck2) != -1) return true;
	}	*/

	return false;
}

void CUser::HealMagic()
{
	int region_x = m_curx / VIEW_DIST;
	int region_z = m_curz / VIEW_DIST;

	MAP* pMap = m_pMain->GetMapByIndex(m_sZoneIndex);
	if (pMap == nullptr)
	{
		spdlog::error("User::HealMagic: map not found [userId={} charId={} zoneId={}]",
			m_iUserId, m_strUserID, m_sZoneIndex);
		return;
	}

	int max_xx = pMap->m_sizeRegion.cx;
	int max_zz = pMap->m_sizeRegion.cy;

	int min_x = region_x - 1;
	if (min_x < 0)
		min_x = 0;

	int min_z = region_z - 1;
	if (min_z < 0)
		min_z = 0;

	int max_x = region_x + 1;
	if (max_x >= max_xx)
		max_x = max_xx - 1;

	int max_z = region_z + 1;
	if (min_z >= max_zz)
		min_z = max_zz - 1;

	int search_x = max_x - min_x + 1;
	int search_z = max_z - min_z + 1;

	for (int i = 0; i < search_x; i++)
	{
		for (int j = 0; j < search_z; j++)
			HealAreaCheck(min_x + i, min_z + j);
	}
}

void CUser::HealAreaCheck(int rx, int rz)
{
	MAP* pMap = m_pMain->GetMapByIndex(m_sZoneIndex);
	if (pMap == nullptr)
	{
		spdlog::error("User::HealAreaCheck: map not found [userId={} charId={} zoneId={}]",
			m_iUserId, m_strUserID, m_sZoneIndex);
		return;
	}

	// 자신의 region에 있는 NpcMap을 먼저 검색하여,, 가까운 거리에 Monster가 있는지를 판단..
	if (rx < 0
		|| rz < 0
		|| rx > pMap->GetXRegionMax()
		|| rz > pMap->GetZRegionMax())
	{
		spdlog::error("User::HealAreaCheck: out of region bounds [userId={} charId={} x={} z={}]",
			m_iUserId, m_strUserID, m_sRegionX, m_sRegionZ);
		return;
	}
	
	// 30m
	float fRadius = 10.0f;

	__Vector3 vStart, vEnd;
	CNpc* pNpc = nullptr;
	float fDis = 0.0f;
	vStart.Set(m_curx, (float) 0, m_curz);
	char send_buff[256] = {};
	int nid = 0, send_index = 0, result = 1, count = 0, total_mon = 0;
	int* pNpcIDList = nullptr;

	EnterCriticalSection(&g_region_critical);

	auto Iter1 = pMap->m_ppRegion[rx][rz].m_RegionNpcArray.begin();
	auto Iter2 = pMap->m_ppRegion[rx][rz].m_RegionNpcArray.end();

	total_mon = pMap->m_ppRegion[rx][rz].m_RegionNpcArray.GetSize();
	pNpcIDList = new int[total_mon];
	for (; Iter1 != Iter2; Iter1++)
	{
		nid = *((*Iter1).second);
		pNpcIDList[count] = nid;
		count++;
	}
	LeaveCriticalSection(&g_region_critical);

	for (int i = 0; i < total_mon; i++)
	{
		nid = pNpcIDList[i];
		if (nid < NPC_BAND)
			continue;

		pNpc = m_pMain->m_NpcMap.GetData(nid - NPC_BAND);

		if (pNpc != nullptr
			&& pNpc->m_NpcState != NPC_DEAD)
		{
			if (m_bNation == pNpc->m_byGroup)
				continue;

			vEnd.Set(pNpc->m_fCurX, pNpc->m_fCurY, pNpc->m_fCurZ);
			fDis = pNpc->GetDistance(vStart, vEnd);

			// NPC가 반경안에 있을 경우...
			if (fDis > fRadius)
				continue;

			pNpc->ChangeTarget(1004, this, m_pIocport);
		}
	}

	delete[] pNpcIDList;
	pNpcIDList = nullptr;
}

void CUser::WriteUserLog()
{
	for (const _USERLOG* pUserLog : m_UserLogList)
	{
		if (pUserLog->byFlag == USER_LOGIN)
			spdlog::get(logger::AIServerUser)->info("Login: level={}, charId={}",
				pUserLog->byLevel, pUserLog->strUserID);
		else if (pUserLog->byFlag == USER_LOGOUT)
			spdlog::get(logger::AIServerUser)->info("Logout: level={}, charId={}",
				pUserLog->byLevel, pUserLog->strUserID);
		else if (pUserLog->byFlag == USER_LEVEL_UP)
			spdlog::get(logger::AIServerUser)->info("LevelUp: level={}, charId={}",
				pUserLog->byLevel, pUserLog->strUserID);
	}

	InitUserLog();
}

void CUser::InitUserLog()
{
	while (!m_UserLogList.empty())
	{
		delete m_UserLogList.front();
		m_UserLogList.pop_front();
	}
	m_UserLogList.clear();
}

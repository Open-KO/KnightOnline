#include "stdafx.h"
#include "math.h"
#include "Npc.h"
#include "Serverdlg.h"
#include "Gamesocket.h"
#include "Region.h"
#include "Party.h"
#include "extern.h"
#include <spdlog/spdlog.h>

//bool g_bDebug = true;

int surround_x[8] = { -1, -1, 0, 1, 1, 1, 0, -1 };
int surround_z[8] = { 0, -1, -1, -1, 0, 1, 1, 1 };

int test_id = 1056;
bool useNpcTrace = false;

constexpr int MAX_MAXWEAPON_CLASSES		= _countof(model::MakeWeapon::Class);
constexpr int MAX_ITEM_GRADECODE_GRADES	= _countof(model::MakeItemGradeCode::Grade);

#define ATROCITY_ATTACK_TYPE 1				// 선공
#define TENDER_ATTACK_TYPE	 0				// 후공	

// 행동변경 관련 
#define NO_ACTION				0
#define ATTACK_TO_TRACE			1				// 공격에서 추격
#define MONSTER_CHANGED			1 
#define LONG_ATTACK_RANGE		30				// 장거리 공격 유효거리
#define SHORT_ATTACK_RANGE		3				// 직접공격 유효거리

// 무기 관련
#define WEAPON_ITEM				1
#define DEFENCR_ITEM			2
#define ACCESSORY_ITEM			3

#define ARROW_MIN				391010000
#define ARROW_MAX				392010000

#define KARUS_MAN				1
#define ELMORAD_MAN				2

#define ATTACK_LIMIT_LEVEL		10
#define FAINTING_TIME			2

extern CRITICAL_SECTION g_region_critical;
extern CRITICAL_SECTION g_LogFileWrite;

/*
	 ** Repent AI Server 작업시 참고 사항 **
	1. m_fSpeed_1,m_fSpeed_2라는 변수가 없으므로 NPC_SECFORMETER_MOVE(4)로 수정,,
			나중에 이것도 나이츠 방식으로 수정할 것 (테이블)
	2. m_byGroup 변수 없음.. (이것 나오는 것 전부 주석..)
	3. m_byTracingRange 변수 없음 . m_bySearchRange*2으로 대치
	4. GetFinalDamage(), GetNFinalDamage(), GetDefense() 함수 수정..  공격치 계산식..
	5. FillNpcInfo() 수정
	6. SendNpcInfoAll() 수정
	7. SendAttackSuccess() 부분 수정.. 호출하는 부분도 수정할 것...
*/


//////////////////////////////////////////////////////////////////////
//	Inline Function
//
inline bool CNpc::SetUid(float x, float z, int id)
{
	if (m_sNid == test_id)
	{
		int x = 0;
	}

	MAP* pMap = m_pMain->GetMapByIndex(m_ZoneIndex);
	if (pMap == nullptr)
	{
		spdlog::error("Npc::SetUid: map not found [zoneIndex={} npcId={} npcName={}]",
			m_ZoneIndex, m_sSid, m_strName);
		return false;
	}

	int x1 = (int) x / TILE_SIZE;
	int z1 = (int) z / TILE_SIZE;
	int nRX = (int) x / VIEW_DIST;
	int nRZ = (int) z / VIEW_DIST;

	if (x1 < 0
		|| z1 < 0
		|| x1 > pMap->m_sizeMap.cx
		|| z1 > pMap->m_sizeMap.cy)
	{
		spdlog::error("Npc::SetUid: out of map bounds [serial={} npcId={} x={} z={}]",
			m_sNid + NPC_BAND, m_sSid, x1, z1);
		return false;
	}

	// map 이동이 불가능이면 npc등록 실패.. 
	// 작업 : 이 부분을 나중에 수정 처리....
	// if(pMap->m_pMap[x1][z1].m_sEvent == 0) return false;
	if (nRX > pMap->GetXRegionMax()
		|| nRZ > pMap->GetZRegionMax()
		|| nRX < 0
		|| nRZ < 0)
	{
		spdlog::error("Npc::SetUid: out of region bounds [serial={} npcId={} x={} z={}]",
			m_sNid + NPC_BAND, m_sSid, nRX, nRZ);
		return false;
	}

	if (m_iRegion_X != nRX
		|| m_iRegion_Z != nRZ)
	{
		int nOld_RX = m_iRegion_X;
		int nOld_RZ = m_iRegion_Z;
		m_iRegion_X = nRX;
		m_iRegion_Z = nRZ;

		//TRACE(_T("++ Npc-SetUid RegionAdd : [nid=%d, name=%hs], x=%.2f, z=%.2f, nRX=%d, nRZ=%d \n"), m_sNid+NPC_BAND, m_strName,x,z, nRX, nRY);
		// 새로운 region으로 npc이동 - npc의 정보 추가..
		CNpc* pNpc = m_pMain->m_NpcMap.GetData(id - NPC_BAND);
		if (pNpc == nullptr)
			return false;

		pMap->RegionNpcAdd(m_iRegion_X, m_iRegion_Z, id);

		// 기존의 region정보에서 npc의 정보 삭제..
		pMap->RegionNpcRemove(nOld_RX, nOld_RZ, id);
		//TRACE(_T("-- Npc-SetUid RegionRemove : [nid=%d, name=%hs], nRX=%d, nRZ=%d \n"), m_sNid+NPC_BAND, m_strName, nOld_RX, nOld_RZ);
	}

	return true;
}

CNpc::CNpc()
{
	m_NpcState = NPC_LIVE;
	m_byGateOpen = GATE_CLOSE;
	m_byObjectType = NORMAL_OBJECT;
	m_byPathCount = 0;
	m_byAttackPos = 0;
	m_sThreadNumber = -1;
	InitTarget();

	m_ItemUserLevel = 0;
	m_Delay = 0;
	m_fDelayTime = TimeGet();

	m_tNpcAttType = ATROCITY_ATTACK_TYPE;		// 공격 성향
	m_tNpcOldAttType = ATROCITY_ATTACK_TYPE;		// 공격 성향
	m_tNpcLongType = 0;		// 원거리(1), 근거리(0)
	m_tNpcGroupType = 0;	// 도움을 주는냐(1), 안주는냐?(0)
	m_byNpcEndAttType = 1;
	m_byWhatAttackType = 0;
	m_byMoveType = 1;
	m_byInitMoveType = 1;
	m_byRegenType = 0;
	m_byDungeonFamily = 0;
	m_bySpecialType = 0;
	m_byTrapNumber = 0;
	m_byChangeType = 0;
	m_byDeadType = 0;
	m_sChangeSid = 0;
	m_sControlSid = 0;
	m_sPathCount = 0;
	m_sMaxPathCount = 0;
	m_byMoneyType = 0;

	m_pPath = nullptr;
	m_pOrgMap = nullptr;

	m_bFirstLive = true;

	m_fHPChangeTime = TimeGet();
	m_fFaintingTime = 0.0f;

	::ZeroMemory(m_pMap, sizeof(m_pMap));// 일차원 맵으로 초기화한다.

	m_iRegion_X = 0;
	m_iRegion_Z = 0;
	m_nLimitMinX = m_nLimitMinZ = 0;
	m_nLimitMaxX = m_nLimitMaxZ = 0;
	m_lEventNpc = 0;
	m_fSecForRealMoveMetor = 0.0f;
	InitUserList();
	InitMagicValuable();

	m_pMain = (CServerDlg*) AfxGetApp()->GetMainWnd();
	m_MagicProcess.m_pMain = m_pMain;
	m_MagicProcess.m_pSrcNpc = this;

	for (int i = 0; i < NPC_MAX_PATH_LIST; i++)
	{
		m_PathList.pPattenPos[i].x = -1;
		m_PathList.pPattenPos[i].z = -1;
	}

	m_pPattenPos.x = m_pPattenPos.z = 0;
}

CNpc::~CNpc()
{
	ClearPathFindData();
	InitUserList();
}

///////////////////////////////////////////////////////////////////////
//	길찾기 데이터를 지운다.
//
void CNpc::ClearPathFindData()
{
	::ZeroMemory(m_pMap, sizeof(m_pMap));	// 일차원 맵을 위해

	m_bPathFlag = false;
	m_sStepCount = 0;
	m_iAniFrameCount = 0;
	m_iAniFrameIndex = 0;
	m_fAdd_x = 0.0f;
	m_fAdd_z = 0.0f;

	for (int i = 0; i < MAX_PATH_LINE; i++)
	{
		m_pPoint[i].byType = 0;
		m_pPoint[i].bySpeed = 0;
		m_pPoint[i].fXPos = -1.0f;
		m_pPoint[i].fZPos = -1.0f;
	}
}

/////////////////////////////////////////////////////////////////////////////
//	유저리스트를 초기화한다.
//
void CNpc::InitUserList()
{
	m_sMaxDamageUserid = -1;
	m_TotalDamage = 0;

	for (int i = 0; i < NPC_HAVE_USER_LIST; i++)
	{
		m_DamagedUserList[i].bIs = false;
		m_DamagedUserList[i].iUid = -1;
		m_DamagedUserList[i].nDamage = 0;
		::ZeroMemory(m_DamagedUserList[i].strUserID, sizeof(m_DamagedUserList[i].strUserID));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//	공격대상(Target)을 초기화 한다.
//
inline void CNpc::InitTarget()
{
	if (m_byAttackPos != 0)
	{
		if (m_Target.id >= 0 && m_Target.id < NPC_BAND)
		{
			CUser* pUser = m_pMain->GetUserPtr(m_Target.id);
			if (pUser != nullptr)
			{
				if (m_byAttackPos > 0
					&& m_byAttackPos < 9)
					pUser->m_sSurroundNpcNumber[m_byAttackPos - 1] = -1;
			}
		}
	}
	m_byAttackPos = 0;
	m_Target.id = -1;
	m_Target.x = 0.0;
	m_Target.y = 0.0;
	m_Target.z = 0.0;
	m_Target.failCount = 0;
}

void CNpc::Load(const model::Npc* pNpcTable, bool transformSpeeds)
{
	constexpr short MONSTER_SPEED = 1500;

	_ASSERT(pNpcTable != nullptr);

	if (pNpcTable == nullptr)
		return;

	if (pNpcTable->Name.has_value())
		m_strName = *pNpcTable->Name;				// MONSTER(NPC) Name

	m_sPid = pNpcTable->PictureId;					// MONSTER(NPC) Picture ID
	m_sSize = pNpcTable->Size;						// 캐릭터의 비율(100 퍼센트 기준)
	m_iWeapon_1 = pNpcTable->Weapon1;				// 착용무기
	m_iWeapon_2 = pNpcTable->Weapon2;				// 착용무기
	m_byGroup = pNpcTable->Group;					// 소속집단
	m_byActType = pNpcTable->ActType;				// 행동패턴
	m_byRank = pNpcTable->Rank;						// 작위
	m_byTitle = pNpcTable->Title;					// 지위
	m_iSellingGroup = pNpcTable->SellingGroup;
	m_sLevel = pNpcTable->Level;					// level
	m_iExp = pNpcTable->Exp;						// 경험치
	m_iLoyalty = pNpcTable->Loyalty;				// loyalty
	m_iHP = pNpcTable->HitPoints;					// 최대 HP
	m_iMaxHP = pNpcTable->HitPoints;				// 현재 HP
	m_sMP = pNpcTable->ManaPoints;					// 최대 MP
	m_sMaxMP = pNpcTable->ManaPoints;				// 현재 MP
	m_sAttack = pNpcTable->Attack;					// 공격값
	m_sDefense = pNpcTable->Armor;					// 방어값
	m_sHitRate = pNpcTable->HitRate;				// 타격성공률
	m_sEvadeRate = pNpcTable->EvadeRate;			// 회피성공률
	m_sDamage = pNpcTable->Damage;					// 기본 데미지
	m_sAttackDelay = pNpcTable->AttackDelay;		// 공격딜레이

	m_sSpeed = MONSTER_SPEED;						// 이동속도

	m_fSpeed_1 = (float) pNpcTable->WalkSpeed;		// 기본 이동 타입
	m_fSpeed_2 = (float) pNpcTable->RunSpeed;		// 뛰는 이동 타입..
	m_fOldSpeed_1 = (float) pNpcTable->WalkSpeed;	// 기본 이동 타입
	m_fOldSpeed_2 = (float) pNpcTable->RunSpeed;	// 뛰는 이동 타입..

	if (transformSpeeds)
	{
		constexpr float dbSpeed = MONSTER_SPEED;

		m_fSpeed_1		*= (dbSpeed / 1000.0f);		// 기본 이동 타입
		m_fSpeed_2		*= (dbSpeed / 1000.0f);		// 뛰는 이동 타입..
		m_fOldSpeed_1	*= (dbSpeed / 1000.0f);		// 기본 이동 타입
		m_fOldSpeed_2	*= (dbSpeed / 1000.0f);		// 뛰는 이동 타입..
	}

	m_sStandTime = pNpcTable->StandTime;			// 서있는 시간
	m_iMagic1 = pNpcTable->Magic1;					// 사용마법 1
	m_iMagic2 = pNpcTable->Magic2;					// 사용마법 2
	m_iMagic3 = pNpcTable->Magic3;					// 사용마법 3
	m_sFireR = pNpcTable->FireResist;				// 화염 저항력
	m_sColdR = pNpcTable->ColdResist;				// 냉기 저항력
	m_sLightningR = pNpcTable->LightningResist;		// 전기 저항력
	m_sMagicR = pNpcTable->MagicResist;				// 마법 저항력
	m_sDiseaseR = pNpcTable->DiseaseResist;			// 저주 저항력
	m_sPoisonR = pNpcTable->PoisonResist;			// 독 저항력
	m_sLightR = pNpcTable->LightResist;				// 빛 저항력
	m_fBulk = (float) (((double) pNpcTable->Bulk / 100) * ((double) pNpcTable->Size / 100));
	m_bySearchRange = pNpcTable->SearchRange;		// 적 탐지 범위
	m_byAttackRange = pNpcTable->AttackRange;		// 사정거리
	m_byTracingRange = pNpcTable->TracingRange;		// 추격거리
	m_tNpcType = pNpcTable->Type;					// NPC Type
	m_byFamilyType = pNpcTable->Family;				// 몹들사이에서 가족관계를 결정한다.
	m_iMoney = pNpcTable->Money;					// 떨어지는 돈
	m_iItem = pNpcTable->Item;						// 떨어지는 아이템
	m_tNpcLongType = pNpcTable->DirectAttack;
	m_byWhatAttackType = pNpcTable->DirectAttack;
}

//	NPC 기본정보 초기화
void CNpc::Init()
{
	if (m_ZoneIndex == -1)
		m_ZoneIndex = m_pMain->GetZoneIndex(m_sCurZone);

	m_Delay = 0;
	m_fDelayTime = TimeGet();

	MAP* pMap = m_pMain->GetMapByIndex(m_ZoneIndex);
	if (pMap == nullptr)
	{
		spdlog::error("Npc::Init: map not found [zoneIndex={} npcId={} npcName={}]",
					m_strName, m_sSid, m_ZoneIndex);
		return;
	}

	m_pOrgMap = pMap->m_pMap;	// MapInfo 정보 셋팅
}

//	NPC 기본위치 정보 초기화(패스 따라 움직이는 NPC의 진형을 맞추어 준다..
void CNpc::InitPos()
{
	float fDD = 1.5f;
	if (m_byBattlePos == 0)
	{
		m_fBattlePos_x = 0.0f;
		m_fBattlePos_z = 0.0f;
	}
	else if (m_byBattlePos == 1)
	{
		float fx[5] = { 0.0f, -(fDD * 2),  -(fDD * 2), -(fDD * 4),  -(fDD * 4) };
		float fz[5] = { 0.0f,  (fDD * 1),  -(fDD * 1),  (fDD * 1),  -(fDD * 1) };
		m_fBattlePos_x = fx[m_byPathCount - 1];
		m_fBattlePos_z = fz[m_byPathCount - 1];
	}
	else if (m_byBattlePos == 2)
	{
		float fx[5] = { 0.0f,  0.0f, -(fDD * 2), -(fDD * 2), -(fDD * 2) };
		float fz[5] = { 0.0f, -(fDD * 2), (fDD * 1), (fDD * 1), (fDD * 3) };
		m_fBattlePos_x = fx[m_byPathCount - 1];
		m_fBattlePos_z = fz[m_byPathCount - 1];
	}
	else if (m_byBattlePos == 3)
	{
		float fx[5] = { 0.0f, -(fDD * 2),  -(fDD * 2), -(fDD * 2), -(fDD * 4) };
		float fz[5] = { 0.0f,  (fDD * 2),   0.0f,    -(fDD * 2),  0.0f };
		m_fBattlePos_x = fx[m_byPathCount - 1];
		m_fBattlePos_z = fz[m_byPathCount - 1];
	}
}

void CNpc::InitMagicValuable()
{
	for (int i = 0; i < MAX_MAGIC_TYPE4; i++)
	{
		m_MagicType4[i].byAmount = 100;
		m_MagicType4[i].sDurationTime = 0;
		m_MagicType4[i].fStartTime = 0.0f;
	}

	for (int i = 0; i < MAX_MAGIC_TYPE3; i++)
	{
		m_MagicType3[i].sHPAttackUserID = -1;
		m_MagicType3[i].sHPAmount = 0;
		m_MagicType3[i].byHPDuration = 0;
		m_MagicType3[i].byHPInterval = 2;
		m_MagicType3[i].fStartTime = 0.0f;
	}
}

// NPC 상태별로 분화한다.
void CNpc::NpcLive(CIOCPort* pIOCP)
{
	// Dungeon Work : 변하는 몬스터의 경우 변하게 처리..
	if (m_byRegenType == 2
		// 리젠이 되지 못하도록,,, 
		|| (m_byRegenType == 1 && m_byChangeType == 100))
	{
		m_NpcState = NPC_LIVE;
		m_Delay = m_sRegenTime;
		m_fDelayTime = TimeGet();
		return;
	}

	// 몬스터의 정보를 바꾸어 준다..
	if (m_byChangeType == 1)
	{
		m_byChangeType = 2;
		ChangeMonsterInfo(1);
	}

	if (SetLive(pIOCP))
	{
		m_NpcState = NPC_STANDING;
		m_Delay = m_sStandTime;
		m_fDelayTime = TimeGet();
	}
	else
	{
		m_NpcState = NPC_LIVE;
		m_Delay = m_sStandTime;
		m_fDelayTime = TimeGet();
	}
}

/////////////////////////////////////////////////////////////////////////////
//	NPC가 공격하는 경우.
//
void CNpc::NpcFighting(CIOCPort* pIOCP)
{
	NpcTrace("NpcFighting()");

	if (m_iHP <= 0)
	{
		Dead(pIOCP);
		return;
	}

	m_Delay = Attack(pIOCP);
	m_fDelayTime = TimeGet();
}

/////////////////////////////////////////////////////////////////////////////
//	NPC가 유저를 추적하는 경우.
//
void CNpc::NpcTracing(CIOCPort* pIOCP)
{
	char pBuf[1024] = {};
	int index = 0;
	float fMoveSpeed = 0.0f;

	if (m_sStepCount != 0)
	{
		if (m_fPrevX < 0
			|| m_fPrevZ < 0)
		{
			spdlog::error("Npc::NpcTracing: previous coordinates invalid [serial={} npcName={} prevX={} prevZ={}]",
				m_sNid + NPC_BAND, m_strName, m_fPrevX, m_fPrevZ);
		}
		else
		{
			m_fCurX = m_fPrevX;	
			m_fCurZ = m_fPrevZ;
		}
	}

	NpcTrace("NpcTracing()");

	// 고정 경비병은 추적이 되지 않도록한다.
	if (m_tNpcType == NPC_DOOR
		|| m_tNpcType == NPC_ARTIFACT
		|| m_tNpcType == NPC_PHOENIX_GATE
		|| m_tNpcType == NPC_GATE_LEVER
		|| m_tNpcType == NPC_DOMESTIC_ANIMAL
		|| m_tNpcType == NPC_SPECIAL_GATE
		|| m_tNpcType == NPC_DESTORY_ARTIFACT)
	{
		InitTarget();
		m_NpcState = NPC_STANDING;
		m_Delay = m_sStandTime;
		m_fDelayTime = TimeGet();
		return;
	}

	/* 작업할것
	   던젼 몬스터의 경우 일정영역을 벗어나지 못하도록 체크하는 루틴
	*/
	int nFlag = IsCloseTarget(m_byAttackRange, 1);

	// 근접전을 벌일만큼 가까운 거리인가?
	if (nFlag == 1)
	{
		//TRACE(_T("Npc-NpcTracing : trace->attack으로 바뀜 : nid=(%d, %hs), x=%.2f, z=%.2f\n"), m_sNid+NPC_BAND, m_strName, m_fCurX, m_fCurZ);
		NpcMoveEnd(pIOCP);	// 이동 끝..
		m_NpcState = NPC_FIGHTING;
		m_Delay = 0;
		m_fDelayTime = TimeGet();
		return;
	}
	else if (nFlag == -1)		// 타겟이 없어짐...
	{
		InitTarget();
		NpcMoveEnd(pIOCP);	// 이동 끝..
		m_NpcState = NPC_STANDING;
		m_Delay = m_sStandTime;
		m_fDelayTime = TimeGet();
		return;
	}
	//else if(nFlag == 2 && m_tNpcType == NPC_BOSS_MONSTER)	{
	else if (nFlag == 2
		&& m_tNpcLongType == 2)
	{
		NpcMoveEnd(pIOCP);	// 이동 끝..
		m_NpcState = NPC_FIGHTING;
		m_Delay = 0;
		m_fDelayTime = TimeGet();
		return;
	}

	if (m_byActionFlag == ATTACK_TO_TRACE)
	{
		m_byActionFlag = NO_ACTION;
		m_byResetFlag = 1;
	}

	if (m_byResetFlag == 1)
	{
		if (!ResetPath())// && !m_tNpcTraceType)
		{
			spdlog::error("Npc::NpcTracing: pathfinding failed, set state to NPC_STANDING [serial={} npcId={} npcName={}]",
				m_sNid + NPC_BAND, m_sSid, m_strName);
			InitTarget();
			NpcMoveEnd(pIOCP);	// 이동 끝..
			m_NpcState = NPC_STANDING;
			m_Delay = m_sStandTime;
			m_fDelayTime = TimeGet();
			return;
		}
	}

	if (!m_bPathFlag)
	{
//		TRACE(_T("StepMove : x=%.2f, z=%.2f\n"), m_fCurX, m_fCurZ);

		// 한칸 움직임(걷는동작, 달릴때는 2칸)
		if (!StepMove(1, pIOCP))
		{
			m_NpcState = NPC_STANDING;
			m_Delay = m_sStandTime;
			m_fDelayTime = TimeGet();
			spdlog::error("Npc::NpcTracing: StepMove failed [serial={} npcId={} npcName={}]",
				m_sNid + NPC_BAND, m_sSid, m_strName);
			return;
		}
	}
	else // if (m_bPathFlag == true)
	{
//		TRACE(_T("StepNoPathMove : x=%.2f, z=%.2f\n"), m_fCurX, m_fCurZ);
		if (!StepNoPathMove(1))	// 한칸 움직임(걷는동작, 달릴때는 2칸)
		{
			m_NpcState = NPC_STANDING;
			m_Delay = m_sStandTime;
			m_fDelayTime = TimeGet();
			spdlog::error("Npc::NpcTracing: StepNoPathMove failed [serial={} npcId={} npcName={}]",
				m_sNid + NPC_BAND, m_sSid, m_strName);
			return;
		}
	}

	// 이동이 끝났으면
	if (IsMovingEnd())
	{
		memset(pBuf, 0, sizeof(pBuf));
		index = 0;
		SetByte(pBuf, MOVE_RESULT, index);
		SetByte(pBuf, SUCCESS, index);
		SetShort(pBuf, m_sNid + NPC_BAND, index);
		Setfloat(pBuf, m_fCurX, index);
		Setfloat(pBuf, m_fCurZ, index);
		Setfloat(pBuf, m_fCurY, index);
		Setfloat(pBuf, 0, index);
		//TRACE(_T("Npc TRACE end --> nid = %d, cur=[x=%.2f, y=%.2f, metor=%d], prev=[x=%.2f, z=%.2f], frame=%d, speed = %d \n"), m_sNid, m_fCurX, m_fCurZ, 0, m_fPrevX, m_fPrevZ, m_sStepCount, 0);
		SendAll(pIOCP, pBuf, index);   // thread 에서 send
	}
	else
	{
		SetByte(pBuf, MOVE_RESULT, index);
		SetByte(pBuf, SUCCESS, index);
		SetShort(pBuf, m_sNid + NPC_BAND, index);
		Setfloat(pBuf, m_fPrevX, index);
		Setfloat(pBuf, m_fPrevZ, index);
		Setfloat(pBuf, m_fPrevY, index);
		fMoveSpeed = m_fSecForRealMoveMetor / ((double) m_sSpeed / 1000);
		Setfloat(pBuf, fMoveSpeed, index);
		//Setfloat(pBuf, m_fSecForRealMoveMetor, index);
		//TRACE(_T("Npc Tracing --> nid = %d, cur=[x=%.2f, z=%.2f], prev=[x=%.2f, z=%.2f, metor = %.2f], frame=%d, speed = %d \n"), m_sNid, m_fCurX, m_fCurZ, m_fPrevX, m_fPrevZ, m_fSecForRealMoveMetor, m_sStepCount, m_sSpeed);
		SendAll(pIOCP, pBuf, index);   // thread 에서 send
	}

	if (nFlag == 2
		&& m_tNpcLongType == 0
		&& m_tNpcType != NPC_HEALER)
	{
		// Trace Attack
		int nRet = TracingAttack(pIOCP);
		//TRACE(_T("--> Npc-NpcTracing : TracingAttack : nid=(%d, %hs), x=%.2f, z=%.2f\n"), m_sNid+NPC_BAND, m_strName, m_fCurX, m_fCurZ);
		if (nRet == 0)
		{
			InitTarget();
			NpcMoveEnd(pIOCP);	// 이동 끝..
			m_NpcState = NPC_STANDING;
			m_Delay = m_sStandTime;
			m_fDelayTime = TimeGet();
			return;
		}
	}

	m_Delay = m_sSpeed;
	m_fDelayTime = TimeGet();
}

void CNpc::NpcAttacking(CIOCPort* pIOCP)
{
	NpcTrace("NpcAttacking()");

	if (m_iHP <= 0)
	{
		Dead(pIOCP);		 // 바로 몬스터를 죽인다.. (경험치 배분 안함)
		return;
	}

//	TRACE(_T("Npc Attack - [nid=%d, sid=%d]\n"), m_sNid, m_sSid);

	int ret = IsCloseTarget(m_byAttackRange);

	// 공격할 수 있는만큼 가까운 거리인가?
	if (ret == 1)
	{
		m_NpcState = NPC_FIGHTING;
		m_Delay = 0;
		m_fDelayTime = TimeGet();
		return;
	}

	//if(m_tNpcType == NPCTYPE_DOOR || m_tNpcType == NPCTYPE_ARTIFACT || m_tNpcType == NPCTYPE_PHOENIX_GATE || m_tNpcType == NPCTYPE_GATE_LEVER)		return;		// 성문 NPC는 공격처리 안하게

	// 작업 : 이 부분에서.. 경비병도 공격이 가능하게...
	if (m_tNpcType == NPC_DOOR
		|| m_tNpcType == NPC_ARTIFACT
		|| m_tNpcType == NPC_PHOENIX_GATE
		|| m_tNpcType == NPC_GATE_LEVER
		|| m_tNpcType == NPC_DOMESTIC_ANIMAL
		|| m_tNpcType == NPC_SPECIAL_GATE
		|| m_tNpcType == NPC_DESTORY_ARTIFACT)
	{
		m_NpcState = NPC_STANDING;
		m_Delay = m_sStandTime / 2;
		m_fDelayTime = TimeGet();
		return;
	}

	int nValue = GetTargetPath();

	// 타겟이 없어지거나,, 멀어졌음으로...
	if (nValue == -1)
	{
		if (!RandomMove())
		{
			InitTarget();
			m_NpcState = NPC_STANDING;
			m_Delay = m_sStandTime;
			m_fDelayTime = TimeGet();
			return;
		}

		InitTarget();
		m_NpcState = NPC_MOVING;
		m_Delay = m_sSpeed;
		m_fDelayTime = TimeGet();
		return;
	}
	else if (nValue == 0)
	{
		m_fSecForMetor = m_fSpeed_2;			// 공격일때는 뛰는 속도로... 
		IsNoPathFind(m_fSecForMetor);			// 타겟 방향으로 바로 간다..
	}

	m_NpcState = NPC_TRACING;
	m_Delay = 0;
	m_fDelayTime = TimeGet();
}

/////////////////////////////////////////////////////////////////////////////
//	NPC가 이동하는 경우.
//
void CNpc::NpcMoving(CIOCPort* pIOCP)
{
	NpcTrace("NpcMoving()");

	char pBuf[1024] = {};
	int index = 0;
	float fMoveSpeed = 0.0f;

	if (m_iHP <= 0)
	{
		Dead(pIOCP);
		return;
	}

	if (m_sStepCount != 0)
	{
		if (m_fPrevX < 0
			|| m_fPrevZ < 0)
		{
			spdlog::error("Npc::NpcMoving: previous coordinates invalid [serial={} npcName={} prevX={} prevZ={}]",
				m_sNid + NPC_BAND, m_strName, m_fPrevX, m_fPrevZ);
		}
		else
		{
			m_fCurX = m_fPrevX;
			m_fCurZ = m_fPrevZ;
		}
	}

	// 적을 찾는다. 
	if (FindEnemy())
	{
		/*if (m_tNpcType == NPCTYPE_GUARD)
		{
			NpcMoveEnd(pIOCP);	// 이동 끝..
			m_NpcState = NPC_FIGHTING;
			m_Delay = 0;
			m_fDelayTime = TimeGet();
		}
		else*/
		{
			NpcMoveEnd(pIOCP);	// 이동 끝..
			m_NpcState = NPC_ATTACKING;
			m_Delay = m_sSpeed;
			m_fDelayTime = TimeGet();
		}
		return;
	}

	// 이동이 끝났으면
	if (IsMovingEnd())
	{
		m_fCurX = m_fPrevX;
		m_fCurZ = m_fPrevZ;

		if (m_fCurX < 0
			|| m_fCurZ < 0)
		{
			spdlog::error("Npc::NpcMoving: coordinates invalid [serial={} npcId={} npcName={} x={} z={}]",
				m_sNid + NPC_BAND, m_sSid, m_strName, m_fCurX, m_fCurZ);
		}

		int rx = m_fCurX / VIEW_DIST;
		int rz = m_fCurZ / VIEW_DIST;

		//TRACE(_T("** NpcMoving --> IsMovingEnd() 이동이 끝남,, rx=%d, rz=%d, stand로\n"), rx, rz);
		m_NpcState = NPC_STANDING;
		m_Delay = m_sStandTime;
		m_fDelayTime = TimeGet();

		if (m_Delay < 0)
		{
			m_Delay = 0;
			m_fDelayTime = TimeGet();
		}

		return;
	}

	if (!m_bPathFlag)
	{
		// 한칸 움직임(걷는동작, 달릴때는 2칸)
		if (!StepMove(1, pIOCP))
		{
			m_NpcState = NPC_STANDING;
			m_Delay = m_sStandTime;
			m_fDelayTime = TimeGet();
			return;
		}
	}
	else // if (m_bPathFlag)
	{
		// 한칸 움직임(걷는동작, 달릴때는 2칸)
		if (!StepNoPathMove(1))
		{
			m_NpcState = NPC_STANDING;
			m_Delay = m_sStandTime;
			m_fDelayTime = TimeGet();
			return;
		}
	}

	// 이동이 끝났으면
	if (IsMovingEnd())
	{
		memset(pBuf, 0, sizeof(pBuf));
		index = 0;
		SetByte(pBuf, MOVE_RESULT, index);
		SetByte(pBuf, SUCCESS, index);
		SetShort(pBuf, m_sNid + NPC_BAND, index);
		Setfloat(pBuf, m_fPrevX, index);
		Setfloat(pBuf, m_fPrevZ, index);
		Setfloat(pBuf, m_fPrevY, index);
		Setfloat(pBuf, 0, index);
		//TRACE(_T("Npc Move end --> nid = %d, cur=[x=%.2f, y=%.2f, metor=%d], prev=[x=%.2f, z=%.2f], frame=%d, speed = %d \n"), m_sNid+NPC_BAND, m_fCurX, m_fCurZ, 0, m_fPrevX, m_fPrevZ, m_sStepCount, 0);
		SendAll(pIOCP, pBuf, index);   // thread 에서 send
	}
	else
	{
		SetByte(pBuf, MOVE_RESULT, index);
		SetByte(pBuf, SUCCESS, index);
		SetShort(pBuf, m_sNid + NPC_BAND, index);
		Setfloat(pBuf, m_fPrevX, index);
		Setfloat(pBuf, m_fPrevZ, index);
		Setfloat(pBuf, m_fPrevY, index);
		fMoveSpeed = m_fSecForRealMoveMetor / ((double) m_sSpeed / 1000);
		Setfloat(pBuf, fMoveSpeed, index);
		//Setfloat(pBuf, m_fSecForRealMoveMetor, index);
		//TRACE(_T("Npc Move --> nid = %d, cur=[x=%.2f, z=%.2f], prev=[x=%.2f, z=%.2f, metor = %.2f], frame=%d, speed = %d \n"), m_sNid+NPC_BAND, m_fCurX, m_fCurZ, m_fPrevX, m_fPrevZ, m_fSecForRealMoveMetor, m_sStepCount, m_sSpeed);
		SendAll(pIOCP, pBuf, index);   // thread 에서 send
	}

	m_Delay = m_sSpeed;
	m_fDelayTime = TimeGet();
}

/////////////////////////////////////////////////////////////////////////////
//	NPC가 서있는경우.
//
void CNpc::NpcStanding()
{
	NpcTrace("NpcStanding()");

	char send_buff[128];
	int send_index = 0;

/*	if(m_pMain->m_byNight == 2)	{	// 밤이면
		m_NpcState = NPC_SLEEPING;
		m_Delay = 0;
		m_fDelayTime = TimeGet();
		return;
	}	*/

	MAP* pMap = m_pMain->GetMapByIndex(m_ZoneIndex);
	if (pMap == nullptr)
	{
		spdlog::error("Npc::NpcStanding: map not found [zoneIndex={} npcId={} npcName={}]",
					m_strName, m_sSid, m_ZoneIndex);
		return;
	}

/*	bool bCheckRange = false;
	bCheckRange = IsInRange( (int)m_fCurX, (int)m_fCurZ);
	if( bCheckRange )	{	// 활동영역안에 있다면
		if( m_tNpcAttType != m_tNpcOldAttType )	{
			m_tNpcAttType = ATROCITY_ATTACK_TYPE;	// 공격성향으로
			//TRACE(_T("공격성향이 선공으로 변함\n"));
		}
	}
	else	{
		if( m_tNpcAttType == ATROCITY_ATTACK_TYPE )	{
			m_tNpcAttType = TENDER_ATTACK_TYPE;
			//TRACE(_T("공격성향이 후공으로 변함\n"));
		}
	}	*/

	// dungeon work
	// 던젼 존인지를 먼저 판단
	CRoomEvent* pRoom = pMap->m_arRoomEventArray.GetData(m_byDungeonFamily);
	if (pRoom != nullptr)
	{
		// 방의 상태가 실행되지 않았다면,, 몬스터는 standing
		if (pRoom->m_byStatus == 1)
		{
			m_NpcState = NPC_STANDING;
			m_Delay = m_sStandTime;
			m_fDelayTime = TimeGet();
			return;
		}
	}

	if (RandomMove())
	{
		m_iAniFrameCount = 0;
		m_NpcState = NPC_MOVING;
		m_Delay = m_sStandTime;
		m_fDelayTime = TimeGet();
		return;
	}

	m_NpcState = NPC_STANDING;
	m_Delay = m_sStandTime;
	m_fDelayTime = TimeGet();

	if (m_tNpcType == NPC_SPECIAL_GATE
		&& m_pMain->m_byBattleEvent == BATTLEZONE_OPEN)
	{
		// 문이 자동으로 열리고 닫히도록(2분을 주기로-standing time을 이용)
		m_byGateOpen = !m_byGateOpen;		// 

		// client와 gameserver에 정보전송
		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;
		SetByte(send_buff, AG_NPC_GATE_OPEN, send_index);
		SetShort(send_buff, m_sNid + NPC_BAND, send_index);
		SetByte(send_buff, m_byGateOpen, send_index);
		SendAll(&m_pMain->m_Iocport, send_buff, send_index);
	}
}

/////////////////////////////////////////////////////////////////////////////
//	타겟과의 거리를 사정거리 범위로 유지한다.(셀단위)
//
void CNpc::NpcBack(CIOCPort* pIOCP)
{
	if (m_Target.id >= 0
		&& m_Target.id < NPC_BAND)
	{
		if (m_pMain->GetUserPtr((m_Target.id - USER_BAND)) == nullptr)
		{	// Target User 가 존재하는지 검사
			m_NpcState = NPC_STANDING;
			m_Delay = m_sSpeed;//STEP_DELAY;
			m_fDelayTime = TimeGet();
			return;
		}
	}
	else if (m_Target.id >= NPC_BAND
		&& m_Target.id < INVALID_BAND)
	{
		if (m_pMain->m_NpcMap.GetData(m_Target.id - NPC_BAND) == nullptr)
		{
			m_NpcState = NPC_STANDING;
			m_Delay = m_sSpeed;
			m_fDelayTime = TimeGet();
			return;
		}
	}

	char pBuf[1024] = {};
	int index = 0;

	if (m_iHP <= 0)
	{
		Dead(pIOCP);
		return;
	}

	if (m_sStepCount != 0)
	{
		if (m_fPrevX < 0
			|| m_fPrevZ < 0)
		{
			spdlog::error("Npc::NpcBack: previous coordinates invalid [serial={} npcName={} prevX={} prevZ={}]",
				m_sNid + NPC_BAND, m_strName, m_fPrevX, m_fPrevZ);
		}
		else
		{
			m_fCurX = m_fPrevX;
			m_fCurZ = m_fPrevZ;
		}
	}

	// 이동이 끝났으면
	if (IsMovingEnd())
	{
		m_fCurX = m_fPrevX;
		m_fCurZ = m_fPrevZ;

		if (m_fCurX < 0
			|| m_fCurZ < 0)
		{
			spdlog::error("Npc::NpcBack: coordinates invalid [serial={} npcId={} npcName={} x={} z={}]",
				m_sNid + NPC_BAND, m_sSid, m_strName, m_fCurX, m_fCurZ);
		}

		memset(pBuf, 0, sizeof(pBuf));
		index = 0;
		SetByte(pBuf, MOVE_RESULT, index);
		SetByte(pBuf, SUCCESS, index);
		SetShort(pBuf, m_sNid + NPC_BAND, index);
		Setfloat(pBuf, m_fCurX, index);
		Setfloat(pBuf, m_fCurZ, index);
		Setfloat(pBuf, m_fCurY, index);
		Setfloat(pBuf, 0, index);
//		TRACE(_T("NpcBack end --> nid = %d, cur=[x=%.2f, y=%.2f, metor=%d], prev=[x=%.2f, z=%.2f], frame=%d, speed = %d \n"), m_sNid, m_fCurX, m_fCurZ, 0, m_fPrevX, m_fPrevZ, m_sStepCount, 0);
		SendAll(pIOCP, pBuf, index);   // thread 에서 send

//		TRACE(_T("** NpcBack 이동이 끝남,, stand로\n"));
		m_NpcState = NPC_STANDING;

		//영역 밖에 있으면 서있는 시간을 짧게...
		m_Delay = m_sStandTime;
		m_fDelayTime = TimeGet();

		if (m_Delay < 0)
		{
			m_Delay = 0;
			m_fDelayTime = TimeGet();
		}

		return;
	}

	if (!m_bPathFlag)
	{
		if (!StepMove(1, pIOCP))
		{
			m_NpcState = NPC_STANDING;
			m_Delay = m_sStandTime;
			m_fDelayTime = TimeGet();
			return;
		}
	}
	else // if (m_bPathFlag)
	{
		if (!StepNoPathMove(1))
		{
			m_NpcState = NPC_STANDING;
			m_Delay = m_sStandTime;
			m_fDelayTime = TimeGet();
			return;
		}
	}

	float fMoveSpeed = 0.0f;

	SetByte(pBuf, MOVE_RESULT, index);
	SetByte(pBuf, SUCCESS, index);
	SetShort(pBuf, m_sNid + NPC_BAND, index);
	Setfloat(pBuf, m_fPrevX, index);
	Setfloat(pBuf, m_fPrevZ, index);
	Setfloat(pBuf, m_fPrevY, index);
	fMoveSpeed = m_fSecForRealMoveMetor / ((double) m_sSpeed / 1000);
	Setfloat(pBuf, fMoveSpeed, index);
	//Setfloat(pBuf, m_fSecForRealMoveMetor, index);

//	TRACE(_T("NpcBack --> nid = %d, cur=[x=%.2f, z=%.2f], prev=[x=%.2f, z=%.2f, metor = %.2f], frame=%d, speed = %d \n"), m_sNid, m_fCurX, m_fCurZ, m_fPrevX, m_fPrevZ, m_fSecForRealMoveMetor, m_sStepCount, m_sSpeed);
	SendAll(pIOCP, pBuf, index);   // thread 에서 send

	m_Delay = m_sSpeed;//STEP_DELAY;	*/
	m_fDelayTime = TimeGet();
}

///////////////////////////////////////////////////////////////////////
// NPC 가 처음 생기거나 죽었다가 살아날 때의 처리
//
bool CNpc::SetLive(CIOCPort* pIOCP)
{
	//TRACE(_T("**** Npc SetLive ***********\n"));
	// NPC의 HP, PP 초기화 ----------------------//	
	int i = 0, j = 0;
	m_iHP = m_iMaxHP;
	m_sMP = m_sMaxMP;
	m_sPathCount = 0;
	m_iPattenFrame = 0;
	m_byResetFlag = 0;
	m_byActionFlag = NO_ACTION;
	m_byMaxDamagedNation = KARUS_MAN;

	m_iRegion_X = m_iRegion_Z = -1;
	m_fAdd_x = m_fAdd_z = 0.0f;
	m_fStartPoint_X = m_fStartPoint_Y = 0.0f;
	m_fEndPoint_X = m_fEndPoint_Y = 0.0f;
	m_min_x = m_min_y = m_max_x = m_max_y = 0;

	InitTarget();
	ClearPathFindData();
	InitUserList();					// 타겟을위한 리스트를 초기화.
	//InitPos();

	CNpc* pNpc = nullptr;

	/* If the Event Monster respawns, kill it and null the pointer in the event thread. */
	if (m_lEventNpc == 1
		&& !m_bFirstLive)
	{
		for (int i = 0; i < NPC_NUM; i++)
		{
			pNpc = m_pMain->m_EventNpcThreadArray[0]->m_ThreadInfo.pNpc[i];
			if (m_pMain->m_EventNpcThreadArray[0]->m_ThreadInfo.pNpc[i] != nullptr)
			{
				if (m_pMain->m_EventNpcThreadArray[0]->m_ThreadInfo.pNpc[i]->m_sNid == m_sNid)
				{
					m_pMain->m_EventNpcThreadArray[0]->m_ThreadInfo.m_byNpcUsed[i] = 0;
					m_lEventNpc = 0;
					m_pMain->m_EventNpcThreadArray[0]->m_ThreadInfo.pNpc[i] = nullptr;
					spdlog::debug("Npc::SetLive: returning summoned monster pointer [threadIndex={} serial={} npcId={} npcName={}]",
						i, m_sNid + NPC_BAND, m_sSid, m_strName);
					return true;
				}
			}
		}
		return true;
	}

	// NPC 초기위치 결정 ------------------------//
	MAP* pMap = m_pMain->GetMapByIndex(m_ZoneIndex);
	if (pMap == nullptr)
	{
		spdlog::error("Npc::SetLive: map not found [zoneIndex={} npcId={} npcName={}]",
					m_strName, m_sSid, m_ZoneIndex);
		return false;
	}

	// NPC 가 처음 살아나는 경우	
	if (m_bFirstLive)
	{
		m_nInitX = m_fPrevX = m_fCurX;
		m_nInitY = m_fPrevY = m_fCurY;
		m_nInitZ = m_fPrevZ = m_fCurZ;
	}

	if (m_fCurX < 0
		|| m_fCurZ < 0)
	{
		spdlog::error("Npc::SetLive: coordinates invalid [serial={} npcId={} npcName={} x={} z={}]",
			m_sNid + NPC_BAND, m_sSid, m_strName, m_fCurX, m_fCurZ);
	}

	int dest_x = (int) m_nInitX / TILE_SIZE;
	int dest_z = (int) m_nInitZ / TILE_SIZE;

	bool bMove = pMap->IsMovable(dest_x, dest_z);

	if (m_tNpcType != NPCTYPE_MONSTER
		|| m_lEventNpc == 1)
	{
		m_fCurX = m_fPrevX = m_nInitX;
		m_fCurY = m_fPrevY = m_nInitY;
		m_fCurZ = m_fPrevZ = m_nInitZ;
	}
	else
	{
		int nX = 0;
		int nZ = 0;
		int nTileX = 0;
		int nTileZ = 0;
		int nRandom = 0;
		uint16_t retryCount = 0;
		uint16_t maxRetry = 500;

		while (1)
		{
			nRandom = abs(m_nInitMinX - m_nInitMaxX);
			if (nRandom <= 1)
			{
				nX = m_nInitMinX;
			}
			else
			{
				if (m_nInitMinX < m_nInitMaxX)
					nX = myrand(m_nInitMinX, m_nInitMaxX);
				else
					nX = myrand(m_nInitMaxX, m_nInitMinX);
			}

			nRandom = abs(m_nInitMinY - m_nInitMaxY);
			if (nRandom <= 1)
			{
				nZ = m_nInitMinY;
			}
			else
			{
				if (m_nInitMinY < m_nInitMaxY)
					nZ = myrand(m_nInitMinY, m_nInitMaxY);
				else
					nZ = myrand(m_nInitMaxY, m_nInitMinY);
			}

			nTileX = nX / TILE_SIZE;
			nTileZ = nZ / TILE_SIZE;

			if (nTileX >= (pMap->m_sizeMap.cx - 1))
				nTileX = pMap->m_sizeMap.cx - 1;
			if (nTileZ >= (pMap->m_sizeMap.cy - 1))
				nTileZ = pMap->m_sizeMap.cy - 1;

			if (nTileX < 0
				|| nTileZ < 0)
			{
				spdlog::error("Npc::SetLive: tile coordinates invalid [serial={} npcId={} npcName={} tileX={} tileZ={}]",
					m_sNid + NPC_BAND, m_sSid, m_strName, nTileX, nTileZ);
				return false;
			}

			if (pMap->m_pMap[nTileX][nTileZ].m_sEvent <= 0)
			{
				if (retryCount >= maxRetry)
				{
					m_nInitX = m_fPrevX = m_fCurX;
					m_nInitY = m_fPrevY = m_fCurY;
					m_nInitZ = m_fPrevZ = m_fCurZ;
					spdlog::error("Npc::SetLive: failed to spawn NPC, max retries exceeded [npcId={} serial={} zoneId={} retryCount={} x={} z={}]",
						m_sSid, m_sNid + NPC_BAND, m_sCurZone, retryCount, nX, nZ);
					return false;
				}
				retryCount++;
				continue;
			}

			m_nInitX = m_fPrevX = m_fCurX = (float) nX;
			m_nInitZ = m_fPrevZ = m_fCurZ = (float) nZ;

			if (m_fCurX < 0 || m_fCurZ < 0)
			{
				spdlog::error("Npc::SetLive: post-move coordinates invalid [serial={} npcId={} npcName={} x={} z={}]",
					m_sNid + NPC_BAND, m_sSid, m_strName, m_fCurX, m_fCurZ);
			}

			break;
		}
	}

	//SetUid(m_fCurX, m_fCurZ, m_sNid + NPC_BAND);

	// 상태이상 정보 초기화
	m_fHPChangeTime = TimeGet();
	m_fFaintingTime = 0.0f;
	InitMagicValuable();

	// NPC 가 처음 살아나는 경우
	if (m_bFirstLive)
	{
		NpcTypeParser();
		m_bFirstLive = false;

		InterlockedIncrement(&m_pMain->m_CurrentNPC);

		//TRACE(_T("Npc - SerLive :  cur = %d\n"), m_pMain->m_CurrentNPC);

		// 몬스터 총 수와 초기화한 몬스터의 수가 같다면
		if (m_pMain->m_TotalNPC == m_pMain->m_CurrentNPC)
		{
			std::string logstr = fmt::format("All NPCs initialized [count={}]",
				m_pMain->m_CurrentNPC);
			m_pMain->AddOutputMessage(logstr);
			spdlog::info("Npc::SetLive: {}", logstr);
			m_pMain->GameServerAcceptThread();				// 게임서버 Accept
		}
		//TRACE(_T("Npc - SerLive : CurrentNpc = %d\n"), m_pMain->m_CurrentNPC);
	}

	// 해야 할 일 : Npc의 초기 방향,, 결정하기..
	// Npc인 경우 초기 방향이 중요함으로써..
	if (m_byMoveType == 3
		&& m_sMaxPathCount == 2)
	{
		__Vector3 vS, vE, vDir;
		float fDir;
		vS.Set((float) m_PathList.pPattenPos[0].x, 0, (float) m_PathList.pPattenPos[0].z);
		vE.Set((float) m_PathList.pPattenPos[1].x, 0, (float) m_PathList.pPattenPos[1].z);
		vDir = vE - vS;
		vDir.Normalize();
		Yaw2D(vDir.x, vDir.z, fDir);

		m_byDirection = (uint8_t) fDir;
	}

	// 처음에 죽어있다가 살아나는 몬스터
	if (m_bySpecialType == 5
		&& m_byChangeType == 0)
		return false;

	// 몬스터의 출현,,,
	if (m_bySpecialType == 5
		&& m_byChangeType == 3)
	{
		char notify[50] = {};
		//wsprintf( notify, "** 알림 : %hs 몬스터가 출현하였습니다 **", m_strName);
		//m_pMain->SendSystemMsg( notify, m_sCurZone, PUBLIC_CHAT, SEND_ALL);
	}

	SetUid(m_fCurX, m_fCurZ, m_sNid + NPC_BAND);
	m_byDeadType = 0;
	spdlog::trace("Npc::SetLive: NPC initialized [serial={} npcId={} threadNumber={} npcName={} x={} z={} gateOpen={} deadType={}]",
		m_sNid + NPC_BAND, m_sSid, m_sThreadNumber, m_strName, m_fCurX, m_fCurZ, m_byGateOpen, m_byDeadType);
	// 유저에게 NPC 정보전송...
	int modify_index = 0;
	char modify_send[2048] = {};

	FillNpcInfo(modify_send, modify_index, INFO_MODIFY);
	SendAll(pIOCP, modify_send, modify_index);   // thread 에서 send

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//	주변에 적이 없거나 수동몹의 경우 임의의 점으로 길찾기를 한 후 움직인다.
//
bool CNpc::RandomMove()
{
	// 보통이동일때는 걷는 속도로 맞추어준다...
	m_fSecForMetor = m_fSpeed_1;

	if (m_bySearchRange == 0)
		return false;

	// 제자리에서,, 서있는 npc
	if (m_byMoveType == 0)
		return false;

	/* 이곳에서 영역 검사해서 npc의 가시거리에 유저가 하나도 없다면 standing상태로...
	  있다면 패턴이나,, 노드를 따라서 행동하게 처리...  */
	if (!GetUserInView())
		return false;

	float fDestX = -1.0f, fDestZ = -1.0f;

	MAP* pMap = m_pMain->GetMapByIndex(m_ZoneIndex);
	if (pMap == nullptr)
		return false;

	int x = 0, y = 0;

	__Vector3 vStart, vEnd, vNewPos;
	float fDis = 0.0f;

	int nPathCount = 0;

	int random_x = 0, random_z = 0;
	bool bPeedBack = false;

	// 랜덤하게 조금씩 움직이는 NPC
	if (m_byMoveType == 1)
	{
		bPeedBack = IsInRange((int) m_fCurX, (int) m_fCurZ);
		if (!bPeedBack)
		{
			//TRACE(_T("초기위치를 벗어났군,,,  patten=%d \n"), m_iPattenFrame);
		}

		// 처음위치로 돌아가도록...
		if (m_iPattenFrame == 0)
		{
			m_pPattenPos.x = (short) m_nInitX;
			m_pPattenPos.z = (short) m_nInitZ;
		}

		random_x = myrand(3, 7);
		random_z = myrand(3, 7);

		fDestX = m_fCurX + (float) random_x;
		fDestZ = m_fCurZ + (float) random_z;

		if (m_iPattenFrame == 2)
		{
			fDestX = m_pPattenPos.x;
			fDestZ = m_pPattenPos.z;
			m_iPattenFrame = 0;
		}
		else
		{
			m_iPattenFrame++;
		}

		vStart.Set(m_fCurX, m_fCurY, m_fCurZ);
		vEnd.Set(fDestX, 0, fDestZ);
		fDis = GetDistance(vStart, vEnd);

		// 초기유효거리 50m를 벗어난 경우
		if (fDis > 50)
		{
			vNewPos = GetVectorPosition(vStart, vEnd, 40);
			fDestX = vNewPos.x;
			fDestZ = vNewPos.z;
			m_iPattenFrame = 2;
			bPeedBack = true;
			//TRACE(_T("&&& RandomMove 초기위치 이탈.. %d,%hs ==> x=%.2f, z=%.2f,, init_x=%.2f, init_z=%.2f\n"), m_sNid+NPC_BAND, m_strName, fDestX, fDestZ, m_nInitX, m_nInitZ); 
		}

//		TRACE(_T("&&& RandomMove ==> x=%.2f, z=%.2f,, dis=%.2f, patten = %d\n"), fDestX, fDestZ, fDis, m_iPattenFrame); 
	}
	// PathLine을 따라서 움직이는 NPC	
	else if (m_byMoveType == 2)
	{ 
		if (m_sPathCount == m_sMaxPathCount)
			m_sPathCount = 0;

		// 나의 위치가,, 패스 리스트에서 멀어졌다면,, 현재의 m_sPathCount나 다음의 m_sPathCount의 위치를 
		// 판단해서 가까운 위치로 길찾기를 한다,,
		if (m_sPathCount != 0
			&& !IsInPathRange())
		{
			m_sPathCount--;
			nPathCount = GetNearPathPoint();

			// If the NPC moves to a location that is unreachable,
			// force it to return to the beginning of its path.
			if (nPathCount == -1)
			{
				spdlog::debug("Npc::RandomMove: unreachable path point, returning to beginning of path. [serial={} npcId={} pathCount={} maxPathCount={}]",
					m_sNid + NPC_BAND, m_sSid, m_sPathCount, m_sMaxPathCount);
				
				// Force the NPC to move 40 meters towards the beginning of its path
				vStart.Set(m_fCurX, m_fCurY, m_fCurZ);
				fDestX = (float) m_PathList.pPattenPos[0].x + m_fBattlePos_x;
				fDestZ = (float) m_PathList.pPattenPos[0].z + m_fBattlePos_z;
				vEnd.Set(fDestX, 0, fDestZ);
				vNewPos = GetVectorPosition(vStart, vEnd, 40);
				fDestX = vNewPos.x;
				fDestZ = vNewPos.z;
				//m_sPathCount++;
				//return false;	// 지금은 standing상태로..
			}
			else
			{
				//m_byPathCount; 번호를 더해주기
				if (nPathCount < 0)
					return false;

				fDestX = (float) m_PathList.pPattenPos[nPathCount].x + m_fBattlePos_x;
				fDestZ = (float) m_PathList.pPattenPos[nPathCount].z + m_fBattlePos_z;
				m_sPathCount = nPathCount;
			}
		}
		else
		{
			if (m_sPathCount < 0)
				return false;

			fDestX = (float) m_PathList.pPattenPos[m_sPathCount].x + m_fBattlePos_x;
			fDestZ = (float) m_PathList.pPattenPos[m_sPathCount].z + m_fBattlePos_z;
		}

		//TRACE(_T("RandomMove 길따라 이동 : [nid=%d, sid=%d, name=%hs], path=%d/%d, nPathCount=%d, curx=%.2f, z=%.2f -> dest_X=%.2f, z=%.2f\n"), m_sNid+NPC_BAND, m_sSid, m_strName, m_sPathCount, m_sMaxPathCount, nPathCount, m_fCurX, m_fCurZ, fDestX, fDestZ);
		m_sPathCount++;
	}
	// PathLine을 따라서 움직이는 NPC
	else if (m_byMoveType == 3)
	{
		if (m_sPathCount == m_sMaxPathCount)
		{
			m_byMoveType = 0;
			m_sPathCount = 0;
			return false;
		}

		// 나의 위치가,, 패스 리스트에서 멀어졌다면,, 현재의 m_sPathCount나 다음의 m_sPathCount의 위치를 
		// 판단해서 가까운 위치로 길찾기를 한다,,
		if (m_sPathCount != 0
			&& !IsInPathRange())
		{
			m_sPathCount--;
			nPathCount = GetNearPathPoint();

			// If the NPC moves to a location that is unreachable,
			// force it to return to the beginning of its path.
			if (nPathCount == -1)
			{
				spdlog::debug("Npc::RandomMove: unreachable path point, returning to beginning of path. [serial={} npcId={} pathCount={} maxPathCount={}]",
					m_sNid + NPC_BAND, m_sSid, m_sPathCount, m_sMaxPathCount);

				// Force the NPC to move 40 meters towards the beginning of its path
				vStart.Set(m_fCurX, m_fCurY, m_fCurZ);
				fDestX = (float) m_PathList.pPattenPos[0].x + m_fBattlePos_x;
				fDestZ = (float) m_PathList.pPattenPos[0].z + m_fBattlePos_z;
				vEnd.Set(fDestX, 0, fDestZ);
				vNewPos = GetVectorPosition(vStart, vEnd, 40);
				fDestX = vNewPos.x;
				fDestZ = vNewPos.z;
				//return false;	// 지금은 standing상태로..
			}
			else
			{
				if (nPathCount < 0)
					return false;

				fDestX = (float) m_PathList.pPattenPos[nPathCount].x + m_fBattlePos_x;
				fDestZ = (float) m_PathList.pPattenPos[nPathCount].z + m_fBattlePos_x;
				m_sPathCount = nPathCount;
			}
		}
		else
		{
			if (m_sPathCount < 0)
				return false;

			fDestX = (float) m_PathList.pPattenPos[m_sPathCount].x + m_fBattlePos_x;
			fDestZ = (float) m_PathList.pPattenPos[m_sPathCount].z + m_fBattlePos_x;
		}

		m_sPathCount++;
	}

	vStart.Set(m_fCurX, 0, m_fCurZ);
	vEnd.Set(fDestX, 0, fDestZ);

	int mapMaxX = (pMap->m_sizeMap.cx-1) * pMap->m_fUnitDist;
	int mapMaxZ = (pMap->m_sizeMap.cy-1) * pMap->m_fUnitDist;
	if (!pMap->IsValidPosition(m_fCurX, m_fCurZ))
	{
		spdlog::error("Npc::RandomMove: coordinates invalid [serial={} npcName={} x={} z={} destX={} destZ={} mapBounds=[x:{} z:{}]]",
				m_sNid + NPC_BAND, m_strName, m_fCurX, m_fCurZ, fDestX, fDestZ, mapMaxX, mapMaxZ);
		return false;
	}

	if (!pMap->IsValidPosition(fDestX, fDestZ))
	{
		spdlog::error("Npc::RandomMove: destination coordinates invalid [serial={} npcName={} x={} z={} destX={} destZ={} mapBounds=[x:{} z:{}]]",
				m_sNid + NPC_BAND, m_strName, m_fCurX, m_fCurZ, fDestX, fDestZ, mapMaxX, mapMaxZ);
		return false;
	}
	

	// 작업할것 :	 던젼 몬스터의 경우 일정영역을 벗어나지 못하도록 체크하는 루틴 	
	if (m_tNpcType == NPC_DUNGEON_MONSTER)
	{
		if (!IsInRange((int) fDestX, (int) fDestZ))
			return false;
	}

	fDis = GetDistance(vStart, vEnd);

	// 100미터 보다 넓으면 스탠딩상태로..
	if (fDis > NPC_MAX_MOVE_RANGE)
	{
		if (m_byMoveType == 2
			|| m_byMoveType == 3)
		{
			m_sPathCount--;
			if (m_sPathCount <= 0)
				m_sPathCount = 0;
		}

		spdlog::error("Npc::RandomMove: tried to move further than max move distance [serial={} npcId={} npcName={} distance={}]",
			m_sNid + NPC_BAND, m_sSid, m_strName, fDis);
		return false;
	}

	// 이동거리 안에 목표점이 있다면 바로 이동하게 처리...
	if (fDis <= m_fSecForMetor)
	{
		ClearPathFindData();

		m_fStartPoint_X = m_fCurX;
		m_fStartPoint_Y = m_fCurZ;
		m_fEndPoint_X = fDestX;
		m_fEndPoint_Y = fDestZ;
		m_bPathFlag = true;
		m_iAniFrameIndex = 1;
		m_pPoint[0].fXPos = m_fEndPoint_X;
		m_pPoint[0].fZPos = m_fEndPoint_Y;
		//TRACE(_T("** Npc Random Direct Move  : [nid = %d], fDis <= %d, %.2f **\n"), m_sNid, m_fSecForMetor, fDis);
		return true;
	}

	// 일시적으로 보정한다.
	float fTempRange = (float) fDis + 2;
	int min_x = (int) (m_fCurX - fTempRange) / TILE_SIZE;
	if (min_x < 0)
		min_x = 0;

	int min_z = (int) (m_fCurZ - fTempRange) / TILE_SIZE;
	if (min_z < 0)
		min_z = 0;

	int max_x = (int) (m_fCurX + fTempRange) / TILE_SIZE;
	if (max_x >= pMap->m_sizeMap.cx)
		max_x = pMap->m_sizeMap.cx-1;

	int max_z = (int) (m_fCurZ + fTempRange) / TILE_SIZE;
	if (min_z >= pMap->m_sizeMap.cy)
		min_z = pMap->m_sizeMap.cy-1;

	CPoint start, end;
	start.x = (int) (m_fCurX / TILE_SIZE) - min_x;
	start.y = (int) (m_fCurZ / TILE_SIZE) - min_z;
	end.x = (int) (fDestX / TILE_SIZE) - min_x;
	end.y = (int) (fDestZ / TILE_SIZE) - min_z;

	if (start.x < 0
		|| start.y < 0
		|| end.x < 0
		|| end.y < 0)
		return false;

	m_fStartPoint_X = m_fCurX;
	m_fStartPoint_Y = m_fCurZ;
	m_fEndPoint_X = fDestX;
	m_fEndPoint_Y = fDestZ;

	m_min_x = min_x;
	m_min_y = min_z;
	m_max_x = max_x;
	m_max_y = max_z;

	// 패스를 따라 가는 몬스터나 NPC는 패스파인딩을 하지않고 실좌표로 바로 이동..
	if (m_byMoveType == 2
		|| m_byMoveType == 3
		|| bPeedBack)
	{
		IsNoPathFind(m_fSecForMetor);
		return true;
	}

	int nValue = PathFind(start, end, m_fSecForMetor);
	if (nValue == 1)
		return true;

	return false;
}

// Target User와 반대 방향으로 랜덤하게 움직인다.
bool CNpc::RandomBackMove()
{
	// 도망갈때도.. 속도를 뛰는 속도로 맞추어준다..
	m_fSecForMetor = m_fSpeed_2;

	if (m_bySearchRange == 0)
		return false;

	MAP* pMap = m_pMain->GetMapByIndex(m_ZoneIndex);
	if (pMap == nullptr)
	{
		spdlog::error("Npc::RandomBackMove: map not found [zoneIndex={} npcId={} npcName={}]",
		m_ZoneIndex, m_sSid, m_strName);
		return false;
	}

	float fDestX = -1.0f, fDestZ = -1.0f;

	int max_xx = pMap->m_sizeMap.cx;
	int max_zz = pMap->m_sizeMap.cy;
	int x = 0, y = 0;

	// 일시적으로 보정한다.
	float fTempRange = (float) m_bySearchRange * 2;
	int min_x = (int) (m_fCurX - fTempRange) / TILE_SIZE;
	if (min_x < 0)
		min_x = 0;

	int min_z = (int) (m_fCurZ - fTempRange) / TILE_SIZE;
	if (min_z < 0)
		min_z = 0;

	int max_x = (int) (m_fCurX + fTempRange) / TILE_SIZE;
	if (max_x >= max_xx)
		max_x = max_xx - 1;

	int max_z = (int) (m_fCurZ + fTempRange) / TILE_SIZE;
	if (max_z >= max_zz)
		max_z = max_zz - 1;

	__Vector3 vStart, vEnd, vEnd22;
	float fDis = 0.0f;
	vStart.Set(m_fCurX, m_fCurY, m_fCurZ);

	// Target 을 구한다.
	int nID = m_Target.id;
	CUser* pUser = nullptr;

	int iDir = 0;

	int iRandomX = 0, iRandomZ = 0, iRandomValue = 0;
	iRandomValue = rand() % 2;

	// Target 이 User 인 경우
	if (nID >= USER_BAND
		&& nID < NPC_BAND)
	{
		pUser = m_pMain->GetUserPtr(nID - USER_BAND);
		if (pUser == nullptr)
			return false;

		// 도주할 방향을 결정,,  먼저 x축으로
		if ((int) pUser->m_curx != (int) m_fCurX)
		{
			iRandomX = myrand((int) m_bySearchRange, (int) (m_bySearchRange * 1.5));
			iRandomZ = myrand(0, (int) m_bySearchRange);

			if ((int) pUser->m_curx > (int) m_fCurX)
				iDir = 1;
			else
				iDir = 2;
		}
		// z축으로
		else
		{
			iRandomZ = myrand((int) m_bySearchRange, (int) (m_bySearchRange * 1.5));
			iRandomX = myrand(0, (int) m_bySearchRange);
			if ((int) pUser->m_curz > (int) m_fCurZ)
				iDir = 3;
			else
				iDir = 4;
		}

		switch (iDir)
		{
			case 1:
				fDestX = m_fCurX - iRandomX;
				if (iRandomValue == 0)
					fDestZ = m_fCurZ - iRandomX;
				else
					fDestZ = m_fCurZ + iRandomX;
				break;

			case 2:
				fDestX = m_fCurX + iRandomX;
				if (iRandomValue == 0)
					fDestZ = m_fCurZ - iRandomX;
				else
					fDestZ = m_fCurZ + iRandomX;
				break;

			case 3:
				fDestZ = m_fCurZ - iRandomX;
				if (iRandomValue == 0)
					fDestX = m_fCurX - iRandomX;
				else
					fDestX = m_fCurX + iRandomX;
				break;

			case 4:
				fDestZ = m_fCurZ - iRandomX;
				if (iRandomValue == 0)
					fDestX = m_fCurX - iRandomX;
				else
					fDestX = m_fCurX + iRandomX;
				break;
		}

		vEnd.Set(fDestX, 0, fDestZ);
		fDis = GetDistance(vStart, vEnd);

		// 20미터 이상이면 20미터로 맞춘다,,
		if (fDis > 20)
		{
			vEnd22 = GetVectorPosition(vStart, vEnd, 20);
			fDestX = vEnd22.x;
			fDestZ = vEnd22.z;
		}
	}
	// Target 이 Npc 인 경우
	else if (nID >= NPC_BAND
		&& m_Target.id < INVALID_BAND)
	{
	}

	CPoint start, end;
	start.x = (int) (m_fCurX / TILE_SIZE) - min_x;
	start.y = (int) (m_fCurZ / TILE_SIZE) - min_z;
	end.x = (int) (fDestX / TILE_SIZE) - min_x;
	end.y = (int) (fDestZ / TILE_SIZE) - min_z;

	if (start.x < 0
		|| start.y < 0
		|| end.x < 0
		|| end.y < 0)
		return false;

	m_fStartPoint_X = m_fCurX;
	m_fStartPoint_Y = m_fCurZ;
	m_fEndPoint_X = fDestX;
	m_fEndPoint_Y = fDestZ;

	m_min_x = min_x;
	m_min_y = min_z;
	m_max_x = max_x;
	m_max_y = max_z;

	int nValue = PathFind(start, end, m_fSecForMetor);
	if (nValue == 1)
		return true;

	return false;
}

bool CNpc::IsInPathRange()
{
	if (m_sNid == test_id)
	{
		int x = 0;
	}

	float fPathRange = 40.0f;	// 오차 패스 범위 
	__Vector3 vStart, vEnd;
	float fDistance = 0.0f;
	vStart.Set(m_fCurX, m_fCurY, m_fCurZ);

	if (m_sPathCount < 0)
		return false;

	vEnd.Set(
		(float) m_PathList.pPattenPos[m_sPathCount].x + m_fBattlePos_x,
		0,
		(float) m_PathList.pPattenPos[m_sPathCount].z + m_fBattlePos_z);

	fDistance = GetDistance(vStart, vEnd);

	if ((int) fDistance <= (int) fPathRange + 1)
		return true;

	return false;
}

int CNpc::GetNearPathPoint()
{
	__Vector3 vStart, vEnd;
	float fMaxPathRange = (float) NPC_MAX_MOVE_RANGE;
	float fDis1 = 0.0f, fDis2 = 0.0f;
	int nRet = -1;
	vStart.Set(m_fCurX, m_fCurY, m_fCurZ);
	vEnd.Set(
		(float) m_PathList.pPattenPos[m_sPathCount].x + m_fBattlePos_x,
		0,
		(float) m_PathList.pPattenPos[m_sPathCount].z + m_fBattlePos_z);

	fDis1 = GetDistance(vStart, vEnd);

	if (m_sPathCount + 1 >= m_sMaxPathCount)
	{
		if (m_sPathCount - 1 > 0)
		{
			vEnd.Set((float) m_PathList.pPattenPos[m_sPathCount - 1].x + m_fBattlePos_x, 0, (float) m_PathList.pPattenPos[m_sPathCount - 1].z + m_fBattlePos_z);
			fDis2 = GetDistance(vStart, vEnd);
		}
		else
		{
			vEnd.Set((float) m_PathList.pPattenPos[0].x + m_fBattlePos_x, 0, (float) m_PathList.pPattenPos[0].z + m_fBattlePos_z);
			fDis2 = GetDistance(vStart, vEnd);
		}
	}
	else
	{
		vEnd.Set((float) m_PathList.pPattenPos[m_sPathCount + 1].x + m_fBattlePos_x, 0, (float) m_PathList.pPattenPos[m_sPathCount + 1].z + m_fBattlePos_z);
		fDis2 = GetDistance(vStart, vEnd);
	}

	if (fDis1 <= fDis2)
	{
		if (fDis1 <= fMaxPathRange)
			nRet = m_sPathCount;
	}
	else
	{
		if (fDis2 <= fMaxPathRange)
			nRet = m_sPathCount + 1;
	}

	return nRet;
}

/////////////////////////////////////////////////////////////////////////////////////
//	NPC 가 초기 생성위치 안에 있는지 검사
//
bool CNpc::IsInRange(int nX, int nZ)
{
	// NPC 가 초기 위치를 벗어났는지 판단한다.
	bool bFlag_1 = false, bFlag_2 = false;
	if (m_nLimitMinX < m_nLimitMaxX)
	{
		if (COMPARE(nX, m_nLimitMinX, m_nLimitMaxX))
			bFlag_1 = true;
	}
	else
	{
		if (COMPARE(nX, m_nLimitMaxX, m_nLimitMinX))
			bFlag_1 = true;
	}

	if (m_nLimitMinZ < m_nLimitMaxZ)
	{
		if (COMPARE(nZ, m_nLimitMinZ, m_nLimitMaxZ))
			bFlag_2 = true;
	}
	else
	{
		if (COMPARE(nZ, m_nLimitMaxZ, m_nLimitMinZ))
			bFlag_2 = true;
	}

	if (bFlag_1
		&& bFlag_2)
		return true;

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
//	PathFind 를 수행한다.
//
int CNpc::PathFind(CPoint start, CPoint end, float fDistance)
{
	ClearPathFindData();

	if (start.x < 0
		|| start.y < 0
		|| end.x < 0
		|| end.y < 0)
		return -1;

	// 같은 타일 안에서,, 조금 움직임이 있었다면,,
	if (start.x == end.x
		&& start.y == end.y)
	{
		m_bPathFlag = true;
		m_iAniFrameIndex = 1;
		m_pPoint[0].fXPos = m_fEndPoint_X;
		m_pPoint[0].fZPos = m_fEndPoint_Y;
		spdlog::trace("Npc::PathFind: minimal movement.... [serial={} npcId={} npcName={} x={} z={}]",
			m_sNid + NPC_BAND, m_sSid, m_strName,m_pPoint[0].fXPos, m_pPoint[0].fZPos);
		return 1;
	}

	// 여기에서 패스파인드를 실행할건지.. 바로 목표점으로 갈건인지를 판단..
	if (IsPathFindCheck(fDistance))
	{
		m_bPathFlag = true;
		return 1;
	}


	int i, j;
	int min_x, max_x;
	int min_y, max_y;

	min_x = m_min_x;
	min_y = m_min_y;
	max_x = m_max_x;
	max_y = m_max_y;

	m_vMapSize.cx = max_x - min_x + 1;
	m_vMapSize.cy = max_y - min_y + 1;

	for (i = 0; i < m_vMapSize.cy; i++)
	{
		for (j = 0; j < m_vMapSize.cx; j++)
		{
			if ((min_x + j) < 0
				|| (min_y + i) < 0)
				return 0;

			if (m_pOrgMap[min_x + j][min_y + i].m_sEvent == 0)
			{
				if ((j * m_vMapSize.cy + i) < 0)
					return 0;

				m_pMap[j * m_vMapSize.cy + i] = 1;
			}
			else
			{
				if ((j * m_vMapSize.cy + i) < 0)
					return 0;

				m_pMap[j * m_vMapSize.cy + i] = 0;
			}
		}
	}

	m_pPath = nullptr;
	m_vPathFind.SetMap(m_vMapSize.cx, m_vMapSize.cy, m_pMap);
	m_pPath = m_vPathFind.FindPath(end.x, end.y, start.x, start.y);
	int count = 0;

	while (m_pPath)
	{
		m_pPath = m_pPath->Parent;
		if (m_pPath == nullptr)
			break;

		m_pPoint[count].pPoint.x = m_pPath->x + min_x;
		m_pPoint[count].pPoint.y = m_pPath->y + min_y;
		//m_pPath = m_pPath->Parent;
		count++;
	}

	if (count <= 0
		|| m_pPath == nullptr
		|| count >= MAX_PATH_LINE)
	{
		//TRACE(_T("#### PathFind Fail : nid=%d,%hs, count=%d ####\n"), m_sNid+NPC_BAND, m_strName, count);
		return 0;
	}

	m_iAniFrameIndex = count - 1;

	float x1 = 0.0f;
	float z1 = 0.0f;

	int nAdd = GetDir(m_fStartPoint_X, m_fStartPoint_Y, m_fEndPoint_X, m_fEndPoint_Y);

	for (i = 0; i < count; i++)
	{
		if (i == 1)
		{
			if (i == (count - 1))
			{
				m_pPoint[i].fXPos = m_fEndPoint_X;
				m_pPoint[i].fZPos = m_fEndPoint_Y;
			}
			else
			{
				x1 = (float) (m_pPoint[i].pPoint.x * TILE_SIZE + m_fAdd_x);
				z1 = (float) (m_pPoint[i].pPoint.y * TILE_SIZE + m_fAdd_z);
				m_pPoint[i].fXPos = x1;
				m_pPoint[i].fZPos = z1;
			}
		}
		else if (i == (count - 1))
		{
			m_pPoint[i].fXPos = m_fEndPoint_X;
			m_pPoint[i].fZPos = m_fEndPoint_Y;
		}
		else
		{
			x1 = (float) (m_pPoint[i].pPoint.x * TILE_SIZE + m_fAdd_x);
			z1 = (float) (m_pPoint[i].pPoint.y * TILE_SIZE + m_fAdd_z);
			m_pPoint[i].fXPos = x1;
			m_pPoint[i].fZPos = z1;
		}
	}

	return 1;
}


//	NPC 사망처리
void CNpc::Dead(CIOCPort* pIOCP, int iDeadType)
{
	MAP* pMap = m_pMain->GetMapByIndex(m_ZoneIndex);
	if (pMap == nullptr)
	{
		spdlog::error("Npc::Dead: map not found [zoneIndex={} npcId={} npcName={}]",
			m_strName, m_sSid, m_ZoneIndex);
		return;
	}

	m_iHP = 0;
	m_NpcState = NPC_DEAD;
	m_Delay = m_sRegenTime;
	m_fDelayTime = TimeGet();
	m_bFirstLive = false;
	m_byDeadType = 100;		// 전쟁이벤트중에서 죽는 경우

	if (m_iRegion_X > pMap->GetXRegionMax()
		|| m_iRegion_Z > pMap->GetZRegionMax())
	{
		spdlog::error("Npc::Dead: out of region bounds [serial={} npcId={} x={} z={}]",
			m_sNid + NPC_BAND, m_sSid, m_iRegion_X, m_iRegion_Z);
		return;
	}

	// map에 region에서 나의 정보 삭제..
	//pMap->m_ppRegion[m_iRegion_X][m_iRegion_Z].DeleteNpc(m_sNid);
	pMap->RegionNpcRemove(m_iRegion_X, m_iRegion_Z, m_sNid + NPC_BAND);

	//TRACE(_T("-- Npc-Dead RegionRemove : [nid=%d, name=%hs], nRX=%d, nRZ=%d \n"), m_sNid+NPC_BAND, m_strName, m_iRegion_X, m_iRegion_Z);
	CTime t = CTime::GetCurrentTime();
	//TRACE(_T("****** (%hs,%d) Dead regentime = %d , m_byDeadType=%d, dungeonfam=%d, time=%d:%d-%d ****************\n"), m_strName, m_sNid+NPC_BAND, m_sRegenTime, m_byDeadType, m_byDungeonFamily, t.GetHour(), t.GetMinute(), t.GetSecond());

	// User에 의해 죽은것이 아니기 때문에... 클라이언트에 Dead패킷전송...
	if (iDeadType == 1)
	{
		char buff[256] = {};
		int send_index = 0;
		SetByte(buff, AG_DEAD, send_index);
		SetShort(buff, m_sNid + NPC_BAND, send_index);
		SendAll(pIOCP, buff, send_index);
	}

	// Dungeon Work : 변하는 몬스터의 경우 변하게 처리..
	if (m_bySpecialType == 1
		|| m_bySpecialType == 4)
	{
		if (m_byChangeType == 0)
		{
			m_byChangeType = 1;
			//ChangeMonsterInfomation( 1 );
		}
		// 대장 몬스터의 죽음 (몬스터 타입이 대장몬스터인지도 검사해야 함)
		else if (m_byChangeType == 2)
		{
			if (m_byDungeonFamily < 0
				|| m_byDungeonFamily >= MAX_DUNGEON_BOSS_MONSTER)
			{
				spdlog::error("Npc::Dead: dungeonFamily out of range [serial={} npcId={} npcName={} dungeonFamily={}]",
					m_sNid + NPC_BAND, m_sSid, m_strName, m_byDungeonFamily);
				return;
			}
//			pMap->m_arDungeonBossMonster[m_byDungeonFamily] = 0;
		}
	}
	else
	{
		m_byChangeType = 100;
	}


/*
	if( m_byDungeonFamily < 0 || m_byDungeonFamily >= MAX_DUNGEON_BOSS_MONSTER )	{
		TRACE(_T("#### Npc-Dead() m_byDungeonFamily Fail : [nid=%d, name=%hs], m_byDungeonFamily=%d #####\n"), m_sNid+NPC_BAND, m_strName, m_byDungeonFamily);
		return;
	}
	if( pMap->m_arDungeonBossMonster[m_byDungeonFamily] == 0 )	{
		m_byRegenType = 2;				// 리젠이 안되도록..
	}	*/

	// 몬스터 소환 테스트
	//if(m_sNid == 0)
	//	m_pMain->MonsterSummon("클립토돈", m_sCurZone, 2605.0, 1375.0);
}

//	NPC 주변의 적을 찾는다.
bool CNpc::FindEnemy()
{
	if (m_tNpcType == NPC_DOOR
		|| m_tNpcType == NPC_ARTIFACT
		|| m_tNpcType == NPC_PHOENIX_GATE
		|| m_tNpcType == NPC_GATE_LEVER
		|| m_tNpcType == NPC_DOMESTIC_ANIMAL
		|| m_tNpcType == NPC_SPECIAL_GATE
		|| m_tNpcType == NPC_DESTORY_ARTIFACT)
		return false;

	MAP* pMap = m_pMain->GetMapByIndex(m_ZoneIndex);
	if (pMap == nullptr)
	{
		spdlog::error("Npc::FindEnemy: map not found [zoneIndex={} npcId={} npcName={}]",
			m_strName, m_sSid, m_ZoneIndex);
		return false;
	}

/*	bool bCheckRange = false;
	if (m_NpcState == NPC_STANDING)
	{
		bCheckRange = IsInRange((int) m_fCurX, (int) m_fCurZ);

		// 활동영역안에 있다면
		if (bCheckRange)
		{
			// 공격성향으로
			if (m_tNpcAttType != m_tNpcOldAttType)
			{
				m_tNpcAttType = ATROCITY_ATTACK_TYPE;
				//TRACE(_T("공격성향이 선공으로 변함\n"));
			}
		}
		else
		{
			if (m_tNpcAttType == ATROCITY_ATTACK_TYPE)
			{
				m_tNpcAttType = TENDER_ATTACK_TYPE;
				//TRACE(_T("공격성향이 후공으로 변함\n"));
			}
		}
	}	*/

	// Healer Npc
	int iMonsterNid = 0;

	// Heal
	if (m_tNpcType == NPC_HEALER)
	{
		iMonsterNid = FindFriend(2);
		if (iMonsterNid != 0)
			return true;
	}

	CUser* pUser = nullptr;
	CNpc* pNpc = nullptr;

	int target_uid = 0;
	__Vector3 vUser, vNpc;
	float fDis = 0.0f;
	float fCompareDis = 0.0f;
	vNpc.Set(m_fCurX, m_fCurY, m_fCurZ);

	float fSearchRange = (float) m_bySearchRange;

	int iExpand = FindEnemyRegion();

	// 자신의 region에 있는 UserArray을 먼저 검색하여,, 가까운 거리에 유저가 있는지를 판단..
	if (m_iRegion_X > pMap->GetXRegionMax()
		|| m_iRegion_Z > pMap->GetZRegionMax()
		|| m_iRegion_X < 0
		|| m_iRegion_Z < 0)
	{
		// TRACE(_T("#### Npc-FindEnemy() Fail : [nid=%d, sid=%d, name=%hs, th_num=%d, cur_x=%.2f, cur_z=%.2f], nRX=%d, nRZ=%d #####\n"), m_sNid+NPC_BAND, m_sSid, m_strName, m_sThreadNumber, m_fCurX, m_fCurZ, m_iRegion_X, m_iRegion_Z);
		return false;
	}

	bool bIsHostileToPlayers = true;

	if (m_sCurZone == ZONE_BIFROST
		|| m_sCurZone == ZONE_MORADON
		|| (m_sCurZone / 10) == 5)
	{
		if (m_byGroup != 0)
			bIsHostileToPlayers = false;
	}
	
	// NOTE: This should be included in the above, but officially it checks both
	// (although it erroneously does so in the loop, rather than upfront)
	if (m_tNpcType == NPC_GUARD
		|| m_tNpcType == NPC_PATROL_GUARD
		|| m_tNpcType == NPC_STORE_GUARD)
	{
		if (m_sCurZone == ZONE_MORADON)
			bIsHostileToPlayers = false;
	}

	if (bIsHostileToPlayers)
	{
		fCompareDis = FindEnemyExpand(m_iRegion_X, m_iRegion_Z, fCompareDis, 1);

		int x = 0, y = 0;

		// 이웃해 있는 Region을 검색해서,,  몬의 위치와 제일 가까운 User을 향해.. 이동..
		for (int l = 0; l < 4; l++)
		{
			if (m_iFind_X[l] == 0
				&& m_iFind_Y[l] == 0)
				continue;

			x = m_iRegion_X + (m_iFind_X[l]);
			y = m_iRegion_Z + (m_iFind_Y[l]);

			// 이부분 수정요망,,
			if (x < 0
				|| y < 0
				|| x > pMap->GetXRegionMax()
				|| y > pMap->GetZRegionMax())
				continue;

			fCompareDis = FindEnemyExpand(x, y, fCompareDis, 1);
		}


		if (m_Target.id >= 0
			&& fCompareDis <= fSearchRange)
			return true;
	}

	fCompareDis = 0.0f;

	// 타입이 경비병인 경우에는 같은 나라의 몬스터가 아닌경우에는 몬스터를 공격하도록 한다..
	if (m_tNpcType == NPC_GUARD
		|| m_tNpcType == NPC_PATROL_GUARD
		|| m_tNpcType == NPC_STORE_GUARD
		// || m_tNpcType == NPCTYPE_MONSTER
		)
	{
		fCompareDis = FindEnemyExpand(m_iRegion_X, m_iRegion_Z, fCompareDis, 2);

		int x = 0, y = 0;

		// 이웃해 있는 Region을 검색해서,,  몬의 위치와 제일 가까운 User을 향해.. 이동..
		for (int l = 0; l < 4; l++)
		{
			if (m_iFind_X[l] == 0
				&& m_iFind_Y[l] == 0)
				continue;

			x = m_iRegion_X + (m_iFind_X[l]);
			y = m_iRegion_Z + (m_iFind_Y[l]);

			// 이부분 수정요망,,
			if (x < 0
				|| y < 0
				|| x > pMap->GetXRegionMax()
				|| y > pMap->GetZRegionMax())
				continue;

			fCompareDis = FindEnemyExpand(x, y, fCompareDis, 2);
		}
	}

	if (m_Target.id >= 0
		&& fCompareDis <= fSearchRange)
		return true;

	// 아무도 없으므로 리스트에 관리하는 유저를 초기화한다.
	InitUserList();
	InitTarget();
	return false;
}

// Npc가 유저를 검색할때 어느 Region까지 검색해야 하는지를 판단..
int CNpc::FindEnemyRegion()
{
	/*
		1	2	3
		4	0	5
		6	7	8
	*/
	int iRetValue = 0;
	int  iSX = m_iRegion_X * VIEW_DIST;
	int  iSZ = m_iRegion_Z * VIEW_DIST;
	int  iEX = (m_iRegion_X + 1) * VIEW_DIST;
	int  iEZ = (m_iRegion_Z + 1) * VIEW_DIST;
	int  iSearchRange = m_bySearchRange;
	int iCurSX = (int) m_fCurX - m_bySearchRange;
	int iCurSY = (int) m_fCurX - m_bySearchRange;
	int iCurEX = (int) m_fCurX + m_bySearchRange;
	int iCurEY = (int) m_fCurX + m_bySearchRange;

	int iMyPos = GetMyField();

	switch (iMyPos)
	{
		case 1:
			if (iCurSX < iSX
				&& iCurSY < iSZ)
				iRetValue = 1;
			else if (iCurSX > iSX
				&& iCurSY < iSZ)
				iRetValue = 2;
			else if (iCurSX < iSX
				&& iCurSY > iSZ)
				iRetValue = 4;
			else if (iCurSX >= iSX
				&& iCurSY >= iSZ)
				iRetValue = 0;
			break;

		case 2:
			if (iCurEX < iEX
				&& iCurSY < iSZ)
				iRetValue = 2;
			else if (iCurEX > iEX
				&& iCurSY < iSZ)
				iRetValue = 3;
			else if (iCurEX <= iEX
				&& iCurSY >= iSZ)
				iRetValue = 0;
			else if (iCurEX > iEX
				&& iCurSY > iSZ)
				iRetValue = 5;
			break;

		case 3:
			if (iCurSX < iSX
				&& iCurEY < iEZ)
				iRetValue = 4;
			else if (iCurSX >= iSX
				&& iCurEY <= iEZ)
				iRetValue = 0;
			else if (iCurSX < iSX
				&& iCurEY > iEZ)
				iRetValue = 6;
			else if (iCurSX > iSX
				&& iCurEY > iEZ)
				iRetValue = 7;
			break;

		case 4:
			if (iCurEX <= iEX
				&& iCurEY <= iEZ)
				iRetValue = 0;
			else if (iCurEX > iEX
				&& iCurEY < iEZ)
				iRetValue = 5;
			else if (iCurEX < iEX
				&& iCurEY > iEZ)
				iRetValue = 7;
			else if (iCurEX > iEX
				&& iCurEY > iEZ)
				iRetValue = 8;
			break;
	}

	// 임시로 보정(문제시),, 하기 위한것..
	if (iRetValue <= 0)
		iRetValue = 0;

	switch (iRetValue)
	{
		case 0:
			m_iFind_X[0] = 0;
			m_iFind_Y[0] = 0;
			m_iFind_X[1] = 0;
			m_iFind_Y[1] = 0;
			m_iFind_X[2] = 0;
			m_iFind_Y[2] = 0;
			m_iFind_X[3] = 0;
			m_iFind_Y[3] = 0;
			break;

		case 1:
			m_iFind_X[0] = -1;
			m_iFind_Y[0] = -1;
			m_iFind_X[1] = 0;
			m_iFind_Y[1] = -1;
			m_iFind_X[2] = -1;
			m_iFind_Y[2] = 0;
			m_iFind_X[3] = 0;
			m_iFind_Y[3] = 0;
			break;

		case 2:
			m_iFind_X[0] = 0;
			m_iFind_Y[0] = -1;
			m_iFind_X[1] = 0;
			m_iFind_Y[1] = 0;
			m_iFind_X[2] = 0;
			m_iFind_Y[2] = 0;
			m_iFind_X[3] = 0;
			m_iFind_Y[3] = 0;
			break;

		case 3:
			m_iFind_X[0] = 0;
			m_iFind_Y[0] = 0;
			m_iFind_X[1] = 1;
			m_iFind_Y[1] = 0;
			m_iFind_X[2] = 0;
			m_iFind_Y[2] = 1;
			m_iFind_X[3] = 1;
			m_iFind_Y[3] = 1;
			break;

		case 4:
			m_iFind_X[0] = -1;
			m_iFind_Y[0] = 0;
			m_iFind_X[1] = 0;
			m_iFind_Y[1] = 0;
			m_iFind_X[2] = 0;
			m_iFind_Y[2] = 0;
			m_iFind_X[3] = 0;
			m_iFind_Y[3] = 0;
			break;

		case 5:
			m_iFind_X[0] = 0;
			m_iFind_Y[0] = 0;
			m_iFind_X[1] = 1;
			m_iFind_Y[1] = 0;
			m_iFind_X[2] = 0;
			m_iFind_Y[2] = 0;
			m_iFind_X[3] = 0;
			m_iFind_Y[3] = 0;
			break;

		case 6:

			m_iFind_X[0] = -1;
			m_iFind_Y[0] = 0;
			m_iFind_X[1] = 0;
			m_iFind_Y[1] = 0;
			m_iFind_X[2] = -1;
			m_iFind_Y[2] = 1;
			m_iFind_X[3] = 0;
			m_iFind_Y[3] = 1;
			break;

		case 7:
			m_iFind_X[0] = 0;
			m_iFind_Y[0] = 0;
			m_iFind_X[1] = 0;
			m_iFind_Y[1] = 0;
			m_iFind_X[2] = 0;
			m_iFind_Y[2] = 1;
			m_iFind_X[3] = 0;
			m_iFind_Y[3] = 0;
			break;

		case 8:
			m_iFind_X[0] = 0;
			m_iFind_Y[0] = 0;
			m_iFind_X[1] = 1;
			m_iFind_Y[1] = 0;
			m_iFind_X[2] = 0;
			m_iFind_Y[2] = 1;
			m_iFind_X[3] = 1;
			m_iFind_Y[3] = 1;
			break;
	}

	return iRetValue;
}

float CNpc::FindEnemyExpand(int nRX, int nRZ, float fCompDis, int nType)
{
	MAP* pMap = m_pMain->GetMapByIndex(m_ZoneIndex);
	if (pMap == nullptr)
	{
		spdlog::error("Npc::FindEnemyExpand: map not found [zoneIndex={} npcId={} npcName={}]",
					m_strName, m_sSid, m_ZoneIndex);
		return 0.0f;
	}

	float fDis = 0.0f;

	float fComp = fCompDis;
	float fSearchRange = (float) m_bySearchRange;
	int target_uid = -1;
	__Vector3 vUser, vNpc, vMon;
	vNpc.Set(m_fCurX, m_fCurY, m_fCurZ);
	int* pIDList = nullptr;
	int iLevelComprison = 0;

	// user을 타겟으로 잡는 경우
	if (nType == 1)
	{
		int nUserid = 0, count = 0;
		CUser* pUser = nullptr;

		EnterCriticalSection(&g_region_critical);

		auto Iter1 = pMap->m_ppRegion[nRX][nRZ].m_RegionUserArray.begin();
		auto Iter2 = pMap->m_ppRegion[nRX][nRZ].m_RegionUserArray.end();

		int nUser = pMap->m_ppRegion[nRX][nRZ].m_RegionUserArray.GetSize();
		//TRACE(_T("FindEnemyExpand type1,, region_x=%d, region_z=%d, user=%d, mon=%d\n"), nRX, nRZ, nUser, nMonster);
		if (nUser == 0)
		{
			LeaveCriticalSection(&g_region_critical);
			return 0.0f;
		}

		pIDList = new int[nUser];
		for (; Iter1 != Iter2; Iter1++)
		{
			nUserid = *((*Iter1).second);
			pIDList[count] = nUserid;
			count++;
		}
		LeaveCriticalSection(&g_region_critical);

		for (int i = 0; i < nUser; i++)
		{
			nUserid = pIDList[i];
			if (nUserid < 0)
				continue;

			pUser = m_pMain->GetUserPtr(nUserid);
			if (pUser == nullptr
				|| pUser->m_bLive != USER_LIVE)
				continue;

			// 같은 국가의 유저는 공격을 하지 않도록 한다...
			if (m_byGroup == pUser->m_bNation)
				continue;

			// 운영자 무시
			if (pUser->m_byIsOP == MANAGER_USER)
				continue;

			vUser.Set(pUser->m_curx, pUser->m_cury, pUser->m_curz);
			fDis = GetDistance(vUser, vNpc);

			// 작업 : 여기에서 나의 공격거리에 있는 유저인지를 판단
			if (fDis > fSearchRange)
				continue;

			if (fDis >= fComp)
			{
				target_uid = pUser->m_iUserId;
				fComp = fDis;

				//후공몹...
				// 날 공격한 놈을 찾는다.
				if (m_tNpcAttType == 0)
				{
					if (IsDamagedUserList(pUser)
						|| (m_tNpcGroupType && m_Target.id == target_uid))
					{
						m_Target.id = target_uid;
						m_Target.failCount = 0;
						m_Target.x = pUser->m_curx;
						m_Target.y = pUser->m_cury;
						m_Target.z = pUser->m_curz;
					}
				}
				// 선공몹...
				else
				{
					iLevelComprison = pUser->m_sLevel - m_sLevel;

					// 작업할 것 : 타입에 따른 공격성향으로..
					//if(iLevelComprison > ATTACK_LIMIT_LEVEL)	continue;

					m_Target.id = target_uid;
					m_Target.failCount = 0;
					m_Target.x = pUser->m_curx;
					m_Target.y = pUser->m_cury;
					m_Target.z = pUser->m_curz;
					//TRACE(_T("Npc-FindEnemyExpand - target_x = %.2f, z=%.2f\n"), m_Target.x, m_Target.z);
				}
			}
		}
	}
	// 경비병이 몬스터를 타겟으로 잡는 경우
	else if (nType == 2)
	{
		int nNpcid = 0, count = 0;
		CNpc* pNpc = nullptr;

		EnterCriticalSection(&g_region_critical);

		auto Iter1 = pMap->m_ppRegion[nRX][nRZ].m_RegionNpcArray.begin();
		auto Iter2 = pMap->m_ppRegion[nRX][nRZ].m_RegionNpcArray.end();

		int nMonster = pMap->m_ppRegion[nRX][nRZ].m_RegionNpcArray.GetSize();
		//TRACE(_T("FindEnemyExpand type1,, region_x=%d, region_z=%d, user=%d, mon=%d\n"), nRX, nRZ, nUser, nMonster);
		if (nMonster == 0)
		{
			LeaveCriticalSection(&g_region_critical);
			return 0.0f;
		}

		pIDList = new int[nMonster];
		for (; Iter1 != Iter2; Iter1++)
		{
			nNpcid = *((*Iter1).second);
			pIDList[count] = nNpcid;
			count++;
		}
		LeaveCriticalSection(&g_region_critical);

		//TRACE(_T("FindEnemyExpand type2,, region_x=%d, region_z=%d, user=%d, mon=%d\n"), nRX, nRZ, nUser, nMonster);

		for (int i = 0; i < nMonster; i++)
		{
			nNpcid = pIDList[i];
			if (nNpcid < NPC_BAND)
				continue;

			pNpc = m_pMain->m_NpcMap.GetData(nNpcid - NPC_BAND);

			if (m_sNid == pNpc->m_sNid)
				continue;

			if (pNpc == nullptr
				|| pNpc->m_NpcState == NPC_DEAD
				|| pNpc->m_sNid == m_sNid)
				continue;

			if (m_sCurZone == ZONE_BIFROST
				|| m_sCurZone == ZONE_MORADON
				|| (m_sCurZone / 10) == 5)
			{
				if (pNpc->m_byGroup != 0)
					continue;
			}

			// 같은 국가의 몬스터는 공격을 하지 않도록 한다...
			if (m_byGroup == pNpc->m_byGroup)
				continue;

			vMon.Set(pNpc->m_fCurX, pNpc->m_fCurY, pNpc->m_fCurZ);
			fDis = GetDistance(vMon, vNpc);

			// 작업 : 여기에서 나의 공격거리에 있는 유저인지를 판단
			if (fDis > fSearchRange)
				continue;

			if (fDis >= fComp)
			{
				target_uid = nNpcid;
				fComp = fDis;
				m_Target.id = target_uid;
				m_Target.failCount = 0;
				m_Target.x = pNpc->m_fCurX;
				m_Target.y = pNpc->m_fCurY;
				m_Target.z = pNpc->m_fCurZ;
			//	TRACE(_T("Npc-IsCloseTarget - target_x = %.2f, z=%.2f\n"), m_Target.x, m_Target.z);
			}
		}
	}

	delete[] pIDList;
	pIDList = nullptr;

	return fComp;
}

// region을 4등분해서 몬스터의 현재 위치가 region의 어느 부분에 들어가는지를 판단
int CNpc::GetMyField()
{
	int iRet = 0;
	int iX = m_iRegion_X * VIEW_DIST;
	int iZ = m_iRegion_Z * VIEW_DIST;
	int iAdd = VIEW_DIST / 2;
	int iCurX = (int) m_fCurX;	// monster current position_x
	int iCurZ = (int) m_fCurZ;
	if (COMPARE(iCurX, iX, iX + iAdd)
		&& COMPARE(iCurZ, iZ, iZ + iAdd))
		iRet = 1;
	else if (COMPARE(iCurX, iX + iAdd, iX + VIEW_DIST)
		&& COMPARE(iCurZ, iZ, iZ + iAdd))
		iRet = 2;
	else if (COMPARE(iCurX, iX, iX + iAdd)
		&& COMPARE(iCurZ, iZ + iAdd, iZ + VIEW_DIST))
		iRet = 3;
	else if (COMPARE(iCurX, iX + iAdd, iX + VIEW_DIST)
		&& COMPARE(iCurZ, iZ + iAdd, iZ + VIEW_DIST))
		iRet = 4;

	return iRet;
}

//	주변에 나를 공격한 유저가 있는지 알아본다
bool CNpc::IsDamagedUserList(CUser* pUser)
{
	if (pUser == nullptr)
		return false;

	for (int i = 0; i < NPC_HAVE_USER_LIST; i++)
	{
		if (strcmp(m_DamagedUserList[i].strUserID, pUser->m_strUserID) == 0)
			return true;
	}

	return false;
}

//	타겟이 둘러 쌓여 있으면 다음 타겟을 찾는다.
int CNpc::IsSurround(CUser* pUser)
{
	//원거리는 통과
	if (m_tNpcLongType)
		return 0;

	// User가 없으므로 타겟지정 실패..
	if (pUser == nullptr)
		return -2;

	int nDir = pUser->IsSurroundCheck(m_fCurX, 0.0f, m_fCurZ, m_sNid + NPC_BAND);
	if (nDir != 0)
	{
		m_byAttackPos = nDir;
		return nDir;
	}

	// 타겟이 둘러 쌓여 있음...
	return -1;
}

//	x, y 가 움직일 수 있는 좌표인지 판단
bool CNpc::IsMovable(float x, float z)
{
	if (x < 0
		|| z < 0)
		return false;

	MAP* pMap = m_pMain->GetMapByIndex(m_ZoneIndex);
	if (pMap == nullptr)
	{
		spdlog::error("Npc::IsMovable: map not found [zoneIndex={} npcId={} npcName={}]",
					m_strName, m_sSid, m_ZoneIndex);
		return false;
	}

	if (pMap->m_pMap == nullptr)
		return false;

	if (x >= pMap->m_sizeMap.cx
		|| z >= pMap->m_sizeMap.cy)
		return false;

	if (pMap->m_pMap[(int) x][(int) z].m_sEvent == 0)
		return false;

	return true;
}

//	Path Find 로 찾은길을 다 이동 했는지 판단
bool CNpc::IsMovingEnd()
{
	//if(m_fCurX == m_fEndPoint_X && m_fCurZ == m_fEndPoint_Y) 
	if (m_fPrevX == m_fEndPoint_X
		&& m_fPrevZ == m_fEndPoint_Y)
	{
		//m_sStepCount = 0;
		m_iAniFrameCount = 0;
		return true;
	}

	return false;
}

//	Step 수 만큼 타켓을 향해 이동한다.
bool CNpc::StepMove(int nStep, CIOCPort* pIOCP)
{
	if (m_NpcState != NPC_MOVING
		&& m_NpcState != NPC_TRACING
		&& m_NpcState != NPC_BACK)
		return false;

	POINT ptPre = { -1,-1 };

	__Vector3 vStart, vEnd, vDis;
	float fDis;

	float fOldCurX = 0.0f, fOldCurZ = 0.0f;

	if (m_sStepCount == 0)
	{
		fOldCurX = m_fCurX;
		fOldCurZ = m_fCurZ;
	}
	else
	{
		fOldCurX = m_fPrevX;
		fOldCurZ = m_fPrevZ;
	}

	vStart.Set(fOldCurX, 0, fOldCurZ);
	vEnd.Set(m_pPoint[m_iAniFrameCount].fXPos, 0, m_pPoint[m_iAniFrameCount].fZPos);

	// 안전 코드..
	if (m_pPoint[m_iAniFrameCount].fXPos < 0
		|| m_pPoint[m_iAniFrameCount].fZPos < 0)
	{
		m_fPrevX = m_fEndPoint_X;
		m_fPrevZ = m_fEndPoint_Y;
		spdlog::error("Npc::StepMove: aniFrameCount out of bounds [serial={} npcId={} npcName={} frameCount={} frameIndex={}]",
			m_sNid + NPC_BAND, m_sSid, m_strName, m_iAniFrameCount, m_iAniFrameIndex);
		SetUid(m_fPrevX, m_fPrevZ, m_sNid + NPC_BAND);
		return false;
	}

	fDis = GetDistance(vStart, vEnd);

	if (fDis >= m_fSecForMetor)
	{
		vDis = GetVectorPosition(vStart, vEnd, m_fSecForMetor);
		m_fPrevX = vDis.x;
		m_fPrevZ = vDis.z;
	}
	else
	{
		m_iAniFrameCount++;
		if (m_iAniFrameCount == m_iAniFrameIndex)
		{
			vEnd.Set(m_pPoint[m_iAniFrameCount].fXPos, 0, m_pPoint[m_iAniFrameCount].fZPos);
			fDis = GetDistance(vStart, vEnd);
			// 마지막 좌표는 m_fSecForMetor ~ m_fSecForMetor+1 사이도 가능하게 이동
			if (fDis > m_fSecForMetor)
			{
				vDis = GetVectorPosition(vStart, vEnd, m_fSecForMetor);
				m_fPrevX = vDis.x;
				m_fPrevZ = vDis.z;
				m_iAniFrameCount--;
			}
			else
			{
				m_fPrevX = m_fEndPoint_X;
				m_fPrevZ = m_fEndPoint_Y;
			}
		}
		else
		{
			vEnd.Set(m_pPoint[m_iAniFrameCount].fXPos, 0, m_pPoint[m_iAniFrameCount].fZPos);
			fDis = GetDistance(vStart, vEnd);
			if (fDis >= m_fSecForMetor)
			{
				vDis = GetVectorPosition(vStart, vEnd, m_fSecForMetor);
				m_fPrevX = vDis.x;
				m_fPrevZ = vDis.z;
			}
			else
			{
				m_fPrevX = m_fEndPoint_X;
				m_fPrevZ = m_fEndPoint_Y;
			}
		}
	}

	vStart.Set(fOldCurX, 0, fOldCurZ);
	vEnd.Set(m_fPrevX, 0, m_fPrevZ);

	m_fSecForRealMoveMetor = GetDistance(vStart, vEnd);

	if (m_fSecForRealMoveMetor > m_fSecForMetor + 1)
	{
		spdlog::error("Npc::StepMove: exceeding max speed [serial={} npcId={} npcName={} meterPerSecond={}]",
			m_sNid + NPC_BAND, m_sSid, m_strName, m_fSecForRealMoveMetor);
	}

	if (m_sStepCount == 0)
	{
		m_sStepCount++;
	}
	else
	{
		m_sStepCount++;
		m_fCurX = fOldCurX;
		m_fCurZ = fOldCurZ;

		if (m_fCurX < 0
			|| m_fCurZ < 0)
		{
			spdlog::error("Npc::StepMove: coordinates invalid [serial={} npcId={} npcName={} x={} z={}]",
				m_sNid + NPC_BAND, m_sSid, m_strName, m_fCurX, m_fCurZ);
		}

		if (!SetUid(m_fCurX, m_fCurZ, m_sNid + NPC_BAND))
			return false;
	}

	return true;
}

bool CNpc::StepNoPathMove(int nStep)
{
	if (m_NpcState != NPC_MOVING
		&& m_NpcState != NPC_TRACING
		&& m_NpcState != NPC_BACK)
		return false;

	__Vector3 vStart, vEnd;
	float fOldCurX = 0.0f, fOldCurZ = 0.0f;

	if (m_sStepCount == 0)
	{
		fOldCurX = m_fCurX;
		fOldCurZ = m_fCurZ;
	}
	else
	{
		fOldCurX = m_fPrevX;
		fOldCurZ = m_fPrevZ;
	}

	if (m_sStepCount < 0
		|| m_sStepCount >= m_iAniFrameIndex)
	{
		spdlog::error("Npc::StepNoPathMove: stepCount out of bounds [serial={} npcId={} npcName={} stepCount={} frameIndex={}]",
			m_sNid + NPC_BAND, m_sSid, m_strName, m_sStepCount, m_iAniFrameIndex);
		return false;
	}

	vStart.Set(fOldCurX, 0, fOldCurZ);
	m_fPrevX = m_pPoint[m_sStepCount].fXPos;
	m_fPrevZ = m_pPoint[m_sStepCount].fZPos;
	vEnd.Set(m_fPrevX, 0, m_fPrevZ);

	if (m_fPrevX == -1
		|| m_fPrevZ == -1)
	{
		spdlog::error("Npc::StepNoPathMove: previous coordinates invalid [serial={} npcId={} npcName={} prevX={} prevZ={}]",
			m_sNid + NPC_BAND, m_sSid, m_strName, m_fPrevX, m_fPrevZ);
		return false;
	}

	m_fSecForRealMoveMetor = GetDistance(vStart, vEnd);

	if (++m_sStepCount > 1)
	{
		if (fOldCurX < 0
			|| fOldCurZ < 0)
		{
			spdlog::error("Npc::StepNoPathMove: old previous coordinates invalid [serial={} npcId={} npcName={} oldCurX={} oldCurZ={}]",
				m_sNid + NPC_BAND, m_sSid, m_strName, fOldCurX, fOldCurZ);
			return false;
		}
		else
		{
			m_fCurX = fOldCurX;
			m_fCurZ = fOldCurZ;
		}

		if (!SetUid(m_fCurX, m_fCurZ, m_sNid + NPC_BAND))
			return false;
	}

	return true;
}

//	NPC와 Target 과의 거리가 지정 범위보다 작은지 판단
int CNpc::IsCloseTarget(int nRange, int Flag)
{
	__Vector3 vUser, vWillUser, vNpc, vDistance;
	CUser* pUser = nullptr;
	CNpc* pNpc = nullptr;
	float fDis = 0.0f, fWillDis = 0.0f, fX = 0.0f, fZ = 0.0f;
	bool  bUserType = false;	// 타겟이 유저이면 true
	vNpc.Set(m_fCurX, m_fCurY, m_fCurZ);

	// Target 이 User 인 경우
	if (m_Target.id >= USER_BAND
		&& m_Target.id < NPC_BAND)
	{
		pUser = m_pMain->GetUserPtr(m_Target.id - USER_BAND);
		if (pUser == nullptr)
		{
			InitTarget();
			return -1;
		}

		vUser.Set(pUser->m_curx, pUser->m_cury, pUser->m_curz);
		vWillUser.Set(pUser->m_fWill_x, pUser->m_fWill_y, pUser->m_fWill_z);
		fX = pUser->m_curx;
		fZ = pUser->m_curz;

		vDistance = vWillUser - vNpc;
		fWillDis = vDistance.Magnitude();
		fWillDis = fWillDis - m_fBulk;
		bUserType = true;
	}
	// Target 이 mon 인 경우
	else if (m_Target.id >= NPC_BAND
		&& m_Target.id < INVALID_BAND)
	{
		pNpc = m_pMain->m_NpcMap.GetData(m_Target.id - NPC_BAND);
		if (pNpc == nullptr)
		{
			InitTarget();
			return -1;
		}

		vUser.Set(pNpc->m_fCurX, pNpc->m_fCurY, pNpc->m_fCurZ);
		fX = pNpc->m_fCurX;
		fZ = pNpc->m_fCurZ;
	}
	else
	{
		return -1;
	}

	vDistance = vUser - vNpc;
	fDis = vDistance.Magnitude();

	fDis = fDis - m_fBulk;

	// 작업할것 :	 던젼 몬스터의 경우 일정영역을 벗어나지 못하도록 체크하는 루틴 	
	if (m_tNpcType == NPC_DUNGEON_MONSTER)
	{
		if (!IsInRange((int) vUser.x, (int) vUser.z))
			return -1;
	}

	if (Flag == 1)
	{
		m_byResetFlag = 1;
		if (pUser)
		{
			if (m_Target.x == pUser->m_curx && m_Target.z == pUser->m_curz)
				m_byResetFlag = 0;
		}
		//TRACE(_T("NpcTracing-IsCloseTarget - target_x = %.2f, z=%.2f, dis=%.2f, Flag=%d\n"), m_Target.x, m_Target.z, fDis, m_byResetFlag);
	}

	if ((int) fDis > nRange)
	{
		if (Flag == 2)
		{
			//TRACE(_T("NpcFighting-IsCloseTarget - target_x = %.2f, z=%.2f, dis=%.2f\n"), m_Target.x, m_Target.z, fDis);
			m_byResetFlag = 1;
			m_Target.x = fX;
			m_Target.z = fZ;
		}

		return 0;
	}

	/* 타겟의 좌표를 최신 것으로 수정하고, 마지막 포인터 좌표를 수정한다,, */
	m_fEndPoint_X = m_fCurX;
	m_fEndPoint_Y = m_fCurZ;
	m_Target.x = fX;
	m_Target.z = fZ;

	//if( m_tNpcLongType && m_tNpcType != NPC_BOSS_MONSTER)	{		// 장거리 공격이 가능한것은 공격거리로 판단..
	// 장거리 공격이 가능한것은 공격거리로 판단..
	if (m_tNpcLongType == 1)
	{
		if (fDis < LONG_ATTACK_RANGE)
			return 1;
		
		if (fDis > LONG_ATTACK_RANGE
			&& fDis <= nRange)
			return 2;
	}
	// 단거리(직접공격)
	else
	{
		// 몬스터의 이동하면서이 거리체크시
		if (Flag == 1)
		{
			if (fDis < (SHORT_ATTACK_RANGE + m_fBulk))
				return 1;

			// 유저의 현재좌표를 기준으로
			if (fDis > (SHORT_ATTACK_RANGE + m_fBulk)
				&& fDis <= nRange)
				return 2;

			// 유저일때만,, Will좌표를 기준으로 한다
			if (bUserType)
			{
				// 유저의 Will돠표를 기준으로
				if (fWillDis > (SHORT_ATTACK_RANGE + m_fBulk)
					&& fWillDis <= nRange)
					return 2;
			}
		}
		else
		{
			if (fDis < (SHORT_ATTACK_RANGE + m_fBulk))
				return 1;

			if (fDis > (SHORT_ATTACK_RANGE + m_fBulk)
				&& fDis <= nRange)
				return 2;
		}
	}

	//TRACE(_T("Npc-IsCloseTarget - target_x = %.2f, z=%.2f\n"), m_Target.x, m_Target.z);
	return 0;
}

//	Target 과 NPC 간 Path Finding을 수행한다.
int CNpc::GetTargetPath(int option)
{
	// sungyong 2002.06.12
	int nInitType = m_byInitMoveType;
	if (m_byInitMoveType >= 100)
		nInitType = m_byInitMoveType - 100;

	// 행동 타입 수정
	if (m_tNpcType != 0)
	{
		// 자기 자리로 돌아갈 수 있도록..
		//if(m_byMoveType != m_byInitMoveType)
		//	m_byMoveType = m_byInitMoveType;

		// 자기 자리로 돌아갈 수 있도록..
		if (m_byMoveType != nInitType)
			m_byMoveType = nInitType;
	}
	// ~sungyong 2002.06.12

	// 추격할때는 뛰는 속도로 맞추어준다...
	m_fSecForMetor = m_fSpeed_2;
	CUser* targetUser = nullptr;
	CNpc* npcTarget = nullptr;
	float chaseRange = 0.0f;
	__Vector3 vUser, vNpc, vDistance, vEnd22;
	float fDis = 0.0f;
	float fDegree = 0.0f, fTargetDistance = 0.0f;
	float fSurX = 0.0f, fSurZ = 0.0f;

	// 1m
	//float surround_fx[8] = {0.0f, -0.7071f, -1.0f, -0.7083f,  0.0f,  0.7059f,  1.0000f, 0.7083f};
	//float surround_fz[8] = {1.0f,  0.7071f,  0.0f, -0.7059f, -1.0f, -0.7083f, -0.0017f, 0.7059f};
	// 2m
	float surround_fx[8] = { 0.0f, -1.4142f, -2.0f, -1.4167f,  0.0f,  1.4117f,  2.0000f, 1.4167f };
	float surround_fz[8] = { 2.0f,  1.4142f,  0.0f, -1.4167f, -2.0f, -1.4167f, -0.0035f, 1.4117f };

	// Target 이 User 인 경우
	if (m_Target.id >= USER_BAND
		&& m_Target.id < NPC_BAND)
	{
		targetUser = m_pMain->GetUserPtr(m_Target.id - USER_BAND);
		if (targetUser == nullptr)
		{
			InitTarget();
			return -1;
		}

		if (targetUser->m_sHP <= 0
			/*|| pUser->m_state != STATE_GAMESTARTED*/
			|| !targetUser->m_bLive)
		{
			InitTarget();
			return -1;
		}

		if (targetUser->m_curZone != m_sCurZone)
		{
			InitTarget();
			return -1;
		}

		// magic이나 활등으로 공격 당했다면...
		if (option == 1)
		{
			vNpc.Set(m_fCurX, m_fCurY, m_fCurZ);
			vUser.Set(targetUser->m_curx, targetUser->m_cury, targetUser->m_curz);
			fDis = GetDistance(vNpc, vUser);

			// 너무 거리가 멀어서,, 추적이 안되게..
			if (fDis >= NPC_MAX_MOVE_RANGE)
				return -1;

			chaseRange = fDis + 10;
		}
		else
		{
			// 일시적으로 보정한다.
			chaseRange = (float) m_bySearchRange;

			// 공격받은 상태면 찾을 범위 증가.
			if (IsDamagedUserList(targetUser))
				chaseRange = (float) m_byTracingRange;
			else
				chaseRange += 2;
		}
	}
	// Target 이 mon 인 경우
	else if (m_Target.id >= NPC_BAND
		&& m_Target.id < INVALID_BAND)
	{
		npcTarget = m_pMain->m_NpcMap.GetData(m_Target.id - NPC_BAND);
		if (npcTarget == nullptr)
		{
			InitTarget();
			return false;
		}

		if (npcTarget->m_iHP <= 0
			|| npcTarget->m_NpcState == NPC_DEAD)
		{
			InitTarget();
			return -1;
		}

		chaseRange = (float) m_byTracingRange;				// 일시적으로 보정한다.
	}

	MAP* pMap = m_pMain->GetMapByIndex(m_ZoneIndex);
	if (pMap == nullptr)
	{
		spdlog::error("Npc::GetTargetPath: map not found [zoneIndex={} npcId={} npcName={}]",
		m_strName, m_sSid, m_ZoneIndex);
		return -1;
	}

	int max_xx = pMap->m_sizeMap.cx;
	int max_zz = pMap->m_sizeMap.cy;

	int min_x = (int) (m_fCurX - chaseRange) / TILE_SIZE;
	if (min_x < 0)
		min_x = 0;

	int min_z = (int) (m_fCurZ - chaseRange) / TILE_SIZE;
	if (min_z < 0)
		min_z = 0;

	int max_x = (int) (m_fCurX + chaseRange) / TILE_SIZE;
	if (max_x >= max_xx)
		max_x = max_xx - 1;

	int max_z = (int) (m_fCurZ + chaseRange) / TILE_SIZE;
	if (min_z >= max_zz)
		min_z = max_zz - 1;

	// Target 이 User 인 경우
	if (targetUser != nullptr)
	{
		// Check if user is within search range
		CRect r(min_x, min_z, max_x + 1, max_z + 1);
		if (!r.PtInRect(CPoint((int) targetUser->m_curx / TILE_SIZE, (int) targetUser->m_curz / TILE_SIZE)))
		{
			spdlog::debug("Npc::GetTargetPath: user outside of search range [serial={} npcId={} npcName={} charId={} attackPos={}]",
				m_sNid + NPC_BAND, m_sSid, m_strName, targetUser->m_strUserID, m_byAttackPos);
			return -1;
		}

		m_fStartPoint_X = m_fCurX;
		m_fStartPoint_Y = m_fCurZ;

		vNpc.Set(m_fCurX, m_fCurY, m_fCurZ);
		vUser.Set(targetUser->m_curx, targetUser->m_cury, targetUser->m_curz);

		// 여기에서 유저의 어느 방향으로 공격할것인지를 판단...(셋팅)
		// 이 부분에서 Npc의 공격점을 알아와서 공격하도록 한다,,
		IsSurround(targetUser);	//둘러 쌓여 있으면 무시한다.(원거리, 근거리 무시)

		//vEnd22 = CalcAdaptivePosition(vNpc, vUser, 2.0+m_fBulk);

		//TRACE(_T("Npc-GetTargetPath() : [nid=%d] t_Name=%hs, AttackPos=%d\n"), m_sNid, pUser->m_strUserID, m_byAttackPos);

		if (m_byAttackPos > 0
			&& m_byAttackPos < 9)
		{
			fDegree = (m_byAttackPos - 1) * 45;
			fTargetDistance = 2.0f + m_fBulk;
			vEnd22 = ComputeDestPos(vUser, 0.0f, fDegree, fTargetDistance);
			fSurX = vEnd22.x - vUser.x;
			fSurZ = vEnd22.z - vUser.z;
			//m_fEndPoint_X = vUser.x + surround_fx[m_byAttackPos-1];	m_fEndPoint_Y = vUser.z + surround_fz[m_byAttackPos-1];
			m_fEndPoint_X = vUser.x + fSurX;
			m_fEndPoint_Y = vUser.z + fSurZ;
		}
		else
		{
			vEnd22 = CalcAdaptivePosition(vNpc, vUser, 2.0 + m_fBulk);
			m_fEndPoint_X = vEnd22.x;
			m_fEndPoint_Y = vEnd22.z;
		}
	}
	// Target 이 mon 인 경우
	else if (npcTarget != nullptr)
	{
		// check if target is in search range
		CRect r(min_x, min_z, max_x + 1, max_z + 1);
		if (!r.PtInRect( { (int) npcTarget->m_fCurX / TILE_SIZE, (int) npcTarget->m_fCurZ / TILE_SIZE }))
		{
			spdlog::debug("Npc::GetTargetPath: target outside of search range [serial={} npcId={} npcName={} targetSerial={} targetId={} targetName={} attackPos={}]",
				m_sNid + NPC_BAND, m_sSid, m_strName,
				npcTarget->m_sNid + NPC_BAND, npcTarget->m_sSid, npcTarget->m_strName,
				m_byAttackPos);
			return -1;
		}

		m_fStartPoint_X = m_fCurX;
		m_fStartPoint_Y = m_fCurZ;

		vNpc.Set(m_fCurX, m_fCurY, m_fCurZ);
		vUser.Set(npcTarget->m_fCurX, npcTarget->m_fCurY, npcTarget->m_fCurZ);

		vEnd22 = CalcAdaptivePosition(vNpc, vUser, 2.0f + m_fBulk);
		m_fEndPoint_X = vEnd22.x;
		m_fEndPoint_Y = vEnd22.z;
	}

	vDistance = vEnd22 - vNpc;
	fDis = vDistance.Magnitude();

	if (fDis <= m_fSecForMetor)
	{
		ClearPathFindData();
		m_bPathFlag = true;
		m_iAniFrameIndex = 1;
		m_pPoint[0].fXPos = m_fEndPoint_X;
		m_pPoint[0].fZPos = m_fEndPoint_Y;
		//TRACE(_T("** Npc Direct Trace Move  : [nid = %d], fDis <= %d, %.2f **\n"), m_sNid, m_fSecForMetor, fDis);
		return true;
	}

	if ((int) fDis > chaseRange)
	{
		spdlog::debug("Npc::GetTargetPath: target outside of chase range [serial={} npcId={} npcName={} destDist={} chaseRange={}]",
			m_sNid + NPC_BAND, m_sSid, m_strName, fDis, chaseRange);
		return -1;
	}

	// 던젼 몬스터는 무조건 패스파인딩을 하도록..
	if (m_tNpcType != NPC_DUNGEON_MONSTER)
	{
		// 공격대상이 있으면 패스파인딩을 하지 않고 바로 타겟으로 가게 한다.
		if (m_Target.id != -1)
			return 0;
	}

	CPoint start, end;
	start.x = (int) (m_fCurX / TILE_SIZE) - min_x;
	start.y = (int) (m_fCurZ / TILE_SIZE) - min_z;
	end.x = (int) (vEnd22.x / TILE_SIZE) - min_x;
	end.y = (int) (vEnd22.z / TILE_SIZE) - min_z;

	// 작업할것 :	 던젼 몬스터의 경우 일정영역을 벗어나지 못하도록 체크하는 루틴 	
	if (m_tNpcType == NPC_DUNGEON_MONSTER)
	{
		if (!IsInRange((int) vEnd22.x, (int) vEnd22.z))
			return -1;
	}

	m_min_x = min_x;
	m_min_y = min_z;
	m_max_x = max_x;
	m_max_y = max_z;

	// Run Path Find ---------------------------------------------//
	return PathFind(start, end, m_fSecForMetor);
}

int CNpc::Attack(CIOCPort* pIOCP)
{
	if (pIOCP == nullptr)
		return 10000;

	// 텔레포트 가능하게,, (렌덤으로,, )
	int nRandom = 0, nPercent = 1000;
	int send_index = 0;
	bool bTeleport = false;
	char buff[256] = {};

/*	nRandom = myrand(1, 10000);
	if (COMPARE(nRandom, 8000, 10000))
	{
		bTeleport = Teleport(pIOCP);
		if (bTeleport)
			return m_Delay;
	}	*/

	// if (m_tNpcLongType == 1 && m_tNpcType != NPC_BOSS_MONSTER)
	// 장거리 공격이 가능한것은 공격거리로 판단..
	if (m_tNpcLongType == 1)
	{
		m_Delay = LongAndMagicAttack(pIOCP);
		return m_Delay;
	}

	int ret = 0;
	int nStandingTime = m_sStandTime;

	ret = IsCloseTarget(m_byAttackRange, 2);

	if (ret == 0)
	{
		// 고정 경비병은 추적을 하지 않도록..
		if (m_tNpcType == NPC_DOOR
			|| m_tNpcType == NPC_ARTIFACT
			|| m_tNpcType == NPC_PHOENIX_GATE
			|| m_tNpcType == NPC_GATE_LEVER
			|| m_tNpcType == NPC_DOMESTIC_ANIMAL
			|| m_tNpcType == NPC_SPECIAL_GATE
			|| m_tNpcType == NPC_DESTORY_ARTIFACT)
		{
			m_NpcState = NPC_STANDING;
			InitTarget();
			return 0;
		}

		m_sStepCount = 0;
		m_byActionFlag = ATTACK_TO_TRACE;
		m_NpcState = NPC_TRACING;			// 공격하고 도망가는 유저를 따라 잡기위해(반응을 좀더 빠르게) 
		return 0;							// IsCloseTarget()에 유저 x, y값을 갱신하고 Delay = 0으로 줌
	}
	else if (ret == 2)
	{
		//if(m_tNpcType == NPC_BOSS_MONSTER)	{		// 대장 몬스터이면.....

		// 직접, 간접(롱)공격이 가능한 몬스터 이므로 장거리 공격을 할 수 있다.
		if (m_tNpcLongType == 2)
		{
			m_Delay = LongAndMagicAttack(pIOCP);
			return m_Delay;
		}
		else
		{
			// 고정 경비병은 추적을 하지 않도록..
			if (m_tNpcType == NPC_DOOR
				|| m_tNpcType == NPC_ARTIFACT
				|| m_tNpcType == NPC_PHOENIX_GATE
				|| m_tNpcType == NPC_GATE_LEVER
				|| m_tNpcType == NPC_DOMESTIC_ANIMAL
				|| m_tNpcType == NPC_SPECIAL_GATE
				|| m_tNpcType == NPC_DESTORY_ARTIFACT)
			{
				m_NpcState = NPC_STANDING;
				InitTarget();
				return 0;
			}

			m_sStepCount = 0;
			m_byActionFlag = ATTACK_TO_TRACE;
			m_NpcState = NPC_TRACING;			// 공격하고 도망가는 유저를 따라 잡기위해(반응을 좀더 빠르게) 
			return 0;							// IsCloseTarget()에 유저 x, y값을 갱신하고 Delay = 0으로 줌
		}
	}
	else if (ret == -1)
	{
		m_NpcState = NPC_STANDING;
		InitTarget();
		return 0;
	}

	CNpc* pNpc = nullptr;
	CUser* pUser = nullptr;
	int		nDamage = 0;
	int nID = m_Target.id;					// Target 을 구한다.

	// 회피값/명중판정/데미지 계산 -----------------------------------------//
	// Target 이 User 인 경우
	if (nID >= USER_BAND
		&& nID < NPC_BAND)
	{
		pUser = m_pMain->GetUserPtr(nID - USER_BAND);

		// User 가 Invalid 한 경우
		if (pUser == nullptr)
		{
			InitTarget();
			m_NpcState = NPC_STANDING;
			return nStandingTime;
		}

		// User 가 이미 죽은경우
		if (pUser->m_bLive == USER_DEAD)
		{
			//SendAttackSuccess(pIOCP, ATTACK_TARGET_DEAD_OK, pUser->m_iUserId, 0, pUser->m_iHP);
			SendAttackSuccess(pIOCP, ATTACK_TARGET_DEAD_OK, pUser->m_iUserId, 0, 0);
			InitTarget();
			m_NpcState = NPC_STANDING;
			return nStandingTime;
		}

		if (pUser->m_state == STATE_DISCONNECTED)
		{
			InitTarget();
			m_NpcState = NPC_STANDING;
			return nStandingTime;
		}

		// 운영자는 공격을 안하게..
		if (pUser->m_byIsOP == MANAGER_USER)
		{
			InitTarget();
			m_NpcState = NPC_MOVING;
			return nStandingTime;
		}

		// Npc와 유저와의 HP를 비교하여.. 도망을 갈 것인지를 판단...
		/* if (m_byNpcEndAttType != 0)
		{
			if (IsCompStatus(pUser))
			{
				m_NpcState = NPC_BACK;
				return 0;
			}
		}*/

		// if (m_tNpcType == NPC_BOSS_MONSTER)	// 대장 몬스터이면.....

		// 지역 마법 사용 몬스터이면.....
		if (m_byWhatAttackType == 4
			|| m_byWhatAttackType == 5)
		{
			nRandom = myrand(1, 10000);

			// 지역마법공격...
			if (nRandom < nPercent)
			{
				memset(buff, 0, sizeof(buff));
				send_index = 0;
				SetByte(buff, MAGIC_EFFECTING, send_index);
				SetDWORD(buff, m_iMagic2, send_index);			// Area Magic
				SetShort(buff, m_sNid + NPC_BAND, send_index);
				SetShort(buff, -1, send_index);					// tid는 반드시 -1
				SetShort(buff, (short) m_fCurX, send_index);	// target point
				SetShort(buff, (short) m_fCurY, send_index);
				SetShort(buff, (short) m_fCurZ, send_index);
				SetShort(buff, 0, send_index);
				SetShort(buff, 0, send_index);
				SetShort(buff, 0, send_index);

				m_MagicProcess.MagicPacket(buff, send_index, pIOCP);
				//TRACE(_T("++++ AreaMagicAttack --- sid=%d, magicid=%d\n"), m_sNid+NPC_BAND, m_iMagic2);
				return m_sAttackDelay + 1000;	// 지역마법은 조금 시간이 걸리도록.....
			}
		}
		else
		{
			// 독 공격하는 몬스터라면... (10%의 공격으로)
			if (m_byWhatAttackType == 2)
			{
				nRandom = myrand(1, 10000);

				// sungyong test ,, 무조건 독공격만 
				//nRandom = 100;

				// 독공격...
				if (nRandom < nPercent)
				{
					memset(buff, 0, sizeof(buff));
					send_index = 0;
					SetByte(buff, AG_MAGIC_ATTACK_RESULT, send_index);
					SetByte(buff, MAGIC_EFFECTING, send_index);
					SetDWORD(buff, m_iMagic1, send_index);			// FireBall
					SetShort(buff, m_sNid + NPC_BAND, send_index);
					SetShort(buff, pUser->m_iUserId, send_index);
					SetShort(buff, 0, send_index);					// data0
					SetShort(buff, 0, send_index);
					SetShort(buff, 0, send_index);
					SetShort(buff, 0, send_index);
					SetShort(buff, 0, send_index);
					SetShort(buff, 0, send_index);

					//m_MagicProcess.MagicPacket(buff, send_index, pIOCP);
					SendAll(pIOCP, buff, send_index);

					//TRACE(_T("LongAndMagicAttack --- sid=%d, tid=%d\n"), m_sNid+NPC_BAND, pUser->m_iUserId);
					return m_sAttackDelay;
				}
			}
		}

		// 명중이면 //Damage 처리 ----------------------------------------------------------------//
		nDamage = GetFinalDamage(pUser);	// 최종 대미지

		// sungyong test
		if (m_pMain->m_byTestMode)
			nDamage = 10;

		//TRACE(_T("Npc-Attack() : [mon: x=%.2f, z=%.2f], [user : x=%.2f, z=%.2f]\n"), m_fCurX, m_fCurZ, pUser->m_curx, pUser->m_curz);

		if (nDamage > 0)
		{
			pUser->SetDamage(nDamage, m_sNid + NPC_BAND);
			if (pUser->m_bLive != USER_DEAD)
			{
				//	TRACE(_T("Npc-Attack Success : [npc=%d, %hs]->[user=%d, %hs]\n"), m_sNid+NPC_BAND, m_strName, pUser->m_iUserId, pUser->m_strUserID);
				SendAttackSuccess(pIOCP, ATTACK_SUCCESS, pUser->m_iUserId, nDamage, pUser->m_sHP);
			}
		}
		else
		{
			SendAttackSuccess(pIOCP, ATTACK_FAIL, pUser->m_iUserId, nDamage, pUser->m_sHP);
		}

		// 방어측 내구도 감소
	}
	else if (nID >= NPC_BAND
		&& m_Target.id < INVALID_BAND)
	{
		pNpc = m_pMain->m_NpcMap.GetData(nID - NPC_BAND);

		// User 가 Invalid 한 경우
		if (pNpc == nullptr)
		{
			InitTarget();
			m_NpcState = NPC_STANDING;
			return nStandingTime;
		}

		// healer이면서 같은국가의 NPC인경우에는 힐
		if (m_tNpcType == NPC_HEALER
			&& pNpc->m_byGroup == m_byGroup)
		{
			m_NpcState = NPC_HEALING;
			return 0;
		}

		if (pNpc->m_iHP <= 0
			|| pNpc->m_NpcState == NPC_DEAD)
		{
			SendAttackSuccess(pIOCP, ATTACK_TARGET_DEAD, pNpc->m_sNid + NPC_BAND, 0, 0);
			InitTarget();
			m_NpcState = NPC_STANDING;
			return nStandingTime;
		}

		// Npc와 유저와의 HP를 비교하여.. 도망을 갈 것인지를 판단...
	/*	if (IsCompStatus(pUser))
		{
			m_NpcState = NPC_BACK;
			return 0;
		}*/

		// MoveAttack
		//MoveAttack(pIOCP);

		// 명중이면 //Damage 처리 ----------------------------------------------------------------//
		nDamage = GetNFinalDamage(pNpc);	// 최종 대미지

		if (pUser != nullptr)
		{
			//TRACE(_T("Npc-Attack() : [mon: x=%.2f, z=%.2f], [user : x=%.2f, z=%.2f]\n"), m_fCurX, m_fCurZ, pUser->m_curx, pUser->m_curz);
		}

		if (nDamage > 0)
		{
			pNpc->SetDamage(0, nDamage, m_strName.c_str(), m_sNid + NPC_BAND, pIOCP);
			//if(pNpc->m_iHP > 0)
			SendAttackSuccess(pIOCP, ATTACK_SUCCESS, pNpc->m_sNid + NPC_BAND, nDamage, pNpc->m_iHP);
		}
		else
		{
			SendAttackSuccess(pIOCP, ATTACK_FAIL, pNpc->m_sNid + NPC_BAND, nDamage, pNpc->m_iHP);
		}
	}

	return m_sAttackDelay;
}

int CNpc::LongAndMagicAttack(CIOCPort* pIOCP)
{
	if (pIOCP == nullptr)
		return 10000;

	int ret = 0;
	int nStandingTime = m_sStandTime;
	int send_index = 0;
	char buff[256] = {};

	ret = IsCloseTarget(m_byAttackRange, 2);

	if (ret == 0)
	{
		m_sStepCount = 0;
		m_byActionFlag = ATTACK_TO_TRACE;
		m_NpcState = NPC_TRACING;			// 공격하고 도망가는 유저를 따라 잡기위해(반응을 좀더 빠르게) 
		return 0;							// IsCloseTarget()에 유저 x, y값을 갱신하고 Delay = 0으로 줌
	}
	else if (ret == 2)
	{
		//if(m_tNpcType != NPC_BOSS_MONSTER)	{		// 대장 몬스터이면.....

		// 장거리 몬스터이면.....
		if (m_tNpcLongType == 1)
		{
			m_sStepCount = 0;
			m_byActionFlag = ATTACK_TO_TRACE;
			m_NpcState = NPC_TRACING;			// 공격하고 도망가는 유저를 따라 잡기위해(반응을 좀더 빠르게) 
			return 0;							// IsCloseTarget()에 유저 x, y값을 갱신하고 Delay = 0으로 줌
		}
	}

	if (ret == -1)
	{
		m_NpcState = NPC_STANDING;
		InitTarget();
		return 0;
	}

	CNpc* pNpc = nullptr;
	CUser* pUser = nullptr;
	int nDamage = 0;
	int nID = m_Target.id;					// Target 을 구한다.

	// 회피값/명중판정/데미지 계산 -----------------------------------------//

	// Target 이 User 인 경우
	if (nID >= USER_BAND
		&& nID < NPC_BAND)
	{
		pUser = m_pMain->GetUserPtr(nID - USER_BAND);

		// User 가 Invalid 한 경우
		if (pUser == nullptr)
		{
			InitTarget();
			m_NpcState = NPC_STANDING;
			return nStandingTime;
		}

		// User 가 이미 죽은경우
		if (pUser->m_bLive == USER_DEAD)
		{
			SendAttackSuccess(pIOCP, ATTACK_TARGET_DEAD_OK, pUser->m_iUserId, 0, 0);
			InitTarget();
			m_NpcState = NPC_STANDING;
			return nStandingTime;
		}

		if (pUser->m_state == STATE_DISCONNECTED)
		{
			InitTarget();
			m_NpcState = NPC_STANDING;
			return nStandingTime;
		}

		// 운영자는 공격을 안하게..
		if (pUser->m_byIsOP == MANAGER_USER)
		{
			InitTarget();
			m_NpcState = NPC_MOVING;
			return nStandingTime;
		}

		// Npc와 유저와의 HP를 비교하여.. 도망을 갈 것인지를 판단...
	/*	if (m_byNpcEndAttType != 0)
		{
			if (IsCompStatus(pUser))
			{
				m_NpcState = NPC_BACK;
				return 0;
			}
		}	*/

		// 조건을 판단해서 마법 공격 사용 (지금은 마법 1만 사용토록 하자)
		SetByte(buff, MAGIC_CASTING, send_index);
		SetDWORD(buff, m_iMagic1, send_index);			// FireBall
		SetShort(buff, m_sNid + NPC_BAND, send_index);
		SetShort(buff, pUser->m_iUserId, send_index);
		SetShort(buff, 0, send_index);					// data0
		SetShort(buff, 0, send_index);
		SetShort(buff, 0, send_index);
		SetShort(buff, 0, send_index);
		SetShort(buff, 0, send_index);
		SetShort(buff, 0, send_index);

		m_MagicProcess.MagicPacket(buff, send_index, pIOCP);

		//TRACE(_T("**** LongAndMagicAttack --- sid=%d, tid=%d\n"), m_sNid+NPC_BAND, pUser->m_iUserId);
	}
	else if (nID >= NPC_BAND
		&& m_Target.id < INVALID_BAND)
	{
		pNpc = m_pMain->m_NpcMap.GetData(nID - NPC_BAND);
		//pNpc = m_pMain->m_NpcMap[nID - NPC_BAND];

		// User 가 Invalid 한 경우
		if (pNpc == nullptr)
		{
			InitTarget();
			m_NpcState = NPC_STANDING;
			return nStandingTime;
		}

		if (pNpc->m_iHP <= 0
			|| pNpc->m_NpcState == NPC_DEAD)
		{
			SendAttackSuccess(pIOCP, ATTACK_TARGET_DEAD, pNpc->m_sNid + NPC_BAND, 0, 0);
			InitTarget();
			m_NpcState = NPC_STANDING;
			return nStandingTime;
		}

		// Npc와 유저와의 HP를 비교하여.. 도망을 갈 것인지를 판단...
	/*	if (IsCompStatus(pUser))
		{
			m_NpcState = NPC_BACK;
			return 0;
		}*/

		/*
		SetByte( buff, AG_LONG_MAGIC_ATTACK, send_index );
		SetByte( buff, type, send_index );
		SetDWORD( buff, magicid, send_index );
		SetShort( buff, m_sNid+NPC_BAND, send_index );
		SetShort( buff, pNpc->m_sNid+NPC_BAND, send_index );	*/
	}

	return m_sAttackDelay;
}

int CNpc::TracingAttack(CIOCPort* pIOCP)		// 0:attack fail, 1:attack success
{
	if (!pIOCP)
		return 0;

	CNpc* pNpc = nullptr;
	CUser* pUser = nullptr;

	int nDamage = 0;

	int nID = m_Target.id;					// Target 을 구한다.

	// 회피값/명중판정/데미지 계산 -----------------------------------------//
	// Target 이 User 인 경우
	if (nID >= USER_BAND
		&& nID < NPC_BAND)
	{
		pUser = m_pMain->GetUserPtr(nID - USER_BAND);

		// User 가 Invalid 한 경우
		if (pUser == nullptr)
			return 0;

		// User 가 이미 죽은경우
		if (pUser->m_bLive == USER_DEAD)
		{
			SendAttackSuccess(pIOCP, ATTACK_TARGET_DEAD_OK, pUser->m_iUserId, 0, 0);
			return 0;
		}

		if (pUser->m_state == STATE_DISCONNECTED)
			return 0;

		// 운영자는 공격을 안하게..
		if (pUser->m_byIsOP == MANAGER_USER)
			return 0;

		// 명중이면 //Damage 처리 ----------------------------------------------------------------//
		nDamage = GetFinalDamage(pUser);	// 최종 대미지

		// sungyong test
		if (m_pMain->m_byTestMode)
			nDamage = 1;

		if (nDamage > 0)
		{
			pUser->SetDamage(nDamage, m_sNid + NPC_BAND);

			if (pUser->m_bLive != USER_DEAD)
			{
				// TRACE(_T("Npc-Attack Success : [npc=%d, %hs]->[user=%d, %hs]\n"), m_sNid+NPC_BAND, m_strName, pUser->m_iUserId, pUser->m_strUserID);
				SendAttackSuccess(pIOCP, ATTACK_SUCCESS, pUser->m_iUserId, nDamage, pUser->m_sHP);
			}
		}
		else
		{
			SendAttackSuccess(pIOCP, ATTACK_FAIL, pUser->m_iUserId, nDamage, pUser->m_sHP);
		}

		// 방어측 내구도 감소
	}
	else if (nID >= NPC_BAND
		&& m_Target.id < INVALID_BAND)
	{
		pNpc = m_pMain->m_NpcMap.GetData(nID - NPC_BAND);

		// User 가 Invalid 한 경우
		if (pNpc == nullptr)
			return 0;

		if (pNpc->m_iHP <= 0
			|| pNpc->m_NpcState == NPC_DEAD)
		{
			SendAttackSuccess(pIOCP, ATTACK_TARGET_DEAD, pNpc->m_sNid + NPC_BAND, 0, 0);
			return 0;
		}

		// 명중이면 //Damage 처리 ----------------------------------------------------------------//
		nDamage = GetNFinalDamage(pNpc);	// 최종 대미지
		//TRACE(_T("Npc-Attack() : [mon: x=%.2f, z=%.2f], [user : x=%.2f, z=%.2f]\n"), m_fCurX, m_fCurZ, pUser->m_curx, pUser->m_curz);

		if (nDamage > 0)
		{
			if (pNpc->SetDamage(0, nDamage, m_strName.c_str(), m_sNid + NPC_BAND, pIOCP))
			{
				SendAttackSuccess(pIOCP, ATTACK_SUCCESS, pNpc->m_sNid + NPC_BAND, nDamage, pNpc->m_iHP);
			}
			else
			{
				SendAttackSuccess(pIOCP, ATTACK_SUCCESS, pNpc->m_sNid + NPC_BAND, nDamage, pNpc->m_iHP);
				SendAttackSuccess(pIOCP, ATTACK_TARGET_DEAD, pNpc->m_sNid + NPC_BAND, nDamage, pNpc->m_iHP);
				return 0;
			}
		}
		else
		{
			SendAttackSuccess(pIOCP, ATTACK_FAIL, pNpc->m_sNid + NPC_BAND, nDamage, pNpc->m_iHP);
		}
	}

	return 1;
}

void CNpc::MoveAttack(CIOCPort* pIOCP)
{
	char pBuf[1024] = {};
	int index = 0;
	__Vector3 vUser;
	__Vector3 vNpc;
	__Vector3 vDistance;
	__Vector3 vEnd22;
	CUser* pUser = nullptr;
	CNpc* pNpc = nullptr;
	float fDis = 0.0f;
	float fX = 0.0f, fZ = 0.0f;
	vNpc.Set(m_fCurX, m_fCurY, m_fCurZ);

	// 1m
	//float surround_fx[8] = {0.0f, -0.7071f, -1.0f, -0.7083f,  0.0f,  0.7059f,  1.0000f, 0.7083f};
	//float surround_fz[8] = {1.0f,  0.7071f,  0.0f, -0.7059f, -1.0f, -0.7083f, -0.0017f, 0.7059f};
	// 2m
	float surround_fx[8] = { 0.0f, -1.4142f, -2.0f, -1.4167f,  0.0f,  1.4117f,  2.0000f, 1.4167f };
	float surround_fz[8] = { 2.0f,  1.4142f,  0.0f, -1.4167f, -2.0f, -1.4167f, -0.0035f, 1.4117f };

	// Target 이 User 인 경우
	if (m_Target.id >= USER_BAND
		&& m_Target.id < NPC_BAND)
	{
		pUser = m_pMain->GetUserPtr(m_Target.id - USER_BAND);
		if (pUser == nullptr)
		{
			InitTarget();
			return;
		}

		vUser.Set(pUser->m_curx, pUser->m_cury, pUser->m_curz);

		vEnd22 = CalcAdaptivePosition(vNpc, vUser, 2);

		if (m_byAttackPos > 0
			&& m_byAttackPos < 9)
		{
			fX = vUser.x + surround_fx[m_byAttackPos - 1];
			fZ = vUser.z + surround_fz[m_byAttackPos - 1];
			vEnd22.Set(fX, 0, fZ);
			//TRACE(_T("MoveAttack 11 - nid(%hs, %d), fx=%.2f, fz=%.2f, attackpos=%d\n"), m_strName, m_sNid+NPC_BAND, fX, fZ, m_byAttackPos);
		}
		else
		{
			fX = vEnd22.x;
			fZ = vEnd22.z;
			//TRACE(_T("MoveAttack 22 - nid(%hs, %d), fx=%.2f, fz=%.2f, attackpos=%d\n"), m_strName, m_sNid+NPC_BAND, fX, fZ, m_byAttackPos);
		}
	}
	// Target 이 mon 인 경우
	else if (m_Target.id >= NPC_BAND
		&& m_Target.id < INVALID_BAND)
	{
		pNpc = m_pMain->m_NpcMap.GetData(m_Target.id - NPC_BAND);
		//pNpc = m_pMain->m_NpcMap[m_Target.id - NPC_BAND];
		if (pNpc == nullptr)
		{
			InitTarget();
			return;
		}

		vUser.Set(pNpc->m_fCurX, pNpc->m_fCurY, pNpc->m_fCurZ);

		vEnd22 = CalcAdaptivePosition(vNpc, vUser, 2);
		fX = vEnd22.x;
		fZ = vEnd22.z;
	}

	vDistance = vUser - vNpc;
	fDis = vDistance.Magnitude();

	// target과의 거리가 3미터 미만이면 멈춘상태에서 공격이고..
	if ((int) fDis < 3)
		return;

	// 장거리 공격이 가능한것은 공격거리로 판단..
	/*if (m_tNpcLongType != 0)
	{
		if ((int)fDis > nRange)
			return false;
	}
	// 단거리(직접공격)
	else
	{
		// 작업 :공격가능거리를 2.5로 임시 수정함..
		if (fDis > 2.5f)
			return false;
	}*/

	vDistance = vEnd22 - vNpc;
	fDis = vDistance.Magnitude();
	m_fCurX = vEnd22.x;
	m_fCurZ = vEnd22.z;

	if (m_fCurX < 0
		|| m_fCurZ < 0)
	{
		spdlog::error("Npc::MoveAttack: coordinates invalid [serial={} npcId={} npcName={} x={} z={}]",
			m_sNid + NPC_BAND, m_sSid, m_strName, m_fCurX, m_fCurZ);
	}

	// 이동공격.. 
	memset(pBuf, 0, sizeof(pBuf));
	index = 0;
	SetByte(pBuf, MOVE_RESULT, index);
	SetByte(pBuf, SUCCESS, index);
	SetShort(pBuf, m_sNid + NPC_BAND, index);
	Setfloat(pBuf, m_fCurX, index);
	Setfloat(pBuf, m_fCurZ, index);
	Setfloat(pBuf, m_fCurY, index);
	Setfloat(pBuf, fDis, index);
	//TRACE(_T("Npc moveattack --> nid = %d, cur=[x=%.2f, y=%.2f, metor=%.2f]\n"), m_sNid+NPC_BAND, m_fCurX, m_fCurZ, fDis);
	SendAll(pIOCP, pBuf, index);   // thread 에서 send

	// 이동 끝
	memset(pBuf, 0, sizeof(pBuf));
	index = 0;
	SetByte(pBuf, MOVE_RESULT, index);
	SetByte(pBuf, SUCCESS, index);
	SetShort(pBuf, m_sNid + NPC_BAND, index);
	Setfloat(pBuf, m_fCurX, index);
	Setfloat(pBuf, m_fCurZ, index);
	Setfloat(pBuf, m_fCurY, index);
	Setfloat(pBuf, 0, index);
	//TRACE(_T("Npc moveattack end --> nid = %d, cur=[x=%.2f, y=%.2f, metor=%d]\n"), m_sNid+NPC_BAND, m_fCurX, m_fCurZ, 0);
	SendAll(pIOCP, pBuf, index);   // thread 에서 send

	SetUid(m_fCurX, m_fCurZ, m_sNid + NPC_BAND);

	/* 타겟의 좌표를 최신 것으로 수정하고, 마지막 포인터 좌표를 수정한다,, */
	m_fEndPoint_X = m_fCurX;
	m_fEndPoint_Y = m_fCurZ;

	//m_Target.x = fX;
	//m_Target.z = fZ;
}

int CNpc::GetNFinalDamage(CNpc* pNpc)
{
	short damage = 0;
	float Attack = 0;
	float Avoid = 0;
	short Hit = 0;
	short Ac = 0;
	int random = 0;
	BYTE result;

	if (pNpc == nullptr)
		return damage;

	// 공격민첩
	Attack = (float) m_sHitRate;

	// 방어민첩
	Avoid = (float) pNpc->m_sEvadeRate;

	//공격자 Hit 
	Hit = m_sDamage;

	// 방어자 Ac 
	Ac = (short) pNpc->m_sDefense;

	// 타격비 구하기
	result = GetHitRate(Attack / Avoid);

	switch (result)
	{
//		case GREAT_SUCCESS:
//			damage = (short)(0.6 * (2 * Hit));
//			if(damage <= 0){
//				damage = 0;
//				break;
//			}
//			damage = myrand(0, damage);
//			damage += (short)(0.7 * (2 * Hit));
//			break;

		case GREAT_SUCCESS:
			damage = (short) (0.6 * Hit);
			if (damage <= 0)
			{
				damage = 0;
				break;
			}

			damage = myrand(0, damage);
			damage += (short) (0.7 * Hit);
			break;

		case SUCCESS:
		case NORMAL:
			if (Hit - Ac > 0)
			{
				damage = (short) (0.6 * (Hit - Ac));
				if (damage <= 0)
				{
					damage = 0;
					break;
				}

				damage = myrand(0, damage);
				damage += (short) (0.7 * (Hit - Ac));
			}
			else
			{
				damage = 0;
			}
			break;

		case FAIL:
			damage = 0;
			break;
	}

	return damage;
}

bool CNpc::IsCompStatus(CUser* pUser)
{
	if (IsHPCheck(pUser->m_sHP))
	{
		if (RandomBackMove())
			return true;
	}

	return false;
}

//	Target 의 위치가 다시 길찾기를 할 정도로 변했는지 판단
bool CNpc::IsChangePath(int nStep)
{
	// 패스파인드의 마지막 좌표를 가지고,, Target이 내 공격거리에 있는지를 판단,,
//	if(!m_pPath) return true;

	float fCurX = 0.0f, fCurZ = 0.0f;
	GetTargetPos(fCurX, fCurZ);

	__Vector3 vStart, vEnd;
	vStart.Set(m_fEndPoint_X, 0, m_fEndPoint_Y);
	vEnd.Set(fCurX, 0, fCurZ);

	float fDis = GetDistance(vStart, vEnd);
	float fCompDis = 3.0f;

	//if(fDis <= m_byAttackRange)
	if (fDis < fCompDis)
	{
		//TRACE(_T("#### Npc-IsChangePath() : [nid=%d] -> attack range in #####\n"), m_sNid);
		return false;
	}

	 //TRACE(_T("#### IsChangePath() - [몬 - cur:x=%.2f, z=%.2f, 목표점:x=%.2f, z=%.2f], [target : x=%.2f, z=%.2f]\n"), 
	//	 m_fCurX, m_fCurZ, m_fEndPoint_X, m_fEndPoint_Y, fCurX, fCurZ);
	return true;
}

//	Target 의 현재 위치를 얻는다.
bool CNpc::GetTargetPos(float& x, float& z)
{
	// Target 이 User 인 경우
	if (m_Target.id >= USER_BAND
		&& m_Target.id < NPC_BAND)
	{
		CUser* pUser = m_pMain->GetUserPtr(m_Target.id - USER_BAND);
		if (pUser == nullptr)
			return false;

		x = pUser->m_curx;
		z = pUser->m_curz;
	}
	else if (m_Target.id >= NPC_BAND
		&& m_Target.id < INVALID_BAND)
	{
		CNpc* pNpc = m_pMain->m_NpcMap.GetData(m_Target.id - NPC_BAND);
		//CNpc* pNpc = m_pMain->m_NpcMap[m_Target.id - NPC_BAND];
		if (pNpc == nullptr)
			return false;

		x = pNpc->m_fCurX;
		z = pNpc->m_fCurZ;
	}

	return true;
}

//	Target 과 NPC 간에 길찾기를 다시한다.
bool CNpc::ResetPath()
{
	float cur_x, cur_z;
	GetTargetPos(cur_x, cur_z);

//	TRACE(_T("ResetPath : user pos ,, x=%.2f, z=%.2f\n"), cur_x, cur_z);

	m_Target.x = cur_x;
	m_Target.z = cur_z;

	int nValue = GetTargetPath();

	// Target has been lost or ran away.
	if (nValue == -1)
	{
		spdlog::debug("Npc::ResetPath: target lost [serial={} npcId={} npcName={} targetX={} targetZ={}]",
			m_sNid + NPC_BAND, m_sSid, m_strName, m_Target.x, m_Target.z);
		return false;
	}

	// 타겟 방향으로 바로 간다..
	if (nValue == 0)
	{
		m_fSecForMetor = m_fSpeed_2;	// 공격일때는 뛰는 속도로... 
		IsNoPathFind(m_fSecForMetor);
	}

	//TRACE(_T("Npc-ResetPath - target_x = %.2f, z=%.2f, value=%d\n"), m_Target.x, m_Target.z, nValue);

	return true;
}

int CNpc::GetFinalDamage(CUser* pUser, int type)
{
	short damage = 0;
	float Attack = 0;
	float Avoid = 0;
	short Hit = 0;
	short Ac = 0;
	short HitB = 0;
	int random = 0;
	BYTE result;

	if (pUser == nullptr)
		return damage;

	Attack = (float) m_sHitRate;								// 공격민첩
	Avoid = (float) pUser->m_fAvoidrate;						// 방어민첩	
	Hit = m_sDamage;											// 공격자 Hit 		
//	Ac = (short) pUser->m_sAC;									// 방어자 Ac 

//	Ac = (short) pUser->m_sItemAC + (short) pUser->m_sLevel;	// 방어자 Ac 
//	Ac = (short) pUser->m_sAC - (short) pUser->m_sLevel;		// 방어자 Ac. 잉...성래씨 미워 ㅜ.ㅜ
	Ac = (short) pUser->m_sItemAC + (short) pUser->m_sLevel + (short) (pUser->m_sAC - pUser->m_sLevel - pUser->m_sItemAC);

//	ASSERT(Ac != 0);
//	short kk = (short) pUser->m_sItemAC;
//	short tt = (short) pUser->m_sLevel;
//	Ac = kk + tt;

	HitB = (int) ((Hit * 200) / (Ac + 240));

	int nMaxDamage = (int) (2.6 * m_sDamage);

	// 타격비 구하기
	result = GetHitRate(Attack / Avoid);

//	TRACE(_T("Hitrate : %d     %f/%f\n"), result, Attack, Avoid);

	switch (result)
	{
		case GREAT_SUCCESS:
	/*
			damage = (short)(0.6 * (2 * Hit));
			if(damage <= 0){
				damage = 0;
				break;
			}
			damage = myrand(0, damage);
			damage += (short)(0.7 * (2 * Hit));
			break;
	*/
	//		damage = 0;
	//		break;

			damage = (short) HitB;
			if (damage <= 0)
			{
				damage = 0;
				break;
			}

			damage = (int) (0.3f * myrand(0, damage));
			damage += (short) (0.85f * (float) HitB);
	//		damage = damage * 2;
			damage = (damage * 3) / 2;
			break;

		case SUCCESS:
	/*
			damage = (short) (0.6f * Hit);
			if (damage <= 0)
			{
				damage = 0;
				break;
			}

			damage = myrand(0, damage);
			damage += (short) (0.7f * Hit);
			break;
	*/
	/*
			damage = (short) (0.3f * (float) HitB);
			if (damage <= 0)
			{
				damage = 0;
				break;
			}

			damage = myrand(0, damage);
			damage += (short) (0.85f * (float) HitB);
	*/
	/*
			damage = (short) HitB;
			if (damage <= 0)
			{
				damage = 0;
				break;
			}

			damage = (int) (0.3f * myrand(0, damage));
			damage += (short) (0.85f * (float)HitB);
			damage = damage * 2;

			break;
	*/
		case NORMAL:
			/*
			if (Hit - Ac > 0)
			{
				damage = (short) (0.6f * (Hit - Ac));
				if (damage <= 0)
				{
					damage = 0;
					break;
				}

				damage = myrand(0, damage);
				damage += (short)(0.7f * (Hit - Ac));
			}
			else
			{
				damage = 0;
			}
			*/
	/*
			damage = (short) (0.3f * (float) HitB);
			if (damage <= 0)
			{
				damage = 0;
				break;
			}

			damage = myrand(0, damage);
			damage += (short) (0.85f * (float) HitB);
	*/
			damage = (short) HitB;
			if (damage <= 0)
			{
				damage = 0;
				break;
			}

			damage = (int) (0.3f * myrand(0, damage));
			damage += (short) (0.85f * (float) HitB);
			break;

		case FAIL:
			damage = 0;

			break;
	}

//	TRACE ("%d\n", damage);

	if (damage > nMaxDamage)
	{
		spdlog::trace("Npc::GetFinalDamage: damage exceeded maximum, clamped to max. [serial={} npcId={} npcName={} damage={} maxDamage={}]",
			m_sNid + NPC_BAND, m_sSid, m_strName, damage, nMaxDamage);
		damage = nMaxDamage;
	}

	// sungyong test
	return damage;
	//return 1;
}

//	나를 공격한 유저를 타겟으로 삼는다.(기준 : 렙과 HP를 기준으로 선정)
void CNpc::ChangeTarget(int nAttackType, CUser* pUser, CIOCPort* pIOCP)
{
	int preDamage, lastDamage;
	__Vector3 vUser, vNpc;
	float fDistance1 = 0.0f, fDistance2 = 0.0f;
	int iRandom = myrand(0, 100);

	if (pUser == nullptr)
		return;

	if (pUser->m_bLive == USER_DEAD)
		return;

	// 같은 국가는 공격을 안하도록...
	if (pUser->m_bNation == m_byGroup)
		return;

	// 운영자는 무시...^^
	if (pUser->m_byIsOP == MANAGER_USER)
		return;
	
	// 성문 NPC는 공격처리 안하게
	if (m_tNpcType == NPC_DOOR
		|| m_tNpcType == NPC_ARTIFACT
		|| m_tNpcType == NPC_PHOENIX_GATE
		|| m_tNpcType == NPC_GATE_LEVER
		|| m_tNpcType == NPC_DOMESTIC_ANIMAL
		|| m_tNpcType == NPC_SPECIAL_GATE
		|| m_tNpcType == NPC_DESTORY_ARTIFACT)	
		return;

	// 기절상태이면 무시..
	if (m_NpcState == NPC_FAINTING)
		return;

	CUser* preUser = nullptr;
	if (m_Target.id >= 0
		&& m_Target.id < NPC_BAND)
		preUser = m_pMain->GetUserPtr(m_Target.id - USER_BAND);

	if (pUser == preUser)
	{
		// 가족타입이면 시야안에 같은 타입에게 목표 지정
		if (m_tNpcGroupType)
		{
			m_Target.failCount = 0;
			if (m_tNpcType == NPC_BOSS_MONSTER)
				FindFriend(1);
			else
				FindFriend();
		}
		else
		{
			if (m_tNpcType == NPC_BOSS_MONSTER)
			{
				m_Target.failCount = 0;
				FindFriend(1);
			}
		}
		return;
	}
	if (preUser != nullptr
		/* && preUser->m_state == STATE_GAMESTARTED */)
	{
		preDamage = 0;
		lastDamage = 0;

		// 몬스터 자신을 가장 강하게 타격한 유저
		if (iRandom >= 0
			&& iRandom < 50)
		{
			preDamage = preUser->GetDamage(m_sNid + NPC_BAND);
			lastDamage = pUser->GetDamage(m_sNid + NPC_BAND);
			//TRACE(_T("Npc-changeTarget 111 - iRandom=%d, pre=%d, last=%d\n"), iRandom, preDamage, lastDamage);
			if (preDamage > lastDamage)
				return;
		}
		// 가장 가까운 플레이어
		else if (iRandom >= 50
			&& iRandom < 80)
		{
			vNpc.Set(m_fCurX, m_fCurY, m_fCurZ);
			vUser.Set(preUser->m_curx, 0, preUser->m_curz);
			fDistance1 = GetDistance(vNpc, vUser);
			vUser.Set(pUser->m_curx, 0, pUser->m_curz);
			fDistance2 = GetDistance(vNpc, vUser);
			//TRACE(_T("Npc-changeTarget 222 - iRandom=%d, preDis=%.2f, lastDis=%.2f\n"), iRandom, fDistance1, fDistance2);
			if (fDistance2 > fDistance1)
				return;
		}

		// 몬스터가 유저에게 가장 많이 타격을 줄 수 있는 유저
		if (iRandom >= 80
			&& iRandom < 95)
		{
			preDamage = GetFinalDamage(preUser, 0);
			lastDamage = GetFinalDamage(pUser, 0);
			//TRACE(_T("Npc-changeTarget 333 - iRandom=%d, pre=%d, last=%d\n"), iRandom, preDamage, lastDamage);
			if (preDamage > lastDamage)
				return;
		}

		// Heal Magic을 사용한 유저
		if (iRandom >= 95
			&& iRandom < 101)
		{
		}
	}
	// Heal magic에 반응하지 않도록..
	else if (preUser == nullptr
		&& nAttackType == 1004)
	{
		return;
	}

	m_Target.id = pUser->m_iUserId + USER_BAND;
	m_Target.x = pUser->m_curx;
	m_Target.y = pUser->m_cury;
	m_Target.z = pUser->m_curz;

	//TRACE(_T("Npc-changeTarget - target_x = %.2f, z=%.2f\n"), m_Target.x, m_Target.z);

	int nValue = 0;

	// 어슬렁 거리는데 공격하면 바로 반격
	if (m_NpcState == NPC_STANDING
		|| m_NpcState == NPC_MOVING
		|| m_NpcState == NPC_SLEEPING)
	{
		// 가까이 있으면 반격으로 이어지구
		if (IsCloseTarget(pUser, m_byAttackRange))
		{
			m_NpcState = NPC_FIGHTING;
			m_Delay = 0;
			m_fDelayTime = TimeGet();
		}
		// 바로 도망가면 좌표를 갱신하고 추적	
		else
		{
			nValue = GetTargetPath(1);

			// 반격 동작후 약간의 딜레이 시간이 있음	
			if (nValue == 1)
			{
				m_NpcState = NPC_TRACING;
				m_Delay = 0;
				m_fDelayTime = TimeGet();
			}
			else if (nValue == -1)
			{
				m_NpcState = NPC_STANDING;
				m_Delay = 0;
				m_fDelayTime = TimeGet();
			}
			else if (nValue == 0)
			{
				m_fSecForMetor = m_fSpeed_2;	// 공격일때는 뛰는 속도로... 
				IsNoPathFind(m_fSecForMetor);
				m_NpcState = NPC_TRACING;
				m_Delay = 0;
				m_fDelayTime = TimeGet();
			}
		}
	}
//	else m_NpcState = NPC_ATTACKING;	// 한참 공격하는데 누가 방해하면 목표를 바꿈

// 가족타입이면 시야안에 같은 타입에게 목표 지정
	if (m_tNpcGroupType)
	{
		m_Target.failCount = 0;
		if (m_tNpcType == NPC_BOSS_MONSTER)
			FindFriend(1);
		else
			FindFriend();
	}
	else
	{
		if (m_tNpcType == NPC_BOSS_MONSTER)
		{
			m_Target.failCount = 0;
			FindFriend(1);
		}
	}
}

//	나를 공격한 Npc를 타겟으로 삼는다.(기준 : 렙과 HP를 기준으로 선정)
void CNpc::ChangeNTarget(CNpc* pNpc, CIOCPort* pIOCP)
{
	int preDamage, lastDamage;
	__Vector3 vMonster, vNpc;
	float fDist = 0.0f;

	if (pNpc == nullptr)
		return;

	if (pNpc->m_NpcState == NPC_DEAD)
		return;

	CNpc* preNpc = nullptr;
	if (m_Target.id >= 0
		&& m_Target.id < NPC_BAND)
	{
	}
	else if (m_Target.id >= NPC_BAND
		&& m_Target.id < INVALID_BAND)
	{
		preNpc = m_pMain->m_NpcMap.GetData(m_Target.id - NPC_BAND);
	}

	if (pNpc == preNpc)
		return;

	if (preNpc != nullptr)
	{
		preDamage = 0;
		lastDamage = 0;
		preDamage = GetNFinalDamage(preNpc);
		lastDamage = GetNFinalDamage(pNpc);

		// 조건을 검색,,   (거리와 유저의 공격력을 판단해서,,)
		vNpc.Set(m_fCurX, m_fCurY, m_fCurZ);
		vMonster.Set(preNpc->m_fCurX, 0, preNpc->m_fCurZ);
		fDist = GetDistance(vNpc, vMonster);
		preDamage = (int) ((double) preDamage / fDist + 0.5);
		vMonster.Set(pNpc->m_fCurX, 0, pNpc->m_fCurZ);
		fDist = GetDistance(vNpc, vMonster);
		lastDamage = (int) ((double) lastDamage / fDist + 0.5);

		if (preDamage > lastDamage)
			return;
	}

	m_Target.id = pNpc->m_sNid + NPC_BAND;
	m_Target.x = pNpc->m_fCurX;
	m_Target.y = pNpc->m_fCurZ;
	m_Target.z = pNpc->m_fCurZ;

	int nValue = 0;

	// 어슬렁 거리는데 공격하면 바로 반격
	if (m_NpcState == NPC_STANDING
		|| m_NpcState == NPC_MOVING
		|| m_NpcState == NPC_SLEEPING)
	{	
		// 가까이 있으면 반격으로 이어지구
		if (IsCloseTarget(m_byAttackRange) == 1)
		{
			m_NpcState = NPC_FIGHTING;
			m_Delay = 0;
			m_fDelayTime = TimeGet();
		}
		// 바로 도망가면 좌표를 갱신하고 추적	
		else
		{
			nValue = GetTargetPath();

			// 반격 동작후 약간의 딜레이 시간이 있음	
			if (nValue == 1)
			{
				m_NpcState = NPC_TRACING;
				m_Delay = 0;
				m_fDelayTime = TimeGet();
			}
			else if (nValue == -1)
			{
				m_NpcState = NPC_STANDING;
				m_Delay = 0;
				m_fDelayTime = TimeGet();
			}
			else if (nValue == 0)
			{
				m_fSecForMetor = m_fSpeed_2;	// 공격일때는 뛰는 속도로... 
				IsNoPathFind(m_fSecForMetor);
				m_NpcState = NPC_TRACING;
				m_Delay = 0;
				m_fDelayTime = TimeGet();
			}
		}
	}
//	else m_NpcState = NPC_ATTACKING;	// 한참 공격하는데 누가 방해하면 목표를 바꿈

	if (m_tNpcGroupType)					// 가족타입이면 시야안에 같은 타입에게 목표 지정
	{
		m_Target.failCount = 0;
		FindFriend();
	}
}

//	NPC 의 방어력을 얻어온다.
int CNpc::GetDefense()
{
	return m_sDefense;
}

//	Damage 계산, 만약 m_iHP 가 0 이하이면 사망처리
bool CNpc::SetDamage(int nAttackType, int nDamage, const char* sourceName, int uid, CIOCPort* pIOCP)
{
	int i = 0, len = 0;
	int userDamage = 0;
	bool bFlag = false;
	_ExpUserList* tempUser = nullptr;

	if (m_NpcState == NPC_DEAD)
		return true;

	if (m_iHP <= 0)
		return true;

	if (nDamage < 0)
		return true;

	// Npc의 포인터가 잘못된 경우에는 리턴..
	MAP* pMap = m_pMain->GetMapByIndex(m_ZoneIndex);
	if (pMap == nullptr)
	{
		spdlog::error("Npc::SetDamage: map not found [zoneIndex={} npcId={} npcName={}]",
					m_strName, m_sSid, m_ZoneIndex);
		return true;
	}

	CUser* pUser = nullptr;
	CNpc* pNpc = nullptr;
	char strDurationID[MAX_ID_SIZE + 1] = {};

	// Target 이 User 인 경우
	if (uid >= USER_BAND
		&& uid < NPC_BAND)
	{
		pUser = m_pMain->GetUserPtr(uid);	// 해당 사용자인지 인증
		if (pUser == nullptr)
			return true;
	}
	// Target 이 mon 인 경우
	else if (uid >= NPC_BAND
		&& m_Target.id < INVALID_BAND)
	{
		pNpc = m_pMain->m_NpcMap.GetData(uid - NPC_BAND);
		if (pNpc == nullptr)
			return true;

		userDamage = nDamage;
		goto go_result;
	}

	userDamage = nDamage;
													// 잉여 데미지는 소용없다.		
	if ((m_iHP - nDamage) < 0)
		userDamage = m_iHP;

	for (i = 0; i < NPC_HAVE_USER_LIST; i++)
	{
		if (m_DamagedUserList[i].iUid == uid)
		{
			if (_stricmp("**duration**", sourceName) == 0)
			{
				bFlag = true;
				strcpy(strDurationID, pUser->m_strUserID);
				if (_stricmp(m_DamagedUserList[i].strUserID, strDurationID) == 0)
				{
					m_DamagedUserList[i].nDamage += userDamage;
					goto go_result;
				}
			}
			else if (_stricmp(m_DamagedUserList[i].strUserID, sourceName) == 0)
			{
				m_DamagedUserList[i].nDamage += userDamage;
				goto go_result;
			}
		}
	}

	// Does the player limit affect the final damage? || 인원 제한이 최종 대미지에 영향을 미치나?
	for (i = 0; i < NPC_HAVE_USER_LIST; i++)
	{
		if (m_DamagedUserList[i].iUid == -1)
		{
			if (m_DamagedUserList[i].nDamage <= 0)
			{
				len = strlen(sourceName);
				if (len > MAX_ID_SIZE
					|| len <= 0)
				{
					spdlog::error("Npc::SetDamage: sourceName length out of bounds [serial={} npcId={} npcName={} len={} sourceName={}]",
						m_sNid + NPC_BAND, m_sSid, m_strName, len, sourceName);
					continue;
				}

				if (bFlag)
				{
					strcpy(m_DamagedUserList[i].strUserID, strDurationID);
				}
				else
				{
					if (_stricmp("**duration**", sourceName) == 0)
						strcpy(m_DamagedUserList[i].strUserID, pUser->m_strUserID);
					else
						strcpy(m_DamagedUserList[i].strUserID, sourceName);
				}
				m_DamagedUserList[i].iUid = uid;
				m_DamagedUserList[i].nDamage = userDamage;
				m_DamagedUserList[i].bIs = false;
				break;
			}
		}
	}

go_result:
	m_TotalDamage += userDamage;
	m_iHP -= nDamage;

	if (m_iHP <= 0)
	{
	//	m_ItemUserLevel = pUser->m_sLevel;
		m_iHP = 0;
		Dead(pIOCP);
		return false;
	}

	int iRandom = myrand(1, 100);
	int iLightningR = 0;

	// Target 이 User 인 경우
	if (uid >= USER_BAND
		&& uid < NPC_BAND)
	{
		// 기절 시키는 스킬을 사용했다면..
		if (nAttackType == 3
			&& m_NpcState != NPC_FAINTING)
		{
			// 확률 계산..
			iLightningR = 10 + (40 - 40 * ((double) m_sLightningR / 80));
			if (COMPARE(iRandom, 0, iLightningR))
			{
				m_NpcState = NPC_FAINTING;
				m_Delay = 0;
				m_fDelayTime = TimeGet();
				m_fFaintingTime = TimeGet();
			}
			else
			{
				ChangeTarget(nAttackType, pUser, pIOCP);
			}
		}
		else
		{
			ChangeTarget(nAttackType, pUser, pIOCP);
		}
	}

	// Target 이 mon 인 경우
	if (uid >= NPC_BAND
		&& m_Target.id < INVALID_BAND)
		ChangeNTarget(pNpc, pIOCP);

	return true;
}

// Heal계열 마법공격
bool CNpc::SetHMagicDamage(int nDamage, CIOCPort* pIOCP)
{
	if (m_NpcState == NPC_DEAD)
		return false;

	if (m_iHP <= 0)
		return false;

	if (nDamage <= 0)
		return false;

	// 죽기직전일때는 회복 안됨...
	if (m_iHP < 1)
		return false;

	if (pIOCP == nullptr)
		return false;

	char buff[256] = {};
	int send_index = 0, oldHP = 0;

	oldHP = m_iHP;
	m_iHP += nDamage;
	if (m_iHP < 0)
		m_iHP = 0;
	else if (m_iHP > m_iMaxHP)
		m_iHP = m_iMaxHP;

	spdlog::trace("Npc::SetHMagicDamage: [serial={} npcId={} npcName={} oldHp={} newHp={}]",
		m_sNid + NPC_BAND, m_sSid, m_strName, oldHP, m_iHP);
	SetByte(buff, AG_USER_SET_HP, send_index);
	SetShort(buff, m_sNid + NPC_BAND, send_index);
	SetDWORD(buff, m_iHP, send_index);
	SendAll(pIOCP, buff, send_index);

	return true;
}

//	NPC 사망처리시 경험치 분배를 계산한다.(일반 유저와 버디 사용자구분)
void CNpc::SendExpToUserList()
{
	MAP* pMap = m_pMain->GetMapByIndex(m_ZoneIndex);
	if (pMap == nullptr)
	{
		spdlog::error("Npc::SendExpToUserList: map not found [zoneIndex={} npcId={} npcName={}]",
			m_strName, m_sSid, m_ZoneIndex);
		return;
	}

	int i = 0;
	int nExp = 0;
	int nPartyExp = 0;
	int nLoyalty = 0;
	int nPartyLoyalty = 0;
	double totalDamage = 0;
	double CompDamage = 0;
	double TempValue = 0;
	int nPartyNumber = -1;
	int nUid = -1;
	CUser* pUser = nullptr;
	CUser* pPartyUser = nullptr;
	CUser* pMaxDamageUser = nullptr;
	_PARTY_GROUP* pParty = nullptr;
	char strMaxDamageUser[MAX_ID_SIZE + 1] = {};

	IsUserInSight();	// 시야권내에 있는 유저 셋팅..

	// 일단 리스트를 검색한다.
	for (i = 0; i < NPC_HAVE_USER_LIST; i++)
	{
		if (m_DamagedUserList[i].iUid < 0
			|| m_DamagedUserList[i].nDamage <= 0)
			continue;

		if (m_DamagedUserList[i].bIs)
			pUser = m_pMain->GetUserPtr(m_DamagedUserList[i].iUid);
		if (pUser == nullptr)
			continue;

		// 파티 소속
		if (pUser->m_byNowParty == 1)
		{
			totalDamage = GetPartyDamage(pUser->m_sPartyNumber);
			if (totalDamage == 0
				|| m_TotalDamage == 0)
			{
				nPartyExp = 0;
			}
			else
			{
				if (CompDamage < totalDamage)
				{
					CompDamage = totalDamage;
					m_sMaxDamageUserid = m_DamagedUserList[i].iUid;
					pMaxDamageUser = m_pMain->GetUserPtr(m_DamagedUserList[i].iUid);
					if (pMaxDamageUser == nullptr)
					{
						m_byMaxDamagedNation = pUser->m_bNation;
						strcpy(strMaxDamageUser, pUser->m_strUserID);
					}
					else
					{
						m_byMaxDamagedNation = pMaxDamageUser->m_bNation;
						strcpy(strMaxDamageUser, pMaxDamageUser->m_strUserID);
					}
				}

				TempValue = m_iExp * (totalDamage / m_TotalDamage);
				nPartyExp = (int) TempValue;
				if (TempValue > nPartyExp)
					++nPartyExp;
			}

			if (m_iLoyalty == 0
				|| totalDamage == 0
				|| m_TotalDamage == 0)
			{
				nPartyLoyalty = 0;
			}
			else
			{
				TempValue = m_iLoyalty * (totalDamage / m_TotalDamage);
				nPartyLoyalty = (int) TempValue;
				if (TempValue > nPartyLoyalty)
					++nPartyLoyalty;
			}

			// 파티원 전체를 돌면서 경험치 분배
			if (i != 0)
			{
				bool bFlag = false;
				int count = 0;
				for (int j = 0; j < i; j++)
				{
					if (m_DamagedUserList[j].iUid < 0
						|| m_DamagedUserList[j].nDamage <= 0)
						continue;

					if (m_DamagedUserList[j].bIs)
						pPartyUser = m_pMain->GetUserPtr(m_DamagedUserList[j].iUid);

					if (pPartyUser == nullptr)
						continue;

					if (pUser->m_sPartyNumber == pPartyUser->m_sPartyNumber)
						continue;

					++count;
				}

				if (count == i)
					bFlag = true;

				// 여기에서 또 작업...
				if (bFlag)
				{
					int uid = 0;
					pParty = m_pMain->m_PartyMap.GetData(pUser->m_sPartyNumber);
					if (pParty != nullptr)
					{
						int nTotalMan = 0, nTotalLevel = 0;
						for (int j = 0; j < 8; j++)
						{
							uid = pParty->uid[j];
							pPartyUser = m_pMain->GetUserPtr(uid);
							if (pPartyUser == nullptr)
								continue;

							++nTotalMan;
							nTotalLevel += pPartyUser->m_sLevel;
						}

						nPartyExp = GetPartyExp(nTotalLevel, nTotalMan, nPartyExp);
						//TRACE(_T("* PartyUser GetPartyExp total_level=%d, total_man = %d, exp=%d *\n"), nTotalLevel, nTotalMan, nPartyExp);

						for (int k = 0; k < 8; k++)
						{
							uid = pParty->uid[k];
							pPartyUser = m_pMain->GetUserPtr(uid);
							if (pPartyUser == nullptr)
								continue;

							// monster와 거리를 판단
							if (!IsInExpRange(pPartyUser))
								continue;

							TempValue = (nPartyExp * (1 + 0.3 * (nTotalMan - 1))) * (double) pPartyUser->m_sLevel / (double) nTotalLevel;
							//TempValue = ( nPartyExp * ( 1+0.3*( nTotalMan-1 ) ) );
							nExp = (int) TempValue;

							if (TempValue > nExp)
								++nExp;

							if (nPartyLoyalty <= 0)
							{
								nLoyalty = 0;
							}
							else
							{
								TempValue = (nPartyLoyalty * (1 + 0.2 * (nTotalMan - 1))) * (double) pPartyUser->m_sLevel / (double) nTotalLevel;
								nLoyalty = (int) TempValue;
								if (TempValue > nLoyalty)
									++nLoyalty;
							}

							//TRACE(_T("* PartyUser Exp id=%hs, damage=%d, total=%d, exp=%d, loral=%d, level=%d/%d *\n"), pPartyUser->m_strUserID, (int)totalDamage, m_TotalDamage, nExp, nLoyalty, pPartyUser->m_sLevel, nTotalLevel);
							//pPartyUser->SetExp(nExp, nLoyalty, m_sLevel);
							pPartyUser->SetPartyExp(nExp, nLoyalty, nTotalLevel, nTotalMan);
						}
					}
				}
			}
			else if (i == 0)
			{
				int uid = 0;
				pParty = m_pMain->m_PartyMap.GetData(pUser->m_sPartyNumber);
				if (pParty != nullptr)
				{
					int nTotalMan = 0, nTotalLevel = 0;
					for (int j = 0; j < 8; j++)
					{
						uid = pParty->uid[j];
						pPartyUser = m_pMain->GetUserPtr(uid);
						if (pPartyUser == nullptr)
							continue;

						++nTotalMan;
						nTotalLevel += pPartyUser->m_sLevel;
					}

					nPartyExp = GetPartyExp(nTotalLevel, nTotalMan, nPartyExp);
					//TRACE(_T("* PartyUser GetPartyExp total_level=%d, total_man = %d, exp=%d *\n"), nTotalLevel, nTotalMan, nPartyExp);

					for (int k = 0; k < 8; k++)
					{
						uid = pParty->uid[k];
						pPartyUser = m_pMain->GetUserPtr(uid);
						if (pPartyUser == nullptr)
							continue;

						// monster와 거리를 판단
						if (!IsInExpRange(pPartyUser))
							continue;

						TempValue = (nPartyExp * (1 + 0.3 * (nTotalMan - 1))) * (double) pPartyUser->m_sLevel / (double) nTotalLevel;
						//TempValue = ( nPartyExp * ( 1+0.3*( nTotalMan-1 ) ) );
						nExp = (int) TempValue;

						if (TempValue > nExp)
							++nExp;

						if (nPartyLoyalty <= 0)
						{
							nLoyalty = 0;
						}
						else
						{
							TempValue = (nPartyLoyalty * (1 + 0.2 * (nTotalMan - 1))) * (double) pPartyUser->m_sLevel / (double) nTotalLevel;
							nLoyalty = (int) TempValue;
							if (TempValue > nLoyalty)
								++nLoyalty;
						}

						//TRACE(_T("* PartyUser Exp id=%hs, damage=%d, total=%d, exp=%d, loral=%d, level=%d/%d *\n"), pPartyUser->m_strUserID, (int)totalDamage, m_TotalDamage, nExp, nLoyalty, pPartyUser->m_sLevel, nTotalLevel);
						//pPartyUser->SetExp(nExp, nLoyalty, m_sLevel);
						pPartyUser->SetPartyExp(nExp, nLoyalty, nTotalLevel, nTotalMan);
					}
				}
			}
			//nExp = 
		}
		// 부대 소속
		else if (pUser->m_byNowParty == 2)
		{
		}
		// 개인
		else
		{
			totalDamage = m_DamagedUserList[i].nDamage;

			if (totalDamage == 0
				|| m_TotalDamage == 0)
			{
				nExp = 0;
				nLoyalty = 0;
			}
			else
			{
				if (CompDamage < totalDamage)
				{
					CompDamage = totalDamage;
					m_sMaxDamageUserid = m_DamagedUserList[i].iUid;
					pMaxDamageUser = m_pMain->GetUserPtr(m_DamagedUserList[i].iUid);
					if (pMaxDamageUser == nullptr)
					{
						m_byMaxDamagedNation = pUser->m_bNation;
						strcpy(strMaxDamageUser, pUser->m_strUserID);
					}
					else
					{
						m_byMaxDamagedNation = pMaxDamageUser->m_bNation;
						strcpy(strMaxDamageUser, pMaxDamageUser->m_strUserID);
					}
				}

				TempValue = m_iExp * (totalDamage / m_TotalDamage);
				nExp = (int) TempValue;
				if (TempValue > nExp)
					++nExp;

				if (m_iLoyalty == 0)
				{
					nLoyalty = 0;
				}
				else
				{
					TempValue = m_iLoyalty * (totalDamage / m_TotalDamage);
					nLoyalty = (int) TempValue;
					if (TempValue > nLoyalty)
						++nLoyalty;
				}

				//TRACE(_T("* User Exp id=%hs, damage=%d, total=%d, exp=%d, loral=%d *\n"), pUser->m_strUserID, (int)totalDamage, m_TotalDamage, nExp, nLoyalty);
				pUser->SetExp(nExp, nLoyalty, m_sLevel);
			}
		}
	}

	// 전쟁중
	if (m_pMain->m_byBattleEvent == BATTLEZONE_OPEN)
	{
		// 죽었을때 데미지를 많이 입힌 유저를 기록해 주세여
		if (m_bySpecialType >= 90
			&& m_bySpecialType <= 100)
		{
			// 몬스터에게 가장 데미지를 많이 입힌 유저의 이름을 전송
			if (strlen(strMaxDamageUser) != 0)
			{
				char send_buff[100] = {};
				int send_index = 0;
				SetByte(send_buff, AG_BATTLE_EVENT, send_index);
				SetByte(send_buff, BATTLE_EVENT_MAX_USER, send_index);
				if (m_bySpecialType == 100)
				{
					SetByte(send_buff, 1, send_index);
				}
				else if (m_bySpecialType == 90)
				{
					SetByte(send_buff, 3, send_index);
					m_pMain->m_sKillKarusNpc++;
				}
				else if (m_bySpecialType == 91)
				{
					SetByte(send_buff, 4, send_index);
					m_pMain->m_sKillKarusNpc++;
				}
				else if (m_bySpecialType == 92)
				{
					SetByte(send_buff, 5, send_index);
					m_pMain->m_sKillElmoNpc++;
				}
				else if (m_bySpecialType == 93)
				{
					SetByte(send_buff, 6, send_index);
					m_pMain->m_sKillElmoNpc++;
				}
				else if (m_bySpecialType == 98)
				{
					SetByte(send_buff, 7, send_index);
					m_pMain->m_sKillKarusNpc++;
				}
				else if (m_bySpecialType == 99)
				{
					SetByte(send_buff, 8, send_index);
					m_pMain->m_sKillElmoNpc++;
				}

				SetString1(send_buff, strMaxDamageUser, send_index);
				m_pMain->Send(send_buff, send_index, m_sCurZone);
				spdlog::info("Npc::SendExpToUserList: maxDamageUser={} [serial={} npcId={} npcName={}]",
					strMaxDamageUser, m_sNid + NPC_BAND, m_sSid, m_strName);

				memset(send_buff, 0, sizeof(send_buff));
				send_index = 0;

				if (m_pMain->m_sKillKarusNpc == pMap->m_sKarusRoom)
				{
					SetByte(send_buff, AG_BATTLE_EVENT, send_index);
					SetByte(send_buff, BATTLE_EVENT_RESULT, send_index);
					SetByte(send_buff, ELMORAD_ZONE, send_index);
					SetString1(send_buff, strMaxDamageUser, send_index);
					m_pMain->Send(send_buff, send_index, m_sCurZone);
					spdlog::info("Npc::SendExpToUserList: Karus Victory [killKarusNpc={} karusRoom={}]",
					m_pMain->m_sKillKarusNpc, pMap->m_sKarusRoom);
				}
				else if (m_pMain->m_sKillElmoNpc == pMap->m_sElmoradRoom)
				{
					SetByte(send_buff, AG_BATTLE_EVENT, send_index);
					SetByte(send_buff, BATTLE_EVENT_RESULT, send_index);
					SetByte(send_buff, KARUS_ZONE, send_index);
					SetString1(send_buff, strMaxDamageUser, send_index);
					m_pMain->Send(send_buff, send_index, m_sCurZone);
					spdlog::info("Npc::SendExpToUserList: Elmorad Victory [killElmoNpc={} elmoradRoom={}]",
					m_pMain->m_sKillElmoNpc, pMap->m_sElmoradRoom);
				}
			}
		}
	}
}

int CNpc::SendDead(CIOCPort* pIOCP, int type)
{
	if (pIOCP == nullptr)
		return 0;

	if (m_NpcState != NPC_DEAD
		|| m_iHP > 0)
		return 0;

	// 아이템 떨구기(경비면이면 안떨어트림)
	if (type != 0)
		GiveNpcHaveItem(pIOCP);

	return m_sRegenTime;
}

//	NPC와 Target 과의 거리가 지정 범위보다 작은지 판단
bool CNpc::IsCloseTarget(CUser* pUser, int nRange)
{
	if (pUser == nullptr)
		return false;

	if (pUser->m_sHP <= 0
		/* || pUser->m_state != STATE_GAMESTARTED*/
		|| !pUser->m_bLive)
		return false;

	__Vector3 vUser;
	__Vector3 vNpc;
	float fDis = 0.0f;
	vNpc.Set(m_fCurX, m_fCurY, m_fCurZ);
	vUser.Set(pUser->m_curx, pUser->m_cury, pUser->m_curz);
	fDis = GetDistance(vNpc, vUser);

	// 공격받은 상태기 때문에 2배의 거리감지영역,,
	if ((int) fDis > nRange * 2)
		return false;

	//InitTarget();

	m_Target.id = pUser->m_iUserId + USER_BAND;
	m_Target.x = pUser->m_curx;
	m_Target.y = pUser->m_cury;
	m_Target.z = pUser->m_curz;

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// 시야 범위내의 내동료를 찾는다.
// type = 0: 같은 그룹이면서 같은 패밀리 타입만 도움, 1:그룹이나 패밀리에 관계없이 도움, 
//        2:사제NPC가 같은 아군의 상태를 체크해서 치료할 목적으로,, 리턴으로 치료될 아군의 NID를 리턴한다
int CNpc::FindFriend(int type)
{
	MAP* pMap = m_pMain->GetMapByIndex(m_ZoneIndex);
	if (pMap == nullptr)
	{
		spdlog::error("Npc::FindFriend: map not found [zoneIndex={} npcId={} npcName={}]",
			m_strName, m_sSid, m_ZoneIndex);
		return 0;
	}

	if (m_bySearchRange == 0)
		return 0;

	if (type != 2
		&& m_Target.id == -1)
		return 0;

	int max_xx = pMap->m_sizeRegion.cx;
	int max_zz = pMap->m_sizeRegion.cy;

	int min_x = (int) (m_fCurX - m_bySearchRange) / VIEW_DIST;
	if (min_x < 0)
		min_x = 0;

	int min_z = (int) (m_fCurZ - m_bySearchRange) / VIEW_DIST;
	if (min_z < 0)
		min_z = 0;

	int max_x = (int) (m_fCurX + m_bySearchRange) / VIEW_DIST;
	if (max_x >= max_xx)
		max_x = max_xx - 1;

	int max_z = (int) (m_fCurZ + m_bySearchRange) / VIEW_DIST;
	if (min_z >= max_zz)
		min_z = max_zz - 1;

	int search_x = max_x - min_x + 1;
	int search_z = max_z - min_z + 1;

	int i, j, count = 0;
	_TargetHealer arHealer[9];
	for (i = 0; i < 9; i++)
	{
		arHealer[i].sNID = -1;
		arHealer[i].sValue = 0;
	}

	for (i = 0; i < search_x; i++)
	{
		for (j = 0; j < search_z; j++)
		{
			FindFriendRegion(min_x + i, min_z + j, pMap, &arHealer[count], type);
			//FindFriendRegion(min_x+i, min_z+j, pMap, type);
		}
	}

	int iValue = 0, iMonsterNid = 0;
	for (i = 0; i < 9; i++)
	{
		if (iValue < arHealer[i].sValue)
		{
			iValue = arHealer[i].sValue;
			iMonsterNid = arHealer[i].sNID;
		}
	}

	if (iMonsterNid != 0)
	{
		m_Target.id = iMonsterNid;
		return iMonsterNid;
	}

	return 0;
}

void CNpc::FindFriendRegion(int x, int z, MAP* pMap, _TargetHealer* pHealer, int type)
//void CNpc::FindFriendRegion(int x, int z, MAP* pMap, int type)
{
	// 자신의 region에 있는 UserArray을 먼저 검색하여,, 가까운 거리에 유저가 있는지를 판단..
	if (x < 0
		|| z < 0
		|| x > pMap->GetXRegionMax()
		|| z > pMap->GetZRegionMax())
	{
		spdlog::error("Npc::FindFriendRegion: out of region bounds [serial={} npcId={} x={} z={}]",
			m_sNid + NPC_BAND, m_sSid, x, z);
		return;
	}

	int* pNpcIDList = nullptr;
	int total_mon = 0, count = 0, nid = 0;

	EnterCriticalSection(&g_region_critical);

	auto Iter1 = pMap->m_ppRegion[x][z].m_RegionNpcArray.begin();
	auto Iter2 = pMap->m_ppRegion[x][z].m_RegionNpcArray.end();

	total_mon = pMap->m_ppRegion[x][z].m_RegionNpcArray.GetSize();
	pNpcIDList = new int[total_mon];
	for (; Iter1 != Iter2; Iter1++)
	{
		nid = *((*Iter1).second);
		pNpcIDList[count] = nid;
		count++;
	}
	LeaveCriticalSection(&g_region_critical);

	CNpc* pNpc = nullptr;
	__Vector3 vStart, vEnd;
	float fDis = 0.0f;
	// 공격 받은 상태이기때문에.. searchrange를 2배로..
	float fSearchRange = 0.0f;
	if (type == 2)
		fSearchRange = (float) m_byAttackRange;
	else
		fSearchRange = (float) m_byTracingRange;
	vStart.Set(m_fCurX, m_fCurY, m_fCurZ);
	int iValue = 0, iCompValue = 0, iHP = 0;

	for (int i = 0; i < total_mon; i++)
	{
		nid = pNpcIDList[i];
		if (nid < NPC_BAND)
			continue;

		pNpc = m_pMain->m_NpcMap.GetData(nid - NPC_BAND);

		if (pNpc != nullptr
			&& pNpc->m_NpcState != NPC_DEAD
			&& pNpc->m_sNid != m_sNid)
		{
			vEnd.Set(pNpc->m_fCurX, pNpc->m_fCurY, pNpc->m_fCurZ);
			fDis = GetDistance(vStart, vEnd);

			// 여기에서 나의 공격거리에 있는 유저인지를 판단
			if (fDis > fSearchRange)
				continue;

			if (type == 1)
			{
				if (m_sNid != pNpc->m_sNid)
				{
					if (pNpc->m_Target.id > -1
						&& pNpc->m_NpcState == NPC_FIGHTING)
						continue;

					pNpc->m_Target.id = m_Target.id;		// 모든 동료에게 도움을 요청한다.
					pNpc->m_Target.x = m_Target.x;			// 같은 목표를 공격하자고...
					pNpc->m_Target.y = m_Target.y;
					pNpc->m_Target.z = m_Target.z;
					pNpc->m_Target.failCount = 0;
					pNpc->NpcStrategy(NPC_ATTACK_SHOUT);
				}
			}
			else if (type == 0)
			{
				if (pNpc->m_tNpcGroupType
					&& m_sNid != pNpc->m_sNid
					&& pNpc->m_byFamilyType == m_byFamilyType)
				{
					if (pNpc->m_Target.id > -1
						&& pNpc->m_NpcState == NPC_FIGHTING)
						continue;

					pNpc->m_Target.id = m_Target.id;		// 같은 타입의 동료에게 도움을 요청한다.
					pNpc->m_Target.x = m_Target.x;			// 같은 목표를 공격하자고...
					pNpc->m_Target.y = m_Target.y;
					pNpc->m_Target.z = m_Target.z;
					pNpc->m_Target.failCount = 0;
					pNpc->NpcStrategy(NPC_ATTACK_SHOUT);
				}
			}
			else if (type == 2)
			{
				if (pHealer == nullptr)
					continue;

				// HP상태를 체크
				iHP = pNpc->m_iMaxHP * 0.9;

				// HP 체크
				if (pNpc->m_iHP <= iHP)
				{
					iCompValue = (pNpc->m_iMaxHP - pNpc->m_iHP) / (pNpc->m_iMaxHP * 0.01);
					if (iValue < iCompValue)
					{
						iValue = iCompValue;
						pHealer->sNID = pNpc->m_sNid + NPC_BAND;
						pHealer->sValue = iValue;
					}
				}
			}
		}
	}

	delete[] pNpcIDList;
	pNpcIDList = nullptr;
}

void CNpc::NpcStrategy(BYTE type)
{
	switch (type)
	{
		case NPC_ATTACK_SHOUT:
			m_NpcState = NPC_TRACING;
			m_Delay = m_sSpeed;//STEP_DELAY;
			m_fDelayTime = TimeGet();
			break;
	}
}

//	NPC 정보를 버퍼에 저장한다.
void CNpc::FillNpcInfo(char* temp_send, int& index, BYTE flag)
{
	SetByte(temp_send, AG_NPC_INFO, index);
	if (m_bySpecialType == 5 && m_byChangeType == 0)
		SetByte(temp_send, 0, index);					// region에 등록하지 말아라
	else
		SetByte(temp_send, 1, index);					// region에 등록		
	SetShort(temp_send, m_sNid + NPC_BAND, index);
	SetShort(temp_send, m_sSid, index);
	SetShort(temp_send, m_sPid, index);
	SetShort(temp_send, m_sSize, index);
	SetInt(temp_send, m_iWeapon_1, index);
	SetInt(temp_send, m_iWeapon_2, index);
	SetShort(temp_send, m_sCurZone, index);
	SetShort(temp_send, m_ZoneIndex, index);
	SetVarString(temp_send, m_strName.c_str(), static_cast<int>(m_strName.length()), index);
	SetByte(temp_send, m_byGroup, index);
	SetByte(temp_send, (BYTE) m_sLevel, index);
	Setfloat(temp_send, m_fCurX, index);
	Setfloat(temp_send, m_fCurZ, index);
	Setfloat(temp_send, m_fCurY, index);
	SetByte(temp_send, m_byDirection, index);

	if (m_iHP <= 0)
		SetByte(temp_send, 0x00, index);
	else
		SetByte(temp_send, 0x01, index);

	SetByte(temp_send, m_tNpcType, index);
	SetInt(temp_send, m_iSellingGroup, index);
	SetDWORD(temp_send, m_iMaxHP, index);
	SetDWORD(temp_send, m_iHP, index);
	SetByte(temp_send, m_byGateOpen, index);
	SetShort(temp_send, m_sHitRate, index);
	SetByte(temp_send, m_byObjectType, index);
	SetByte(temp_send, m_byTrapNumber, index);
}

// game server에 npc정보를 전부 전송...
void CNpc::SendNpcInfoAll(char* temp_send, int& index, int count)
{
	if (m_bySpecialType == 5
		&& m_byChangeType == 0)
		SetByte(temp_send, 0, index);					// region에 등록하지 말아라
	else
		SetByte(temp_send, 1, index);					// region에 등록		
	SetShort(temp_send, m_sNid + NPC_BAND, index);
	SetShort(temp_send, m_sSid, index);
	SetShort(temp_send, m_sPid, index);
	SetShort(temp_send, m_sSize, index);
	SetInt(temp_send, m_iWeapon_1, index);
	SetInt(temp_send, m_iWeapon_2, index);
	SetShort(temp_send, m_sCurZone, index);
	SetShort(temp_send, m_ZoneIndex, index);
	SetVarString(temp_send, m_strName.c_str(), static_cast<int>(m_strName.length()), index);
	SetByte(temp_send, m_byGroup, index);
	SetByte(temp_send, (BYTE) m_sLevel, index);
	Setfloat(temp_send, m_fCurX, index);
	Setfloat(temp_send, m_fCurZ, index);
	Setfloat(temp_send, m_fCurY, index);
	SetByte(temp_send, m_byDirection, index);
	SetByte(temp_send, m_tNpcType, index);
	SetInt(temp_send, m_iSellingGroup, index);
	SetDWORD(temp_send, m_iMaxHP, index);
	SetDWORD(temp_send, m_iHP, index);
	SetByte(temp_send, m_byGateOpen, index);
	SetShort(temp_send, m_sHitRate, index);
	SetByte(temp_send, m_byObjectType, index);
	SetByte(temp_send, m_byTrapNumber, index);

	//TRACE(_T("monster info all = %d, name=%hs, count=%d \n"), m_sNid+NPC_BAND, m_strName, count);
}

int CNpc::GetDir(float x1, float z1, float x2, float z2)
{
	int nDir;					//	3 4 5
								//	2 8 6
								//	1 0 7

	int nDirCount = 0;

	int x11 = (int) x1 / TILE_SIZE;
	int y11 = (int) z1 / TILE_SIZE;
	int x22 = (int) x2 / TILE_SIZE;
	int y22 = (int) z2 / TILE_SIZE;

	int deltax = x22 - x11;
	int deltay = y22 - y11;

	int fx = ((int) x1 / TILE_SIZE) * TILE_SIZE;
	int fy = ((int) z1 / TILE_SIZE) * TILE_SIZE;

	float add_x = x1 - fx;
	float add_y = z1 - fy;

	if (deltay == 0)
	{
		if (x22 > x11)
			nDir = DIR_RIGHT;
		else
			nDir = DIR_LEFT;

		goto result_value;
	}

	if (deltax == 0)
	{
		if (y22 > y11)
			nDir = DIR_DOWN;
		else
			nDir = DIR_UP;

		goto result_value;
	}
	else
	{
		if (y22 > y11)
		{
			if (x22 > x11)
				nDir = DIR_DOWNRIGHT;		// -> 
			else
				nDir = DIR_DOWNLEFT;		// -> 
		}
		else
		{
			if (x22 > x11)
				nDir = DIR_UPRIGHT;
			else
				nDir = DIR_UPLEFT;
		}
	}

result_value:
	switch (nDir)
	{
		case DIR_DOWN:
			m_fAdd_x = add_x;
			m_fAdd_z = 3;
			break;

		case DIR_DOWNLEFT:
			m_fAdd_x = 1;
			m_fAdd_z = 3;
			break;

		case DIR_LEFT:
			m_fAdd_x = 1;
			m_fAdd_z = add_y;
			break;

		case DIR_UPLEFT:
			m_fAdd_x = 1;
			m_fAdd_z = 1;
			break;

		case DIR_UP:
			m_fAdd_x = add_x;
			m_fAdd_z = 1;
			break;

		case DIR_UPRIGHT:
			m_fAdd_x = 3;
			m_fAdd_z = 1;
			break;

		case DIR_RIGHT:
			m_fAdd_x = 3;
			m_fAdd_z = add_y;
			break;

		case DIR_DOWNRIGHT:
			m_fAdd_x = 3;
			m_fAdd_z = 3;
			break;
	}

	return nDir;
}

inline float CNpc::RandomGenf(float max, float min)
{
	if (max == min)
		return max;

	if (min > max)
		std::swap(min, max);

	int k = rand() % (int) ((max * 100 - min * 100));
	return (float) ((float) (k * 0.01f) + min);
}

//////////////////////////////////////////////////////////////////////
// 인자: 현재 위치, 방향, random각, 이동거리
// 반환값: 최종위치
//////////////////////////////////////////////////////////////////////
__Vector3 CNpc::MyFunc(__Vector3 vCur, __Vector3 vDir, float fYDegree, float fDistance)
{
	float fYRandom = RandomGenf(fYDegree / 2.0f, -fYDegree / 2.0f);

	vDir.Normalize();
	__Matrix44 mtx;
	mtx.Identity();
	__Quaternion qt;

	qt.RotationAxis(0.0f, 1.0f, 0.0f, D3DXToRadian(fYRandom));
	D3DXMatrixRotationQuaternion(&mtx, &qt);

	vDir *= mtx;
	vDir *= fDistance;

	__Vector3 vResult;
	vResult = vCur + vDir;
	return vResult;
}

__Vector3 CNpc::GetDirection(__Vector3 vStart, __Vector3 vEnd)
{
	__Vector3 vDir = vEnd - vStart;
	vDir.Normalize();
	return vDir;
}

// sungyong 2002.05.22
void CNpc::SendAll(CIOCPort* pIOCP, const char* pBuf, int nLength)
{
	if (nLength <= 0
		|| nLength > sizeof(SEND_DATA::pBuf))
		return;

	SEND_DATA* pNewData = nullptr;
	pNewData = new SEND_DATA;
	if (pNewData == nullptr)
		return;

	pNewData->sCurZone = m_sCurZone;
	pNewData->sLength = nLength;
	::CopyMemory(pNewData->pBuf, pBuf, nLength);

	EnterCriticalSection(&(pIOCP->m_critSendData));
	pIOCP->m_SendDataList.push_back(pNewData);
	LeaveCriticalSection(&(pIOCP->m_critSendData));

	PostQueuedCompletionStatus(pIOCP->m_hSendIOCP, 0, 0, nullptr);
}
// ~sungyong 2002.05.22

void CNpc::NpcTrace(std::string_view msg)
{
	if (useNpcTrace)
	{
		spdlog::trace("NPCTrace: {} [serial={} npcId={} npcName={} x={} z={}]",
			msg, m_sNid + NPC_BAND, m_sSid, m_strName, m_fCurX, m_fCurZ);
	}
}

void CNpc::NpcMoveEnd(CIOCPort* pIOCP)
{
	SetUid(m_fCurX, m_fCurZ, m_sNid + NPC_BAND);

	char pBuf[1024] = {};
	int index = 0;
	SetByte(pBuf, MOVE_RESULT, index);
	SetByte(pBuf, SUCCESS, index);
	SetShort(pBuf, m_sNid + NPC_BAND, index);
	Setfloat(pBuf, m_fCurX, index);
	Setfloat(pBuf, m_fCurZ, index);
	Setfloat(pBuf, m_fCurY, index);
	Setfloat(pBuf, 0, index);

	int rx = m_fCurX / VIEW_DIST;
	int rz = m_fCurZ / VIEW_DIST;
	//TRACE(_T("NpcMoveEnd() --> nid = %d, x=%f, y=%f, rx=%d,rz=%d, frame=%d, speed = %d \n"), m_sNid, m_fCurX, m_fCurZ, rx,rz, m_iAniFrameCount, m_sSpeed);
	SendAll(pIOCP, pBuf, index);   // thread 에서 send
}

__Vector3 CNpc::GetVectorPosition(__Vector3 vOrig, __Vector3 vDest, float fDis)
{
	__Vector3 vOff;
	vOff = vDest - vOrig;
	float ttt = vOff.Magnitude();
	vOff.Normalize();
	vOff *= fDis;
	return vOrig + vOff;
}

float CNpc::GetDistance(__Vector3 vOrig, __Vector3 vDest)
{
	__Vector3 vDis = vOrig - vDest;
	return vDis.Magnitude();
}

bool CNpc::GetUserInView()
{
	MAP* pMap = m_pMain->GetMapByIndex(m_ZoneIndex);
	if (pMap == nullptr)
	{
		spdlog::error("Npc::GetUserInView: map not found [zoneIndex={} npcId={} npcName={}]",
			m_strName, m_sSid, m_ZoneIndex);
		return false;
	}

	//if( m_ZoneIndex > 5 || m_ZoneIndex < 0) return false;		// 임시코드 ( 2002.03.24 )
	int max_xx = pMap->m_sizeRegion.cx;
	int max_zz = pMap->m_sizeRegion.cy;
	int min_x = (int) (m_fCurX - NPC_VIEW_RANGE) / VIEW_DIST;
	if (min_x < 0)
		min_x = 0;

	int min_z = (int) (m_fCurZ - NPC_VIEW_RANGE) / VIEW_DIST;
	if (min_z < 0)
		min_z = 0;

	int max_x = (int) (m_fCurX + NPC_VIEW_RANGE) / VIEW_DIST;
	if (max_x >= max_xx)
		max_x = max_xx - 1;

	int max_z = (int) (m_fCurZ + NPC_VIEW_RANGE) / VIEW_DIST;
	if (max_z >= max_zz)
		max_z = max_zz - 1;


	int search_x = max_x - min_x + 1;
	int search_z = max_z - min_z + 1;

	bool bFlag = false;

	for (int i = 0; i < search_x; i++)
	{
		for (int j = 0; j < search_z; j++)
		{
			bFlag = GetUserInViewRange(min_x + i, min_z + j);
			if (bFlag)
				return true;
		}
	}

	return false;
}

bool CNpc::GetUserInViewRange(int x, int z)
{
	MAP* pMap = m_pMain->GetMapByIndex(m_ZoneIndex);
	if (pMap == nullptr)
	{
		spdlog::error("Npc::GetUserInViewRange: map not found [zoneIndex={} npcId={} npcName={}]",
			m_strName, m_sSid, m_ZoneIndex);
		return false;
	}

	if (x < 0
		|| z < 0
		|| x > pMap->GetXRegionMax()
		|| z > pMap->GetZRegionMax())
	{
		spdlog::error("Npc::GetUserInViewRange: out of map bounds [serial={} npcId={} x={} z={}]",
        			m_sNid + NPC_BAND, m_sSid, x, z);
		return false;
	}

	EnterCriticalSection(&g_region_critical);
	CUser* pUser = nullptr;
	__Vector3 vStart, vEnd;
	vStart.Set(m_fCurX, 0, m_fCurZ);
	float fDis = 0.0f;
	int nUserid = 0;

	auto Iter1 = pMap->m_ppRegion[x][z].m_RegionUserArray.begin();
	auto Iter2 = pMap->m_ppRegion[x][z].m_RegionUserArray.end();

	for (; Iter1 != Iter2; Iter1++)
	{
		nUserid = *((*Iter1).second);
		if (nUserid < 0)
			continue;

		pUser = m_pMain->GetUserPtr(nUserid);

		if (pUser == nullptr)
			continue;

		// 가시 거리 계산 
		vEnd.Set(pUser->m_curx, 0, pUser->m_curz);
		fDis = GetDistance(vStart, vEnd);
		if (fDis <= NPC_VIEW_RANGE)
		{
			LeaveCriticalSection(&g_region_critical);
			return true;
		}
	}

	LeaveCriticalSection(&g_region_critical);
	return false;
}

void CNpc::SendAttackSuccess(CIOCPort* pIOCP, BYTE byResult, int tuid, short sDamage, int nHP, BYTE byFlag, short sAttack_type)
{
	int send_index = 0;
	int sid = -1, tid = -1;
	BYTE type, result = 0;
	char buff[256] = {};
	float rx = 0.0f, ry = 0.0f, rz = 0.0f;

	if (byFlag == 0)
	{
		type = 0x02;
		sid = m_sNid + NPC_BAND;
		tid = tuid;

		SetByte(buff, AG_ATTACK_RESULT, send_index);
		SetByte(buff, type, send_index);
		SetByte(buff, byResult, send_index);
		SetShort(buff, sid, send_index);
		SetShort(buff, tid, send_index);
		SetShort(buff, sDamage, send_index);
		SetDWORD(buff, nHP, send_index);
		SetByte(buff, sAttack_type, send_index);
	}
	else
	{
		type = 0x01;
		sid = tuid;
		tid = m_sNid + NPC_BAND;

		SetByte(buff, AG_ATTACK_RESULT, send_index);
		SetByte(buff, type, send_index);
		SetByte(buff, byResult, send_index);
		SetShort(buff, sid, send_index);
		SetShort(buff, tid, send_index);
		SetShort(buff, sDamage, send_index);
		SetDWORD(buff, nHP, send_index);
		SetByte(buff, sAttack_type, send_index);
	}

	//TRACE(_T("Npc - SendAttackSuccess() : [sid=%d, tid=%d, result=%d], damage=%d, hp = %d\n"), sid, tid, byResult, sDamage, sHP);
	//SetShort( buff, sMaxHP, send_index );

	SendAll(pIOCP, buff, send_index);   // thread 에서 send
}

__Vector3 CNpc::CalcAdaptivePosition(__Vector3 vPosOrig, __Vector3 vPosDest, float fAttackDistance)
{
	__Vector3 vTemp, vReturn;
	vTemp = vPosOrig - vPosDest;
	vTemp.Normalize();
	vTemp *= fAttackDistance;
	vReturn = vPosDest + vTemp;
	return vReturn;
}

//	현재 몹을 기준으로 한 화면 범위안에 있는지 판단
void CNpc::IsUserInSight()
{
	CUser* pUser = nullptr;
	// Npc와 User와의 거리가 50미터 안에 있는 사람에게만,, 경험치를 준다..
	int iSearchRange = NPC_EXP_RANGE;

	int i, j;
	__Vector3 vStart, vEnd;
	float fDis = 0.0f;

	vStart.Set(m_fCurX, m_fCurY, m_fCurZ);

	for (j = 0; j < NPC_HAVE_USER_LIST; j++)
		m_DamagedUserList[j].bIs = false;

	for (i = 0; i < NPC_HAVE_USER_LIST; i++)
	{
		pUser = m_pMain->GetUserPtr(m_DamagedUserList[i].iUid);
		if (pUser == nullptr)
			continue;

		vEnd.Set(pUser->m_curx, pUser->m_cury, pUser->m_curz);
		fDis = GetDistance(vStart, vEnd);

		if ((int) fDis <= iSearchRange)
		{
			// 갖고있는 리스트상의 유저와 같다면
			if (m_DamagedUserList[i].iUid == pUser->m_iUserId)
			{
				// 최종 ID를 비교해서 동일하면	
				if (_stricmp(m_DamagedUserList[i].strUserID, pUser->m_strUserID) == 0)
				{
					// 이때서야 존재한다는 표시를 한다
					m_DamagedUserList[i].bIs = true;
				}
			}
		}
	}
}

BYTE CNpc::GetHitRate(float rate)
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

bool CNpc::IsLevelCheck(int iLevel)
{
	// 몬스터의 레벨보다 낮으면,,,  바로 공격
	if (iLevel <= m_sLevel)
		return false;

	int compLevel = iLevel - m_sLevel;

	// 레벨을 비교해서 8미만이면 바로 공격
	if (compLevel < 8)
		return false;

	return true;
}

bool CNpc::IsHPCheck(int iHP)
{
	if (m_iHP < (m_iMaxHP * 0.2))
	{
//		if(iHP > 1.5*m_iHP)
		return true;
	}

	return false;
}

// 패스 파인드를 할것인지를 체크하는 루틴..
bool CNpc::IsPathFindCheck(float fDistance)
{
	int nX = 0, nZ = 0;
	__Vector3 vStart, vEnd, vDis, vOldDis;
	float fDis = 0.0f;
	vStart.Set(m_fStartPoint_X, 0, m_fStartPoint_Y);
	vEnd.Set(m_fEndPoint_X, 0, m_fEndPoint_Y);
	vDis.Set(m_fStartPoint_X, 0, m_fStartPoint_Y);
	int count = 0;
	int nError = 0;

	MAP* pMap = m_pMain->GetMapByIndex(m_ZoneIndex);
	if (pMap == nullptr)
	{
		spdlog::error("Npc::IsPathFindCheck: map not found [zoneIndex={} npcId={} npcName={}]",
			m_strName, m_sSid, m_ZoneIndex);
		return false;
	}

	nX = (int) (vStart.x / TILE_SIZE);
	nZ = (int) (vStart.z / TILE_SIZE);
	if (pMap->IsMovable(nX, nZ))
	{
		nError = -1;
		return false;
	}

	nX = (int) (vEnd.x / TILE_SIZE);
	nZ = (int) (vEnd.z / TILE_SIZE);
	if (pMap->IsMovable(nX, nZ))
	{
		nError = -1;
		return false;
	}

	while (1)
	{
		vOldDis.Set(vDis.x, 0, vDis.z);
		vDis = GetVectorPosition(vDis, vEnd, fDistance);
		fDis = GetDistance(vOldDis, vEnd);

		if (fDis > NPC_MAX_MOVE_RANGE)
		{
			nError = -1;
			break;
		}

		if (fDis <= fDistance)
		{
			nX = (int) (vDis.x / TILE_SIZE);
			nZ = (int) (vDis.z / TILE_SIZE);
			if (pMap->IsMovable(nX, nZ))
			{
				nError = -1;
				break;
			}

			if (count >= MAX_PATH_LINE)
			{
				nError = -1;
				break;
			}

			m_pPoint[count].fXPos = vEnd.x;
			m_pPoint[count].fZPos = vEnd.z;
			count++;
			break;
		}
		else
		{
			nX = (int) (vDis.x / TILE_SIZE);
			nZ = (int) (vDis.z / TILE_SIZE);
			if (pMap->IsMovable(nX, nZ))
			{
				nError = -1;
				break;
			}

			if (count >= MAX_PATH_LINE)
			{
				nError = -1;
				break;
			}

			m_pPoint[count].fXPos = vDis.x;
			m_pPoint[count].fZPos = vDis.z;
		}

		count++;
	}

	m_iAniFrameIndex = count;

	if (nError == -1)
		return false;

	return true;
}

// 패스 파인드를 하지 않고 공격대상으로 가는 루틴..
void CNpc::IsNoPathFind(float fDistance)
{
	ClearPathFindData();
	m_bPathFlag = true;

	int nX = 0, nZ = 0;
	__Vector3 vStart, vEnd, vDis, vOldDis;
	float fDis = 0.0f;
	vStart.Set(m_fStartPoint_X, 0, m_fStartPoint_Y);
	vEnd.Set(m_fEndPoint_X, 0, m_fEndPoint_Y);
	vDis.Set(m_fStartPoint_X, 0, m_fStartPoint_Y);
	int count = 0;
	int nError = 0;

	fDis = GetDistance(vStart, vEnd);

	// 100미터 보다 넓으면 스탠딩상태로..
	if (fDis > NPC_MAX_MOVE_RANGE)
	{
		ClearPathFindData();
		spdlog::error("Npc::IsNoPathFind: tried to move further than max move distance [serial={} npcId={} npcName={} distance={}]",
			m_sNid + NPC_BAND, m_sSid, m_strName, fDis);
		return;
	}

	MAP* pMap = m_pMain->m_ZoneArray[m_ZoneIndex];
	if (pMap == nullptr)
	{
		ClearPathFindData();
		spdlog::error("Npc::IsNoPathFind: map not found [zoneIndex={} npcId={} npcName={}]",
			m_strName, m_sSid, m_ZoneIndex);
		return;
	}

	while (1)
	{
		vOldDis.Set(vDis.x, 0, vDis.z);
		vDis = GetVectorPosition(vDis, vEnd, fDistance);
		fDis = GetDistance(vOldDis, vEnd);

		if (fDis <= fDistance)
		{
			nX = (int) (vDis.x / TILE_SIZE);
			nZ = (int) (vDis.z / TILE_SIZE);
			if (count < 0
				|| count >= MAX_PATH_LINE)
			{
				ClearPathFindData();
				spdlog::error("Npc::IsNoPathFind: invalid pathCount [serial={} npcId={} npcName={} pathCount={}]",
					m_sNid + NPC_BAND, m_sSid, m_strName, count);
				return;
			}

			m_pPoint[count].fXPos = vEnd.x;
			m_pPoint[count].fZPos = vEnd.z;
			count++;
			break;
		}
		else
		{
			if (count < 0
				|| count >= MAX_PATH_LINE)
			{
				ClearPathFindData();
				spdlog::error("Npc::IsNoPathFind: invalid pathCount [serial={} npcId={} npcName={} pathCount={}]",
					m_sNid + NPC_BAND, m_sSid, m_strName, count);
				return;
			}

			nX = (int) (vDis.x / TILE_SIZE);
			nZ = (int) (vDis.z / TILE_SIZE);
			m_pPoint[count].fXPos = vDis.x;
			m_pPoint[count].fZPos = vDis.z;
		}

		count++;
	}

	if (count <= 0
		|| count >= MAX_PATH_LINE)
	{
		ClearPathFindData();
		spdlog::error("Npc::IsNoPathFind: invalid pathCount [serial={} npcId={} npcName={} pathCount={}]",
							m_sNid + NPC_BAND, m_sSid, m_strName, count);
		return;
	}

	m_iAniFrameIndex = count;
}

//	NPC 가 가진 아이템을 떨군다.
void CNpc::GiveNpcHaveItem(CIOCPort* pIOCP)
{
	char pBuf[1024] = {};
	char logfile[256] = {};
	int index = 0;
	int temp = 0;
	int iPer = 0, iMakeItemCode = 0, iMoney = 0;
	int iRandom;
	int nCount = 1, i = 0;

/*	if( m_byMoneyType == 1 )	{
		SetByte(pBuf, AG_NPC_EVENT_ITEM, index);
		SetShort(pBuf, m_sMaxDamageUserid, index);
		SetShort(pBuf, m_sNid+NPC_BAND, index);
		SetDWORD(pBuf, TYPE_MONEY_SID, index);
		SetDWORD(pBuf, m_iMoney, index);
		return;
	}	*/

	iRandom = myrand(70, 100);
	iMoney = m_iMoney * iRandom / 100;
	//m_iMoney, m_iItem;
	_NpcGiveItem m_GiveItemList[NPC_HAVE_ITEM_LIST];			// Npc의 ItemList
	if (iMoney <= 0)
	{
		nCount = 0;
	}
	else
	{
		m_GiveItemList[0].sSid = TYPE_MONEY_SID;
		if (iMoney > 32767)
		{
			iMoney = 32000;						// sungyong : short형이기 때문에,,
			m_GiveItemList[0].count = iMoney;
		}
		else
		{
			m_GiveItemList[0].count = iMoney;
		}
	}

	for (i = 0; i < m_pMain->m_NpcItem.m_nRow; i++)
	{
		if (m_pMain->m_NpcItem.m_ppItem[i][0] != m_iItem)
			continue;

		for (int j = 1; j < m_pMain->m_NpcItem.m_nField; j += 2)
		{
			if (m_pMain->m_NpcItem.m_ppItem[i][j] == 0)
				continue;

			iRandom = myrand(1, 10000);
			iPer = m_pMain->m_NpcItem.m_ppItem[i][j + 1];
			if (iPer == 0)
				continue;

			// 우선 기본테이블를 참조하기위해	
			if (iRandom <= iPer)
			{
				// 아이템 생성..
				if (j == 1)
				{
					iMakeItemCode = ItemProdution(m_pMain->m_NpcItem.m_ppItem[i][j]);
					if (iMakeItemCode == 0)	continue;

					m_GiveItemList[nCount].sSid = iMakeItemCode;
					m_GiveItemList[nCount].count = 1;
				}
				else
				{
					m_GiveItemList[nCount].sSid = m_pMain->m_NpcItem.m_ppItem[i][j];

					// 화살이라면
					if (COMPARE(m_GiveItemList[nCount].sSid, ARROW_MIN, ARROW_MAX))
						m_GiveItemList[nCount].count = 20;
					else
						m_GiveItemList[nCount].count = 1;
				}
				nCount++;
			}
		}
	}

	if (m_sMaxDamageUserid < 0
		|| m_sMaxDamageUserid > MAX_USER)
	{
		//TRACE(_T("### Npc-GiveNpcHaveItem() User Array Fail : [nid - %d,%hs], userid=%d ###\n"), m_sNid+NPC_BAND, m_strName, m_sMaxDamageUserid);
		return;
	}

	SetByte(pBuf, AG_NPC_GIVE_ITEM, index);
	SetShort(pBuf, m_sMaxDamageUserid, index);
	SetShort(pBuf, m_sNid + NPC_BAND, index);
	SetShort(pBuf, m_sCurZone, index);
	SetShort(pBuf, (short) m_iRegion_X, index);
	SetShort(pBuf, (short) m_iRegion_Z, index);
	Setfloat(pBuf, m_fCurX, index);
	Setfloat(pBuf, m_fCurZ, index);
	Setfloat(pBuf, m_fCurY, index);
	SetByte(pBuf, nCount, index);
	for (i = 0; i < nCount; i++)
	{
		SetInt(pBuf, m_GiveItemList[i].sSid, index);
		SetShort(pBuf, m_GiveItemList[i].count, index);

		if (m_GiveItemList[i].sSid != TYPE_MONEY_SID)
			spdlog::get(logger::AIServerItem)->info(m_GiveItemList[i].sSid);

		//TRACE(_T("Npc-GiveNpcHaveItem() : [nid - %d,%hs,  giveme=%d, count=%d, num=%d], list=%d, count=%d\n"), m_sNid+NPC_BAND, m_strName, m_sMaxDamageUserid, nCount, i, m_GiveItemList[i].sSid, m_GiveItemList[i].count);
	}

	SendAll(pIOCP, pBuf, index);   // thread 에서 send
}


void CNpc::Yaw2D(float fDirX, float fDirZ, float& fYawResult)
{
	if (fDirX >= 0.0f)
	{
		if (fDirZ >= 0.0f)
			fYawResult = (float) (asin(fDirX));
		else
			fYawResult = D3DXToRadian(90.0f) + (float) (acos(fDirX));
	}
	else
	{
		if (fDirZ >= 0.0f)
			fYawResult = D3DXToRadian(270.0f) + (float) (acos(-fDirX));
		else
			fYawResult = D3DXToRadian(180.0f) + (float) (asin(-fDirX));
	}
}

__Vector3 CNpc::ComputeDestPos(__Vector3 vCur, float fDegree, float fDegreeOffset, float fDistance)
{
	__Vector3 vReturn, vDir;
	vDir.Set(0.0f, 0.0f, 1.0f);
	__Matrix44 mtxRot;
	mtxRot.RotationY(D3DXToRadian(fDegree + fDegreeOffset));
	vDir *= mtxRot;
	vDir *= fDistance;
	vReturn = (vCur + vDir);
	return vReturn;
}

int	CNpc::GetPartyDamage(int iNumber)
{
	int i = 0;
	int nDamage = 0;
	CUser* pUser = nullptr;

	// 일단 리스트를 검색한다.
	for (i = 0; i < NPC_HAVE_USER_LIST; i++)
	{
		if (m_DamagedUserList[i].iUid < 0
			|| m_DamagedUserList[i].nDamage <= 0)
			continue;

		if (m_DamagedUserList[i].bIs)
			pUser = m_pMain->GetUserPtr(m_DamagedUserList[i].iUid);

		if (pUser == nullptr)
			continue;

		if (pUser->m_sPartyNumber != iNumber)
			continue;

		nDamage += m_DamagedUserList[i].nDamage;
	}

	return nDamage;
}

void CNpc::NpcTypeParser()
{
	// 선공인지 후공인지를 결정한다
	switch (m_byActType)
	{
		case 1:
			m_tNpcAttType = m_tNpcOldAttType = 0;
			break;

		case 2:
			m_tNpcAttType = m_tNpcOldAttType = 0;
			m_byNpcEndAttType = 0;
			break;

		case 3:
			m_tNpcGroupType = 1;
			m_tNpcAttType = m_tNpcOldAttType = 0;
			break;

		case 4:
			m_tNpcGroupType = 1;
			m_tNpcAttType = m_tNpcOldAttType = 0;
			m_byNpcEndAttType = 0;
			break;

		case 6:
			m_byNpcEndAttType = 0;
			break;

		case 5:
		case 7:
			m_tNpcAttType = m_tNpcOldAttType = 1;
			break;

		default:
			m_tNpcAttType = m_tNpcOldAttType = 1;
	}
}

void CNpc::HpChange(CIOCPort* pIOCP)
{
	m_fHPChangeTime = TimeGet();

	//if(m_NpcState == NPC_FIGHTING || m_NpcState == NPC_DEAD)	return;
	if (m_NpcState == NPC_DEAD)
		return;

	// 죽기직전일때는 회복 안됨...
	if (m_iHP < 1)
		return;

	// HP가 만빵이기 때문에.. 
	if (m_iHP == m_iMaxHP) 
		return;

	if (pIOCP == nullptr)
		return;

	//int amount =  (int)(m_sLevel*(1+m_sLevel/60.0) + 1) ;
	int amount = (int) (m_iMaxHP / 20);

	char buff[256] = {};
	int send_index = 0;

	m_iHP += amount;
	if (m_iHP < 0)
		m_iHP = 0;
	else if (m_iHP > m_iMaxHP)
		m_iHP = m_iMaxHP;

	SetByte(buff, AG_USER_SET_HP, send_index);
	SetShort(buff, m_sNid + NPC_BAND, send_index);
	SetDWORD(buff, m_iHP, send_index);
	SetDWORD(buff, m_iMaxHP, send_index);

	SendAll(pIOCP, buff, send_index);   // thread 에서 send
}

bool CNpc::IsInExpRange(CUser* pUser)
{
	// Npc와 User와의 거리가 50미터 안에 있는 사람에게만,, 경험치를 준다..
	int iSearchRange = NPC_EXP_RANGE;
	__Vector3 vStart, vEnd;
	float fDis = 0.0f;

	vStart.Set(m_fCurX, m_fCurY, m_fCurZ);
	vEnd.Set(pUser->m_curx, pUser->m_cury, pUser->m_curz);
	fDis = GetDistance(vStart, vEnd);
	if ((int) fDis <= iSearchRange)
	{
		if (m_sCurZone == pUser->m_curZone)
			return true;
	}

	return false;
}

bool CNpc::CheckFindEnemy()
{
	// 경비병은 몬스터도 공격하므로 제외
	if (m_tNpcType == NPC_GUARD
		|| m_tNpcType == NPC_PATROL_GUARD
		|| m_tNpcType == NPC_STORE_GUARD
		// || m_tNpcType == NPCTYPE_MONSTER
		)
		return true;

	MAP* pMap = m_pMain->GetMapByIndex(m_ZoneIndex);
	if (pMap == nullptr)
	{
		spdlog::error("Npc::CheckFindEnemy: map not found [zoneIndex={} npcId={} npcName={}]",
			m_strName, m_sSid, m_ZoneIndex);
		return false;
	}

	if (m_iRegion_X > pMap->GetXRegionMax()
		|| m_iRegion_Z > pMap->GetZRegionMax()
		|| m_iRegion_X < 0
		|| m_iRegion_Z < 0)
	{
		spdlog::error("Npc::CheckFindEnemy: out of region bounds [serial={} npcId={} x={} z={}]",
			m_sNid + NPC_BAND, m_sSid, m_iRegion_X, m_iRegion_Z);
		return false;
	}

	if (pMap->m_ppRegion[m_iRegion_X][m_iRegion_Z].m_byMoving == 1)
		return true;

	return false;
}

void CNpc::MSpChange(int type, int amount)
{
	if (type == 2)
	{
		m_sMP += amount;
		if (m_sMP < 0)
			m_sMP = 0;
		else if (m_sMP > m_sMaxMP)
			m_sMP = m_sMaxMP;
	}
	// monster는 SP가 없음..
	else if (type == 3)
	{
	}
}

void CNpc::ItemWoreOut(int type, int damage)
{
	// 몬스터의 방어구 내구력 감소로직..
}

// 아이템 제작
int	CNpc::ItemProdution(int item_number)
{
	int iItemNumber = 0, iRandom = 0, i = 0, iItemGrade = 0, iItemLevel = 0;
	int iDefault = 0, iItemCode = 0, iItemKey = 0, iRand2 = 0, iRand3 = 0, iRand4 = 0, iRand5 = 0;
	int iTemp1 = 0, iTemp2 = 0, iTemp3 = 0;

	iRandom = myrand(1, 10000);
	//iRandom = myrand(1, 4000);

	//TRACE(_T("ItemProdution : nid=%d, sid=%d, name=%hs, item_number = %d\n"), m_sNid+NPC_BAND, m_sSid, m_strName, item_number);
	iItemGrade = GetItemGrade(item_number);
	//TRACE(_T("ItemProdution : GetItemGrade() = %d\n"), iItemGrade);
	if (iItemGrade == 0)
		return 0;

	iItemLevel = m_sLevel / 5;

	// 무기구 아이템
	if (COMPARE(iRandom, 1, 4001))
	{
		iDefault = 100000000;
		// 무기의 종류를 결정(단검, 검, 도끼,,,,)
		iRandom = myrand(1, 10000);	
		if (COMPARE(iRandom, 1, 701))
			iRand2 = 10000000;
		else if (COMPARE(iRandom, 701, 1401))
			iRand2 = 20000000;
		else if (COMPARE(iRandom, 1401, 2101))
			iRand2 = 30000000;
		else if (COMPARE(iRandom, 2101, 2801))
			iRand2 = 40000000;
		else if (COMPARE(iRandom, 2801, 3501))
			iRand2 = 50000000;
		else if (COMPARE(iRandom, 3501, 5501))
			iRand2 = 60000000;
		else if (COMPARE(iRandom, 5501, 6501))
			iRand2 = 70000000;
		else if (COMPARE(iRandom, 6501, 8501))
			iRand2 = 80000000;
		else if (COMPARE(iRandom, 8501, 10001))
			iRand2 = 90000000;

		iTemp1 = GetWeaponItemCodeNumber(1);
		//TRACE(_T("ItemProdution : GetWeaponItemCodeNumber() = %d, iRand2=%d\n"), iTemp1, iRand2);
		if (iTemp1 == 0)
			return 0;

		// 루팅분포표 참조
		iItemCode = iTemp1 * 100000;

		// 종족(엘모, 카루스)
		iRand3 = myrand(1, 10000);
		if (COMPARE(iRand3, 1, 5000))
			iRand3 = 10000;
		else
			iRand3 = 50000;

		// 한손, 양손무기인지를 결정
		iRand4 = myrand(1, 10000);
		if (COMPARE(iRand4, 1, 5000))
			iRand4 = 0;
		else
			iRand4 = 5000000;

		// 레이매직표 적용
		iRandom = GetItemCodeNumber(iItemLevel, 1);
		//TRACE(_T("ItemProdution : GetItemCodeNumber() = %d, iRand2=%d, iRand3=%d, iRand4=%d\n"), iRandom, iRand2, iRand3, iRand4);

		// 잘못된 아이템 생성실패
		if (iRandom == -1)
			return 0;

		iRand5 = iRandom * 10;
		iItemNumber = iDefault + iItemCode + iRand2 + iRand3 + iRand4 + iRand5 + iItemGrade;

		//TRACE(_T("ItemProdution : Weapon Success item_number = %d, default=%d, itemcode=%d, iRand2=%d, iRand3=%d, iRand4=%d, iRand5, iItemGrade=%d\n"), iItemNumber, iDefault, iItemCode, iRand2, iRand3, iRand4, iRand5, iItemGrade);
	}
	// 방어구 아이템
	else if (COMPARE(iRandom, 4001, 8001))
	{
		iDefault = 200000000;

		iTemp1 = GetWeaponItemCodeNumber(2);
		//TRACE(_T("ItemProdution : GetWeaponItemCodeNumber() = %d\n"), iTemp1 );
		if (iTemp1 == 0)
			return 0;

		// 루팅분포표 참조
		iItemCode = iTemp1 * 1000000;

		// 종족
		if (m_byMaxDamagedNation == KARUS_MAN)
		{
			// 직업의 갑옷을 결정		
			iRandom = myrand(0, 10000);
			if (COMPARE(iRandom, 0, 2000))
			{
				iRand2 = 0;
				iRand3 = 10000;							// 전사갑옷은 아크투아렉만 가지도록
			}
			else if (COMPARE(iRandom, 2000, 4000))
			{
				iRand2 = 40000000;
				iRand3 = 20000;							// 로그갑옷은 투아렉만 가지도록
			}
			else if (COMPARE(iRandom, 4000, 6000))
			{
				iRand2 = 60000000;
				iRand3 = 30000;							// 마법사갑옷은 링클 투아렉만 가지도록
			}
			else if (COMPARE(iRandom, 6000, 10001))
			{
				iRand2 = 80000000;
				iRandom = myrand(0, 10000);
				if (COMPARE(iRandom, 0, 5000))
					iRand3 = 20000;						// 사제갑옷은 투아렉
				else									
					iRand3 = 40000;						// 사제갑옷은 퓨리투아렉
			}
		}
		else if (m_byMaxDamagedNation == ELMORAD_MAN)
		{
			// 직업의 갑옷을 결정
			iRandom = myrand(0, 10000);
			if (COMPARE(iRandom, 0, 3300))
			{
				iRand2 = 0;

				// 전사갑옷은 모든 종족이 가짐
				iItemKey = myrand(0, 10000);
				if (COMPARE(iItemKey, 0, 3333))
					iRand3 = 110000;
				else if (COMPARE(iItemKey, 3333, 6666))
					iRand3 = 120000;
				else if (COMPARE(iItemKey, 6666, 10001))
					iRand3 = 130000;
			}
			else if (COMPARE(iRandom, 3300, 5600))
			{
				iRand2 = 40000000;

				// 로그갑옷은 남자와 여자만 가짐
				iItemKey = myrand(0, 10000);
				if (COMPARE(iItemKey, 0, 5000))
					iRand3 = 120000;
				else
					iRand3 = 130000;
			}
			else if (COMPARE(iRandom, 5600, 7800))
			{
				iRand2 = 60000000;

				// 마법사갑옷은 남자와 여자만 가짐
				iItemKey = myrand(0, 10000);
				if (COMPARE(iItemKey, 0, 5000))
					iRand3 = 120000;
				else
					iRand3 = 130000;
			}
			else if (COMPARE(iRandom, 7800, 10001))
			{
				iRand2 = 80000000;

				// 사제갑옷은 남자와 여자만 가짐
				iItemKey = myrand(0, 10000);
				if (COMPARE(iItemKey, 0, 5000))
					iRand3 = 120000;
				else
					iRand3 = 130000;
			}

		}

		// 몸의 부위 아이템 결정
		iTemp2 = myrand(0, 10000);
		if (COMPARE(iTemp2, 0, 2000))
			iRand4 = 1000;
		else if (COMPARE(iTemp2, 2000, 4000))
			iRand4 = 2000;
		else if (COMPARE(iTemp2, 4000, 6000))
			iRand4 = 3000;
		else if (COMPARE(iTemp2, 6000, 8000))
			iRand4 = 4000;
		else if (COMPARE(iTemp2, 8000, 10001))
			iRand4 = 5000;

		// 레이매직표 적용
		iRandom = GetItemCodeNumber(iItemLevel, 2);

		// 잘못된 아이템 생성실패
		if (iRandom == -1)
			return 0;

		iRand5 = iRandom * 10;

		// iItemGrade : 아이템 등급생성표 적용
		iItemNumber = iDefault + iRand2 + iItemCode + iRand3 + iRand4 + iRand5 + iItemGrade;
		//TRACE(_T("ItemProdution : Defensive Success item_number = %d, default=%d, iRand2=%d, itemcode=%d, iRand3=%d, iRand4=%d, iRand5, iItemGrade=%d\n"), iItemNumber, iDefault, iRand2, iItemCode, iRand3, iRand4, iRand5, iItemGrade);
	}
	// 악세사리 아이템
	else if (COMPARE(iRandom, 8001, 10001))
	{
		iDefault = 300000000;

		// 악세사리 종류결정(귀고리, 목걸이, 반지, 벨트)
		iRandom = myrand(0, 10000);
		if (COMPARE(iRandom, 0, 2500))
			iRand2 = 10000000;
		else if (COMPARE(iRandom, 2500, 5000))
			iRand2 = 20000000;
		else if (COMPARE(iRandom, 5000, 7500))
			iRand2 = 30000000;
		else if (COMPARE(iRandom, 7500, 10001))
			iRand2 = 40000000;

		// 종족(엘모라드, 카루스)
		iRand3 = myrand(1, 10000);
		if (COMPARE(iRand3, 1, 5000))
			iRand3 = 110000;
		else
			iRand3 = 150000;

		// 레이매직표 적용
		iRandom = GetItemCodeNumber(iItemLevel, 3);
		//TRACE(_T("ItemProdution : GetItemCodeNumber() = %d\n"), iRandom);

		// 잘못된 아이템 생성실패
		if (iRandom == -1)
			return 0;

		iRand4 = iRandom * 10;
		iItemNumber = iDefault + iRand2 + iRand3 + iRand4 + iItemGrade;
		//TRACE(_T("ItemProdution : Accessary Success item_number = %d, default=%d, iRand2=%d, iRand3=%d, iRand4=%d, iItemGrade=%d\n"), iItemNumber, iDefault, iRand2, iRand3, iRand4, iItemGrade);
	}

	return iItemNumber;
}

int CNpc::GetItemGrade(int item_grade)
{
	model::MakeItemGradeCode* pItemData = m_pMain->m_MakeGradeItemArray.GetData(item_grade);
	if (pItemData == nullptr)
		return 0;

	int iRandom = myrand(1, 1000);

	int iPercent = 0;
	for (int i = 0; i < MAX_ITEM_GRADECODE_GRADES; i++)
	{
		int iGrade = pItemData->Grade[i];
		if (iGrade == 0)
			continue;

		if (COMPARE(iRandom, iPercent, iPercent + iGrade))
			return i + 1;

		iPercent += iGrade;
	}

	return 0;
}

int CNpc::GetWeaponItemCodeNumber(int item_type)
{
	int iPercent = 0, iItem_level = 0;
	model::MakeWeapon* pItemData = nullptr;

	// 무기구
	if (item_type == 1)
	{
		iItem_level = m_sLevel / 10;
		pItemData = m_pMain->m_MakeWeaponTableMap.GetData(iItem_level);
	}
	// 방어구
	else if (item_type == 2)
	{
		iItem_level = m_sLevel / 10;
		pItemData = m_pMain->m_MakeDefensiveTableMap.GetData(iItem_level);
	}

	if (pItemData == nullptr)
		return 0;

	int iRandom = myrand(0, 1000);

	for (int i = 0; i < MAX_MAXWEAPON_CLASSES; i++)
	{
		if (pItemData->Class[i] == 0)
			continue;

		if (COMPARE(iRandom, iPercent, iPercent + pItemData->Class[i]))
			return i + 1;

		iPercent += pItemData->Class[i];
	}

	return 0;
}

int CNpc::GetItemCodeNumber(int level, int item_type)
{
	int iItemCode = 0, iItemType = 0, iPercent = 0;
	int iItemPercent[3];

	int iRandom = myrand(0, 1000);
	model::MakeItemRareCode* pItemData = m_pMain->m_MakeItemRareCodeTableMap.GetData(level);
	if (pItemData == nullptr)
		return -1;

	iItemPercent[0] = pItemData->RareItem;
	iItemPercent[1] = pItemData->MagicItem;
	iItemPercent[2] = pItemData->GeneralItem;

	for (int i = 0; i < 3; i++)
	{
		if (COMPARE(iRandom, iPercent, iPercent + iItemPercent[i]))
		{
			iItemType = i + 1;
			break;
		}

		iPercent += iItemPercent[i];
	}

	switch (iItemType)
	{
		// 잘못된 아이템
		case 0:
			iItemCode = 0;
			break;

		// lare item
		case 1:
			// 무기구
			if (item_type == 1)
				iItemCode = myrand(16, 24);
			// 방어구
			else if (item_type == 2)
				iItemCode = myrand(12, 24);
			// 악세사리
			else if (item_type == 3)
				iItemCode = myrand(0, 10);
			break;

		// magic item
		case 2:
			// 무기구
			if (item_type == 1)
				iItemCode = myrand(6, 15);
			// 방어구
			else if (item_type == 2)
				iItemCode = myrand(6, 11);
			// 악세사리
			else if (item_type == 3)
				iItemCode = myrand(0, 10);
			break;

		// general item
		case 3:
			// 무기구
			if (item_type == 1)
				iItemCode = 5;
			// 방어구
			else if (item_type == 2)
				iItemCode = 5;
			// 악세사리
			else if (item_type == 3)
				iItemCode = myrand(0, 10);
			break;
	}

	return iItemCode;
}

void CNpc::DurationMagic_4(CIOCPort* pIOCP, float currenttime)
{
	int send_index = 0, buff_type = 0;
	char send_buff[128] = {};

	// Dungeon Work : 던젼몬스터의 경우 해당 대장몬스터가 죽은경우 나의 상태를 죽은 상태로....
	MAP* pMap = m_pMain->GetMapByIndex(m_ZoneIndex);
	if (pMap == nullptr)
	{
		spdlog::error("Npc::DurationMagic_4: map not found [zoneIndex={} npcId={} npcName={}]",
			m_strName, m_sSid, m_ZoneIndex);
		return;
	}

	if (m_byDungeonFamily > 0)
	{
		CRoomEvent* pRoom = nullptr;
		//if( m_byDungeonFamily < 0 || m_byDungeonFamily >= MAX_DUNGEON_BOSS_MONSTER )	{
		if (m_byDungeonFamily < 0
			|| m_byDungeonFamily > pMap->m_arRoomEventArray.GetSize() + 1)
		{
			spdlog::error("Npc::DurationMagic_4: dungeonFamily out of range [serial={} npcId={} npcName={} dungeonFamily={}]",
				m_sNid + NPC_BAND, m_sSid, m_strName, m_byDungeonFamily);
			//return;
		}
		else
		{
			pRoom = pMap->m_arRoomEventArray.GetData(m_byDungeonFamily);
			if (pRoom == nullptr)
			{
				spdlog::error("Npc::DurationMagic_4: RoomEvent not found for dungeonFamily [serial={} npcId={} npcName={} dungeonFamily={}]",
					m_sNid + NPC_BAND, m_sSid, m_strName, m_byDungeonFamily);
			}
			else
			{
				//if( pMap->m_arDungeonBossMonster[m_byDungeonFamily] == 0 )	{	// 대장 몬스터가 죽은 경우

				// 방이 클리어 된경우
				if (pRoom->m_byStatus == 3)
				{
					if (m_NpcState != NPC_DEAD)
					{
						if (m_byRegenType == 0)
						{
							m_byRegenType = 2;      // 리젠이 되지 않도록,,
							Dead(pIOCP, 1);
							return;
						}
					}
					//return;
				}
			}
		}
	}

	for (int i = 0; i < MAX_MAGIC_TYPE4; i++)
	{
		if (m_MagicType4[i].sDurationTime)
		{
			if (currenttime > (m_MagicType4[i].fStartTime + m_MagicType4[i].sDurationTime))
			{
				m_MagicType4[i].sDurationTime = 0;
				m_MagicType4[i].fStartTime = 0.0f;
				m_MagicType4[i].byAmount = 0;
				buff_type = i + 1;

				// 속도 관련... 능력치..
				if (i == 5)
				{
					m_fSpeed_1 = m_fOldSpeed_1;
					m_fSpeed_2 = m_fOldSpeed_2;
				}
			}
		}
	}
/*
	if (buff_type)
	{
		SetByte( send_buff, AG_MAGIC_ATTACK_RESULT, send_index );
		SetByte( send_buff, MAGIC_TYPE4_END, send_index );
		SetByte( send_buff, buff_type, send_index );
		SendAll(pIOCP, send_buff, send_index );
	}	*/
}

// 변화되는 몬스터의 정보를 바꾸어준다...
void CNpc::ChangeMonsterInfo(int iChangeType)
{
	// sungyong test
	//m_sChangeSid = 500;		m_byChangeType = 2;

	// 변하지 않는 몬스터
	if (m_sChangeSid == 0
		|| m_byChangeType == 0)
		return;

	// 죽은 상태
	if (m_NpcState != NPC_DEAD)
		return;

	model::Npc* pNpcTable = nullptr;
	if (m_byInitMoveType >= 0
		&& m_byInitMoveType < 100)
	{
		// 다른 몬스터로 변환..
		if (iChangeType == 1)
			pNpcTable = m_pMain->m_MonTableMap.GetData(m_sChangeSid);
		// 원래의 몬스터로 변환..
		else if (iChangeType == 2)
			pNpcTable = m_pMain->m_MonTableMap.GetData(m_sSid);

		if (pNpcTable == nullptr)
		{
			spdlog::error("Npc::ChangeMonsterInfo: changeNpcId not found [serial={} npcId={} npcName={} changeNpcId={}]",
				m_sNid + NPC_BAND, m_sSid, m_strName, m_sChangeSid);
		}
	}
	else if (m_byInitMoveType >= 100)
	{
		// 다른 몬스터로 변환..
		if (iChangeType == 1)
			pNpcTable = m_pMain->m_NpcTableMap.GetData(m_sChangeSid);
		// 원래의 몬스터로 변환..
		else if (iChangeType == 2)
			pNpcTable = m_pMain->m_NpcTableMap.GetData(m_sSid);

		if (pNpcTable == nullptr)
		{
			spdlog::error("Npc::ChangeMonsterInfo: changeNpcId not found [serial={} npcId={} npcName={} changeNpcId={}]",
				m_sNid + NPC_BAND, m_sSid, m_strName, m_sChangeSid);
		}
	}

	// 정보수정......
	Load(pNpcTable, false);
}

void CNpc::DurationMagic_3(CIOCPort* pIOCP, float currenttime)
{
	int duration_damage = 0;

	for (int i = 0; i < MAX_MAGIC_TYPE3; i++)
	{
		if (m_MagicType3[i].byHPDuration == 0)
			continue;

		// 2초간격으로
		if (currenttime < (m_MagicType3[i].fStartTime + m_MagicType3[i].byHPInterval))
			continue;

		m_MagicType3[i].byHPInterval += 2;
		//TRACE(_T("DurationMagic_3,, [%d] curtime = %.2f, dur=%.2f, nid=%d, damage=%d\n"), i, currenttime, m_MagicType3[i].fStartTime, m_sNid+NPC_BAND, m_MagicType3[i].sHPAmount);

		// healing
		if (m_MagicType3[i].sHPAmount >= 0)
		{
		}
		// damage 계산식...
		else
		{
			duration_damage = m_MagicType3[i].sHPAmount;
			duration_damage = abs(duration_damage);

			// Npc가 죽은 경우,,
			if (!SetDamage(0, duration_damage, "**duration**", m_MagicType3[i].sHPAttackUserID, pIOCP))
			{
				SendExpToUserList(); // 경험치 분배!!
				SendDead(pIOCP);
				SendAttackSuccess(pIOCP, MAGIC_ATTACK_TARGET_DEAD, m_MagicType3[i].sHPAttackUserID, duration_damage, m_iHP, 1, DURATION_ATTACK);
				//TRACE(_T("&&&& Duration Magic attack .. pNpc->m_byHPInterval[%d] = %d &&&& \n"), i, m_MagicType3[i].byHPInterval);
				m_MagicType3[i].fStartTime = 0.0f;
				m_MagicType3[i].byHPDuration = 0;
				m_MagicType3[i].byHPInterval = 2;
				m_MagicType3[i].sHPAmount = 0;
				m_MagicType3[i].sHPAttackUserID = -1;
				duration_damage = 0;
			}
			else
			{
				SendAttackSuccess(pIOCP, ATTACK_SUCCESS, m_MagicType3[i].sHPAttackUserID, duration_damage, m_iHP, 1, DURATION_ATTACK);
				//TRACE(_T("&&&& Duration Magic attack .. pNpc->m_byHPInterval[%d] = %d &&&& \n"), i, m_MagicType3[i].byHPInterval);
			}
		}

		if (currenttime >= (m_MagicType3[i].fStartTime + m_MagicType3[i].byHPDuration))
		{	// 총 공격시간..
			m_MagicType3[i].fStartTime = 0.0f;
			m_MagicType3[i].byHPDuration = 0;
			m_MagicType3[i].byHPInterval = 2;
			m_MagicType3[i].sHPAmount = 0;
			m_MagicType3[i].sHPAttackUserID = -1;
			duration_damage = 0;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//	NPC가 잠자는경우.
//
void CNpc::NpcSleeping(CIOCPort* pIOCP)
{
	NpcTrace("NpcSleeping()");

	// sungyong test~
	/*
	if(m_byChangeType == 1)	{
		Dead(pIOCP, 1);
		ChangeMonsterInfomation(1);
		return;
	}	*/
	// ~sungyong test


	// 낮
	if (m_pMain->m_byNight == 1)
	{
		m_NpcState = NPC_STANDING;
		m_Delay = 0;
	}
	// 밤
	else
	{
		m_NpcState = NPC_SLEEPING;
		m_Delay = m_sStandTime;
	}

	m_fDelayTime = TimeGet();
}

/////////////////////////////////////////////////////////////////////////////
// 몬스터가 기절상태로..........
void CNpc::NpcFainting(CIOCPort* pIOCP, float currenttime)
{
	NpcTrace("NpcFainting()");

	// 2초동안 기절해 있다가,,  standing상태로....
	if (currenttime > (m_fFaintingTime + FAINTING_TIME))
	{
		m_NpcState = NPC_STANDING;
		m_Delay = 0;
		m_fDelayTime = TimeGet();
		m_fFaintingTime = 0.0f;
	}
}

/////////////////////////////////////////////////////////////////////////////
// 몬스터가 치료상태로..........
void CNpc::NpcHealing(CIOCPort* pIOCP)
{
	NpcTrace("NpcHealing()");

	if (m_tNpcType != NPC_HEALER)
	{
		InitTarget();
		m_NpcState = NPC_STANDING;
		m_Delay = m_sStandTime;
		m_fDelayTime = TimeGet();
		return;
	}

	// 치료대상이 치료가 다 됐는지를 판단.. 
	CNpc* pNpc = nullptr;
	int nID = m_Target.id;
	bool bFlag = false;
	char buff[256] = {};
	int send_index = 0, iHP = 0;

	int ret = 0;
	int nStandingTime = m_sStandTime;

	ret = IsCloseTarget(m_byAttackRange, 2);

	if (ret == 0)
	{
		// 고정 경비병은 추적을 하지 않도록..
		if (m_tNpcType == NPC_DOOR
			|| m_tNpcType == NPC_ARTIFACT
			|| m_tNpcType == NPC_PHOENIX_GATE
			|| m_tNpcType == NPC_GATE_LEVER
			|| m_tNpcType == NPC_DOMESTIC_ANIMAL
			|| m_tNpcType == NPC_SPECIAL_GATE
			|| m_tNpcType == NPC_DESTORY_ARTIFACT)
		{
			m_NpcState = NPC_STANDING;
			InitTarget();
			m_Delay = 0;
			m_fDelayTime = TimeGet();
			return;
		}

		m_sStepCount = 0;
		m_byActionFlag = ATTACK_TO_TRACE;
		m_NpcState = NPC_TRACING;			// 공격하고 도망가는 유저를 따라 잡기위해(반응을 좀더 빠르게) 
		m_Delay = 0;
		m_fDelayTime = TimeGet();
		return;							// IsCloseTarget()에 유저 x, y값을 갱신하고 Delay = 0으로 줌
	}
	else if (ret == 2)
	{
		//if(m_tNpcType == NPC_BOSS_MONSTER)	{		// 대장 몬스터이면.....

		// 직접, 간접(롱)공격이 가능한 몬스터 이므로 장거리 공격을 할 수 있다.
		if (m_tNpcLongType == 2)
		{
			m_Delay = LongAndMagicAttack(pIOCP);
			m_fDelayTime = TimeGet();
			return;
		}
		else
		{
			// 고정 경비병은 추적을 하지 않도록..
			if (m_tNpcType == NPC_DOOR
				|| m_tNpcType == NPC_ARTIFACT
				|| m_tNpcType == NPC_PHOENIX_GATE
				|| m_tNpcType == NPC_GATE_LEVER
				|| m_tNpcType == NPC_DOMESTIC_ANIMAL
				|| m_tNpcType == NPC_SPECIAL_GATE
				|| m_tNpcType == NPC_DESTORY_ARTIFACT)
			{
				m_NpcState = NPC_STANDING;
				InitTarget();
				m_Delay = 0;
				m_fDelayTime = TimeGet();
				return;
			}

			m_sStepCount = 0;
			m_byActionFlag = ATTACK_TO_TRACE;
			m_NpcState = NPC_TRACING;			// 공격하고 도망가는 유저를 따라 잡기위해(반응을 좀더 빠르게) 
			m_Delay = 0;
			m_fDelayTime = TimeGet();
			return;								// IsCloseTarget()에 유저 x, y값을 갱신하고 Delay = 0으로 줌
		}
	}
	else if (ret == -1)
	{
		m_NpcState = NPC_STANDING;
		InitTarget();
		m_Delay = 0;
		m_fDelayTime = TimeGet();
		return;
	}

	if (nID >= NPC_BAND
		&& nID < INVALID_BAND)
	{
		pNpc = m_pMain->m_NpcMap.GetData(nID - NPC_BAND);

		// User 가 Invalid 한 경우
		if (pNpc == nullptr
			|| pNpc->m_iHP <= 0
			|| pNpc->m_NpcState == NPC_DEAD)
		{
			InitTarget();
			m_NpcState = NPC_STANDING;
			m_Delay = m_sStandTime;
			m_fDelayTime = TimeGet();
			return;
		}

		// 치료 체크여부 
		iHP = pNpc->m_iMaxHP * 0.9;		// 90퍼센트의 HP

		// Heal 완료상태..
		if (pNpc->m_iHP >= iHP)
		{
			InitTarget();
		}
		// Heal 해주기
		else
		{
			memset(buff, 0x00, 256);
			send_index = 0;
			//SetByte( buff, AG_MAGIC_ATTACK_RESULT, send_index );
			SetByte(buff, MAGIC_EFFECTING, send_index);
			SetDWORD(buff, m_iMagic3, send_index);				// FireBall
			SetShort(buff, m_sNid + NPC_BAND, send_index);
			SetShort(buff, nID, send_index);
			SetShort(buff, 0, send_index);					// data0
			SetShort(buff, 0, send_index);
			SetShort(buff, 0, send_index);
			SetShort(buff, 0, send_index);
			SetShort(buff, 0, send_index);
			SetShort(buff, 0, send_index);
			m_MagicProcess.MagicPacket(buff, send_index, pIOCP);

			m_Delay = m_sAttackDelay;
			m_fDelayTime = TimeGet();
			return;
			//SendAll(pIOCP, buff, send_index);
		}
	}

	// 새로운 치료대상을 찾아서 힐해준다
	int iMonsterNid = FindFriend(2);
	if (iMonsterNid == 0)
	{
		InitTarget();
		m_NpcState = NPC_STANDING;
		m_Delay = m_sStandTime;
		m_fDelayTime = TimeGet();
		return;
	}

	memset(buff, 0, sizeof(buff));
	send_index = 0;
	//SetByte( buff, AG_MAGIC_ATTACK_RESULT, send_index );
	SetByte(buff, MAGIC_EFFECTING, send_index);
	SetDWORD(buff, m_iMagic3, send_index);				// FireBall
	SetShort(buff, m_sNid + NPC_BAND, send_index);
	SetShort(buff, iMonsterNid, send_index);
	SetShort(buff, 0, send_index);					// data0
	SetShort(buff, 0, send_index);
	SetShort(buff, 0, send_index);
	SetShort(buff, 0, send_index);
	SetShort(buff, 0, send_index);
	SetShort(buff, 0, send_index);

	m_MagicProcess.MagicPacket(buff, send_index, pIOCP);
	//SendAll(pIOCP, buff, send_index);

	m_Delay = m_sAttackDelay;
	m_fDelayTime = TimeGet();
}

int CNpc::GetPartyExp(int party_level, int man, int nNpcExp)
{
	int nPartyExp = 0;
	int nLevel = party_level / man;
	double TempValue = 0;
	nLevel = m_sLevel - nLevel;

	//TRACE(_T("GetPartyExp ==> party_level=%d, man=%d, exp=%d, nLevle=%d, mon=%d\n"), party_level, man, nNpcExp, nLevel, m_sLevel);

	if (nLevel < 2)
	{
		nPartyExp = nNpcExp;
	}
	else if (nLevel < 5)
	{
		TempValue = nNpcExp * 1.2;
		nPartyExp = (int) TempValue;
		if (TempValue > nPartyExp)
			++nPartyExp;
	}
	else if (nLevel < 8)
	{
		TempValue = nNpcExp * 1.5;
		nPartyExp = (int) TempValue;
		if (TempValue > nPartyExp)
			++nPartyExp;
	}
	else if (nLevel >= 8)
	{
		nPartyExp = nNpcExp * 2;
	}

	return nPartyExp;
}

// iChangeType - 0:능력치 다운, 1:능력치 회복
void CNpc::ChangeAbility(int iChangeType)
{
	if (iChangeType > 2)
		return;

	int nHP = 0, nAC = 0, nDamage = 0, nLightR = 0, nMagicR = 0, nDiseaseR = 0, nPoisonR = 0, nLightningR = 0, nFireR = 0, nColdR = 0;
	model::Npc* pNpcTable = nullptr;
	if (m_byInitMoveType >= 0
		&& m_byInitMoveType < 100)
	{
		spdlog::error("Npc::ChangeAbility: invalid initMoveType [serial={} npcId={} npcName={} initMoveType={}]",
			m_sNid + NPC_BAND, m_sSid, m_strName, m_byInitMoveType);
		return;
	}
	else if (m_byInitMoveType >= 100)
	{
		pNpcTable = m_pMain->m_NpcTableMap.GetData(m_sSid);
		if (pNpcTable == nullptr)
		{
			spdlog::error("Npc::ChangeAbility: invalid npcId [serial={} npcId={} npcName={}]",
				m_sNid + NPC_BAND, m_sSid, m_strName);
			return;
		}
	}

	// 정보수정......
	// 능력치 다운
	if (iChangeType == BATTLEZONE_OPEN)
	{
		nHP			= static_cast<int>(pNpcTable->HitPoints * 0.5);
		nAC			= static_cast<int>(pNpcTable->Armor * 0.2);
		nDamage		= static_cast<int>(pNpcTable->Damage * 0.3);
		nLightR		= static_cast<int>(pNpcTable->LightResist * 0.5);
		nMagicR		= static_cast<int>(pNpcTable->MagicResist * 0.5);
		nDiseaseR	= static_cast<int>(pNpcTable->DiseaseResist * 0.5);
		nPoisonR	= static_cast<int>(pNpcTable->PoisonResist * 0.5);
		nLightningR	= static_cast<int>(pNpcTable->LightningResist * 0.5);
		nFireR		= static_cast<int>(pNpcTable->FireResist * 0.5);
		nColdR		= static_cast<int>(pNpcTable->ColdResist * 0.5);

		m_iMaxHP	= nHP;

		// HP도 바꿔야 겠군,,
		if (m_iHP > nHP)
			HpChange(&m_pMain->m_Iocport);

		m_sDefense = nAC;
		m_sDamage = nDamage;
		m_sFireR = nFireR;				// 화염 저항력
		m_sColdR = nColdR;				// 냉기 저항력
		m_sLightningR = nLightningR;	// 전기 저항력
		m_sMagicR = nMagicR;			// 마법 저항력
		m_sDiseaseR = nDiseaseR;		// 저주 저항력
		m_sPoisonR = nPoisonR;			// 독 저항력
		m_sLightR = nLightR;			// 빛 저항력
		//TRACE(_T("-- ChangeAbility down : nid=%d, name=%hs, hp:%d->%d, damage=%d->%d\n"), m_sNid+NPC_BAND, m_strName, pNpcTable->m_iMaxHP, nHP, pNpcTable->m_sDamage, nDamage); 
	}
	// 능력치 회복
	else if (iChangeType == BATTLEZONE_CLOSE)
	{
		m_iMaxHP = pNpcTable->HitPoints;			// 현재 HP
		//TRACE(_T("++ ChangeAbility up : nid=%d, name=%hs, hp:%d->%d, damage=%d->%d\n"), m_sNid+NPC_BAND, m_strName, m_iHP, m_iMaxHP, pNpcTable->m_sDamage, nDamage); 

		// HP도 바꿔야 겠군,,
		if (m_iMaxHP > m_iHP)
		{
			m_iHP = m_iMaxHP - 50;
			HpChange(&m_pMain->m_Iocport);
		}

		m_sDamage = pNpcTable->Damage;				// 기본 데미지
		m_sDefense = pNpcTable->Armor;				// 방어값
		m_sFireR = pNpcTable->FireResist;			// 화염 저항력
		m_sColdR = pNpcTable->ColdResist;			// 냉기 저항력
		m_sLightningR = pNpcTable->LightningResist;	// 전기 저항력
		m_sMagicR = pNpcTable->MagicResist;			// 마법 저항력
		m_sDiseaseR = pNpcTable->DiseaseResist;		// 저주 저항력
		m_sPoisonR = pNpcTable->PoisonResist;		// 독 저항력
		m_sLightR = pNpcTable->LightResist;			// 빛 저항력
	}
}

bool CNpc::Teleport(CIOCPort* pIOCP)
{
	int send_index = 0, retryCount = 0, maxRetry = 500;
	char buff[256] = {};
	int nX = 0, nZ = 0, nTileX = 0, nTileZ = 0;

	MAP* pMap = m_pMain->GetMapByIndex(m_ZoneIndex);
	if (pMap == nullptr)
		return false;

	while (1)
	{
		retryCount++;
		nX = myrand(0, 10);
		nX = myrand(0, 10);
		nX = (int) m_fCurX + nX;
		nZ = (int) m_fCurZ + nZ;
		nTileX = nX / TILE_SIZE;
		nTileZ = nZ / TILE_SIZE;

		if (nTileX >= (pMap->m_sizeMap.cx - 1))
			nTileX = pMap->m_sizeMap.cx - 1;

		if (nTileZ >= (pMap->m_sizeMap.cy - 1))
			nTileZ = pMap->m_sizeMap.cy - 1;

		if (nTileX < 0
			|| nTileZ < 0)
		{
			spdlog::error("Npc::Teleport: tile coordinates invalid [serial={} npcId={} npcName={} tileX={} tileZ={}]",
				m_sNid + NPC_BAND, m_sSid, m_strName, nTileX, nTileZ);
			return false;
		}

		if (pMap->m_pMap[nTileX][nTileZ].m_sEvent <= 0)
		{
			if (retryCount >= maxRetry)
			{
				spdlog::error("Npc::Teleport: max retries exceeded [npcId={} serial={} zoneId={} retryCount={} x={} z={}]",
					m_sSid, m_sNid + NPC_BAND, m_sCurZone, retryCount, nX, nZ);
				return false;
			}

			continue;
		}
		break;
	}

	SetByte(buff, AG_NPC_INOUT, send_index);
	SetByte(buff, NPC_OUT, send_index);
	SetShort(buff, m_sNid + NPC_BAND, send_index);
	Setfloat(buff, m_fCurX, send_index);
	Setfloat(buff, m_fCurZ, send_index);
	Setfloat(buff, m_fCurY, send_index);
	SendAll(pIOCP, buff, send_index);   // thread 에서 send

	m_fCurX = nX;
	m_fCurZ = nZ;

	memset(buff, 0, sizeof(buff));
	send_index = 0;
	SetByte(buff, AG_NPC_INOUT, send_index);
	SetByte(buff, NPC_IN, send_index);
	SetShort(buff, m_sNid + NPC_BAND, send_index);
	Setfloat(buff, m_fCurX, send_index);
	Setfloat(buff, m_fCurZ, send_index);
	Setfloat(buff, 0, send_index);
	SendAll(pIOCP, buff, send_index);   // thread 에서 send

	SetUid(m_fCurX, m_fCurZ, m_sNid + NPC_BAND);

	return true;
}

#ifndef SERVER_AISERVER_NPC_H
#define SERVER_AISERVER_NPC_H

#pragma once

#include "Map.h"
#include "PathFind.h"
#include "User.h"
#include "NpcMagicProcess.h"

#include <shared-server/My_3DStruct.h>
#include <shared-server/GeometricStructs.h>

#define MAX_MAP_SIZE		10000
#define MAX_PATH_SIZE		100

#define NPC_MAX_USER_LIST	5

#define NPC_ATTACK_SHOUT	0
#define NPC_SUBTYPE_LONG_MON 1

#define NPC_TRACING_STEP	100

#define NPC_HAVE_USER_LIST	5
#define NPC_HAVE_ITEM_LIST	6
#define NPC_PATTEN_LIST		5
#define NPC_PATH_LIST		50
#define NPC_MAX_PATH_LIST	100
#define NPC_EXP_RANGE		50
#define NPC_EXP_PERSENT		50

#define NPC_SECFORMETER_MOVE	4
#define NPC_SECFORMETER_RUN		4
#define NPC_VIEW_RANGE			100

#define MAX_MAGIC_TYPE3			20
#define MAX_MAGIC_TYPE4			9

struct  _NpcSkillList
{
	int16_t	sSid;
	uint8_t	tLevel;
	uint8_t	tOnOff;
};

struct  _NpcGiveItem
{
	int 	sSid;			// item serial number
	int16_t	count;			// item 갯수(돈은 단위)
};

struct	_ExpUserList
{
	char	strUserID[MAX_ID_SIZE + 1];		// 아이디(캐릭터 이름)
	int		iUid;							// User uid
	int		nDamage;						// 타격치 합
	bool	bIs;							// 시야에 존재하는지를 판단(true:존재)
	//bool	bSameParty;						// 같은 파티 소속이 있다면 true, 그렇지 않으면 false
};

struct _Target
{
	int	id;							// 공격대상 User uid
	float x;						// User의 x pos
	float y;						// User의 y pos
	float z;						// User의 z pos
	int failCount;
};

struct _PattenPos
{
	int16_t x;
	int16_t z;
};

struct _Patten
{
	int	patten_index;
	_PattenPos pPattenPos[NPC_MAX_PATH_LIST];
};

struct _PathList
{
	_PattenPos pPattenPos[NPC_MAX_PATH_LIST];
};

struct _MagicType3
{
	int16_t		sHPAttackUserID;	// 지속 마법을 사용한 유저의 아이디 저장
	int16_t		sHPAmount;			// 지속 damage ( 지속총양 / (지속시간 / 2) )
	uint8_t		byHPDuration;		// 지속 시간
	uint8_t		byHPInterval;		// 지속 간격
	double		fStartTime;			// 지속이 시작되는 시간..
};

struct _MagicType4
{
	uint8_t	byAmount;			// 양
	int16_t	sDurationTime;		// 지속 시간
	double	fStartTime;			// 지속이 시작되는 시간..
};

struct _TargetHealer
{
	int16_t	sNID;				// npc nid
	int16_t	sValue;				// 점수
};

class AIServerApp;

/*
	 ** Repent AI Server 작업시 참고 사항 **
	1. MONSTER DB 쪽에 있는 변수들 전부 수정..
*/
class CNpc
{
public:
	AIServerApp*		m_pMain;
	CNpcMagicProcess	m_MagicProcess;

	_Target		m_Target;			// 공격할 유저 저장,,
	int16_t		m_ItemUserLevel;	// 죽을때 매직 이상 아이템를 떨구기위해 참조해야하는 유저의레벨

	int			m_TotalDamage;		// 총 누적된 대미지양

	/// \brief List of user information that has dealt damage to the npc.
	/// Used for experience point distribution
	_ExpUserList m_DamagedUserList[NPC_HAVE_USER_LIST];
	
	int16_t		m_sMaxDamageUserid;	// 나에게 최고의 데미지를 준 유저의 아이디 저장..

	_PathList	m_PathList;			// Npc의 패스 리스트 
	_PattenPos	m_pPattenPos;		// Npc의 패턴,,

	//int m_iPattenNumber;			// 현재의 패턴번호
	int16_t		m_iPattenFrame;		// 패턴의 현재 위치..

	uint8_t		m_byMoveType;		// NPC의 행동타입(이동관련)
	uint8_t		m_byInitMoveType;	// NPC의 초기 행동타입(이동관련)
	int16_t		m_sPathCount;		// NPC의 PathList Count
	int16_t		m_sMaxPathCount;	// NPC의 PathList Max Count

	bool		m_bFirstLive;		// NPC 가 처음 생성되는지 죽었다 살아나는지 판단.
	uint8_t		m_NpcState;			// NPC의 상태 - 살았다, 죽었다, 서있다 등등...
	int16_t		m_ZoneIndex;		// NPC 가 존재하고 있는 존의 인덱스

	int16_t		m_sNid;				// NPC (서버상의)일련번호

	CMapInfo**	m_pOrgMap;			// 원본 MapInfo 에 대한 포인터

	float		m_nInitX;			// 처음 생성된 위치 X
	float		m_nInitY;			// 처음 생성된 위치 Y
	float		m_nInitZ;			// 처음 생성된 위치 Z

	int16_t		m_sCurZone;			// Current Zone;
	float		m_fCurX;			// Current X Pos;
	float		m_fCurY;			// Current Y Pos;
	float		m_fCurZ;			// Current Z Pos;

	float		m_fPrevX;			// Prev X Pos;
	float		m_fPrevY;			// Prev Y Pos;
	float		m_fPrevZ;			// Prev Z Pos;

	//
	//	PathFind Info
	//
	int16_t		m_min_x;
	int16_t		m_min_y;
	int16_t		m_max_x;
	int16_t		m_max_y;

	int			m_pMap[MAX_MAP_SIZE]; // 2차원 -> 1차원 배열로 x * sizey + y

	_SIZE		m_vMapSize;

	float		m_fStartPoint_X, m_fStartPoint_Y;
	float		m_fEndPoint_X, m_fEndPoint_Y;

	int16_t		m_sStepCount;

	CPathFind	m_vPathFind;
	_PathNode*	m_pPath;

	int			m_nInitMinX;		// 초기위치
	int			m_nInitMinY;
	int			m_nInitMaxX;
	int			m_nInitMaxY;

	// NPC Class, Skill
	//_NpcSkillList	m_NpcSkill[SKILL_NUM];	// NPC 가 가지고 있는 스킬
	//uint8_t			m_tWeaponClass;			// NPC 의 무기 클래스 

	// 지속 마법 관련..
	double		m_fHPChangeTime;			// Hp 회복율
	double		m_fFaintingTime;			// 기절해 있는 시간..
	_MagicType3  m_MagicType3[MAX_MAGIC_TYPE3];			// HP 관련된 마법..
	_MagicType4  m_MagicType4[MAX_MAGIC_TYPE4];			// 능력치 관련된 마법..

	//----------------------------------------------------------------
	//	MONSTER DB 쪽에 있는 변수들
	//----------------------------------------------------------------
	int16_t	m_sSid;				// MONSTER(NPC) Serial ID
	std::string	m_strName;		// MONSTER(NPC) Name
	int16_t	m_sPid;				// MONSTER(NPC) Picture ID
	int16_t	m_sSize;			// 캐릭터의 비율(100 퍼센트 기준)
	int		m_iWeapon_1;		// 착용 무기
	int		m_iWeapon_2;		// 착용 무기
	uint8_t	m_byGroup;			// 소속집단(국가 개념)
	uint8_t	m_byActType;		// 행동패턴
	uint8_t	m_byRank;			// 작위
	uint8_t	m_byTitle;			// 지위
	int 	m_iSellingGroup;	// 아이템 그룹(물건매매 담당 NPC의 경우만)
	int16_t	m_sLevel;			// level
	int		m_iExp;				// 경험치
	int		m_iLoyalty;			// loyalty
	int		m_iMaxHP;			// 최대 HP
	int16_t	m_sMaxMP;			// 최대 MP
	int16_t	m_sAttack;			// 공격값(지금 사용하지 않음..)
	int16_t	m_sDefense;			// 방어값
	int16_t	m_sHitRate;			// 공격민첩
	int16_t	m_sEvadeRate;		// 방어민첩
	int16_t	m_sDamage;			// 기본 데미지 - 공격값
	int16_t	m_sAttackDelay;		// 공격딜레이
	int16_t	m_sSpeed;			// 이동속도	
	float	m_fSpeed_1;			// 기본 이동 타입		(1초에 갈 수 있는 거리)
	float	m_fSpeed_2;			// 뛰는 이동 타입..		(1초에 갈 수 있는 거리)
	int16_t	m_sStandTime;		// 서있는 시간
	int		m_iMagic1;			// 사용마법 1 (공격)
	int		m_iMagic2;			// 사용마법 2 (지역)	
	int		m_iMagic3;			// 사용마법 3 (능력치, 힐링)	
	int16_t	m_sFireR;			// 화염 저항력
	int16_t	m_sColdR;			// 냉기 저항력
	int16_t	m_sLightningR;		// 전기 저항력
	int16_t	m_sMagicR;			// 마법 저항력
	int16_t	m_sDiseaseR;		// 저주 저항력
	int16_t	m_sPoisonR;			// 독 저항력
	int16_t	m_sLightR;			// 빛 저항력
	float	m_fBulk;			// 몬스터의 크기 (실제 비율)
	uint8_t	m_bySearchRange;	// 적 탐지 범위
	uint8_t	m_byAttackRange;	// 사정거리
	uint8_t	m_byTracingRange;	// 추격 거리

	uint8_t	m_tNpcType;			// NPC Type
								// 0 : Normal Monster
								// 1 : NPC

	int16_t	m_byFamilyType;		// 몹들사이에서 가족관계를 결정한다.
	uint8_t	m_byMoneyType;		// Event몬스터일 경우 돈을 많이 주는 것, (0:루팅, 1:루팅을 하지 않고 바로 나눠갖는다)

	int		m_iMoney;			// 떨어지는 돈
	int		m_iItem;			// 떨어지는 아이템
	//uint8_t	m_byMemory;		// 공격 타입 

	int		m_iHP;				// 현재 HP
	int16_t	m_sMP;				// 현재 MP

	/// \brief Distance that can be traveled per second
	float   m_fSecForMetor;

	//----------------------------------------------------------------
	//	MONSTER AI에 관련된 변수들
	//----------------------------------------------------------------
	uint8_t	m_tNpcLongType;			// 공격 거리 : 원거리(1), 근거리(0), 직.간접(2)
	uint8_t	m_tNpcAttType;			// 공격 성향 : 선공(1), 후공(0)
	uint8_t	m_tNpcOldAttType;		// 공격 성향 : 선공(1), 후공(0) (활동영역 제어)
	uint8_t	m_tNpcGroupType;		// 군집을 형성하냐(1), 안하냐?(0)
	uint8_t	m_byNpcEndAttType;		// 마지막까지 싸우면(1), 그렇지 않으면(0)
//	uint8_t	m_tNpcTraceType;		// 끝까지 따라간다(1), 시야에서 없어지면 그만(0)
	uint8_t	m_byAttackPos;			// User의 어느 부분에서 공격하느지를 판단(8방향)
	uint8_t	m_byBattlePos;			// 어떤 진형을 선택할 것인지를 판단..
	uint8_t	m_byWhatAttackType;		// 공격 타입 : Normal(0), 근.장거리마법(1), 독(2), 힐링(3), 지역마법만(4), 1+4번 마법(5)
	uint8_t	m_byGateOpen;			// 성문일 경우에.. 사용... Gate Npc Status -> 1 : open 0 : close
	uint8_t	m_byMaxDamagedNation;	// 나를 죽인 유저의 국가를 저장.. (1:카루스, 2:엘모라드)
	uint8_t	m_byObjectType;         // 보통은 0, object타입(성문, 레버)은 1
	uint8_t	m_byDungeonFamily;		// 던젼에서 같은 패밀리 묶음 (같은 방)
	uint8_t	m_bySpecialType;		// 몬스터의 형태가 변하는지를 판단(0:변하지 않음, 1:변하는 몬스터, 
									// 2:죽는경우 조정하는 몬스터(대장몬스터 죽을경우 성문이 열림), 
									// 3:대장몬스터의 죽음과 관련이 있는 몬스터(대장몬스터가 죽으면 관계되는 몬스터는 같이 죽도록)
									// 4:변하면서 죽는경우 조정하는 몬스터 (m_sControlSid)
									// 5:처음에 죽었있다가 출현하는 몬스터,,
									// 6:일정시간이 지난 후에 행동하는 몬스터,,
									// 100:죽었을때 데미지를 많이 입힌 유저를 기록해 주세여
	uint8_t	m_byTrapNumber;			// 던젼에서 트랩의 번호,,

	/// \brief change state, one of:
	/// 0: normal state
	/// 1: preparing to change
	/// 2: changed into different monster
	/// 3: spawned
	/// 4: dead
	uint8_t	m_byChangeType;

	/// \brief Respawn type, one of:
	/// 0: normal respawn
	/// 1: special monster, does not respawn after death
	/// 2: does not respawn
	uint8_t	m_byRegenType;
	
	uint8_t	m_byDeadType;			// 0:살아 있는 경우, 100:전쟁이벤트중 죽은 경우
	int16_t	m_sChangeSid;			// 변하는 몬스터의 Sid번호..
	int16_t	m_sControlSid;			// 조정하는 몬스터의 Sid번호..


	//----------------------------------------------------------------
	//	MONSTER_POS DB 쪽에 있는 변수들
	//----------------------------------------------------------------
	int		m_Delay;			// 다음 상태로 전이되기 까지의 시간
	double	m_fDelayTime;		// Npc Thread체크 타임...

	uint8_t	m_byType;
	int		m_sRegenTime;		// NPC 재생시간

	int		m_nLimitMinX;		// 활동 영역
	int		m_nLimitMinZ;
	int		m_nLimitMaxX;
	int		m_nLimitMaxZ;

	float	m_fAdd_x;
	float	m_fAdd_z;
			
	float	m_fBattlePos_x;
	float	m_fBattlePos_z;

	/// \brief Distance that can be traveled per second (actual distance sent to the client)
	float	m_fSecForRealMoveMetor;
	
	uint8_t	m_byDirection;					// NPC의 방향

	bool	m_bPathFlag;					// 패스 파인드 실행여부 체크 변수..

	//----------------------------------------------------------------
	//	NPC 이동 관련
	//----------------------------------------------------------------
	_NpcPosition	m_pPoint[MAX_PATH_LINE];			// 이동시 참고 좌표

	int16_t m_iAniFrameIndex;
	int16_t m_iAniFrameCount;
	uint8_t m_byPathCount;				// 패스를 따라 이동하는 몬스터 끼리 겹치지 않도록,, 
	uint8_t m_byResetFlag;				// 추적공격시 패스파인딩을 다시 할것인지,, 말것인지를 판단..
	uint8_t m_byActionFlag;				// 행동변화 플래그 ( 0 : 행동변화 없음, 1 : 공격에서 추격)

	int16_t m_iRegion_X;				// 현재의 region - x pos
	int16_t m_iRegion_Z;				// 현재의 region - z pos
	int16_t m_iFind_X[4];				// find enemy에서 찾을 Region검사영역
	int16_t m_iFind_Y[4];

	float   m_fOldSpeed_1;				// 기본 이동 타입		(1초에 갈 수 있는 거리)
	float   m_fOldSpeed_2;				// 뛰는 이동 타입..		(1초에 갈 수 있는 거리)

	// test
	int16_t m_sThreadNumber;			// 자신이 속한 스레드의 번호

public:
	CNpc();
	virtual ~CNpc();

	void Load(const model::Npc* pNpcTable, bool transformSpeeds);
	void Init();	//	NPC 기본정보 초기화
	void InitTarget(void);
	void InitUserList();
	void InitPos();
	void InitMagicValuable();

protected:
	void ClearPathFindData(void);

public:
	void FillNpcInfo(char* temp_send, int& index, uint8_t flag);
	void NpcStrategy(uint8_t type);
	void NpcTypeParser();
	int  FindFriend(int type = 0);
	void  FindFriendRegion(int x, int z, MAP* pMap, _TargetHealer* pHealer, int type = 0);
	//void  FindFriendRegion(int x, int z, MAP* pMap, int type=0);
	bool IsCloseTarget(CUser* pUser, int nRange);
	int SendDead(int type = 1);			// Npc Dead
	void SendExpToUserList();								// User 경험치 분배..
	bool SetDamage(int nAttackType, int nDamage, const char* sourceName, int uid);	// Npc의 데미지 계산..
	bool SetHMagicDamage(int nDamage);	// Npc의 데미지 계산..
	int GetDefense();										// Npc의 방어값..
	void ChangeTarget(int nAttackType, CUser* pUser);
	void ChangeNTarget(CNpc* pNpc);
	int GetFinalDamage(CUser* pUser, int type = 1);
	int GetNFinalDamage(CNpc* pNpc);
	uint8_t GetHitRate(float rate);
	bool ResetPath();
	bool GetTargetPos(float& x, float& z);
	bool IsChangePath(int nStep = 1);
	int Attack();
	int LongAndMagicAttack();
	int TracingAttack();
	int GetTargetPath(int option = 0);
	int	GetPartyDamage(int iNumber);
	int IsCloseTarget(int nRange, int Flag = 0);
	bool StepMove(int nStep);
	bool StepNoPathMove(int nStep);
	bool IsMovingEnd();
	bool IsMovable(float x, float z);
	int  IsSurround(CUser* pUser);
	bool IsDamagedUserList(CUser* pUser);
	void IsUserInSight();
	bool IsLevelCheck(int iLevel);
	bool IsHPCheck(int iHP);
	bool IsCompStatus(CUser* pUser);
	bool IsPathFindCheck(float fDistance);						// 패스 파인드를 할것인지를 체크하는 루틴..
	void IsNoPathFind(float fDistance);						// 패스 파인드를 하지 않고 공격대상으로 가는 루틴..
	bool IsInExpRange(CUser* pUser);
	void GiveNpcHaveItem();		// NPC 가 가진 아이템을 떨군다

	void NpcLive();
	void NpcFighting();
	void NpcTracing();
	void NpcAttacking();
	void NpcMoving();
	void NpcSleeping();
	void NpcFainting(double currentTime);
	void NpcHealing();
	void NpcStanding();
	void NpcBack();
	bool SetLive();

	bool IsInRange(int nX, int nZ);
	bool RandomMove();
	bool RandomBackMove();
	bool IsInPathRange();
	int GetNearPathPoint();

	// Packet Send부분..
	void SendAll(const char* pBuf, int nLength);
	void SendAttackSuccess(uint8_t byResult, int tuid, int16_t sDamage, int nHP = 0, uint8_t byFlag = 0, uint8_t byAttackType = 1);
	void SendNpcInfoAll(char* temp_send, int& index, int count);	// game server에 npc정보를 전부 전송...

	// Inline Function
	bool SetUid(float x, float z, int id);

	void Dead(int iDeadType = 0);
	bool FindEnemy();
	bool CheckFindEnemy();
	int FindEnemyRegion();
	float FindEnemyExpand(int nRX, int nRZ, float fCompDis, int nType);
	int GetMyField();

	void NpcTrace(std::string_view msg);

	int GetDir(float x1, float z1, float x2, float z2);
	void NpcMoveEnd();

	__Vector3 GetDirection(__Vector3 vStart, __Vector3 vEnd);
	// GetVectorPosition : vOrig->vDest방향으로 vOrig에서 fDis거리만큼 떨어진 좌표를 리턴
	__Vector3 GetVectorPosition(__Vector3 vOrig, __Vector3 vDest, float fDis);
	// CalcAdaptivePosition : vPosDest->vPosOrig방향으로 vPosDest에서 fDis거리만큼 떨어진 좌표를 리턴
	__Vector3 CalcAdaptivePosition(__Vector3 vPosOrig, __Vector3 vPosDest, float fAttackDistance);
	__Vector3 ComputeDestPos(__Vector3 vCur, float fDegree, float fDegreeOffset, float fDistance);
	void Yaw2D(float fDirX, float fDirZ, float& fYawResult);
	float GetDistance(__Vector3 vOrig, __Vector3 vDest);
	int  PathFind(_POINT start, _POINT end, float fDistance);
	bool GetUserInView();	// Npc의 가시 거리안에 User가 있는지를 판단
	bool GetUserInViewRange(int x, int z);
	void MoveAttack();
	void HpChange();
	void MSpChange(int type, int amount);
	void ItemWoreOut(int type, int damage);
	int	 ItemProdution(int item_number) const;
	int  GetItemGrade(int item_grade) const;
	int  GetItemCodeNumber(int level, int item_type) const;
	int  GetWeaponItemCodeNumber(int item_type) const;
	int  GetItemGroupNumber(int groupId) const;
	void DurationMagic_4(double currentTime);
	void DurationMagic_3(double currentTime);
	void ChangeMonsterInfo(int iChangeType);
	int  GetPartyExp(int party_level, int man, int nNpcExp);
	void ChangeAbility(int iChangeType);
	bool Teleport();
};

#endif // SERVER_AISERVER_NPC_H

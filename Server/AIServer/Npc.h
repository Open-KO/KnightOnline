// Npc.h: interface for the CNpc class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NPC_H__6077F7AF_6166_463A_AA80_FBF218781BC6__INCLUDED_)
#define AFX_NPC_H__6077F7AF_6166_463A_AA80_FBF218781BC6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "iocport.h"
#include "./N3Base/My_3DStruct.h"
#include "Map.h"
#include "PathFind.h"
#include "user.h"
#include "NpcMagicProcess.h"

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
	short	sSid;
	BYTE	tLevel;
	BYTE	tOnOff;
};

struct  _NpcGiveItem
{
	int 	sSid;			// item serial number
	short	count;			// item 갯수(돈은 단위)
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
	short x;
	short z;
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
	short		sHPAttackUserID;	// 지속 마법을 사용한 유저의 아이디 저장
	short		sHPAmount;			// 지속 damage ( 지속총양 / (지속시간 / 2) )
	BYTE		byHPDuration;		// 지속 시간
	BYTE		byHPInterval;		// 지속 간격
	float		fStartTime;			// 지속이 시작되는 시간..
};

struct _MagicType4
{
	BYTE	byAmount;			// 양
	short	sDurationTime;		// 지속 시간
	float   fStartTime;			// 지속이 시작되는 시간..
};

struct _TargetHealer
{
	short	sNID;				// npc nid
	short	sValue;				// 점수
};

class CServerDlg;

/*
	 ** Repent AI Server 작업시 참고 사항 **
	1. MONSTER DB 쪽에 있는 변수들 전부 수정..
*/

class CNpc
{
public:

	CServerDlg* m_pMain;
	CNpcMagicProcess m_MagicProcess;

	_Target	m_Target;				// 공격할 유저 저장,,
	short		m_ItemUserLevel;		// 죽을때 매직 이상 아이템를 떨구기위해 참조해야하는 유저의레벨

	int		m_TotalDamage;	// 총 누적된 대미지양

	/// \brief List of user information that has dealt damage to the npc.
	/// Used for experience point distribution
	_ExpUserList m_DamagedUserList[NPC_HAVE_USER_LIST];
	
	short   m_sMaxDamageUserid;		// 나에게 최고의 데미지를 준 유저의 아이디 저장..

	_PathList m_PathList;			// Npc의 패스 리스트 
	_PattenPos m_pPattenPos;		// Npc의 패턴,,

	//int m_iPattenNumber;		// 현재의 패턴번호
	short m_iPattenFrame;			// 패턴의 현재 위치..

	BYTE  m_byMoveType;			// NPC의 행동타입(이동관련)
	BYTE  m_byInitMoveType;		// NPC의 초기 행동타입(이동관련)
	short m_sPathCount;			// NPC의 PathList Count
	short m_sMaxPathCount;		// NPC의 PathList Max Count

	bool	m_bFirstLive;		// NPC 가 처음 생성되는지 죽었다 살아나는지 판단.
	BYTE	m_NpcState;			// NPC의 상태 - 살았다, 죽었다, 서있다 등등...
	short	m_ZoneIndex;		// NPC 가 존재하고 있는 존의 인덱스

	short	m_sNid;				// NPC (서버상의)일련번호

	CMapInfo** m_pOrgMap;		// 원본 MapInfo 에 대한 포인터

	float		m_nInitX;			// 처음 생성된 위치 X
	float		m_nInitY;			// 처음 생성된 위치 Y
	float		m_nInitZ;			// 처음 생성된 위치 Z

	short		m_sCurZone;			// Current Zone;
	float		m_fCurX;			// Current X Pos;
	float		m_fCurY;			// Current Y Pos;
	float		m_fCurZ;			// Current Z Pos;

	float		m_fPrevX;			// Prev X Pos;
	float		m_fPrevY;			// Prev Y Pos;
	float		m_fPrevZ;			// Prev Z Pos;

	//
	//	PathFind Info
	//
	short		m_min_x;
	short		m_min_y;
	short		m_max_x;
	short		m_max_y;

	int		m_pMap[MAX_MAP_SIZE];// 2차원 -> 1차원 배열로 x * sizey + y

	SIZE	m_vMapSize;

	float m_fStartPoint_X, m_fStartPoint_Y;
	float m_fEndPoint_X, m_fEndPoint_Y;

	short m_sStepCount;

	CPathFind m_vPathFind;
	_PathNode* m_pPath;

	int		m_nInitMinX;					// 초기위치
	int		m_nInitMinY;
	int		m_nInitMaxX;
	int		m_nInitMaxY;

	// NPC Class, Skill
	//_NpcSkillList	m_NpcSkill[SKILL_NUM];	// NPC 가 가지고 있는 스킬
	//BYTE			m_tWeaponClass;			// NPC 의 무기 클래스 

	// 지속 마법 관련..
	float		m_fHPChangeTime;			// Hp 회복율
	float       m_fFaintingTime;			// 기절해 있는 시간..
	_MagicType3  m_MagicType3[MAX_MAGIC_TYPE3];			// HP 관련된 마법..
	_MagicType4  m_MagicType4[MAX_MAGIC_TYPE4];			// 능력치 관련된 마법..

	//----------------------------------------------------------------
	//	MONSTER DB 쪽에 있는 변수들
	//----------------------------------------------------------------
	short	m_sSid;				// MONSTER(NPC) Serial ID
	std::string	m_strName;		// MONSTER(NPC) Name
	short	m_sPid;							// MONSTER(NPC) Picture ID
	short   m_sSize;						// 캐릭터의 비율(100 퍼센트 기준)
	int     m_iWeapon_1;			// 착용 무기
	int     m_iWeapon_2;			// 착용 무기
	BYTE	m_byGroup;			// 소속집단(국가 개념)
	BYTE	m_byActType;		// 행동패턴
	BYTE	m_byRank;			// 작위
	BYTE	m_byTitle;			// 지위
	int 	m_iSellingGroup;	// 아이템 그룹(물건매매 담당 NPC의 경우만)
	short	m_sLevel;			// level
	int		m_iExp;				// 경험치
	int		m_iLoyalty;			// loyalty
	int		m_iMaxHP;			// 최대 HP
	short	m_sMaxMP;			// 최대 MP
	short	m_sAttack;			// 공격값(지금 사용하지 않음..)
	short	m_sDefense;			// 방어값
	short	m_sHitRate;			// 공격민첩
	short   m_sEvadeRate;		// 방어민첩
	short	m_sDamage;			// 기본 데미지 - 공격값
	short	m_sAttackDelay;		// 공격딜레이
	short	m_sSpeed;			// 이동속도	
	float   m_fSpeed_1;			// 기본 이동 타입		(1초에 갈 수 있는 거리)
	float   m_fSpeed_2;			// 뛰는 이동 타입..		(1초에 갈 수 있는 거리)
	short	m_sStandTime;		// 서있는 시간
	int		m_iMagic1;			// 사용마법 1 (공격)
	int		m_iMagic2;			// 사용마법 2 (지역)	
	int		m_iMagic3;			// 사용마법 3 (능력치, 힐링)	
	short	m_sFireR;			// 화염 저항력
	short	m_sColdR;			// 냉기 저항력
	short	m_sLightningR;		// 전기 저항력
	short	m_sMagicR;			// 마법 저항력
	short	m_sDiseaseR;		// 저주 저항력
	short	m_sPoisonR;		// 독 저항력
	short	m_sLightR;			// 빛 저항력
	float	m_fBulk;			// 몬스터의 크기 (실제 비율)
	BYTE	m_bySearchRange;	// 적 탐지 범위
	BYTE	m_byAttackRange;	// 사정거리
	BYTE	m_byTracingRange;	// 추격 거리

	BYTE	m_tNpcType;			// NPC Type
								// 0 : Normal Monster
								// 1 : NPC

	short	m_byFamilyType;		// 몹들사이에서 가족관계를 결정한다.
	BYTE    m_byMoneyType;		// Event몬스터일 경우 돈을 많이 주는 것, (0:루팅, 1:루팅을 하지 않고 바로 나눠갖는다)

	int		m_iMoney;			// 떨어지는 돈
	int		m_iItem;			// 떨어지는 아이템
	//BYTE	m_byMemory;			// 공격 타입 

	int			m_iHP;				// 현재 HP
	short		m_sMP;				// 현재 MP

	/// \brief Distance that can be traveled per second
	float   m_fSecForMetor;

	//----------------------------------------------------------------
	//	MONSTER AI에 관련된 변수들
	//----------------------------------------------------------------
	BYTE	m_tNpcLongType;		// 공격 거리 : 원거리(1), 근거리(0), 직.간접(2)
	BYTE	m_tNpcAttType;		// 공격 성향 : 선공(1), 후공(0)
	BYTE	m_tNpcOldAttType;	// 공격 성향 : 선공(1), 후공(0) (활동영역 제어)
	BYTE	m_tNpcGroupType;	// 군집을 형성하냐(1), 안하냐?(0)
	BYTE	m_byNpcEndAttType;	// 마지막까지 싸우면(1), 그렇지 않으면(0)
//	BYTE	m_tNpcTraceType;	// 끝까지 따라간다(1), 시야에서 없어지면 그만(0)
	BYTE    m_byAttackPos;		// User의 어느 부분에서 공격하느지를 판단(8방향)
	BYTE    m_byBattlePos;		// 어떤 진형을 선택할 것인지를 판단..
	BYTE	m_byWhatAttackType; // 공격 타입 : Normal(0), 근.장거리마법(1), 독(2), 힐링(3), 지역마법만(4), 1+4번 마법(5)
	BYTE	m_byGateOpen;		// 성문일 경우에.. 사용... Gate Npc Status -> 1 : open 0 : close
	BYTE    m_byMaxDamagedNation;	// 나를 죽인 유저의 국가를 저장.. (1:카루스, 2:엘모라드)
	BYTE    m_byObjectType;         // 보통은 0, object타입(성문, 레버)은 1
	BYTE	m_byDungeonFamily;		// 던젼에서 같은 패밀리 묶음 (같은 방)
	BYTE	m_bySpecialType;		// 몬스터의 형태가 변하는지를 판단(0:변하지 않음, 1:변하는 몬스터, 
									// 2:죽는경우 조정하는 몬스터(대장몬스터 죽을경우 성문이 열림), 
									// 3:대장몬스터의 죽음과 관련이 있는 몬스터(대장몬스터가 죽으면 관계되는 몬스터는 같이 죽도록)
									// 4:변하면서 죽는경우 조정하는 몬스터 (m_sControlSid)
									// 5:처음에 죽었있다가 출현하는 몬스터,,
									// 6:일정시간이 지난 후에 행동하는 몬스터,,
									// 100:죽었을때 데미지를 많이 입힌 유저를 기록해 주세여
	BYTE	m_byTrapNumber;			// 던젼에서 트랩의 번호,,

	/// \brief change state, one of:
	/// 0: normal state
	/// 1: preparing to change
	/// 2: changed into different monster
	/// 3: spawned
	/// 4: dead
	BYTE	m_byChangeType;

	/// \brief Respawn type, one of:
	/// 0: normal respawn
	/// 1: special monster, does not respawn after death
	/// 2: does not respawn
	BYTE	m_byRegenType;
	
	BYTE    m_byDeadType;			// 0:살아 있는 경우, 100:전쟁이벤트중 죽은 경우
	short   m_sChangeSid;			// 변하는 몬스터의 Sid번호..
	short   m_sControlSid;			// 조정하는 몬스터의 Sid번호..


	//----------------------------------------------------------------
	//	MONSTER_POS DB 쪽에 있는 변수들
	//----------------------------------------------------------------
	int		m_Delay;			// 다음 상태로 전이되기 까지의 시간
	float   m_fDelayTime;		// Npc Thread체크 타임...

	BYTE	m_byType;
	int		m_sRegenTime;		// NPC 재생시간

	int		m_nLimitMinX;		// 활동 영역
	int		m_nLimitMinZ;
	int		m_nLimitMaxX;
	int		m_nLimitMaxZ;

	long	m_lEventNpc;

	float m_fAdd_x;
	float m_fAdd_z;

	float m_fBattlePos_x;
	float m_fBattlePos_z;

	/// \brief Distance that can be traveled per second (actual distance sent to the client)
	float m_fSecForRealMoveMetor;
	
	BYTE m_byDirection;					// NPC의 방향

	bool m_bPathFlag;					// 패스 파인드 실행여부 체크 변수..

	//----------------------------------------------------------------
	//	NPC 이동 관련
	//----------------------------------------------------------------
	_NpcPosition	m_pPoint[MAX_PATH_LINE];			// 이동시 참고 좌표

	short m_iAniFrameIndex;
	short m_iAniFrameCount;
	BYTE m_byPathCount;					// 패스를 따라 이동하는 몬스터 끼리 겹치지 않도록,, 
	BYTE m_byResetFlag;					// 추적공격시 패스파인딩을 다시 할것인지,, 말것인지를 판단..
	BYTE m_byActionFlag;				// 행동변화 플래그 ( 0 : 행동변화 없음, 1 : 공격에서 추격)

	short m_iRegion_X;					// 현재의 region - x pos
	short m_iRegion_Z;					// 현재의 region - z pos
	short m_iFind_X[4];					// find enemy에서 찾을 Region검사영역
	short m_iFind_Y[4];

	float   m_fOldSpeed_1;			// 기본 이동 타입		(1초에 갈 수 있는 거리)
	float   m_fOldSpeed_2;			// 뛰는 이동 타입..		(1초에 갈 수 있는 거리)

	// test
	short m_sThreadNumber;				// 자신이 속한 스레드의 번호


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

	void FillNpcInfo(char* temp_send, int& index, BYTE flag);
	void NpcStrategy(BYTE type);
	void NpcTypeParser();
	int  FindFriend(int type = 0);
	void  FindFriendRegion(int x, int z, MAP* pMap, _TargetHealer* pHealer, int type = 0);
	//void  FindFriendRegion(int x, int z, MAP* pMap, int type=0);
	bool IsCloseTarget(CUser* pUser, int nRange);
	int SendDead(CIOCPort* pIOCP, int type = 1);			// Npc Dead
	void SendExpToUserList();								// User 경험치 분배..
	bool SetDamage(int nAttackType, int nDamage, const char* sourceName, int uid, CIOCPort* pIOCP);	// Npc의 데미지 계산..
	bool SetHMagicDamage(int nDamage, CIOCPort* pIOCP);	// Npc의 데미지 계산..
	int GetDefense();										// Npc의 방어값..
	void ChangeTarget(int nAttackType, CUser* pUser, CIOCPort* pIOCP);
	void ChangeNTarget(CNpc* pNpc, CIOCPort* pIOCP);
	int GetFinalDamage(CUser* pUser, int type = 1);
	int GetNFinalDamage(CNpc* pNpc);
	BYTE GetHitRate(float rate);
	bool ResetPath();
	bool GetTargetPos(float& x, float& z);
	bool IsChangePath(int nStep = 1);
	int Attack(CIOCPort* pIOCP);
	int LongAndMagicAttack(CIOCPort* pIOCP);
	int TracingAttack(CIOCPort* pIOCP);
	int GetTargetPath(int option = 0);
	int	GetPartyDamage(int iNumber);
	int IsCloseTarget(int nRange, int Flag = 0);
	bool StepMove(int nStep, CIOCPort* pIOCP);
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
	void GiveNpcHaveItem(CIOCPort* pIOCP);		// NPC 가 가진 아이템을 떨군다

	void NpcLive(CIOCPort* pIOCP);
	void NpcFighting(CIOCPort* pIOCP);
	void NpcTracing(CIOCPort* pIOCP);
	void NpcAttacking(CIOCPort* pIOCP);
	void NpcMoving(CIOCPort* pIOCP);
	void NpcSleeping(CIOCPort* pIOCP);
	void NpcFainting(CIOCPort* pIOCP, float currenttime);
	void NpcHealing(CIOCPort* pIOCP);
	void NpcStanding();
	void NpcBack(CIOCPort* pIOCP);
	bool SetLive(CIOCPort* pIOCP);

	bool IsInRange(int nX, int nZ);
	bool RandomMove();
	bool RandomBackMove();
	bool IsInPathRange();
	int GetNearPathPoint();

	// Packet Send부분..
	void SendAll(CIOCPort* pIOCP, const char* pBuf, int nLength);
	void SendAttackSuccess(CIOCPort* pIOCP, BYTE byResult, int tuid, short sDamage, int nHP = 0, BYTE byFlag = 0, short sAttack_type = 1);
	void SendNpcInfoAll(char* temp_send, int& index, int count);	// game server에 npc정보를 전부 전송...

	// Inline Function
	bool SetUid(float x, float z, int id);

	void Dead(CIOCPort* pIOCP, int iDeadType = 0);
	bool FindEnemy();
	bool CheckFindEnemy();
	int FindEnemyRegion();
	float FindEnemyExpand(int nRX, int nRZ, float fCompDis, int nType);
	int GetMyField();

	void NpcTrace(std::string_view msg);

	int GetDir(float x1, float z1, float x2, float z2);
	void NpcMoveEnd(CIOCPort* pIOCP);

	inline float RandomGenf(float max, float min);
	__Vector3 MyFunc(__Vector3 vCur, __Vector3 vDir, float fYDegree, float fDistance);
	__Vector3 GetDirection(__Vector3 vStart, __Vector3 vEnd);
	// GetVectorPosition : vOrig->vDest방향으로 vOrig에서 fDis거리만큼 떨어진 좌표를 리턴
	__Vector3 GetVectorPosition(__Vector3 vOrig, __Vector3 vDest, float fDis);
	// CalcAdaptivePosition : vPosDest->vPosOrig방향으로 vPosDest에서 fDis거리만큼 떨어진 좌표를 리턴
	__Vector3 CalcAdaptivePosition(__Vector3 vPosOrig, __Vector3 vPosDest, float fAttackDistance);
	__Vector3 ComputeDestPos(__Vector3 vCur, float fDegree, float fDegreeOffset, float fDistance);
	void Yaw2D(float fDirX, float fDirZ, float& fYawResult);
	float GetDistance(__Vector3 vOrig, __Vector3 vDest);
	int  PathFind(CPoint start, CPoint end, float fDistance);
	bool GetUserInView();	// Npc의 가시 거리안에 User가 있는지를 판단
	bool GetUserInViewRange(int x, int z);
	void MoveAttack(CIOCPort* pIOCP);
	void HpChange(CIOCPort* pIOCP);
	void MSpChange(int type, int amount);
	void ItemWoreOut(int type, int damage);
	int	 ItemProdution(int item_number);
	int  GetItemGrade(int item_grade);
	int  GetItemCodeNumber(int level, int item_type);
	int  GetWeaponItemCodeNumber(int item_type);
	void DurationMagic_4(CIOCPort* pIOCP, float currenttime);
	void DurationMagic_3(CIOCPort* pIOCP, float currenttime);
	void ChangeMonsterInfo(int iChangeType);
	int  GetPartyExp(int party_level, int man, int nNpcExp);
	void ChangeAbility(int iChangeType);
	bool Teleport(CIOCPort* pIOCP);
};

#endif // !defined(AFX_NPC_H__6077F7AF_6166_463A_AA80_FBF218781BC6__INCLUDED_)

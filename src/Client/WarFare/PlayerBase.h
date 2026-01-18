// CPlayerBase.h: interface for the CPlayerBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PlayerBase_H__B8B8986B_3635_462D_8C38_A052CA75B331__INCLUDED_)
#define AFX_PlayerBase_H__B8B8986B_3635_462D_8C38_A052CA75B331__INCLUDED_

#pragma once

#include "GameBase.h"
#include "GameDef.h"
#include "Bitset.h"
#include <deque>
#include <string>

#include <N3Base/N3Chr.h>

//	By : Ecli666 ( On 2002-07-22 오전 9:59:19 )
//
inline constexpr int SHADOW_SIZE          = 32; // 2의 승수만 된다..
inline constexpr float SHADOW_PLANE_SIZE  = 4.6f;
inline constexpr uint8_t SHADOW_COLOR     = 10; // 16진수 한자리.. 알파
//	~(By Ecli666 On 2002-07-22 오전 9:59:19 )

inline constexpr float TIME_CORPSE_REMAIN = 90.0f; // 시체가 남는 시간..
inline constexpr float TIME_CORPSE_REMOVE = 10.0f; // 투명해지면서 없앨시간..

class CDFont;
class CN3SndObj;

struct __InfoPlayerBase
{
	int iID;          // 고유 ID
	std::string szID; // 이름
	D3DCOLOR crID;    // 이름 색깔..
	e_Race eRace;     // 캐릭터 골격에 따른 종족
	e_Nation eNation; // 소속 국가..
	e_Class eClass;   // 직업
	int iLevel;       // 레벨
	int iHPMax;
	int iHP;
	int iMP;
	int iMPMax;
	int iAuthority; // 권한 - 0 관리자, 1 - 일반유저, 255 - 블럭당한 유저...
	int iKnightsID; // Clan ID
	int iAllianceID;
	int iKnightsWarEnemyID;

	bool bRenderID; // 화면에 ID 를 찍는지..

	__InfoPlayerBase()
	{
		Init();
	}

	void Init()
	{
		iID = 0;                             // 고유 ID
		szID.clear();                        // 이름
		crID               = 0;              // 이름 색깔..
		eRace              = RACE_UNKNOWN;   // 캐릭터 골격에 따른 종족
		eNation            = NATION_UNKNOWN; // 소속 국가..
		eClass             = CLASS_UNKNOWN;  // 직업
		iLevel             = 0;              // 레벨
		iHPMax             = 0;
		iHP                = 0;
		iMP                = 0;
		iMPMax             = 0;
		iAuthority         = 1; // 권한 - 0 관리자, 1 - 일반유저, 255 - 블럭당한 유저...
		iKnightsID         = 0;
		iAllianceID        = 0;
		iKnightsWarEnemyID = 0;
		bRenderID          = true;
	}
};

class CN3ShapeExtra;
class CPlayerBase : public CGameBase
{
	friend class CPlayerBase;
	friend class CPlayerOtherMgr;

protected:
	e_PlayerType m_ePlayerType = PLAYER_BASE;    // Player Type ... Base, NPC, OTher, MySelf

	std::deque<e_Ani> m_AnimationDeque;          // 에니메이션 큐... 여기다 집어 넣으면 tick 을 돌면서 차례대로 한다..
	bool m_bAnimationChanged          = false;   // 큐에 넣은 에니메이션이 변하는 순간만 세팅된다..

	CN3Chr m_Chr                      = {};      // 캐릭터 기본 객체...
	__TABLE_PLAYER_LOOKS* m_pLooksRef = nullptr; // 기본 참조 테이블 - 캐릭터에 관한 리소스 정보, 관절 위치, 사운드 파일등등..
	__TABLE_ITEM_BASIC* m_pItemPartBasics[PART_POS_COUNT] = {}; // 캐릭터에 붙은 무기들..
	__TABLE_ITEM_EXT* m_pItemPartExts[PART_POS_COUNT]     = {}; // 캐릭터에 붙은 무기들..
	__TABLE_ITEM_BASIC* m_pItemPlugBasics[PLUG_POS_COUNT] = {}; // 캐릭터에 붙은 무기들..
	__TABLE_ITEM_EXT* m_pItemPlugExts[PLUG_POS_COUNT]     = {}; // 캐릭터에 붙은 무기들..

	// ID
	CDFont* m_pClanFont                                   = nullptr;                  // clan or knights..이름 찍는데 쓰는 Font.. -.-;
	CDFont* m_pIDFont                                     = nullptr;                  // ID 찍는데 쓰는 Font.. -.-;
	CDFont* m_pInfoFont                                   = nullptr;                  // 파티원 모집등.. 기타 정보 표시..
	CDFont* m_pBalloonFont                                = nullptr;                  // 풍선말 표시...
	float m_fTimeBalloon                                  = 0.0f;                     // 풍선말 표시 시간..

	e_StateAction m_eState                                = PSA_BASIC;                // 행동 상태..
	e_StateAction m_eStatePrev                            = PSA_BASIC;                // 직전에 세팅된 행동 상태..
	e_StateAction m_eStateNext                            = PSA_BASIC;                // 직전에 세팅된 행동 상태..
	e_StateMove m_eStateMove                              = PSM_STOP;                 // 움직이는 상태..
	e_StateDying m_eStateDying                            = PSD_UNKNOWN;              // 죽을때 어떻게 죽는가..??
	float m_fTimeDying                                    = 0.0f;                     // 죽는 모션을 취하는 시간..

	__ColorValue m_cvDuration                             = { 1, 1, 1, 1 };           // 지속 컬러 값
	float m_fDurationColorTimeCur                         = 0.0f;                     // 현재 시간..
	float m_fDurationColorTime                            = 0.0f;                     // 지속시간..

	float m_fFlickeringFactor                             = 1.0f;                     // 깜박거림..
	float m_fFlickeringTime                               = 0.0f;                     // 깜박거림 시간..

	float m_fRotRadianPerSec                              = DegreesToRadians(270.0f); // 초당 회전 라디안값
	float m_fMoveSpeedPerSec = 0.0f; // 초당 움직임 값.. 이값은 기본값이고 상태(걷기, 달리기, 뒤로, 저주등) 에 따라 가감해서 쓴다..

	float m_fYawCur          = 0.0f; // 현재 회전값..
	float m_fYawToReach      = 0.0f; // 이 회전값을 목표로 Tick 에서 회전한다..

	float m_fYNext           = 0.0f; // 오브젝트 혹은 지형의 충돌 체크에 따른 높이값..
	float m_fGravityCur      = 0.0f; // 중력값..

	float m_fScaleToSet      = 1.0f; // 점차 스케일 값변화..
	float m_fScalePrev       = 1.0f;

public:
	CN3ShapeExtra* m_pShapeExtraRef = nullptr;     // 이 NPC 가 성문이나 집등 오브젝트의 형태이면 이 포인터를 세팅해서 쓴,다..

	int m_iMagicAni                 = 0;
	int m_iIDTarget                 = -1;          // 타겟 ID...
	int m_iDroppedItemID            = 0;           // 죽은후 떨어트린 아이템
	bool m_bGuardSuccess            = false;       // 방어에 성공했는지에 대한 플래그..
	bool m_bVisible                 = true;        // 보이는지??

	__InfoPlayerBase m_InfoBase     = {};          // 캐릭터 정보..
	__Vector3 m_vPosFromServer      = {};          // 최근에 서버에게서 받은 현재 위치..

	float m_fTimeAfterDeath         = 0.0f;        // 죽은지 지난시간 - 5초정도면 적당한가?? 그전에 공격을 받으면 바로 죽는다.

	float m_fAttackDelta            = 1.0f;        // 스킬이나 마법에 의해 변하는 공격 속도.. 1.0 이 기본이고 클수록 더 빨리 공격한다.
	float m_fMoveDelta              = 1.0f;        // 스킬이나 마법에 의해 변하는 이동 속도 1.0 이 기본이고 클수록 더 빨리 움직인다.
	__Vector3 m_vDirDying           = { 0, 0, 1 }; // 죽을때 밀리는 방향..

	//sound..
	bool m_bSoundAllSet             = false;
	CN3SndObj* m_pSnd_Attack_0      = nullptr;
	CN3SndObj* m_pSnd_Move          = nullptr;
	CN3SndObj* m_pSnd_Struck_0      = nullptr;
	CN3SndObj* m_pSnd_Breathe_0     = nullptr;
	CN3SndObj* m_pSnd_Blow          = nullptr;

	float m_fCastFreezeTime         = 0.0f;

	// 함수...
	//	By : Ecli666 ( On 2002-03-29 오후 1:32:12 )
	//
	CBitset m_bitset[SHADOW_SIZE]   = {}; // Used in Quake3.. ^^
	__VertexT1 m_pvVertex[4]        = {};
	uint16_t m_pIndex[6]            = {};
	__VertexT1 m_vTVertex[4]        = {};
	float m_fShaScale               = 1.0f;
	CN3Texture m_N3Tex              = {};
	static CN3SndObj* m_pSnd_MyMove;

	bool IsHostileTarget(const CPlayerBase* rhs) const;

	const __Matrix44 CalcShadowMtxBasicPlane(const __Vector3& vOffs);
	void CalcPart(CN3CPart* pPart, int nLOD, const __Vector3& vLP);
	void CalcPlug(CN3CPlugBase* pPlug, const __Matrix44* pmtxJoint, const __Vector3& vLP);

protected:
	void RenderShadow(float fSunAngle);
	//	~(By Ecli666 On 2002-03-29 오후 1:32:12 )

	virtual bool ProcessAttack(CPlayerBase* pTarget); // 공격 루틴 처리.. 타겟 포인터를 구하고 충돌체크까지 하며 충돌하면 참을 리턴..

	/// \brief applies any on-hit elemental effects associated with a weapon
	bool TryWeaponElementEffect(e_PlugPosition plugPosition, const CPlayerBase& target, const __Vector3& collisionPosition);

public:
	const __Matrix44* JointMatrixGet(int nJointIndex)
	{
		return m_Chr.MatrixGet(nJointIndex);
	}

	bool JointPosGet(int iJointIdx, __Vector3& vPos);

	e_PlayerType PlayerType() const
	{
		return m_ePlayerType;
	}

	e_Race Race() const
	{
		return m_InfoBase.eRace;
	}

	e_Nation Nation() const
	{
		return m_InfoBase.eNation;
	}

	virtual void SetSoundAndInitFont(uint32_t dwFontFlag = 0U);
	void SetSoundPlug(__TABLE_ITEM_BASIC* pItemBasic);
	void ReleaseSoundAndFont();
	void RegenerateCollisionMesh(); // 최대 최소값을 다시 찾고 충돌메시를 다시 만든다..

	// 행동 상태...
	e_StateAction State() const
	{
		return m_eState;
	}

	// 움직이는 상태
	e_StateMove StateMove() const
	{
		return m_eStateMove;
	}

	e_ItemClass ItemClass_RightHand() const
	{
		if (m_pItemPlugBasics[PLUG_POS_RIGHTHAND])
			return (e_ItemClass) (m_pItemPlugBasics[PLUG_POS_RIGHTHAND]->byClass); // 아이템 타입 - 오른손

		return ITEM_CLASS_UNKNOWN;
	}

	e_ItemClass ItemClass_LeftHand() const
	{
		if (m_pItemPlugBasics[PLUG_POS_LEFTHAND])
			return (e_ItemClass) (m_pItemPlugBasics[PLUG_POS_LEFTHAND]->byClass); // 아이템 타입 - 오른손

		return ITEM_CLASS_UNKNOWN;
	}

	e_Ani JudgeAnimationBreath();       // 숨쉬기 모션 판단하기.. 가진 아이템과 타겟이 있는냐에 따라 다른 에니메이션 인덱스를 리턴.
	e_Ani JudgeAnimationWalk();         // 걷기 모드판단하기.. 가진 아이템과 타겟이 있는냐에 따라 다른 에니메이션 인덱스를 리턴.
	e_Ani JudgeAnimationRun();          // 걷기 모드판단하기.. 가진 아이템과 타겟이 있는냐에 따라 다른 에니메이션 인덱스를 리턴.
	e_Ani JudgeAnimationWalkBackward(); // 걷기 모드판단하기.. 가진 아이템과 타겟이 있는냐에 따라 다른 에니메이션 인덱스를 리턴.
	e_Ani JudgeAnimationAttack();       // 공격 모션 판단하기.. 가진 아이템에 따라 다른 에니메이션 인덱스를 리턴.
	e_Ani JudgeAnimationStruck();       // 단지 NPC 와 유저를 구별해서 에니메이션 인덱스를 리턴
	e_Ani JudgeAnimationGuard();        // 막는 동작 판단하기.  단지 NPC 와 유저를 구별해서 에니메이션 인덱스를 리턴
	e_Ani JudgeAnimationDying();        // 단지 NPC 와 유저를 구별해서 에니메이션 인덱스를 리턴
	e_Ani JudgetAnimationSpellMagic();  // 마법 동작

	// 죽어있는지?
	bool IsDead() const
	{
		return (PSA_DYING == m_eState || PSA_DEATH == m_eState);
	}

	// 살아있는지?
	bool IsAlive() const
	{
		return !IsDead();
	}

	// 움직이고 있는지?
	bool IsMovingNow() const
	{
		return (PSM_WALK == m_eStateMove || PSM_RUN == m_eStateMove || PSM_WALK_BACKWARD == m_eStateMove);
	}

	void AnimationAdd(e_Ani eAni, bool bImmediately);

	void AnimationClear()
	{
		m_AnimationDeque.clear();
	}

	int AnimationCountRemain() const
	{
		return static_cast<int>(m_AnimationDeque.size()) + 1;
	}

	// 큐에 넣은 에니메이션이 변하는 순간만 세팅된다..
	bool IsAnimationChange() const
	{
		return m_bAnimationChanged;
	}

	// 행동 테이블에 따른 행동을 한다..
	bool Action(e_StateAction eState, bool bLooping, CPlayerBase* pTarget = nullptr, bool bForceSet = false);

	// 움직이기..
	bool ActionMove(e_StateMove eMove);

	// 죽는 방법 결정하기..
	void ActionDying(e_StateDying eSD, const __Vector3& vDir);

	// 회전값..
	float Yaw() const
	{
		return m_fYawCur;
	}

	float MoveSpeed() const
	{
		return m_fMoveSpeedPerSec;
	}

	const __Vector3& Position() const
	{
		return m_Chr.Pos();
	}

	void PositionSet(const __Vector3& vPos, bool bForcely)
	{
		m_Chr.PosSet(vPos);
		if (bForcely)
			m_fYNext = vPos.y;
	}

	float Distance(const __Vector3& vPos) const
	{
		return (m_Chr.Pos() - vPos).Magnitude();
	}

	__Vector3 Scale() const
	{
		return m_Chr.Scale();
	}

	void ScaleSet(float fScale)
	{
		m_fScaleToSet = m_fScalePrev = fScale;
		m_Chr.ScaleSet(fScale, fScale, fScale);
	}

	// 점차 스케일 변화..
	void ScaleSetGradually(float fScale)
	{
		m_fScaleToSet = fScale;
		m_fScalePrev  = m_Chr.Scale().y;
	}

	__Vector3 Direction() const;

	const __Quaternion& Rotation() const
	{
		return m_Chr.Rot();
	}

	void RotateTo(float fYaw, bool bImmediately);
	void RotateTo(CPlayerBase* pOther); // 이넘을 바라본다.
	float Height() const;
	float Radius() const;
	__Vector3 HeadPosition() const;     // 항상 변하는 머리위치를 가져온다..

	__Vector3 RootPosition() const
	{
		if (!m_Chr.m_MtxJoints.empty())
			return m_Chr.m_MtxJoints[0].Pos();
		return __Vector3(0, 0, 0);
	}

	int LODLevel() const
	{
		return m_Chr.m_nLOD;
	}

	__Vector3 Max() const;
	__Vector3 Min() const;
	__Vector3 Center() const;

	void DurationColorSet(const _D3DCOLORVALUE& color, float fDurationTime); // 컬러를 정하는 시간대로 유지하면서 원래색대로 돌아간다.
	void FlickerFactorSet(float fAlpha);

	void InfoStringSet(const std::string& szInfo, D3DCOLOR crFont);
	void BalloonStringSet(const std::string& szBalloon, D3DCOLOR crFont);
	void IDSet(int iID, const std::string& szID, D3DCOLOR crID);
	virtual void KnightsInfoSet(int iID, const std::string& szName, int iGrade, int iRank);

	// ID 는 Character 포인터의 이름으로 대신한다.
	const std::string& IDString() const
	{
		return m_InfoBase.szID;
	}

	int IDNumber() const
	{
		return m_InfoBase.iID;
	}

	CPlayerBase* TargetPointerCheck(bool bMustAlive);

	////////////////////
	// 충돌 체크 함수들...
	bool CheckCollisionByBox(const __Vector3& v0, const __Vector3& v1, __Vector3* pVCol, __Vector3* pVNormal);
	bool CheckCollisionToTargetByPlug(CPlayerBase* pTarget, int nPlug, __Vector3* pVCol);

	virtual bool InitChr(__TABLE_PLAYER_LOOKS* pTbl);

	virtual void InitHair()
	{
	}

	virtual void InitFace()
	{
	}

	CN3CPart* Part(e_PartPosition ePos)
	{
		return m_Chr.Part(ePos);
	}

	CN3CPlugBase* Plug(e_PlugPosition ePos)
	{
		return m_Chr.Plug(ePos);
	}

	virtual CN3CPart* PartSet(e_PartPosition ePos, const std::string& szFN, __TABLE_ITEM_BASIC* pItemBasic, __TABLE_ITEM_EXT* pItemExt);
	virtual CN3CPlugBase* PlugSet(e_PlugPosition ePos, const std::string& szFN, __TABLE_ITEM_BASIC* pItemBasic, __TABLE_ITEM_EXT* pItemExt);
	virtual void DurabilitySet(e_ItemSlot eSlot, int iDurability);

	void TickYaw();           // 회전값 처리.
	void TickAnimation();     // 에니메이션 처리.
	void TickDurationColor(); // 캐릭터 색깔 변화 처리.
	void TickSound();         // Sound 처리..

	virtual void Tick();
	virtual void Render(float fSunAngle);
#ifdef _DEBUG
	virtual void RenderCollisionMesh()
	{
		m_Chr.RenderCollisionMesh();
	}
#endif
	void RenderChrInRect(CN3Chr* pChr, const RECT& Rect); // Dino 추가, 지정된 사각형안에 캐릭터를 그린다.

	void Release() override;

	CPlayerBase();
	~CPlayerBase() override;

	int GetNPCOriginID() const
	{
		if (m_pLooksRef != nullptr)
			return m_pLooksRef->dwID;

		return -1;
	}
};

#endif // !defined(AFX_PlayerBase_H__B8B8986B_3635_462D_8C38_A052CA75B331__INCLUDED_)

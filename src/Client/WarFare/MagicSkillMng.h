#ifndef CLIENT_WARFARE_MAGICSKILLMNG_H
#define CLIENT_WARFARE_MAGICSKILLMNG_H

#pragma once

#include "GameDef.h"
#include "GameBase.h"
#include <map>

typedef std::map<int, uint32_t>::value_type stlmapVAL_INT_DWORD;
typedef std::multimap<int, uint32_t>::value_type stlmultimapVAL_INT_DWORD;

class CMagicSkillMng : public CGameBase
{
public:
	class CGameProcMain* m_pGameProcMain;

	CN3TableBase<struct __TABLE_UPC_SKILL_TYPE_1>* m_pTbl_Type_1;
	CN3TableBase<struct __TABLE_UPC_SKILL_TYPE_2>* m_pTbl_Type_2;
	CN3TableBase<struct __TABLE_UPC_SKILL_TYPE_3>* m_pTbl_Type_3;
	CN3TableBase<struct __TABLE_UPC_SKILL_TYPE_4>* m_pTbl_Type_4;
	//	CN3TableBase<struct __TABLE_UPC_SKILL_TYPE_6>*	m_pTbl_Type_6;
	CN3TableBase<struct __TABLE_UPC_SKILL_TYPE_7>* m_pTbl_Type_7;
	//	CN3TableBase<struct __TABLE_UPC_SKILL_TYPE_9>*	m_pTbl_Type_9;

	std::map<int, uint32_t> m_MySelf;

	int m_iTarget;
	__Vector3 m_vTargetPos;

	//나를 타겟으로 잡은 몬스터처리 관련..
	std::map<int, uint32_t> m_MobList; //	int m_iMobID; uint32_t m_dwMobMagicID;

	//related type4.....
	std::multimap<int, uint32_t> m_ListBuffTypeID;

	int m_iBuffType;

	float m_fAttackSpeed;
	float m_fSpeed;

	int m_iAC;
	int m_iAttack;
	int m_iMaxHP;
	int m_iStr;
	int m_iSta;
	int m_iDex;
	int m_iInt;
	int m_iMAP; // Magic Attack Point

	int m_iFireR;
	int m_iColdR;
	int m_iLightningR;
	int m_iMagicR;
	int m_iDeseaseR;
	int m_iPoisonR;

	//recast time...
	//float					m_fRecastTime;
	float m_fDelay;

	//related region magic...
	uint32_t m_dwRegionMagicState;     //0:마침..1:지역선택중..2:지역선택했음.
	__TABLE_UPC_SKILL m_dwRegionSkill; //선택된 지역스킬..

	//related non-casting action magic...
	uint32_t m_dwCastingStateNonAction; //0:아무것도 없는 평온한 상태 1: 캐스팅중
	float m_fCastTimeNonAction;
	uint32_t m_dwNonActionMagicID;
	int m_iNonActionMagicTarget;
	//float					m_fRecastTimeNonAction;

	//지역마법..
	int m_iMyRegionTargetFXID;

private:
	std::map<uint32_t, float> m_RecastTimes;          // Casting skills
	std::map<uint32_t, float> m_NonActionRecastTimes; // Instant skills, don't think this is actually needed
#ifdef _DEBUG
	float m_fMsgUpdateTimer = 0.0f;
#endif
	float m_fZonePointerRotRad;
	float m_fZonePointerRadius;
	float m_fZonePointerRadiusEffective;

protected:
	bool CheckValidCondition(int iTargetID, const __TABLE_UPC_SKILL* pSkill) const;
	bool CheckValidDistance(const __TABLE_UPC_SKILL* pSkill, const __Vector3& vTargetPos, float fTargetRadius) const;

	void InitType4();

	void StartSkillMagicAtTargetPacket(__TABLE_UPC_SKILL* pSkill, int16_t TargetID);
	void StartSkillMagicAtPosPacket(__TABLE_UPC_SKILL* pSkill, const __Vector3& vPos);

	void ProcessCasting();

	void MobCasting(__TABLE_UPC_SKILL* pSkill, int iSourceID);

	void FlyingType2(__TABLE_UPC_SKILL* pSkill, int iSourceID, int iTargetID, int16_t* pData);

	void EffectingType4(uint32_t dwMagicID);
	void EffectingType3(uint32_t dwMagicID);
	bool EffectingType1(uint32_t dwMagicID, int iSourceID, int iTargetID);

	int AddIdx(uint32_t dwMagicID, int iNum = 1); //return value is index...
	void RemoveIdx(int idx);

	void SuccessCast(__TABLE_UPC_SKILL* pSkill, CPlayerBase* pTarget);
	void FailCast(__TABLE_UPC_SKILL* pSkill);

	// returns true if the player is currently equipping the required item group
	bool HasEquippedRequiredItemGroup(const __TABLE_UPC_SKILL* pSkill) const;

	// returns true if the player is currently equipping a weapon, if applicable
	bool HasRequiredWeaponEquipped(const __TABLE_UPC_SKILL* pSkill) const;

	bool HasRequiredMana(const __TABLE_UPC_SKILL* pSkill) const;
	bool HasRequiredHealth(const __TABLE_UPC_SKILL* pSkill) const;
	bool HasRequiredExhaustItem(const __TABLE_UPC_SKILL* pSkill, bool* reportError = nullptr) const;

public:
	bool CheckValidSkillMagic(const __TABLE_UPC_SKILL* pSkill) const;
	void StunMySelf(__TABLE_UPC_SKILL_TYPE_3* pType3);
	void StopCastingByRatio();
	void ClearDurationalMagic();

	bool IsPositiveMagic(uint32_t dwMagicID);
	bool IsCasting();
	uint32_t GetMagicID(int idx);

	bool MsgSend_MagicProcess(int iTargetID, __TABLE_UPC_SKILL* pSkill);
	void SetSkillCooldown(__TABLE_UPC_SKILL* pSkill);
	void MsgRecv_Casting(Packet& pkt);
	void MsgRecv_Flying(Packet& pkt);
	void MsgRecv_Effecting(Packet& pkt);
	void MsgRecv_Fail(Packet& pkt);
	void MsgRecv_BuffType(Packet& pkt);

	void Init();
	void Tick();
	void UpdateZonePointerPositions();
	void CancelZonePointer();
	float GetCooldown(const __TABLE_UPC_SKILL* pSkill) const;

	CMagicSkillMng();
	CMagicSkillMng(CGameProcMain* pGameProcMain);
	~CMagicSkillMng() override;
};

#endif // CLIENT_WARFARE_MAGICSKILLMNG_H

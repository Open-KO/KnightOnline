#ifndef SERVER_AISERVER_USER_H
#define SERVER_AISERVER_USER_H

#pragma once

#include "MagicProcess.h"
#include "Extern.h"

#include <shared-server/STLMap.h>

namespace AIServer
{

class AIServerApp;
class CUser
{
public:
	AIServerApp* m_pMain;
	CMagicProcess m_MagicProcess;

	// 필요한 정보 변수만 선언,,,
	// 캐릭터 기본 속성
	char m_strUserID[MAX_ID_SIZE + 1]; // 캐릭터의 이름
	int m_iUserId;                     // User의 번호
	uint8_t m_bLive;                   // 죽었니? 살았니?

	float m_curx;                      // 현재 X 좌표
	float m_cury;                      // 현재 Y 좌표
	float m_curz;                      // 현재 Z 좌표
	float m_fWill_x;                   // 다음 X 좌표
	float m_fWill_y;                   // 다음 Y 좌표
	float m_fWill_z;                   // 다음 Z 좌표
	int16_t m_sSpeed;                  // 유저의 스피드
	uint8_t m_curZone;                 // 현재 존
	int16_t m_sZoneIndex;              // 현재 존의 index 번호..

	uint8_t m_bNation;                 // 소속국가
	uint8_t m_byLevel;                 // 레벨

	int16_t m_sHP;                     // HP
	int16_t m_sMP;                     // MP
	int16_t m_sSP;                     // SP
	int16_t m_sMaxHP;                  // HP
	int16_t m_sMaxMP;                  // MP
	int16_t m_sMaxSP;                  // SP

	int16_t m_sRegionX;                // 현재 영역 X 좌표
	int16_t m_sRegionZ;                // 현재 영역 Z 좌표
	int16_t m_sOldRegionX;             // 이전 영역 X 좌표
	int16_t m_sOldRegionZ;             // 이전 영역 Z 좌표

	uint8_t m_bResHp;                  // 회복량
	uint8_t m_bResMp;
	uint8_t m_bResSta;

	uint8_t m_byNowParty;            // 파티중이면 1, 부대중이면 2, 둘다 아니면 0
	uint8_t m_byPartyTotalMan;       // 파티 맺은 총 구성 인원수
	int16_t m_sPartyTotalLevel;      // 파티 맺은 사람의 총 레벨
	int16_t m_sPartyNumber;          // 파티 번호

	int16_t m_sHitDamage;            // Hit
	float m_fHitrate;                // 공격 민첩률
	float m_fAvoidrate;              // 방어 민첩률
	int16_t m_sAC;                   // 방어율
	int16_t m_sItemAC;               // 아이템 방어률

	int16_t m_sSurroundNpcNumber[8]; // Npc 다굴~

	uint8_t m_byIsOP;                // 운영자인지를 판단..
	long m_lUsed;   // 포인터 사용유무.. (1:사용중.. 접근 허락치 않음. 0:사용해도 무방)

	bool m_bLogOut; // Logout 중인가?

	// 2002.7.10 - Yookozuna
	uint8_t m_bMagicTypeLeftHand;    // The type of magic item in user's left hand
	uint8_t m_bMagicTypeRightHand;   // The type of magic item in user's right hand
	int16_t m_sMagicAmountLeftHand;  // The amount of magic item in user's left hand
	int16_t m_sMagicAmountRightHand; // The amount of magic item in user's left hand

public:
	int16_t GetMagicDamage(int damage, int16_t tid);
	void Initialize();
	void InitNpcAttack();
	void Attack(int sid, int tid);                                          // ATTACK
	void SetDamage(int damage, int tid);                                    // user damage
	void Dead(int tid, int nDamage);                                        // user dead
	void SetExp(int iNpcExp, int iLoyalty, int iLevel);                     // user exp
	void SetPartyExp(int iNpcExp, int iLoyalty, int iPartyLevel, int iMan); // user exp
	int16_t GetDamage(int tid, int magicid = 0);
	uint8_t GetHitRate(float rate);
	int IsSurroundCheck(float fX, float fY, float fZ, int NpcID);
	void HealMagic();
	void HealAreaCheck(int rx, int rz);

	void SendAttackSuccess(int tuid, uint8_t result, int16_t sDamage, int nHP = 0,
		uint8_t sAttack_type = 1);               // 공격 성공
	void SendMagicAttackResult(uint8_t opcode, int magicId, int targetId, int data1 = 0,
		int data2 = 0, int data3 = 0, int data4 = 0, int data5 = 0, int data6 = 0, int data7 = 0);
	void SendHP();                               // user의 HP
	void SendExp(int iExp, int iLoyalty, int tType = 1);
	void SendSystemMsg(const std::string_view msg, uint8_t type, int nWho);
	void SendAll(const char* pBuf, int nLength); // game server로 패킷 전송...
	bool IsOpIDCheck(const char* szName);

	CUser();
	virtual ~CUser();
};

} // namespace AIServer

#endif // SERVER_AISERVER_USER_H

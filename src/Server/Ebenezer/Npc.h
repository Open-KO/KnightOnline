#ifndef SERVER_EBENEZER_NPC_H
#define SERVER_EBENEZER_NPC_H

#pragma once

#include "Define.h"

namespace Ebenezer
{

class EbenezerApp;
class CNpc
{
public:
	EbenezerApp* m_pMain                  = nullptr;

	/// \brief Serial number of NPC (server-side) || NPC (서버상의)일련번호
	int16_t m_sNid                        = -1;

	/// \brief Reference number for the NPC table || NPC 테이블 참조번호
	int16_t m_sSid                        = -1;

	int16_t m_sCurZone                    = 0;  // Current Zone;
	int16_t m_sZoneIndex                  = -1; // NPC 가 존재하고 있는 존의 인덱스
	float m_fCurX                         = 0;  // Current X Pos;
	float m_fCurY                         = 0;  // Current Y Pos;
	float m_fCurZ                         = 0;  // Current Z Pos;
	int16_t m_sPid                        = 0;  // MONSTER(NPC) Picture ID
	int16_t m_sSize                       = 0;  // MONSTER(NPC) Size
	int m_iWeapon_1                       = 0;
	int m_iWeapon_2                       = 0;
	char m_strName[MAX_NPC_NAME_SIZE + 1] = {}; // MONSTER(NPC) Name
	int m_iMaxHP                          = 0;  // 최대 HP
	int m_iHP                             = 0;  // 현재 HP
	uint8_t m_byState                     = 0;  // 몬스터 (NPC) 상태
	uint8_t m_byGroup                     = 0;  // 소속 집단
	uint8_t m_byLevel                     = 0;  // 레벨
	uint8_t m_tNpcType                    = 0;  // NPC Type
												// 0 : Normal Monster
												// 1 : NPC
												// 2 : 각 입구,출구 NPC
												// 3 : 경비병
	int m_iSellingGroup                   = 0;  // ItemGroup

	int16_t m_sRegion_X                   = -1; // region x position
	int16_t m_sRegion_Z                   = -1; // region z position
	uint8_t m_NpcState                    = 0;  // NPC의 상태 - 살았다, 죽었다, 서있다 등등...
	uint8_t m_byGateOpen                  = 0;  // Gate Npc Status -> 1 : open 0 : close
	int16_t m_sHitRate                    = 0;  // 공격 성공률
	uint8_t m_byObjectType                = 0;  // 보통은 0, object타입(성문, 레버)은 1
	uint8_t m_byDirection                 = 0;  // NPC가 보고 있는 방향

	int16_t m_byEvent                     = -1; // This is for the quest.
	uint8_t m_byTrapNumber                = 0;

public:
	CNpc();
	virtual ~CNpc();

	void Initialize();
	void MoveResult(float xpos, float ypos, float zpos, float speed);
	void NpcInOut(uint8_t Type, float fx, float fz, float fy);
	void RegisterRegion();
	void RemoveRegion(int del_x, int del_z);
	void InsertRegion(int del_x, int del_z);
	int GetRegionNpcList(int region_x, int region_z, char* buff, int& t_count);
	void GetNpcInfo(char* buff, int& buff_index);

	uint8_t GetState() const
	{
		return m_byState;
	}
};

} // namespace Ebenezer

#endif // SERVER_EBENEZER_NPC_H

#include "pch.h"
#include "MagicProcess.h"
#include "Npc.h"
#include "NpcThread.h"
#include "Region.h"
#include "AIServerApp.h"
#include "User.h"

#include <spdlog/spdlog.h>

namespace AIServer
{

extern std::mutex g_region_mutex;

CMagicProcess::CMagicProcess()
{
	m_pMain    = AIServerApp::instance();
	m_pSrcUser = nullptr;
}

CMagicProcess::~CMagicProcess()
{
}

void CMagicProcess::MagicPacket(char* pBuf)
{
	int index = 0, magicid = 0, sid = -1, tid = -1, TotalDex = 0, righthand_damage = 0;
	int data1 = 0, data2 = 0, data3 = 0, data4 = 0, data5 = 0, data6 = 0, result = 1;
	model::Magic* pTable = nullptr;

	sid                  = m_pSrcUser->m_iUserId;

	uint8_t command      = GetByte(pBuf, index);  // Get the magic status.
	tid                  = GetShort(pBuf, index); // Get ID of target.
	magicid              = GetDWORD(pBuf, index); // Get ID of magic.
	data1                = GetShort(pBuf, index);
	data2                = GetShort(pBuf, index);
	data3                = GetShort(pBuf, index);
	data4                = GetShort(pBuf, index);
	data5                = GetShort(pBuf, index);
	data6                = GetShort(pBuf, index);
	TotalDex             = GetShort(pBuf, index);
	righthand_damage     = GetShort(pBuf, index);

	//TRACE(_T("MagicPacket - command=%d, tid=%d, magicid=%d\n"), command, tid, magicid);

	// If magic was successful.......
	pTable               = IsAvailable(magicid, tid, command);
	if (pTable == nullptr)
		return;

	// Is target another player?
	if (command == MAGIC_EFFECTING)
	{
		switch (pTable->Type1)
		{
			case 0:
				/* do nothing */
				break;

			case 1:
				result = ExecuteType1(pTable->ID, tid, data1, data2, data3, 1);
				break;

			case 2:
				result = ExecuteType2(pTable->ID, tid, data1, data2, data3);
				break;

			case 3:
				ExecuteType3(pTable->ID, tid, data1, data2, data3, pTable->Moral, TotalDex,
					righthand_damage);
				break;

			case 4:
				ExecuteType4(pTable->ID, sid, tid, data1, data2, data3, pTable->Moral);
				break;

			case 5:
				ExecuteType5(pTable->ID);
				break;

			case 6:
				ExecuteType6(pTable->ID);
				break;

			case 7:
				ExecuteType7(pTable->ID, tid, data1, data2, data3, pTable->Moral);
				break;

			case 8:
				ExecuteType8(pTable->ID);
				break;

			case 9:
				ExecuteType9(pTable->ID);
				break;

			case 10:
				ExecuteType10(pTable->ID);
				break;

			default:
				spdlog::error("MagicProcess::MagicPacket: Unhandled type1 type - ID={} Type1={}",
					pTable->ID, pTable->Type1);
				break;
		}

		if (result != 0)
		{
			switch (pTable->Type2)
			{
				case 0:
					/* do nothing */
					break;

				case 1:
					ExecuteType1(pTable->ID, tid, data4, data5, data6, 2);
					break;

				case 2:
					ExecuteType2(pTable->ID, tid, data1, data2, data3);
					break;

				case 3:
					ExecuteType3(pTable->ID, tid, data1, data2, data3, pTable->Moral, TotalDex,
						righthand_damage);
					break;

				case 4:
					ExecuteType4(pTable->ID, sid, tid, data1, data2, data3, pTable->Moral);
					break;

				case 5:
					ExecuteType5(pTable->ID);
					break;

				case 6:
					ExecuteType6(pTable->ID);
					break;

				case 7:
					ExecuteType7(pTable->ID, tid, data1, data2, data3, pTable->Moral);
					break;

				case 8:
					ExecuteType8(pTable->ID);
					break;

				case 9:
					ExecuteType9(pTable->ID);
					break;

				case 10:
					ExecuteType10(pTable->ID);
					break;

				default:
					spdlog::error(
						"MagicProcess::MagicPacket: Unhandled type2 type - ID={} Type2={}",
						pTable->ID, pTable->Type2);
					break;
			}
		}
	}
}

model::Magic* CMagicProcess::IsAvailable(int magicid, int /*tid*/, uint8_t /*type*/)
{
	model::Magic* pTable = nullptr;

	if (m_pSrcUser == nullptr)
		return nullptr;

	// Get main magic table.
	pTable = m_pMain->_magicTableMap.GetData(magicid);
	if (pTable == nullptr)
		return nullptr;

	return pTable; // Magic was successful!
}

// Applied to an attack skill using a weapon.
uint8_t CMagicProcess::ExecuteType1(
	int magicid, int tid, int data1, int /*data2*/, int data3, uint8_t /*sequence*/)
{
	model::Magic* pMagic = nullptr;
	int damage = 0, result = 1; // Variable initialization. result == 1 : success, 0 : fail

	pMagic = m_pMain->_magicTableMap.GetData(magicid); // Get main magic table.
	if (pMagic == nullptr)
		return 0;

	damage     = m_pSrcUser->GetDamage(tid, magicid); // Get damage points of enemy.
													  // 	if(damage <= 0)	damage = 1;
	//TRACE(_T("magictype1 ,, magicid=%d, damage=%d\n"), magicid, damage);

	//	if (damage > 0) {
	CNpc* pNpc = m_pMain->_npcMap.GetData(tid - NPC_BAND);
	if (pNpc != nullptr && pNpc->m_NpcState != NPC_DEAD && pNpc->m_iHP > 0)
	{
		if (!pNpc->SetDamage(
				magicid, damage, m_pSrcUser->m_strUserID, m_pSrcUser->m_iUserId + USER_BAND))
		{
			// Npc가 죽은 경우,,
			pNpc->SendExpToUserList(); // 경험치 분배!!
			pNpc->SendDead();
			//m_pSrcUser->SendAttackSuccess(tid, MAGIC_ATTACK_TARGET_DEAD, 0, pNpc->m_iHP);
			m_pSrcUser->SendAttackSuccess(tid, ATTACK_TARGET_DEAD, damage, pNpc->m_iHP);
		}
		else
		{
			// 공격 결과 전송
			m_pSrcUser->SendAttackSuccess(tid, ATTACK_SUCCESS, damage, pNpc->m_iHP);
		}
		//	}
		//	else
		//		result = 0;
	}
	else // if (pNpc == nullptr || pNpc->m_NpcState == NPC_DEAD || pNpc->m_iHP <= 0)
	{
		result = 0;
	}

	if (pMagic->Type2 == 0 || pMagic->Type2 == 1)
	{
		m_pSrcUser->SendMagicAttackResult(MAGIC_EFFECTING, magicid, tid, data1, result, data3, 0, 0,
			0, damage == 0 ? SKILLMAGIC_FAIL_ATTACKZERO : 0);
	}

	return result;
}

uint8_t CMagicProcess::ExecuteType2(int magicid, int tid, int data1, int /*data2*/, int data3)
{
	int damage = 0, result = 1; // Variable initialization. result == 1 : success, 0 : fail

	damage = m_pSrcUser->GetDamage(tid, magicid); // Get damage points of enemy.
												  //	if(damage <= 0)	damage = 1;
	//TRACE(_T("magictype2 ,, magicid=%d, damage=%d\n"), magicid, damage);

	if (damage > 0)
	{
		CNpc* pNpc = m_pMain->_npcMap.GetData(tid - NPC_BAND);
		if (pNpc != nullptr && pNpc->m_NpcState != NPC_DEAD && pNpc->m_iHP != 0)
		{
			if (!pNpc->SetDamage(
					magicid, damage, m_pSrcUser->m_strUserID, m_pSrcUser->m_iUserId + USER_BAND))
			{
				m_pSrcUser->SendMagicAttackResult(MAGIC_EFFECTING, magicid, tid, data1, result,
					data3, 0, 0, 0, damage == 0 ? SKILLMAGIC_FAIL_ATTACKZERO : 0);

				// Npc가 죽은 경우,,
				pNpc->SendExpToUserList(); // 경험치 분배!!
				pNpc->SendDead();
				m_pSrcUser->SendAttackSuccess(tid, MAGIC_ATTACK_TARGET_DEAD, damage, pNpc->m_iHP);
				//m_pSrcUser->SendAttackSuccess(tid, ATTACK_TARGET_DEAD, damage, pNpc->m_iHP);

				return result;
			}
			else
			{
				// 공격 결과 전송
				m_pSrcUser->SendAttackSuccess(tid, ATTACK_SUCCESS, damage, pNpc->m_iHP);
			}
		}
		else // if (pNpc == nullptr || pNpc->m_NpcState == NPC_DEAD || pNpc->m_iHP == 0)
		{
			result = 0;
		}
	}

	m_pSrcUser->SendMagicAttackResult(MAGIC_EFFECTING, magicid, tid, data1, result, data3, 0, 0, 0,
		damage == 0 ? SKILLMAGIC_FAIL_ATTACKZERO : 0);

	return result;
}

// Applied when a magical attack, healing, and mana restoration is done.
void CMagicProcess::ExecuteType3(int magicid, int tid, int data1, int data2, int data3, int moral,
	int dexpoint, int righthand_damage)
{
	int damage = 0, result = 1, attack_type = 0;
	model::MagicType3* pType = nullptr;
	CNpc* pNpc               = nullptr;                                  // Pointer initialization!

	model::Magic* pMagic     = m_pMain->_magicTableMap.GetData(magicid); // Get main magic table.
	if (pMagic == nullptr)
		return;

	// 지역 공격
	if (tid == -1)
	{
		AreaAttack(3, magicid, moral, data1, data2, data3, dexpoint, righthand_damage);
		return;
	}

	pNpc = m_pMain->_npcMap.GetData(tid - NPC_BAND);
	if (pNpc == nullptr || pNpc->m_NpcState == NPC_DEAD || pNpc->m_iHP == 0)
	{
		result = 0;
		m_pSrcUser->SendMagicAttackResult(
			MAGIC_EFFECTING, magicid, tid, data1, result, data3, moral);
		return;
	}

	pType = m_pMain->_magicType3TableMap.GetData(magicid); // Get magic skill table type 3.
	if (pType == nullptr)
		return;

	if (pType->FirstDamage < 0 && pType->DirectType == 1 && magicid < 400000)
		damage = GetMagicDamage(
			tid, pType->FirstDamage, pType->Attribute, dexpoint, righthand_damage);
	else
		damage = pType->FirstDamage;

	//TRACE(_T("magictype3 ,, magicid=%d, damage=%d\n"), magicid, damage);

	// Non-Durational Spells.
	if (pType->Duration == 0)
	{
		// Health Point related !
		if (pType->DirectType == 1)
		{
			//damage = pType->FirstDamage;     // Reduce target health point
			//			if(damage >= 0)	{
			if (damage > 0)
			{
				result = pNpc->SetHMagicDamage(damage);
			}
			else
			{
				damage = abs(damage);

				// 기절시키는 마법이라면.....
				if (pType->Attribute == 3)
					attack_type = 3;
				else
					attack_type = magicid;

				if (!pNpc->SetDamage(attack_type, damage, m_pSrcUser->m_strUserID,
						m_pSrcUser->m_iUserId + USER_BAND))
				{
					// Npc가 죽은 경우,,
					pNpc->SendExpToUserList(); // 경험치 분배!!
					pNpc->SendDead();
					m_pSrcUser->SendAttackSuccess(
						tid, MAGIC_ATTACK_TARGET_DEAD, damage, pNpc->m_iHP, MAGIC_ATTACK);
				}
				else
				{
					// 공격 결과 전송
					m_pSrcUser->SendAttackSuccess(
						tid, ATTACK_SUCCESS, damage, pNpc->m_iHP, MAGIC_ATTACK);
				}
			}
		}
		// Magic or Skill Point related !
		else if (pType->DirectType == 2 || pType->DirectType == 3)
			pNpc->MSpChange(
				pType->DirectType, pType->FirstDamage); // Change the SP or the MP of the target.
	}
	// Durational Spells! Remember, durational spells only involve HPs.
	else if (pType->Duration != 0)
	{
		if (damage >= 0)
		{
		}
		else
		{
			damage = abs(damage);

			// 기절시키는 마법이라면.....
			if (pType->Attribute == 3)
				attack_type = 3;
			else
				attack_type = magicid;

			if (!pNpc->SetDamage(attack_type, damage, m_pSrcUser->m_strUserID,
					m_pSrcUser->m_iUserId + USER_BAND))
			{
				// Npc가 죽은 경우,,
				pNpc->SendExpToUserList(); // 경험치 분배!!
				pNpc->SendDead();
				m_pSrcUser->SendAttackSuccess(tid, MAGIC_ATTACK_TARGET_DEAD, damage, pNpc->m_iHP);
			}
			else
			{
				// 공격 결과 전송
				m_pSrcUser->SendAttackSuccess(tid, ATTACK_SUCCESS, damage, pNpc->m_iHP);
			}
		}

		damage = GetMagicDamage(
			tid, pType->TimeDamage, pType->Attribute, dexpoint, righthand_damage);

		// The duration magic routine.
		for (int i = 0; i < MAX_MAGIC_TYPE3; i++)
		{
			if (pNpc->m_MagicType3[i].sHPAttackUserID == -1
				&& pNpc->m_MagicType3[i].byHPDuration == 0)
			{
				pNpc->m_MagicType3[i].sHPAttackUserID = m_pSrcUser->m_iUserId;
				pNpc->m_MagicType3[i].fStartTime      = TimeGet();
				pNpc->m_MagicType3[i].byHPDuration    = pType->Duration;
				pNpc->m_MagicType3[i].byHPInterval    = 2;
				pNpc->m_MagicType3[i].sHPAmount       = damage / (pType->Duration / 2);
				break;
			}
		}
	}

	m_pSrcUser->SendMagicAttackResult(MAGIC_EFFECTING, magicid, tid, data1, result, data3, moral);
}

void CMagicProcess::ExecuteType4(
	int magicid, int sid, int tid, int data1, int data2, int data3, int moral)
{
	model::MagicType4* pType = nullptr;
	CNpc* pNpc               = nullptr; // Pointer initialization!
	int result               = 1;       // Variable initialization. result == 1 : success, 0 : fail

	// 지역 공격
	if (tid == -1)
	{
		result = AreaAttack(4, magicid, moral, data1, data2, data3, 0, 0);
		if (result == 0)
			SendMagicAttackResult(MAGIC_FAIL, magicid, sid, tid);

		return;
	}

	pNpc = m_pMain->_npcMap.GetData(tid - NPC_BAND);
	if (pNpc == nullptr || pNpc->m_NpcState == NPC_DEAD || pNpc->m_iHP == 0)
	{
		SendMagicAttackResult(MAGIC_FAIL, magicid, sid, tid);
		return;
	}

	pType = m_pMain->_magicType4TableMap.GetData(magicid); // Get magic skill table type 4.
	if (pType == nullptr)
		return;

	//TRACE(_T("magictype4 ,, magicid=%d\n"), magicid);

	// Depending on which buff-type it is.....
	switch (pType->BuffType)
	{
		case 1: // HP 올리기..
		case 2: // 방어력 올리기..
		case 4: // 공격력 올리기..
		case 5: // 공격 속도 올리기..
		case 7: // 능력치 올리기...
		case 8: // 저항력 올리기...
		case 9: // 공격 성공율 및 회피 성공율 올리기..
			/* nothing to handle here */
			break;

		case 6: // 이동 속도 올리기..
			pNpc->m_MagicType4[pType->BuffType - 1].byAmount      = pType->Speed;
			pNpc->m_MagicType4[pType->BuffType - 1].sDurationTime = pType->Duration;
			pNpc->m_MagicType4[pType->BuffType - 1].fStartTime    = TimeGet();
			pNpc->m_fSpeed_1 = pNpc->m_fOldSpeed_1 * (pType->Speed / 100.0f);
			pNpc->m_fSpeed_2 = pNpc->m_fOldSpeed_2 * (pType->Speed / 100.0f);
			//TRACE(_T("executeType4 ,, speed1=%.2f, %.2f,, type=%d, cur=%.2f, %.2f\n"), pNpc->m_fOldSpeed_1, pNpc->m_fOldSpeed_2, pType->bSpeed, pNpc->m_fSpeed_1, pNpc->m_fSpeed_2);
			break;

		default:
			SendMagicAttackResult(MAGIC_FAIL, magicid, sid, tid);
			return;
	}

	SendMagicAttackResult(MAGIC_EFFECTING, magicid, sid, tid, data1, result, data3);
}

void CMagicProcess::ExecuteType5(int /*magicid*/)
{
}

void CMagicProcess::ExecuteType6(int /*magicid*/)
{
}

void CMagicProcess::ExecuteType7(int magicid, int tid, int data1, int data2, int data3, int moral)
{
	int damage = 0, result = 1, attack_type = 0;
	model::MagicType7* pType = nullptr;
	CNpc* pNpc               = nullptr;

	model::Magic* pMagic     = m_pMain->_magicTableMap.GetData(magicid);
	if (pMagic == nullptr)
		return;

	// AoE skills, AoE sleep needs to be implemented
	if (tid == -1)
	{
		AreaAttack(7, magicid, moral, data1, data2, data3, 0, 0);
		return;
	}

	pNpc = m_pMain->_npcMap.GetData(tid - NPC_BAND);
	if (pNpc == nullptr || pNpc->m_NpcState == NPC_DEAD || pNpc->m_iHP == 0)
	{
		result = 0;
		m_pSrcUser->SendMagicAttackResult(
			MAGIC_EFFECTING, magicid, tid, data1, result, data3, moral);
		return;
	}

	pType = m_pMain->_magicType7TableMap.GetData(magicid); // Get magic skill table type 3.
	if (pType == nullptr)
		return;

	damage = pType->Damage;
	// attacking ex: binding
	if (damage > 0)
	{
		attack_type = magicid;
		if (!pNpc->SetDamage(
				attack_type, damage, m_pSrcUser->m_strUserID, m_pSrcUser->m_iUserId + USER_BAND))
		{
			pNpc->SendExpToUserList();
			pNpc->SendDead();
			m_pSrcUser->SendAttackSuccess(tid, MAGIC_ATTACK_TARGET_DEAD, damage, pNpc->m_iHP);
		}
		else
		{
			m_pSrcUser->SendAttackSuccess(tid, ATTACK_SUCCESS, damage, pNpc->m_iHP);
		}
	}
	// sleeping
	else
	{
		// note: sleep works, but duration does not (infinite).
		pNpc->m_NpcState = NPC_SLEEPING;
		pNpc->m_Delay    = pType->Duration;
	}

	m_pSrcUser->SendMagicAttackResult(MAGIC_EFFECTING, magicid, tid, data1, result, data3, moral);
}

void CMagicProcess::ExecuteType8(int /*magicid*/)
{
}

void CMagicProcess::ExecuteType9(int /*magicid*/)
{
}

void CMagicProcess::ExecuteType10(int /*magicid*/)
{
}

void CMagicProcess::SendMagicAttackResult(uint8_t opcode, int magicId, int sourceId, int targetId,
	int data1 /*= 0*/, int data2 /*= 0*/, int data3 /*= 0*/, int data4 /*= 0*/, int data5 /*= 0*/,
	int data6 /*= 0*/, int data7 /*= 0*/)
{
	int sendIndex = 0;
	char sendBuffer[128] {};
	SetByte(sendBuffer, AG_MAGIC_ATTACK_RESULT, sendIndex);
	SetByte(sendBuffer, opcode, sendIndex);
	SetDWORD(sendBuffer, magicId, sendIndex);
	SetShort(sendBuffer, sourceId, sendIndex);
	SetShort(sendBuffer, targetId, sendIndex);
	if (opcode == MAGIC_CASTING)
		SetShort(sendBuffer, SKILLMAGIC_FAIL_CASTING, sendIndex);
	else
		SetShort(sendBuffer, data1, sendIndex);
	SetShort(sendBuffer, data2, sendIndex);
	SetShort(sendBuffer, data3, sendIndex);
	SetShort(sendBuffer, data4, sendIndex);
	SetShort(sendBuffer, data5, sendIndex);
	SetShort(sendBuffer, data6, sendIndex);
	if (opcode == MAGIC_EFFECTING)
		SetShort(sendBuffer, data7, sendIndex);
	m_pSrcUser->SendAll(sendBuffer, sendIndex);
}

int16_t CMagicProcess::GetMagicDamage(
	int tid, int total_hit, int attribute, int dexpoint, int righthand_damage)
{
	int16_t damage = 0;
	int random = 0, total_r = 0;
	uint8_t result = 0;
	bool bSign     = true; // false이면 -, true이면 +

	// Check if target id is valid.
	if (tid < NPC_BAND || tid > INVALID_BAND)
		return 0;

	CNpc* pNpc = m_pMain->_npcMap.GetData(tid - NPC_BAND);
	if (pNpc == nullptr || pNpc->m_NpcState == NPC_DEAD || pNpc->m_iHP == 0)
		return 0;

	if (pNpc->m_tNpcType == NPC_ARTIFACT || pNpc->m_tNpcType == NPC_PHOENIX_GATE
		|| pNpc->m_tNpcType == NPC_GATE_LEVER || pNpc->m_tNpcType == NPC_SPECIAL_GATE)
		return 0;

	//result = m_pSrcUser->GetHitRate(m_pSrcUser->m_fHitrate / pNpc->m_sEvadeRate );
	result = SUCCESS;

	// In case of SUCCESS (and SUCCESS only!) ....
	if (result != FAIL)
	{
		switch (attribute)
		{
			case NONE_R:
				total_r = 0;
				break;

			case FIRE_R:
				total_r = pNpc->m_sFireR;
				break;

			case COLD_R:
				total_r = pNpc->m_sColdR;
				break;

			case LIGHTNING_R:
				total_r = pNpc->m_sLightningR;
				break;

			case MAGIC_R:
				total_r = pNpc->m_sMagicR;
				break;

			case DISEASE_R:
				total_r = pNpc->m_sDiseaseR;
				break;

			case POISON_R:
				total_r = pNpc->m_sPoisonR;
				break;

			default:
				/* unhandled */
				break;
		}

		total_hit = (total_hit * (dexpoint + 20)) / 170;

		if (total_hit < 0)
		{
			total_hit = abs(total_hit);
			bSign     = false;
		}

		damage = static_cast<int16_t>(total_hit - (0.7f * total_hit * total_r / 200));
		random = myrand(0, damage);
		damage = static_cast<int16_t>(
			(0.7f * (total_hit - (0.9f * total_hit * total_r / 200))) + 0.2f * random);
		//		damage = damage + (3 * righthand_damage);
		damage = damage + righthand_damage;
	}
	else
	{
		damage = 0;
	}

	if (!bSign && damage != 0)
		damage = -damage;

	//return 1;
	return damage;
}

int16_t CMagicProcess::AreaAttack(int magictype, int magicid, int moral, int data1, int data2,
	int data3, int dexpoint, int righthand_damage)
{
	model::MagicType3* pType3 = nullptr;
	model::MagicType4* pType4 = nullptr;
	model::MagicType7* pType7 = nullptr;
	int radius                = 0;

	if (magictype == 3)
	{
		pType3 = m_pMain->_magicType3TableMap.GetData(magicid);
		if (pType3 == nullptr)
		{
			spdlog::error(
				"MagicProcess::AreaAttack: No MAGIC_TYPE3 definition [magicId={}]", magicid);
			return 0;
		}

		radius = pType3->Radius;
	}
	else if (magictype == 4)
	{
		pType4 = m_pMain->_magicType4TableMap.GetData(magicid); // Get magic skill table type 3.
		if (pType4 == nullptr)
		{
			spdlog::error(
				"MagicProcess::AreaAttack: No MAGIC_TYPE4 definition [magicId={}]", magicid);
			return 0;
		}

		radius = pType4->Radius;
	}
	else if (magictype == 7)
	{
		pType7 = m_pMain->_magicType7TableMap.GetData(magicid);
		if (pType7 == nullptr)
		{
			spdlog::error(
				"MagicProcess::AreaAttack: No MAGIC_TYPE7 definition [magicId={}]", magicid);
			return 0;
		}

		radius = pType7->Radius;
	}

	if (radius <= 0)
	{
		spdlog::error(
			"MagicProcess::AreaAttack: Invalid radius [magicId={} radius={}", magicid, radius);
		return 0;
	}

	int region_x = data1 / VIEW_DIST;
	int region_z = data3 / VIEW_DIST;

	MAP* pMap    = m_pMain->GetMapByIndex(m_pSrcUser->m_sZoneIndex);
	if (pMap == nullptr)
	{
		spdlog::error("MagicProcess::AreaAttack: No map found: [charId={} zoneIndex={}]",
			m_pSrcUser->m_strUserID, m_pSrcUser->m_sZoneIndex);
		return 0;
	}

	int max_xx = pMap->m_sizeRegion.cx;
	int max_zz = pMap->m_sizeRegion.cy;

	int min_x  = region_x - 1;
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
			AreaAttackDamage(magictype, min_x + i, min_z + j, magicid, moral, data1, data2, data3,
				dexpoint, righthand_damage);
	}

	//damage = GetMagicDamage(tid, pType->FirstDamage, pType->bAttribute);

	return 1;
}

void CMagicProcess::AreaAttackDamage(int magictype, int rx, int rz, int magicid, int moral,
	int data1, int /*data2*/, int data3, int dexpoint, int righthand_damage)
{
	MAP* pMap = m_pMain->GetMapByIndex(m_pSrcUser->m_sZoneIndex);
	if (pMap == nullptr)
	{
		spdlog::error("MagicProcess::AreaAttackDamage: No map found: [charId={} zoneIndex={}]",
			m_pSrcUser->m_strUserID, m_pSrcUser->m_sZoneIndex);
		return;
	}

	// 자신의 region에 있는 UserArray을 먼저 검색하여,, 가까운 거리에 유저가 있는지를 판단..
	if (rx < 0 || rz < 0 || rx > pMap->GetXRegionMax() || rz > pMap->GetZRegionMax())
	{
		spdlog::error(
			"MagicProcess::AreaAttackDamage: region out of bounds [userId={} charId={} x={} z={}]",
			m_pSrcUser->m_iUserId, m_pSrcUser->m_strUserID, rx, rz);
		return;
	}

	model::MagicType3* pType3 = nullptr;
	model::MagicType4* pType4 = nullptr;
	model::MagicType7* pType7 = nullptr;
	model::Magic* pMagic      = nullptr;

	int damage = 0, target_damage = 0, attribute = 0;
	float fRadius = 0;

	pMagic        = m_pMain->_magicTableMap.GetData(magicid);
	if (pMagic == nullptr)
	{
		spdlog::error("MagicProcess::AreaAttackDamage: No MAGIC definition [magicId={}]", magicid);
		return;
	}

	if (magictype == 3)
	{
		pType3 = m_pMain->_magicType3TableMap.GetData(magicid);
		if (pType3 == nullptr)
		{
			spdlog::error(
				"MagicProcess::AreaAttackDamage: No MAGIC_TYPE3 definition [magicId={}]", magicid);
			return;
		}

		target_damage = pType3->FirstDamage;
		attribute     = pType3->Attribute;
		fRadius       = (float) pType3->Radius;
	}
	else if (magictype == 4)
	{
		pType4 = m_pMain->_magicType4TableMap.GetData(magicid);
		if (pType4 == nullptr)
		{
			spdlog::error(
				"MagicProcess::AreaAttackDamage: No MAGIC_TYPE4 definition [magicId={}]", magicid);
			return;
		}

		fRadius = (float) pType4->Radius;
	}
	else if (magictype == 7)
	{
		pType7 = m_pMain->_magicType7TableMap.GetData(magicid);
		if (pType7 == nullptr)
		{
			spdlog::error(
				"MagicProcess::AreaAttackDamage: No MAGIC_TYPE7 definition [magicId={}]", magicid);
			return;
		}

		target_damage = pType7->Damage;
		fRadius       = static_cast<float>(pType7->Radius);
	}

	if (fRadius <= 0)
	{
		spdlog::error("MagicProcess::AreaAttackDamage: invalid radius [magicId={} radius={}]",
			magicid, fRadius);
		return;
	}

	__Vector3 vStart, vEnd;
	float fDis    = 0.0f;
	int sendIndex = 0, result = 1, attack_type = 0;
	char sendBuffer[256] {};

	std::vector<int> npcIds;

	vStart.Set(static_cast<float>(data1), 0.0f, static_cast<float>(data3));

	{
		std::lock_guard<std::mutex> lock(g_region_mutex);
		const auto& regionNpcArray = pMap->m_ppRegion[rx][rz].m_RegionNpcArray.m_UserTypeMap;
		if (regionNpcArray.empty())
			return;

		npcIds.reserve(regionNpcArray.size());
		for (const auto& [npcId, _] : regionNpcArray)
			npcIds.push_back(npcId);
	}

	for (int npcId : npcIds)
	{
		if (npcId < NPC_BAND)
			continue;

		CNpc* pNpc = m_pMain->_npcMap.GetData(npcId - NPC_BAND);
		if (pNpc == nullptr || pNpc->m_NpcState == NPC_DEAD)
			continue;

		if (m_pSrcUser->m_bNation == pNpc->m_byGroup)
			continue;

		vEnd.Set(pNpc->m_fCurX, pNpc->m_fCurY, pNpc->m_fCurZ);
		fDis = pNpc->GetDistance(vStart, vEnd);

		// NPC가 반경안에 있을 경우...
		if (fDis > fRadius)
			continue;

		// 타잎 3일 경우...
		if (magictype == 3)
		{
			damage = GetMagicDamage(
				pNpc->m_sNid + NPC_BAND, target_damage, attribute, dexpoint, righthand_damage);
			spdlog::trace("MagicProcess::AreaAttackDamage: magic damage calculated [magicId={} "
						  "damage={}]",
				magicid, damage);
			if (damage >= 0)
			{
				result = pNpc->SetHMagicDamage(damage);
			}
			else
			{
				damage = abs(damage);
				if (pType3->Attribute == 3)
					attack_type = 3; // 기절시키는 마법이라면.....
				else
					attack_type = magicid;

				if (!pNpc->SetDamage(attack_type, damage, m_pSrcUser->m_strUserID,
						m_pSrcUser->m_iUserId + USER_BAND))
				{
					// Npc가 죽은 경우,,
					pNpc->SendExpToUserList(); // 경험치 분배!!
					pNpc->SendDead();
					m_pSrcUser->SendAttackSuccess(
						pNpc->m_sNid + NPC_BAND, MAGIC_ATTACK_TARGET_DEAD, damage, pNpc->m_iHP);
				}
				else
				{
					m_pSrcUser->SendAttackSuccess(
						pNpc->m_sNid + NPC_BAND, ATTACK_SUCCESS, damage, pNpc->m_iHP);
				}
			}

			memset(sendBuffer, 0, sizeof(sendBuffer));
			sendIndex = 0;

			// 패킷 전송.....
			//if ( pMagic->bType2 == 0 || pMagic->bType2 == 3 )
			{
				SetByte(sendBuffer, AG_MAGIC_ATTACK_RESULT, sendIndex);
				SetByte(sendBuffer, MAGIC_EFFECTING, sendIndex);
				SetDWORD(sendBuffer, magicid, sendIndex);
				SetShort(sendBuffer, m_pSrcUser->m_iUserId, sendIndex);
				SetShort(sendBuffer, pNpc->m_sNid + NPC_BAND, sendIndex);
				SetShort(sendBuffer, data1, sendIndex);
				SetShort(sendBuffer, result, sendIndex);
				SetShort(sendBuffer, data3, sendIndex);
				SetShort(sendBuffer, moral, sendIndex);
				SetShort(sendBuffer, 0, sendIndex);
				SetShort(sendBuffer, 0, sendIndex);

				m_pMain->Send(sendBuffer, sendIndex, m_pSrcUser->m_curZone);
			}
		}
		// 타잎 4일 경우...
		else if (magictype == 4)
		{
			memset(sendBuffer, 0, sizeof(sendBuffer));
			sendIndex = 0;
			result    = 1;

			// Depending on which buff-type it is.....
			switch (pType4->BuffType)
			{
				case 1: // HP 올리기..
				case 2: // 방어력 올리기..
				case 4: // 공격력 올리기..
				case 5: // 공격 속도 올리기..
				case 7: // 능력치 올리기...
				case 8: // 저항력 올리기...
				case 9: // 공격 성공율 및 회피 성공율 올리기..
					break;

				case 6: // 이동 속도 올리기..
					//if (pNpc->m_MagicType4[pType4->bBuffType-1].sDurationTime > 0) {
					//	result = 0 ;
					//}
					//else {
					pNpc->m_MagicType4[pType4->BuffType - 1].byAmount      = pType4->Speed;
					pNpc->m_MagicType4[pType4->BuffType - 1].sDurationTime = pType4->Duration;
					pNpc->m_MagicType4[pType4->BuffType - 1].fStartTime    = TimeGet();
					pNpc->m_fSpeed_1 = pNpc->m_fOldSpeed_1 * (pType4->Speed / 100.0f);
					pNpc->m_fSpeed_2 = pNpc->m_fOldSpeed_2 * (pType4->Speed / 100.0f);
					//}
					break;

				default:
					result = 0;
					break;
			}

			spdlog::trace("MagicProcess::AreaAttackDamage: type4 processed [magicId={}]", magicid);

			SetByte(sendBuffer, AG_MAGIC_ATTACK_RESULT, sendIndex);
			SetByte(sendBuffer, MAGIC_EFFECTING, sendIndex);
			SetDWORD(sendBuffer, magicid, sendIndex);
			SetShort(sendBuffer, m_pSrcUser->m_iUserId, sendIndex);
			SetShort(sendBuffer, pNpc->m_sNid + NPC_BAND, sendIndex);
			SetShort(sendBuffer, data1, sendIndex);
			SetShort(sendBuffer, result, sendIndex);
			SetShort(sendBuffer, data3, sendIndex);
			SetShort(sendBuffer, 0, sendIndex);
			SetShort(sendBuffer, 0, sendIndex);
			SetShort(sendBuffer, 0, sendIndex);
			m_pMain->Send(sendBuffer, sendIndex, m_pSrcUser->m_curZone);
		}
		else if (magictype == 7)
		{
			damage      = target_damage;
			attack_type = magicid;
			if (!pNpc->SetDamage(attack_type, damage, m_pSrcUser->m_strUserID,
					m_pSrcUser->m_iUserId + USER_BAND))
			{
				pNpc->SendExpToUserList();
				pNpc->SendDead();
				m_pSrcUser->SendAttackSuccess(
					pNpc->m_sNid + NPC_BAND, MAGIC_ATTACK_TARGET_DEAD, damage, pNpc->m_iHP);
			}
			else
			{
				m_pSrcUser->SendAttackSuccess(
					pNpc->m_sNid + NPC_BAND, ATTACK_SUCCESS, damage, pNpc->m_iHP);
			}

			memset(sendBuffer, 0, sizeof(sendBuffer));
			sendIndex = 0;

			SetByte(sendBuffer, AG_MAGIC_ATTACK_RESULT, sendIndex);
			SetByte(sendBuffer, MAGIC_EFFECTING, sendIndex);
			SetDWORD(sendBuffer, magicid, sendIndex);
			SetShort(sendBuffer, m_pSrcUser->m_iUserId, sendIndex);
			SetShort(sendBuffer, pNpc->m_sNid + NPC_BAND, sendIndex);
			SetShort(sendBuffer, data1, sendIndex);
			SetShort(sendBuffer, result, sendIndex);
			SetShort(sendBuffer, data3, sendIndex);
			SetShort(sendBuffer, moral, sendIndex);
			SetShort(sendBuffer, 0, sendIndex);
			SetShort(sendBuffer, 0, sendIndex);
			m_pMain->Send(sendBuffer, sendIndex, m_pSrcUser->m_curZone);
		}
	}
}

int16_t CMagicProcess::GetWeatherDamage(int16_t damage, int16_t attribute)
{
	bool weather_buff = false;

	switch (m_pMain->_weatherType)
	{
		case WEATHER_FINE:
			if (attribute == ATTRIBUTE_FIRE)
				weather_buff = true;
			break;

		case WEATHER_RAIN:
			if (attribute == ATTRIBUTE_LIGHTNING)
				weather_buff = true;
			break;

		case WEATHER_SNOW:
			if (attribute == ATTRIBUTE_ICE)
				weather_buff = true;
			break;

		default:
			return damage;
	}

	if (weather_buff)
		damage = (damage * 110) / 100;

	return damage;
}

} // namespace AIServer

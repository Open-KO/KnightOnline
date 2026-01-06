#include "pch.h"
#include "NpcMagicProcess.h"
#include "AIServerApp.h"
#include "User.h"
#include "Npc.h"

namespace AIServer
{

CNpcMagicProcess::CNpcMagicProcess()
{
	m_pMain   = AIServerApp::instance();
	m_pSrcNpc = nullptr;
}

CNpcMagicProcess::~CNpcMagicProcess()
{
}

void CNpcMagicProcess::MagicPacket(char* pBuf, int len)
{
	int index = 0, sendIndex = 0, magicid = 0, sid = -1, tid = -1, data1 = 0, data2 = 0, data3 = 0,
		data4 = 0, data5 = 0, data6 = 0;
	model::Magic* pTable = nullptr;
	char sendBuffer[128] {};

	// Get the magic status.
	uint8_t command = GetByte(pBuf, index);

	// Client indicates that magic failed. Nothing to handle.
	if (command == MAGIC_FAIL)
		return;

	magicid = GetDWORD(pBuf, index); // Get ID of magic.
	sid     = GetShort(pBuf, index); // Get ID of source.
	tid     = GetShort(pBuf, index); // Get ID of target.

	data1   = GetShort(pBuf, index); // ( Remember, you don't definately need this. )
	data2   = GetShort(pBuf, index); // ( Only use it when you really feel it's needed. )
	data3   = GetShort(pBuf, index);
	data4   = GetShort(pBuf, index);
	data5   = GetShort(pBuf, index);
	data6   = GetShort(pBuf, index);

	// If magic was successful.......
	pTable  = IsAvailable(magicid, tid);
	if (pTable == nullptr)
		return;

	if (command == MAGIC_EFFECTING)
	{
		// Is target another player?
		//if (tid < -1 || tid >= MAX_USER) return;

		switch (pTable->Type1)
		{
			case 0:
				/* do nothing */
				break;

			case 1:
				ExecuteType1(pTable->ID, tid, data1, data2, data3);
				break;

			case 2:
				ExecuteType2(pTable->ID, tid, data1, data2, data3);
				break;

			case 3:
				ExecuteType3(pTable->ID, tid, data1, data2, data3, pTable->Moral);
				break;

			case 4:
				ExecuteType4(pTable->ID, tid);
				break;

			case 5:
				ExecuteType5(pTable->ID);
				break;

			case 6:
				ExecuteType6(pTable->ID);
				break;

			case 7:
				ExecuteType7(pTable->ID);
				break;

			case 8:
				ExecuteType8(pTable->ID, tid, sid, data1, data2, data3);
				break;

			case 9:
				ExecuteType9(pTable->ID);
				break;

			case 10:
				ExecuteType10(pTable->ID);
				break;

			default:
				spdlog::error("NpcMagicProcess::MagicPacket: Unhandled type1 type - ID={} Type1={}",
					pTable->ID, pTable->Type1);
				break;
		}

		switch (pTable->Type2)
		{
			case 0:
				/* do nothing */
				break;

			case 1:
				ExecuteType1(pTable->ID, tid, data4, data5, data6);
				break;

			case 2:
				ExecuteType2(pTable->ID, tid, data1, data2, data3);
				break;

			case 3:
				ExecuteType3(pTable->ID, tid, data1, data2, data3, pTable->Moral);
				break;

			case 4:
				ExecuteType4(pTable->ID, tid);
				break;

			case 5:
				ExecuteType5(pTable->ID);
				break;

			case 6:
				ExecuteType6(pTable->ID);
				break;

			case 7:
				ExecuteType7(pTable->ID);
				break;

			case 8:
				ExecuteType8(pTable->ID, tid, sid, data1, data2, data3);
				break;

			case 9:
				ExecuteType9(pTable->ID);
				break;

			case 10:
				ExecuteType10(pTable->ID);
				break;

			default:
				spdlog::error("NpcMagicProcess::MagicPacket: Unhandled type2 type - ID={} Type1={}",
					pTable->ID, pTable->Type2);
				break;
		}
	}
	else if (command == MAGIC_CASTING)
	{
		SetByte(sendBuffer, AG_MAGIC_ATTACK_RESULT, sendIndex);
		// len ==> include WIZ_MAGIC_PROCESS command byte.
		SetString(sendBuffer, pBuf, len - 1, sendIndex);
		m_pSrcNpc->SendAll(sendBuffer, sendIndex);
	}
}

void CNpcMagicProcess::SendMagicAttackResult(uint8_t opcode, int magicId, int sourceId,
	int targetId, int data1 /*= 0*/, int data2 /*= 0*/, int data3 /*= 0*/, int data4 /*= 0*/,
	int data5 /*= 0*/, int data6 /*= 0*/, int data7 /*= 0*/)
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
	m_pSrcNpc->SendAll(sendBuffer, sendIndex);
}

model::Magic* CNpcMagicProcess::IsAvailable(int magicid, int tid)
{
	CUser* pUser         = nullptr;
	CNpc* pNpc           = nullptr;
	model::Magic* pTable = nullptr;
	int moral            = 0;

	if (m_pSrcNpc == nullptr)
		return nullptr;

	// Get main magic table.
	pTable = m_pMain->_magicTableMap.GetData(magicid);
	if (pTable == nullptr)
	{
		m_pSrcNpc->SendMagicAttackResult(MAGIC_FAIL, magicid, tid);
		return nullptr;
	}

	// Compare morals between source and target character.
	if (tid >= 0 && tid < MAX_USER)
	{
		pUser = m_pMain->GetUserPtr(tid);
		if (pUser == nullptr || pUser->m_bLive == USER_DEAD)
		{
			m_pSrcNpc->SendMagicAttackResult(MAGIC_FAIL, magicid, tid);
			return nullptr;
		}

		moral = pUser->m_bNation;
	}
	// Compare morals between source and target NPC.
	else if (tid >= NPC_BAND)
	{
		pNpc = m_pMain->_npcMap.GetData(tid - NPC_BAND);
		if (pNpc == nullptr || pNpc->m_NpcState == NPC_DEAD)
		{
			m_pSrcNpc->SendMagicAttackResult(MAGIC_FAIL, magicid, tid);
			return nullptr;
		}

		moral = pNpc->m_byGroup;
	}
	// Party Moral check routine.
	else if (tid == -1)
	{
		if (pTable->Moral == MORAL_AREA_ENEMY)
		{
			// Switch morals. 작업할것 : 몬스터는 국가라는 개념이 없기 때문에.. 나중에 NPC가 이 마법을 사용하면 문제가 됨
			if (m_pSrcNpc->m_byGroup == 0)
				moral = 2;
			else
				moral = 1;
		}
		else
		{
			moral = m_pSrcNpc->m_byGroup;
		}
	}
	else
	{
		moral = m_pSrcNpc->m_byGroup;
	}

	// tid >= 0 case only
	switch (pTable->Moral)
	{
		case MORAL_SELF:
			if (tid != (m_pSrcNpc->m_sNid + NPC_BAND))
			{
				m_pSrcNpc->SendMagicAttackResult(MAGIC_FAIL, magicid, tid);
				return nullptr;
			}
			break;

		case MORAL_FRIEND_WITHME:
			if (m_pSrcNpc->m_byGroup != moral)
			{
				m_pSrcNpc->SendMagicAttackResult(MAGIC_FAIL, magicid, tid);
				return nullptr;
			}
			break;

		case MORAL_FRIEND_EXCEPTME:
			if (m_pSrcNpc->m_byGroup != moral || tid == (m_pSrcNpc->m_sNid + NPC_BAND))
			{
				m_pSrcNpc->SendMagicAttackResult(MAGIC_FAIL, magicid, tid);
				return nullptr;
			}
			break;

		case MORAL_PARTY:
		case MORAL_PARTY_ALL:
			break;

		case MORAL_NPC:
			if (pNpc == nullptr || pNpc->m_byGroup != moral)
			{
				m_pSrcNpc->SendMagicAttackResult(MAGIC_FAIL, magicid, tid);
				return nullptr;
			}
			break;

		case MORAL_ENEMY:
			if (m_pSrcNpc->m_byGroup == moral)
			{
				m_pSrcNpc->SendMagicAttackResult(MAGIC_FAIL, magicid, tid);
			}
			break;

		default:
			break;
	}

	return pTable; // Magic was successful!
}

// Applied to an attack skill using a weapon.
void CNpcMagicProcess::ExecuteType1(
	int /*magicid*/, int /*tid*/, int /*data1*/, int /*data2*/, int /*data3*/)
{
}

void CNpcMagicProcess::ExecuteType2(
	int /*magicid*/, int /*tid*/, int /*data1*/, int /*data2*/, int /*data3*/)
{
}

void CNpcMagicProcess::ExecuteType3(int magicid, int tid, int data1, int /*data2*/, int data3,
	int moral) // Applied when a magical attack, healing, and mana restoration is done.
{
	int damage = 0, result = 1;
	model::MagicType3* pType = nullptr;
	CNpc* pNpc               = nullptr; // Pointer initialization!
	int dexpoint             = 0;

	model::Magic* pMagic     = nullptr;

	// Get main magic table.
	pMagic                   = m_pMain->_magicTableMap.GetData(magicid);
	if (pMagic == nullptr)
		return;

	// 지역 공격,, 몬스터의 지역공격은 게임서버에서 처리한다.. 유저들을 상대로..
	if (tid == -1)
	{
		m_pSrcNpc->SendMagicAttackResult(
			MAGIC_EFFECTING, magicid, tid, data1, result, data3, moral);
		return;
	}

	pNpc = m_pMain->_npcMap.GetData(tid - NPC_BAND);
	if (pNpc == nullptr || pNpc->m_NpcState == NPC_DEAD || pNpc->m_iHP == 0)
	{
		result = 0;
		m_pSrcNpc->SendMagicAttackResult(
			MAGIC_EFFECTING, magicid, tid, data1, result, data3, moral);
		return;
	}

	// Get magic skill table type 3.
	pType = m_pMain->_magicType3TableMap.GetData(magicid);
	if (pType == nullptr)
		return;

	damage = GetMagicDamage(tid, pType->FirstDamage, pType->Attribute, dexpoint);
	//	if(damage == 0)	damage = -1;

	//TRACE(_T("magictype3 ,, magicid=%d, damage=%d\n"), magicid, damage);

	// Non-Durational Spells.
	if (pType->Duration == 0)
	{
		// Health Point related !
		if (pType->DirectType == 1)
		{
			if (damage > 0)
				result = pNpc->SetHMagicDamage(damage);
		}
	}

	m_pSrcNpc->SendMagicAttackResult(MAGIC_EFFECTING, magicid, tid, data1, result, data3, moral);
}

void CNpcMagicProcess::ExecuteType4(int /*magicid*/, int /*tid*/)
{
}

void CNpcMagicProcess::ExecuteType5(int /*magicid*/)
{
}

void CNpcMagicProcess::ExecuteType6(int /*magicid*/)
{
}

void CNpcMagicProcess::ExecuteType7(int /*magicid*/)
{
}

// Warp, resurrection, and summon spells.
void CNpcMagicProcess::ExecuteType8(
	int /*magicid*/, int /*tid*/, int /*sid*/, int /*data1*/, int /*data2*/, int /*data3*/)
{
}

void CNpcMagicProcess::ExecuteType9(int /*magicid*/)
{
}

void CNpcMagicProcess::ExecuteType10(int /*magicid*/)
{
}

int16_t CNpcMagicProcess::GetMagicDamage(int tid, int total_hit, int attribute, int dexpoint)
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
	}
	else
	{
		damage = 0;
	}

	if (!bSign && damage != 0)
		damage = -damage;

	return damage;
}

} // namespace AIServer

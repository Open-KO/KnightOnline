// NpcMagicProcess.cpp: implementation of the CNpcMagicProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "server.h"
#include "NpcMagicProcess.h"
#include "ServerDlg.h"
#include "User.h"
#include "Npc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNpcMagicProcess::CNpcMagicProcess()
{
	m_pMain = nullptr;
	m_pSrcNpc = nullptr;
	m_bMagicState = NONE;
}

CNpcMagicProcess::~CNpcMagicProcess()
{

}

void CNpcMagicProcess::MagicPacket(char* pBuf, int len, CIOCPort* pIOCP)
{
	int index = 0, send_index = 0, magicid = 0, sid = -1, tid = -1, data1 = 0, data2 = 0, data3 = 0, data4 = 0, data5 = 0, data6 = 0;
	char send_buff[128] = {};
	model::Magic* pTable = nullptr;

	// Get the magic status. 
	BYTE command = GetByte(pBuf, index);

	// Client indicates that magic failed. Just send back packet.
	if (command == MAGIC_FAIL)
	{
		SetByte(send_buff, AG_MAGIC_ATTACK_RESULT, send_index);
		SetString(send_buff, pBuf, len - 1, send_index);	// len ==> include WIZ_MAGIC_PROCESS command byte. 
		//m_pMain->Send_Region( send_buff, send_index, m_pSrcUser->m_pUserData->m_bZone, m_pSrcUser->m_RegionX, m_pSrcUser->m_RegionZ );
		m_bMagicState = NONE;
		return;
	}

	magicid = GetDWORD(pBuf, index);	// Get ID of magic.
	sid = GetShort(pBuf, index);		// Get ID of source.
	tid = GetShort(pBuf, index);		// Get ID of target.

	data1 = GetShort(pBuf, index);		// ( Remember, you don't definately need this. )
	data2 = GetShort(pBuf, index);		// ( Only use it when you really feel it's needed. )
	data3 = GetShort(pBuf, index);
	data4 = GetShort(pBuf, index);
	data5 = GetShort(pBuf, index);
	data6 = GetShort(pBuf, index);		// Get data1 ~ data6 (No, I don't know what the hell 'data' is.) 

	// If magic was successful.......
	pTable = IsAvailable(magicid, tid, command);
	if (!pTable)
		return;

	if (command == MAGIC_EFFECTING)
	{
		// Is target another player? 
		//if (tid < -1 || tid >= MAX_USER) return;	

		switch (pTable->Type1)
		{
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
		}

		switch (pTable->Type2)
		{
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
		}
	}
	else if (command == MAGIC_CASTING)
	{
		SetByte(send_buff, AG_MAGIC_ATTACK_RESULT, send_index);
		SetString(send_buff, pBuf, len - 1, send_index);	// len ==> include WIZ_MAGIC_PROCESS command byte. 
		m_pSrcNpc->SendAll(pIOCP, send_buff, send_index);
	}
}

model::Magic* CNpcMagicProcess::IsAvailable(int magicid, int tid, BYTE type)
{
	CUser* pUser = nullptr;
	CNpc* pNpc = nullptr;
	model::Magic* pTable = nullptr;

	int modulator = 0, Class = 0, send_index = 0, moral = 0;

	char send_buff[128] = {};
	if (m_pSrcNpc == nullptr)
		return nullptr;

	// Get main magic table.
	pTable = m_pMain->m_MagicTableMap.GetData(magicid);
	if (!pTable)
		goto fail_return;

	// Compare morals between source and target character.
	if (tid >= 0
		&& tid < MAX_USER)
	{
		pUser = m_pMain->GetUserPtr(tid);
		if (!pUser
			|| pUser->m_bLive == USER_DEAD)
			goto fail_return;

		moral = pUser->m_bNation;
	}
	// Compare morals between source and target NPC.            
	else if (tid >= NPC_BAND)
	{
		pNpc = m_pMain->m_NpcMap.GetData(tid - NPC_BAND);
		if (!pNpc
			|| pNpc->m_NpcState == NPC_DEAD)
			goto fail_return;

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
				goto fail_return;
			break;

		case MORAL_FRIEND_WITHME:
			if (m_pSrcNpc->m_byGroup != moral)
				goto fail_return;
			break;

		case MORAL_FRIEND_EXCEPTME:
			if (m_pSrcNpc->m_byGroup != moral)
				goto fail_return;
			if (tid == (m_pSrcNpc->m_sNid + NPC_BAND))
				goto fail_return;
			break;

		case MORAL_PARTY:
		case MORAL_PARTY_ALL:
			break;

		case MORAL_NPC:
			if (!pNpc)
				goto fail_return;

			if (pNpc->m_byGroup != moral)
				goto fail_return;
			break;

		case MORAL_ENEMY:
			if (m_pSrcNpc->m_byGroup == moral)
				goto fail_return;
			break;
	}

	// Make sure you subtract MPs (SPs) after you use spell (skill).
	if (type == MAGIC_EFFECTING)
	{
		// MP만 달다록 처리한당.. Npc는 SP가 없음..
		//if( pTable->sMsp > m_pSrcNpc->m_sMP )
		//	goto fail_return;
		//m_pSrcNpc->MSpChange(2, -(pTable->sMsp) );
		//m_bMagicState = NONE;
	}

	return pTable;      // Magic was successful! 

fail_return:    // In case the magic failed.
	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	SetByte(send_buff, AG_MAGIC_ATTACK_RESULT, send_index);
	SetByte(send_buff, MAGIC_FAIL, send_index);
	SetDWORD(send_buff, magicid, send_index);
	SetShort(send_buff, m_pSrcNpc->m_sNid + NPC_BAND, send_index);
	SetShort(send_buff, tid, send_index);
	if (type == MAGIC_CASTING)
		SetShort(send_buff, -100, send_index);
	else
		SetShort(send_buff, 0, send_index);
	SetShort(send_buff, 0, send_index);
	SetShort(send_buff, 0, send_index);
	SetShort(send_buff, 0, send_index);
	SetShort(send_buff, 0, send_index);
	SetShort(send_buff, 0, send_index);

/*	if( m_bMagicState == CASTING )
		m_pMain->Send_Region( send_buff, send_index, m_pSrcUser->m_pUserData->m_bZone, m_pSrcUser->m_RegionX, m_pSrcUser->m_RegionZ );
	else m_pSrcUser->Send( send_buff, send_index );	*/

	m_pSrcNpc->SendAll(&m_pMain->m_Iocport, send_buff, send_index);

	m_bMagicState = NONE;

	return nullptr;     // Magic was a failure!
}

// Applied to an attack skill using a weapon.
void CNpcMagicProcess::ExecuteType1(int magicid, int tid, int data1, int data2, int data3)
{
}

void CNpcMagicProcess::ExecuteType2(int magicid, int tid, int data1, int data2, int data3)
{
}

void CNpcMagicProcess::ExecuteType3(int magicid, int tid, int data1, int data2, int data3, int moral)  // Applied when a magical attack, healing, and mana restoration is done.
{
	int damage = 0, result = 1, send_index = 0, attack_type = 0;
	char send_buff[256] = {};
	model::MagicType3* pType = nullptr;
	CNpc* pNpc = nullptr;      // Pointer initialization!
	int dexpoint = 0;

	model::Magic* pMagic = nullptr;

	// Get main magic table.
	pMagic = m_pMain->m_MagicTableMap.GetData(magicid);
	if (!pMagic)
		return;

	// 지역 공격,, 몬스터의 지역공격은 게임서버에서 처리한다.. 유저들을 상대로..
	if (tid == -1)
		goto packet_send;

	pNpc = m_pMain->m_NpcMap.GetData(tid - NPC_BAND);
	if (pNpc == nullptr
		|| pNpc->m_NpcState == NPC_DEAD
		|| pNpc->m_iHP == 0)
	{
		result = 0;
		goto packet_send;
	}

	// Get magic skill table type 3.
	pType = m_pMain->m_MagicType3TableMap.GetData(magicid);
	if (!pType)
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
			{
				result = pNpc->SetHMagicDamage(damage, &m_pMain->m_Iocport);
			}
			else
			{
				damage = abs(damage);
/*				if(pType->bAttribute == 3)   attack_type = 3; // 기절시키는 마법이라면.....
				else attack_type = magicid;

				if (!pNpc->SetDamage(attack_type, damage, m_pSrcUser->m_strUserID, m_pSrcUser->m_iUserId + USER_BAND, m_pSrcUser->m_pIocport))	{
					// Npc가 죽은 경우,,
					pNpc->SendExpToUserList(); // 경험치 분배!!
					pNpc->SendDead(m_pSrcUser->m_pIocport);
					m_pSrcUser->SendAttackSuccess(tid, MAGIC_ATTACK_TARGET_DEAD, damage, pNpc->m_iHP, MAGIC_ATTACK);
				}
				else	{
					// 공격 결과 전송
					m_pSrcUser->SendAttackSuccess(tid, ATTACK_SUCCESS, damage, pNpc->m_iHP, MAGIC_ATTACK);
				}	*/
			}
		}
	}
	// Durational Spells! Remember, durational spells only involve HPs.
	else if (pType->Duration != 0)
	{
	}

packet_send:
	//if ( pMagic->bType2 == 0 || pMagic->bType2 == 3 ) 
	{
		SetByte(send_buff, AG_MAGIC_ATTACK_RESULT, send_index);
		SetByte(send_buff, MAGIC_EFFECTING, send_index);
		SetDWORD(send_buff, magicid, send_index);
		SetShort(send_buff, m_pSrcNpc->m_sNid + NPC_BAND, send_index);
		SetShort(send_buff, tid, send_index);
		SetShort(send_buff, data1, send_index);
		SetShort(send_buff, result, send_index);
		SetShort(send_buff, data3, send_index);
		SetShort(send_buff, moral, send_index);
		SetShort(send_buff, 0, send_index);
		SetShort(send_buff, 0, send_index);
		m_pSrcNpc->SendAll(&m_pMain->m_Iocport, send_buff, send_index);
	}

/*	int send_index = 0, result = 1;
	char send_buff[256] = {};
	SetByte( send_buff, AG_MAGIC_ATTACK_RESULT, send_index );
	SetByte( send_buff, MAGIC_EFFECTING, send_index );
	SetDWORD( send_buff, magicid, send_index );
	SetShort( send_buff, m_pSrcNpc->m_sNid+NPC_BAND, send_index );
	SetShort( send_buff, tid, send_index );
	SetShort( send_buff, data1, send_index );
	SetShort( send_buff, result, send_index );
	SetShort( send_buff, data3, send_index );
	SetShort( send_buff, moral, send_index );
	SetShort( send_buff, 0, send_index );
	SetShort( send_buff, 0, send_index );
	m_pSrcNpc->SendAll(&m_pMain->m_Iocport, send_buff, send_index);	*/
}

void CNpcMagicProcess::ExecuteType4(int magicid, int tid)
{
}

void CNpcMagicProcess::ExecuteType5(int magicid)
{
}

void CNpcMagicProcess::ExecuteType6(int magicid)
{
}

void CNpcMagicProcess::ExecuteType7(int magicid)
{
}

// Warp, resurrection, and summon spells.
void CNpcMagicProcess::ExecuteType8(int magicid, int tid, int sid, int data1, int data2, int data3)
{
}

void CNpcMagicProcess::ExecuteType9(int magicid)
{
}

void CNpcMagicProcess::ExecuteType10(int magicid)
{
}

short CNpcMagicProcess::GetMagicDamage(int tid, int total_hit, int attribute, int dexpoint)
{
	short damage = 0, temp_hit = 0;
	int random = 0, total_r = 0;
	BYTE result;
	bool bSign = true;			// false이면 -, true이면 +

	// Check if target id is valid.
	if (tid < NPC_BAND
		|| tid > INVALID_BAND)
		return 0;

	CNpc* pNpc = nullptr;
	pNpc = m_pMain->m_NpcMap.GetData(tid - NPC_BAND);
	if (pNpc == nullptr
		|| pNpc->m_NpcState == NPC_DEAD
		|| pNpc->m_iHP == 0)
		return 0;

	if (pNpc->m_tNpcType == NPC_ARTIFACT
		|| pNpc->m_tNpcType == NPC_PHOENIX_GATE
		|| pNpc->m_tNpcType == NPC_GATE_LEVER
		|| pNpc->m_tNpcType == NPC_SPECIAL_GATE)
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

			case LIGHT_R:
				// LATER !!!
				break;

			case DARKNESS_R:
				// LATER !!!
				break;
		}

		total_hit = (total_hit * (dexpoint + 20)) / 170;

		if (total_hit < 0)
		{
			total_hit = abs(total_hit);
			bSign = false;
		}

		damage = (short) (total_hit - (0.7f * total_hit * total_r / 200));
		random = myrand(0, damage);
		damage = (short) (0.7f * (total_hit - (0.9f * total_hit * total_r / 200))) + 0.2f * random;
	}
	else
	{
		damage = 0;
	}

	if (!bSign
		&& damage != 0)
		damage = -damage;

	return damage;
}

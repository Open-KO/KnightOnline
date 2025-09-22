// MagicProcess.cpp: implementation of the CMagicProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ebenezer.h"
#include "MagicProcess.h"
#include "EbenezerDlg.h"
#include "User.h"
#include "Npc.h"
#include "GameDefine.h"

#include <shared/packets.h>
#include <spdlog/spdlog.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define MORAL_SELF				1		// 나 자신..
#define MORAL_FRIEND_WITHME		2		// 나를 포함한 우리편(국가) 중 하나 ..
#define MORAL_FRIEND_EXCEPTME	3		// 나를 뺀 우리편 중 하나 
#define MORAL_PARTY				4		// 나를 포함한 우리파티 중 하나..
#define MORAL_NPC				5		// NPC중 하나.
#define MORAL_PARTY_ALL			6		// 나를 호함한 파티 모두..
#define MORAL_ENEMY				7		// 울편을 제외한 모든 적중 하나(NPC포함)
#define MORAL_ALL				8		// 겜상에 존재하는 모든 것중 하나.
#define MORAL_AREA_ENEMY		10		// 지역에 포함된 적
#define MORAL_AREA_FRIEND		11		// 지역에 포함된 우리편
#define MORAL_AREA_ALL			12		// 지역에 포함된 모두
#define MORAL_SELF_AREA			13		// 나를 중심으로 한 지역
// 비러머글 클랜소환
#define MORAL_CLAN				14		// 클랜 맴버 중 한명...
#define MORAL_CLAN_ALL			15		// 나를 포함한 클랜 맴버 다...
//

#define MORAL_UNDEAD			16		// Undead Monster
#define MORAL_PET_WITHME		17      // My Pet
#define MORAL_PET_ENEMY			18		// Enemy's Pet
#define MORAL_ANIMAL1			19		// Animal #1
#define MORAL_ANIMAL2			20		// Animal #2
#define MORAL_ANIMAL3			21		// Animal #3
#define MORAL_ANGEL				22		// Angel
#define MORAL_DRAGON			23		// Dragon
#define MORAL_CORPSE_FRIEND     25		// A Corpse of the same race.
#define MORAL_CORPSE_ENEMY      26		// A Corpse of a different race.

#define WARP_RESURRECTION		1		// To the resurrection point.

#define REMOVE_TYPE3			1
#define REMOVE_TYPE4			2
#define RESURRECTION			3
#define	RESURRECTION_SELF		4
#define REMOVE_BLESS			5

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMagicProcess::CMagicProcess()
{
	m_pSrcUser = nullptr;
	m_pMain = nullptr;
	m_bMagicState = MAGIC_STATE_NONE;
}

CMagicProcess::~CMagicProcess()
{
}

void CMagicProcess::MagicPacket(char* pBuf, int len)
{
	int index = 0, send_index = 0, magicid = 0, sid = -1, tid = -2, data1 = 0, data2 = 0, data3 = 0, data4 = 0, data5 = 0, data6 = 0;
	char send_buff[128] = {};
	int type3_attribute = 0;

	model::Magic* pTable = nullptr;
	CNpc* pMon = nullptr;

	BYTE command = GetByte(pBuf, index);	// Get the magic status.  
	magicid = GetDWORD(pBuf, index);		// Get ID of magic.
	sid = GetShort(pBuf, index);			// Get ID of source.
	tid = GetShort(pBuf, index);			// Get ID of target.
	data1 = GetShort(pBuf, index);			// Additional Info :)
	data2 = GetShort(pBuf, index);
	data3 = GetShort(pBuf, index);
	data4 = GetShort(pBuf, index);
	data5 = GetShort(pBuf, index);
	data6 = GetShort(pBuf, index);

	// 눈싸움전쟁존에서 눈싸움중이라면 공격은 눈을 던지는 것만 가능하도록,,,
	if (m_pSrcUser != nullptr)
	{
		if (m_pSrcUser->m_pUserData->m_bZone == ZONE_SNOW_BATTLE
			&& m_pMain->m_byBattleOpen == SNOW_BATTLE)
		{
			if (magicid != SNOW_EVENT_SKILL)
				return;		// 하드 코딩 싫어,,, 
		}
	}

	if (command == MAGIC_CANCEL)
	{
		Type3Cancel(magicid, sid);	 // Type 3 cancel procedure.
		Type4Cancel(magicid, sid);   // Type 4 cancel procedure.
		return;
	}

	model::Magic* pMagic = m_pMain->m_MagicTableMap.GetData(magicid);   // Get main magic table.
	if (pMagic == nullptr)
		return;

	if (sid >= NPC_BAND)
	{
		pMon = m_pMain->m_NpcMap.GetData(sid);
		if (pMon == nullptr
			|| pMon->m_NpcState == NPC_DEAD)
			return;
	}
	else if (sid >= 0
		&& sid < MAX_USER)
	{
		CUser* pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[sid];
		if (pUser == nullptr)
			return;

		if (pUser->m_bResHpType == USER_DEAD
			|| pUser->m_pUserData->m_sHp == 0)
		{
			spdlog::error("MagicProcess::MagicPacket: user is dead [charId={} userId={} resHpType={} hp={}]",
				pUser->m_pUserData->m_id, pUser->GetSocketID(),
				pUser->m_bResHpType, pUser->m_pUserData->m_sHp);
			return;
		}
	}

	// Type 4 Repeat Check!!!
	if (tid >= 0
		&& tid < MAX_USER)
	{
		if (pMagic->Type1 == 4)
		{
			if (pMagic->Moral < 5)
			{
				CUser* pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[tid];
				if (pTUser == nullptr)
					return;

				model::MagicType4* pType4 = m_pMain->m_MagicType4TableMap.GetData(magicid);     // Get magic skill table type 4.
				if (pType4 == nullptr)
					return;

				if (pTUser->m_bType4Buff[pType4->BuffType - 1] > 0)
				{
					SetByte(send_buff, WIZ_MAGIC_PROCESS, send_index);
					SetByte(send_buff, MAGIC_FAIL, send_index);
					SetDWORD(send_buff, magicid, send_index);
					SetShort(send_buff, sid, send_index);
					SetShort(send_buff, tid, send_index);
					SetShort(send_buff, 0, send_index);
					SetShort(send_buff, 0, send_index);
					SetShort(send_buff, 0, send_index);
					SetShort(send_buff, -103, send_index);
					SetShort(send_buff, 0, send_index);
					SetShort(send_buff, 0, send_index);

					if (m_bMagicState == MAGIC_STATE_CASTING)
					{
						m_pMain->Send_Region(send_buff, send_index, m_pSrcUser->m_pUserData->m_bZone, m_pSrcUser->m_RegionX, m_pSrcUser->m_RegionZ, nullptr, false);
					}
					else
					{
						m_pSrcUser->Send(send_buff, send_index);
					}

					m_bMagicState = MAGIC_STATE_NONE;
					return;     // Magic was a failure!						
				}
			}
		}
	}

	// Type 3 Repeat Check!!!
	if (tid >= 0
		&& tid < MAX_USER)
	{
		if (pMagic->Type1 == 3)
		{
			if (pMagic->Moral < 5)
			{
				CUser* pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[tid];
				if (pTUser == nullptr)
					return;

				model::MagicType3* pType3 = m_pMain->m_MagicType3TableMap.GetData(magicid);     // Get magic skill table type 4.
				if (pType3 == nullptr)
					return;

				if (pType3->TimeDamage > 0)
				{
					for (int i = 0; i < MAX_TYPE3_REPEAT; i++)
					{
						if (pTUser->m_bHPAmount[i] > 0)
						{
							SetByte(send_buff, WIZ_MAGIC_PROCESS, send_index);
							SetByte(send_buff, MAGIC_FAIL, send_index);
							SetDWORD(send_buff, magicid, send_index);
							SetShort(send_buff, sid, send_index);
							SetShort(send_buff, tid, send_index);
							SetShort(send_buff, 0, send_index);
							SetShort(send_buff, 0, send_index);
							SetShort(send_buff, 0, send_index);
							SetShort(send_buff, -103, send_index);
							SetShort(send_buff, 0, send_index);
							SetShort(send_buff, 0, send_index);

							if (m_bMagicState == MAGIC_STATE_CASTING)
							{
								m_pMain->Send_Region(send_buff, send_index, m_pSrcUser->m_pUserData->m_bZone, m_pSrcUser->m_RegionX, m_pSrcUser->m_RegionZ, nullptr, false);
							}
							else
							{
								m_pSrcUser->Send(send_buff, send_index);
							}

							m_bMagicState = MAGIC_STATE_NONE;
							return;     // Magic was a failure!	
						}
					}
				}
			}
		}
	}

	// 비러머글 클랜 소환 >.<
	// Make sure the source is a user!
	if (sid >= 0
		&& sid < MAX_USER)
	{
		// Make sure the zone is a battlezone!
		if (m_pSrcUser->m_pUserData->m_bZone == ZONE_BATTLE)
		{
			// Make sure the target is another player.
			if (tid >= 0
				&& tid < MAX_USER)
			{
				// Is it a warp spell?
				if (pMagic->Type1 == 8)
				{
					if (pMagic->Moral < 5
						|| pMagic->Moral == MORAL_CLAN)
					{
						float currenttime = TimeGet();

						CUser* pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[tid];
						if (pTUser == nullptr)
							return;

						if ((currenttime - pTUser->m_fLastRegeneTime) < CLAN_SUMMON_TIME)
						{
							SetByte(send_buff, WIZ_MAGIC_PROCESS, send_index);
							SetByte(send_buff, MAGIC_FAIL, send_index);
							SetDWORD(send_buff, magicid, send_index);
							SetShort(send_buff, sid, send_index);
							SetShort(send_buff, tid, send_index);
							SetShort(send_buff, 0, send_index);
							SetShort(send_buff, 0, send_index);
							SetShort(send_buff, 0, send_index);
							SetShort(send_buff, -103, send_index);
							SetShort(send_buff, 0, send_index);
							SetShort(send_buff, 0, send_index);

							if (m_bMagicState == MAGIC_STATE_CASTING)
							{
								m_pMain->Send_Region(send_buff, send_index, m_pSrcUser->m_pUserData->m_bZone, m_pSrcUser->m_RegionX, m_pSrcUser->m_RegionZ, nullptr, false);
							}
							else
							{
								m_pSrcUser->Send(send_buff, send_index);
							}

							m_bMagicState = MAGIC_STATE_NONE;
							return;     // Magic was a failure!	
						}
					}
				}
			}
		}
	}

	// Client indicates that magic failed. Just send back packet.
	if (command == MAGIC_FAIL)
		goto return_echo;

	// When the arrow starts flying....
	if (command == MAGIC_FLYING)
	{
		if (pMagic->Type1 == 2)
		{
			model::MagicType2* pType = m_pMain->m_MagicType2TableMap.GetData(magicid);     // Get magic skill table type 2.
			if (pType == nullptr)
				return;

			// If the PLAYER shoots an arrow.
			if (sid >= 0
				&& sid < MAX_USER)
			{
				// Only if Flying Effect is greater than 0.
				if (pMagic->FlyingEffect > 0)
				{
					// Reduce Magic Point!
					if (pMagic->ManaCost > m_pSrcUser->m_pUserData->m_sMp)
					{
						command = MAGIC_FAIL;
						goto return_echo;
					}

					m_pSrcUser->MSpChange(-(pMagic->ManaCost));
				}

				// Subtract Arrow!
				if (m_pSrcUser->ItemCountChange(pMagic->UseItem, 1, pType->NeedArrow) < 2)
				{
					command = MAGIC_FAIL;
					goto return_echo;
				}
			}
		}
		goto return_echo;
	}

	// If magic was successful...
	pTable = IsAvailable(magicid, tid, sid, command, data1, data2, data3);
	if (pTable == nullptr)
		return;

	if (command == MAGIC_EFFECTING)
	{
		int initial_result = 1;

		if (sid >= 0
			&& sid < MAX_USER)
		{
			// If the target is an NPC.
			if (tid >= NPC_BAND
				|| (tid == -1 && (pMagic->Moral == MORAL_AREA_ENEMY || pMagic->Moral == MORAL_SELF_AREA)))
			{
				int total_magic_damage = 0;

				SetByte(send_buff, AG_MAGIC_ATTACK_REQ, send_index);
				SetShort(send_buff, sid, send_index);
				SetByte(send_buff, command, send_index);
				SetShort(send_buff, tid, send_index);
				SetDWORD(send_buff, magicid, send_index);
				SetShort(send_buff, data1, send_index);
				SetShort(send_buff, data2, send_index);
				SetShort(send_buff, data3, send_index);
				SetShort(send_buff, data4, send_index);
				SetShort(send_buff, data5, send_index);
				SetShort(send_buff, data6, send_index);
				short total_cha = m_pSrcUser->m_pUserData->m_bCha + m_pSrcUser->m_sItemCham;
				SetShort(send_buff, total_cha, send_index);

				// Does the magic user have a staff?
				if (m_pSrcUser->m_pUserData->m_sItemArray[RIGHTHAND].nNum != 0)
				{
					model::Item* pRightHand = m_pMain->m_ItemTableMap.GetData(m_pSrcUser->m_pUserData->m_sItemArray[RIGHTHAND].nNum);

					if (pRightHand != nullptr
						&& m_pSrcUser->m_pUserData->m_sItemArray[LEFTHAND].nNum == 0
						&& (pRightHand->Kind / 10) == WEAPON_STAFF)
					{
//						total_magic_damage += pRightHand->m_sDamage;
//						total_magic_damage += ((pRightHand->m_sDamage * 0.8f)+ (pRightHand->m_sDamage * m_pSrcUser->m_pUserData->m_bLevel) / 60);

						if (pMagic->Type1 == 3)
						{
//
							total_magic_damage += ((pRightHand->Damage * 0.8f) + (pRightHand->Damage * m_pSrcUser->m_pUserData->m_bLevel) / 60);
//

							model::MagicType3* pType3 = m_pMain->m_MagicType3TableMap.GetData(magicid);     // Get magic skill table type 4.
							if (pType3 == nullptr)
								return;

							if (m_pSrcUser->m_bMagicTypeRightHand == pType3->Attribute)
							{
//								total_magic_damage += pRightHand->Damage;
								total_magic_damage += ((pRightHand->Damage * 0.8f) + (pRightHand->Damage * m_pSrcUser->m_pUserData->m_bLevel) / 30);
							}
//
							// Remember what Sunglae told ya!
							if (pType3->Attribute == 4)
								total_magic_damage = 0;
//
						}

						SetShort(send_buff, total_magic_damage, send_index);
					}
					else
					{
						SetShort(send_buff, 0, send_index);
					}
				}
				// If not, just use the normal formula :)
				else
				{
					SetShort(send_buff, 0, send_index);
				}

				m_pMain->Send_AIServer(m_pSrcUser->m_pUserData->m_bZone, send_buff, send_index);
			}
		}

		// Make sure the target is another player and it exists.
		if (tid < -1
			|| tid >= MAX_USER)
			return;

		switch (pTable->Type1)
		{
			case 1:
				initial_result = ExecuteType1(pTable->ID, sid, tid, data1, data2, data3);
				break;

			case 2:
				initial_result = ExecuteType2(pTable->ID, sid, tid, data1, data2, data3);
				break;

			case 3:
				ExecuteType3(pTable->ID, sid, tid, data1, data2, data3);
				break;

			case 4:
				ExecuteType4(pTable->ID, sid, tid, data1, data2, data3);
				break;

			case 5:
				ExecuteType5(pTable->ID, sid, tid, data1, data2, data3);
				break;

			case 6:
				ExecuteType6(pTable->ID);
				break;

			case 7:
				ExecuteType7(pTable->ID);
				break;

			case 8:
				ExecuteType8(pTable->ID, sid, tid, data1, data2, data3);
				break;

			case 9:
				ExecuteType9(pTable->ID);
				break;

			case 10:
				ExecuteType10(pTable->ID);
				break;
		}

		if (initial_result != 0)
		{
			switch (pTable->Type2)
			{
				case 1:
					ExecuteType1(pTable->ID, sid, tid, data1, data2, data3);
					break;

				case 2:
					ExecuteType2(pTable->ID, sid, tid, data1, data2, data3);
					break;

				case 3:
					ExecuteType3(pTable->ID, sid, tid, data1, data2, data3);
					break;

				case 4:
					ExecuteType4(pTable->ID, sid, tid, data1, data2, data3);
					break;

				case 5:
					ExecuteType5(pTable->ID, sid, tid, data1, data2, data3);
					break;

				case 6:
					ExecuteType6(pTable->ID);
					break;

				case 7:
					ExecuteType7(pTable->ID);
					break;

				case 8:
					ExecuteType8(pTable->ID, sid, tid, data1, data2, data3);
					break;

				case 9:
					ExecuteType9(pTable->ID);
					break;

				case 10:
					ExecuteType10(pTable->ID);
					break;
			}
		}
	}
	// 원래 이한줄만 있었음.....
	else if (command == MAGIC_CASTING)
	{
		goto return_echo;
	}

	return;

return_echo:
	SetByte(send_buff, WIZ_MAGIC_PROCESS, send_index);
	SetByte(send_buff, command, send_index);
	SetDWORD(send_buff, magicid, send_index);
	SetShort(send_buff, sid, send_index);
	SetShort(send_buff, tid, send_index);
	SetShort(send_buff, data1, send_index);
	SetShort(send_buff, data2, send_index);
	SetShort(send_buff, data3, send_index);
	SetShort(send_buff, data4, send_index);
	SetShort(send_buff, data5, send_index);
	SetShort(send_buff, data6, send_index);

	if (sid >= 0
		&& sid < MAX_USER)
	{
		m_pMain->Send_Region(send_buff, send_index, m_pSrcUser->m_pUserData->m_bZone, m_pSrcUser->m_RegionX, m_pSrcUser->m_RegionZ, nullptr, false);
	}
	else if (sid >= NPC_BAND)
	{
		m_pMain->Send_Region(send_buff, send_index, pMon->m_sCurZone, pMon->m_sRegion_X, pMon->m_sRegion_Z, nullptr, false);
	}
}

model::Magic* CMagicProcess::IsAvailable(int magicid, int tid, int sid, BYTE type, int data1, int data2, int data3)
{
	CUser* pUser = nullptr;		// When the target is a player....
	CUser* pParty = nullptr;	// When the target is a party....
	CNpc* pNpc = nullptr;		// When the monster is the target....
	CNpc* pMon = nullptr;		// When the monster is the source....
	bool bFlag = false;			// Identifies source : true means source is NPC.
	model::MagicType5* pType = nullptr;		// Only for type 5 magic!

	int modulator = 0, Class = 0, send_index = 0, moral = 0;	// Variable Initialization.
	char send_buff[128] = {};
	int skill_mod = 0;

	model::Magic* pTable = m_pMain->m_MagicTableMap.GetData(magicid);   // Get main magic table.
	if (pTable == nullptr)
		goto fail_return;

	// Check source validity when the source is a player.
	if (sid >= 0
		&& sid < MAX_USER)
	{
		if (m_pSrcUser == nullptr)
			goto fail_return;

		if (m_pSrcUser->m_bAbnormalType == ABNORMAL_BLINKING)
			goto fail_return;
	}
	//  Check source validity when the source is a NPC.
	else if (sid >= NPC_BAND)
	{
		bFlag = true;
		pMon = m_pMain->m_NpcMap.GetData(sid);
		if (pMon == nullptr
			|| pMon->m_NpcState == NPC_DEAD)
			goto fail_return;
	}
	else
	{
		goto fail_return;
	}

	// Target existence check routine for player.
	if (tid >= 0
		&& tid < MAX_USER)
	{
		pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[tid];

		// If not a Warp/Resurrection spell...
		if (pTable->Type1 != 5)
		{
			if (pUser == nullptr
				|| pUser->m_bResHpType == USER_DEAD
				|| pUser->m_bAbnormalType == ABNORMAL_BLINKING)
				goto fail_return;
		}
		else if (pTable->Type1 == 5)
		{
			pType = m_pMain->m_MagicType5TableMap.GetData(magicid);
			if (pType == nullptr)
				goto fail_return;

			if (pUser == nullptr)
				goto fail_return;

			if (pUser->m_bAbnormalType == ABNORMAL_BLINKING)
				goto fail_return;

			if (pUser->m_bResHpType == USER_DEAD
				&& pType->NeedStone == 0
				&& pType->ExpRecover == 0)
				goto fail_return;
		}

		moral = pUser->m_pUserData->m_bNation;
	}
	// Target existence check routine for NPC.
	else if (tid >= NPC_BAND)
	{
		// 포인터 참조하면 안됨
		if (!m_pMain->m_bPointCheckFlag)
			goto fail_return;

		pNpc = m_pMain->m_NpcMap.GetData(tid);
		if (pNpc == nullptr
			//... Assuming NPCs can't be resurrected.
			|| pNpc->m_NpcState == NPC_DEAD)
			goto fail_return;

		moral = pNpc->m_byGroup;
	}
	// Party Moral check routine.
	else if (tid == -1)
	{
		if (pTable->Moral == MORAL_AREA_ENEMY)
		{
			if (!bFlag)
			{
				// Switch morals.
				if (m_pSrcUser->m_pUserData->m_bNation == 1)
					moral = 2;
				else
					moral = 1;
			}
			else
			{
				moral = 1;
			}
		}
		else
		{
			if (!bFlag)
				moral = m_pSrcUser->m_pUserData->m_bNation;
			else
				moral = 1;
		}
	}
	else
	{
		moral = m_pSrcUser->m_pUserData->m_bNation;
	}

	// Compare morals between source and target character.
	switch (pTable->Moral)
	{
		case MORAL_SELF:   // #1         // ( to see if spell is cast on the right target or not )
			if (bFlag)
			{
				if (tid != pMon->m_sNid)
					goto fail_return;
			}
			else
			{
				if (tid != m_pSrcUser->GetSocketID())
					goto fail_return;
			}
			break;

		case MORAL_FRIEND_WITHME:	// #2
			if (bFlag)
			{
				if (pMon->m_byGroup != moral)
					goto fail_return;
			}
			else
			{
				if (m_pSrcUser->m_pUserData->m_bNation != moral)
					goto fail_return;
			}
			break;

		case MORAL_FRIEND_EXCEPTME:	   // #3
			if (bFlag)
			{
				if (pMon->m_byGroup != moral)
					goto fail_return;

				if (tid == pMon->m_sNid)
					goto fail_return;
			}
			else
			{
				if (m_pSrcUser->m_pUserData->m_bNation != moral)
					goto fail_return;

				if (tid == m_pSrcUser->GetSocketID())
					goto fail_return;
			}
			break;

		case MORAL_PARTY:	 // #4
			if (m_pSrcUser->m_sPartyIndex == -1
				&& sid != tid)
				goto fail_return;

			if (m_pSrcUser->m_pUserData->m_bNation != moral)
				goto fail_return;

			if (pUser != nullptr
				&& pUser->m_sPartyIndex != m_pSrcUser->m_sPartyIndex)
				goto fail_return;
			break;

		case MORAL_NPC:		// #5
			if (pNpc == nullptr)
				goto fail_return;

			if (pNpc->m_byGroup != moral)
				goto fail_return;
			break;

		case MORAL_PARTY_ALL:     // #6
//			if (m_pSrcUser->m_sPartyIndex == -1)
//				goto fail_return;

//			if (m_pSrcUser->m_sPartyIndex == -1
//				&& sid != tid)
//				goto fail_return;
			break;

		case MORAL_ENEMY:	// #7	
			if (bFlag)
			{
				if (pMon->m_byGroup == moral)
					goto fail_return;
			}
			else
			{
				if (m_pSrcUser->m_pUserData->m_bNation == moral)
					goto fail_return;
			}
			break;

		case MORAL_ALL:	 // #8
			// N/A
			break;

		case MORAL_AREA_ENEMY:		// #10
			// N/A
			break;

		case MORAL_AREA_FRIEND:		// #11
			if (m_pSrcUser->m_pUserData->m_bNation != moral)
				goto fail_return;
			break;

		case MORAL_AREA_ALL:	// #12
			// N/A
			break;

		case MORAL_SELF_AREA:     // #13
			// Remember, EVERYONE in the area is affected by this one. No moral check!!!
			break;

		case MORAL_CORPSE_FRIEND:		// #25
			if (bFlag)
			{
				if (pMon->m_byGroup != moral)
					goto fail_return;

				if (tid == pMon->m_sNid)
					goto fail_return;
			}
			else
			{
				if (m_pSrcUser->m_pUserData->m_bNation != moral)
					goto fail_return;

				if (tid == m_pSrcUser->GetSocketID())
					goto fail_return;

				if (pUser->m_bResHpType != USER_DEAD)
					goto fail_return;
			}
			break;
//
		case MORAL_CLAN:		// #14
			if (m_pSrcUser->m_pUserData->m_bKnights == -1
				&& sid != tid)
				goto fail_return;

			if (m_pSrcUser->m_pUserData->m_bNation != moral)
				goto fail_return;

			if (pUser != nullptr
				&& pUser->m_pUserData->m_bKnights != m_pSrcUser->m_pUserData->m_bKnights)
				goto fail_return;
			break;

		case MORAL_CLAN_ALL:	// #15
			break;
//
	}

	// If the user cast the spell (and not the NPC).....
	if (!bFlag)
	{
/*	나중에 반듯이 이 부분 고칠것 !!!
		if( type == MAGIC_CASTING ) {
			if( m_bMagicState == MAGIC_STATE_CASTING && pTable->Type1 != 2 ) goto fail_return;
			if( pTable->bCastTime == 0 )  goto fail_return;
			m_bMagicState = MAGIC_STATE_CASTING;
		}
		else if ( type == MAGIC_EFFECTING && pTable->Type1 != 2 ) {
			if( m_bMagicState == MAGIC_STATE_NONE  && pTable->bCastTime != 0 ) goto fail_return;
		}
*/
		modulator = pTable->Skill % 10;     // Hacking prevention!
		if (modulator != 0)
		{
			Class = pTable->Skill / 10;
			if (Class != m_pSrcUser->m_pUserData->m_sClass)
				goto fail_return;

			if (pTable->SkillLevel > m_pSrcUser->m_pUserData->m_bstrSkill[modulator])
				goto fail_return;
		}
		else if (pTable->SkillLevel > m_pSrcUser->m_pUserData->m_bLevel)
		{
			goto fail_return;
		}

		// MP/SP SUBTRACTION ROUTINE!!! ITEM AND HP TOO!!!
		if (type == MAGIC_EFFECTING)
		{
			// Type 2 related...
			if (pTable->Type1 == 2
				&& pTable->FlyingEffect != 0)
			{
				m_bMagicState = MAGIC_STATE_NONE;
				return pTable;		// Do not reduce MP/SP when flying effect is not 0.
			}

			if (pTable->Type1 == 1
				&& data1 > 1)
			{
				m_bMagicState = MAGIC_STATE_NONE;
				return pTable;		// Do not reduce MP/SP when combo number is higher than 0.
			}

			if (pTable->ManaCost > m_pSrcUser->m_pUserData->m_sMp)
				goto fail_return;
			

/*
			 // Actual deduction of Skill or Magic point.
			if (pTable->Type1 == 1
				|| pTable->Type1 == 2)
			{
				m_pSrcUser->MSpChange(-skill_mana);
			}
			else if (pTable->Type1 != 4
				|| (pTable->Type1 == 4 && tid == -1))
			{
				m_pSrcUser->MSpChange(-pTable->ManaCost);
			}

			// DEDUCTION OF HPs in Magic/Skill using HPs.
			if (pTable->sHP > 0
				&& pTable->ManaCost == 0)
			{
				if (pTable->sHP > m_pSrcUser->m_pUserData->m_sHp)
					goto fail_return;

				m_pSrcUser->HpChange(-pTable->sHP);
			}
*/
			// If the PLAYER uses an item to cast a spell.
			if (pTable->Type1 == 3
				|| pTable->Type1 == 4)
			{
				if (sid >= 0
					&& sid < MAX_USER)
				{
					if (pTable->UseItem != 0)
					{
						// 이것두 성래씨 요청에 의해 하는 짓입니다 --;
						// This checks if such an item exists.
						model::Item* pItem = m_pMain->m_ItemTableMap.GetData(pTable->UseItem);
						if (pItem == nullptr)
							return nullptr;

						if (pItem->Race != 0)
						{
							if (m_pSrcUser->m_pUserData->m_bRace != pItem->Race)
							{
								type = MAGIC_CASTING;
								goto fail_return;
							}
						}

						if (pItem->ClassId != 0)
						{
							if (!m_pSrcUser->JobGroupCheck(pItem->ClassId))
							{
								type = MAGIC_CASTING;
								goto fail_return;
							}
						}

						if (pItem->MinLevel != 0)
						{
							if (m_pSrcUser->m_pUserData->m_bLevel < pItem->MinLevel)
							{
								type = MAGIC_CASTING;
								goto fail_return;
							}
						}

						if (m_pSrcUser->ItemCountChange(pTable->UseItem, 1, 1) < 2)
						{
							type = MAGIC_CASTING;
							goto fail_return;
						}
					}
				}
			}

			if (pTable->Type1 == 5)
			{
				if (tid >= 0
					&& tid < MAX_USER)
				{
					if (pTable->UseItem != 0)
					{
						pType = m_pMain->m_MagicType5TableMap.GetData(magicid);
						if (pType == nullptr)
							goto fail_return;

						CUser* pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[tid];
						if (pTUser == nullptr)
							goto fail_return;

						// 비러머글 부활 --;
						if (pType->Type == 3
							&& pTUser->m_pUserData->m_bLevel <= 5)
						{
							type = MAGIC_CASTING;	// No resurrections for low level users...
							goto fail_return;
						}
						//

						if (pType->Type == 3)
						{
							if (pTUser->ItemCountChange(pTable->UseItem, 1, pType->NeedStone) < 2)
							{
								type = MAGIC_CASTING;
								goto fail_return;
							}
						}
						else
						{
							if (m_pSrcUser->ItemCountChange(pTable->UseItem, 1, pType->NeedStone) < 2)
							{
								type = MAGIC_CASTING;
								goto fail_return;
							}
						}
					}
				}
			}

			// Actual deduction of Skill or Magic point.
			if (pTable->Type1 != 4 || (pTable->Type1 == 4 && tid == -1))
			{
				m_pSrcUser->MSpChange(-pTable->ManaCost);
			}

			// DEDUCTION OF HPs in Magic/Skill using HPs.
			if (pTable->HpCost > 0
				&& pTable->ManaCost == 0)
			{
				if (pTable->HpCost > m_pSrcUser->m_pUserData->m_sHp)
					goto fail_return;

				m_pSrcUser->HpChange(-pTable->HpCost);
			}

			m_bMagicState = MAGIC_STATE_NONE;
		}
	}

	return pTable;      // Magic was successful! 

fail_return:    // In case the magic failed, just send a packet.
	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	SetByte(send_buff, WIZ_MAGIC_PROCESS, send_index);
	SetByte(send_buff, MAGIC_FAIL, send_index);
	SetDWORD(send_buff, magicid, send_index);
	SetShort(send_buff, sid, send_index);
	SetShort(send_buff, tid, send_index);
	SetShort(send_buff, 0, send_index);
	SetShort(send_buff, 0, send_index);
	SetShort(send_buff, 0, send_index);
	if (type == MAGIC_CASTING)
		SetShort(send_buff, -100, send_index);
	else
		SetShort(send_buff, 0, send_index);
	SetShort(send_buff, 0, send_index);
	SetShort(send_buff, 0, send_index);

	if (m_bMagicState == MAGIC_STATE_CASTING)
	{
		if (!bFlag)
		{
			m_pMain->Send_Region(send_buff, send_index, m_pSrcUser->m_pUserData->m_bZone, m_pSrcUser->m_RegionX, m_pSrcUser->m_RegionZ, nullptr, false);
		}
		else
		{
			m_pMain->Send_Region(send_buff, send_index, pMon->m_sCurZone, pMon->m_sRegion_X, pMon->m_sRegion_Z, nullptr, false);
		}
	}
	else
	{
		if (!bFlag)
			m_pSrcUser->Send(send_buff, send_index);
	}

	m_bMagicState = MAGIC_STATE_NONE;
	return nullptr;     // Magic was a failure!
}

BYTE CMagicProcess::ExecuteType1(int magicid, int sid, int tid, int data1, int data2, int data3)   // Applied to an attack skill using a weapon.
{
	int damage = 0, send_index = 0, result = 1;     // Variable initialization. result == 1 : success, 0 : fail
	char send_buff[128] = {};

	model::Magic* pMagic = m_pMain->m_MagicTableMap.GetData(magicid);   // Get main magic table.
	if (pMagic == nullptr)
		return 0;

	model::MagicType1* pType = m_pMain->m_MagicType1TableMap.GetData(magicid);     // Get magic skill table type 1.
	if (pType == nullptr)
	{
		result = 0;
		return result;
	}

	damage = m_pSrcUser->GetDamage(tid, magicid);  // Get damage points of enemy.

	CUser* pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[tid];     // Get target info.  
	if (pTUser == nullptr
		|| pTUser->m_bResHpType == USER_DEAD)
	{
		result = 0;
		goto packet_send;
	}

	pTUser->HpChange(-damage);     // Reduce target health point.

	// Check if the target is dead.
	if (pTUser->m_pUserData->m_sHp == 0)
	{
		pTUser->m_bResHpType = USER_DEAD;     // Target status is officially dead now.

		// sungyong work : loyalty

/* 전쟁존을 위해 임시로 뺌
//		pTUser->ExpChange( -pTUser->m_iMaxExp/100 );     // Reduce target experience.
		if (m_pSrcUser->m_sPartyIndex == -1)     // Something regarding loyalty points.
			m_pSrcUser->LoyaltyChange((pTUser->m_pUserData->m_bLevel * pTUser->m_pUserData->m_bLevel));
		else
			m_pSrcUser->LoyaltyDivide((pTUser->m_pUserData->m_bLevel * pTUser->m_pUserData->m_bLevel));
*/
		// Something regarding loyalty points.
		if (m_pSrcUser->m_sPartyIndex == -1)
			m_pSrcUser->LoyaltyChange(tid);
		else
			m_pSrcUser->LoyaltyDivide(tid);

		m_pSrcUser->GoldChange(tid, 0);

		// 기범이의 완벽한 보호 코딩!!!
		pTUser->InitType3();	// Init Type 3.....
		pTUser->InitType4();	// Init Type 4.....
//
		if (pTUser->m_pUserData->m_bZone != pTUser->m_pUserData->m_bNation
			&& pTUser->m_pUserData->m_bZone < 3)
		{
			pTUser->ExpChange(-pTUser->m_iMaxExp / 100);
			//TRACE(_T("정말로 1%만 깍였다니까요 ㅠ.ㅠ\r\n"));
		}
//
		pTUser->m_sWhoKilledMe = sid;		// Who the hell killed me?
	}

	m_pSrcUser->SendTargetHP(0, tid, -damage);     // Change the HP of the target.

packet_send:
	if (pMagic->Type2 == 0
		|| pMagic->Type2 == 1)
	{
		SetByte(send_buff, WIZ_MAGIC_PROCESS, send_index);
		SetByte(send_buff, MAGIC_EFFECTING, send_index);
		SetDWORD(send_buff, magicid, send_index);
		SetShort(send_buff, sid, send_index);
		SetShort(send_buff, tid, send_index);
		SetShort(send_buff, data1, send_index);
		SetShort(send_buff, data2, send_index);
		SetShort(send_buff, data3, send_index);
		if (damage == 0)
			SetShort(send_buff, -104, send_index);
		else
			SetShort(send_buff, 0, send_index);

		m_pMain->Send_Region(send_buff, send_index, m_pSrcUser->m_pUserData->m_bZone, m_pSrcUser->m_RegionX, m_pSrcUser->m_RegionZ, nullptr, false);
	}

	return result;
}

BYTE CMagicProcess::ExecuteType2(int magicid, int sid, int tid, int data1, int data2, int data3)
{
	int damage = 0, send_index = 0, result = 1; // Variable initialization. result == 1 : success, 0 : fail	
	char send_buff[128] = {};	// For the packet. 

	int total_range = 0;	// These variables are used for range verification!
	int sx, sz, tx, tz;

	model::Magic* pMagic = m_pMain->m_MagicTableMap.GetData(magicid);   // Get main magic table.
	if (pMagic == nullptr)
		return 0;

	model::Item* pTable = nullptr;		// Get item info.
	if (m_pSrcUser->m_pUserData->m_sItemArray[LEFTHAND].nNum)
		pTable = m_pMain->m_ItemTableMap.GetData(m_pSrcUser->m_pUserData->m_sItemArray[LEFTHAND].nNum);
	else
		pTable = m_pMain->m_ItemTableMap.GetData(m_pSrcUser->m_pUserData->m_sItemArray[RIGHTHAND].nNum);

	if (pTable == nullptr)
		return 0;

	model::MagicType2* pType = m_pMain->m_MagicType2TableMap.GetData(magicid);     // Get magic skill table type 2.
	if (pType == nullptr)
		return 0;

	CUser* pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[tid];     // Get target info.  
	if (pTUser == nullptr
		|| pTUser->m_bResHpType == USER_DEAD)
	{
		result = 0;
		goto packet_send;
	}

	total_range = pow(((pType->RangeMod * pTable->Range) / 100), 2);     // Range verification procedure.
//	total_range = pow(((pType->RangeMod / 100) * pTable->Range), 2) ;     // Range verification procedure.

	sx = m_pSrcUser->m_pUserData->m_curx;
	tx = pTUser->m_pUserData->m_curx;

	// Y-AXIS DISABLED TEMPORARILY!!!
	// sy = m_pSrcUser->m_pUserData->m_cury;
	// ty = pTUser->m_pUserData->m_cury;

	sz = m_pSrcUser->m_pUserData->m_curz;
	tz = pTUser->m_pUserData->m_curz;

	// Target is out of range, exit.
	if ((pow((sx - tx), 2) /* + pow((sy - ty), 2)*/ + pow((sz - tz), 2)) > total_range)
	{
		result = 0;
		goto packet_send;
	}

	damage = m_pSrcUser->GetDamage(tid, magicid);  // Get damage points of enemy.	

	pTUser->HpChange(-damage);     // Reduce target health point.

	// Check if the target is dead.
	if (pTUser->m_pUserData->m_sHp == 0)
	{
		pTUser->m_bResHpType = USER_DEAD;     // Target status is officially dead now.

		// sungyong work : loyalty

/* 전쟁존을 위해 임시로 뺌
//		pTUser->ExpChange( -pTUser->m_iMaxExp/100 );     // Reduce target experience.
		if( m_pSrcUser->m_sPartyIndex == -1 )     // Something regarding loyalty points.
			m_pSrcUser->LoyaltyChange( (pTUser->m_pUserData->m_bLevel * pTUser->m_pUserData->m_bLevel) );
		else
			m_pSrcUser->LoyaltyDivide( (pTUser->m_pUserData->m_bLevel * pTUser->m_pUserData->m_bLevel) );
*/

		// Something regarding loyalty points.
		if (m_pSrcUser->m_sPartyIndex == -1)
			m_pSrcUser->LoyaltyChange(tid);
		else
			m_pSrcUser->LoyaltyDivide(tid);

		m_pSrcUser->GoldChange(tid, 0);

		// 기범이의 완벽한 보호 코딩!!!
		pTUser->InitType3();	// Init Type 3.....
		pTUser->InitType4();	// Init Type 4.....
//
		if (pTUser->m_pUserData->m_bZone != pTUser->m_pUserData->m_bNation
			&& pTUser->m_pUserData->m_bZone < 3)
		{
			pTUser->ExpChange(-pTUser->m_iMaxExp / 100);
			//TRACE(_T("정말로 1%만 깍였다니까요 ㅠ.ㅠ\r\n"));
		}
//
		pTUser->m_sWhoKilledMe = sid;		// Who the hell killed me?
	}

	m_pSrcUser->SendTargetHP(0, tid, -damage);     // Change the HP of the target.

packet_send:
	if (pMagic->Type2 == 0
		|| pMagic->Type2 == 2)
	{
		SetByte(send_buff, WIZ_MAGIC_PROCESS, send_index);
		SetByte(send_buff, MAGIC_EFFECTING, send_index);
		SetDWORD(send_buff, magicid, send_index);
		SetShort(send_buff, sid, send_index);
		SetShort(send_buff, tid, send_index);
		SetShort(send_buff, data1, send_index);
		SetShort(send_buff, result, send_index);
		SetShort(send_buff, data3, send_index);

		if (damage == 0)
			SetShort(send_buff, -104, send_index);
		else
			SetShort(send_buff, 0, send_index);

		m_pMain->Send_Region(send_buff, send_index, m_pSrcUser->m_pUserData->m_bZone, m_pSrcUser->m_RegionX, m_pSrcUser->m_RegionZ, nullptr, false);
	}
	return result;
}

void CMagicProcess::ExecuteType3(int magicid, int sid, int tid, int data1, int data2, int data3)  // Applied when a magical attack, healing, and mana restoration is done.
{
	int damage = 0, duration_damage = 0, send_index = 0, result = 1;     // Variable initialization. result == 1 : success, 0 : fail
	char send_buff[128] = {};
	char strLogData[128] = {};
	bool bFlag = false;
	CNpc* pMon = nullptr;

	int casted_member[MAX_USER], party_index = 0;

	for (int h = 0; h < MAX_USER; h++)
		casted_member[h] = -1;

	model::Magic* pMagic = m_pMain->m_MagicTableMap.GetData(magicid);   // Get main magic table.
	if (pMagic == nullptr)
		return;

	model::MagicType3* pType = m_pMain->m_MagicType3TableMap.GetData(magicid);      // Get magic skill table type 3.
	if (pType == nullptr)
		return;

	if (sid >= NPC_BAND)
	{
		bFlag = true;
	
		pMon = m_pMain->m_NpcMap.GetData(sid);
		if (pMon == nullptr
			|| pMon->m_NpcState == NPC_DEAD)
			return;
	}

	// If the target was the source's party....
	if (tid == -1)
	{
		// Maximum number of users in the server....
		for (int i = 0; i < MAX_USER; i++)
		{
			CUser* pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[i];     // Get target info.  
			if (pTUser == nullptr
				|| pTUser->m_bResHpType == USER_DEAD
				// || pTUser->m_bResHpType == USER_BLINKING
				|| pTUser->m_bAbnormalType == ABNORMAL_BLINKING)
				continue;

//			if (UserRegionCheck(sid, i, magicid, pType->Radius))
			if (UserRegionCheck(sid, i, magicid, pType->Radius, data1, data3))
			{
				casted_member[party_index] = i;
				party_index++;
			}
		}

		// If none of the members are in the region, return.
		if (party_index == 0)
		{
			SetByte(send_buff, WIZ_MAGIC_PROCESS, send_index);
			SetByte(send_buff, MAGIC_FAIL, send_index);
			SetDWORD(send_buff, magicid, send_index);
			SetShort(send_buff, sid, send_index);
			SetShort(send_buff, tid, send_index);
			SetShort(send_buff, 0, send_index);
			SetShort(send_buff, 0, send_index);
			SetShort(send_buff, 0, send_index);
			SetShort(send_buff, 0, send_index);
			SetShort(send_buff, 0, send_index);
			SetShort(send_buff, 0, send_index);

			if (!bFlag)
			{
				m_pMain->Send_Region(send_buff, send_index, m_pSrcUser->m_pUserData->m_bZone, m_pSrcUser->m_RegionX, m_pSrcUser->m_RegionZ, nullptr, false);
			}
			else
			{
				m_pMain->Send_Region(send_buff, send_index, pMon->m_sCurZone, pMon->m_sRegion_X, pMon->m_sRegion_Z, nullptr, false);
			}

			return;
		}
	}
	// If the target was another single player.
	else
	{
		CUser* pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[tid];     // Get target info.  

		// Check if target exists.
		if (pTUser == nullptr)
			return;

		casted_member[0] = tid;
		party_index = 1;
	}

	// THIS IS WHERE THE FUN STARTS!!!
	for (int j = 0; j < party_index; j++)
	{
		CUser* pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[casted_member[j]];     // Get target info.
		if (pTUser == nullptr
			|| pTUser->m_bResHpType == USER_DEAD)
			continue;

#if 0 // NOTE: negated by the above check
		// Check if target exists and not already dead.
		if (pTUser == nullptr
			|| pTUser->m_bResHpType == USER_DEAD)
		{
			result = 0;
			goto packet_send;
		}
#endif

		// If you are casting an attack spell.
		if (pType->FirstDamage < 0
			&& pType->DirectType == 1
			&& magicid < 400000)
		{
			damage = GetMagicDamage(sid, casted_member[j], pType->FirstDamage, pType->Attribute);	// Get Magical damage point.
		}
		else
		{
			damage = pType->FirstDamage;
		}

		// 눈싸움전쟁존에서 눈싸움중이라면 공격은 눈을 던지는 것만 가능하도록,,,
		if (m_pSrcUser != nullptr)
		{
			if (m_pSrcUser->m_pUserData->m_bZone == ZONE_SNOW_BATTLE
				&& m_pMain->m_byBattleOpen == SNOW_BATTLE)
				damage = -10;
		}

		// Non-Durational Spells.
		if (pType->Duration == 0)
		{
			if (pType->DirectType == 1)     // Health Point related !
			{
				pTUser->HpChange(damage);     // Reduce target health point.

				// Check if the target is dead.
				if (pTUser->m_pUserData->m_sHp == 0)
				{
					pTUser->m_bResHpType = USER_DEAD;

					// sungyong work : loyalty

					// Killed by monster/NPC.
					if (bFlag)
					{
//
						if (pTUser->m_pUserData->m_bZone != pTUser->m_pUserData->m_bNation
							&& pTUser->m_pUserData->m_bZone < 3)
						{
							pTUser->ExpChange(-pTUser->m_iMaxExp / 100);
							//TRACE(_T("정말로 1%만 깍였다니까요 ㅠ.ㅠ\r\n"));
						}
						else
						{
							pTUser->ExpChange(-pTUser->m_iMaxExp / 20);
						}
//
					}

					// Killed by another player.
					if (!bFlag)
					{
						// Players can only attack with snowballs during snow wars
						if (m_pSrcUser->m_pUserData->m_bZone == ZONE_SNOW_BATTLE
							&& m_pMain->m_byBattleOpen == SNOW_BATTLE)
						{
							m_pSrcUser->GoldGain(SNOW_EVENT_MONEY);	// 10000노아를 주는 부분,,,,,
							spdlog::get(logger::EbenezerEvent)->info("{} killed {}",
								m_pSrcUser->m_pUserData->m_id, pTUser->m_pUserData->m_id);

							if (m_pSrcUser->m_pUserData->m_bZone == ZONE_SNOW_BATTLE
								&& m_pMain->m_byBattleOpen == SNOW_BATTLE)
							{
								if (pTUser->m_pUserData->m_bNation == KARUS)
								{
									++m_pMain->m_sKarusDead;
									//TRACE(_T("++ ExecuteType3 - ka=%d, el=%d\n"), m_pMain->m_sKarusDead, m_pMain->m_sElmoradDead);
								}
								else if (pTUser->m_pUserData->m_bNation == ELMORAD)
								{
									++m_pMain->m_sElmoradDead;
									//TRACE(_T("++ ExecuteType3 - ka=%d, el=%d\n"), m_pMain->m_sKarusDead, m_pMain->m_sElmoradDead);
								}
							}
						}
						else
						{
							// Something regarding loyalty points.
							if (m_pSrcUser->m_sPartyIndex == -1)
								m_pSrcUser->LoyaltyChange(casted_member[j]);
							else
								m_pSrcUser->LoyaltyDivide(casted_member[j]);

							m_pSrcUser->GoldChange(casted_member[j], 0);
						}
					}

					// 기범이의 완벽한 보호 코딩!!!
					pTUser->InitType3();	// Init Type 3.....
					pTUser->InitType4();	// Init Type 4.....

					if (sid >= 0 && sid < MAX_USER)
					{
//
						if (pTUser->m_pUserData->m_bZone != pTUser->m_pUserData->m_bNation
							&& pTUser->m_pUserData->m_bZone < 3)
						{
							pTUser->ExpChange(-pTUser->m_iMaxExp / 100);
							//TRACE(_T("정말로 1%만 깍였다니까요 ㅠ.ㅠ\r\n"));
						}
//
						pTUser->m_sWhoKilledMe = sid;	// Who the hell killed me?
					}
				}

				if (!bFlag)
					m_pSrcUser->SendTargetHP(0, casted_member[j], damage);     // Change the HP of the target.			
			}
			// Magic or Skill Point related !
			else if (pType->DirectType == 2
				|| pType->DirectType == 3)
				pTUser->MSpChange(damage);     // Change the SP or the MP of the target.		
			// Armor Durability related.
			else if (pType->DirectType == 4)
				pTUser->ItemWoreOut(DURABILITY_TYPE_DEFENCE, -damage);     // Reduce Slot Item Durability
		}
		// Durational Spells! Remember, durational spells only involve HPs.
		else if (pType->Duration != 0)
		{
			// In case there was first damage......
			if (damage != 0)
			{
				pTUser->HpChange(damage);			// Initial damage!!!

				// Check if the target is dead.	
				if (pTUser->m_pUserData->m_sHp == 0)
				{
					pTUser->m_bResHpType = USER_DEAD;

					// sungyong work : loyalty

					// Killed by monster/NPC.
					if (bFlag)
					{
//
						if (pTUser->m_pUserData->m_bZone != pTUser->m_pUserData->m_bNation
							&& pTUser->m_pUserData->m_bZone < 3)
						{
							pTUser->ExpChange(-pTUser->m_iMaxExp / 100);
							//TRACE(_T("정말로 1%만 깍였다니까요 ㅠ.ㅠ\r\n"));
						}
						else
						{
							pTUser->ExpChange(-pTUser->m_iMaxExp / 20);
						}
//
					}

					// Killed by another player.
					if (!bFlag)
					{
						// Something regarding loyalty points.
						if (m_pSrcUser->m_sPartyIndex == -1)
							m_pSrcUser->LoyaltyChange(casted_member[j]);
						else
							m_pSrcUser->LoyaltyDivide(casted_member[j]);

						m_pSrcUser->GoldChange(casted_member[j], 0);
					}

					// 기범이의 완벽한 보호 코딩 !!!
					pTUser->InitType3();	// Init Type 3.....
					pTUser->InitType4();	// Init Type 4..... 

					if (sid >= 0
						&& sid < MAX_USER)
					{
//
						if (pTUser->m_pUserData->m_bZone != pTUser->m_pUserData->m_bNation
							&& pTUser->m_pUserData->m_bZone < 3)
						{
							pTUser->ExpChange(-pTUser->m_iMaxExp / 100);
							//TRACE(_T("정말로 1%만 깍였다니까요 ㅠ.ㅠ\r\n"));
						}
//
						pTUser->m_sWhoKilledMe = sid;	// Who the hell killed me?
					}
				}

				if (!bFlag)
					m_pSrcUser->SendTargetHP(0, casted_member[j], damage);     // Change the HP of the target. 
			}

			// 여기도 보호 코딩 했슴...
			if (pTUser->m_bResHpType != USER_DEAD)
			{
				if (pType->TimeDamage < 0)
					duration_damage = GetMagicDamage(sid, casted_member[j], pType->TimeDamage, pType->Attribute);
				else
					duration_damage = pType->TimeDamage;

				// For continuous damages...
				for (int k = 0; k < MAX_TYPE3_REPEAT; k++)
				{
					if (pTUser->m_bHPInterval[k] == 5)
					{
						pTUser->m_fHPStartTime[k] = pTUser->m_fHPLastTime[k] = TimeGet();     // The durational magic routine.
						pTUser->m_bHPDuration[k] = pType->Duration;
						pTUser->m_bHPInterval[k] = 2;
						pTUser->m_bHPAmount[k] = duration_damage / (pTUser->m_bHPDuration[k] / pTUser->m_bHPInterval[k]);
						pTUser->m_sSourceID[k] = sid;
						break;
					}
				}

				pTUser->m_bType3Flag = true;
			}
//
			//	Send Party Packet.....
			if (pTUser->m_sPartyIndex != -1
				&& pType->TimeDamage < 0)
			{
				SetByte(send_buff, WIZ_PARTY, send_index);
				SetByte(send_buff, PARTY_STATUSCHANGE, send_index);
				SetShort(send_buff, casted_member[j], send_index);
				SetByte(send_buff, 1, send_index);
				SetByte(send_buff, 0x01, send_index);
				m_pMain->Send_PartyMember(pTUser->m_sPartyIndex, send_buff, send_index);
				memset(send_buff, 0, sizeof(send_buff));
				send_index = 0;
			}
			//  end of Send Party Packet......//
//
		}

	packet_send:
		if (pMagic->Type2 == 0
			|| pMagic->Type2 == 3)
		{
			SetByte(send_buff, WIZ_MAGIC_PROCESS, send_index);
			SetByte(send_buff, MAGIC_EFFECTING, send_index);
			SetDWORD(send_buff, magicid, send_index);
			SetShort(send_buff, sid, send_index);
			SetShort(send_buff, casted_member[j], send_index);
			SetShort(send_buff, data1, send_index);
			SetShort(send_buff, result, send_index);
			SetShort(send_buff, data3, send_index);
			if (!bFlag)
			{
				m_pMain->Send_Region(send_buff, send_index, m_pSrcUser->m_pUserData->m_bZone, m_pSrcUser->m_RegionX, m_pSrcUser->m_RegionZ, nullptr, false);
			}
			else
			{
				m_pMain->Send_Region(send_buff, send_index, pMon->m_sCurZone, pMon->m_sRegion_X, pMon->m_sRegion_Z, nullptr, false);
			}
		}

		// Heal magic
		if (pType->DirectType == 1
			&& damage > 0)
		{
			if (!bFlag)
			{
				if (sid != tid)
				{
					memset(send_buff, 0, sizeof(send_buff));
					send_index = 0;
					SetByte(send_buff, AG_HEAL_MAGIC, send_index);
					SetShort(send_buff, sid, send_index);
					m_pMain->Send_AIServer(m_pSrcUser->m_pUserData->m_bZone, send_buff, send_index);
				}
			}
		}

		result = 1;
		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;
	}

	return;
}

void CMagicProcess::ExecuteType4(int magicid, int sid, int tid, int data1, int data2, int data3)
{
	int damage = 0, send_index = 0, result = 1;     // Variable initialization. result == 1 : success, 0 : fail
	char send_buff[128] = {};

	int casted_member[MAX_USER], party_index = 0;

	for (int h = 0; h < MAX_USER; h++)
		casted_member[h] = -1;

	model::Magic* pMagic = m_pMain->m_MagicTableMap.GetData(magicid);   // Get main magic table.
	if (pMagic == nullptr)
		return;

	model::MagicType4* pType = m_pMain->m_MagicType4TableMap.GetData(magicid);     // Get magic skill table type 4.
	if (pType == nullptr)
		return;

	// If the target was the source's party......
	if (tid == -1)
	{
		// Maximum number of members in a party...
		for (int i = 0; i < MAX_USER; i++)
		{
			CUser* pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[i];     // Get target info.  
			if (pTUser == nullptr
				|| pTUser->m_bResHpType == USER_DEAD
				// || pTUser->m_bResHpType == USER_BLINKING
				|| pTUser->m_bAbnormalType == ABNORMAL_BLINKING)
				continue;

//			if (UserRegionCheck(sid, i, magicid, pType->Radius))
			if (UserRegionCheck(sid, i, magicid, pType->Radius, data1, data3))
			{
				casted_member[party_index] = i;
				party_index++;
			}
		}

		// If none of the members are in the region, return.
		if (party_index == 0)
		{
			SetByte(send_buff, WIZ_MAGIC_PROCESS, send_index);
			SetByte(send_buff, MAGIC_FAIL, send_index);
			SetDWORD(send_buff, magicid, send_index);
			SetShort(send_buff, sid, send_index);
			SetShort(send_buff, tid, send_index);
			SetShort(send_buff, 0, send_index);
			SetShort(send_buff, 0, send_index);
			SetShort(send_buff, 0, send_index);
			SetShort(send_buff, 0, send_index);
			SetShort(send_buff, 0, send_index);
			SetShort(send_buff, 0, send_index);
			if (sid >= 0
				&& sid < MAX_USER)
			{
				m_pMain->Send_Region(send_buff, send_index, m_pSrcUser->m_pUserData->m_bZone, m_pSrcUser->m_RegionX, m_pSrcUser->m_RegionZ, nullptr, false);
			}

			return;
		}
	}
	// If the target was another single player.
	else
	{
		CUser* pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[tid];     // Get target info.

		// Check if target exists.
		if (pTUser == nullptr)
			return;

		casted_member[0] = tid;
		party_index = 1;
	}

	// THIS IS WHERE THE FUN STARTS!!!
	for (int j = 0; j < party_index; j++)
	{
		CUser* pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[casted_member[j]];     // Get target info.  
		if (pTUser == nullptr
			|| pTUser->m_bResHpType == USER_DEAD)
			continue;

		// Is this buff-type already casted on the player?
		if (pTUser->m_bType4Buff[pType->BuffType - 1] == 2
			&& tid == -1)
		{
			result = 0;				// If so, fail! 
			goto fail_return;
		}

		// Depending on which buff-type it is.....
		switch (pType->BuffType)
		{
			case 1:
				pTUser->m_sMaxHPAmount = pType->MaxHp;		// Get the amount that will be added/multiplied.
				pTUser->m_sDuration1 = pType->Duration;	// Get the duration time.
				pTUser->m_fStartTime1 = TimeGet();			// Get the time when Type 4 spell starts.	
				break;

			case 2:
				pTUser->m_sACAmount = pType->Armor;
				pTUser->m_sDuration2 = pType->Duration;
				pTUser->m_fStartTime2 = TimeGet();
				break;

			case 3:
				// Bezoar!!!
				if (magicid == 490034)
				{
					memset(send_buff, 0, sizeof(send_buff));
					send_index = 0;
					SetByte(send_buff, 3, send_index);	// You are now a giant!!!
					SetByte(send_buff, ABNORMAL_GIANT, send_index);
					pTUser->StateChange(send_buff);
					memset(send_buff, 0, sizeof(send_buff));
					send_index = 0;
				}
				// Rice Cake!!!
				else if (magicid == 490035)
				{
					memset(send_buff, 0, sizeof(send_buff));
					send_index = 0;
					SetByte(send_buff, 3, send_index);	// You are now a dwarf!!!
					SetByte(send_buff, ABNORMAL_DWARF, send_index);
					pTUser->StateChange(send_buff);
					memset(send_buff, 0, sizeof(send_buff));
					send_index = 0;
				}

				pTUser->m_sDuration3 = pType->Duration;
				pTUser->m_fStartTime3 = TimeGet();
				break;

			case 4:
				pTUser->m_bAttackAmount = pType->AttackPower;
				pTUser->m_sDuration4 = pType->Duration;
				pTUser->m_fStartTime4 = TimeGet();
				break;

			case 5:
				pTUser->m_bAttackSpeedAmount = pType->AttackSpeed;
				pTUser->m_sDuration5 = pType->Duration;
				pTUser->m_fStartTime5 = TimeGet();
				break;

			case 6:
				pTUser->m_bSpeedAmount = pType->Speed;
				pTUser->m_sDuration6 = pType->Duration;
				pTUser->m_fStartTime6 = TimeGet();
				break;

			case 7:
				pTUser->m_bStrAmount = pType->Strength;
				pTUser->m_bStaAmount = pType->Stamina;
				pTUser->m_bDexAmount = pType->Dexterity;
				pTUser->m_bIntelAmount = pType->Intelligence;
				pTUser->m_bChaAmount = pType->Charisma;
				pTUser->m_sDuration7 = pType->Duration;
				pTUser->m_fStartTime7 = TimeGet();
				break;

			case 8:
				pTUser->m_bFireRAmount = pType->FireResist;
				pTUser->m_bColdRAmount = pType->ColdResist;
				pTUser->m_bLightningRAmount = pType->LightningResist;
				pTUser->m_bMagicRAmount = pType->MagicResist;
				pTUser->m_bDiseaseRAmount = pType->DiseaseResist;
				pTUser->m_bPoisonRAmount = pType->PoisonResist;
				pTUser->m_sDuration8 = pType->Duration;
				pTUser->m_fStartTime8 = TimeGet();
				break;

			case 9:
				pTUser->m_bHitRateAmount = pType->HitRate;
				pTUser->m_sAvoidRateAmount = pType->AvoidRate;
				pTUser->m_sDuration9 = pType->Duration;
				pTUser->m_fStartTime9 = TimeGet();
				break;

			default:
				result = 0;
				goto fail_return;
		}

		if (tid != -1
			&& pMagic->Type1 == 4)
		{
			// 비러머글 하피 >.<
			if (sid >= 0
				&& sid < MAX_USER)
				m_pSrcUser->MSpChange(-(pMagic->ManaCost));
		}

		if (sid >= 0 && sid < MAX_USER)
		{
			if (m_pSrcUser->m_pUserData->m_bNation == pTUser->m_pUserData->m_bNation)
				pTUser->m_bType4Buff[pType->BuffType - 1] = 2;
			else
				pTUser->m_bType4Buff[pType->BuffType - 1] = 1;
		}
		else
		{
			pTUser->m_bType4Buff[pType->BuffType - 1] = 1;
		}

		pTUser->m_bType4Flag = true;

		pTUser->SetSlotItemValue();				// Update character stats.
		pTUser->SetUserAbility();
//
		//	Send Party Packet.....	
		if (pTUser->m_sPartyIndex != -1
			&& pTUser->m_bType4Buff[pType->BuffType - 1] == 1)
		{
			SetByte(send_buff, WIZ_PARTY, send_index);
			SetByte(send_buff, PARTY_STATUSCHANGE, send_index);
			SetShort(send_buff, tid, send_index);
			SetByte(send_buff, 2, send_index);
			SetByte(send_buff, 0x01, send_index);
			m_pMain->Send_PartyMember(pTUser->m_sPartyIndex, send_buff, send_index);
			memset(send_buff, 0, sizeof(send_buff));
			send_index = 0;
		}
		//  end of Send Party Packet.....//
//	
		pTUser->Send2AI_UserUpdateInfo();	// AI Server에 바끤 데이타 전송....

		if (pMagic->Type2 == 0
			|| pMagic->Type2 == 4)
		{
			SetByte(send_buff, WIZ_MAGIC_PROCESS, send_index);
			SetByte(send_buff, MAGIC_EFFECTING, send_index);
			SetDWORD(send_buff, magicid, send_index);
			SetShort(send_buff, sid, send_index);
			SetShort(send_buff, casted_member[j], send_index);
			SetShort(send_buff, data1, send_index);
			SetShort(send_buff, result, send_index);
			SetShort(send_buff, data3, send_index);
			if (sid >= 0
				&& sid < MAX_USER)
			{
				m_pMain->Send_Region(send_buff, send_index, m_pSrcUser->m_pUserData->m_bZone, m_pSrcUser->m_RegionX, m_pSrcUser->m_RegionZ, nullptr, false);
			}
			else
			{
				m_pMain->Send_Region(send_buff, send_index, pTUser->m_pUserData->m_bZone, pTUser->m_RegionX, pTUser->m_RegionZ, nullptr, false);
			}
		}

		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;
		result = 1;
		continue;

	fail_return:
		if (pMagic->Type2 == 4)
		{
			SetByte(send_buff, WIZ_MAGIC_PROCESS, send_index);
			SetByte(send_buff, MAGIC_EFFECTING, send_index);
			SetDWORD(send_buff, magicid, send_index);
			SetShort(send_buff, sid, send_index);
			SetShort(send_buff, casted_member[j], send_index);
			SetShort(send_buff, data1, send_index);
			SetShort(send_buff, result, send_index);
			SetShort(send_buff, data3, send_index);
			if (sid >= 0 && sid < MAX_USER)
			{
				m_pMain->Send_Region(send_buff, send_index, m_pSrcUser->m_pUserData->m_bZone, m_pSrcUser->m_RegionX, m_pSrcUser->m_RegionZ, nullptr, false);
			}
			else
			{
				m_pMain->Send_Region(send_buff, send_index, pTUser->m_pUserData->m_bZone, pTUser->m_RegionX, pTUser->m_RegionZ, nullptr, false);
			}

			memset(send_buff, 0, sizeof(send_buff));
			send_index = 0;
		}

		if (sid >= 0 && sid < MAX_USER)
		{
			SetByte(send_buff, WIZ_MAGIC_PROCESS, send_index);
			SetByte(send_buff, MAGIC_FAIL, send_index);
			SetDWORD(send_buff, magicid, send_index);
			SetShort(send_buff, sid, send_index);
			SetShort(send_buff, casted_member[j], send_index);
			SetShort(send_buff, 0, send_index);
			SetShort(send_buff, 0, send_index);
			SetShort(send_buff, 0, send_index);
			SetShort(send_buff, 0, send_index);
			SetShort(send_buff, 0, send_index);
			SetShort(send_buff, 0, send_index);
			m_pSrcUser->Send(send_buff, send_index);
		}

		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;
		result = 1;
		continue;
	}
}

void CMagicProcess::ExecuteType5(int magicid, int sid, int tid, int data1, int data2, int data3)
{
	int damage = 0, send_index = 0, result = 1;     // Variable initialization. result == 1 : success, 0 : fail
	char send_buff[128] = {};
	int i = 0, j = 0, k = 0, buff_test = 0;
	bool bType3Test = true, bType4Test = true;

	model::Magic* pMagic = m_pMain->m_MagicTableMap.GetData(magicid);   // Get main magic table.
	if (pMagic == nullptr)
		return;

	model::MagicType5* pType = m_pMain->m_MagicType5TableMap.GetData(magicid);     // Get magic skill table type 4.
	if (pType == nullptr)
		return;

	CUser* pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[tid];     // Get target info.
	if (pTUser == nullptr)
		return;

	if (pTUser->m_bResHpType == USER_DEAD)
	{
		if (pType->Type != RESURRECTION)
			return;
	}
	else
	{
		if (pType->Type == RESURRECTION)
			return;
	}

	switch (pType->Type)
	{
		// REMOVE TYPE 3!!!
		case REMOVE_TYPE3:
			for (i = 0; i < MAX_TYPE3_REPEAT; i++)
			{
				if (pTUser->m_bHPAmount[i] < 0)
				{
					pTUser->m_fHPStartTime[i] = 0.0f;
					pTUser->m_fHPLastTime[i] = 0.0f;
					pTUser->m_bHPAmount[i] = 0;
					pTUser->m_bHPDuration[i] = 0;
					pTUser->m_bHPInterval[i] = 5;
					pTUser->m_sSourceID[i] = -1;

					memset(send_buff, 0, sizeof(send_buff));
					send_index = 0;
					SetByte(send_buff, WIZ_MAGIC_PROCESS, send_index);
					SetByte(send_buff, MAGIC_TYPE3_END, send_index);
					SetByte(send_buff, 200, send_index);		// REMOVE ALL!!!
					pTUser->Send(send_buff, send_index);
				}
			}

			buff_test = 0;
			for (j = 0; j < MAX_TYPE3_REPEAT; j++)
				buff_test += pTUser->m_bHPDuration[j];

			if (buff_test == 0)
				pTUser->m_bType3Flag = false;

			// Check for Type 3 Curses.
			for (k = 0; k < MAX_TYPE3_REPEAT; k++)
			{
				if (pTUser->m_bHPAmount[k] < 0)
				{
					bType3Test = false;
					break;
				}
			}

			// Send Party Packet....   
			if (pTUser->m_sPartyIndex != -1
				&& bType3Test)
			{
				memset(send_buff, 0, sizeof(send_buff));
				send_index = 0;
				SetByte(send_buff, WIZ_PARTY, send_index);
				SetByte(send_buff, PARTY_STATUSCHANGE, send_index);
				SetShort(send_buff, tid, send_index);
				SetByte(send_buff, 1, send_index);
				SetByte(send_buff, 0x00, send_index);
				m_pMain->Send_PartyMember(pTUser->m_sPartyIndex, send_buff, send_index);
			}
			//  end of Send Party Packet.....  //
//
			break;

		// REMOVE TYPE 4!!!
		case REMOVE_TYPE4:
			if (pTUser->m_bType4Buff[0] == 1)
			{
				pTUser->m_sDuration1 = 0;
				pTUser->m_fStartTime1 = 0.0f;
				pTUser->m_sMaxHPAmount = 0;
				pTUser->m_bType4Buff[0] = 0;

				SendType4BuffRemove(tid, 1);
			}

			if (pTUser->m_bType4Buff[1] == 1)
			{
				pTUser->m_sDuration2 = 0;
				pTUser->m_fStartTime2 = 0.0f;
				pTUser->m_sACAmount = 0;
				pTUser->m_bType4Buff[1] = 0;

				SendType4BuffRemove(tid, 2);
			}

			if (pTUser->m_bType4Buff[3] == 1)
			{
				pTUser->m_sDuration4 = 0;
				pTUser->m_fStartTime4 = 0.0f;
				pTUser->m_bAttackAmount = 100;
				pTUser->m_bType4Buff[3] = 0;

				SendType4BuffRemove(tid, 4);
			}

			if (pTUser->m_bType4Buff[4] == 1)
			{
				pTUser->m_sDuration5 = 0;
				pTUser->m_fStartTime5 = 0.0f;
				pTUser->m_bAttackSpeedAmount = 100;
				pTUser->m_bType4Buff[4] = 0;

				SendType4BuffRemove(tid, 5);
			}

			if (pTUser->m_bType4Buff[5] == 1)
			{
				pTUser->m_sDuration6 = 0;
				pTUser->m_fStartTime6 = 0.0f;
				pTUser->m_bSpeedAmount = 100;
				pTUser->m_bType4Buff[5] = 0;

				SendType4BuffRemove(tid, 6);
			}

			if (pTUser->m_bType4Buff[6] == 1)
			{
				pTUser->m_sDuration7 = 0;
				pTUser->m_fStartTime7 = 0.0f;
				pTUser->m_bStrAmount = 0;
				pTUser->m_bStaAmount = 0;
				pTUser->m_bDexAmount = 0;
				pTUser->m_bIntelAmount = 0;
				pTUser->m_bChaAmount = 0;
				pTUser->m_bType4Buff[6] = 0;

				SendType4BuffRemove(tid, 7);
			}

			if (pTUser->m_bType4Buff[7] == 1)
			{
				pTUser->m_sDuration8 = 0;
				pTUser->m_fStartTime8 = 0.0f;
				pTUser->m_bFireRAmount = 0;
				pTUser->m_bColdRAmount = 0;
				pTUser->m_bLightningRAmount = 0;
				pTUser->m_bMagicRAmount = 0;
				pTUser->m_bDiseaseRAmount = 0;
				pTUser->m_bPoisonRAmount = 0;
				pTUser->m_bType4Buff[7] = 0;

				SendType4BuffRemove(tid, 8);
			}

			if (pTUser->m_bType4Buff[8] == 1)
			{
				pTUser->m_sDuration9 = 0;
				pTUser->m_fStartTime9 = 0.0f;
				pTUser->m_bHitRateAmount = 100;
				pTUser->m_sAvoidRateAmount = 100;
				pTUser->m_bType4Buff[8] = 0;

				SendType4BuffRemove(tid, 9);
			}

			pTUser->SetSlotItemValue();
			pTUser->SetUserAbility();
			pTUser->Send2AI_UserUpdateInfo();	// AI Server에 바끤 데이타 전송....

			/*	Send Party Packet.....
			if (m_sPartyIndex != -1) {
				SetByte( send_buff, WIZ_PARTY, send_index );
				SetByte( send_buff, PARTY_STATUSCHANGE, send_index );
				SetShort( send_buff, m_Sid, send_index );
	//			if (buff_type != 5 && buff_type != 6) {
	//				SetByte( send_buff, 3, send_index );
	//			}
	//			else {
				SetByte( send_buff, 2, send_index );
	//			}
				SetByte( send_buff, 0x00, send_index);
				m_pMain->Send_PartyMember(m_sPartyIndex, send_buff, send_index);
				memset(send_buff, 0, sizeof(send_buff));
				send_index = 0;
			}
			//  end of Send Party Packet.....  */

			buff_test = 0;
			for (i = 0; i < MAX_TYPE4_BUFF; i++)
				buff_test += pTUser->m_bType4Buff[i];

			if (buff_test == 0)
				pTUser->m_bType4Flag = false;

			bType4Test = true;
			for (j = 0; j < MAX_TYPE4_BUFF; j++)
			{
				if (pTUser->m_bType4Buff[j] == 1)
				{
					bType4Test = false;
					break;
				}
			}
//
			// Send Party Packet.....
			if (pTUser->m_sPartyIndex != -1
				&& bType4Test)
			{
				memset(send_buff, 0, sizeof(send_buff));
				send_index = 0;
				SetByte(send_buff, WIZ_PARTY, send_index);
				SetByte(send_buff, PARTY_STATUSCHANGE, send_index);
				SetShort(send_buff, tid, send_index);
				SetByte(send_buff, 2, send_index);
				SetByte(send_buff, 0x00, send_index);
				m_pMain->Send_PartyMember(pTUser->m_sPartyIndex, send_buff, send_index);

				memset(send_buff, 0, sizeof(send_buff));
				send_index = 0;
			}
			//  end of Send Party Packet.....  //
//
			break;

		case RESURRECTION:		// RESURRECT A DEAD PLAYER!!!
			memset(send_buff, 0, sizeof(send_buff));
			send_index = 0;
			SetByte(send_buff, 1, send_index);
			pTUser->Regene(send_buff, magicid);

			memset(send_buff, 0, sizeof(send_buff));
			send_index = 0;
			break;

		case REMOVE_BLESS:
			if (pTUser->m_bType4Buff[0] == 2)
			{
				pTUser->m_sDuration1 = 0;
				pTUser->m_fStartTime1 = 0.0f;
				pTUser->m_sMaxHPAmount = 0;
				pTUser->m_bType4Buff[0] = 0;

				SendType4BuffRemove(tid, 1);

				pTUser->SetSlotItemValue();
				pTUser->SetUserAbility();
				pTUser->Send2AI_UserUpdateInfo();	// AI Server에 바끤 데이타 전송....

				buff_test = 0;
				for (i = 0; i < MAX_TYPE4_BUFF; i++)
					buff_test += pTUser->m_bType4Buff[i];

				if (buff_test == 0)
					pTUser->m_bType4Flag = false;

				bType4Test = true;
				for (j = 0; j < MAX_TYPE4_BUFF; j++)
				{
					if (pTUser->m_bType4Buff[j] == 1)
					{
						bType4Test = false;
						break;
					}
				}

				// Send Party Packet.....
				if (pTUser->m_sPartyIndex != -1 && bType4Test)
				{
					memset(send_buff, 0, sizeof(send_buff));
					send_index = 0;
					SetByte(send_buff, WIZ_PARTY, send_index);
					SetByte(send_buff, PARTY_STATUSCHANGE, send_index);
					SetShort(send_buff, tid, send_index);
					SetByte(send_buff, 2, send_index);
					SetByte(send_buff, 0x00, send_index);
					m_pMain->Send_PartyMember(pTUser->m_sPartyIndex, send_buff, send_index);

					memset(send_buff, 0, sizeof(send_buff));
					send_index = 0;
				}
				//  end of Send Party Packet.....  //
			}
			break;
	}

	// In case of success!!!
	if (pMagic->Type2 == 0
		|| pMagic->Type2 == 5)
	{
		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;
		SetByte(send_buff, WIZ_MAGIC_PROCESS, send_index);
		SetByte(send_buff, MAGIC_EFFECTING, send_index);
		SetDWORD(send_buff, magicid, send_index);
		SetShort(send_buff, sid, send_index);
		SetShort(send_buff, tid, send_index);
		SetShort(send_buff, data1, send_index);
		SetShort(send_buff, result, send_index);
		SetShort(send_buff, data3, send_index);

		if (sid >= 0
			&& sid < MAX_USER)
		{
			m_pMain->Send_Region(send_buff, send_index, m_pSrcUser->m_pUserData->m_bZone, m_pSrcUser->m_RegionX, m_pSrcUser->m_RegionZ, nullptr, false);
		}
		else
		{
			m_pMain->Send_Region(send_buff, send_index, pTUser->m_pUserData->m_bZone, pTUser->m_RegionX, pTUser->m_RegionZ, nullptr, false);
		}
	}

	return;

fail_return:
	if (sid >= 0
		&& sid < MAX_USER)
	{
		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;
		SetByte(send_buff, WIZ_MAGIC_PROCESS, send_index);		// In case of failure!!!
		SetByte(send_buff, MAGIC_FAIL, send_index);
		SetDWORD(send_buff, magicid, send_index);
		SetShort(send_buff, sid, send_index);
		SetShort(send_buff, tid, send_index);
		SetShort(send_buff, 0, send_index);
		SetShort(send_buff, 0, send_index);
		SetShort(send_buff, 0, send_index);
		SetShort(send_buff, 0, send_index);
		SetShort(send_buff, 0, send_index);
		SetShort(send_buff, 0, send_index);
		m_pSrcUser->Send(send_buff, send_index);
	}
}

void CMagicProcess::ExecuteType6(int magicid)
{
}

void CMagicProcess::ExecuteType7(int magicid)
{
}

void CMagicProcess::ExecuteType8(int magicid, int sid, int tid, int data1, int data2, int data3)	// Warp, resurrection, and summon spells.
{
	int damage = 0, send_index = 0, result = 1;     // Variable initialization. result == 1 : success, 0 : fail
	char send_buff[128] = {};

	int casted_member[MAX_USER], party_index = 0;

	for (int h = 0; h < MAX_USER; h++)
		casted_member[h] = -1;

	model::MagicType8* pType = m_pMain->m_MagicType8TableMap.GetData(magicid);      // Get magic skill table type 8.
	if (pType == nullptr)
		return;

	// If the target was the source's party...
	if (tid == -1)
	{
		// Maximum number of members in a party...
		for (int i = 0; i < MAX_USER; i++)
		{
			CUser* pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[i];     // Get target info.

			// Check if target exists.
			if (pTUser == nullptr)
				continue;

//			if (UserRegionCheck(sid, i, magicid, pType->Radius))
			if (UserRegionCheck(sid, i, magicid, pType->Radius, data1, data3))
			{
				casted_member[party_index] = i;
				party_index++;
			}
		}

		// If none of the members are in the region, return.
		if (party_index == 0)
			return;
	}
	// If the target was another single player.
	else
	{
		CUser* pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[tid];     // Get target info.  

		// Check if target exists.
		if (pTUser == nullptr)
			return;

		casted_member[0] = tid;
		party_index = 1;
	}

	// THIS IS WHERE THE FUN STARTS!!!
	for (int j = 0; j < party_index; j++)
	{
		_OBJECT_EVENT* pEvent = nullptr;
		C3DMap* pTMap = nullptr;

		float x = 0.0f, z = 0.0f;
		x = (float) (myrand(0, 400) / 100.0f);
		z = (float) (myrand(0, 400) / 100.0f);

		if (x < 2.5f)
			x += 1.5f;

		if (z < 2.5f)
			z += 1.5f;

		CUser* pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[casted_member[j]];     // Get target info.
		if (pTUser == nullptr)
			continue;

		pTMap = m_pMain->GetMapByIndex(pTUser->m_iZoneIndex);
		if (pTMap == nullptr)
			continue;

		// 비러머글 대만 써비스 >.<
		model::Home* pHomeInfo = m_pMain->m_HomeTableMap.GetData(pTUser->m_pUserData->m_bNation);
		if (pHomeInfo == nullptr)
			return;

		// Warp or Summon related.
		if (pType->WarpType != 11)
		{
			// Check if target is not already dead.
			if (pTUser->m_bResHpType == USER_DEAD)
			{
				result = 0;
				goto packet_send;
			}
		}
		// Resurrection related.
		else
		{
			// Check if target is not already alive.
			if (pTUser->m_bResHpType != USER_DEAD)
			{
				result = 0;
				goto packet_send;
			}
		}

		// Is target already warping?
		if (pTUser->m_bWarp)
		{
			result = 0;
			goto packet_send;
		}

		switch (pType->WarpType)
		{
			// Send source to resurrection point.
			case 1:
				SetByte(send_buff, WIZ_MAGIC_PROCESS, send_index);
				SetByte(send_buff, MAGIC_EFFECTING, send_index);
				SetDWORD(send_buff, magicid, send_index);
				SetShort(send_buff, sid, send_index);
				SetShort(send_buff, casted_member[j], send_index);
				SetShort(send_buff, data1, send_index);
				SetShort(send_buff, result, send_index);
				SetShort(send_buff, data3, send_index);
				m_pMain->Send_Region(send_buff, send_index, pTUser->m_pUserData->m_bZone, pTUser->m_RegionX, pTUser->m_RegionZ, nullptr, false);

				send_index = 0;
				memset(send_buff, 0, sizeof(send_buff));
				/*
				pTUser->Regene(nullptr);	// Use Regene() to warp player to resurrection point.
*/

				pEvent = pTMap->GetObjectEvent(pTUser->m_pUserData->m_sBind);
				if (pEvent != nullptr)
				{

//					m_pUserData->m_curx = m_fWill_x = pEvent->fPosX + x;
//					m_pUserData->m_curz = m_fWill_z = pEvent->fPosZ + z;
//					m_pUserData->m_cury = 0;

					SetShort(send_buff, (WORD) ((pEvent->fPosX * 10) + x), send_index);
					SetShort(send_buff, (WORD) ((pEvent->fPosZ * 10) + z), send_index);
					pTUser->Warp(send_buff);

				}
				// User is in different zone.
				else if (pTUser->m_pUserData->m_bNation != pTUser->m_pUserData->m_bZone
					&& pTUser->m_pUserData->m_bZone < 3)
				{
					// Land of Karus
					if (pTUser->m_pUserData->m_bNation == 1)
					{

//						m_pUserData->m_curx = m_fWill_x = (float)852.0 + x;
//						m_pUserData->m_curz = m_fWill_z = (float)164.0 + z;

						SetShort(send_buff, 852 + x, send_index);
						SetShort(send_buff, 164 + z, send_index);
						pTUser->Warp(send_buff);
					}
					// Land of Elmorad
					else
					{

//						m_pUserData->m_curx = m_fWill_x = (float)177.0 + x;
//						m_pUserData->m_curz = m_fWill_z = (float)923.0 + z;

						SetShort(send_buff, 177 + x, send_index);
						SetShort(send_buff, 923 + z, send_index);
						pTUser->Warp(send_buff);
					}
				}
// 비러머글 대만 써비스 >.<
				// 전쟁존 --;
				// 개척존 --;
				else if (pTUser->m_pUserData->m_bZone == ZONE_BATTLE)
				{
					SetShort(send_buff, (WORD) ((pHomeInfo->BattleZoneX * 10) + x), send_index);
					SetShort(send_buff, (WORD) ((pHomeInfo->BattleZoneZ * 10) + z), send_index);
					pTUser->Warp(send_buff);
				}
				else if (pTUser->m_pUserData->m_bZone == ZONE_FRONTIER)
				{
					SetShort(send_buff, (WORD) ((pHomeInfo->FreeZoneX * 10) + x), send_index);
					SetShort(send_buff, (WORD) ((pHomeInfo->FreeZoneZ * 10) + z), send_index);
					pTUser->Warp(send_buff);
				}
				// No, I don't have any idea what this part means....
				else
				{
					SetShort(send_buff, (WORD) ((pTMap->m_fInitX * 10) + x), send_index);
					SetShort(send_buff, (WORD) ((pTMap->m_fInitZ * 10) + z), send_index);
					pTUser->Warp(send_buff);
				}

				send_index = 0;
				memset(send_buff, 0, sizeof(send_buff));
				break;

			// Send target to teleport point WITHIN the zone.
			case 2:
				// LATER!!!
				break;

			// Send target to teleport point OUTSIDE the zone.
			case 3:
				// LATER!!!
				break;

			// Send target to a hidden zone.
			case 5:
				// LATER!!!
				break;

			// Resurrect a dead player.
			case 11:
				SetByte(send_buff, WIZ_MAGIC_PROCESS, send_index);
				SetByte(send_buff, MAGIC_EFFECTING, send_index);
				SetDWORD(send_buff, magicid, send_index);
				SetShort(send_buff, sid, send_index);
				SetShort(send_buff, casted_member[j], send_index);
				SetShort(send_buff, data1, send_index);
				SetShort(send_buff, result, send_index);
				SetShort(send_buff, data3, send_index);
				m_pMain->Send_Region(send_buff, send_index, pTUser->m_pUserData->m_bZone, pTUser->m_RegionX, pTUser->m_RegionZ, nullptr, false);

				send_index = 0;
				memset(send_buff, 0, sizeof(send_buff));

				pTUser->m_bResHpType = USER_STANDING;     // Target status is officially alive now.
				pTUser->HpChange(pTUser->m_iMaxHp);	 // Refill HP.	
				pTUser->ExpChange(pType->ExpRecover / 100);     // Increase target experience.

				SetByte(send_buff, AG_USER_REGENE, send_index);		// Send a packet to AI server.
				SetShort(send_buff, casted_member[j], send_index);
				SetShort(send_buff, pTUser->m_pUserData->m_bZone, send_index);
				m_pMain->Send_AIServer(pTUser->m_pUserData->m_bZone, send_buff, send_index);

				send_index = 0;			// Clear index and buffer!
				memset(send_buff, 0, sizeof(send_buff));
				break;

			// Summon a target within the zone.
			case 12:
				// Same zone?
				if (m_pSrcUser->m_pUserData->m_bZone != pTUser->m_pUserData->m_bZone)
				{
					result = 0;
					goto packet_send;
				}

				SetByte(send_buff, WIZ_MAGIC_PROCESS, send_index);
				SetByte(send_buff, MAGIC_EFFECTING, send_index);
				SetDWORD(send_buff, magicid, send_index);
				SetShort(send_buff, sid, send_index);
				SetShort(send_buff, casted_member[j], send_index);
				SetShort(send_buff, data1, send_index);
				SetShort(send_buff, result, send_index);
				SetShort(send_buff, data3, send_index);
				m_pMain->Send_Region(send_buff, send_index, pTUser->m_pUserData->m_bZone, pTUser->m_RegionX, pTUser->m_RegionZ, nullptr, false);
				send_index = 0;
				memset(send_buff, 0, sizeof(send_buff));

				SetShort(send_buff, (WORD) (m_pSrcUser->m_pUserData->m_curx * 10 /* + myrand(1,3) */), send_index);	// Send packet with new positions to the Warp() function.
				SetShort(send_buff, (WORD) (m_pSrcUser->m_pUserData->m_curz * 10 /* + myrand(1,3) */), send_index);
				pTUser->Warp(send_buff);

				send_index = 0;
				memset(send_buff, 0, sizeof(send_buff));
				break;

			// Summon a target outside the zone.
			case 13:
				// Different zone?
				if (m_pSrcUser->m_pUserData->m_bZone == pTUser->m_pUserData->m_bZone)
				{
					result = 0;
					goto packet_send;
				}

				SetByte(send_buff, WIZ_MAGIC_PROCESS, send_index);
				SetByte(send_buff, MAGIC_EFFECTING, send_index);
				SetDWORD(send_buff, magicid, send_index);
				SetShort(send_buff, sid, send_index);
				SetShort(send_buff, casted_member[j], send_index);
				SetShort(send_buff, data1, send_index);
				SetShort(send_buff, result, send_index);
				SetShort(send_buff, data3, send_index);
				m_pMain->Send_Region(send_buff, send_index, pTUser->m_pUserData->m_bZone, pTUser->m_RegionX, pTUser->m_RegionZ, nullptr, false);

				memset(send_buff, 0, sizeof(send_buff));
				send_index = 0;

				pTUser->ZoneChange(m_pSrcUser->m_pUserData->m_bZone, m_pSrcUser->m_pUserData->m_curx, m_pSrcUser->m_pUserData->m_curz);
				//pTUser->UserInOut( USER_IN );
				pTUser->UserInOut(USER_REGENE);
				//TRACE(_T(" Summon ,, name=%hs, x=%.2f, z=%.2f\n"), pTUser->m_pUserData->m_id, pTUser->m_pUserData->m_curx, pTUser->m_pUserData->m_curz);
				break;

			// Randomly teleport the source (within 20 meters)
			case 20:
				SetByte(send_buff, WIZ_MAGIC_PROCESS, send_index);
				SetByte(send_buff, MAGIC_EFFECTING, send_index);
				SetDWORD(send_buff, magicid, send_index);
				SetShort(send_buff, sid, send_index);
				SetShort(send_buff, casted_member[j], send_index);
				SetShort(send_buff, data1, send_index);
				SetShort(send_buff, result, send_index);
				SetShort(send_buff, data3, send_index);
				m_pMain->Send_Region(send_buff, send_index, pTUser->m_pUserData->m_bZone, pTUser->m_RegionX, pTUser->m_RegionZ, nullptr, false);

				send_index = 0;
				memset(send_buff, 0, sizeof(send_buff));

				float warp_x, warp_z;		// Variable Initialization.
				float temp_warp_x, temp_warp_z;

				warp_x = pTUser->m_pUserData->m_curx;	// Get current locations.
				warp_z = pTUser->m_pUserData->m_curz;

				temp_warp_x = myrand(0, 20);	// Get random positions (within 20 meters)
				temp_warp_z = myrand(0, 20);

				if (temp_warp_x > 10)	// Get new x-position.
					warp_x = warp_x + (temp_warp_x - 10);
				else
					warp_x = warp_x - temp_warp_x;

				if (temp_warp_z > 10)	// Get new z-position.
					warp_z = warp_z + (temp_warp_z - 10);
				else
					warp_z = warp_z - temp_warp_z;

				// Make sure all positions are within range.
				// Change it if it isn't!!!
				// (Warp function does not check this!)
				if (warp_x < 0.0f)
					warp_x = 0.0f;

				if (warp_x > 4096)
					warp_x = 4096;

				if (warp_z < 0.0f)
					warp_z = 0.0f;

				if (warp_z > 4096)
					warp_z = 4096;

				SetShort(send_buff, (WORD) warp_x, send_index);	// Send packet with new positions to the Warp() function.
				SetShort(send_buff, (WORD) warp_z, send_index);
				pTUser->Warp(send_buff);

				send_index = 0;			// Clear index and buffer!
				memset(send_buff, 0, sizeof(send_buff));
				break;

			// Summon a monster within a zone.
			case 21:
				// LATER!!! 
				break;

			default:
				result = 0;
				goto packet_send;
		}

	packet_send:
		SetByte(send_buff, WIZ_MAGIC_PROCESS, send_index);
		SetByte(send_buff, MAGIC_EFFECTING, send_index);
		SetDWORD(send_buff, magicid, send_index);
		SetShort(send_buff, sid, send_index);
		SetShort(send_buff, casted_member[j], send_index);
		SetShort(send_buff, data1, send_index);
		SetShort(send_buff, result, send_index);
		SetShort(send_buff, data3, send_index);
		m_pMain->Send_Region(send_buff, send_index, m_pSrcUser->m_pUserData->m_bZone, m_pSrcUser->m_RegionX, m_pSrcUser->m_RegionZ, nullptr, false);

		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;
		result = 1;
	}
}

void CMagicProcess::ExecuteType9(int magicid)
{
}

void CMagicProcess::ExecuteType10(int magicid)
{
}

short CMagicProcess::GetMagicDamage(int sid, int tid, int total_hit, int attribute)
{
	CNpc* pMon = nullptr;

	short damage = 0, temp_hit = 0, righthand_damage = 0, attribute_damage = 0;
	int random = 0, total_r = 0;
	BYTE result;

	// Check if target id is valid.
	if (tid < 0
		|| tid >= MAX_USER)
		return -1;

	CUser* pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[tid];	   // Get target info.
	if (pTUser == nullptr
		|| pTUser->m_bResHpType == USER_DEAD)
		return -1;

	// If the source is a monster.
	if (sid >= NPC_BAND)
	{
		pMon = m_pMain->m_NpcMap.GetData(sid);
		if (pMon == nullptr
			|| pMon->m_NpcState == NPC_DEAD)
			return 0;

		result = m_pSrcUser->GetHitRate(pMon->m_sHitRate / pTUser->m_sTotalEvasionrate);
	}
	// If the source is another player.
	else
	{
		total_hit = total_hit * m_pSrcUser->m_pUserData->m_bCha / 170;
		result = SUCCESS;
	}

	// In case of SUCCESS.... 
	if (result != FAIL)
	{
		switch (attribute)
		{
			case NONE_R:
				total_r = 0;
				break;

			case FIRE_R:
				total_r = pTUser->m_bFireR + pTUser->m_bFireRAmount;
				break;

			case COLD_R:
				total_r = pTUser->m_bColdR + pTUser->m_bColdRAmount;
				break;

			case LIGHTNING_R:
				total_r = pTUser->m_bLightningR + pTUser->m_bLightningRAmount;
				break;

			case MAGIC_R:
				total_r = pTUser->m_bMagicR + pTUser->m_bMagicRAmount;
				break;

			case DISEASE_R:
				total_r = pTUser->m_bDiseaseR + pTUser->m_bDiseaseRAmount;
				break;

			case POISON_R:
				total_r = pTUser->m_bPoisonR + pTUser->m_bPoisonRAmount;
				break;

			case LIGHT_R:
				// LATER !!!
				break;

			case DARKNESS_R:
				// LATER !!!
				break;
		}

		if (sid >= 0
			&& sid < MAX_USER)
		{
			// Does the magic user have a staff?
			if (m_pSrcUser->m_pUserData->m_sItemArray[RIGHTHAND].nNum != 0)
			{
				model::Item* pRightHand
					= m_pMain->m_ItemTableMap.GetData(m_pSrcUser->m_pUserData->m_sItemArray[RIGHTHAND].nNum);

				if (pRightHand != nullptr
					&& m_pSrcUser->m_pUserData->m_sItemArray[LEFTHAND].nNum == 0
					&& pRightHand->Kind / 10 == WEAPON_STAFF)
				{
					righthand_damage = pRightHand->Damage;

					if (m_pSrcUser->m_bMagicTypeRightHand == attribute)
						attribute_damage = pRightHand->Damage;
				}
				else
				{
					righthand_damage = 0;
				}
			}
		}

		damage = (short) (total_hit - ((0.7 * total_hit * total_r) / 200));
		random = myrand(0, damage);
		damage = (short) (0.7 * (total_hit - ((0.9 * total_hit * total_r) / 200))) + 0.2 * random;
//	
		if (sid >= NPC_BAND)
		{
			damage = damage - (3 * righthand_damage) - (3 * attribute_damage);
		}
		else
		{
			// Only if the staff has an attribute.
			if (attribute != 4)
			{
				damage = damage - ((righthand_damage * 0.8f) + (righthand_damage * m_pSrcUser->m_pUserData->m_bLevel) / 60) - ((attribute_damage * 0.8f) + (attribute_damage * m_pSrcUser->m_pUserData->m_bLevel) / 30);
			}
		}
//
	}

	damage = damage / 3;	// 성래씨 요청 

	return damage;
}

bool CMagicProcess::UserRegionCheck(int sid, int tid, int magicid, int radius, short mousex, short mousez) const
{
	CNpc* pMon = nullptr;

	float currenttime = 0.0f;
	bool bFlag = false;

	CUser* pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[tid];     // Get target info.

	// Check if target exists.
	if (pTUser == nullptr)
		return false;

	if (sid >= NPC_BAND)
	{
		pMon = m_pMain->m_NpcMap.GetData(sid);
		if (pMon == nullptr
			|| pMon->m_NpcState == NPC_DEAD)
			return false;

		bFlag = true;
	}

	model::Magic* pMagic = m_pMain->m_MagicTableMap.GetData(magicid);   // Get main magic table.
	if (pMagic == nullptr)
		return false;

	switch (pMagic->Moral)
	{
		// Check that it's your party.
		case MORAL_PARTY_ALL:
			// 비러머글 전쟁존 파티 소환 >.<
			if (pTUser->m_sPartyIndex == -1)
				return (sid == tid);

			if (pTUser->m_sPartyIndex == m_pSrcUser->m_sPartyIndex)
			{
				if (pMagic->Type1 != 8)
					goto final_test;

				currenttime = TimeGet();
				if (pTUser->m_pUserData->m_bZone == ZONE_BATTLE
					&& (currenttime - pTUser->m_fLastRegeneTime < CLAN_SUMMON_TIME))
					return false;

				goto final_test;
			}
			break;

		case MORAL_SELF_AREA:
		case MORAL_AREA_ENEMY:
			if (!bFlag)
			{
				// Check that it's your enemy.
				if (pTUser->m_pUserData->m_bNation != m_pSrcUser->m_pUserData->m_bNation)
					goto final_test;
			}
			else
			{
				if (pTUser->m_pUserData->m_bNation != pMon->m_byGroup)
					goto final_test;
			}
			break;

		case MORAL_AREA_FRIEND:
			// Check that it's your ally.
			if (pTUser->m_pUserData->m_bNation == m_pSrcUser->m_pUserData->m_bNation)
				goto final_test;
			break;

		// 비러머글 클랜 소환!!!
		case MORAL_CLAN_ALL:
			if (pTUser->m_pUserData->m_bKnights == -1)
				return (sid == tid);
/*
			if ( pTUser->m_pUserData->m_bKnights == m_pSrcUser->m_pUserData->m_bKnights)
				goto final_test;
*/
			if (pTUser->m_pUserData->m_bKnights == m_pSrcUser->m_pUserData->m_bKnights)
			{
				if (pMagic->Type1 != 8)
					goto final_test;

				currenttime = TimeGet();
				if (pTUser->m_pUserData->m_bZone == ZONE_BATTLE
					&& (currenttime - pTUser->m_fLastRegeneTime < CLAN_SUMMON_TIME))
					return false;

				goto final_test;
			}
			break;
//
	}

	return false;

final_test:
	// When players attack...
	if (!bFlag)
	{
		// Zone Check!
		if (pTUser->m_pUserData->m_bZone == m_pSrcUser->m_pUserData->m_bZone)
		{
			// Region Check!
			if (pTUser->m_RegionX == m_pSrcUser->m_RegionX
				&& pTUser->m_RegionZ == m_pSrcUser->m_RegionZ)
			{
				// Radius check! ( ...in case there is one :(  )
				if (radius != 0)
				{
					// Y-AXIS DISABLED TEMPORARILY!!!
					int temp_x = pTUser->m_pUserData->m_curx - mousex;
					int temp_z = pTUser->m_pUserData->m_curz - mousez;
					int distance = pow(temp_x, 2) + pow(temp_z, 2);
					if (distance > pow(radius, 2))
						return false;
				}

				// Target is in the area.
				return true;
			}
		}
	}
	// When monsters attack...
	else
	{
		// Zone Check!
		if (pTUser->m_pUserData->m_bZone == pMon->m_sCurZone)
		{
			// Region Check!
			if (pTUser->m_RegionX == pMon->m_sRegion_X
				&& pTUser->m_RegionZ == pMon->m_sRegion_Z)
			{
				// Radius check! ( ...in case there is one :(  )
				if (radius != 0)
				{
					int temp_x = pTUser->m_pUserData->m_curx - pMon->m_fCurX;
					int temp_z = pTUser->m_pUserData->m_curz - pMon->m_fCurZ;
					int distance = pow(temp_x, 2) + pow(temp_z, 2);
					if (distance > pow(radius, 2))
						return false;
				}

				// Target is in the area.
				return true;
			}
		}
	}

	return false;
}

void CMagicProcess::Type4Cancel(int magicid, short tid)
{
	int send_index = 0;
	char send_buff[128] = {};

	CUser* pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[tid];     // Get target info.  

	// Check if target exists.
	if (pTUser == nullptr)
		return;

	model::MagicType4* pType = m_pMain->m_MagicType4TableMap.GetData(magicid);     // Get magic skill table type 4.
	if (pType == nullptr)
		return;

	bool buff = false;
	BYTE buff_type = pType->BuffType;

	switch (buff_type)
	{
		case 1:
			if (pTUser->m_sMaxHPAmount > 0)
			{
				pTUser->m_sDuration1 = 0;
				pTUser->m_fStartTime1 = 0.0f;
				pTUser->m_sMaxHPAmount = 0;
				buff = true;
			}
			break;

		case 2:
			if (pTUser->m_sACAmount > 0)
			{
				pTUser->m_sDuration2 = 0;
				pTUser->m_fStartTime2 = 0.0f;
				pTUser->m_sACAmount = 0;
				buff = true;
			}
			break;
// 
		case 3:
			pTUser->m_sDuration3 = 0;
			pTUser->m_fStartTime3 = 0.0f;

			memset(send_buff, 0, sizeof(send_buff));
			send_index = 0;
			SetByte(send_buff, 3, send_index);
			SetByte(send_buff, ABNORMAL_NORMAL, send_index);
			pTUser->StateChange(send_buff);

			memset(send_buff, 0, sizeof(send_buff));
			send_index = 0;
			buff = true;
			break;
//  
		case 4:
			if (pTUser->m_bAttackAmount > 100)
			{
				pTUser->m_sDuration4 = 0;
				pTUser->m_fStartTime4 = 0.0f;
				pTUser->m_bAttackAmount = 100;
				buff = true;
			}
			break;

		case 5:
			if (pTUser->m_bAttackSpeedAmount > 100)
			{
				pTUser->m_sDuration5 = 0;
				pTUser->m_fStartTime5 = 0.0f;
				pTUser->m_bAttackSpeedAmount = 100;
				buff = true;
			}
			break;

		case 6:
			if (pTUser->m_bSpeedAmount > 100)
			{
				pTUser->m_sDuration6 = 0;
				pTUser->m_fStartTime6 = 0.0f;
				pTUser->m_bSpeedAmount = 100;
				buff = true;
			}
			break;

		case 7:
			if ((pTUser->m_bStrAmount
				+ pTUser->m_bStaAmount
				+ pTUser->m_bDexAmount
				+ pTUser->m_bIntelAmount
				+ pTUser->m_bChaAmount) > 0)
			{
				pTUser->m_sDuration7 = 0;
				pTUser->m_fStartTime7 = 0.0f;
				pTUser->m_bStrAmount = 0;
				pTUser->m_bStaAmount = 0;
				pTUser->m_bDexAmount = 0;
				pTUser->m_bIntelAmount = 0;
				pTUser->m_bChaAmount = 0;
				buff = true;
			}
			break;

		case 8:
			if ((pTUser->m_bFireRAmount
				+ pTUser->m_bColdRAmount
				+ pTUser->m_bLightningRAmount
				+ pTUser->m_bMagicRAmount
				+ pTUser->m_bDiseaseRAmount
				+ pTUser->m_bPoisonRAmount) > 0)
			{
				pTUser->m_sDuration8 = 0;
				pTUser->m_fStartTime8 = 0.0f;
				pTUser->m_bFireRAmount = 0;
				pTUser->m_bColdRAmount = 0;
				pTUser->m_bLightningRAmount = 0;
				pTUser->m_bMagicRAmount = 0;
				pTUser->m_bDiseaseRAmount = 0;
				pTUser->m_bPoisonRAmount = 0;
				buff = true;
			}
			break;

		case 9:
			if ((pTUser->m_bHitRateAmount + pTUser->m_sAvoidRateAmount) > 200)
			{
				pTUser->m_sDuration9 = 0;
				pTUser->m_fStartTime9 = 0.0f;
				pTUser->m_bHitRateAmount = 100;
				pTUser->m_sAvoidRateAmount = 100;
				buff = true;
			}
			break;
	}

	if (buff)
	{
		pTUser->m_bType4Buff[buff_type - 1] = 0;
		pTUser->SetSlotItemValue();
		pTUser->SetUserAbility();
		pTUser->Send2AI_UserUpdateInfo();	// AI Server에 바끤 데이타 전송....

		/*	Send Party Packet.....
		if (m_sPartyIndex != -1) {
			SetByte( send_buff, WIZ_PARTY, send_index );
			SetByte( send_buff, PARTY_STATUSCHANGE, send_index );
			SetShort( send_buff, m_Sid, send_index );
//			if (buff_type != 5 && buff_type != 6) {
//				SetByte( send_buff, 3, send_index );
//			}
//			else {
			SetByte( send_buff, 2, send_index );
//			}
			SetByte( send_buff, 0x00, send_index);
			m_pMain->Send_PartyMember(m_sPartyIndex, send_buff, send_index);
			memset(send_buff, 0, sizeof(send_buff));
			send_index = 0;
		}
		//  end of Send Party Packet.....  */

		SetByte(send_buff, WIZ_MAGIC_PROCESS, send_index);
		SetByte(send_buff, MAGIC_TYPE4_END, send_index);
		SetByte(send_buff, buff_type, send_index);
		pTUser->Send(send_buff, send_index);
	}

	int buff_test = 0;
	for (int i = 0; i < MAX_TYPE4_BUFF; i++)
		buff_test += pTUser->m_bType4Buff[i];

	if (buff_test == 0)
		pTUser->m_bType4Flag = false;
//
	// Send Party Packet.....
	if (pTUser->m_sPartyIndex != -1
		&& !pTUser->m_bType4Flag)
	{
		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;
		SetByte(send_buff, WIZ_PARTY, send_index);
		SetByte(send_buff, PARTY_STATUSCHANGE, send_index);
		SetShort(send_buff, tid, send_index);
		SetByte(send_buff, 2, send_index);
		SetByte(send_buff, 0x00, send_index);
		m_pMain->Send_PartyMember(pTUser->m_sPartyIndex, send_buff, send_index);
	}
	//  end of Send Party Packet.....  //
//
}

void CMagicProcess::Type3Cancel(int magicid, short tid)
{
	int send_index = 0;
	char send_buff[128] = {};

	CUser* pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[tid];     // Get target info.

	// Check if target exists and not already dead.		
	if (pTUser == nullptr)
		return;

	model::MagicType3* pType = m_pMain->m_MagicType3TableMap.GetData(magicid);     // Get magic skill table type 3.
	if (pType == nullptr)
		return;

	for (int i = 0; i < MAX_TYPE3_REPEAT; i++)
	{
		if (pTUser->m_bHPAmount[i] > 0)
		{
			pTUser->m_fHPStartTime[i] = 0.0f;
			pTUser->m_fHPLastTime[i] = 0.0f;
			pTUser->m_bHPAmount[i] = 0;
			pTUser->m_bHPDuration[i] = 0;
			pTUser->m_bHPInterval[i] = 5;
			pTUser->m_sSourceID[i] = -1;
			break;
		}
	}

	SetByte(send_buff, WIZ_MAGIC_PROCESS, send_index);
	SetByte(send_buff, MAGIC_TYPE3_END, send_index);
	SetByte(send_buff, 100, send_index);
	pTUser->Send(send_buff, send_index);

	int buff_test = 0;
	for (int j = 0; j < MAX_TYPE3_REPEAT; j++)
		buff_test += pTUser->m_bHPDuration[j];

	if (buff_test == 0)
		pTUser->m_bType3Flag = false;

	// Send Party Packet....   
	if (pTUser->m_sPartyIndex != -1
		&& !pTUser->m_bType3Flag)
	{
		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;
		SetByte(send_buff, WIZ_PARTY, send_index);
		SetByte(send_buff, PARTY_STATUSCHANGE, send_index);
		SetShort(send_buff, tid, send_index);
		SetByte(send_buff, 1, send_index);
		SetByte(send_buff, 0x00, send_index);
		m_pMain->Send_PartyMember(pTUser->m_sPartyIndex, send_buff, send_index);
	}
	//  end of Send Party Packet.....  //
//
}

void CMagicProcess::SendType4BuffRemove(short tid, BYTE buff)
{
	int send_index = 0;
	char send_buff[128];

	CUser* pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[tid];     // Get target info.

	// Check if target exists.
	if (pTUser == nullptr)
		return;

	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	SetByte(send_buff, WIZ_MAGIC_PROCESS, send_index);
	SetByte(send_buff, MAGIC_TYPE4_END, send_index);
	SetByte(send_buff, buff, send_index);
	pTUser->Send(send_buff, send_index);
}

short CMagicProcess::GetWeatherDamage(short damage, short attribute)
{
	bool weather_buff = false;

	switch (m_pMain->m_nWeather)
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
	}

	if (weather_buff)
		damage = (damage * 110) / 100;

	return damage;
}

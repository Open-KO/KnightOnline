﻿#include "stdafx.h"
#include "Map.h"

void CUser::MoveProcess(Packet & pkt)
{
	ASSERT(GetMap() != nullptr);
	if (m_bWarp || isDead()) 
		return;

	uint16_t will_x, will_z, will_y;
	int16_t speed=0;
	float real_x, real_z, real_y;
	uint8_t echo;

	pkt >> will_x >> will_z >> will_y >> speed >> echo;
	real_x = will_x/10.0f; real_z = will_z/10.0f; real_y = will_y/10.0f;

	m_sSpeed = speed;
	SpeedHackUser();

	if (!GetMap()->IsValidPosition(real_x, real_z, real_y)) 
		return;

	if (m_oldx != GetX()
		|| m_oldz != GetZ())
	{
		m_oldx = GetX();
		m_oldy = GetY();
		m_oldz = GetZ();
	}

	// TODO: Ensure this is checked properly to prevent speedhacking
	SetPosition(real_x, real_y, real_z);

	if (RegisterRegion())
	{
		g_pMain->RegionNpcInfoForMe(this);
		g_pMain->RegionUserInOutForMe(this);
		g_pMain->MerchantUserInOutForMe(this);
	}

	if (m_bInvisibilityType == INVIS_DISPEL_ON_MOVE)
		CMagicProcess::RemoveStealth(this, INVIS_DISPEL_ON_MOVE);

	Packet result(WIZ_MOVE);
	result << GetSocketID() << will_x << will_z << will_y << speed << echo;
	SendToRegion(&result);

	GetMap()->CheckEvent(real_x, real_z, this);

	result.Initialize(AG_USER_MOVE);
	result << GetSocketID() << m_curx << m_curz << m_cury << speed;
	Send_AIServer(&result);
}

void CUser::AddToRegion(int16_t new_region_x, int16_t new_region_z)
{
	GetRegion()->Remove(this);
	SetRegion(new_region_x, new_region_z);
	GetRegion()->Add(this);
}

void CUser::GetInOut(Packet & result, uint8_t bType)
{
	result.Initialize(WIZ_USER_INOUT);
	result << uint8_t(bType) << GetID(); // uint8_t for 1298 not uint16_t
	if (bType != INOUT_OUT)
		GetUserInfo(result);
}

void CUser::UserInOut(uint8_t bType)
{
	if (GetRegion() == nullptr)
		return;

	Packet result;

	if (bType != INOUT_OUT)
		ResetGMVisibility();

	GetInOut(result, bType);

	if (bType == INOUT_OUT)
		GetRegion()->Remove(this);
	else
		GetRegion()->Add(this);

	SendToRegion(&result, this);

	if (bType == INOUT_OUT || !isBlinking())
	{
		result.Initialize(AG_USER_INOUT);
		result.SByte();
		result << bType << GetSocketID() << GetName() << m_curx << m_curz;
		Send_AIServer(&result);
	}
}

void CUser::GetUserInfo(
	Packet& pkt)
{
	pkt.SByte();
	pkt
		<< GetName()
		<< uint8_t(GetNation())
		<< uint16_t(GetClanID())
		<< uint8_t(GetFame());

	CKnights* pKnights = g_pMain->GetClanPtr(GetClanID());
	if (pKnights == nullptr)
	{
		pkt
			<< uint16_t(0)		// Alliance ID
			<< uint16_t(0)		// Name (length)
			<< uint8_t(0)		// Grade
			<< uint8_t(0)		// Ranking
			<< uint16_t(0)		// Mark version
			<< int16_t(-1);		// Cape ID
	}
	else
	{
		pkt
			<< pKnights->GetAllianceID()
			<< pKnights->m_strName
			<< pKnights->m_byGrade
			<< pKnights->m_byRanking
			<< uint16_t(pKnights->m_sMarkVersion) // symbol/mark version
			<< pKnights->GetCapeID(pKnights); // cape ID 
	}

	// There are two event-driven invisibility states; dispel on attack, and dispel on move.
	// These are handled primarily server-side; from memory the client only cares about value 1 (which we class as 'dispel on move').
	// As this is the only place where this flag is actually sent to the client, we'll just convert 'dispel on attack' 
	// back to 'dispel on move' as the client expects.
	uint8_t bInvisibilityType = m_bInvisibilityType;
	if (bInvisibilityType != INVIS_NONE)
		bInvisibilityType = INVIS_DISPEL_ON_MOVE;

	pkt
		<< uint8_t(GetLevel())
		<< uint8_t(m_bRace)
		<< uint16_t(m_sClass)
		<< uint16_t(GetSPosX())
		<< uint16_t(GetSPosZ())
		<< int16_t(GetSPosY())
		<< uint8_t(m_bFace)
		<< uint8_t(m_byHair)
		<< uint8_t(m_bResHpType)
		<< uint32_t(m_bAbnormalType)
		<< uint8_t(m_bNeedParty)
		<< uint8_t(m_bAuthority)
		<< bool(m_bPartyLeader)			// is party leader (bool)
		<< uint8_t(bInvisibilityType)	// visibility state
		<< int16_t(m_sDirection)
		<< bool(m_bIsChicken)			// chicken/beginner flag
		<< uint8_t(m_bRank)				// king flag
		<< int8_t(m_bKnightsRank)		// NP ranks (total, monthly)
		<< int8_t(m_bPersonalRank);

	uint8_t equippedItems[] =
	{
		BREAST, LEG, HEAD, GLOVE, FOOT, SHOULDER, RIGHTHAND, LEFTHAND
	};

	foreach_array(i, equippedItems)
	{
		_ITEM_DATA* pItem = GetItem(equippedItems[i]);
		if (pItem == nullptr)
			continue;

		pkt
			<< uint32_t(pItem->nNum)
			<< uint16_t(pItem->sDuration)
			<< uint8_t(pItem->bFlag);
	}
}

void CUser::Rotate(Packet & pkt)
{
	if (isDead())
		return;

	Packet result(WIZ_ROTATE);
	pkt >> m_sDirection;
	result << GetSocketID() << m_sDirection;
	SendToRegion(&result, this);
}

bool CUser::CanChangeZone(C3DMap* pTargetMap, e_WarpListError& errorReason)
{
	// While unofficial, game masters should be allowed to teleport anywhere.
	if (isGM())
		return true;

	// Generic error reason; this should only be checked when the method returns false.
	errorReason = WARP_LIST_ERROR_GENERIC;

	if (GetLevel() < pTargetMap->GetMinLevelReq())
	{
		errorReason = WARP_LIST_ERROR_MIN_LEVEL;
		return false;
	}

	if (GetLevel() > pTargetMap->GetMaxLevelReq()
		/*|| !CanLevelQualify(pTargetMap->GetMaxLevelReq())*/)
	{
		errorReason = WARP_LIST_ERROR_DO_NOT_QUALIFY;
		return false;
	}

	switch (pTargetMap->GetID())
	{
	case ZONE_KARUS:
		// Users may enter Luferson (1)/El Morad (2) if they are that nation, 
		if (GetNation() == pTargetMap->GetID()) 
			return true;

		// Users may also enter if there's a war invasion happening in that zone.
		if (GetNation() == ELMORAD)
			return g_pMain->m_byKarusOpenFlag;
		else
			return g_pMain->m_byElmoradOpenFlag;

	case ZONE_ELMORAD:
		// Users may enter Luferson (1)/El Morad (2) if they are that nation, 
		if (GetNation() == pTargetMap->GetID()) 
			return true;

		// Users may also enter if there's a war invasion happening in that zone.
		if (GetNation() == KARUS)
			return g_pMain->m_byElmoradOpenFlag;
		else
			return g_pMain->m_byKarusOpenFlag;

	case ZONE_KARUS_ESLANT:
		return GetNation() == pTargetMap->GetID() - 10;

	case ZONE_ELMORAD_ESLANT:
		return GetNation() == pTargetMap->GetID() - 10;

	case ZONE_DELOS: // TODO: implement CSW logic.
		if (g_pMain->m_byBattleOpen == CLAN_BATTLE
			&& !g_pMain->m_byBattleSiegeWarTeleport)
		{
			errorReason = WARP_LIST_ERROR_NOT_DURING_CSW;
			return false;
		}

		return true;

	case ZONE_BIFROST:
		return true;

	case ZONE_ARDREAM:
		if (g_pMain->isWarOpen())
		{
			errorReason = WARP_LIST_ERROR_NOT_DURING_WAR;
			return false;
		}

		if (GetLoyalty() <= 0)
		{
			errorReason = WARP_LIST_ERROR_NEED_LOYALTY;
			return false;
		}

		return true;

	case ZONE_RONARK_LAND_BASE:
		if (g_pMain->isWarOpen()
			&& g_pMain->m_byBattleZoneType != ZONE_ARDREAM)
		{
			errorReason = WARP_LIST_ERROR_NOT_DURING_WAR;
			return false;
		}

		if (GetLoyalty() <= 0)
		{
			errorReason = WARP_LIST_ERROR_NEED_LOYALTY;
			return false;
		}

		return false;

	case ZONE_RONARK_LAND:
		if (g_pMain->isWarOpen()
			&& g_pMain->m_byBattleZoneType != ZONE_ARDREAM)
		{
			errorReason = WARP_LIST_ERROR_NOT_DURING_WAR;
			return false;
		}

		if (GetLoyalty() <= 0)
		{
			errorReason = WARP_LIST_ERROR_NEED_LOYALTY;
			return false;
		}

		return true;

	default:
		// War zones may only be entered if that war zone is active.
		if (pTargetMap->isWarZone())
		{
			if (pTargetMap->GetID() == ZONE_SNOW_BATTLE)
			{
				if ((pTargetMap->GetID() - ZONE_SNOW_BATTLE) != g_pMain->m_byBattleZone)
					return false;
			}
			else if ((pTargetMap->GetID() - ZONE_BATTLE_BASE) != g_pMain->m_byBattleZone)
			{
				return false;
			}
			else if ((GetNation() == ELMORAD && g_pMain->m_byElmoradOpenFlag)
				|| (GetNation() == KARUS && g_pMain->m_byKarusOpenFlag))
			{
				return false;
			}
		}
	}

	return true;
}

bool CUser::CanLevelQualify(uint8_t sLevel)
{
	int16_t nStatTotal = 300 + (sLevel - 1) * 3;
	uint8_t nSkillTotal = (sLevel - 9) * 2;

	if (sLevel > 60)
		nStatTotal += 2 * (sLevel - 60);

	if ((m_sPoints + GetStatTotal()) > nStatTotal || GetTotalSkillPoints() > nSkillTotal)
		return false;

	return true;
}

void CUser::ZoneChange(uint16_t sNewZone, float x, float z)
{
	C3DMap * pMap = g_pMain->GetZoneByID(sNewZone);
	_KNIGHTS_SIEGE_WARFARE *pKnightSiege = g_pMain->GetSiegeMasterKnightsPtr(1);

	CKnights* pKnightsMaster = NULL;
	if(pKnightSiege) pKnightsMaster = g_pMain->GetClanPtr(pKnightSiege->sMasterKnights);

	if (pMap == nullptr) 
		return;

	e_WarpListError errorReason;
	if (!CanChangeZone(pMap, errorReason))
	{
		Packet result(WIZ_WARP_LIST);

		result << uint8_t(2) << uint8_t(errorReason);

		if (errorReason == WARP_LIST_ERROR_MIN_LEVEL)
			result << pMap->GetMinLevelReq();

		Send(&result);
		return;
	}

	if (x == 0.0f && z == 0.0f)
	{
		_START_POSITION * pStartPosition = g_pMain->GetStartPosition(sNewZone);
		if (pStartPosition != nullptr) 
		{
			x = (float)(GetNation() == KARUS ? pStartPosition->sKarusX : pStartPosition->sElmoradX + myrand(0, pStartPosition->bRangeX)); 
			z =	(float)(GetNation() == KARUS ? pStartPosition->sKarusZ : pStartPosition->sElmoradZ + myrand(0, pStartPosition->bRangeZ));
		}
	}

	m_bWarp = true;
	m_bZoneChangeFlag = true;

	if (sNewZone == ZONE_DELOS)
	{
		if (pKnightSiege && (pKnightSiege->sMasterKnights==GetClanID()) && (GetClanID()!=0))
		{
			if (GetNation() == KARUS)
			{
			x = (float)(455 + myrand(0, 5));
			z = (float)(790 + myrand(0, 5));
			}
			else
			{
			x = (float)(555 + myrand(0, 5));
			z = (float)(790 + myrand(0, 5));
			}
		}
	}


	m_LastX = x;
	m_LastZ = z;

	if (GetZoneID() != sNewZone)
	{
		UserInOut(INOUT_OUT);

		SetZoneAbilityChange(sNewZone);

		m_bZoneChangeSameZone = false;

		/* 
		Here we also send a clan packet with subopcode 0x16 (with a byte flag of 2) if war zone/Moradon
		or subopcode 0x17 (with nWarEnemyID) for all else
		*/
#if 0
		if (isInClan())
		{
			CKnights * pKnights = g_pMain->GetClanPtr(GetClanID());
			if (pKnights != nullptr
				&& pKnights->bKnightsWarStarted)
			{
				Packet clanPacket(WIZ_KNIGHTS_PROCESS);
				if (pMap->isWarZone() || sNewZone == ZONE_MORADON)
					clanPacket << uint8_t(0x17) << uint8_t(2);
				else 
					clanPacket << uint16_t(0x16) << uint16_t(0 /*nWarEnemyID*/);

				Send(&clanPacket);
			}
		}
#endif

		if (isInParty() && !m_bZoneChangeSameZone)
			if (isPartyLeader())
			{
			_PARTY_GROUP * pParty = g_pMain->GetPartyPtr(GetPartyID());
			PartyPromote(pParty->uid[1]);
			}
			PartyRemove(GetSocketID());

		ResetWindows();
	}
	else
	{
		m_bWarp = false;
		Warp(uint16_t(x * 10), uint16_t(z * 10));
		return;
	}

	if (sNewZone != ZONE_SNOW_BATTLE && GetZoneID() == ZONE_SNOW_BATTLE) 
		SetMaxHp(1);
	if (sNewZone != ZONE_CHAOS_DUNGEON && GetZoneID() == ZONE_CHAOS_DUNGEON)
	{
		SetMaxHp(1);
		RobChaosSkillItems();
	}
	else if (sNewZone == ZONE_FORGOTTEN_TEMPLE)
		g_pMain->m_nForgettenTempleUsers.push_back(GetSocketID());
	else if (sNewZone != ZONE_FORGOTTEN_TEMPLE && GetZoneID() == ZONE_FORGOTTEN_TEMPLE)
		g_pMain->m_nForgettenTempleUsers.erase(std::remove(g_pMain->m_nForgettenTempleUsers.begin(), g_pMain->m_nForgettenTempleUsers.end(), GetSocketID()), g_pMain->m_nForgettenTempleUsers.end());

	m_bZone = (uint8_t) sNewZone; // this is 2 bytes to support the warp data loaded from SMDs. It should not go above a byte, however.
	SetPosition(x, 0.0f, z);
	m_pMap = pMap;

	if (g_pMain->m_nServerNo != pMap->m_nServerNo)
	{
		_ZONE_SERVERINFO *pInfo = g_pMain->m_ServerArray.GetData(pMap->m_nServerNo);
		if (pInfo == nullptr) 
			return;

		UserDataSaveToAgent();

		m_bLogout = 2;	// server change flag
		SendServerChange(pInfo->strServerIP, 2);
		return;
	}

	SetRegion(GetNewRegionX(), GetNewRegionZ());

	Packet result(WIZ_ZONE_CHANGE);
	result
		<< uint8_t(ZONE_CHANGE_TELEPORT)
		<< uint8_t(GetZoneID())
		<< uint8_t(0) // subzone ID
		<< uint16_t(GetSPosX())
		<< uint16_t(GetSPosZ())
		<< int16_t(GetSPosY())
		<< uint8_t(g_pMain->m_byOldVictory);
	Send(&result);

	if (!m_bZoneChangeSameZone)
	{
		m_sWhoKilledMe = -1;
		m_iLostExp = 0;
		m_bRegeneType = 0;
		m_tLastRegeneTime = 0;
		m_sBind = -1;
		InitType3();
		InitType4();
		CMagicProcess::CheckExpiredType9Skills(this, true);
		SetUserAbility();
	}	

	result.Initialize(AG_ZONE_CHANGE);
	result << GetSocketID() << GetZoneID();
	Send_AIServer(&result);

	m_bZoneChangeFlag = false;

	if (pKnightsMaster != nullptr && GetZoneID() == ZONE_DELOS)
	{
		Packet result(WIZ_SIEGE);
		result << uint8_t(2) << pKnightsMaster->GetID() << pKnightsMaster->m_sMarkVersion;
		Send(&result);	
	}
}

void CUser::PlayerRankingProcess(uint16_t ZoneID, bool RemoveInZone)
{
	if(m_bZoneChangeSameZone)
		return;

	if (ZoneID == ZONE_ARDREAM 
		|| ZoneID == ZONE_RONARK_LAND_BASE 
		|| ZoneID == ZONE_RONARK_LAND 
		|| ZoneID == ZONE_BORDER_DEFENSE_WAR 
		|| ZoneID == ZONE_CHAOS_DUNGEON)
	{
		if (RemoveInZone)
			RemovePlayerRank();
		else
		{
			RemovePlayerRank();
			AddPlayerRank(ZoneID);
		}
	}
	else
		RemovePlayerRank();
}

void CUser::AddPlayerRank(uint16_t ZoneID)
{
	m_iLoyaltyDaily = 0;
	m_iLoyaltyPremiumBonus = 0;
	m_KillCount = 0;
	m_DeathCount = 0;

	_USER_RANKING * pData = new _USER_RANKING;

	pData->m_socketID = GetSocketID();
	pData->m_bZone = ZoneID;
	pData->m_bNation = GetNation();
	pData->m_iLoyaltyDaily = m_iLoyaltyDaily;
	pData->m_iLoyaltyPremiumBonus = m_iLoyaltyPremiumBonus;
	pData->m_KillCount = 0;
	pData->m_DeathCount = 0;

	if (!g_pMain->m_UserRankingArray[GetNation() - 1].PutData(pData->m_socketID, pData))
		delete pData;
}

void CUser::RemovePlayerRank()
{
	g_pMain->m_UserRankingArray[GetNation() - 1].DeleteData(GetSocketID());
}

void CUser::UpdatePlayerRank()
{
	if (isGM())
		return;

	_USER_RANKING * pRankInfo = g_pMain->m_UserRankingArray[GetNation() -1].GetData(GetSocketID());
	if (pRankInfo == nullptr)
		return;

	pRankInfo->m_iLoyaltyDaily = m_iLoyaltyDaily;
	pRankInfo->m_iLoyaltyPremiumBonus = m_iLoyaltyPremiumBonus;
	pRankInfo->m_KillCount = m_KillCount;
	pRankInfo->m_DeathCount = m_DeathCount;
}

/**
* @brief	Changes the zone of all party members within the user's zone.
* 			If the user is not in a party, they should still be teleported.
*
* @param	sNewZone	ID of the new zone.
* @param	x			The x coordinate.
* @param	z			The z coordinate.
*/
void CUser::ZoneChangeParty(uint16_t sNewZone, float x, float z)
{
	_PARTY_GROUP * pParty = g_pMain->GetPartyPtr(GetPartyID());
	if (pParty == nullptr)
		return ZoneChange(sNewZone, x, z);

	int16_t partyUsers[MAX_PARTY_USERS];

	for (int i = 0; i < MAX_PARTY_USERS; i++)
		partyUsers[i] = pParty->uid[i];

	for (int i = 0; i < MAX_PARTY_USERS; i++)
	{
		CUser * pUser = g_pMain->GetUserPtr(partyUsers[i]);
		if (pUser != nullptr)
			pUser->ZoneChange(sNewZone, x, z);
	}
}

/**
* @brief	Changes the zone of all clan members in home/neutral zones (including Eslant).
* 			If the user is not in a clan, they should not be teleported.
*
* @param	sNewZone	ID of the new zone.
* @param	x			The x coordinate.
* @param	z			The z coordinate.
*/
void CUser::ZoneChangeClan(uint16_t sNewZone, float x, float z)
{
	CKnights * pKnights = g_pMain->GetClanPtr(GetClanID());
	if (pKnights == nullptr)
		return;

	for (int i = 0; i < MAX_CLAN_USERS; i++)
	{
		_KNIGHTS_USER * p = &pKnights->m_arKnightsUser[i];
		CUser * pUser = p->pSession;
		if (p->byUsed && pUser != nullptr
			&& pUser->GetZoneID() < ZONE_DELOS)
			pUser->ZoneChange(sNewZone, x, z);
	}
}

void CUser::Warp(uint16_t sPosX, uint16_t sPosZ)
{
	ASSERT(GetMap() != nullptr);
	if (m_bWarp)
		return;

	float real_x = sPosX / 10.0f, real_z = sPosZ / 10.0f;
	if (!GetMap()->IsValidPosition(real_x, real_z, 0.0f)) 
	{
		TRACE("Invalid position %f,%f\n", real_x, real_z);
		return;
	}

	m_LastX = real_x;
	m_LastZ = real_z;

	Packet result(WIZ_WARP);
	result << sPosX << sPosZ;
	Send(&result);

	UserInOut(INOUT_OUT);

	m_curx = real_x;
	m_curz = real_z;

	SetRegion(GetNewRegionX(), GetNewRegionZ());

	UserInOut(INOUT_WARP);
	g_pMain->UserInOutForMe(this);
	g_pMain->NpcInOutForMe(this);
	g_pMain->MerchantUserInOutForMe(this);

	ResetWindows();
}

void CUser::RecvWarp(Packet & pkt)
{
	uint16_t warp_x, warp_z;
	pkt >> warp_x >> warp_z;
	Warp(warp_x, warp_z);	
}

void CUser::RecvZoneChange(Packet & pkt)
{
	if (isDead()) // we also need to make sure we're actually waiting on this request...
		return;

	uint8_t opcode = pkt.read<uint8_t>();
	if (opcode == ZONE_CHANGE_LOADING)
	{
		g_pMain->RegionUserInOutForMe(this);
		g_pMain->NpcInOutForMe(this);
		g_pMain->MerchantUserInOutForMe(this);

		Packet result(WIZ_ZONE_CHANGE);
		result << uint8_t(ZONE_CHANGE_LOADED); // finalise the zone change
		Send(&result);
	}
	else if (opcode == ZONE_CHANGE_LOADED)
	{
		UserInOut(INOUT_RESPAWN);

		// TODO: Fix all this up (it's too messy/confusing)
		if (!m_bZoneChangeSameZone)
			BlinkStart();

		if (GetZoneID() != ZONE_CHAOS_DUNGEON)
		{
			InitType4();
			RecastSavedMagic();
		}
		m_bZoneChangeFlag = false;
		m_bWarp = false;
	}
}

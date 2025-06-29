#include "stdafx.h"
#include "Map.h"
#include "KnightsManager.h"
#include "KingSystem.h"
#include "MagicInstance.h"
#include "DBAgent.h"
#include <algorithm>
#include "../shared/DateTime.h"
#include <afx.h>

CUser::CUser(uint16_t socketID, SocketMgr *mgr) : KOSocket(socketID, mgr, -1, 16384, 3172), Unit(UnitPlayer)
{
	m_bHasCheckedClientVersion = false;
}

/**
* @brief	Executes the connect action.
*/
void CUser::OnConnect()
{
	KOSocket::OnConnect();
	Initialize();
}

/**
* @brief	Initializes this object.
*/
void CUser::Initialize()
{
	Unit::Initialize();

	m_strUserID.clear();
	m_strAccountID.clear();
	m_bLogout = 0;

	m_bAuthority = 1;
	m_sBind = -1;

	m_state = GAME_STATE_CONNECTED;

	m_bSelectedCharacter = false;
	m_bStoreOpen = false;
	m_bPartyLeader = false;
	m_bIsChicken = false;
	m_bInParty = false;

	m_bMerchantState = MERCHANT_STATE_NONE;
	m_bInvisibilityType = INVIS_NONE;

	m_sDirection = 0;

	m_sItemMaxHp = m_sItemMaxMp = 0;
	m_sItemWeight = 0;
	m_sItemAc = 0;

	m_sExpGainAmount = m_bNPGainAmount = m_bNoahGainAmount = 100;
	m_bItemExpGainAmount = m_bItemNoahGainAmount = 0;
	m_bItemNPBonus = m_bSkillNPBonus = 0;

	m_byAPBonusAmount = 0;
	memset(&m_byAPClassBonusAmount, 0, sizeof(m_byAPClassBonusAmount));
	memset(&m_byAcClassBonusAmount, 0, sizeof(m_byAcClassBonusAmount));

	memset(&m_bStats, 0, sizeof(m_bStats));
	memset(&m_sStatItemBonuses, 0, sizeof(m_sStatItemBonuses));
	memset(&m_bStatBuffs, 0, sizeof(m_bStatBuffs));
	memset(&m_bstrSkill, 0, sizeof(m_bstrSkill));

	m_bPlayerAttackAmount = 100;

	m_bAddWeaponDamage = 0;
	m_bPctArmourAc = 100;
	m_sAddArmourAc = 0;

	m_sItemHitrate = 100;
	m_sItemEvasionrate = 100;

	m_sSpeed = 0;

	m_bAuthority = AUTHORITY_PLAYER;
	m_bLevel = 1;
	m_iExp = 0;
	m_iBank = m_iGold = 0;
	m_iLoyalty = m_iLoyaltyMonthly = 0;
	m_iMannerPoint = 0;
	m_sHp = m_sMp = m_sSp = 0;

	m_iMaxHp = 0;
	m_iMaxMp = 1;
	m_iMaxExp = 0;
	m_sMaxWeight = 0;
	m_sMaxWeightBonus = 0;

	m_bResHpType = USER_STANDING;
	m_bWarp = false;

	m_sMerchantsSocketID = -1;
	m_sChallengeUser = -1;
	m_sPartyIndex = -1;		
	m_sExchangeUser = -1;
	m_bRequestingChallenge = 0;
	m_bChallengeRequested = 0;
	m_bExchangeOK = 0x00;
	m_bBlockPrivateChat = false;
	m_sPrivateChatUser = -1;
	m_bNeedParty = 0x01;

	m_tHPLastTimeNormal = 0;		// For Automatic HP recovery. 
	m_tHPStartTimeNormal = 0;
	m_bHPAmountNormal = 0;
	m_bHPDurationNormal = 0;
	m_bHPIntervalNormal = 5;

	m_tGameStartTimeSavedMagic = 0;

	m_fSpeedHackClientTime = 0;
	m_fSpeedHackServerTime = 0;
	m_bSpeedHackCheck = 0;

	m_tBlinkExpiryTime = 0;

	m_bAbnormalType = ABNORMAL_NORMAL;	// User starts out in normal size.
	m_nOldAbnormalType = m_bAbnormalType;

	m_sWhoKilledMe = -1;
	m_iLostExp = 0;

	m_tLastTrapAreaTime = 0;

	memset(m_iSelMsgEvent, -1,  MAX_MESSAGE_EVENT);

	m_sEventNid = m_sEventSid = -1;
	m_bZoneChangeFlag = false;
	m_bRegeneType = 0;
	m_tLastRegeneTime = 0;
	m_bZoneChangeSameZone = false;

	m_transformationType = TransformationNone;
	m_sTransformID = 0;
	m_tTransformationStartTime = 0;
	m_sTransformationDuration = 0;

	m_sEventDataIndex = 0;

	m_pKnightsUser = nullptr;

	m_bWeaponsDisabled = false;

	m_iLoyaltyDaily = 0;
	m_iLoyaltyPremiumBonus = 0;
	m_KillCount = 0;
	m_DeathCount = 0;

	m_LastX = 0;
	m_LastZ = 0;
}

/**
* @brief	Executes the disconnect action.
*/
void CUser::OnDisconnect()
{
	KOSocket::OnDisconnect();
	
	g_pMain->RemoveSessionNames(this);

	if (isInGame())
	{
		UserInOut(INOUT_OUT);

		_PARTY_GROUP* pParty = g_pMain->GetPartyPtr(GetPartyID());
		if (pParty != nullptr)
		{
			if (isPartyLeader())
				PartyPromote(pParty->uid[1]);

			PartyRemove(GetSocketID());
		}

		CKnights* pKnights = g_pMain->GetClanPtr(GetClanID());
		if (pKnights != nullptr)
		{
			_KNIGHTS_ALLIANCE* pAllianceKnights = g_pMain->GetAlliancePtr(pKnights->GetAllianceID());
			if (pAllianceKnights != nullptr)
				pKnights->OnLogoutAlliance(this);
			else
				pKnights->OnLogout(this);
		}

		ResetWindows();
	}

	LogOut();
}

/**
* @brief	Handles an incoming user packet.
*
* @param	pkt	The packet.
*
* @return	true if it succeeds, false if it fails.
*/
bool CUser::HandlePacket(Packet & pkt)
{
	uint8_t command;
	pkt >> command;

	TRACE("[SID=%d] Packet: %X (len=%d)\n", GetSocketID(), command, pkt.wpos());

	// If crypto's not been enabled yet, force the version packet to be sent.
	//if (!isCryptoEnabled())
	/*
	if(!m_bHasCheckedClientVersion)
	{
		if (command == WIZ_VERSION_CHECK)
			VersionCheck(pkt);

		m_bHasCheckedClientVersion = true;
		return true;
	}*/


	// If we're not authed yet, forced us to before we can do anything else.
	// NOTE: We're checking the account ID store here because it's only set on successful auth,
	// at which time the other account ID will be cleared out (yes, it's messy -- need to clean it up).
	/*else*/ if (m_strAccountID.empty())
	{
		// TEMP
		if (command == WIZ_VERSION_CHECK)
			VersionCheck(pkt);

		if (command == WIZ_LOGIN)
			LoginProcess(pkt);

		return true;
	}
	// If we haven't logged in yet, don't let us hit in-game packets.
	// TODO: Make sure we support all packets in the loading stage (and rewrite this logic considerably better).
	else if (!m_bSelectedCharacter)
	{
		switch (command)
		{
		case WIZ_SEL_NATION:
			SelNationToAgent(pkt);
			break;
		case WIZ_ALLCHAR_INFO_REQ:
			AllCharInfoToAgent();
			break;
		case WIZ_NEW_CHAR:
			NewCharToAgent(pkt);
			break;
		case WIZ_DEL_CHAR:
			DelCharToAgent(pkt);
			break;
		case WIZ_SEL_CHAR:
			SelCharToAgent(pkt);
			break;
		case WIZ_SPEEDHACK_CHECK:
			SpeedHackTime(pkt);
			break;
		default:
			TRACE("[SID=%d] Unhandled packet (%X) prior to selecting character\n", GetSocketID(), command);
			break;
		}
		return true;
	}

	// Otherwise, assume we're authed & in-game.
	switch (command)
	{
	case WIZ_GAMESTART:
		GameStart(pkt);
		break;
	case WIZ_SERVER_INDEX:
		SendServerIndex();
		break;
	case WIZ_RENTAL:
		RentalSystem(pkt);
		break;
	case WIZ_SKILLDATA:
		SkillDataProcess(pkt);
		break;
	case WIZ_MOVE:
		MoveProcess(pkt);
		break;
	case WIZ_ROTATE:
		Rotate(pkt);
		break;
	case WIZ_ATTACK:
		Attack(pkt);
		break;
	case WIZ_CHAT:
		Chat(pkt);
		break;
	case WIZ_CHAT_TARGET:
		ChatTargetSelect(pkt);
		break;
	case WIZ_REGENE:	
		Regene(pkt.read<uint8_t>()); // respawn type
		break;
	case WIZ_REQ_USERIN:
		RequestUserIn(pkt);
		break;
	case WIZ_REQ_NPCIN:
		RequestNpcIn(pkt);
		break;
	case WIZ_WARP:
		if (isGM())
			RecvWarp(pkt);
		break;
	case WIZ_ITEM_MOVE:
		ItemMove(pkt);
		break;
	case WIZ_NPC_EVENT:
		NpcEvent(pkt);
		break;
	case WIZ_ITEM_TRADE:
		ItemTrade(pkt);
		break;
	case WIZ_TARGET_HP:
		{
			uint16_t uid = pkt.read<uint16_t>();
			uint8_t echo = pkt.read<uint8_t>();
			m_targetID = uid;
			SendTargetHP(echo, uid);
		}
		break;
	case WIZ_BUNDLE_OPEN_REQ:
		BundleOpenReq(pkt);
		break;
	case WIZ_ITEM_GET:
		ItemGet(pkt);
		break;
	case WIZ_ZONE_CHANGE:
		RecvZoneChange(pkt);
		break;
	case WIZ_POINT_CHANGE:
		PointChange(pkt);
		break;
	case WIZ_STATE_CHANGE:
		StateChange(pkt);
		break;
	case WIZ_PARTY:
		PartyProcess(pkt);
		break;
	case WIZ_EXCHANGE:
		ExchangeProcess(pkt);
		break;
	case WIZ_QUEST:
		QuestDataRequest(pkt);
		break;
	case WIZ_MERCHANT:
		MerchantProcess(pkt);
		break;
	case WIZ_MAGIC_PROCESS:
		CMagicProcess::MagicPacket(pkt, this);
		break;
	case WIZ_SKILLPT_CHANGE:
		SkillPointChange(pkt);
		break;
	case WIZ_OBJECT_EVENT:
		ObjectEvent(pkt);
		break;
	case WIZ_TIME:
	case WIZ_WEATHER:
		UpdateGameWeather(pkt);
		break;
	case WIZ_CLASS_CHANGE:
		ClassChange(pkt);
		break;
	case WIZ_CONCURRENTUSER:
		CountConcurrentUser();
		break;
	case WIZ_DATASAVE:
		UserDataSaveToAgent();
		break;
	case WIZ_ITEM_REPAIR:
		ItemRepair(pkt);
		break;
	case WIZ_KNIGHTS_PROCESS:
		CKnightsManager::PacketProcess(this, pkt);
		break;
	case WIZ_ITEM_REMOVE:
		ItemRemove(pkt);
		break;
	case WIZ_SPEEDHACK_CHECK:
		SpeedHackTime(pkt);
		break;
	case WIZ_WAREHOUSE:
		WarehouseProcess(pkt);
		break;
	case WIZ_HOME:
		Home();
		break; 
	case WIZ_FRIEND_PROCESS:
		FriendProcess(pkt);
		break;
	case WIZ_WARP_LIST:
		SelectWarpList(pkt);
		break;
	case WIZ_VIRTUAL_SERVER:
		ServerChangeOk(pkt);
		break;
	case WIZ_PARTY_BBS:
		PartyBBS(pkt);
		break;
	case WIZ_MAP_EVENT:
		break;
	case WIZ_CLIENT_EVENT:
		ClientEvent(pkt.read<uint16_t>());
		break;
	case WIZ_SELECT_MSG:
		RecvSelectMsg(pkt);
		break;
	case WIZ_ITEM_UPGRADE:
		ItemUpgradeProcess(pkt);
		break;
	case WIZ_SHOPPING_MALL: // letter system's used in here too
		ShoppingMall(pkt);
		break;
	case WIZ_NAME_CHANGE:
		HandleNameChange(pkt);
		break;
	case WIZ_KING:
		CKingSystem::PacketProcess(this, pkt);
		break;
	case WIZ_CAPE:
		HandleCapeChange(pkt);
		break;
	case WIZ_CHALLENGE:
		HandleChallenge(pkt);
		break;
	case WIZ_SIEGE:
		SiegeWarFareNpc(pkt);
		break;
	case WIZ_ADMIN_GIVE_ITEM:
		AdminGiveItemHandler(pkt);
		break;
	case WIZ_ADMIN_GIVE_COUNTABLE_ITEM:
		AdminGiveCountableItemHandler(pkt);
		break;
	case WIZ_ADMIN_GIVE_EXP:
		AdminGiveExpHandler(pkt);
		break;
	case WIZ_ADMIN_ARREST_USER:
		AdminArrestUserHandler(pkt);
		break;
	case WIZ_ADMIN_SUMMON_USER:
		AdminSummonUserHandler(pkt);
		break;
	case WIZ_ADMIN_CUTOFF_USER:
		AdminCutoffUserHandler(pkt);
		break;
	case WIZ_ADMIN_BAN_USER:
		AdminBanUserHandler(pkt);
		break;
	case WIZ_ADMIN_UNBAN_USER:
		AdminUnbanUserHandler(pkt);
		break;
	case WIZ_ADMIN_MUTE_USER:
		AdminMuteUserHandler(pkt);
		break;
	case WIZ_ADMIN_UNMUTE_USER:
		AdminUnMuteUserHandler(pkt);
		break;
	case WIZ_ADMIN_DISABLE_ATTACK:
		AdminDisableAttackHandler(pkt);
		break;
	case WIZ_ADMIN_ENABLE_ATTACK:
		AdminEnableAttackHandler(pkt);
		break;
	case WIZ_ADMIN_ZONE_CHANGE:
		AdminZoneChangeHandler(pkt);
		break;
	case WIZ_ADMIN_GIVE_COIN:
		AdminGiveCoinHandler(pkt);
		break;
	case WIZ_ADMIN_GIVE_NP:
		AdminGiveNationalPointHandler(pkt);
		break;
	case WIZ_ADMIN_MAKE_VISIBLE:
		AdminMakeVisible(pkt);
		break;
	case WIZ_ADMIN_MAKE_INVISIBLE:
		AdminMakeInvisible(pkt);
		break;
	case WIZ_ADMIN_HELP:
		AdminHelp(pkt);
		break;
	default:
		TRACE("[SID=%d] Unknown packet %X\n", GetSocketID(), command);
		return false;
	}

	Update();
	return true;
}

/**
* @brief	Updates timed player data, e.g. skills & save requests.
*/
void CUser::Update()
{
	if (m_tGameStartTimeSavedMagic != 0 && (UNIXTIME - m_tGameStartTimeSavedMagic) >= 2)
	{
		m_tGameStartTimeSavedMagic = 0;
		// Restore scrolls...
		InitType4();
		RecastSavedMagic();
	}

	if (!isBlinking() && m_tHPLastTimeNormal != 0 && (UNIXTIME - m_tHPLastTimeNormal) > m_bHPIntervalNormal)
		HPTimeChange();	// For Sitdown/Standup HP restoration.

	// Handles DOT/HOT skills (not COLD skills though.)
	if (m_bType3Flag) 
		HPTimeChangeType3();	

	// Check for expired type 4 buffs
	Type4Duration();

	// Expire any timed out saved skills.
	CheckSavedMagic();

	if (isTransformed())
		CMagicProcess::CheckExpiredType6Skills(this);

	// Check for expired type 9/visibility skills
	CMagicProcess::CheckExpiredType9Skills(this);

	if (isBlinking())		// Should you stop blinking?
		BlinkTimeCheck();

	if ((UNIXTIME - m_lastSaveTime) >= PLAYER_SAVE_INTERVAL)
	{
		m_lastSaveTime = UNIXTIME; // this is set by UpdateUser(), however it may result in multiple requests unless it's set first.
		UserDataSaveToAgent();
	}
		for (int i = 0; i < WAREHOUSE_MAX; i++)
		{
			_ITEM_DATA *pItem = &m_sWarehouseArray[i];

			if(pItem == nullptr)
			continue; 

			if (pItem->nExpirationTime < (uint32_t)UNIXTIME && pItem->nExpirationTime != 0)
				memset(pItem, 0, sizeof(_ITEM_DATA));
		}

		for (int i = 0; i < INVENTORY_TOTAL; i++)
		{
			_ITEM_DATA *pItem = &m_sItemArray[i];

			if(pItem == nullptr)
				continue; 

			if (pItem->nExpirationTime < (uint32_t)UNIXTIME && pItem->nExpirationTime != 0)
				memset(pItem, 0, sizeof(_ITEM_DATA));
		}
}

/**
* @brief	Adjusts a player's loyalty (NP) and sends the loyalty 
* 			change packet.
*
* @param	nChangeAmount	The amount to adjust the loyalty points by.
* @param	bIsKillReward	When set to true, enables the use of NP-modifying buffs
*							and includes monthly NP gains.
*/
void CUser::SendLoyaltyChange(int32_t nChangeAmount /*= 0*/, bool bIsKillReward /* false */, bool bIsBonusReward /* false */, bool bIsAddLoyaltyMonthly /* true */)
{
	Packet result(WIZ_LOYALTY_CHANGE);
	int32_t nChangeAmountLoyaltyMonthly = nChangeAmount;

	// If we're taking NP, we need to prevent us from hitting values below 0.
	if (nChangeAmount < 0)
	{
		// Negate the value so it becomes positive (i.e. -50 -> 50)
		// so we can determine if we're trying to take more NP than we have.
		uint32_t amt = -nChangeAmount; /* avoids unsigned/signed comparison warning */

		if (amt > m_iLoyalty)
			m_iLoyalty = 0;
		else
			m_iLoyalty += nChangeAmount;

		// We should only adjust monthly NP when NP was lost when killing a player.
		if (bIsKillReward)
		{
			if (GetZoneID() == ZONE_ARDREAM || GetZoneID() == ZONE_RONARK_LAND_BASE)
				bIsAddLoyaltyMonthly = false;

			if (bIsAddLoyaltyMonthly)
			{
				if (nChangeAmountLoyaltyMonthly > 40)
					nChangeAmountLoyaltyMonthly += 20;
				else if (nChangeAmountLoyaltyMonthly >= 20 && nChangeAmountLoyaltyMonthly < 40)
					nChangeAmountLoyaltyMonthly += 10;

				if (amt > m_iLoyaltyMonthly)
					m_iLoyaltyMonthly = 0;
				else 
					m_iLoyaltyMonthly += nChangeAmountLoyaltyMonthly;
			}
		}
	}
	// We're simply adding NP here.
	else
	{
		// If you're using an NP modifying buff then add the bonus
		nChangeAmount = m_bNPGainAmount * nChangeAmount / 100;

		// Add on any additional NP earned because of a global NP event.
		// NOTE: They officially check to see if the NP is <= 100,000.
		nChangeAmount = nChangeAmount * (100 + g_pMain->m_byNPEventAmount) / 100;

		// We should only apply NP bonuses when NP was gained as a reward for killing a player.
		if (bIsKillReward)
		{
			// Add on any additional NP gained from items/skills.
			nChangeAmount += m_bItemNPBonus + m_bSkillNPBonus;

			// Add monument bonus.
			if (isInPKZone() && GetPVPMonumentNation() == GetNation())
				nChangeAmount += PVP_MONUMENT_NP_BONUS;
		}

		if (m_iLoyalty + nChangeAmount > LOYALTY_MAX)
			m_iLoyalty = LOYALTY_MAX;
		else
			m_iLoyalty += nChangeAmount;

		if (isInPKZone() && !bIsBonusReward)
		{
			if (GetZoneID() == ZONE_ARDREAM || GetZoneID() == ZONE_RONARK_LAND_BASE)
				bIsAddLoyaltyMonthly = false;

			m_iLoyaltyDaily += nChangeAmount;
			UpdatePlayerRank();
		}

		//// We should only apply additional monthly NP when NP was gained as a reward for killing a player.
		if (!bIsBonusReward)
		{
			if (bIsAddLoyaltyMonthly)
			{
				if (nChangeAmountLoyaltyMonthly > 40)
					nChangeAmountLoyaltyMonthly -= 20;
				else if (nChangeAmountLoyaltyMonthly >= 20 && nChangeAmountLoyaltyMonthly < 40)
					nChangeAmountLoyaltyMonthly -= 10;

				if (m_iLoyaltyMonthly + nChangeAmountLoyaltyMonthly > LOYALTY_MAX)
					m_iLoyaltyMonthly = LOYALTY_MAX;
				else
					m_iLoyaltyMonthly += nChangeAmountLoyaltyMonthly;
			}
		}

		if (bIsKillReward)
		{
			if (GetPremiumProperty(PremiumBonusLoyalty) > 0)
			{
				m_iLoyalty += GetPremiumProperty(PremiumBonusLoyalty);

				if (bIsAddLoyaltyMonthly)
					m_iLoyaltyMonthly += GetPremiumProperty(PremiumBonusLoyalty);

				m_iLoyaltyPremiumBonus += GetPremiumProperty(PremiumBonusLoyalty);
			}
		}
	}

	result
		<< uint8_t(LOYALTY_NATIONAL_POINTS)
		<< uint32_t(m_iLoyalty)
		<< uint32_t(m_iLoyaltyMonthly);

	Send(&result);

	// Player is give first np, second exp and third meat dumpling etc.
	if (bIsKillReward
		&& nChangeAmount > 0)
	{
		if (isInPKZone() || GetMap()->isWarZone())
			ExpChange(PVP_BONUS_EXP, true);

		// Additionally, we should receive a "Meat dumpling"
		if (isInPKZone())
			GiveItem(ITEM_MEAT_DUMPLING);
	}
}

/**
* @brief	Get a player loyalty reward.
*
* @param	isMonthly	Monthly reward.
*/
uint8_t CUser::GetRankReward(bool isMonthly)
{
	enum RankErrorCodes
	{
		NoRank				= 0,
		RewardSuccessfull	= 1,
		RewardAlreadyTaken	= 2
	};

	int8_t nRank = -1;
	int32_t nGoldAmount = 0;

	Guard lock(g_pMain->m_userRankingsLock);

	std::string strUserID = GetName();
	STRTOUPPER(strUserID);

	UserNameRankMap::iterator itr;

	if (isMonthly)
	{
		itr = g_pMain->m_UserPersonalRankMap.find(strUserID);
		nRank = itr != g_pMain->m_UserPersonalRankMap.end() ? int8_t(itr->second->nRank) : -1;
	} else {
		itr = g_pMain->m_UserKnightsRankMap.find(strUserID);
		nRank = itr != g_pMain->m_UserKnightsRankMap.end() ? int8_t(itr->second->nRank) : -1;
	}

	nRank = 1;

	if (nRank > 0 && nRank <= 100)
	{
		if (nRank == 1)
			nGoldAmount = 1000000;
		else if (nRank >= 2 && nRank <= 4)
			nGoldAmount = 700000;
		else if (nRank >= 5 && nRank <= 10)
			nGoldAmount = 500000;
		else if (nRank >= 11 && nRank <= 40)
			nGoldAmount = 300000;
		else if (nRank >= 41 && nRank <= 100)
			nGoldAmount = 200000;
		else
			nGoldAmount = 0;

		if (nGoldAmount > 0) 
		{
			// TODO: Restore official behaviour here.
			// This is intended to use the table.
#if 0
			if (GetUserDailyOp(isMonthly ? DAILY_USER_PERSONAL_RANK_REWARD : DAILY_USER_RANK_REWARD) == 0)
				return RewardAlreadyTaken;
#endif

			GoldGain(nGoldAmount);
			return RewardSuccessfull;

		}
	}

	return NoRank;
}

/**
* @brief	Changes a player's fame.
*
* @param	bFame	The fame.
*/
void CUser::ChangeFame(uint8_t bFame)
{
	Packet result(WIZ_AUTHORITY_CHANGE);

	m_bFame = bFame;
	result << uint8_t(COMMAND_AUTHORITY) << GetSocketID() << GetFame();
	SendToRegion(&result);
}

/**
* @brief	Sends the server index.
*/
void CUser::SendServerIndex()
{
	Packet result(WIZ_SERVER_INDEX);
	result << uint16_t(1) << uint16_t(g_pMain->m_nServerNo);
	Send(&result);
}

/**
* @brief	Packet handler for skillbar requests.
*
* @param	pkt	The packet.
*/
void CUser::SkillDataProcess(Packet & pkt)
{
	uint8_t opcode = pkt.read<uint8_t>();
	switch (opcode)
	{
	case SKILL_DATA_SAVE:
		SkillDataSave(pkt);
		break;

	case SKILL_DATA_LOAD:
		SkillDataLoad();
		break;
	}
}

/**
* @brief	Packet handler for saving a skillbar.
*
* @param	pkt	The packet.
*/
void CUser::SkillDataSave(Packet & pkt)
{
	uint16_t sCount = pkt.read<uint16_t>();
	if (sCount == 0 || sCount > 64)
		return;

	Packet result(WIZ_SKILLDATA);
	result	<< uint8_t(SKILL_DATA_SAVE) << sCount;
	for (int i = 0; i < sCount; i++)
		result << pkt.read<uint32_t>();

	g_pMain->AddDatabaseRequest(result, this);
}

/**
* @brief	Packet handler for loading a skillbar.
*/
void CUser::SkillDataLoad()
{
	Packet result(WIZ_SKILLDATA);
	result << uint8_t(SKILL_DATA_LOAD);
	g_pMain->AddDatabaseRequest(result, this);
}

/**
* @brief	Initiates a database request to save the character's information.
*/
void CUser::UserDataSaveToAgent()
{
	if (!isInGame())
		return;

	Packet result(WIZ_DATASAVE);
	result << GetAccountName() << GetName();
	g_pMain->AddDatabaseRequest(result, this);
}

/**
* @brief	Logs a player out.
*/
void CUser::LogOut()
{
	if (m_strUserID.empty()) 
		return; 

	Packet result(AG_USER_LOG_OUT);
	result << GetID() << GetName();
	Send_AIServer(&result);

	result.Initialize(WIZ_LOGOUT);
	m_deleted = true; // make this session unusable until the logout is complete
	g_pMain->AddDatabaseRequest(result, this);
}

/**
* @brief	Sends the player's information on initial login.
*/
void CUser::SendMyInfo()
{
	C3DMap* pMap = GetMap();
	CKnights* pKnights = nullptr;

	if (!pMap->IsValidPosition(GetX(), GetZ(), 0.0f))
	{
		int16_t x = 0, z = 0;
		GetStartPosition(x, z); 

		m_curx = (float)x;
		m_curz = (float)z;
	}

	if (JobGroupCheck(ClassWarrior)
		&& CheckExistEvent(QUEST_ID_MASTERY_WARRIOR, QUEST_STATE_COMPLETED))
		SendQuestStateUpdate(QUEST_ID_MASTERY_WARRIOR, QUEST_STATE_COMPLETED, false);
	else if (JobGroupCheck(ClassRogue)
		&& CheckExistEvent(QUEST_ID_MASTERY_ROGUE, QUEST_STATE_COMPLETED))
		SendQuestStateUpdate(QUEST_ID_MASTERY_ROGUE, QUEST_STATE_COMPLETED, false);
	else if (JobGroupCheck(ClassMage)
		&& CheckExistEvent(QUEST_ID_MASTERY_MAGE, QUEST_STATE_COMPLETED))
		SendQuestStateUpdate(QUEST_ID_MASTERY_MAGE, QUEST_STATE_COMPLETED, false);
	else if (JobGroupCheck(ClassPriest)
		&& CheckExistEvent(QUEST_ID_MASTERY_PRIEST, QUEST_STATE_COMPLETED))
		SendQuestStateUpdate(QUEST_ID_MASTERY_PRIEST, QUEST_STATE_COMPLETED, false);

	Packet result(WIZ_MYINFO);

	// Load up our user rankings (for our NP symbols).
	g_pMain->GetUserRank(this);

	// Are we the King? Let's see, shall we?
	CKingSystem * pData = g_pMain->m_KingSystemArray.GetData(GetNation());
	if (pData != nullptr
		&& STRCASECMP(pData->m_strKingName.c_str(), m_strUserID.c_str()) == 0)
		m_bRank = 1; // We're da King, man.
	else
		m_bRank = 0; // totally not da King.

	result.SByte();
	result
		<< uint16_t(GetSocketID())
		<< GetName()
		<< uint16_t(GetSPosX())
		<< uint16_t(GetSPosZ())
		<< int16_t(GetSPosY())
		<< uint8_t(GetNation())
		<< uint8_t(m_bRace)
		<< uint16_t(m_sClass)
		<< uint8_t(m_bFace)
		<< uint8_t(m_byHair)
		<< uint8_t(m_bRank)
		<< uint8_t(m_bTitle)
		<< uint8_t(GetLevel())
		<< uint8_t(m_sPoints) // NOTE: int16_t to uint8_t
		<< uint32_t(m_iMaxExp)
		<< uint32_t(m_iExp)
		<< uint32_t(GetLoyalty())
		<< uint32_t(GetMonthlyLoyalty())
		<< uint8_t(m_bCity)
		<< uint16_t(GetClanID());

	if (isInClan())
		pKnights = g_pMain->GetClanPtr(GetClanID());

	if (pKnights == nullptr)
	{
		result
			<< uint8_t(0)	// Fame
			<< uint16_t(0)	// Alliance ID
			<< uint8_t(0)	// Clan type ('Flag')
			<< uint8_t(0)	// Name length
			<< uint8_t(0)	// Grade
			<< uint8_t(0)	// Rank
			<< uint16_t(0)	// Mark version
			<< int16_t(-1);	// Cape ID
	}
	else 
	{
		if (pKnights->m_sAlliance != 0)
			pKnights->OnLoginAlliance(this);
		else
			pKnights->OnLogin(this);

		CKnights* aKnights = g_pMain->GetClanPtr(pKnights->GetAllianceID());

		result
			<< uint8_t(GetFame())
			<< uint16_t(pKnights->GetAllianceID())
			<< uint8_t(pKnights->m_byFlag)
			<< pKnights->m_strName
			<< uint8_t(pKnights->m_byGrade)
			<< uint8_t(pKnights->m_byRanking)
			<< uint16_t(pKnights->m_sMarkVersion)
			<< int16_t(pKnights->GetCapeID(aKnights));
	}

	result
		<< uint16_t(m_iMaxHp)
		<< uint16_t(m_sHp)
		<< uint16_t(m_iMaxMp)
		<< uint16_t(m_sMp)
		<< uint16_t(m_sMaxWeight)
		<< uint16_t(m_sItemWeight)
		<< uint8_t(GetStat(STAT_STR))
		<< uint8_t(GetStatItemBonus(STAT_STR))
		<< uint8_t(GetStat(STAT_STA))
		<< uint8_t(GetStatItemBonus(STAT_STA))
		<< uint8_t(GetStat(STAT_DEX))
		<< uint8_t(GetStatItemBonus(STAT_DEX))
		<< uint8_t(GetStat(STAT_INT))
		<< uint8_t(GetStatItemBonus(STAT_INT))
		<< uint8_t(GetStat(STAT_CHA))
		<< uint8_t(GetStatItemBonus(STAT_CHA))
		<< uint16_t(m_sTotalHit)
		<< uint16_t(m_sTotalAc)
		<< uint8_t(m_sFireR)
		<< uint8_t(m_sColdR)
		<< uint8_t(m_sLightningR)
		<< uint8_t(m_sMagicR)
		<< uint8_t(m_sDiseaseR)
		<< uint8_t(m_sPoisonR)
		<< uint32_t(m_iGold)
		<< uint8_t(m_bAuthority)
#if __VERSION > 1264
		<< int8_t(m_bKnightsRank)	 // national rank, leader rank
		<< int8_t(m_bPersonalRank)
#endif
	;

	result.append(m_bstrSkill, 9);

	for (int i = 0; i < SLOT_MAX; i++)
	{
		_ITEM_DATA* pItem = GetItem(i);
		result
			<< uint32_t(pItem->nNum)
			<< uint16_t(pItem->sDuration)
			<< uint16_t(pItem->sCount)
			<< uint8_t(pItem->bFlag)	// item type flag (e.g. rented)
			<< uint16_t(pItem->sRemainingRentalTime);
	}

	for (int i = 0; i < HAVE_MAX; i++)
	{
		_ITEM_DATA* pItem = GetItem(i + SLOT_MAX);
		result
			<< uint32_t(pItem->nNum)
			<< uint16_t(pItem->sDuration)
			<< uint16_t(pItem->sCount)
			<< uint8_t(pItem->bFlag)	// item type flag (e.g. rented)
			<< uint16_t(pItem->sRemainingRentalTime);
	}

	m_bIsChicken = CheckExistEvent(50, 1);

	result
		<< uint8_t(m_bAccountStatus)	// account status (0 = none, 1 = normal prem with expiry in hours, 2 = pc room)
		<< uint8_t(m_bPremiumType)		// premium type (7 = platinum premium)
		<< uint16_t(m_sPremiumTime)		// premium time
		<< bool(m_bIsChicken)				// chicken/beginner flag
		<< uint32_t(m_iMannerPoint);

	Send(&result);

	g_pMain->AddCharacterName(this);

	SetZoneAbilityChange(GetZoneID());
	SendPremiumInfo();
	Send2AI_UserUpdateInfo(true); 
}

/**
* @brief	Calculates & sets a player's maximum HP.
*
* @param	iFlag	If set to 1, additionally resets the HP to max. 
* 					If set to 2, additionally resets the max HP to 100 (i.e. Snow war). 
*/
void CUser::SetMaxHp(int iFlag)
{
	_CLASS_COEFFICIENT* p_TableCoefficient = nullptr;
	p_TableCoefficient = g_pMain->m_CoefficientArray.GetData( m_sClass );
	if( !p_TableCoefficient ) return;

	int temp_sta = getStatTotal(STAT_STA);

	if (GetZoneID() == ZONE_SNOW_BATTLE && iFlag == 0)
		if(GetFame() == COMMAND_CAPTAIN || isKing())
		m_iMaxHp = 300;
		else
		m_iMaxHp = 100;

	else if (GetZoneID() == ZONE_CHAOS_DUNGEON && iFlag == 0)
		m_iMaxHp = 10000 / 10;
	else	
	{
		m_iMaxHp = (int16_t)(((p_TableCoefficient->HP * GetLevel() * GetLevel() * temp_sta ) 
			+ 0.1 * (GetLevel() * temp_sta) + (temp_sta / 5)) + m_sMaxHPAmount + m_sItemMaxHp + 20);

		// A player's max HP should be capped at (currently) 14,000 HP.
		if (m_iMaxHp > MAX_PLAYER_HP && !isGM()) 
			m_iMaxHp = MAX_PLAYER_HP;

		if (iFlag == 1)
			m_sHp = m_iMaxHp;
		else if (iFlag == 2)
			m_iMaxHp = 100;
	}

	if(m_iMaxHp < m_sHp) {
		m_sHp = m_iMaxHp;
		HpChange( m_sHp );
	}
}

/**
* @brief	Calculates & sets a player's maximum MP.
*/
void CUser::SetMaxMp()
{
	_CLASS_COEFFICIENT* p_TableCoefficient = nullptr;
	p_TableCoefficient = g_pMain->m_CoefficientArray.GetData( m_sClass );
	if( !p_TableCoefficient ) return;

	int temp_intel = 0, temp_sta = 0;
	temp_intel = getStatTotal(STAT_INT) + 30;
	//	if( temp_intel > 255 ) temp_intel = 255;
	temp_sta = getStatTotal(STAT_STA);
	//	if( temp_sta > 255 ) temp_sta = 255;

	if( p_TableCoefficient->MP != 0)
	{
		m_iMaxMp = (int16_t)((p_TableCoefficient->MP * GetLevel() * GetLevel() * temp_intel)
			+ (0.1f * GetLevel() * 2 * temp_intel) + (temp_intel / 5) + m_sMaxMPAmount + m_sItemMaxMp + 20);
	}
	else if( p_TableCoefficient->SP != 0)
	{
		m_iMaxMp = (int16_t)((p_TableCoefficient->SP * GetLevel() * GetLevel() * temp_sta )
			+ (0.1f * GetLevel() * temp_sta) + (temp_sta / 5) + m_sMaxMPAmount + m_sItemMaxMp);
	}

	if(m_iMaxMp < m_sMp) {
		m_sMp = m_iMaxMp;
		MSpChange( m_sMp );
	}
}

/**
* @brief	Sends the server time.
*/
void CUser::SendTime()
{
	Packet result(WIZ_TIME);
	result	<< uint16_t(g_pMain->m_sYear) << uint16_t(g_pMain->m_sMonth) << uint16_t(g_pMain->m_sDate)
		<< uint16_t(g_pMain->m_sHour) << uint16_t(g_pMain->m_sMin);
	Send(&result);
}

/**
* @brief	Sends the weather status.
*/
void CUser::SendWeather()
{
	Packet result(WIZ_WEATHER);
	result << g_pMain->m_byWeather << g_pMain->m_sWeatherAmount;
	Send(&result);
}

/**
* @brief	Sets various zone flags to control how
* 			the client handles other players/NPCs.
* 			Also sends the zone's current tax rate.
*/
void CUser::SetZoneAbilityChange(uint16_t sNewZone)
{
	C3DMap * pMap = g_pMain->GetZoneByID(sNewZone);
	_KNIGHTS_SIEGE_WARFARE *pSiegeWar = g_pMain->GetSiegeMasterKnightsPtr(1);
	CKingSystem *pKingSystem = g_pMain->m_KingSystemArray.GetData(GetNation());
	if (pMap == nullptr)
		return;

	if (pSiegeWar != nullptr && pKingSystem != nullptr)
	{
		switch (sNewZone)
		{
		case ZONE_KARUS:
		pMap->SetTariff(pKingSystem->m_nTerritoryTariff);
			break;
		case ZONE_ELMORAD:
		pMap->SetTariff(pKingSystem->m_nTerritoryTariff);
			break;
		case ZONE_MORADON:
		pMap->SetTariff((uint8_t)pSiegeWar->sMoradonTariff);
			break;
		case ZONE_DELOS:
		pMap->SetTariff((uint8_t)pSiegeWar->sDellosTariff);
			break;

		default:
			break;
		}
	}

	{
		Packet result(WIZ_ZONEABILITY);

		result << uint8_t(1)
			<< pMap->canTradeWithOtherNation()
			<< pMap->GetZoneType()
			<< pMap->canTalkToOtherNation()
			<< uint16_t(pMap->GetTariff());

		Send(&result);
	}

	{
		Packet result(WIZ_ZONEABILITY);

		// NOTE(srmeier): temp custom zoneability packet for source client
		uint16_t zoneFlags = pMap->GetZoneFlags();
		uint8_t zoneType = pMap->GetZoneType();
		uint8_t zoneTariff = pMap->GetTariff();
		uint8_t minLevel = pMap->GetMinLevelReq();
		uint8_t maxLevel = pMap->GetMaxLevelReq();

		result << uint8_t(3) << zoneFlags << zoneType << zoneTariff << minLevel << maxLevel;

		Send(&result);
	}

	if (!isGM())
		PlayerRankingProcess(sNewZone,false);

	g_pMain->KillNpc(GetSocketID());

	if (sNewZone == ZONE_BIFROST || sNewZone == ZONE_BATTLE4  || sNewZone ==  ZONE_RONARK_LAND)
		g_pMain->SendEventRemainingTime(false, this, (uint8_t)sNewZone);

	// Clear skill cooldowns...
	m_RHitRepeatList.clear();
	m_CoolDownList.clear();
	m_MagicTypeCooldownList.clear();
}

/**
* @brief	Sends the user's premium state.
*/
void CUser::SendPremiumInfo()
{
	Packet result(WIZ_PREMIUM);
	result
		<< uint8_t(m_bAccountStatus)
		<< uint8_t(m_bPremiumType)
		<< uint32_t(m_sPremiumTime);
	Send(&result);
}

/**
* @brief	Requests user info for the specified session IDs.
*
* @param	pkt	The packet.
*/
void CUser::RequestUserIn(Packet & pkt)
{
	Packet result(WIZ_REQ_USERIN);
	int16_t user_count = pkt.read<uint16_t>(), online_count = 0;
	if (user_count > 1000)
		user_count = 1000;

	size_t wpos = result.wpos();
	result << uint16_t(0); // placeholder for user count

	for (int i = 0; i < user_count; i++)
	{
		CUser *pUser = g_pMain->GetUserPtr(pkt.read<uint16_t>());
		if (pUser == nullptr || !pUser->isInGame())
			continue;

		result /*<< uint8_t(0)*/ << pUser->GetSocketID();
		pUser->GetUserInfo(result);

		online_count++;
	}

	result.put(wpos, online_count); // substitute count in
	SendCompressed(&result);
}

/**
* @brief	Request NPC info for the specified NPC IDs.
*
* @param	pkt	The packet.
*/
void CUser::RequestNpcIn(Packet & pkt)
{
	if (g_pMain->m_bPointCheckFlag == false)
		return;

	Packet result(WIZ_REQ_NPCIN);
	uint16_t npc_count = pkt.read<uint16_t>();
	if (npc_count > 1000)
		npc_count = 1000;

	size_t wpos = result.wpos();
	result << uint16_t(0); // NPC count placeholder

	CKnights *pKnights = g_pMain->GetClanPtr(m_bKnights);
	_KNIGHTS_SIEGE_WARFARE *pSiegeWars = g_pMain->GetSiegeMasterKnightsPtr(1);

	for (int i = 0; i < npc_count; i++)
	{
		uint16_t nid = pkt.read<uint16_t>();
		if (nid < 0 || nid > NPC_BAND+NPC_BAND)
			continue;

		CNpc *pNpc = g_pMain->GetNpcPtr(nid);
		if (pNpc == nullptr || pNpc->isDead())
			continue;

		if (pNpc->m_sSid == 541 && pNpc->GetType() == NPC_DESTROYED_ARTIFACT && pNpc->m_bZone == ZONE_DELOS)
		if ( pKnights != nullptr && pSiegeWars != nullptr)
		{
			if (pSiegeWars->sMasterKnights == pKnights->m_sAlliance || pSiegeWars->sMasterKnights == pKnights->m_sIndex)
					pNpc->m_bNation = 3;
			else
					pNpc->m_bNation = 0;
		}
		pNpc->GetNpcInfo(result);
	}

	result.put(wpos, npc_count);
	SendCompressed(&result);
}

/**
* @brief	Calculates & resets item stats/bonuses.
*/
void CUser::SetSlotItemValue()
{
	_ITEM_TABLE* pTable = nullptr;
	int item_hit = 0, item_ac = 0;

	m_sItemMaxHp = m_sItemMaxMp = 0;
	m_sItemAc = 0; 
	m_sItemWeight = m_sMaxWeightBonus = 0;	
	m_sItemHitrate = m_sItemEvasionrate = 100; 

	memset(m_sStatItemBonuses, 0, sizeof(uint16_t) * STAT_COUNT);
	m_sFireR = m_sColdR = m_sLightningR = m_sMagicR = m_sDiseaseR = m_sPoisonR = 0;
	m_sDaggerR = m_sSwordR = m_sAxeR = m_sMaceR = m_sSpearR = m_sBowR = 0;

	m_byAPBonusAmount = 0;
	memset(&m_byAPClassBonusAmount, 0, sizeof(m_byAPClassBonusAmount));
	memset(&m_byAcClassBonusAmount, 0, sizeof(m_byAcClassBonusAmount));

	m_bItemExpGainAmount = m_bItemNPBonus = m_bItemNoahGainAmount = 0;

	Guard lock(m_equippedItemBonusLock);
	m_equippedItemBonuses.clear();

	std::map<uint16_t, uint32_t> setItems;

	// Apply stat bonuses from all equipped items.
	// Total up the weight of all items.
	for (int i = 0; i < INVENTORY_TOTAL; i++)
	{
		_ITEM_DATA * pItem = nullptr;
		pTable = GetItemPrototype(i, pItem);
		if (pTable == nullptr)
			continue;

		// Non-stackable items should have a count of 1. If not, something's broken.
		m_sItemWeight += pTable->m_sWeight * pItem->sCount;

		// Do not apply stats to unequipped items
		if ((i >= SLOT_MAX && i < INVENTORY_TOTAL)
			// or disabled weapons.
				|| (isWeaponsDisabled() 
				&& (i == RIGHTHAND || i == LEFTHAND) 
				&& !pTable->isShield())
				|| pItem->isDuplicate())
				continue;

		item_ac = pTable->m_sAc;
		if (pItem->sDuration == 0) 
			item_ac /= 10;

		m_sItemMaxHp += pTable->m_MaxHpB;
		m_sItemMaxMp += pTable->m_MaxMpB;
		m_sItemAc += item_ac;
		m_sStatItemBonuses[STAT_STR] += pTable->m_sStrB;
		m_sStatItemBonuses[STAT_STA] += pTable->m_sStaB;
		m_sStatItemBonuses[STAT_DEX] += pTable->m_sDexB;
		m_sStatItemBonuses[STAT_INT] += pTable->m_sIntelB;
		m_sStatItemBonuses[STAT_CHA] += pTable->m_sChaB;
		m_sItemHitrate += pTable->m_sHitrate;
		m_sItemEvasionrate += pTable->m_sEvarate;

		m_sFireR += pTable->m_bFireR;
		m_sColdR += pTable->m_bColdR;
		m_sLightningR += pTable->m_bLightningR;
		m_sMagicR += pTable->m_bMagicR;
		m_sDiseaseR += pTable->m_bCurseR;
		m_sPoisonR += pTable->m_bPoisonR;

		m_sDaggerR += pTable->m_sDaggerAc;
		m_sSwordR += pTable->m_sSwordAc;
		m_sAxeR += pTable->m_sAxeAc;
		m_sMaceR += pTable->m_sMaceAc;
		m_sSpearR += pTable->m_sSpearAc;
		m_sBowR += pTable->m_sBowAc;

		ItemBonusMap bonusMap;
		if (pTable->m_bFireDamage)
			bonusMap.insert(std::make_pair(ITEM_TYPE_FIRE, pTable->m_bFireDamage));

		if (pTable->m_bIceDamage)
			bonusMap.insert(std::make_pair(ITEM_TYPE_COLD, pTable->m_bIceDamage));

		if (pTable->m_bLightningDamage)
			bonusMap.insert(std::make_pair(ITEM_TYPE_LIGHTNING, pTable->m_bLightningDamage));

		if (pTable->m_bPoisonDamage)
			bonusMap.insert(std::make_pair(ITEM_TYPE_POISON, pTable->m_bPoisonDamage));

		if (pTable->m_bHPDrain)
			bonusMap.insert(std::make_pair(ITEM_TYPE_HP_DRAIN, pTable->m_bHPDrain));

		if (pTable->m_bMPDamage)
			bonusMap.insert(std::make_pair(ITEM_TYPE_MP_DAMAGE, pTable->m_bMPDamage));

		if (pTable->m_bMPDrain)
			bonusMap.insert(std::make_pair(ITEM_TYPE_MP_DRAIN, pTable->m_bMPDrain));

		if (pTable->m_bMirrorDamage)
			bonusMap.insert(std::make_pair(ITEM_TYPE_MIRROR_DAMAGE, pTable->m_bMirrorDamage));

		// If we have bonuses to apply, store them.
		if (!bonusMap.empty())
			m_equippedItemBonuses[i] = bonusMap;
	}

	if (m_sAddArmourAc > 0)
		m_sItemAc += m_sAddArmourAc;
	else
		m_sItemAc = m_sItemAc * m_bPctArmourAc / 100;
}

void CUser::RecvUserExp(Packet & pkt)
{
	CNpc * pNpc;
	_PARTY_GROUP * pParty;
	uint16_t sNpcID;
	int32_t iDamage, iTotalDamage, iNpcExp, iNpcLoyalty;
	uint32_t nFinalExp, nFinalLoyalty;
	double TempValue = 0;

	pkt >> sNpcID >> iDamage >> iTotalDamage >> iNpcExp >> iNpcLoyalty;

	pNpc = g_pMain->GetNpcPtr(sNpcID);
	if (pNpc == nullptr
		|| !isInRangeSlow(pNpc, RANGE_50M)
		|| (iNpcExp <= 0 && iNpcLoyalty <= 0))
		return;

	// Calculate base XP earned for the damage dealt.
	if (iNpcExp > 0)
	{
		TempValue = iNpcExp * ((double)iDamage / (double)iTotalDamage);
		nFinalExp = (int) TempValue;
		if (TempValue > nFinalExp)
			nFinalExp++;
	}

	// Calculate base NP earned for the damage dealt.
	if (iNpcLoyalty > 0)
	{
		TempValue = iNpcLoyalty * ((double)iDamage / (double)iTotalDamage);
		nFinalLoyalty = (int) TempValue;
		if (TempValue > nFinalLoyalty)
			nFinalLoyalty++;
	}

	// Handle solo XP/NP gain
	if (!isInParty()
		|| (pParty = g_pMain->GetPartyPtr(GetPartyID())) == nullptr)
	{
		if (isDead())
			return;

		// Calculate the amount to adjust the XP/NP based on level difference.
		float fModifier = pNpc->GetRewardModifier(GetLevel());

		// Give solo XP
		if (iNpcExp > 0)
		{
			TempValue = nFinalExp * fModifier;
			nFinalExp = (int) TempValue;
			if (TempValue > nFinalExp)
				nFinalExp++;

			ExpChange(nFinalExp);
		}

		// Give solo NP
		if (iNpcLoyalty > 0)
		{
			bool UseModifier = false;

			if (UseModifier)
			{
				TempValue = nFinalLoyalty * fModifier;
				nFinalLoyalty = (int) TempValue;
				if (TempValue > nFinalLoyalty)
					nFinalLoyalty++;
			}

			SendLoyaltyChange(nFinalLoyalty);
		}

		return;
	}

	// Handle party XP/NP gain
	std::vector<CUser *> partyUsers;
	uint32_t nTotalLevel = 0;
	for (int i = 0; i < MAX_PARTY_USERS; i++)
	{
		CUser * pUser = g_pMain->GetUserPtr(pParty->uid[i]);
		if (pUser == nullptr)
			continue;

		partyUsers.push_back(pUser);
		nTotalLevel += pUser->GetLevel();
	}

	const float fPartyModifierXP = 0.3f;
	const float fPartyModifierNP = 0.2f;

	uint32_t nPartyMembers = (uint32_t) partyUsers.size();

	// Calculate the amount to adjust the XP/NP based on level difference.
	float fModifier = pNpc->GetPartyRewardModifier(nTotalLevel, nPartyMembers);

	if (iNpcExp > 0)
	{
		TempValue = nFinalExp * fModifier;
		nFinalExp = (int) TempValue;
		if (TempValue > nFinalExp)
			nFinalExp++;
	}

	if (iNpcLoyalty > 0)
	{
		TempValue = nFinalLoyalty * fModifier;
		nFinalLoyalty = (int) TempValue;
		if (TempValue > nFinalLoyalty)
			nFinalLoyalty++;
	}

	// Hand out kill rewards to all users in the party and still in range.
	foreach (itr, partyUsers)
	{
		CUser * pUser = (*itr);
		if (pUser->isDead()
			|| !pUser->isInRange(pNpc, RANGE_50M))
			continue;

		if (iNpcExp > 0)
		{
			TempValue = (nFinalExp * (1 + fPartyModifierXP * (nPartyMembers - 1))) * (double)pUser->GetLevel() / (double)nTotalLevel;
			int iExp = (int) TempValue;
			if (TempValue > iExp)
				iExp++;

			pUser->ExpChange(iExp);
		}

		if (iNpcLoyalty > 0)
		{
			TempValue = (nFinalLoyalty * (1 + fPartyModifierNP * (nPartyMembers - 1))) * (double)pUser->GetLevel() / (double)nTotalLevel;
			int iLoyalty = (int) TempValue;
			if (TempValue > iLoyalty)
				iLoyalty++;

			pUser->SendLoyaltyChange(iLoyalty);
		}
	}
}

/**
* @brief	Changes the player's experience points by iExp.
*
* @param	iExp	The amount of experience points to adjust by.
*/
void CUser::ExpChange(int64_t iExp, bool bIsBonusReward)
{	
	// Stop players level 5 or under from losing XP on death.
	if ((GetLevel() < 6 && iExp < 0)
		// Stop players in the war zone (TODO: Add other war zones) from losing XP on death.
			|| (GetMap()->isWarZone() && iExp < 0))
			return;

	// Despite being signed, we don't want m_iExp ever going below 0.
	// If this happens, we need to investigate why -- not sweep it under the rug.
	ASSERT(m_iExp >= 0);

	if (iExp > 0)
	{
		if (!bIsBonusReward)
		{
			// Adjust the exp gained based on the percent set by the buff
			iExp = iExp * (m_sExpGainAmount + m_bItemExpGainAmount) / 100;

			// Add on any additional XP earned because of a global XP event.
			// NOTE: They officially check to see if the XP is <= 100,000.
			iExp = iExp * (100 + g_pMain->m_byExpEventAmount) / 100;

			if (GetPremiumProperty(PremiumExpPercent) > 0 && !isDead())
				iExp = iExp * (100 + GetPremiumProperty(PremiumExpPercent)) / 100;
		}
	}

	bool bLevel = true;
	if (iExp < 0 
		&& (m_iExp + iExp) < 0)
		bLevel = false;
	else
		m_iExp += iExp;

	// If we need to delevel...
	if (!bLevel)
	{
		// Drop us back a level.
		m_bLevel--;

		// Get the excess XP (i.e. below 0), so that we can take it off the max XP of the previous level
		// Remember: we're deleveling, not necessarily starting from scratch at the previous level
		int64_t diffXP = m_iExp + iExp;

		// Now reset our XP to max for the former level.
		m_iExp = g_pMain->GetExpByLevel(GetLevel());

		// Get new stats etc.
		LevelChange(GetLevel(), false);

		// Take the remainder of the XP off (and delevel again if necessary).
		ExpChange(diffXP);
		return;
	}
	// If we've exceeded our XP requirement, we've leveled.
	else if (m_iExp >= m_iMaxExp)
	{
		if (GetLevel() < MAX_LEVEL)
		{
			// Reset our XP, level us up.
			m_iExp -= m_iMaxExp;
			LevelChange(++m_bLevel);
			return;
		}

		// Hit the max level? Can't level any further. Cap the XP.
		m_iExp = m_iMaxExp;
	}

	// Tell the client our new XP
	Packet result(WIZ_EXP_CHANGE);
	result /*<< uint8_t(0)*/ << uint32_t(m_iExp); // NOTE: Use proper flag
	Send(&result);

	// If we've lost XP, save it for possible refund later.
	if (iExp < 0)
		m_iLostExp = -iExp;
}

/**
* @brief	Get premium properties.
*/
uint16_t CUser::GetPremiumProperty(e_PremiumPropertyType type)
{
	return 0;
}

/**
* @brief	Handles stat updates after a level change. 
* 			It does not change the level.
*
* @param	level   	The level we've changed to.
* @param	bLevelUp	true to level up, false for deleveling.
*/
void CUser::LevelChange(uint8_t level, bool bLevelUp /*= true*/)
{
	if (level < 1 || level > MAX_LEVEL)
		return;

	if (bLevelUp && level > GetLevel() + 1)
	{
		int16_t nStatTotal = 300 + (level - 1) * 3;
		uint8_t nSkillTotal = (level - 9) * 2;

		if (level > 60)
			nStatTotal += 2 * (level - 60);

		m_sPoints += nStatTotal - GetStatTotal();
		m_bstrSkill[SKILL_POINT_CAT_FREE] += nSkillTotal - GetTotalSkillPoints();
		m_bLevel = level;
	}
	else if (bLevelUp)
	{
		// On each level up, we should give 3 stat points for levels 1-60.
		// For each level above that, we give an additional 2 stat points (so 5 stat points per level).
		int levelsAfter60 = (level > 60 ? level - 60 : 0);
		if ((m_sPoints + GetStatTotal()) < int32_t(297 + (3 * level) + (2 * levelsAfter60)))
			m_sPoints += (levelsAfter60 == 0 ? 3 : 5);

		if (level >= 10 && GetTotalSkillPoints() < 2 * (level - 9))
			m_bstrSkill[SKILL_POINT_CAT_FREE] += 2;
	}

	m_iMaxExp = g_pMain->GetExpByLevel(level);
	SetUserAbility();

	m_sMp = m_iMaxMp;
	HpChange(GetMaxHealth()); 

	Send2AI_UserUpdateInfo();

	Packet result(WIZ_LEVEL_CHANGE);
	result
		<< GetSocketID()
		<< GetLevel()
		<< uint8_t(m_sPoints)
		<< uint8_t(m_bstrSkill[SKILL_POINT_CAT_FREE])
		<< uint32_t(m_iMaxExp)
		<< uint32_t(m_iExp)
		<< uint16_t(m_iMaxHp)
		<< uint16_t(m_sHp)
		<< uint16_t(m_iMaxMp)
		<< uint16_t(m_sMp)
		<< uint16_t(m_sMaxWeight)
		<< uint16_t(m_sItemWeight);

	g_pMain->Send_Region(&result, GetMap(), GetRegionX(), GetRegionZ());
	if (isInParty())
	{
		// TODO: Move this to party specific code
		result.Initialize(WIZ_PARTY);
		result << uint8_t(PARTY_LEVELCHANGE) << GetSocketID() << GetLevel();
		g_pMain->Send_PartyMember(GetPartyID(), &result);

		if (m_bIsChicken)
			GrantChickenManner();
	}

	// We should kick players out of the zone if their level no longer matches the requirements for this zone.
	if (GetLevel() < GetMap()->GetMinLevelReq() 
		|| GetLevel() > GetMap()->GetMaxLevelReq())
		KickOutZoneUser();
}

/**
* @brief	Handles player stat assignment.
*
* @param	pkt	The packet.
*/
void CUser::PointChange(Packet & pkt)
{
	uint8_t type = pkt.read<uint8_t>();
	StatType statType = (StatType)(type - 1);

	if (statType < STAT_STR || statType >= STAT_COUNT 
		|| m_sPoints < 1
		|| GetStat(statType) >= STAT_MAX) 
		return;

	Packet result(WIZ_POINT_CHANGE);

	m_sPoints--; // remove a free point
	result << type << uint16_t(++m_bStats[statType]); // assign the free point to a stat
	SetUserAbility();
	result << m_iMaxHp << m_iMaxMp << m_sTotalHit << uint16_t(m_sMaxWeight);
	Send(&result);
	SendItemMove(1);
}

/**
* @brief	Changes a user's HP.
*
* @param	amount   	The amount to change by.
* @param	pAttacker	The attacker.
* @param	bSendToAI	true to update the AI server.
*/
void CUser::HpChange(int amount, Unit *pAttacker /*= nullptr*/, bool bSendToAI /*= true*/) 
{
	Packet result(WIZ_HP_CHANGE);
	uint16_t tid = (pAttacker != nullptr ? pAttacker->GetID() : -1);
	int16_t oldHP = m_sHp;
	int originalAmount = amount;
	int mirrorDamage = 0;

	// No cheats allowed
	if (pAttacker && pAttacker->GetZoneID() != GetZoneID())
		return; 

	// Implement damage/HP cap.
	if (amount < -MAX_DAMAGE)
		amount = -MAX_DAMAGE;
	else if (amount > MAX_DAMAGE)
		amount = MAX_DAMAGE;

	// If we're taking damage...
	if (amount < 0)
	{
		if (isGM())
			return;

		RemoveStealth();

		// Handle the mirroring of damage.
		if (m_bMirrorDamage && isInParty() && GetZoneID() != ZONE_CHAOS_DUNGEON)
		{
			_PARTY_GROUP *pParty = nullptr;
			CUser *pUser = nullptr;
			mirrorDamage = (m_byMirrorAmount * amount) / 100;
			amount -= mirrorDamage;
			pParty = g_pMain->GetPartyPtr(GetPartyID());
			if (pParty != nullptr)
			{
				mirrorDamage = mirrorDamage / (GetPartyMemberAmount(pParty) - 1);
				for (int i = 0; i < MAX_PARTY_USERS; i++)
				{
					pUser = g_pMain->GetUserPtr(pParty->uid[i]);
					if(pUser == nullptr || pUser == this)
						continue;

					pUser->HpChange(mirrorDamage);
				}
			}
		}

		// Handle mana absorb skills
		if (m_bManaAbsorb > 0 && GetZoneID() != ZONE_CHAOS_DUNGEON)
		{
			int toBeAbsorbed = 0;
			toBeAbsorbed = (originalAmount*m_bManaAbsorb) / 100;
			amount -= toBeAbsorbed;

			if (m_bManaAbsorb == 15)
				toBeAbsorbed *= 4;

			MSpChange(toBeAbsorbed);
		}

		// Handle mastery passives
		if (isMastered() && GetZoneID() != ZONE_CHAOS_DUNGEON)
		{
			// Matchless: [Passive]Decreases all damages received by 15%
			if (CheckSkillPoint(SKILL_POINT_CAT_MASTER, 10, MAX_LEVEL))
				amount = (85 * amount) / 100;
			// Absoluteness: [Passive]Decrease 10 % demage of all attacks
			else if (CheckSkillPoint(SKILL_POINT_CAT_MASTER, 5, 9))
				amount = (90 * amount) / 100;
		}
	}
	// If we're receiving HP and we're undead, all healing must become damage.
	else if (m_bIsUndead)
	{
		amount = -amount;
		originalAmount = amount;
	}

	if (amount < 0 && -amount >= m_sHp)
		m_sHp = 0;
	else if (amount >= 0 && m_sHp + amount > m_iMaxHp)
		m_sHp = m_iMaxHp;
	else
		m_sHp += amount;

	result << m_iMaxHp << m_sHp << tid;

	if (GetHealth() > 0
		&& isMastered()
		&& !isMage() && GetZoneID() != ZONE_CHAOS_DUNGEON)
	{
		const uint16_t hp30Percent = (30 * GetMaxHealth()) / 100;
		if ((oldHP >= hp30Percent && m_sHp < hp30Percent)
			|| (m_sHp > hp30Percent))
		{
			SetUserAbility();

			if (m_sHp < hp30Percent)
				ShowEffect(106800); // skill ID for "Boldness", shown when a player takes damage.
		}
	}

	Send(&result);

	if (bSendToAI)
	{
		result.Initialize(AG_USER_SET_HP);
		result << GetSocketID() << m_sHp << tid;
		Send_AIServer(&result);
	}

	if (isInParty() && GetZoneID() != ZONE_CHAOS_DUNGEON)
		SendPartyHPUpdate();

	// Ensure we send the original damage (prior to passives) amount to the attacker 
	// as it appears to behave that way officially.
	if (pAttacker != nullptr
		&& pAttacker->isPlayer())
		TO_USER(pAttacker)->SendTargetHP(0, GetID(), originalAmount);

	if (m_sHp == 0)
		OnDeath(pAttacker);
}

/**
* @brief	Changes a user's mana points.
*
* @param	amount	The amount to adjust by.
*/
void CUser::MSpChange(int amount)
{
	Packet result(WIZ_MSP_CHANGE);
	int16_t oldMP = m_sMp;

	if (isGM() && amount < 0)
		return;

	// TODO: Make this behave unsigned.
	m_sMp += amount;
	if (m_sMp < 0)
		m_sMp = 0;
	else if (m_sMp > m_iMaxMp)
		m_sMp = m_iMaxMp;

	if (isMasteredMage())
	{
		const uint16_t mp30Percent = (30 * GetMaxMana()) / 100;
		if (oldMP >= mp30Percent
			&& GetMana() < mp30Percent)
			ShowEffect(106800); // skill ID for "Boldness", shown when a player loses mana.
	}

	result << m_iMaxMp << m_sMp;
	Send(&result);

	if (isInParty())
		SendPartyHPUpdate(); // handles MP too
}

/**
* @brief	Sends a HP update to the user's party.
*/
void CUser::SendPartyHPUpdate()
{
	Packet result(WIZ_PARTY);
	result	<< uint8_t(PARTY_HPCHANGE)
		<< GetSocketID()
		<< m_iMaxHp << m_sHp
		<< m_iMaxMp << m_sMp;
	g_pMain->Send_PartyMember(GetPartyID(), &result);
}

/**
* @brief	Shows the specified skill's effect 
* 			to the surrounding regions.
*
* @param	nSkillID	Skill identifier.
*/
void CUser::ShowEffect(uint32_t nSkillID)
{
	Packet result(WIZ_EFFECT);
	result << GetID() << nSkillID;
	SendToRegion(&result);
}

/**
* @brief	Shows an effect on the NPC currently 
* 			being interacted with.
*
* @param	nEffectID	Identifier for the effect.
*/
void CUser::ShowNpcEffect(uint32_t nEffectID, bool bSendToRegion)
{
	Packet result(WIZ_OBJECT_EVENT);
	result << uint8_t(OBJECT_NPC) << uint8_t(3) << m_sEventNid << nEffectID;
	if (bSendToRegion)
		SendToRegion(&result);
	else
		Send(&result);
}

/**
* @brief	Sends a player's base information to the AI server.
*
* @param	initialInfo	true when initially sending a player's information
* 						to the server.
*/
void CUser::Send2AI_UserUpdateInfo(bool initialInfo /*= false*/)
{
	Packet result(initialInfo ? AG_USER_INFO : AG_USER_UPDATE);
	GetUserInfoForAI(result);
	Send_AIServer(&result);
}

/**
* @brief	Calculates and resets the player's stats/resistances.
*
* @param	bSendPacket	true to send a subsequent item movement packet
* 						which is almost always required in addition to
* 						using this method.
*/
void CUser::SetUserAbility(bool bSendPacket /*= true*/)
{
	bool bHaveBow = false;
	_CLASS_COEFFICIENT* p_TableCoefficient = g_pMain->m_CoefficientArray.GetData(GetClass());
	uint16_t sItemDamage = 0;
	if (p_TableCoefficient == nullptr)
		return;

	float hitcoefficient = 0.0f;

	if (!isWeaponsDisabled())
	{
		_ITEM_TABLE * pRightHand = GetItemPrototype(RIGHTHAND);
		_ITEM_DATA * pRightData = GetItem(RIGHTHAND);
		if (pRightHand != nullptr)
		{
			switch (pRightHand->m_bKind/10)
			{
			case WEAPON_DAGGER:
				hitcoefficient = p_TableCoefficient->ShortSword;
				break;
			case WEAPON_SWORD:
				hitcoefficient = p_TableCoefficient->Sword;
				break;
			case WEAPON_AXE:
				hitcoefficient = p_TableCoefficient->Axe;
				break;
			case WEAPON_MACE:
			case WEAPON_MACE2:
				hitcoefficient = p_TableCoefficient->Club;
				break;
			case WEAPON_SPEAR:
				hitcoefficient = p_TableCoefficient->Spear;
				break;
			case WEAPON_BOW:
			case WEAPON_LONGBOW:
			case WEAPON_LAUNCHER:
				hitcoefficient = p_TableCoefficient->Bow;
				bHaveBow = true;
				break;
			case WEAPON_STAFF:
				hitcoefficient = p_TableCoefficient->Staff;
				break;
			}
			if (pRightData->sDuration == 0)
			sItemDamage += (pRightHand->m_sDamage + m_bAddWeaponDamage)/2;
			else
			sItemDamage += pRightHand->m_sDamage + m_bAddWeaponDamage;
		}

		_ITEM_TABLE *pLeftHand = GetItemPrototype(LEFTHAND);
		_ITEM_DATA * pLeftData = GetItem(LEFTHAND);
		if (pLeftHand != nullptr)
		{
			if (pLeftHand->isBow())
			{
				hitcoefficient = p_TableCoefficient->Bow;
				bHaveBow = true;
				if (pLeftData->sDuration == 0)
				sItemDamage = (pLeftHand->m_sDamage + m_bAddWeaponDamage)/2;
				else
				sItemDamage = pLeftHand->m_sDamage + m_bAddWeaponDamage;
			}
			else
			{
				if (pLeftData->sDuration == 0)
				sItemDamage += ((pLeftHand->m_sDamage + m_bAddWeaponDamage) / 2) /2;
				else
				sItemDamage += (pLeftHand->m_sDamage + m_bAddWeaponDamage) / 2;
			}
		}
	}

	if (m_sACAmount < 0)
		m_sACAmount = 0;

	m_sTotalHit = 0;

	if (sItemDamage < 3)
		sItemDamage = 3;

	// Update stats based on item data
	SetSlotItemValue();

	int temp_str = GetStat(STAT_STR), temp_dex = getStatTotal(STAT_DEX);
	//	if( temp_str > 255 ) temp_str = 255;
	//	if( temp_dex > 255 ) temp_dex = 255;

	uint32_t baseAP = 0, ap_stat = 0, additionalAP = 3;
	if (temp_str > 150)
		baseAP = temp_str - 150;

	if (temp_str == 160)
		baseAP--;

	temp_str += GetStatBonusTotal(STAT_STR);

	m_sMaxWeight = (((GetStatWithItemBonus(STAT_STR) + GetLevel()) * 50) + m_sMaxWeightBonus)  * (m_bMaxWeightAmount <= 0 ? 1 :  m_bMaxWeightAmount / 100);

	if (isRogue()) 
	{
		ap_stat = temp_dex;
	}
	else
	{
		ap_stat = temp_str;
		additionalAP += baseAP;
	}	
	
	if (isWarrior() || isPriest())
	{
	m_sTotalHit = (uint16_t)((0.010f * sItemDamage * (ap_stat + 40)) + (hitcoefficient * sItemDamage * GetLevel() * ap_stat));
	m_sTotalHit = (m_sTotalHit + additionalAP) * (100 + m_byAPBonusAmount) / 100;
	}
	if(isRogue())
	{
	m_sTotalHit = (uint16_t)((0.007f * sItemDamage * (ap_stat + 40)) + (hitcoefficient * sItemDamage * GetLevel() * ap_stat));
	m_sTotalHit = (m_sTotalHit + additionalAP) * (100 + m_byAPBonusAmount) / 100;
	}
	else if(isMage())
	{
	m_sTotalHit = (uint16_t)((0.005f * sItemDamage * (ap_stat + 40)) + (hitcoefficient * sItemDamage * GetLevel()));
	m_sTotalHit = (m_sTotalHit + additionalAP) * (100 + m_byAPBonusAmount) / 100;
	}

	m_sTotalAc = (int16_t)(p_TableCoefficient->AC * (GetLevel() + m_sItemAc));
	if (m_sACPercent <= 0)
		m_sACPercent = 100;

		m_sTotalAc = m_sTotalAc * m_sACPercent / 100;

	m_fTotalHitrate = ((1 + p_TableCoefficient->Hitrate * GetLevel() *  temp_dex ) * m_sItemHitrate/100 ) * (m_bHitRateAmount/100);

	m_fTotalEvasionrate = ((1 + p_TableCoefficient->Evasionrate * GetLevel() * temp_dex ) * m_sItemEvasionrate/100) * (m_sAvoidRateAmount/100);

	SetMaxHp();
	SetMaxMp();

	uint8_t bDefenseBonus = 0, bResistanceBonus = 0;

	// Reset resistance bonus
	m_bResistanceBonus = 0;

	// Apply passive skill bonuses
	// NOTE: This is how it's done officially (we should really clean this up)
	// Passive bonuses do NOT stack.
	if (isWarrior())
	{
		// NOTE: These may need updating (they're based on 1.298 stats)
		if (CheckSkillPoint(PRO_SKILL2, 5, 14))
			bDefenseBonus = 20;
		else if (CheckSkillPoint(PRO_SKILL2, 15, 34))
			bDefenseBonus = 30;
		else if (CheckSkillPoint(PRO_SKILL2, 35, 54))
			bDefenseBonus = 40;
		else if (CheckSkillPoint(PRO_SKILL2, 55, 69))
			bDefenseBonus = 50;
		else if (CheckSkillPoint(PRO_SKILL2, 70, MAX_LEVEL))
		{
			// Level 70 skill quest
			if (CheckExistEvent(51, 2))
				bDefenseBonus = 60;
			else
				bDefenseBonus = 50;
		}

		// Resist: [Passive]Increase all resistance by 30. If a shield is not equipped, the effect will decrease by half.
		if (CheckSkillPoint(PRO_SKILL2, 10, 19))
			bResistanceBonus = 30;
		// Endure: [Passive]Increase all resistance by 60. If a shield is not equipped, the effect will decrease by half.
		else if (CheckSkillPoint(PRO_SKILL2, 20, 39))
			bResistanceBonus = 60;
		// Immunity: [Passive]Increase all resistance by 90. If a shield is not equipped, the effect will decrease by half.
		else if (CheckSkillPoint(PRO_SKILL2, 40, MAX_LEVEL))
			bResistanceBonus = 90;

		// If a shield's not equipped, bonuses are decreased by half.
		_ITEM_TABLE *pLeftHand = GetItemPrototype(LEFTHAND);
		if (pLeftHand == nullptr || !pLeftHand->isShield())
		{
			bResistanceBonus /= 2;
			bDefenseBonus /= 2;
		}

		m_bResistanceBonus = bResistanceBonus;
		m_sTotalAc += bDefenseBonus * m_sTotalAc / 100;
		// m_sTotalAcUnk += bDefenseBonus * m_sTotalAcUnk / 100;
	}

	// Mastered warriors / mastered priests
	if (CheckClass(6, 12))
	{
		// Boldness/Daring: [Passive]Increase your defense by 20% when your HP is down to 30% or lower.
		if (m_sHp < 30 * m_iMaxHp / 100)
		{
			m_sTotalAc += 20 * m_sTotalAc / 100;
			// m_sTotalAcUnk += 20 * m_sTotalAcUnk / 100;
		}
	}
	else if (isRogue())
	{
		// Valor: [Passive]Increase your resistance by 50 when your HP is down to 30% or below.
		if (m_sHp < 30 * m_iMaxHp / 100)
			m_bResistanceBonus += 50;
	}

	if (m_bAddWeaponDamage > 0)
		++m_sTotalHit;

	if (m_sAddArmourAc > 0 || m_bPctArmourAc > 100)
		++m_sTotalAc;

	uint8_t bSta = GetStat(STAT_STA);
	if (bSta > 100)
	{
		m_sTotalAc += bSta - 100;
		// m_sTotalAcUnk += (bSta - 100) / 3;
	}

	uint8_t bInt = GetStat(STAT_INT);
	if (bInt > 100)
		m_bResistanceBonus += (bInt - 100) / 2;

	// TODO: Transformation stats need to be applied here

	if (bSendPacket)
		SendItemMove(2);

	// Update the AI server
	Send2AI_UserUpdateInfo();
}

/**
* @brief	Sends the target's HP to the player.
*
* @param	echo  	Client-based flag that we must echo back to the client. 
* 					Set to 0 if not responding to the client.
* @param	tid   	The target's ID.
* @param	damage	The amount of damage taken on this request, 0 if it does not apply.
*/
void CUser::SendTargetHP( uint8_t echo, int tid, int damage )
{
	int hp = 0, maxhp = 0;

	Unit *pTarget = nullptr;

	if (tid >= NPC_BAND)
	{
		if (g_pMain->m_bPointCheckFlag == false) return;
		CNpc *pNpc = g_pMain->GetNpcPtr(tid);
		if (pNpc == nullptr)
			return;

		hp = pNpc->m_iHP;	
		maxhp = pNpc->m_iMaxHP;
		pTarget = pNpc;
	}
	else 
	{
		CUser *pUser = g_pMain->GetUserPtr(tid);
		if (pUser == nullptr || pUser->isDead()) 
			return;

		hp = pUser->m_sHp;	
		maxhp = pUser->m_iMaxHp;
		pTarget = pUser;
	}

	Packet result(WIZ_TARGET_HP);
	result << uint16_t(tid) << echo << maxhp << hp << uint16_t(damage);
	Send(&result);
}

/**
* @brief	Handler for opening a loot box.
*
* @param	pkt	The packet.
*/
void CUser::BundleOpenReq(Packet & pkt)
{
	Packet result(WIZ_BUNDLE_OPEN_REQ);
	uint32_t bundle_index = pkt.read<uint32_t>();
	C3DMap* pMap = GetMap();
	CRegion * pRegion = GetRegion();

	if (pMap == nullptr
		|| bundle_index < 1 
		|| pRegion == nullptr
		|| isDead()) // yeah, we know people abuse this. We do not care!
		return;

	Guard lock(pRegion->m_RegionItemArray.m_lock);
	_LOOT_BUNDLE *pBundle = pRegion->m_RegionItemArray.GetData(bundle_index);
	if (pBundle == nullptr
		|| !isInRange(pBundle->x, pBundle->z, MAX_LOOT_RANGE))
		return;

	// Send all items in the bundle to the player
	foreach(itr, pBundle->Items) {
		result << itr->nItemID << itr->sCount;
	}

	// The client expects all n items, so if there's any excess...
	// send placeholder data for them.
	for (uint32_t i = pBundle->Items.size(); i < LOOT_ITEMS; i++)
		result << uint32_t(0) << uint16_t(0);

	Send(&result);
}

/**
* @brief	Handler for looting an item from a loot box.
*
* @param	pkt	The packet.
*/
void CUser::ItemGet(Packet & pkt)
{
	enum LootErrorCodes
	{
		LootError					= 0,
		LootSolo					= 1,
		LootPartyCoinDistribution	= 2,
		LootPartyNotification		= 3,
		LootPartyItemGivenAway		= 4,
		LootPartyItemGivenToUs		= 5,
		LootNoRoom					= 6
	};

	Packet result(WIZ_ITEM_GET);
	uint32_t nBundleID = pkt.read<uint32_t>(), nItemID = pkt.read<uint32_t>();
	_LOOT_BUNDLE * pBundle = nullptr;
	_LOOT_ITEM * pItem = nullptr;
	CRegion* pRegion = GetRegion();
	CUser * pReceiver = nullptr;

	// Lock the array while we process this request
	// to prevent any race conditions between getting/removing the items...
	Guard lock(pRegion->m_RegionItemArray.m_lock);

	// Are we in any region?
	if (pRegion == nullptr
		|| isTrading()
		|| isDead()
		// Does the bundle exist in this region's list?
		|| (pBundle = pRegion->m_RegionItemArray.GetData(nBundleID)) == nullptr
		// Are we close enough to the bundle?
		|| !isInRange(pBundle->x, pBundle->z, MAX_LOOT_RANGE))
		goto fail_return;

	// Does the specified item we're looting exist in the bundle?
	foreach (itr, pBundle->Items)
	{
		if (itr->nItemID == nItemID)
		{
			pItem = &(*itr);
			break;
		}
	}

	// Attempt to loot the specified item.
	// If looting is possible, we can then give the receiver the item.
	if (pItem == nullptr
		|| pItem->sCount == 0
		|| (pReceiver = GetLootUser(pBundle, pItem)) == nullptr)
		goto fail_return;

	// If we're dealing with coins, either:
	//  - we're not in a party, in which case the coins go to us. 
	//  - we're in a party, in which case we need to distribute the coins (proportionately, by their level). 
	// Error handling should already have occurred in GetLootUser().
	if (nItemID == ITEM_GOLD)
	{
		_PARTY_GROUP * pParty;
		uint32_t pGold = 0;
		// Not in a party, so all the coins go to us.
		if (!isInParty()
			|| (pParty = g_pMain->GetPartyPtr(GetPartyID())) == nullptr)
		{
			// NOTE: Coins have been checked already.
			if (GetPremiumProperty(PremiumNoahPercent) > 0)
				pGold = pItem->sCount * (100 + GetPremiumProperty(PremiumNoahPercent)) / 100;
			else
				pGold = pItem->sCount;

			GoldGain(pGold, false, true);
			result << uint8_t(LootSolo) /*<< nBundleID */<< int8_t(-1) << nItemID << pItem->sCount << GetCoins();
			pReceiver->Send(&result);
		}
		// In a party, so distribute the coins relative to their level.
		else
		{
			uint16_t sumOfLevels = 0;
			std::vector<CUser *> partyUsers;
			for (int i = 0; i < MAX_PARTY_USERS; i++)
			{
				CUser * pUser = g_pMain->GetUserPtr(pParty->uid[i]);
				if (pUser == nullptr)
					continue;

				sumOfLevels += pUser->GetLevel();
				partyUsers.push_back(pUser);
			}

			if (partyUsers.empty())
				goto fail_return;

			foreach (itr, partyUsers)
			{
				if ((*itr)->isDead() || 
					!(*itr)->isInRange(pBundle->x, pBundle->z, MAX_LOOT_RANGE))
					continue;

				// Calculate the number of coins to give the player
				// Give each party member coins relative to their level.
				int coins = (int)(pItem->sCount * (float)((*itr)->GetLevel() / (float)sumOfLevels));

				if ((*itr)->GetPremiumProperty(PremiumNoahPercent) > 0)
					pGold = coins * (100 + (*itr)->GetPremiumProperty(PremiumNoahPercent)) / 100;
				else
					pGold = coins;

				(*itr)->GoldGain(pGold, false, true);

				// Let each player know they received coins.
				result.Initialize(WIZ_ITEM_GET);
				result << uint8_t(LootSolo) /*<< nBundleID */<< int8_t(-1) << nItemID << pItem->sCount << (*itr)->GetCoins();
				//result << uint8_t(LootPartyCoinDistribution) << nBundleID << uint8_t(-1) << nItemID << (*itr)->GetCoins(); old
				(*itr)->Send(&result);
			}
		}
	} // end of coin distribution
	// If the item selected is actually an item...
	else
	{
		if (pReceiver->isDead() || 
			!pReceiver->isInRange(pBundle->x, pBundle->z, MAX_LOOT_RANGE))
			(pReceiver = GetLootUser(pBundle, pItem));

		// Retrieve the position for this item.
		int8_t bDstPos = pReceiver->FindSlotForItem(pItem->nItemID, pItem->sCount);

		// This should NOT happen unless their inventory changed after the check.
		// The item won't be removed until after processing's complete, so it's OK to error out here.
		if (bDstPos < 0)
			goto fail_return;

		// Ensure there's enough room in this user's inventory.
		if (!pReceiver->CheckWeight(pItem->nItemID, pItem->sCount))
		{
			result << uint8_t(LootNoRoom);
			pReceiver->Send(&result);
			return; // don't need to remove the item, so stop here.
		}

		// Add item to receiver's inventory
		_ITEM_TABLE * pTable = g_pMain->GetItemPtr(nItemID); // note: already looked up in GetLootUser() so it definitely exists
		_ITEM_DATA * pDstItem = &pReceiver->m_sItemArray[bDstPos];

		pDstItem->nNum    = pItem->nItemID;
		pDstItem->sCount += pItem->sCount;

		if (pDstItem->sCount == pItem->sCount)
		{
			pDstItem->nSerialNum = g_pMain->GenerateItemSerial();

			// NOTE: Obscure special items that act as if their durations are their stack sizes
			// will be broken here, but usual cases are typically only given in the PUS.
			// Will need to revise this logic (rather, shift it out into its own method).
			pDstItem->sDuration  = pTable->m_sDuration; 
		}

		if (pDstItem->sCount > MAX_ITEM_COUNT)
			pDstItem->sCount = MAX_ITEM_COUNT;
		if(!isInParty())
		{
		result	<< uint8_t(pReceiver == this ? LootSolo : LootPartyItemGivenToUs)
			//<< nBundleID 
			<< uint8_t(bDstPos - SLOT_MAX) 
			<< pDstItem->nNum << pDstItem->sCount
			<< pReceiver->GetCoins();
		
		pReceiver->Send(&result);
		}
		else
		{		if(pReceiver->GetName() == GetName())
				{
				result	<< uint8_t(pReceiver == this ? LootSolo : LootPartyItemGivenToUs)
				/*<< nBundleID */
				<< uint8_t(bDstPos - SLOT_MAX) 
				<< pDstItem->nNum << pDstItem->sCount
				<< pReceiver->GetCoins();
				}
			else{
				result	<< uint8_t(pReceiver == this ? LootSolo : LootPartyItemGivenToUs)
				<< nBundleID 
				<< uint8_t(bDstPos - SLOT_MAX) 
				<< pDstItem->nNum << pDstItem->sCount
				<< pReceiver->GetCoins();
				}

		pReceiver->Send(&result);
		}
		pReceiver->SetUserAbility(false);
		pReceiver->SendItemWeight();

		// Now notify the party that we've looted, if applicable.
		if (isInParty())
		{
			result.Initialize(WIZ_ITEM_GET);
			result << uint8_t(LootPartyNotification) << nBundleID << nItemID << pReceiver->GetName();
			g_pMain->Send_PartyMember(GetPartyID(), &result);

			// If we're not the receiver, i.e. round-robin gave it to someone else
			// we should let us know that this was done (otherwise we'll be like, "GM!!? WHERE'S MY ITEM?!?")
			if (pReceiver != this)
			{
				result.Initialize(WIZ_ITEM_GET);
				result << uint8_t(LootPartyItemGivenAway);
				Send(&result);
			}
		}
	}

	// Everything is OK, we have a target. Now remove the item from the bundle.
	// If there's nothing else in the bundle, remove the bundle from the region.
	GetMap()->RegionItemRemove(pRegion, pBundle, pItem);
	return;

fail_return:
	// Generic error
	result << uint8_t(LootError);
	Send(&result);
}

/**
* @brief	Gets the user to give the loot to.
*
* @param	pBundle	The loot bundle.
* @param	pItem  	The item being looted.
*
* @return	null if it fails, else the loot user.
*/
CUser * CUser::GetLootUser(_LOOT_BUNDLE * pBundle, _LOOT_ITEM * pItem)
{
	CUser * pReceiver = nullptr;

	if (pBundle == nullptr
		|| pItem == nullptr)
		return nullptr;

	// If we're dealing with coins, either:
	//  - we're in a party, in which case we need to distribute the coins (proportionately, by their level). 
	//	  No checks are necessary here (the coins will be miniscule, so if there's no room we can safely ignore them)
	//  - we're not in a party, in which case the coins go to us. 
	//	  In this case, we MUST check to be sure we have room for the coins.
	if (pItem->nItemID == ITEM_GOLD)
	{
		// NOTE: No checks are necessary if we're in a party.
		if (!isInParty())
		{
			// We're not in a party, so we must check to be 
			// sure we have enough room for the coins.
			if ((GetCoins() + pItem->sCount) > COIN_MAX)
				return nullptr;
		}

		// The caller will perform the distribution.
		return this; 
	}

	// If we're dealing with items:
	//	- if we're in a party: 
	//		distribute the item to the next player in the party in round-robin fashion, 
	//		whilst ensuring that user can actually hold the item.
	//  - if not in a party: 
	//		simply ensure that we can hold the item.
	if (isInParty())
	{
		// This ensures the user can hold the item.
		return GetItemRoutingUser(pItem->nItemID, pItem->sCount);
	}
	else
	{
		// NOTE: We check to see if they can hold this item in the caller.
		pReceiver = this;
	}

	return pReceiver;
}

/**
* @brief	Packet handler for various player state changes.
*
* @param	pkt	The packet.
*/
void CUser::StateChange(Packet & pkt)
{
	if (isDead())
		return;

	uint8_t bType = pkt.read<uint8_t>(), buff;
	//buff = pkt.read<uint8_t>();
	uint16_t nBuff = pkt.read<uint16_t>(); //buff;//
	buff = *(uint8_t *)&nBuff; // don't ask

	switch (bType)
	{
	case 1:
		if (buff != USER_STANDING && buff != USER_SITDOWN)
			return;
		break;

	case 3:
		// /unview | /view
		if ((buff == 1 || buff == 5)
			&& !isGM())
			return;
		break;

	case 4: // emotions
		switch (buff)
		{
		case 1: // Greeting 1-3
		case 2:
		case 3:
		case 11: // Provoke 1-3
		case 12:
		case 13:
		case 14: // additional animations randomly used when hitting spacebar
		case 15:
			break; // don't do anything with them (this can be handled neater, but just for testing purposes), just make sure they're allowed

		default:
			TRACE("[SID=%d] StateChange: %s tripped (bType=%d, buff=%d, nBuff=%d) somehow, HOW!?\n", 
				GetSocketID(), GetName().c_str(), bType, buff, nBuff);
			break;
		}
		break;

	case 5:
		if (!isGM())
			return;
		break;

	case 7: // invisibility flag, we don't want users overriding server behaviour.
		return;

	default:
		TRACE("[SID=%d] StateChange: %s tripped (bType=%d, buff=%d, nBuff=%d) somehow, HOW!?\n", 
			GetSocketID(), GetName().c_str(), bType, buff, nBuff);
		return;
	}

	StateChangeServerDirect(bType, nBuff);
}

/**
* @brief	Changes a player's state directly from the server
* 			without any checks.
*
* @param	bType	State type.
* @param	nBuff	The buff/flag (depending on the state type).
*/
void CUser::StateChangeServerDirect(uint8_t bType, uint32_t nBuff)
{
	uint8_t buff = *(uint8_t *)&nBuff; // don't ask //nBuff;//
	switch (bType)
	{
	case 1:
		m_bResHpType = buff;
		break;

	case 2:
		m_bNeedParty = buff;
		break;

	case 3:
		m_nOldAbnormalType = m_bAbnormalType;

		// If we're a GM, we need to show ourselves before transforming.
		// Otherwise the visibility state is completely desynced.
		if (isGM())
			StateChangeServerDirect(5, 1);

		m_bAbnormalType = buff;
		break;

	case 5:
		m_bAbnormalType = buff;
		break;

	case 6:
		buff = m_bPartyLeader; // we don't set this here.
		break;

	case 7:
		UpdateVisibility((InvisibilityType)buff);
		break;

	case 8: // beginner quest
		break;
	}

	Packet result(WIZ_STATE_CHANGE);
	result << GetSocketID() << bType /*<< buff;*/ << nBuff;
	SendToRegion(&result);
}

/**
* @brief	Takes a target's loyalty points (NP)
* 			and rewards some/all to the killer (current user).
*
* @param	tid		The target's ID.
* @param	bonusNP Bonus NP to be awarded to the killer as-is.
*/
void CUser::LoyaltyChange(int16_t tid, uint16_t bonusNP /*= 0*/)
{
	int16_t loyalty_source = 0, loyalty_target = 0;

	// TODO: Rewrite this out, it shouldn't handle all cases so generally like this
	if (!GetMap()->isNationPVPZone() && !g_pMain->m_byBattleSiegeWarOpen
		|| GetZoneID() == ZONE_DESPERATION_ABYSS 
		|| GetZoneID() == ZONE_HELL_ABYSS 
		|| GetZoneID() == ZONE_DRAGON_CAVE 
		|| GetZoneID() == ZONE_CAITHAROS_ARENA)
		return;

	CUser* pTUser = g_pMain->GetUserPtr(tid);  
	if (pTUser == nullptr) 
		return;

	if (pTUser->GetNation() != GetNation() || g_pMain->m_byBattleSiegeWarOpen) 
	{
		if (pTUser->GetLoyalty() == 0) 
		{
			loyalty_source = 0;
			loyalty_target = 0;
		}
		// Ardream
		else if (pTUser->GetZoneID() == ZONE_ARDREAM)
		{
			loyalty_source = g_pMain->m_Loyalty_Ardream_Source; 
			loyalty_target = g_pMain->m_Loyalty_Ardream_Target;
		}
		// Ronark Land Base
		else if (pTUser->GetZoneID() == ZONE_RONARK_LAND_BASE)
		{
			loyalty_source = g_pMain->m_Loyalty_Ronark_Land_Base_Source; 
			loyalty_target = g_pMain->m_Loyalty_Ronark_Land_Base_Target;
		}
		else if (pTUser->GetZoneID() == ZONE_RONARK_LAND) 
		{
			loyalty_source = g_pMain->m_Loyalty_Ronark_Land_Source;
			loyalty_target = g_pMain->m_Loyalty_Ronark_Land_Target;
		}
		// Other zones
		else 
		{
			loyalty_source = g_pMain->m_Loyalty_Other_Zone_Source;
			loyalty_target = g_pMain->m_Loyalty_Other_Zone_Target;
		}
	}

	// Include any bonus NP (e.g. rival NP bonus)
	loyalty_source += bonusNP;
	loyalty_target -= bonusNP;

	SendLoyaltyChange(loyalty_source, true, false, pTUser->GetMonthlyLoyalty() > 0 ? true : false);
	pTUser->SendLoyaltyChange(loyalty_target, true, false, pTUser->GetMonthlyLoyalty() > 0 ? true : false);

	// TODO: Move this to a better place (death handler, preferrably)
	// If a war's running, and we died/killed in a war zone... (this method should NOT be so tied up in specifics( 
	if (g_pMain->m_byBattleOpen && GetMap()->isWarZone()) 
	{
		// Update the casualty count
		if (pTUser->GetNation() == KARUS)
			g_pMain->m_sKarusDead++;
		else 
			g_pMain->m_sElmoradDead++;
	}
}

void CUser::SpeedHackUser()
{
	if (!isInGame() || isGM())
		return;

	int16_t nMaxSpeed = 45;

	if (GetFame() == COMMAND_CAPTAIN || isRogue())
		nMaxSpeed = 90;
	else if (isWarrior() || isMage() || isPriest())
		nMaxSpeed = 67;

	if (m_sSpeed > nMaxSpeed || m_sSpeed < -nMaxSpeed)
	{
		DateTime time;
		Disconnect();
		g_pMain->SendFormattedNotice("%s is currently disconnect for speed hack.",Nation::ALL,GetName().c_str());
		g_pMain->WriteCheatLogFile(string_format("[ SpeedHack - %d:%d:%d ] %s is Disconnected.\n", time.GetHour(),time.GetMinute(),time.GetSecond(),GetName().c_str()));
	}
}

void CUser::UserLookChange(int pos, int itemid, int durability)
{
	if (pos >= SLOT_MAX) // let's leave it at this for the moment, the updated check needs considerable reworking
		return;

	Packet result(WIZ_USERLOOK_CHANGE);
	result << GetSocketID() << uint8_t(pos) << itemid << uint16_t(durability);
	SendToRegion(&result, this);
}

void CUser::SendNotice()
{
	Packet result(WIZ_NOTICE);
	uint8_t count = 0;

	result << uint8_t(2); // new-style notices (top-right of screen)

	size_t wpos = result.wpos();
	result << count; // placeholder the count

	// Use first line for header, 2nd line for data, 3rd line for header... etc.
	// It's most likely what they do officially (as usual, | is their line separator)
	for (int i = 0; i < 10; i += 2)
		AppendNoticeEntry(result, count, g_pMain->m_ppNotice[i + 1], g_pMain->m_ppNotice[i]);

	AppendExtraNoticeData(result, count);
	result.put(wpos, count); // replace the placeholdered line count

	Send(&result);
}

void CUser::AppendNoticeEntry(Packet & pkt, uint8_t & elementCount, const char * message, const char * title)
{
	if (message == nullptr || *message == '\0'
		|| title == nullptr || *title == '\0')
		return;

	pkt << title << message;
	elementCount++;
}

void CUser::AppendExtraNoticeData(Packet & pkt, uint8_t & elementCount)
{
	std::string message;
	if (g_pMain->m_byExpEventAmount > 0)
	{
		g_pMain->GetServerResource(IDS_EXP_REPAY_EVENT, &message, g_pMain->m_byExpEventAmount);
		AppendNoticeEntry(pkt, elementCount, message.c_str(), "EXP Event"); 
	}

	if (g_pMain->m_byCoinEventAmount > 0)
	{
		g_pMain->GetServerResource(IDS_MONEY_REPAY_EVENT, &message, g_pMain->m_byCoinEventAmount);
		AppendNoticeEntry(pkt, elementCount, message.c_str(), "Noah Event"); 
	}

	if (g_pMain->m_byNPEventAmount > 0)
	{
		g_pMain->GetServerResource(IDS_NP_REPAY_EVENT, &message, g_pMain->m_byNPEventAmount);
		AppendNoticeEntry(pkt, elementCount, message.c_str(), "NP Event"); 
	}
}

void CUser::SkillPointChange(Packet & pkt)
{
	uint8_t type = pkt.read<uint8_t>();
	Packet result(WIZ_SKILLPT_CHANGE);
	result << type;

	// invalid type
	if (type < SKILL_POINT_CAT_1
		|| type > SKILL_POINT_CAT_MASTER
		// not enough free skill points to allocate
			|| m_bstrSkill[0] < 1 
			// restrict skill points per category to your level
			|| m_bstrSkill[type] + 1 > GetLevel()
			// we need our first job change to assign skill points
			|| (GetClass() % 100) <= 4
			// to set points in the mastery category, we need to be mastered.
			|| (type == SKILL_POINT_CAT_MASTER
			&& ((GetClass() % 2) != 0 || (GetClass() % 100) < 6
			// force a limit of MAX_LEVEL - 60 (the level you can do the mastery quest)
			// on the master skill category, so the limit's 23 skill points with a level 83 cap.
			|| m_bstrSkill[type] >= (MAX_LEVEL  - 60) 
			// allow only 1 point in the master category for every level above 60.
			|| m_bstrSkill[type] >= (GetLevel() - 60))))

	{
		result << m_bstrSkill[type]; // only send the packet on failure
		Send(&result);
		return;
	}

	--m_bstrSkill[0];
	++m_bstrSkill[type];
}

void CUser::UpdateGameWeather(Packet & pkt)
{
	if (!isGM())	// is this user a GM?
		return;

	if (pkt.GetOpcode() == WIZ_WEATHER)
	{
		pkt >> g_pMain->m_byWeather >> g_pMain->m_sWeatherAmount;
	}
	else
	{
		uint16_t y, m, d;
		pkt >> y >> m >> d >> g_pMain->m_sHour >> g_pMain->m_sMin;
	}
	g_pMain->Send_All(&pkt); // pass the packet straight on
}

void CUser::GetUserInfoForAI(Packet & result)
{
	Guard lock(m_equippedItemBonusLock);

	result.SByte(); 
	result	<< GetSocketID()
		<< GetName() << GetZoneID() << GetNation() << GetLevel()
		<< m_sHp << m_sMp 
		<< m_sTotalHit << m_bAttackAmount
		<< m_sTotalAc << m_sACAmount
		<< m_fTotalHitrate << m_fTotalEvasionrate
		<< m_sItemAc
		<< GetPartyID() << GetAuthority()
		<< m_bInvisibilityType
		<< uint32_t(m_equippedItemBonuses.size());

	foreach (itr, m_equippedItemBonuses)
	{
		result << itr->first << uint32_t(itr->second.size()); // slot ID & number of bonuses
		foreach (bonusItr, itr->second)
			result << bonusItr->first << bonusItr->second; // bonus type, bonus amount
	}
}

void CUser::CountConcurrentUser()
{
	if (!isGM())
		return;

	uint16_t count = 0;
	SessionMap sessMap = g_pMain->m_socketMgr.GetActiveSessionMap();
	foreach (itr, sessMap)
	{
		if (TO_USER(itr->second)->isInGame())
			count++;
	}

	Packet result(WIZ_CONCURRENTUSER);
	result << count;
	Send(&result);
}

/**
* @brief	Takes a target's loyalty points (NP)
* 			and rewards some/all to the killer's party (current user).
*
* @param	tid		The target's ID.
* @param	bonusNP Bonus NP to be awarded to the killer's party as-is.
*/
void CUser::LoyaltyDivide(int16_t tid, uint16_t bonusNP /*= 0*/)
{
	if (m_bZone == ZONE_SNOW_BATTLE
		|| m_bZone == ZONE_DESPERATION_ABYSS 
		|| m_bZone == ZONE_HELL_ABYSS 
		|| m_bZone == ZONE_DRAGON_CAVE 
		|| m_bZone == ZONE_CAITHAROS_ARENA)
		return;

	int16_t loyalty_source = 0, loyalty_target = 0;
	uint8_t total_member = 0;

	if (!isInParty())
		return;

	_PARTY_GROUP *pParty = g_pMain->GetPartyPtr(GetPartyID());
	if (pParty == nullptr)
		return;

	CUser* pTUser = g_pMain->GetUserPtr(tid);
	if (pTUser == nullptr) 
		return;

	for (int i = 0; i < MAX_PARTY_USERS; i++)
	{
		CUser *pUser = g_pMain->GetUserPtr(pParty->uid[i]);
		if (pUser == nullptr)
			continue;

		total_member++;
	}

	if (total_member <= 0)
		return;

	//	This is for the Event Battle on Wednesday :(
	if (g_pMain->m_byBattleOpen
		&& GetZoneID() == (ZONE_BATTLE_BASE + g_pMain->m_byBattleZone))
	{
		if (pTUser->GetNation() == KARUS)
			g_pMain->m_sKarusDead++;
		else
			g_pMain->m_sElmoradDead++;
	}

	if (pTUser->GetNation() != GetNation()) // Different nations!!!
	{
		if (pTUser->GetLoyalty() == 0) // No cheats allowed...
		{
			loyalty_source = 0;
			loyalty_target = 0;
		}
		else
		{
			loyalty_source = GetLoyaltyDivideSource(total_member);
			loyalty_target = GetLoyaltyDivideTarget();

			if (loyalty_source == 0)
			{
				loyalty_source = 0;
				loyalty_target = 0;
			}
		}
	}
	else 
		return;

	for (int j = 0; j < MAX_PARTY_USERS; j++) // Distribute loyalty amongst party members.
	{
		bonusNP = 0;
		CUser *pUser = g_pMain->GetUserPtr(pParty->uid[j]);
		if (pUser == nullptr)
			continue;

		if (pUser->isAlive())
			pUser->SendLoyaltyChange(loyalty_source + bonusNP, true, false, pTUser->GetMonthlyLoyalty() > 0 ? true : false);
	}

	pTUser->SendLoyaltyChange(loyalty_target, true, false, pTUser->GetMonthlyLoyalty() > 0 ? true : false);
}

int16_t CUser::GetLoyaltyDivideSource(uint8_t totalmember)
{
	int16_t nBaseLoyalty = 0;

	if (GetZoneID() == ZONE_ARDREAM)
		nBaseLoyalty = g_pMain->m_Loyalty_Ardream_Source;
	else if (GetZoneID() == ZONE_RONARK_LAND_BASE)
		nBaseLoyalty = g_pMain->m_Loyalty_Ronark_Land_Base_Source;
	else if (GetZoneID() == ZONE_RONARK_LAND)
		nBaseLoyalty = g_pMain->m_Loyalty_Ronark_Land_Source;
	else if (GetZoneID() == ZONE_KROWAZ_DOMINION)
		nBaseLoyalty = (g_pMain->m_Loyalty_Other_Zone_Source / 100) * 20;
	else
		nBaseLoyalty = g_pMain->m_Loyalty_Other_Zone_Source;

	int16_t nMaxLoyalty = (nBaseLoyalty * 3) - 2;
	int16_t nMinLoyalty = nMaxLoyalty / MAX_PARTY_USERS;
	int16_t nLoyaltySource = nMinLoyalty;

	if (nLoyaltySource > 0)
	{
		for (int i = 0; i < (MAX_PARTY_USERS - totalmember); i++)
			nLoyaltySource += 2;
	}

	return nLoyaltySource -1;
}

int16_t CUser::GetLoyaltyDivideTarget()
{
	if (GetZoneID() == ZONE_ARDREAM)
		return g_pMain->m_Loyalty_Ardream_Target;
	else if (GetZoneID() == ZONE_RONARK_LAND_BASE)
		return g_pMain->m_Loyalty_Ronark_Land_Base_Target;
	else if (GetZoneID() == ZONE_RONARK_LAND)
		return g_pMain->m_Loyalty_Ronark_Land_Target;
	else if (GetZoneID() == ZONE_KROWAZ_DOMINION)
		return (g_pMain->m_Loyalty_Other_Zone_Target / 100) * 20;
	else
		return g_pMain->m_Loyalty_Other_Zone_Target;

	return 0;
}

void CUser::ItemWoreOut(int type, int damage)
{
	static uint8_t armourTypes[] = {HEAD, BREAST, LEG, GLOVE, FOOT };
	static uint8_t weaponsTypes[] = {RIGHTHAND, LEFTHAND};
	uint8_t totalSlots;
	int worerate;

	if (type == ACID_ALL)
	worerate = damage;
	else
	worerate = (int)sqrt(damage / 10.0f);

	if (worerate == 0) return;

	ASSERT(type == ATTACK || type == DEFENCE || type == REPAIR_ALL || type == ACID_ALL);

	// Inflict damage on equipped weapons.
	if (type == ATTACK)
		totalSlots = sizeof(weaponsTypes) / sizeof(*weaponsTypes); // use all the slots.
	// Inflict damage on equipped armour.
	else if (type == DEFENCE)
		totalSlots = sizeof(armourTypes) / sizeof(*armourTypes); // use all the slots.
	// Item repair all slots.
	else if (type == REPAIR_ALL)
		totalSlots = sizeof(armourTypes) / sizeof(*armourTypes); 
	// Item acid all slots.
	else if (type == ACID_ALL)
		totalSlots = sizeof(armourTypes) / sizeof(*armourTypes); 

	for (uint8_t i = 0; i < totalSlots; i++) 
	{
		uint8_t slot;
		if (type == DEFENCE)
		slot = armourTypes[i];
		else if (type == ATTACK)
		slot = weaponsTypes[i];
		else
		slot = armourTypes[i];

		_ITEM_DATA * pItem = GetItem(slot);
		_ITEM_TABLE * pTable = nullptr;

		// Is a non-broken item equipped?
		if (pItem == nullptr 
			|| (pItem->sDuration <= 0 && type != REPAIR_ALL) 
			// Does the item exist?
			|| (pTable = g_pMain->GetItemPtr(pItem->nNum)) == nullptr
			// If it's in the left or righthand slot, is it a shield? (this doesn't apply to weapons)
			|| (type == ATTACK 
			&& ((slot == LEFTHAND || slot == RIGHTHAND) && pTable->m_bSlot == ItemSlot1HLeftHand))) 
			continue;

		if (type == REPAIR_ALL)
		{
			pItem->sDuration = pTable->m_sDuration;
			SendDurability(slot, pItem->sDuration); 
			UserLookChange(slot, pItem->nNum, pItem->sDuration);
			continue;
		}

		int beforepercent = (int)((pItem->sDuration / (double)pTable->m_sDuration) * 100);
		int curpercent;

		if (worerate > pItem->sDuration)
			pItem->sDuration = 0;
		else 
			pItem->sDuration -= worerate;

		if (m_sItemArray[slot].sDuration == 0)
		{
			SendDurability(slot, 0);
			SetUserAbility(false);
			SendItemMove(1);
			continue;
		}

		SendDurability(slot, pItem->sDuration); 

		curpercent = (int)((pItem->sDuration / (double)pTable->m_sDuration) * 100);

		if ((curpercent / 5) != (beforepercent / 5)) 
		{
			if (curpercent >= 65 && curpercent < 70
				|| curpercent >= 25 && curpercent < 30)
				UserLookChange(slot, pItem->nNum, pItem->sDuration);
		}
	}
}

void CUser::SendDurability(uint8_t slot, uint16_t durability)
{
	Packet result(WIZ_DURATION);
	result << slot << durability;
	Send(&result);
}

void CUser::SendItemMove(uint8_t subcommand)
{
	Packet result(WIZ_ITEM_MOVE);

	result << subcommand;

	if (m_bAttackAmount == 0)
		m_bAttackAmount = 100;

	// If the subcommand is not error, send the stats.
	if (subcommand != 0)
	{
		result	<< uint16_t(m_sTotalHit * m_bAttackAmount / 100) 
			<< uint16_t(m_sTotalAc + m_sACAmount)
			<< uint16_t(m_sMaxWeight)
			<< m_iMaxHp << m_iMaxMp
			<< GetStatBonusTotal(STAT_STR) << GetStatBonusTotal(STAT_STA)
			<< GetStatBonusTotal(STAT_DEX) << GetStatBonusTotal(STAT_INT)
			<< GetStatBonusTotal(STAT_CHA)
			<< uint16_t(((m_sFireR + m_bAddFireR) * m_bPctFireR / 100) + m_bResistanceBonus) 
			<< uint16_t(((m_sColdR + m_bAddColdR) * m_bPctColdR / 100) + m_bResistanceBonus) 
			<< uint16_t(((m_sLightningR + m_bAddLightningR) * m_bPctLightningR / 100) + m_bResistanceBonus) 
			<< uint16_t(((m_sMagicR + m_bAddMagicR) * m_bPctMagicR / 100) + m_bResistanceBonus) 
			<< uint16_t(((m_sDiseaseR + m_bAddDiseaseR) * m_bPctDiseaseR / 100) + m_bResistanceBonus) 
			<< uint16_t(((m_sPoisonR + m_bAddPoisonR) * m_bPctPoisonR / 100) + m_bResistanceBonus);
	}
	Send(&result);
}

void CUser::HPTimeChange()
{
	m_tHPLastTimeNormal = UNIXTIME;

	if (isDead())
		return;

	int mpPercent = 100;

	if (GetZoneID() == ZONE_SNOW_BATTLE 
		&& g_pMain->m_byBattleOpen == SNOW_BATTLE)
	{
		HpChange(5);
		return;
	}

	// For mages with under 30% of HP
	if (CheckClass(110, 210)
		&& m_sMp < (30 * m_iMaxMp / 100))
		mpPercent = 120;

	if (m_bResHpType == USER_STANDING
		/* // unknown types
		|| m_bResHpType == 4 
		|| m_bResHpType == 5*/)
	{
		if (isTransformed())
		{
			// TODO: Apply transformation rates
		}
		else if (m_iMaxMp != m_sMp)
		{
			MSpChange((int)(((GetLevel() * (1 + GetLevel() / 60.0) + 1) * 0.2) + 3) * mpPercent / 100);
		}
	}
	else if (m_bResHpType == USER_SITDOWN)
	{
		if (isGM())
		{
			HpChange(m_iMaxHp);
			MSpChange(m_iMaxMp);
		}
		else
		{
			if (m_iMaxHp != m_sHp)
				HpChange((int)(GetLevel() * (1 + GetLevel() / 30.0)) + 3);

			if (m_iMaxMp != m_sMp)
				MSpChange((int)(((m_iMaxMp * 5) / ((GetLevel() - 1) + 30 )) + 3) * mpPercent / 100);
		}
	}
}

void CUser::HPTimeChangeType3()
{
	if (isDead()
		|| !m_bType3Flag)
		return;

	uint16_t	totalActiveDurationalSkills = 0, 
		totalActiveDOTSkills = 0;
	bool bIsDOT = false;
	for (int i = 0; i < MAX_TYPE3_REPEAT; i++)
	{
		MagicType3 * pEffect = &m_durationalSkills[i];
		if (!pEffect->m_byUsed)
			continue;

		// Has the required interval elapsed before using this skill?
		if ((UNIXTIME - pEffect->m_tHPLastTime) >= pEffect->m_bHPInterval)
		{
			Unit * pUnit = g_pMain->GetUnitPtr(pEffect->m_sSourceID);

			// Reduce the HP 
			HpChange(pEffect->m_sHPAmount, pUnit); // do we need to specify the source of the DOT?
			pEffect->m_tHPLastTime = UNIXTIME;

			if (pEffect->m_sHPAmount < 0)
				bIsDOT = true;

			// Has the skill expired yet?
			if (++pEffect->m_bTickCount == pEffect->m_bTickLimit)
			{
				Packet result(WIZ_MAGIC_PROCESS);
				result << uint8_t(MAGIC_DURATION_EXPIRED);
				// Healing-over-time skills require the type 100
				if (pEffect->m_sHPAmount > 0)
					result << uint8_t(100);
				else // Damage-over-time requires 200.
					result << uint8_t(200);

				Send(&result);

				// If the skill inflicts poison damage, remember to remove the poison indicator!
				if (pEffect->m_byAttribute == POISON_R)
					SendUserStatusUpdate(USER_STATUS_POISON, USER_STATUS_CURE);

				pEffect->Reset();
			}
		}

		if (pEffect->m_byUsed)
		{
			totalActiveDurationalSkills++;
			if (pEffect->m_sHPAmount < 0)
				totalActiveDOTSkills++;
		}
	}

	// Have all the skills expired?
	if (totalActiveDurationalSkills == 0)
		m_bType3Flag = false;

	// If there was DOT skills when we started, but none anymore... revert the HP bar.
	if (bIsDOT && totalActiveDOTSkills == 0)
		SendUserStatusUpdate(USER_STATUS_DOT, USER_STATUS_CURE);
}

void CUser::Type4Duration()
{
	Guard lock(m_buffLock);
	if (m_buffMap.empty())
		return;

	foreach (itr, m_buffMap)
	{
		if (itr->second.m_tEndTime > UNIXTIME)
			continue;

		CMagicProcess::RemoveType4Buff(itr->first, this, true, isLockableScroll(itr->second.m_bBuffType));
		break; // only ever handle one at a time with the current logic
	}

	if (!isDebuffed())
		SendUserStatusUpdate(USER_STATUS_POISON, USER_STATUS_CURE);
}

void CUser::SendAllKnightsID()
{
	Packet result(WIZ_KNIGHTS_LIST);
	uint16_t count = 0;

	result << uint8_t(1);

	size_t wpos = result.wpos();
	result << count;

	foreach_stlmap (itr, g_pMain->m_KnightsArray)
	{
		CKnights *pKnights = itr->second;
		if (pKnights == nullptr)
			continue;

		result << pKnights->m_sIndex << pKnights->m_strName;
		count++;
	}

	result.put(wpos, count);
	SendCompressed(&result);
}



void CUser::SpeedHackTime(Packet & pkt)
{
	if (!isInGame() || isGM())
		return;

	float nSpeed = 45.0f;

	if (GetFame() == COMMAND_CAPTAIN || isRogue())
		nSpeed = 90.0f;
	else if (isWarrior() || isMage() || isPriest())
		nSpeed = 67.0f;

	nSpeed += 10.0f; // Tolerance...

	float nRange = (pow(GetX() - m_LastX, 2.0f) + pow(GetZ() - m_LastZ, 2.0f)) / 100.0f;

	if (nRange >= nSpeed)
	{
		DateTime time;
		g_pMain->WriteCheatLogFile(string_format("[ SpeedHack - %d:%d:%d ] %s is Warp to Last Position.\n", time.GetHour(),time.GetMinute(),time.GetSecond(),GetName().c_str()));
		Warp(uint16_t(m_LastX) * 10, uint16_t(m_LastZ) * 10);
	}
	else
	{
		m_LastX = GetX();
		m_LastZ = GetZ();
	}

#if 0 // temporarily disabled
	uint8_t b_first;
	float servertime = 0.0f, clienttime = 0.0f, client_gap = 0.0f, server_gap = 0.0f;

	pkt >> b_first >> clienttime;

	if( b_first ) {
		m_fSpeedHackClientTime = clienttime;
		m_fSpeedHackServerTime = TimeGet();
	}
	else {
		servertime = TimeGet();

		server_gap = servertime - m_fSpeedHackServerTime;
		client_gap = clienttime - m_fSpeedHackClientTime;

		if( client_gap - server_gap > 10.0f ) {
			TRACE("%s SpeedHack User Checked By Server Time\n", m_id);
			Close();
		}
		else if( client_gap - server_gap < 0.0f ) {
			m_fSpeedHackClientTime = clienttime;
			m_fSpeedHackServerTime = TimeGet();
		}
	}
#endif
}

void CUser::SendOutputMsg(const char* msg, bool Success)
{
	if (msg == nullptr || strlen(msg) == 0)
		return;

	int16_t len = static_cast<int16_t>(strlen(msg));
	if (len > 1024) len = 1024; // avoid long messages

	Packet result(WIZ_MESSAGE_OUT);
	result << len;
	result.append(msg, len);
	result << static_cast<uint8_t>(Success);
	Send(&result);
}

int CUser::FindSlotForItem(uint32_t nItemID, uint16_t sCount /*= 1*/)
{
	int result = -1;
	_ITEM_TABLE *pTable = g_pMain->GetItemPtr(nItemID);
	if (pTable == nullptr)
		return result;

	// If the item's stackable, try to find it a home.
	// We could do this in the same logic, but I'd prefer one initial check
	// over the additional logic hit each loop iteration.
	if (pTable->m_bCountable)
	{
		for (int i = SLOT_MAX; i < SLOT_MAX+HAVE_MAX; i++)
		{
			_ITEM_DATA *pItem = GetItem(i);

			if(pItem == nullptr)
				continue; 

			// If it's the item we're after, and there will be room to store it...
			if (pItem
				&& pItem->nNum == nItemID 
				&& pItem->sCount + sCount <= ITEMCOUNT_MAX)
				return i;

			// Found a free slot, we'd prefer to stack it though
			// so store the first free slot, and ignore it.
			if (pItem->nNum == 0
				&& result < 0)
				result = i;
		}

		// If we didn't find a slot countaining our stackable item, it's possible we found
		// an empty slot. So return that (or -1 if it none was found; no point searching again).
		return result;
	}

	// If it's not stackable, don't need any additional logic.
	// Just find the first free slot.
	return GetEmptySlot();
}

int CUser::GetEmptySlot()
{
	for (int i = SLOT_MAX; i < SLOT_MAX+HAVE_MAX; i++)
	{
		_ITEM_DATA *pItem = GetItem(i);

		if (!pItem) 
			continue; 

		if (pItem->nNum == 0)
			return i;
	}

	return -1;
}

int CUser::NumEmptySlots()
{
	int iEmptySlotCount = 0;

	for (int i = SLOT_MAX; i < SLOT_MAX + HAVE_MAX; i++)
	{
		_ITEM_DATA *pItem = GetItem(i);

		if (!pItem)
			continue;

		if (pItem->nNum == 0)
			iEmptySlotCount++;
	}

	return iEmptySlotCount;
}

void CUser::Home()
{
	if (isDead()
		// When transformed into a "Kaul", you are unable to /town or attack.
			|| isKaul()
			/* No cheats allowed */
			|| GetHealth() < (GetMaxHealth() / 2)
			|| GetZoneID() == ZONE_CHAOS_DUNGEON
			|| GetZoneID() == ZONE_BORDER_DEFENSE_WAR
			|| hasBuff(BUFF_TYPE_FREEZE))
			return;

	// The point where you will be warped to.
	int16_t x = 0, z = 0;

	// Forgotten Temple
	if (GetZoneID() == ZONE_FORGOTTEN_TEMPLE)
	{
		KickOutZoneUser(true);
		return;
	}
	// Prevent /town'ing in quest arenas
	else if ((GetZoneID() / 10) == 5
		|| !GetStartPosition(x, z))
		return;

	Warp(x * 10, z * 10);
}

bool CUser::GetStartPosition(int16_t & x, int16_t & z, uint8_t bZone /*= 0 */)
{
	// Get start position data for current zone (unless we specified a zone).
	int nZoneID = (bZone == 0 ? GetZoneID() : bZone);
	_START_POSITION *pData = g_pMain->GetStartPosition(nZoneID);
	if (pData == nullptr)
		return false;

	_KNIGHTS_SIEGE_WARFARE *pKnightSiege = g_pMain->GetSiegeMasterKnightsPtr(1);

	// TODO: Allow for Delos/CSW.
	if ((pKnightSiege!=nullptr) && (pKnightSiege->sMasterKnights == GetClanID() && GetZoneID() == ZONE_DELOS && GetClanID() != 0))
	{
		x = 505 + myrand(0, pData->bRangeX);
		z = 840 + myrand(0, pData->bRangeZ);
	}
	else
	{
		// NOTE: This is how mgame does it.
		// This only allows for positive randomisation; we should really allow for the full range...
		if (GetNation() == KARUS) {
			x = pData->sKarusX + myrand(0, pData->bRangeX);
			z = pData->sKarusZ + myrand(0, pData->bRangeZ);
		} else {
			x = pData->sElmoradX + myrand(0, pData->bRangeX);
			z = pData->sElmoradZ + myrand(0, pData->bRangeZ);
		}
	}

	return true;
}

void CUser::ResetWindows()
{
	if (isTrading())
		ExchangeCancel();

	if (m_bRequestingChallenge)
		HandleChallengeCancelled(m_bRequestingChallenge);

	if (m_bChallengeRequested)
		HandleChallengeRejected(m_bChallengeRequested);

	// If we're a vendor, close the stall
	if (isMerchanting())
		MerchantClose();

	// If we're just browsing, free up our spot so others can browse the vendor.
	if (m_sMerchantsSocketID >= 0)
		CancelMerchant();

	/*if (isUsingStore())
	m_bStoreOpen = false;*/
}

CUser * CUser::GetItemRoutingUser(uint32_t nItemID, uint32_t sCount)
{
	if (!isInParty())
		return this;

	_ITEM_TABLE * pTable;
	_PARTY_GROUP * pParty = g_pMain->GetPartyPtr(GetPartyID());
	if (pParty == nullptr
		|| (pTable = g_pMain->GetItemPtr(nItemID)) == nullptr
		|| pParty->bItemRouting >= MAX_PARTY_USERS)
		return nullptr;

	for (int i = 0; i < MAX_PARTY_USERS; i++)
	{
		CUser * pUser = g_pMain->GetUserPtr(pParty->uid[pParty->bItemRouting]);

		if (pParty->bItemRouting > 6)
			pParty->bItemRouting = 0;
		else
			pParty->bItemRouting++;

		if (pUser != nullptr 
			&& pUser->CheckWeight(pTable, nItemID, sCount))
			return pUser;
	}

	return nullptr;
}

void CUser::ClassChangeReq()
{
	Packet result(WIZ_CLASS_CHANGE);
	result << uint8_t(CLASS_CHANGE_RESULT);
	if (GetLevel() < 10) // if we haven't got our first job change
		result << uint8_t(2);
	else if ((m_sClass % 100) > 4) // if we've already got our job change
		result << uint8_t(3);
	else // otherwise
		result << uint8_t(1);
	Send(&result);
}

// Dialog
void CUser::SendStatSkillDistribute()
{
	Packet result(WIZ_CLASS_CHANGE);
	result << uint8_t(CLASS_CHANGE_REQ);
	Send(&result); 
}

void CUser::AllSkillPointChange(bool bIsFree)
{
	Packet result(WIZ_CLASS_CHANGE);
	int index = 0, skill_point = 0, money = 0, temp_value = 0, old_money = 0;
	uint8_t type = 0;

	result << uint8_t(ALL_SKILLPT_CHANGE);

	temp_value = (int)pow((GetLevel() * 2.0f), 3.4f);
	if (GetLevel() < 30)		
		temp_value = (int)(temp_value * 0.4f);
	else if (GetLevel() >= 60)
		temp_value = (int)(temp_value * 1.5f);

	//temp_value = (int)(temp_value * 1.5f);

	// If global discounts are enabled 
	if (g_pMain->m_sDiscount == 2 // or war discounts are enabled
		|| (g_pMain->m_sDiscount == 1 && g_pMain->m_byOldVictory == m_bNation))
		temp_value /= 2;

	// Level too low.
	if (GetLevel() < 10)
		goto fail_return;

	// Get total skill points
	for (int i = 1; i < 9; i++)
		skill_point += m_bstrSkill[i];

	// If we don't have any skill points, there's no point resetting now is there.
	if (skill_point <= 0)
	{
		type = 2;
		goto fail_return;
	}

	// Not enough money.
	if (!bIsFree & !GoldLose(temp_value, false))
		goto fail_return;

	// Reset skill points.
	m_bstrSkill[0] = (GetLevel() - 9) * 2;
	for (int i = 1; i < 9; i++)	
		m_bstrSkill[i] = 0;

	result << uint8_t(1) << GetCoins() << m_bstrSkill[0];
	Send(&result);
	return;

fail_return:
	result << type << temp_value;
	Send(&result);
}

void CUser::AllPointChange(bool bIsFree)
{
	Packet result(WIZ_CLASS_CHANGE);
	int temp_money;
	uint16_t statTotal;

	uint16_t byStr, bySta, byDex, byInt, byCha;
	uint8_t bResult = 0;

	result << uint8_t(ALL_POINT_CHANGE);

	if (GetLevel() > MAX_LEVEL)
		goto fail_return;

	temp_money = (int)pow((GetLevel() * 2.0f ), 3.4f);
	if (GetLevel() < 30)
		temp_money = (int)(temp_money * 0.4f);
	else if (GetLevel() >= 60) 
		temp_money = (int)(temp_money * 1.5f);

	if ((g_pMain->m_sDiscount == 1 && g_pMain->m_byOldVictory == GetNation())
		|| g_pMain->m_sDiscount == 2)
		temp_money /= 2;

	for (int i = 0; i < SLOT_MAX; i++)
	{
		if (m_sItemArray[i].nNum) {
			bResult = 4;
			goto fail_return;
		}
	}

	// It's 300-10 for clarity (the 10 being the stat points assigned on char creation)
	if (GetStatTotal() == 290)
	{
		bResult = 2; // don't need to reallocate stats, it has been done already...
		goto fail_return;
	}

	// Not enough coins
	if (!bIsFree & !GoldLose(temp_money, false))
		goto fail_return;

	// TODO: Pull this from the database.
	switch (m_bRace)
	{
	case KARUS_BIG:	
		if(isWarrior())
		{
		SetStat(STAT_STR, 65);
		SetStat(STAT_STA, 65);
		SetStat(STAT_DEX, 60);
		SetStat(STAT_INT, 50);
		SetStat(STAT_CHA, 50);
		}
		break;
	case KARUS_MIDDLE:
		if(isWarrior())
		{
		SetStat(STAT_STR, 65);
		SetStat(STAT_STA, 65);
		SetStat(STAT_DEX, 60);
		SetStat(STAT_INT, 50);
		SetStat(STAT_CHA, 50);
		}
		else
		{
		SetStat(STAT_STR, 60);
		SetStat(STAT_STA, 60);
		SetStat(STAT_DEX, 70);
		SetStat(STAT_INT, 50);
		SetStat(STAT_CHA, 50);
		}
		break;
	case KARUS_SMALL:
		if (isWarrior())
		{
		SetStat(STAT_STR, 65);
		SetStat(STAT_STA, 65);
		SetStat(STAT_DEX, 60);
		SetStat(STAT_INT, 50);
		SetStat(STAT_CHA, 50);
		}
		else if(isRogue())
		{
		SetStat(STAT_STR, 60);
		SetStat(STAT_STA, 60);
		SetStat(STAT_DEX, 70);
		SetStat(STAT_INT, 50);
		SetStat(STAT_CHA, 50);
		}
		else if(isPriest())
		{
		SetStat(STAT_STR, 50);
		SetStat(STAT_STA, 50);
		SetStat(STAT_DEX, 70);
		SetStat(STAT_INT, 70);
		SetStat(STAT_CHA, 50);
		}
		else  if(isMage())
		{
		SetStat(STAT_STR, 50);
		SetStat(STAT_STA, 60);
		SetStat(STAT_DEX, 60);
		SetStat(STAT_INT, 70);
		SetStat(STAT_CHA, 50);
		}
		break;
	case KARUS_WOMAN:
		if (isWarrior())
		{
		SetStat(STAT_STR, 65);
		SetStat(STAT_STA, 65);
		SetStat(STAT_DEX, 60);
		SetStat(STAT_INT, 50);
		SetStat(STAT_CHA, 50);
		}
		else if(isRogue())
		{
		SetStat(STAT_STR, 60);
		SetStat(STAT_STA, 60);
		SetStat(STAT_DEX, 70);
		SetStat(STAT_INT, 50);
		SetStat(STAT_CHA, 50);
		}
		else if(isPriest())
		{
		SetStat(STAT_STR, 50);
		SetStat(STAT_STA, 50);
		SetStat(STAT_DEX, 70);
		SetStat(STAT_INT, 70);
		SetStat(STAT_CHA, 50);
		}
		else  if(isMage())
		{
		SetStat(STAT_STR, 50);
		SetStat(STAT_STA, 60);
		SetStat(STAT_DEX, 60);
		SetStat(STAT_INT, 70);
		SetStat(STAT_CHA, 50);
		}
		break;
	case BABARIAN:
		if (isWarrior())
		{
		SetStat(STAT_STR, 65);
		SetStat(STAT_STA, 65);
		SetStat(STAT_DEX, 60);
		SetStat(STAT_INT, 50);
		SetStat(STAT_CHA, 50);
		}
		break;
	case ELMORAD_MAN:
		if (isWarrior())
		{
		SetStat(STAT_STR, 65);
		SetStat(STAT_STA, 65);
		SetStat(STAT_DEX, 60);
		SetStat(STAT_INT, 50);
		SetStat(STAT_CHA, 50);
		}
		else if(isRogue())
		{
		SetStat(STAT_STR, 60);
		SetStat(STAT_STA, 60);
		SetStat(STAT_DEX, 70);
		SetStat(STAT_INT, 50);
		SetStat(STAT_CHA, 50);
		}
		else if(isPriest())
		{
		SetStat(STAT_STR, 50);
		SetStat(STAT_STA, 50);
		SetStat(STAT_DEX, 70);
		SetStat(STAT_INT, 70);
		SetStat(STAT_CHA, 50);
		}
		else  if(isMage())
		{
		SetStat(STAT_STR, 50);
		SetStat(STAT_STA, 60);
		SetStat(STAT_DEX, 60);
		SetStat(STAT_INT, 70);
		SetStat(STAT_CHA, 50);
		}
		break;
	case ELMORAD_WOMAN:	
		if (isWarrior())
		{
		SetStat(STAT_STR, 65);
		SetStat(STAT_STA, 65);
		SetStat(STAT_DEX, 60);
		SetStat(STAT_INT, 50);
		SetStat(STAT_CHA, 50);
		}
		else if(isRogue())
		{
		SetStat(STAT_STR, 60);
		SetStat(STAT_STA, 60);
		SetStat(STAT_DEX, 70);
		SetStat(STAT_INT, 50);
		SetStat(STAT_CHA, 50);
		}
		else if(isPriest())
		{
		SetStat(STAT_STR, 50);
		SetStat(STAT_STA, 50);
		SetStat(STAT_DEX, 70);
		SetStat(STAT_INT, 70);
		SetStat(STAT_CHA, 50);
		}
		else  if(isMage())
		{
		SetStat(STAT_STR, 50);
		SetStat(STAT_STA, 60);
		SetStat(STAT_DEX, 60);
		SetStat(STAT_INT, 70);
		SetStat(STAT_CHA, 50);
		}
		break;
	}

	// Players gain 3 stats points for each level up to and including 60.
	// They also received 10 free stat points on creation. 
	m_sPoints = 10 + (GetLevel() - 1) * 3;

	// For every level after 60, we add an additional two points.
	if (GetLevel() > 60)
		m_sPoints += 2 * (GetLevel() - 60);

	statTotal = GetStatTotal();
	ASSERT(statTotal == 290);

	SetUserAbility();
	Send2AI_UserUpdateInfo();

	byStr = GetStat(STAT_STR);
	bySta = GetStat(STAT_STA), 
	byDex = GetStat(STAT_DEX);
	byInt = GetStat(STAT_INT),
	byCha = GetStat(STAT_CHA);

	result << uint8_t(1) // result (success)
		<< GetCoins()
		<< byStr << bySta << byDex << byInt << byCha 
		<< m_iMaxHp << m_iMaxMp << m_sTotalHit << uint16_t(m_sMaxWeight) << m_sPoints;
	Send(&result);
	return;

fail_return:
	result << bResult << temp_money;
	Send(&result);
}

void CUser::GoldChange(int16_t tid, int gold)
{
	if (m_bZone < 3) return;	// Money only changes in Frontier zone and Battle zone!!!
	if (m_bZone == ZONE_SNOW_BATTLE
		|| m_bZone == ZONE_DESPERATION_ABYSS 
		|| m_bZone == ZONE_HELL_ABYSS 
		|| m_bZone == ZONE_DRAGON_CAVE 
		|| m_bZone == ZONE_CAITHAROS_ARENA)
		return;

	CUser* pTUser = g_pMain->GetUserPtr(tid);
	if (pTUser == nullptr || pTUser->m_iGold <= 0)
		return;

	// Reward money in war zone
	if (gold == 0)
	{
		// If we're not in a party, we can distribute cleanly.
		if (!isInParty())
		{
			GoldGain((pTUser->m_iGold * 4) / 10);
			pTUser->GoldLose(pTUser->m_iGold / 2);
			return;
		}

		// Otherwise, if we're in a party, we need to divide it up.
		_PARTY_GROUP* pParty = g_pMain->GetPartyPtr(GetPartyID());
		if (pParty == nullptr)
			return;

		int userCount = 0, levelSum = 0, temp_gold = (pTUser->m_iGold * 4) / 10;
		pTUser->GoldLose(pTUser->m_iGold / 2);

		// TODO: Clean up the party system. 
		for (int i = 0; i < MAX_PARTY_USERS; i++)
		{
			CUser *pUser = g_pMain->GetUserPtr(pParty->uid[i]);
			if (pUser == nullptr)
				continue;

			userCount++;
			levelSum += pUser->GetLevel();
		}

		// No users (this should never happen! Needs to be cleaned up...), don't bother with the below loop.
		if (userCount == 0) 
			return;

		for (int i = 0; i < MAX_PARTY_USERS; i++)
		{		
			CUser * pUser = g_pMain->GetUserPtr(pParty->uid[i]);
			if (pUser == nullptr)
				continue;

			pUser->GoldGain((int)(temp_gold * (float)(pUser->GetLevel() / (float)levelSum)));
		}			
		return;
	}

	// Otherwise, use the coin amount provided.

	// Source gains money
	if (gold > 0)
	{
		GoldGain(gold);
		pTUser->GoldLose(gold);
	}
	// Source loses money
	else
	{
		GoldLose(gold);
		pTUser->GoldGain(gold);
	}
}

void CUser::SelectWarpList(Packet & pkt)
{
	if (isDead())
		return;

	uint16_t /*npcid,*/ warpid;
	pkt /*>> npcid*/ >> warpid;

	_WARP_INFO *pWarp = GetMap()->GetWarp(warpid);
	if (pWarp == nullptr
		|| (pWarp->sNation != 0 && pWarp->sNation != GetNation()))
		return;

	C3DMap *pMap = g_pMain->GetZoneByID(pWarp->sZone);
	if (pMap == nullptr)
		return;

	_ZONE_SERVERINFO *pInfo = g_pMain->m_ServerArray.GetData(pMap->m_nServerNo);
	if (pInfo == nullptr)
		return;

	float rx = 0.0f, rz = 0.0f;
	rx = (float)myrand(0, (int)pWarp->fR * 2);
	if (rx < pWarp->fR)
		rx = -rx;

	rz = (float)myrand(0, (int)pWarp->fR * 2);
	if (rz < pWarp->fR)
		rz = -rz;

	if (m_bZone == pWarp->sZone) 
	{
		m_bZoneChangeSameZone = true;

		Packet result(WIZ_WARP_LIST);
		result << uint8_t(2) << uint8_t(1);
		Send(&result);
	}

	ZoneChange(pWarp->sZone, pWarp->fX + rx, pWarp->fZ + rz);

	if (GetZoneID() == pWarp->sZone && pWarp->dwPay > 0 && hasCoins(pWarp->dwPay))
		GoldLose(pWarp->dwPay);
}

void CUser::ServerChangeOk(Packet & pkt)
{
	C3DMap* pMap = GetMap();
	if (pMap == nullptr)
		return;

	uint16_t warpid = pkt.read<uint16_t>();
	_WARP_INFO* pWarp = pMap->GetWarp(warpid);
	if (pWarp == nullptr)
		return;

	float rx = 0.0f, rz = 0.0f;
	rx = (float)myrand(0, (int)pWarp->fR * 2);
	if (rx < pWarp->fR)
		rx = -rx;

	rz = (float)myrand(0, (int)pWarp->fR * 2);
	if (rz < pWarp->fR)
		rz = -rz;

	ZoneChange(pWarp->sZone, pWarp->fX + rx, pWarp->fZ + rz);
}

bool CUser::GetWarpList(int warp_group)
{
	C3DMap* pMap = GetMap();
	std::set<_WARP_INFO*> warpList;

	if(pMap == nullptr)
		return false; 

	pMap->GetWarpList(warp_group, warpList);

	Packet result(WIZ_WARP_LIST);
	result << uint8_t(1) << uint16_t(warpList.size());
	foreach (itr, warpList)
	{
		C3DMap *pDstMap = g_pMain->GetZoneByID((*itr)->sZone);
		if (pDstMap == nullptr)
			continue;

		if (g_pMain->isWarOpen() 
			&& ((g_pMain->m_byBattleZoneType != ZONE_ARDREAM 
			&& ((*itr)->sZone == ZONE_ARDREAM 
			|| (*itr)->sZone == ZONE_RONARK_LAND_BASE
			|| (*itr)->sZone == ZONE_RONARK_LAND))
			|| (g_pMain->m_byBattleZoneType == ZONE_ARDREAM 
			&& (*itr)->sZone == ZONE_ARDREAM)))
			continue;

		result << (*itr)->sWarpID
			<< (*itr)->strWarpName << (*itr)->strAnnounce
			<< (*itr)->sZone
			<< pDstMap->m_sMaxUser
			<< uint32_t((*itr)->dwPay)//;

			<< uint16_t((*itr)->fX*10.0f) << uint16_t((*itr)->fZ*10.0f) << uint16_t((*itr)->fY*10.0f);
	}

	Send(&result);
	return true;
}

bool CUser::BindObjectEvent(_OBJECT_EVENT *pEvent)
{
	if (pEvent->sBelong != 0 && pEvent->sBelong != GetNation())
		return false;

	m_sBind = pEvent->sIndex;

	Packet result(WIZ_OBJECT_EVENT);
	result << uint8_t(pEvent->sType) << uint8_t(1);
	Send(&result);
	return true;
}

bool CUser::GateLeverObjectEvent(
	_OBJECT_EVENT* pEvent,
	int nid)
{
	C3DMap* pMap = GetMap();
	if (pMap == nullptr)
		return false;

	// Does the lever (object) NPC exist?
	CNpc* pNpc = g_pMain->GetNpcPtr(nid);
	if (pNpc == nullptr)
		return false;

	// Does the corresponding gate object event exist?
	_OBJECT_EVENT* pGateEvent = pMap->GetObjectEvent(pEvent->sControlNpcID);
	if (pGateEvent == nullptr)
		return false;

	// Does the corresponding gate (object) NPC exist?
	CNpc* pGateNpc = g_pMain->FindNpcInZone(pEvent->sControlNpcID, GetZoneID());
	if (pGateNpc == nullptr)
		return false;

	// Is it even a gate?
	if (!pGateNpc->isGate())
		return false;

	// If the gate's closed (i.e. the lever is down), we can't open it unless the lever isn't nation-specific
	// or we're the correct nation. Seems the other nation cannot close them.
	if (pNpc->isGateClosed()
		&& pNpc->GetNation() != 0
		&& pNpc->GetNation() != GetNation())
		return false;

	// NOTE: This logic is most likely egregiously custom...
	// In Delos...
	if (GetZoneID() == ZONE_DELOS)
	{
		// If a clan currently holds the castle...
		_KNIGHTS_SIEGE_WARFARE* pKnightSiegeWarfare = g_pMain->GetSiegeMasterKnightsPtr(1);
		if (pKnightSiegeWarfare != nullptr
			&& pKnightSiegeWarfare->sMasterKnights != 0)
		{
			CKnights* pKnight = g_pMain->GetClanPtr(GetClanID());
			if (pKnight != nullptr)
			{
				// Only allow that clan's leader to be able to open/close the gate.
				if (pKnight->m_sIndex != pKnightSiegeWarfare->sMasterKnights
					|| !isClanLeader())
					return false;
			}
		}
	}
	// In all other zones (presumably we're assuming war zones here),
	// only Kings and war captains/commanders can open/close gates.
	else
	{
		if (GetFame() != COMMAND_CAPTAIN
			&& !isKing())
			return false;
	}

	// Move the lever (up/down).
	pNpc->SendGateFlag(!pNpc->m_byGateOpen);

	// Open/close the gate.
	pGateNpc->SendGateFlag(!pGateNpc->m_byGateOpen);
	return true;
}

/***
* Not sure what this is used for, so keeping logic the same just in case.
***/
bool CUser::FlagObjectEvent(_OBJECT_EVENT *pEvent, int nid)
{
	_OBJECT_EVENT *pFlagEvent;
	CNpc *pNpc, *pFlagNpc;

	// Does the flag object NPC exist?
	if ((pNpc = g_pMain->GetNpcPtr(nid)) == nullptr
		// Does the corresponding flag event exist?
			|| (pFlagEvent = GetMap()->GetObjectEvent(pEvent->sControlNpcID)) == nullptr
			// Does the corresponding flag object NPC exist?
			|| (pFlagNpc = g_pMain->FindNpcInZone(pEvent->sControlNpcID, GetZoneID())) == nullptr
			// Is this marked a gate? (i.e. can control)
			|| !pFlagNpc->isGate()
			// Is the war over or the gate closed?
			|| g_pMain->m_bVictory > 0 || pNpc->isGateClosed())
			return false;

	// Reset objects
	pNpc->SendGateFlag(0);
	pFlagNpc->SendGateFlag(0);

	// Add flag score (not sure what this is, is this even used anymore?)
	if (GetNation() == KARUS) 
		g_pMain->m_bKarusFlag++;
	else
		g_pMain->m_bElmoradFlag++;

	// Did one of the teams win?
	g_pMain->BattleZoneVictoryCheck();	
	return true;
}

bool CUser::WarpListObjectEvent(_OBJECT_EVENT *pEvent)
{
	// If the warp gate belongs to a nation, which isn't us...
	if (pEvent->sBelong != 0 && pEvent->sBelong != GetNation()
		// or we're in the opposing nation's zone...
			|| (GetZoneID() != GetNation() && GetZoneID() <= ELMORAD)
			// or we're unable to retrieve the warp list...
			|| !GetWarpList(pEvent->sControlNpcID)) 
			return false;

	return true;
}

void CUser::ObjectEvent(Packet & pkt)
{
	if (g_pMain->m_bPointCheckFlag == false
		|| isDead())
		return;

	bool bSuccess = false;
	uint16_t objectindex, nid;
	pkt >> objectindex >> nid;

	_OBJECT_EVENT * pEvent = GetMap()->GetObjectEvent(objectindex);
	if (pEvent != nullptr
		&& isInRange(pEvent->fPosX, pEvent->fPosZ, MAX_OBJECT_RANGE))
	{
		switch (pEvent->sType)
		{
		case OBJECT_GATE:
		case OBJECT_BIND:
		case OBJECT_REMOVE_BIND:
			bSuccess = BindObjectEvent(pEvent);
			break;

		case OBJECT_GATE_LEVER:
			bSuccess = GateLeverObjectEvent(pEvent, nid);
			break;

		case OBJECT_FLAG_LEVER:
			bSuccess = FlagObjectEvent(pEvent, nid);
			break;

		case OBJECT_WARP_GATE:
			bSuccess = WarpListObjectEvent(pEvent);
			if (bSuccess)
				return;
			break;

		case OBJECT_ANVIL:
			SendAnvilRequest(nid);
			return;
		}

	}

	if (!bSuccess)
	{
		Packet result(WIZ_OBJECT_EVENT);
		result << uint8_t(pEvent == nullptr ? 0 : pEvent->sType) << uint8_t(0);
		Send(&result);
	}
}

void CUser::SendAnvilRequest(uint16_t sNpcID, uint8_t bType /*= ITEM_UPGRADE_REQ*/)
{
	Packet result(WIZ_ITEM_UPGRADE);
	result << uint8_t(bType) << sNpcID;
	Send(&result);
}

void CUser::UpdateVisibility(InvisibilityType bNewType)
{
	Packet result(AG_USER_VISIBILITY);
	m_bInvisibilityType = (uint8_t)(bNewType);
	result << GetID() << m_bInvisibilityType;
	Send_AIServer(&result);
}

/**
* @brief	Forces a reset of the GM's persistent visibility 
* 			state.
*/
void CUser::ResetGMVisibility()
{
	if (!isGM()
		|| isTransformed())
		return;

	// Force the client to reset
	if (m_bAbnormalType != ABNORMAL_INVISIBLE)
	{
		// Only send this packet to the GM as other users 
		// will already see the GM as invisible.
		Packet result(WIZ_STATE_CHANGE);
		result << GetID() << uint8_t(5) << uint32_t(0);
		Send(&result);
	}

	m_bAbnormalType = ABNORMAL_INVISIBLE;
}

void CUser::BlinkStart()
{
#if !defined(DISABLE_PLAYER_BLINKING)
	// Don't blink in zones where we can attack the other nation (note: this includes the arena!)
	if (GetMap()->canAttackOtherNation())
		return;

	m_bAbnormalType = ABNORMAL_BLINKING;
	m_tBlinkExpiryTime = UNIXTIME + BLINK_TIME;
	m_bRegeneType = REGENE_ZONECHANGE;

	UpdateVisibility(INVIS_DISPEL_ON_ATTACK); // AI shouldn't see us
	m_bInvisibilityType = INVIS_NONE; // but players should. 

	StateChangeServerDirect(3, ABNORMAL_BLINKING);
#endif
}

void CUser::BlinkTimeCheck()
{
	if (UNIXTIME < m_tBlinkExpiryTime)
		return;

	m_bRegeneType = REGENE_NORMAL;

	StateChangeServerDirect(3, ABNORMAL_NORMAL);

	Packet result(AG_USER_REGENE);
	result	<< GetSocketID() << m_sHp;
	Send_AIServer(&result);

	result.Initialize(AG_USER_INOUT);
	result.SByte(); // TODO: Remove this redundant uselessness that is mgame
	result	<< uint8_t(INOUT_RESPAWN) << GetSocketID()
		<< GetName()
		<< GetX() << GetZ();
	Send_AIServer(&result);

	UpdateVisibility(INVIS_NONE);
}

void CUser::GoldGain(uint32_t gold, bool bSendPacket /*= true*/, bool bApplyBonus /*= false*/)
{
	// Assuming it works like this, although this affects (probably) all gold gained (including kills in PvP zones)
	// If this is wrong and it should ONLY affect gold gained from monsters, let us know!
	if (bApplyBonus)
		gold = gold * (m_bNoahGainAmount + m_bItemNoahGainAmount) / 100;

	if (m_iGold + gold > COIN_MAX)
		m_iGold = COIN_MAX;
	else
		m_iGold += gold;

	if (bSendPacket)
	{
		Packet result(WIZ_GOLD_CHANGE);
		result << uint8_t(CoinGain) << gold << GetCoins();
		Send(&result);	
	}
}

bool CUser::GoldLose(uint32_t gold, bool bSendPacket /*= true*/)
{
	if (!hasCoins(gold)) 
		return false;

	m_iGold -= gold;

	if (bSendPacket)
	{
		Packet result(WIZ_GOLD_CHANGE);
		result << uint8_t(CoinLoss) << gold << GetCoins();
		Send(&result);	
	}
	return true;
}

bool CUser::CheckSkillPoint(uint8_t skillnum, uint8_t min, uint8_t max)
{
	if (skillnum < 5 || skillnum > 8) 
		return false;

	return (m_bstrSkill[skillnum] >= min && m_bstrSkill[skillnum] <= max);
}

bool CUser::CheckClass(int16_t class1, int16_t class2, int16_t class3, int16_t class4, int16_t class5, int16_t class6)
{
	return (JobGroupCheck(class1) || JobGroupCheck(class2) || JobGroupCheck(class3) || JobGroupCheck(class4) || JobGroupCheck(class5) || JobGroupCheck(class6));
}

bool CUser::JobGroupCheck(int16_t jobgroupid)
{
	if (jobgroupid > 100) 
		return GetClass() == jobgroupid;

	ClassType subClass = GetBaseClassType();
	switch (jobgroupid) 
	{
	case GROUP_WARRIOR:
		return (subClass == ClassWarrior || subClass == ClassWarriorNovice || subClass == ClassWarriorMaster);

	case GROUP_ROGUE:
		return (subClass == ClassRogue || subClass == ClassRogueNovice || subClass == ClassRogueMaster);

	case GROUP_MAGE:
		return (subClass == ClassMage || subClass == ClassMageNovice || subClass == ClassMageMaster);

	case GROUP_CLERIC:	
		return (subClass == ClassPriest || subClass == ClassPriestNovice || subClass == ClassPriestMaster);
	}

	return (subClass == jobgroupid);
}

void CUser::TrapProcess()
{
	// If the time interval has passed
	if ((UNIXTIME - m_tLastTrapAreaTime) >= ZONE_TRAP_INTERVAL)
	{
		if(GetZoneID() == ZONE_BIFROST)
			SendUserStatusUpdate(USER_STATUS_BLIND,USER_STATUS_INFLICT);

		HpChange(-ZONE_TRAP_DAMAGE, this);
		m_tLastTrapAreaTime = UNIXTIME;
	}
}

void CUser::KickOutZoneUser(bool home, uint8_t nZoneID)
{
	if (home)
	{
		C3DMap* pMap = g_pMain->GetZoneByID(nZoneID);
		if (pMap == nullptr) 
			return;

		int eventID = 0;
		int random = myrand(0, 9000);
		if (random >= 0 && random < 3000)			eventID = 0;
		else if (random >= 3000 && random < 6000)	eventID = 1;
		else if (random >= 6000 && random < 9001)	eventID = 2;

		_REGENE_EVENT* pRegene = pMap->GetRegeneEvent(eventID);
		if (pRegene == nullptr) 
		{
			KickOutZoneUser();
			return;
		}

		ZoneChange(pMap->m_nZoneNumber, 
			pRegene->fRegenePosX + (float)myrand(0, (int)pRegene->fRegeneAreaX), 
			pRegene->fRegenePosZ + (float)myrand(0, (int)pRegene->fRegeneAreaZ));
		return;
	}

	// Teleport the player to their native zone.
	int16_t x, z;

	if (GetStartPosition(x,z,GetNation()))
		ZoneChange(GetNation(), x, z);
	else
	{
		TRACE("### KickOutZoneUser - StartPosition not found : Nation=%d",GetNation());
	}
}

void CUser::NativeZoneReturn()
{
	_START_POSITION *pStartPosition = g_pMain->m_StartPositionArray.GetData(m_bNation);
	if (pStartPosition == nullptr) 
		return; 

	m_bZone = m_bNation;

	m_curx = (float)((m_bNation == KARUS ? pStartPosition->sKarusX : pStartPosition->sElmoradX) + myrand(0, pStartPosition->bRangeX));
	m_curz = (float)((m_bNation == KARUS ? pStartPosition->sKarusZ : pStartPosition->sElmoradZ) + myrand(0, pStartPosition->bRangeZ)); 
}

/**
* @brief	Sends a packet to all players within the 
* 			user's current region and surrounding regions 
* 			(i.e. visible area)
*
* @param	pkt		   	The packet.
* @param	pExceptUser	User to except. If specified, will ignore this user.
*/
void CUser::SendToRegion(Packet* pkt, CUser* pExceptUser /*= nullptr*/)
{
	g_pMain->Send_Region(pkt, GetMap(), GetRegionX(), GetRegionZ(), pExceptUser);
}

/**
* @brief	Sends a packet to all players within the 
* 			user's current zone.
*
* @param	pkt		   	The packet.
* @param	pExceptUser	User to except. If specified, will ignore this user.
*/
void CUser::SendToZone(Packet* pkt, CUser* pExceptUser /*= nullptr*/, float fRange)
{
	g_pMain->Send_Zone(pkt, GetZoneID(), pExceptUser, 0, fRange);
}

void CUser::OnDeath(Unit *pKiller)
{
	if (m_bResHpType == USER_DEAD)
		return;

	m_bResHpType = USER_DEAD;

	// Player is dead stop other process.
	ResetWindows();

	if (GetFame() == COMMAND_CAPTAIN)
	{
		if (GetNation() == KARUS)
			g_pMain->Announcement(KARUS_CAPTAIN_DEPRIVE_NOTIFY, KARUS, 8, this);
		else
			g_pMain->Announcement(ELMORAD_CAPTAIN_DEPRIVE_NOTIFY, ELMORAD, 8, this);
	}

	InitType3();
	InitType4();

	DateTime time;

	if (pKiller != nullptr)
	{
		e_DeathNoticeType noticeType = DEATH_NOTICE_NONE;

		if (pKiller->isNPC())
		{
			CNpc *pNpc = TO_NPC(pKiller);

			int64_t nExpLost = 0;

			if (pNpc->GetType() == NPC_PATROL_GUARD || (GetZoneID() != GetNation() && GetZoneID() <= ELMORAD))
				nExpLost = m_iMaxExp / 100;
			else
				nExpLost = m_iMaxExp / 20;

			if ((pNpc->GetType() == NPC_GUARD_TOWER_NEW || pNpc->GetType() == NPC_GUARD_TOWER) && isInPKZone())
				noticeType = DEATH_NOTICE_GUARD_TOWER;

			if (GetPremiumProperty(PremiumExpRestorePercent) > 0)
				nExpLost = nExpLost * (GetPremiumProperty(PremiumExpRestorePercent)) / 100;

			g_pMain->WriteDeathUserLogFile(string_format("[ NPC/MONSTER - %d:%d:%d ] SID=%d,Killer=%s,Target=%s,Zone=%d,X=%d,Z=%d,TargetExp=%d,LostExp=%d\n",time.GetHour(),time.GetMinute(),time.GetSecond(),pNpc->GetProtoID(),pKiller->GetName().c_str(),GetName().c_str(),GetZoneID(),uint16_t(GetX()),uint16_t(GetZ()),m_iExp, nExpLost));
			
			if (GetZoneID() != ZONE_FORGOTTEN_TEMPLE)
			ExpChange(-nExpLost);	

			if (GetZoneID() == ZONE_FORGOTTEN_TEMPLE)
			{
				KickOutZoneUser(true);
				return;
			}
		}
		else
		{
			CUser *pUser = TO_USER(pKiller);

			// Looks like we died of "natural causes!" (probably residual DOT)
			if (pUser == this)
			{
				m_sWhoKilledMe = -1;
			}
			// Someone else killed us? Need to clean this up.
			else
			{
				if (GetZoneID() == ZONE_CHAOS_DUNGEON)
				{
					noticeType = DEATH_NOTICE_COORDINATES;
					RobChaosSkillItems();
					m_DeathCount++;
					UpdatePlayerRank();

					pUser->m_KillCount++;
					pUser->UpdatePlayerRank();
				}
				else
				{
					// Did we get killed in the snow war? Handle appropriately.
					if (GetZoneID() == ZONE_SNOW_BATTLE
						&& g_pMain->m_byBattleOpen == SNOW_BATTLE)
					{
						pUser->GoldGain(SNOW_EVENT_MONEY);
						pUser->LoyaltyChange(GetID());

						if (GetNation() == KARUS)
							g_pMain->m_sKarusDead++;
						else
							g_pMain->m_sElmoradDead++;
					}
					// All zones other than the snow war.
					else if (GetZoneID() == ZONE_DELOS
						&& g_pMain->m_byBattleOpen == CLAN_BATTLE)
					{
						pUser->LoyaltyChange(GetID());
					}
					else
					{
						if (isInArena())
						{
							// Show death notices in the arena
							noticeType = DEATH_NOTICE_COORDINATES;
						}
						else
						{
							uint16_t bonusNP = 0;

							if (!GetMap()->isWarZone()
								&& g_pMain->m_byBattleOpen != NATION_BATTLE)
							{
								// Show death notices in PVP zones
								noticeType = DEATH_NOTICE_COORDINATES;
							}

							// Loyalty should be awarded on kill.
							if (!pUser->isInParty())
								pUser->LoyaltyChange(GetID(), bonusNP);
							// In parties, the loyalty should be divided up across the party.
							// Each party member in range should also receive a "Meat Dumpling".
							else
								pUser->LoyaltyDivide(GetID(), bonusNP);

							if (!pUser->GetMap()->isWarZone())
								pUser->GoldChange(GetID(), 0);

							if (GetZoneID() != GetNation() && GetZoneID() <= ELMORAD)
							{
								int64_t nExpLost = m_iMaxExp / 100;

								if (GetPremiumProperty(PremiumExpRestorePercent) > 0)
									nExpLost = nExpLost * (GetPremiumProperty(PremiumExpRestorePercent)) / 100;

								ExpChange(-nExpLost);
							}
						}
					}
				}

				m_sWhoKilledMe = pUser->GetID();
			}

			std::string pKillerPartyUsers;
			std::string pTargetPartyUsers;

			if (GetZoneID() != ZONE_CHAOS_DUNGEON && (pUser->isInParty() || isInParty()))
			{
				CUser *pPartyUser;
				_PARTY_GROUP *pParty = g_pMain->GetPartyPtr(pUser->GetPartyID());
				if (pParty)
				{
					for (int i = 0; i < MAX_PARTY_USERS; i++)
					{
						pPartyUser = g_pMain->GetUserPtr(pParty->uid[i]);
						if (pPartyUser)
							pKillerPartyUsers += string_format("%s,",pPartyUser->GetName().c_str());
					}
				}

				pParty = g_pMain->GetPartyPtr(GetPartyID());
				if (pParty)
				{
					for (int i = 0; i < MAX_PARTY_USERS; i++)
					{
						pPartyUser = g_pMain->GetUserPtr(pParty->uid[i]);
						if (pPartyUser)
							pTargetPartyUsers += string_format("%s,",pPartyUser->GetName().c_str());
					}
				}

				if (!pKillerPartyUsers.empty())
					pKillerPartyUsers = pKillerPartyUsers.substr(0,pKillerPartyUsers.length() - 1);

				if (!pTargetPartyUsers.empty())
					pTargetPartyUsers = pTargetPartyUsers.substr(0,pTargetPartyUsers.length() - 1);
			}

			if (pKillerPartyUsers.empty() && pTargetPartyUsers.empty())
				g_pMain->WriteDeathUserLogFile(string_format("[ USER - %d:%d:%d ] Killer=%s,Target=%s,Zone=%d,X=%d,Z=%d,LoyaltyKiller=%d,LoyaltyMonthlyKiller=%d,LoyaltyTarget=%d,LoyaltyMonthlyTarget=%d\n",time.GetHour(),time.GetMinute(),time.GetSecond(),pKiller->GetName().c_str(),GetName().c_str(),GetZoneID(),uint16_t(GetX()),uint16_t(GetZ()),TO_USER(pKiller)->GetLoyalty(),TO_USER(pKiller)->GetMonthlyLoyalty(),GetLoyalty(),GetMonthlyLoyalty()));
			else if (pKillerPartyUsers.empty() && !pTargetPartyUsers.empty())
				g_pMain->WriteDeathUserLogFile(string_format("[ USER - %d:%d:%d ] Killer=%s,Target=%s,TargetParty=%s,Zone=%d,X=%d,Z=%d,LoyaltyKiller=%d,LoyaltyMonthlyKiller=%d,LoyaltyTarget=%d,LoyaltyMonthlyTarget=%d\n",time.GetHour(),time.GetMinute(),time.GetSecond(),pKiller->GetName().c_str(),GetName().c_str(), pTargetPartyUsers.c_str(),GetZoneID(),uint16_t(GetX()),uint16_t(GetZ()),TO_USER(pKiller)->GetLoyalty(),TO_USER(pKiller)->GetMonthlyLoyalty(),GetLoyalty(),GetMonthlyLoyalty()));
			else if (!pKillerPartyUsers.empty() && pTargetPartyUsers.empty())
				g_pMain->WriteDeathUserLogFile(string_format("[ USER - %d:%d:%d ] Killer=%s,KillerParty=%s,Target=%s,Zone=%d,X=%d,Z=%d,LoyaltyKiller=%d,LoyaltyMonthlyKiller=%d,LoyaltyTarget=%d,LoyaltyMonthlyTarget=%d\n",time.GetHour(),time.GetMinute(),time.GetSecond(),pKiller->GetName().c_str(),pKillerPartyUsers.c_str(),GetName().c_str(),GetZoneID(),uint16_t(GetX()),uint16_t(GetZ()),TO_USER(pKiller)->GetLoyalty(),TO_USER(pKiller)->GetMonthlyLoyalty(),GetLoyalty(),GetMonthlyLoyalty()));
			else if (!pKillerPartyUsers.empty() && !pTargetPartyUsers.empty())
				g_pMain->WriteDeathUserLogFile(string_format("[ USER - %d:%d:%d ] Killer=%s,KillerParty=%s,Target=%s,TargetParty=%s,Zone=%d,X=%d,Z=%d,LoyaltyKiller=%d,LoyaltyMonthlyKiller=%d,LoyaltyTarget=%d,LoyaltyMonthlyTarget=%d\n",time.GetHour(),time.GetMinute(),time.GetSecond(),pKiller->GetName().c_str(),pKillerPartyUsers.c_str(),GetName().c_str(), pTargetPartyUsers.c_str(),GetZoneID(),uint16_t(GetX()),uint16_t(GetZ()),TO_USER(pKiller)->GetLoyalty(),TO_USER(pKiller)->GetMonthlyLoyalty(),GetLoyalty(),GetMonthlyLoyalty()));
		}

		if (noticeType != DEATH_NOTICE_NONE)
			SendDeathNotice(pKiller, noticeType);
	}

	Unit::OnDeath(pKiller);
}

// We have no clan handler, we probably won't need to implement it (but we'll see).
void CUser::SendClanUserStatusUpdate(bool bToRegion /*= true*/)
{
	Packet result(WIZ_KNIGHTS_PROCESS);
	result << uint8_t(KNIGHTS_MODIFY_FAME) << uint8_t(1)
		<< GetSocketID() << GetClanID() << GetFame();

	// TODO: Make this region code user-specific to perform faster.
	if (bToRegion)
		SendToRegion(&result);
	else
		Send(&result);
}

void CUser::SendPartyStatusUpdate(uint8_t bStatus, uint8_t bResult /*= 0*/)
{
	if (!isInParty())
		return;

	Packet result(WIZ_PARTY);
	result << uint8_t(PARTY_STATUSCHANGE) << GetSocketID() << bStatus << bResult;
	g_pMain->Send_PartyMember(GetPartyID(), &result);
}

bool Unit::isInAttackRange(Unit * pTarget, _MAGIC_TABLE * pSkill /*= nullptr*/)
{
	if (pTarget == nullptr)
		return false;

	if (pTarget == this
		|| !isPlayer())
		return true;

	const float fBaseMeleeRange		= 15.0f; // far too generous
	const float fBaseRangedRange	= 65.0f;

	float fRange = fBaseMeleeRange, fWeaponRange = 0.0f;

	_ITEM_DATA * pItem = nullptr;
	_ITEM_TABLE * pTable = TO_USER(this)->GetItemPrototype(RIGHTHAND, pItem);

	if (pTable != nullptr
		&& pItem->sDuration > 0)
	{
		fWeaponRange = pTable->m_sRange;
	}
	else
	{
		pTable = TO_USER(this)->GetItemPrototype(LEFTHAND, pItem);
		if (pTable != nullptr
			&& pItem->sDuration != 0)
			fWeaponRange = pTable->m_sRange;
	}

	if (pSkill != nullptr)
	{
		// Not an attack skill, don't need to enforce these restrictions.
		if (pSkill->bMoral != MORAL_ENEMY && pSkill->bMoral > MORAL_PARTY)
			return true;

		// For physical attack skills (type 1 - melee, type 2 - ranged), we'll need take into account 
		// the weapon's range.
		if (pSkill->bType[0] != 3)
			fRange = fWeaponRange;

		// For physical melee & magic skills, try to use the skill's range if it's set.
		// Need to allow more for lag, and poorly thought out skill ranges.
		// If not, resort to using the weapon range -- or predefined 15m range in the case of type 3 skills.
		if (pSkill->bType[0] != 2)
		{
			return isInRangeSlow(pTarget, fBaseMeleeRange + (pSkill->sRange == 0 ? fRange : pSkill->sRange) + (pSkill->bType[0] == 1 ? fWeaponRange : 0)); 
		}
		// Ranged skills (type 2) don't typically have the main skill range set to anything useful, so
		// we need to allow for the: bow's range, flying skill-specific range, and an extra 50m for the
		// also extremely poorly thought out ranges.
		else
		{
			_MAGIC_TYPE2 * pType2 = g_pMain->m_Magictype2Array.GetData(pSkill->iNum);
			return pType2 != nullptr && isInRangeSlow(pTarget, fRange + pType2->sAddRange + fBaseRangedRange);
		}
	}

	// Regular attack range.
	if (fWeaponRange != 0.0f)
		fRange = fBaseMeleeRange + fWeaponRange;

	return isInRangeSlow(pTarget, fRange);
}

/**
* @brief	Determine if we can use the specified item
* 			via the magic/skill system.
*
* @param	itemid	The ID of the item.
* @param	count 	Stack (can probably disregard, as it's always 1).
*
* @return	true if we can use item, false if not.
*/
bool CUser::CanUseItem(uint32_t nItemID, uint32_t sCount /*= 1*/)
{
	_ITEM_TABLE* pItem = pItem = g_pMain->GetItemPtr(nItemID);
	if (pItem == nullptr)
		return false;

	// Disable scroll usage while transformed.
	if (isTransformed())
	{
		// Various NPC transformations ("Transform Scrolls") are exempt from this rule -- it's just monsters.
		// Also, siege transformations can use their own buff scrolls.
		if (isNPCTransformation() && isSiegeTransformation())
			return false;
	}

	// If the item's class specific, ensure it can be used by this user.
	if ((pItem->m_bClass != 0 && !JobGroupCheck(pItem->m_bClass))
		// Check the item's level requirement
			|| (GetLevel() < pItem->m_bReqLevel || GetLevel() > pItem->m_bReqLevelMax)
			// Ensure the item exists.
			|| !CheckExistItem(nItemID, sCount))
			return false;

	return true;
}

void CUser::SendUserStatusUpdate(UserStatus type, UserStatusBehaviour status)
{
	Packet result(WIZ_ZONEABILITY);
	result << uint8_t(2) << uint8_t(type) << uint8_t(status);
	/*
	1				, 0 = Cure damage over time
	1				, 1 = Damage over time
	2				, 0 = Cure poison
	2				, 1 = poison (purple)
	3				, 0 = Cure disease
	3				, 1 = disease (green)
	4				, 1 = blind
	5				, 0 = Cure grey HP
	5				, 1 = HP is grey (not sure what this is)
	*/
	Send(&result);

	if (isInParty())
		SendPartyStatusUpdate(type, status);
}

/**
* @brief	Gets an item's prototype from a slot in a player's inventory.
*
* @param	pos	The position of the item in the player's inventory.
* @returns	nullptr if an invalid position is specified, or if there's no item at that position.
* 			The item's prototype (_ITEM_TABLE *) otherwise.
*/
_ITEM_TABLE* CUser::GetItemPrototype(uint8_t pos, _ITEM_DATA *& pItem)
{
	if (pos >= INVENTORY_TOTAL)
		return nullptr;

	pItem = GetItem(pos);
	return pItem->nNum == 0 ? nullptr : g_pMain->GetItemPtr(pItem->nNum);
}

/**
* @brief	Checks & removes any expired skills from
* 			the saved magic list.
*/
void CUser::CheckSavedMagic()
{
	Guard lock(m_savedMagicLock);
	if (m_savedMagicMap.empty())
		return;

	std::set<uint32_t> deleteSet;
	foreach (itr, m_savedMagicMap)
	{
		if (itr->second <= UNIXTIME)
			deleteSet.insert(itr->first);
	}
	foreach (itr, deleteSet)
		m_savedMagicMap.erase(*itr);
}

/**
* @brief	Inserts a skill to the saved magic list
* 			to persist across zone changes/logouts.
*
* @param	nSkillID 	Identifier for the skill.
* @param	sDuration	The duration.
*/
void CUser::InsertSavedMagic(uint32_t nSkillID, uint16_t sDuration)
{
	Guard lock(m_savedMagicLock);
	auto itr = m_savedMagicMap.find(nSkillID);

	// If the buff is already in the savedBuffMap there's no need to add it again!
	if (itr != m_savedMagicMap.end())
		return;

	m_savedMagicMap.insert(std::make_pair(nSkillID, UNIXTIME + sDuration));
}

/**
* @brief	Removes the specified skill from the saved magic list.
*
* @param	nSkillID	Identifier for the skill.
*/
void CUser::RemoveSavedMagic(uint32_t nSkillID)
{
	Guard lock(m_savedMagicLock);
	m_savedMagicMap.erase(nSkillID);
}

/**
* @brief	Checks if the given skill ID is already in our 
* 			saved magic list.
*
* @param	nSkillID	Identifier for the skill.
*
* @return	true if the skill exists in the saved magic list, false if not.
*/
bool CUser::HasSavedMagic(uint32_t nSkillID)
{
	Guard lock(m_savedMagicLock);
	return m_savedMagicMap.find(nSkillID) != m_savedMagicMap.end();
}

/**
* @brief	Gets the duration for a saved skill, 
* 			if applicable.
*
* @param	nSkillID	Identifier for the skill.
*
* @return	The saved magic duration.
*/
int16_t CUser::GetSavedMagicDuration(uint32_t nSkillID)
{
	Guard lock(m_savedMagicLock);
	auto itr = m_savedMagicMap.find(nSkillID);
	if (itr == m_savedMagicMap.end())
		return 0;

	return int16_t(itr->second - UNIXTIME);
}

/**
* @brief	Recasts any saved skills on login/zone change.
*/
void CUser::RecastSavedMagic(uint8_t buffType /* = 0*/)
{
	Guard lock(m_savedMagicLock);
	UserSavedMagicMap castSet;
	foreach (itr, m_savedMagicMap)
	{
		if (itr->first != 0 || itr->second != 0) 
			castSet.insert(std::make_pair(itr->first, itr->second));
	}

	if (castSet.empty())
		return;

	foreach (itr, castSet)
	{
		if (buffType > 0)
		{
			_MAGIC_TYPE4 * pType = g_pMain->m_Magictype4Array.GetData(itr->first);

			if (pType == nullptr)
				continue;

			if (pType->bBuffType != buffType)
				continue;
		}

		MagicInstance instance;
		instance.sCasterID = GetID();
		instance.sTargetID = GetID();
		instance.nSkillID = itr->first;
		instance.bIsRecastingSavedMagic = true;

		instance.Run();
	}
}

/**
* @brief	Recasts any lockable scrolls on debuff.
*/
void CUser::RecastLockableScrolls(uint8_t buffType)
{
	InitType4(false, buffType);
	RecastSavedMagic(buffType);
}

uint16_t CUser::GetPlayerRank(uint8_t nRankType)
{
	uint16_t nMyRank = 0;
	uint8_t nRankArrayIndex = (nRankType == RANK_TYPE_PK_ZONE 
		|| nRankType == RANK_TYPE_ZONE_BORDER_DEFENSE_WAR 
		? GetNation() -1
		:  NONE-1);

	std::vector<_USER_RANKING> UserRankingSorted[NONE]; // 0 = Karus, 1 = Human and 2 = Both Nations

	for (int nation = KARUS_ARRAY; nation <= ELMORAD_ARRAY; nation++)
	{
		foreach_stlmap (itr, g_pMain->m_UserRankingArray[nation])
			UserRankingSorted[nRankType == RANK_TYPE_CHAOS_DUNGEON ? NONE -1 : nation].push_back(*itr->second);
	}

	if (nRankArrayIndex < ELMORAD)
	{
		std::sort(UserRankingSorted[nRankArrayIndex].begin(), UserRankingSorted[nRankArrayIndex].end(),
			[] (_USER_RANKING const &a, _USER_RANKING const &b ){ return a.m_iLoyaltyDaily > b.m_iLoyaltyDaily; });
	}
	else if (nRankArrayIndex == ELMORAD)
	{
		std::sort(UserRankingSorted[nRankArrayIndex].begin(), UserRankingSorted[nRankArrayIndex].end(),
			[]( _USER_RANKING const &a, _USER_RANKING const &b ){ return a.m_KillCount > b.m_KillCount; });
	}

	for (int i = 0; i < (int32_t)UserRankingSorted[nRankArrayIndex].size(); i++)
	{
		_USER_RANKING * pRankInfo = &UserRankingSorted[nRankArrayIndex][i];

		if (pRankInfo)
		{
			if (GetZoneID() == pRankInfo->m_bZone)
			{
				nMyRank++;

				if (GetSocketID() == pRankInfo->m_socketID)
					break;
			}
		}
	}

	return nMyRank;
}

void CUser::InitializeStealth()
{
	Packet pkt(WIZ_STEALTH);
	pkt << uint8_t(0) << uint16_t(0);
	Send(&pkt);
}

void CUser::GrantChickenManner()
{
	uint8_t bLevel = GetLevel(), bManner = 0;
	// No manner points if you're not a chicken anymore nor when you're not in a party.
	if (!m_bIsChicken || !isInParty())
		return;

	_PARTY_GROUP *pParty = nullptr;
	pParty = g_pMain->GetPartyPtr(GetPartyID());

	if(pParty == nullptr)
		return;

	for (int i = 0; i < MAX_PARTY_USERS; i++)
	{
		CUser *pTargetUser = nullptr;
		if (pParty->uid[i] != GetSocketID())
			pTargetUser = g_pMain->GetUserPtr(pParty->uid[i]);

		if (pTargetUser == nullptr 
			|| pTargetUser->isDead() 
			|| pTargetUser->m_bIsChicken)
			continue;

		if (!isInRange(pTargetUser, RANGE_50M))
			continue;

		if (pTargetUser->GetLevel() > 20 && pTargetUser->GetLevel() < 40)
			bManner = pTargetUser->GetLevel() / 10;
		else
			bManner = 1;

		pTargetUser->SendMannerChange(bManner);
	}
}

void CUser::SendMannerChange(int32_t iMannerPoints)
{
	//Make sure we don't have too many or too little manner points!
	if(m_iMannerPoint + iMannerPoints > LOYALTY_MAX)
		m_iMannerPoint = LOYALTY_MAX;
	else if (m_iMannerPoint + iMannerPoints < 0)
		m_iMannerPoint = 0;
	else
		m_iMannerPoint += iMannerPoints;

	Packet pkt(WIZ_LOYALTY_CHANGE);
	pkt << uint8_t(LOYALTY_MANNER_POINTS) << m_iMannerPoint;
	Send(&pkt);
}

int32_t CUser::GetEventTrigger() const
{
	CNpc* pNpc = g_pMain->GetNpcPtr(GetTargetID());
	if (pNpc == nullptr)
		return 0;

	return g_pMain->GetEventTrigger(
		pNpc->GetType(),
		pNpc->m_byTrapNumber);
}

void CUser::RemoveStealth()
{
	if (this->m_bInvisibilityType != INVIS_NONE)
	{
		CMagicProcess::RemoveStealth(this, INVIS_DISPEL_ON_MOVE);
		CMagicProcess::RemoveStealth(this, INVIS_DISPEL_ON_ATTACK);
	}
}

void CUser::GivePremium(uint8_t bPremiumType, uint16_t sPremiumTime)
{
	if(GetPremium() > 0 
		|| bPremiumType <= 0 
		|| sPremiumTime <= 0)
		return;

	m_bPremiumType = bPremiumType;
	m_sPremiumTime = sPremiumTime * 24;
	m_bAccountStatus = 1;

	g_DBAgent.SavePremiumServiceUser(this);
	SendPremiumInfo();
}

void CUser::RobChaosSkillItems()
{
	if (GetItemCount(ITEM_LIGHT_PIT) > 0)
		RobItem(ITEM_LIGHT_PIT, GetItemCount(ITEM_LIGHT_PIT));
	if (GetItemCount(ITEM_DRAIN_RESTORE) > 0)
		RobItem(ITEM_DRAIN_RESTORE, GetItemCount(ITEM_DRAIN_RESTORE));
	if (GetItemCount(ITEM_KILLING_BLADE) > 0)
		RobItem(ITEM_KILLING_BLADE, GetItemCount(ITEM_KILLING_BLADE));
}

void CUser::SiegeWarFareNpc(Packet & pkt)
{
		uint8_t opcode , type ;
		uint16_t tarrif;
		pkt >> opcode >> type >> tarrif;
		_KNIGHTS_SIEGE_WARFARE *pKnightSiegeWarFare = g_pMain->GetSiegeMasterKnightsPtr(1);
		CKnights *pKnight = g_pMain->GetClanPtr(pKnightSiegeWarFare->sMasterKnights);
			
		Packet result(WIZ_SIEGE);
		switch (opcode)
		{
		case 3: //moradon npc
			{
				result << opcode << type;
					switch (type)
				{
					case 2:
						result << pKnightSiegeWarFare->sCastleIndex 
						<< uint16_t(pKnightSiegeWarFare->bySiegeType)
						<< pKnightSiegeWarFare->byWarDay
						<< pKnightSiegeWarFare->byWarTime 
						<< pKnightSiegeWarFare->byWarMinute;
						Send(&result);
						break;

					case 4:
						result.SByte();
						result 
						<< pKnightSiegeWarFare->sCastleIndex
						<< uint8_t(1)
						<< pKnight->GetName()
						<< pKnight->m_byNation 
						<< pKnight->m_sMembers
						<< pKnightSiegeWarFare->byWarRequestDay
						<< pKnightSiegeWarFare->byWarRequestTime 
						<< pKnightSiegeWarFare->byWarRequestMinute;
						Send(&result);
						break;

					case 5:
						result.SByte();
					result 
						<< pKnightSiegeWarFare->sCastleIndex 
						<< pKnightSiegeWarFare->bySiegeType
						<< pKnight->GetName()
						<< pKnight->m_byNation 
						<< pKnight->m_sMembers;
						Send(&result);
						break;

					default:
						break;
				}
			}break;

		case 4: //delos npc
			{
				result << opcode << type;
					switch (type)
					{
					case 2:
						pKnightSiegeWarFare->nDungeonCharge += pKnightSiegeWarFare->nDellosTax + pKnightSiegeWarFare->nMoradonTax;
						pKnightSiegeWarFare->nDellosTax = 0;
						pKnightSiegeWarFare->nMoradonTax = 0;
						GoldGain(pKnightSiegeWarFare->nDungeonCharge,true);
						pKnightSiegeWarFare->nDungeonCharge = 0;
						g_pMain->UpdateSiegeTax(0 , 0);
						break;
					case 3:
						result << pKnightSiegeWarFare->sCastleIndex
						<< pKnightSiegeWarFare->sMoradonTariff
						<< pKnightSiegeWarFare->sDellosTariff
						<< (pKnightSiegeWarFare->nDungeonCharge += pKnightSiegeWarFare->nDellosTax + pKnightSiegeWarFare->nMoradonTax);
						pKnightSiegeWarFare->nDellosTax = 0;
						pKnightSiegeWarFare->nMoradonTax = 0;
						Send(&result);
						break;
					case 4:
						pKnightSiegeWarFare->sMoradonTariff = tarrif;
						g_pMain->UpdateSiegeTax(ZONE_MORADON , tarrif);
						result << uint16_t(1) << tarrif << uint8_t(ZONE_MORADON);
						g_pMain->Send_All(&result);
						g_pMain->m_KnightsSiegeWarfareArray.GetData(pKnightSiegeWarFare->sMasterKnights);
						break;
					case 5:
						pKnightSiegeWarFare->sDellosTariff = tarrif;
						g_pMain->UpdateSiegeTax(ZONE_DELOS , tarrif);
						result << uint16_t(1) << tarrif << uint8_t(ZONE_DELOS);
						g_pMain->Send_All(&result);
						g_pMain->m_KnightsSiegeWarfareArray.GetData(pKnightSiegeWarFare->sMasterKnights);
						break;
					default:
						break;
					}
			}break;
		default:
			break;
		}
}

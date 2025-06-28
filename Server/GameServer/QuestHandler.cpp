#include "stdafx.h"
#include "KnightsManager.h"

void CUser::QuestDataRequest(
	Packet& pkt)
{
	uint8_t opcode = pkt.read<uint8_t>();
	if (opcode == QUEST_LIST)
	{
		// NOTE: Oddly, the server divides these up by 22 per page
		// and sends the first 19 of them.
		// We won't respect this behaviour and avoid skipping entries.
		constexpr uint16_t QUESTS_PER_PAGE	= 19;
		constexpr uint16_t MAX_PAGE_NO		= 6;
		constexpr uint16_t MAX_QUEST_ID		= 100;

		uint16_t sPageNo = pkt.read<uint16_t>();

		// Allowed: 0..6 inclusive
		if (sPageNo > MAX_PAGE_NO)
			return;

		uint16_t sStartPos	= sPageNo * QUESTS_PER_PAGE;
		uint16_t sEndPos	= sStartPos + QUESTS_PER_PAGE;

		// NOTE: We don't supply a size here, because it expects 19 quest statuses.
		Packet result(WIZ_QUEST);
		result << uint8_t(QUEST_LIST);

		// To some extent, replicate the official CUser::FilterEvent() logic.
		uint16_t sCurrentPos = 0, sAddedQuests = 0;
		for (auto itr = m_questMap.begin(); itr != m_questMap.end(); ++itr)
		{
			const uint16_t sQuestID = itr->first;
			if (sQuestID == 0
				&& sQuestID > MAX_QUEST_ID)
				continue;

			if (sCurrentPos >= sStartPos
				&& sCurrentPos < sEndPos)
			{
				const uint8_t byQuestState = itr->second;
				result
					<< uint16_t(sQuestID)
					<< uint8_t(byQuestState)
					<< bool(false);				// not an update
				++sAddedQuests;
			}

			++sCurrentPos;
		}

		// Client expects 19 quest statuses.
		// If we haven't supplied 19, we should supply the rest with dummy data.
		for (; sAddedQuests < QUESTS_PER_PAGE; sAddedQuests++)
		{
			result
				<< uint16_t(0)	// quest ID
				<< uint8_t(0)	// quest state
				<< bool(false);	// not an update
		}

		result
			<< uint16_t(sPageNo)		// Requested page number
			<< uint16_t(sCurrentPos);	// Overall (applicable) quest count
		Send(&result);
	}
}

void CUser::SendQuestStateUpdate(
	uint16_t sQuestID,
	uint8_t byQuestState,
	bool bIsUpdate /*= true*/)
{
	Packet result(WIZ_QUEST);
	result
		<< uint8_t(QUEST_UPDATE)
		<< uint16_t(sQuestID)
		<< uint8_t(byQuestState)
		<< bool(bIsUpdate);
	Send(&result);	
}

void CUser::SaveEvent(
	uint16_t sQuestID,
	uint8_t byQuestState)
{
	m_questMap[sQuestID] = byQuestState;

	// Officially we only send quest updates for class mastery quests.
	// This is because it has to update class requirements appropriately.
	switch (sQuestID)
	{
		case QUEST_ID_MASTERY_WARRIOR:
		case QUEST_ID_MASTERY_ROGUE:
		case QUEST_ID_MASTERY_MAGE:
		case QUEST_ID_MASTERY_PRIEST:
			SendQuestStateUpdate(sQuestID, byQuestState);
			break;
	}
}

void CUser::DeleteEvent(uint16_t sQuestID)
{
	m_questMap.erase(sQuestID);
}

bool CUser::CheckExistEvent(uint16_t sQuestID, uint8_t byQuestState)
{
	// Attempt to find a quest with that ID in the map
	auto itr = m_questMap.find(sQuestID);

	// If it doesn't exist, it doesn't exist. 
	// Unless of course, we wanted it to not exist, in which case we're right!
	// (this does seem silly, but this behaviour is STILL expected, so do not remove it.)
	if (itr == m_questMap.end())
		return byQuestState == 0;

	return itr->second == byQuestState;
}

// First job change; you're a [novice], Harry!
bool CUser::PromoteUserNovice()
{
	uint8_t bNewClasses[] = { ClassWarriorNovice, ClassRogueNovice, ClassMageNovice, ClassPriestNovice };
	uint8_t bOldClass = GetClassType() - 1; // convert base class 1,2,3,4 to 0,1,2,3 to align with bNewClasses

	// Make sure it's a beginner class.
	if (!isBeginner())
		return false;

	// Build the new class.
	uint16_t sNewClass = (GetNation() * 100) + bNewClasses[bOldClass];

	Packet result(WIZ_CLASS_CHANGE);
	result << uint8_t(6) << sNewClass << GetID();
	SendToRegion(&result);

	// Change the class & update party.
	result.Initialize(WIZ_CLASS_CHANGE);
	result << uint8_t(2) << sNewClass;
	result.rpos(1);
	ClassChange(result, false); // TODO: Clean this up. Shouldn't need to build a packet for this.

	// Update the clan.
	result.clear();
	result << uint16_t(0);
	CKnightsManager::CurrentKnightsMember(this, result); // TODO: Clean this up too.
	return true;
}

// From novice to master.
bool CUser::PromoteUser()
{
	//re-check if user really eligible
	if (CheckPromotionEligible() != 1)
		return false;

	//additional general controlls
	if (isTrading() || isStoreOpen() || isMerchanting() || isSellingMerchant())
		return false;

	//if user did not start master quest return false
	if (!CheckExistEvent(QUEST_ID_MASTERY_WARRIOR, 1) && 
		!CheckExistEvent(QUEST_ID_MASTERY_ROGUE, 1) && 
		!CheckExistEvent(QUEST_ID_MASTERY_MAGE, 1) &&
		!CheckExistEvent(QUEST_ID_MASTERY_PRIEST, 1))
	{
		return false;
	}

	//item check with respect to class
	//10x opal, 10x sapphire, 10x crystal, [same for all classes]
	if (!CheckExistItem(389076000, 10) || 
		!CheckExistItem(389074000, 10) || 
		!CheckExistItem(389075000, 10)
		)
	{
		return false;
	}

	if (isNoviceWarrior())
	{
		//warrior master: 1x lobo,1x lycaon,1x lupus pendants
		
		if (!CheckExistItem(320410011,1) || !CheckExistItem(320410013,1) ||
			!CheckExistItem(320410012,1))
		{
			return false;
		}

		RobItem(320410011, 1);
		RobItem(320410013, 1);
		RobItem(320410012, 1);

	}
	else if (isNoviceRogue())
	{
		//rogue master: tail of shaula, tail of lesath, 10x blood of glyptodont, 1x fang of bakirra
		if (!CheckExistItem(379040000, 1) || !CheckExistItem(379041000, 1) ||
			!CheckExistItem(379014000, 10) || !CheckExistItem(379042000, 1))
		{
			return false;
		}

		RobItem(379040000, 1);
		RobItem(379041000, 1);
		RobItem(379014000, 10);
		RobItem(379042000, 1);
	}
	else if (isNoviceMage())
	{
		//mage master requires: 1x kekuri ring, 50x gavolt wing, 50x zombie eye, 1x cursed bone
		//1x feather of harpy queen, 10x blood of glyptodont
		if (!CheckExistItem(330310014, 1) || !CheckExistItem(379043000, 50) ||
			!CheckExistItem(379044000, 50) || !CheckExistItem(379045000, 1) ||
			!CheckExistItem(379046000, 1) || !CheckExistItem(379014000, 10))
		{
			return false;
		}

		RobItem(330310014, 1);
		RobItem(379043000, 50);
		RobItem(379044000, 50);
		RobItem(379045000, 1);
		RobItem(379046000, 1);
		RobItem(379014000, 10);

	}
	else if (isNovicePriest())
	{
		//priest master: 1x holy water of temple, 10 million coin
		if (!CheckExistItem(379047000,1) || !hasCoins(10000000))
		{
			return false;
		}
		
		RobItem(379047000, 1);
		GoldLose(10000000);
	}


	//rob opal & crystal & sapphire 10x
	RobItem(389076000, 10);
	RobItem(389074000, 10);
	RobItem(389075000, 10);
	TRACE("test_7\n");
	uint8_t bOldClass = GetClassType();

	// Build the new class.
	uint16_t sNewClass = (GetNation() * 100) + bOldClass + 1;

	Packet result(WIZ_CLASS_CHANGE);
	result << uint8_t(6) << sNewClass << GetID();
	SendToRegion(&result);

	// Change the class & update party.
	result.Initialize(WIZ_CLASS_CHANGE);
	result << uint8_t(2) << sNewClass;
	result.rpos(1);
	ClassChange(result, false); // TODO: Clean this up. Shouldn't need to build a packet for this.

	// use integer division to get from 5/7/9/11 (novice classes) to 1/2/3/4 (base classes)
	uint8_t bBaseClass = (bOldClass / 2) - 1; 

	// this should probably be moved to the script
	SaveEvent(bBaseClass, 2); 

	// Update the clan.
	result.clear();
	result << uint16_t(0);
	CKnightsManager::CurrentKnightsMember(this, result); // TODO: Clean this up too.
	return true;
}

uint8_t CUser::CheckPromotionEligible()
{
	//level and first spec check, text id 9001
	if (GetLevel() < 60 || isBeginner())
		return 2;
	
	//already master, text id 9006
	if (isMastered())
		return 3;

	//success, text id 9000
	return 1;
}

bool CUser::GivePromotionQuest()
{
	if (isBeginner())
		return false;
	
	uint16_t iQuestID = 0;
	if (isNoviceWarrior())
	{
		iQuestID = QUEST_ID_MASTERY_WARRIOR;
	}else if(isNoviceRogue())
	{
		iQuestID = QUEST_ID_MASTERY_ROGUE;
	}
	else if (isNoviceMage())
	{
		iQuestID = QUEST_ID_MASTERY_MAGE;
	}
	else if (isNovicePriest())
	{
		iQuestID = QUEST_ID_MASTERY_PRIEST;
	}

	//dont give quest if user already have it
	if (CheckExistEvent(iQuestID, 1))
	{
		return false;
	}

	//return if quest already finished 
	if (CheckExistEvent(iQuestID, 2))
	{
		return false;
	}

	//give quest
	SaveEvent(iQuestID, 1);

	return true;
}

bool CUser::RunZoneQuestScript(
	int32_t iEventID)
{
	CNpc* pNpc = g_pMain->GetNpcPtr(m_sEventNid);

	// TODO: Add logging for these.
	if (pNpc == nullptr
		|| pNpc->isDead()
		|| GetZoneID() != pNpc->GetZoneID())
		return false;

	Guard lock(g_pMain->m_questNpcLock);

	// Increase the NPC's reference count to ensure it doesn't get freed while executing a script
	pNpc->IncRef();

	// These currently mimic the old EVT behaviour in that each script is designed
	// to serve an entire zone.
	// As such, the filename is the zone ID.
	char szFileName[32] = {};
	snprintf(
		szFileName,
		sizeof(szFileName) - 1,
		"%u.lua",
		GetZoneID());

	bool r = g_pMain->GetLuaEngine()->ExecuteScript(
		this,
		pNpc,
		iEventID,
		szFileName);

	// Decrease it now that we've finished with it + free if necessary
	pNpc->DecRef();

	return r;
}

void CUser::PromoteClan(ClanTypeFlag byFlag)
{
	if (!isInClan())
		return;

	CKnightsManager::UpdateKnightsGrade(GetClanID(), byFlag);
}

uint8_t CUser::GetClanGrade()
{
	if (!isInClan())
		return 0;

	CKnights * pClan = g_pMain->GetClanPtr(GetClanID());
	if (pClan == nullptr)
		return 0;

	return pClan->m_byGrade;
}

uint8_t CUser::GetClanRank()
{
	if (!isInClan())
		return ClanTypeNone;

	CKnights * pClan = g_pMain->GetClanPtr(GetClanID());
	if (pClan == nullptr)
		return ClanTypeNone;

	return pClan->m_byFlag;
}

uint8_t CUser::CheckMiddleStatueCapture()
{
	return g_pMain->m_bMiddleStatueNation == GetNation() ? 1 : 0;
}

void CUser::MoveMiddleStatue()
{
	Warp(
		(GetNation() == KARUS ? DODO_CAMP_WARP_X : LAON_CAMP_WARP_X) + myrand(0, DODO_LAON_WARP_RADIUS),
		(GetNation() == KARUS ? DODO_CAMP_WARP_Z : LAON_CAMP_WARP_Z) + myrand(0, DODO_LAON_WARP_RADIUS));
}

uint8_t CUser::GetPVPMonumentNation()
{
	return g_pMain->m_nPVPMonumentNation[GetZoneID()];
}

int32_t CGameServerDlg::GetInitialEventIDForNpc(
	CNpc* pNpc)
	const
{
	if (pNpc == nullptr)
		return 0;

	int32_t iEventID = 0;
	switch (pNpc->m_tNpcType)
	{
		case NPC_SELITH:
			iEventID = 30001;
			break;

		case NPC_ANVIL:
			iEventID = 8030;
			break;

		case NPC_CLAN_MATCH_ADVISOR:
			iEventID = 31001;
			break;

		case NPC_TELEPORT_GATE:
			iEventID = GetEventTrigger(pNpc->m_tNpcType, pNpc->m_byTrapNumber);
			break;

		case NPC_OPERATOR:
			iEventID = 35201;
			break;

		case NPC_ISAAC:
			iEventID = 35001;
			break;

		case NPC_KAISHAN:
		case NPC_NPC_5:
			iEventID = 21001;
			break;

		case NPC_CAPTAIN:
			iEventID = 15002;
			break;

		case NPC_CLAN:
		case NPC_MONK_ELMORAD:
			iEventID = EVENT_LOGOS_ELMORAD;
			break;

		case NPC_CLERIC:
		case NPC_SIEGE_2:
			iEventID = EVENT_POTION;
			break;

		case NPC_LADY:
		case NPC_PRIEST_IRIS:
			iEventID = 20501;
			break;

		case NPC_ATHIAN:
		case NPC_MANAGER_BARREL:
			iEventID = 22001;
			break;

		case NPC_ARENA:
			iEventID = 15951;
			break;

		case NPC_TRAINER_KATE:
		case NPC_NPC_2:
			iEventID = 20701;
			break;

		case NPC_GENERIC:
		case NPC_NPC_4:
			iEventID = 20901;
			break;

		case NPC_SENTINEL_PATRICK:
		case NPC_NPC_3:
			iEventID = 20801;
			break;

		case NPC_TRADER_KIM:
		case NPC_NPC_1:
			iEventID = 20601;
			break;

		case NPC_MONK_KARUS:
			iEventID = EVENT_LOGOS_KARUS;
			break;

		case NPC_MASTER_WARRIOR:
			iEventID = 11001;
			break;

		case NPC_MASTER_ROGUE:
			iEventID = 12001;
			break;

		case NPC_MASTER_MAGE:
			iEventID = 13001;
			break;

		case NPC_MASTER_PRIEST:
			iEventID = 14001;
			break;

		case NPC_BLACKSMITH:
			iEventID = 7001;
			break;

		case NPC_COUPON:
			iEventID = m_iEventNumber;
			if (iEventID == 0)
				iEventID = EVENT_COUPON;
			break;

		case NPC_HERO_STATUE_1:
		case NPC_KARUS_HERO_STATUE:
			iEventID = 31101;
			break;

		case NPC_HERO_STATUE_2:
			iEventID = 31131;
			break;

		case NPC_HERO_STATUE_3:
			iEventID = 31161;
			break;

		case NPC_ELMORAD_HERO_STATUE:
			iEventID = 31171;
			break;

		case NPC_KEY_QUEST_1:
			iEventID = 15801;
			break;

		case NPC_KEY_QUEST_2:
			iEventID = 15821;
			break;

		case NPC_KEY_QUEST_3:
			iEventID = 15841;
			break;

		case NPC_KEY_QUEST_4:
			iEventID = 15861;
			break;

		case NPC_KEY_QUEST_5:
			iEventID = 15881;
			break;

		case NPC_KEY_QUEST_6:
			iEventID = 15901;
			break;

		case NPC_KEY_QUEST_7:
			iEventID = 15921;
			break;

		case NPC_ROBOS:
			iEventID = 35480;
			break;

		case NPC_SERVER_TRANSFER:
			iEventID = 35541;
			break;

		case NPC_RANKING:
			iEventID = 35560;
			break;

		case NPC_LYONI:
			iEventID = 35553;
			break;

		case NPC_BEGINNER_HELPER_1:
			iEventID = 35563;
			break;

		case NPC_BEGINNER_HELPER_2:
			iEventID = 35594;
			break;

		case NPC_BEGINNER_HELPER_3:
			iEventID = 35615;
			break;

		case NPC_FT_1:
			iEventID = EVENT_FT_1;
			break;

		case NPC_FT_2:
			iEventID = EVENT_FT_2;
			break;

		case NPC_FT_3:
			iEventID = EVENT_FT_3;
			break;

		case NPC_PREMIUM_PC:
			iEventID = 35550;
			break;

		case NPC_KJWAR:
			iEventID = 35624;
			break;

		case NPC_CRAFTSMAN:
			iEventID = 32000;
			break;

		case NPC_COLISEUM_ARTES:
			iEventID = 35640;
			break;

		case NPC_UNK_138:
			iEventID = 35650;
			break;

		case NPC_LOVE_AGENT:
			iEventID = 35662;
			break;

		case NPC_SPY:
			iEventID = 1100;
			break;

		case NPC_ROYAL_GUARD:
			iEventID = 17000;
			break;

		case NPC_ROYAL_CHEF:
			iEventID = 17550;
			break;

		case NPC_ESLANT_WOMAN:
			iEventID = 17590;
			break;

		case NPC_FARMER:
			iEventID = 17600;
			break;

		case NPC_NAMELESS_WARRIOR:
			iEventID = 17630;
			break;

		case NPC_UNK_147:
			iEventID = 17100;
			break;

		case NPC_GATE_GUARD:
			iEventID = 17570;
			break;

		case NPC_ROYAL_ADVISOR:
			iEventID = 17520;
			break;

		case NPC_BIFROST_GATE:
			iEventID = 17681;
			break;

		case NPC_SANGDUF:
			iEventID = 15310;
			break;

		case NPC_UNK_152:
			iEventID = 2901;
			break;

		case NPC_ADELIA:
			iEventID = 35212;
			break;

		case NPC_BIFROST_MONUMENT:
			iEventID = 0;
			break;
	}

	return iEventID;
}

int32_t CGameServerDlg::GetEventTrigger(
	uint8_t byNpcType,
	uint16_t sTrapNum)
	const
{
	// TODO: Optimise this.
	// This is more or less the official implementation (they fetch each by ID - not position - from 0..size).
	// Since it's dual-indexed they found no other alternative than to loop over the entire table...
	// At least the table is small, so it isn't as impactful as it seems.
	for (auto itr = m_EventTriggerArray.begin();
		itr != m_EventTriggerArray.end();
		++itr)
	{
		_EVENT_TRIGGER* pEventTrigger = itr->second;
		if (pEventTrigger != nullptr
			&& pEventTrigger->byNpcType == byNpcType
			&& pEventTrigger->sTrapNum == sTrapNum)
			return pEventTrigger->nTriggerNum;
	}

	return 0;
}

#include "stdafx.h"
#include "LuaEngine.h"
#include "User.h"
#include "Npc.h"

/* 
Classes
*/

/**
* Defining a class is simple: just #define LUA_CLASS to your class name, 
* then call DEFINE_LUA_CLASS(), specifying each method to expose.
*
* Remember to #undef LUA_CLASS when you're done.
*
* Doing so sacrifices a little readability, however it
* makes manually defining methods much quicker (less to type).
*
* Also, as our method doesn't support inheritance, you'll need
* to redefine the same methods for each class.
*
* This is because of the way we dynamically pull instance pointers;
* these are typedef'd within the class, so that we can refer to them 
* to grab the class name from within. As we type check these with Lua,
* they'll fail, so we won't be able to use them.
*
* I don't think this is such a big issue, as there's only a handful of
* cases that actually require this behaviour.
**/

#define LUA_CLASS CUser
DEFINE_LUA_CLASS
	(
	// Getters
	MAKE_LUA_METHOD(GetName)
	MAKE_LUA_METHOD(GetAccountName)
	MAKE_LUA_METHOD(GetZoneID)
	MAKE_LUA_METHOD(GetX)
	MAKE_LUA_METHOD(GetY)
	MAKE_LUA_METHOD(GetZ)
	MAKE_LUA_METHOD(GetNation)
	MAKE_LUA_METHOD(GetLevel)
	MAKE_LUA_METHOD(GetClass)
	MAKE_LUA_METHOD(GetCoins)
	MAKE_LUA_METHOD(GetInnCoins)
	MAKE_LUA_METHOD(GetLoyalty)
	MAKE_LUA_METHOD(GetMonthlyLoyalty)
	MAKE_LUA_METHOD(GetManner)
	MAKE_LUA_METHOD(GetActiveQuestID)
	MAKE_LUA_METHOD(GetClanGrade)
	MAKE_LUA_METHOD(GetClanRank)
	MAKE_LUA_METHOD(GetStat)

	MAKE_LUA_METHOD(isWarrior)
	MAKE_LUA_METHOD(isRogue)
	MAKE_LUA_METHOD(isMage)
	MAKE_LUA_METHOD(isPriest)

	MAKE_LUA_METHOD(isBeginner)
	MAKE_LUA_METHOD(isBeginnerWarrior)
	MAKE_LUA_METHOD(isBeginnerRogue)
	MAKE_LUA_METHOD(isBeginnerMage)
	MAKE_LUA_METHOD(isBeginnerPriest)

	MAKE_LUA_METHOD(isNovice)
	MAKE_LUA_METHOD(isNoviceWarrior)
	MAKE_LUA_METHOD(isNoviceRogue)
	MAKE_LUA_METHOD(isNoviceMage)
	MAKE_LUA_METHOD(isNovicePriest)

	MAKE_LUA_METHOD(isMastered)
	MAKE_LUA_METHOD(isMasteredWarrior)
	MAKE_LUA_METHOD(isMasteredRogue)
	MAKE_LUA_METHOD(isMasteredMage)
	MAKE_LUA_METHOD(isMasteredPriest)

	MAKE_LUA_METHOD(isInClan)
	MAKE_LUA_METHOD(isClanLeader)
	MAKE_LUA_METHOD(isInParty)
	MAKE_LUA_METHOD(isPartyLeader)
	MAKE_LUA_METHOD(isKing)

	// Shortcuts for lazy people
	MAKE_LUA_METHOD(hasCoins)
	MAKE_LUA_METHOD(hasInnCoins)
	MAKE_LUA_METHOD(hasLoyalty)
	MAKE_LUA_METHOD(hasMonthlyLoyalty)
	MAKE_LUA_METHOD(hasManner)

	// Here lie the useful methods.
	MAKE_LUA_METHOD(SendDebugString)
	MAKE_LUA_METHOD(HowMuchItem)
	MAKE_LUA_METHOD(CheckClass)
	MAKE_LUA_METHOD(EmptySlotCount)
	MAKE_LUA_METHOD(StateChange)

	MAKE_LUA_METHOD(GiveItem)
	MAKE_LUA_METHOD(RobItem)
	MAKE_LUA_METHOD(CheckExistItem)
	MAKE_LUA_METHOD(CheckExistEvent)	
	MAKE_LUA_METHOD(GoldGain)
	MAKE_LUA_METHOD(GoldLose)
	MAKE_LUA_METHOD(ExpChange)
	MAKE_LUA_METHOD(GiveLoyalty)
	MAKE_LUA_METHOD(RobLoyalty)
	MAKE_LUA_METHOD(SaveEvent)
	MAKE_LUA_METHOD(SelectMsg) // menu
	MAKE_LUA_METHOD(NpcSay) // dialog
	MAKE_LUA_METHOD(CheckWeight)
	MAKE_LUA_METHOD(CheckSkillPoint)
	MAKE_LUA_METHOD(isRoomForItem) // FindSlotForItem()
	MAKE_LUA_METHOD(CheckExchange)
	MAKE_LUA_METHOD(RunExchange)
	MAKE_LUA_METHOD(RunSelectExchange)
	MAKE_LUA_METHOD(SendNameChange)
	MAKE_LUA_METHOD(SendStatSkillDistribute)
	MAKE_LUA_METHOD(StatPointDistribute)
	MAKE_LUA_METHOD(SkillPointDistribute)
	MAKE_LUA_METHOD(ResetSkillPoints)
	MAKE_LUA_METHOD(ResetStatPoints)
	MAKE_LUA_METHOD(PromoteUserNovice)
	MAKE_LUA_METHOD(PromoteUser)
	MAKE_LUA_METHOD(RobAllItemParty)
	MAKE_LUA_METHOD(ZoneChange)
	MAKE_LUA_METHOD(ZoneChangeParty)
	MAKE_LUA_METHOD(ZoneChangeClan)
	MAKE_LUA_METHOD(ShowEffect)
	MAKE_LUA_METHOD(ShowNpcEffect)
	MAKE_LUA_METHOD(KissUser)
	MAKE_LUA_METHOD(ChangeManner)
	MAKE_LUA_METHOD(PromoteClan)
	MAKE_LUA_METHOD(RequestPersonalRankReward)
	MAKE_LUA_METHOD(RequestReward)
	MAKE_LUA_METHOD(RunCountExchange)
	MAKE_LUA_METHOD(GetMaxExchange)
	MAKE_LUA_METHOD(GetPartyMemberAmount)
	MAKE_LUA_METHOD(GetEventTrigger)
	MAKE_LUA_METHOD(GetPremium)
	MAKE_LUA_METHOD(CheckMiddleStatueCapture)
	MAKE_LUA_METHOD(MoveMiddleStatue)
	MAKE_LUA_METHOD(LevelChange)
	MAKE_LUA_METHOD(GivePremium)
	MAKE_LUA_METHOD(GetMonsterChallengeTime)
	MAKE_LUA_METHOD(GetMonsterChallengeUserCount)
	MAKE_LUA_METHOD(CheckMonsterChallengeTime)
	MAKE_LUA_METHOD(CheckMonsterChallengeUserCount)
	MAKE_LUA_METHOD(GetPVPMonumentNation)
	MAKE_LUA_METHOD(CheckPromotionEligible)
	MAKE_LUA_METHOD(GivePromotionQuest)
	MAKE_LUA_METHOD(PromoteUser)

	);
#undef LUA_CLASS

#define LUA_CLASS CNpc
DEFINE_LUA_CLASS
	(
	// Getters
	MAKE_LUA_METHOD(GetID)
	MAKE_LUA_METHOD(GetProtoID)
	MAKE_LUA_METHOD(GetName)
	MAKE_LUA_METHOD(GetNation)
	MAKE_LUA_METHOD(GetType)
	MAKE_LUA_METHOD(GetZoneID)
	MAKE_LUA_METHOD(GetX)
	MAKE_LUA_METHOD(GetY)
	MAKE_LUA_METHOD(GetZ)
	MAKE_LUA_METHOD(CastSkill)

	// Useful methods
	// MAKE_LUA_METHOD(CycleSpawn) // i.e. ChangePosition(), used to cycle a spawn through the various trap numbers (like 7 key quest NPCs)

	);
#undef LUA_CLASS

/* 
	Global functions
*/
LUA_FUNCTION(CheckPercent)
{
	LUA_RETURN(CheckPercent(LUA_ARG(int, 1)));
}

LUA_FUNCTION(RollDice)
{
	//LUA_RETURN(myrand(0, LUA_ARG(uint16_t, 1)));
	LUA_RETURN(myrand(1, LUA_ARG(uint16_t, 1)));
}

LUA_FUNCTION(CheckBeefRoastVictory)
{
	uint8_t nation = Nation::NONE;
	if (g_pMain->m_sBifrostTime <= 90 * MINUTE && g_pMain->m_BifrostVictory != Nation::ALL)
		nation = g_pMain->m_sBifrostVictoryAll;
	else
		nation = g_pMain->m_BifrostVictory;
	LUA_RETURN(nation);
}

LUA_FUNCTION(CheckWarVictory)
{
	LUA_RETURN(g_pMain->m_bVictory);
}

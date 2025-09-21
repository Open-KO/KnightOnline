#pragma once

#include <N3Base/N3Base.h>
#include <N3Base/N3TableBase.h>

#include "GameDef.h"

class CGameBase : public CN3Base
{
#define ACT_WORLD s_pWorldMgr->GetActiveWorld()

public:
	static CN3TableBase<__TABLE_TEXTS>				s_pTbl_Texts;			// Main string resources
	static CN3TableBase<__TABLE_ZONE>				s_pTbl_Zones;			// Zone data (filenames and settings)
	static CN3TableBase<__TABLE_UI_RESRC>			s_pTbl_UI;				// Maps UI filenames per-nation
	static CN3TableBase<__TABLE_ITEM_BASIC>			s_pTbl_Items_Basic;		// Base item data
	static CN3TableBase<__TABLE_ITEM_EXT>			s_pTbl_Items_Exts[MAX_ITEM_EXTENSION];	// Extended item data for a base item
	static CN3TableBase<__TABLE_PLAYER_LOOKS>		s_pTbl_UPC_Looks;		// Default model information for player characters
	static CN3TableBase<__TABLE_PLAYER_LOOKS>		s_pTbl_NPC_Looks;		// Default model information for NPCs/monsters
	static CN3TableBase<__TABLE_UPC_SKILL>			s_pTbl_Skill;			// Base skill data
	static CN3TableBase<__TABLE_EXCHANGE_QUEST>		s_pTbl_Exchange_Quest;	// Old officially unused data for what were originally 'exchange quests'
	static CN3TableBase<__TABLE_FX>					s_pTbl_FXSource;		// Effect data (filename and settings)
	static CN3TableBase<__TABLE_QUEST_MENU>			s_pTbl_QuestMenu;		// Quest menu button resources
	static CN3TableBase<__TABLE_QUEST_TALK>			s_pTbl_QuestTalk;		// Quest dialogue resources
	static CN3TableBase<__TABLE_QUEST_CONTENT>		s_pTbl_QuestContent;
	static CN3TableBase<__TABLE_HELP>				s_pTbl_Help;

	static class CN3WorldManager*		s_pWorldMgr;						// Manages the current loaded zone
	static class CPlayerOtherMgr*		s_pOPMgr;							// Manages other loaded characters and NPCs
	static class CPlayerMySelf*			s_pPlayer;							// The local player instance
	
protected:
	// Initializes base game resources - table data, the world manager, character management, and more.
	static void StaticMemberInit();

	// Releases base game resources - table data, the world manager, character management, and more.
	static void StaticMemberRelease();

public:
	static bool	GetTextByAttrib(e_ItemAttrib eAttrib, std::string& szAttrib);
	static bool GetTextByClass(e_Class eClass, std::string& szText);
	static bool GetTextByItemClass(e_ItemClass eItemClass, std::string& szText);
	static bool GetTextByKnightsDuty(e_KnightsDuty eDuty, std::string& szText);
	static bool GetTextByNation(e_Nation eNation, std::string& szText);
	static bool GetTextByRace(e_Race eRace, std::string& szText);

	// Returns a colour code based on level difference relative to a player
	static D3DCOLOR				GetIDColorByLevelDifference(int iLevelDiff);

	// Returns the representative class for a given player class.
	static e_Class_Represent	GetRepresentClass(e_Class eClass);

	static e_ItemType MakeResrcFileNameForUPC(	__TABLE_ITEM_BASIC* pItem,
												__TABLE_ITEM_EXT* pItemExt,
												std::string* szResrcFN,
												std::string* szIconFN,
												e_PartPosition& ePartPosition,
												e_PlugPosition& ePlugPosition,
												e_Race eRace = RACE_UNKNOWN);

	class CPlayerBase*	CharacterGetByID(int iID, bool bFromAlive);
	bool				IsValidCharacter(CPlayerBase* pCharacter);
	static std::string	FormatNumber(int iNumber);
	static std::string	UnformatNumber(const std::string& input);
	static void			ConvertPipesToNewlines(std::string& input);

	static bool			IsItemClassWeapon(e_ItemClass itemClass);

	static e_ZoneAbilityType GetCurrentZoneAbilityType();

	CGameBase();
	virtual ~CGameBase();
};

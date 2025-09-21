// GameBase.cpp: implementation of the CGameBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "text_resources.h"
#include "GameBase.h"
#include "N3WorldManager.h"
#include "PlayerOtherMgr.h"
#include "PlayerMySelf.h"

#include <N3Base/N3ShapeMgr.h>

#include <ranges>
#include <algorithm>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

CN3TableBase<__TABLE_TEXTS>				CGameBase::s_pTbl_Texts;
CN3TableBase<__TABLE_ZONE>				CGameBase::s_pTbl_Zones;
CN3TableBase<__TABLE_UI_RESRC>			CGameBase::s_pTbl_UI;
CN3TableBase<__TABLE_ITEM_BASIC>		CGameBase::s_pTbl_Items_Basic;
CN3TableBase<__TABLE_ITEM_EXT>			CGameBase::s_pTbl_Items_Exts[MAX_ITEM_EXTENSION];
CN3TableBase<__TABLE_PLAYER_LOOKS>		CGameBase::s_pTbl_UPC_Looks;
CN3TableBase<__TABLE_PLAYER_LOOKS>		CGameBase::s_pTbl_NPC_Looks;
CN3TableBase<__TABLE_UPC_SKILL>			CGameBase::s_pTbl_Skill;
CN3TableBase<__TABLE_EXCHANGE_QUEST>	CGameBase::s_pTbl_Exchange_Quest;
CN3TableBase<__TABLE_FX>				CGameBase::s_pTbl_FXSource;
CN3TableBase<__TABLE_QUEST_MENU>		CGameBase::s_pTbl_QuestMenu;
CN3TableBase<__TABLE_QUEST_TALK>		CGameBase::s_pTbl_QuestTalk;
CN3TableBase<__TABLE_QUEST_CONTENT>		CGameBase::s_pTbl_QuestContent;
CN3TableBase<__TABLE_HELP>				CGameBase::s_pTbl_Help;

CN3WorldManager*	CGameBase::s_pWorldMgr	= nullptr;	// Manages the current loaded zone
CPlayerOtherMgr*	CGameBase::s_pOPMgr		= nullptr;	// Manages other loaded characters and NPCs
CPlayerMySelf*		CGameBase::s_pPlayer	= nullptr;	// The local player instance
	
CGameBase::CGameBase()
{
}

CGameBase::~CGameBase()
{
}

void CGameBase::StaticMemberInit()
{
	std::string szLangTail = "_us.tbl";
	int iLangID = ::GetUserDefaultLangID();
	if (0x0404 == iLangID)
		szLangTail = "_TW.tbl"; // Taiwan Language

	std::string szFN;
	szFN = "Data\\Texts" + szLangTail;		s_pTbl_Texts.LoadFromFile(szFN);
	szFN = "Data\\Zones.tbl";				s_pTbl_Zones.LoadFromFile(szFN);
	szFN = "Data\\UIs" + szLangTail;		s_pTbl_UI.LoadFromFile(szFN);
	szFN = "Data\\UPC_DefaultLooks.tbl";	s_pTbl_UPC_Looks.LoadFromFile(szFN);
	szFN = "Data\\Item_Org" + szLangTail;	s_pTbl_Items_Basic.LoadFromFile(szFN);

	szFN = "Data\\Quest_Menu" + szLangTail;	s_pTbl_QuestMenu.LoadFromFile(szFN);
	szFN = "Data\\Quest_Talk" + szLangTail;	s_pTbl_QuestTalk.LoadFromFile(szFN);
	szFN = "Data\\Quest_Content" + szLangTail;	s_pTbl_QuestContent.LoadFromFile(szFN);
	szFN = "Data\\Help" + szLangTail;		s_pTbl_Help.LoadFromFile(szFN);

	std::string szFNTmp;
	for (int i = 0; i < MAX_ITEM_EXTENSION; i++)
	{
		szFNTmp = fmt::format("Data\\Item_Ext_{}", i);
		szFN = szFNTmp + szLangTail;
		s_pTbl_Items_Exts[i].LoadFromFile(szFN);
	}

	szFN = "Data\\NPC_Looks.tbl";					s_pTbl_NPC_Looks.LoadFromFile(szFN);
	szFN = "Data\\skill_magic_main" + szLangTail;	s_pTbl_Skill.LoadFromFile(szFN);
	szFN = "Data\\Exchange_Quest.tbl";				s_pTbl_Exchange_Quest.LoadFromFile(szFN);
	szFN = "Data\\fx.tbl";							s_pTbl_FXSource.LoadFromFile(szFN);

	s_pWorldMgr = new CN3WorldManager();
	s_pOPMgr = new CPlayerOtherMgr();
	s_pPlayer = new CPlayerMySelf();
}

void CGameBase::StaticMemberRelease()
{
	delete s_pPlayer;	s_pPlayer = nullptr;
	delete s_pOPMgr;	s_pOPMgr = nullptr;
	delete s_pWorldMgr;	s_pWorldMgr = nullptr;
}

bool CGameBase::GetTextByClass(e_Class eClass, std::string& szText)
{
	switch(eClass)
	{
		case CLASS_KINDOF_WARRIOR:
			szText = fmt::format_text_resource(IDS_CLASS_KINDOF_WARRIOR);
			break;
		case CLASS_KINDOF_ROGUE:
			szText = fmt::format_text_resource(IDS_CLASS_KINDOF_ROGUE);
			break;
		case CLASS_KINDOF_WIZARD:
			szText = fmt::format_text_resource(IDS_CLASS_KINDOF_WIZARD);
			break;
		case CLASS_KINDOF_PRIEST:
			szText = fmt::format_text_resource(IDS_CLASS_KINDOF_PRIEST);
			break;
		case CLASS_KINDOF_ATTACK_WARRIOR:
			szText = fmt::format_text_resource(IDS_CLASS_KINDOF_ATTACK_WARRIOR);
			break;
		case CLASS_KINDOF_DEFEND_WARRIOR:
			szText = fmt::format_text_resource(IDS_CLASS_KINDOF_DEFEND_WARRIOR);
			break;
		case CLASS_KINDOF_ARCHER:
			szText = fmt::format_text_resource(IDS_CLASS_KINDOF_ARCHER);
			break;
		case CLASS_KINDOF_ASSASSIN:
			szText = fmt::format_text_resource(IDS_CLASS_KINDOF_ASSASSIN);
			break;
		case CLASS_KINDOF_ATTACK_WIZARD:
			szText = fmt::format_text_resource(IDS_CLASS_KINDOF_ATTACK_WIZARD);
			break;
		case CLASS_KINDOF_PET_WIZARD:
			szText = fmt::format_text_resource(IDS_CLASS_KINDOF_PET_WIZARD);
			break;
		case CLASS_KINDOF_HEAL_PRIEST:
			szText = fmt::format_text_resource(IDS_CLASS_KINDOF_HEAL_PRIEST);
			break;
		case CLASS_KINDOF_CURSE_PRIEST:
			szText = fmt::format_text_resource(IDS_CLASS_KINDOF_CURSE_PRIEST);
			break;

		case CLASS_EL_WARRIOR:
		case CLASS_KA_WARRIOR:
			szText = fmt::format_text_resource(IDS_CLASS_WARRIOR);
			break;
		case CLASS_EL_ROGUE:
		case CLASS_KA_ROGUE:
			szText = fmt::format_text_resource(IDS_CLASS_ROGUE);
			break;
		case CLASS_EL_WIZARD:
		case CLASS_KA_WIZARD:
			szText = fmt::format_text_resource(IDS_CLASS_WIZARD);
			break;
		case CLASS_EL_PRIEST:
		case CLASS_KA_PRIEST:
			szText = fmt::format_text_resource(IDS_CLASS_PRIEST);
			break;
		
		case CLASS_KA_BERSERKER:
			szText = fmt::format_text_resource(IDS_CLASS_KA_BERSERKER);
			break;
		case CLASS_KA_GUARDIAN:
			szText = fmt::format_text_resource(IDS_CLASS_KA_GUARDIAN);
			break;
		case CLASS_KA_HUNTER:
			szText = fmt::format_text_resource(IDS_CLASS_KA_HUNTER);
			break;
		case CLASS_KA_PENETRATOR:
			szText = fmt::format_text_resource(IDS_CLASS_KA_PENETRATOR);
			break;
		case CLASS_KA_SORCERER:
			szText = fmt::format_text_resource(IDS_CLASS_KA_SORCERER);
			break;
		case CLASS_KA_NECROMANCER:
			szText = fmt::format_text_resource(IDS_CLASS_KA_NECROMANCER);
			break;
		case CLASS_KA_SHAMAN:
			szText = fmt::format_text_resource(IDS_CLASS_KA_SHAMAN);
			break;
		case CLASS_KA_DARKPRIEST:
			szText = fmt::format_text_resource(IDS_CLASS_KA_DARKPRIEST);
			break;
		
		case CLASS_EL_BLADE:
			szText = fmt::format_text_resource(IDS_CLASS_EL_BLADE);
			break;
		case CLASS_EL_PROTECTOR:
			szText = fmt::format_text_resource(IDS_CLASS_EL_PROTECTOR);
			break;
		case CLASS_EL_RANGER:
			szText = fmt::format_text_resource(IDS_CLASS_EL_RANGER);
			break;
		case CLASS_EL_ASSASIN:
			szText = fmt::format_text_resource(IDS_CLASS_EL_ASSASIN);
			break;
		case CLASS_EL_MAGE:
			szText = fmt::format_text_resource(IDS_CLASS_EL_MAGE);
			break;
		case CLASS_EL_ENCHANTER:
			szText = fmt::format_text_resource(IDS_CLASS_EL_ENCHANTER);
			break;
		case CLASS_EL_CLERIC:
			szText = fmt::format_text_resource(IDS_CLASS_EL_CLERIC);
			break;
		case CLASS_EL_DRUID:
			szText = fmt::format_text_resource(IDS_CLASS_EL_DRUID);
			break;
		
		default:
			__ASSERT(0, "Invalid Class");
			szText = "Unknown Class";
			return false;
	}

	return true;
}

bool CGameBase::GetTextByKnightsDuty(e_KnightsDuty eDuty, std::string& szText)
{
	switch(eDuty)
	{
		case KNIGHTS_DUTY_UNKNOWN:		szText = fmt::format_text_resource(IDS_KNIGHTS_DUTY_UNKNOWN); break;
		case KNIGHTS_DUTY_PUNISH:		szText = fmt::format_text_resource(IDS_KNIGHTS_DUTY_PUNISH); break;
		case KNIGHTS_DUTY_TRAINEE:		szText = fmt::format_text_resource(IDS_KNIGHTS_DUTY_TRAINEE); break;
		case KNIGHTS_DUTY_KNIGHT:		szText = fmt::format_text_resource(IDS_KNIGHTS_DUTY_KNIGHT); break;
		case KNIGHTS_DUTY_OFFICER:		szText = fmt::format_text_resource(IDS_KNIGHTS_DUTY_OFFICER); break;
		case KNIGHTS_DUTY_VICECHIEF:	szText = fmt::format_text_resource(IDS_KNIGHTS_DUTY_VICECHIEF); break;
		case KNIGHTS_DUTY_CHIEF:		szText = fmt::format_text_resource(IDS_KNIGHTS_DUTY_CHIEF); break;
		default: __ASSERT(0, "Invalid Knights Duty"); szText = "Unknown Duty"; return false;
	}

	return true;
}

bool CGameBase::GetTextByItemClass(e_ItemClass eItemClass, std::string& szText)
{
	switch(eItemClass)
	{
		case ITEM_CLASS_DAGGER:
			szText = fmt::format_text_resource(IDS_ITEM_CLASS_DAGGER);
			break;
		case ITEM_CLASS_SWORD:
			szText = fmt::format_text_resource(IDS_ITEM_CLASS_SWORD);
			break;
		case ITEM_CLASS_SWORD_2H:
			szText = fmt::format_text_resource(IDS_ITEM_CLASS_SWORD_2H);
			break;
		case ITEM_CLASS_AXE:
			szText = fmt::format_text_resource(IDS_ITEM_CLASS_AXE);
			break;
		case ITEM_CLASS_AXE_2H:
			szText = fmt::format_text_resource(IDS_ITEM_CLASS_AXE_2H);
			break;
		case ITEM_CLASS_MACE:
			szText = fmt::format_text_resource(IDS_ITEM_CLASS_MACE);
			break;
		case ITEM_CLASS_MACE_2H:
			szText = fmt::format_text_resource(IDS_ITEM_CLASS_MACE_2H);
			break;
		case ITEM_CLASS_SPEAR:
			szText = fmt::format_text_resource(IDS_ITEM_CLASS_SPEAR);
			break;
		case ITEM_CLASS_POLEARM:
			szText = fmt::format_text_resource(IDS_ITEM_CLASS_POLEARM);
			break;

		case ITEM_CLASS_SHIELD:
			szText = fmt::format_text_resource(IDS_ITEM_CLASS_SHIELD);
			break;

		case ITEM_CLASS_BOW:
			szText = fmt::format_text_resource(IDS_ITEM_CLASS_BOW);
			break;
		case ITEM_CLASS_BOW_CROSS:
			szText = fmt::format_text_resource(IDS_ITEM_CLASS_BOW_CROSS);
			break;
		case ITEM_CLASS_BOW_LONG:
			szText = fmt::format_text_resource(IDS_ITEM_CLASS_BOW_LONG);
			break;

		case ITEM_CLASS_EARRING:
			szText = fmt::format_text_resource(IDS_ITEM_CLASS_EARRING);
			break;
		case ITEM_CLASS_AMULET:
			szText = fmt::format_text_resource(IDS_ITEM_CLASS_AMULET);
			break;
		case ITEM_CLASS_RING:
			szText = fmt::format_text_resource(IDS_ITEM_CLASS_RING);
			break;
		case ITEM_CLASS_BELT:
			szText = fmt::format_text_resource(IDS_ITEM_CLASS_BELT);
			break;
		case ITEM_CLASS_CHARM:
			szText = fmt::format_text_resource(IDS_ITEM_CLASS_CHARM);
			break;
		case ITEM_CLASS_JEWEL:
			szText = fmt::format_text_resource(IDS_ITEM_CLASS_JEWEL);
			break;
		case ITEM_CLASS_POTION:
			szText = fmt::format_text_resource(IDS_ITEM_CLASS_POTION);
			break;
		case ITEM_CLASS_SCROLL:
			szText = fmt::format_text_resource(IDS_ITEM_CLASS_SCROLL);
			break;

		case ITEM_CLASS_LAUNCHER:
			szText = fmt::format_text_resource(IDS_ITEM_CLASS_LAUNCHER);
			break; 
						
		case ITEM_CLASS_STAFF:
			szText = fmt::format_text_resource(IDS_ITEM_CLASS_STAFF);
			break;
		case ITEM_CLASS_ARROW:
			szText = fmt::format_text_resource(IDS_ITEM_CLASS_ARROW);
			break;
		case ITEM_CLASS_JAVELIN:
			szText = fmt::format_text_resource(IDS_ITEM_CLASS_JAVELIN);
			break;
		
		case ITEM_CLASS_ARMOR_WARRIOR:
			szText = fmt::format_text_resource(IDS_ITEM_CLASS_ARMOR_WARRIOR);
			break;
		case ITEM_CLASS_ARMOR_ROGUE:
			szText = fmt::format_text_resource(IDS_ITEM_CLASS_ARMOR_ROGUE);
			break;
		case ITEM_CLASS_ARMOR_MAGE:
			szText = fmt::format_text_resource(IDS_ITEM_CLASS_ARMOR_MAGE);
			break;
		case ITEM_CLASS_ARMOR_PRIEST:
			szText = fmt::format_text_resource(IDS_ITEM_CLASS_ARMOR_PRIEST); 
			break;
		default:
//			__ASSERT(0, "Invalid Item Class"); szText = "Unknown Item Class";
			return false;
	}

	return true;
}

bool CGameBase::GetTextByAttrib(e_ItemAttrib eAttrib, std::string& szAttrib)
{
	switch(eAttrib)
	{
		case ITEM_ATTRIB_GENERAL:		szAttrib = fmt::format_text_resource(IDS_ITEM_ATTRIB_GENERAL); break;
		case ITEM_ATTRIB_MAGIC:			szAttrib = fmt::format_text_resource(IDS_ITEM_ATTRIB_MAGIC); break;
		case ITEM_ATTRIB_LAIR:			szAttrib = fmt::format_text_resource(IDS_ITEM_ATTRIB_LAIR); break;
		case ITEM_ATTRIB_CRAFT:			szAttrib = fmt::format_text_resource(IDS_ITEM_ATTRIB_CRAFT); break;
		case ITEM_ATTRIB_UNIQUE:		szAttrib = fmt::format_text_resource(IDS_ITEM_ATTRIB_UNIQUE); break;
		case ITEM_ATTRIB_UPGRADE:		szAttrib = fmt::format_text_resource(IDS_ITEM_ATTRIB_UPGRADE); break;
		default:
			return false;
	}

	return true;
}

e_Class_Represent CGameBase::GetRepresentClass(e_Class eClass)
{
	switch(eClass)
	{
		case CLASS_KA_WARRIOR:
		case CLASS_KA_BERSERKER:
		case CLASS_KA_GUARDIAN:
		case CLASS_EL_WARRIOR:
		case CLASS_EL_BLADE:
		case CLASS_EL_PROTECTOR:
			return CLASS_REPRESENT_WARRIOR;

		case CLASS_KA_ROGUE:
		case CLASS_KA_HUNTER:
		case CLASS_KA_PENETRATOR:
		case CLASS_EL_ROGUE:
		case CLASS_EL_RANGER:
		case CLASS_EL_ASSASIN:
			return CLASS_REPRESENT_ROGUE;

		case CLASS_KA_WIZARD:
		case CLASS_KA_SORCERER:
		case CLASS_KA_NECROMANCER:
		case CLASS_EL_WIZARD:
		case CLASS_EL_MAGE:
		case CLASS_EL_ENCHANTER:
			return CLASS_REPRESENT_WIZARD;

		case CLASS_KA_PRIEST:
		case CLASS_KA_SHAMAN:
		case CLASS_KA_DARKPRIEST:
		case CLASS_EL_PRIEST:
		case CLASS_EL_CLERIC:
		case CLASS_EL_DRUID:
			return CLASS_REPRESENT_PRIEST;
	}

	return CLASS_REPRESENT_UNKNOWN;
}

bool CGameBase::GetTextByNation(e_Nation eNation, std::string& szText)
{
	switch(eNation)
	{
		case NATION_ELMORAD:	szText = fmt::format_text_resource(IDS_NATION_ELMORAD); break;
		case NATION_KARUS:		szText = fmt::format_text_resource(IDS_NATION_KARUS); break;
		default: szText = fmt::format_text_resource(IDS_NATION_UNKNOWN); return false;
	}

	return true;
}

bool CGameBase::GetTextByRace(e_Race eRace, std::string& szText)
{
	switch(eRace)
	{
		case RACE_EL_BABARIAN:
			szText = fmt::format_text_resource(IDS_RACE_EL_BABARIAN);
			break;
		case RACE_EL_MAN:
			szText = fmt::format_text_resource(IDS_RACE_EL_MAN);
			break;
		case RACE_EL_WOMEN:
			szText = fmt::format_text_resource(IDS_RACE_EL_WOMEN);
			break;

		case RACE_KA_ARKTUAREK:
			szText = fmt::format_text_resource(IDS_RACE_KA_ARKTUAREK);
			break;
		case RACE_KA_TUAREK:
			szText = fmt::format_text_resource(IDS_RACE_KA_TUAREK);
			break;
		case RACE_KA_WRINKLETUAREK:
			szText = fmt::format_text_resource(IDS_RACE_KA_WRINKLETUAREK);
			break;
		case RACE_KA_PURITUAREK:
			szText = fmt::format_text_resource(IDS_RACE_KA_PURITUAREK);
			break;
			
		default:
			szText = fmt::format_text_resource(IDS_NATION_UNKNOWN); 
			return false;
	}

	return true;
}

D3DCOLOR CGameBase::GetIDColorByLevelDifference(int iLevelDiff)
{
	// Returns a colour code based on level difference relative to a player:
	// Purple   = 8+ levels above
	// Red      = 5 to 7 levels above
	// Yellow   = 2 to 4 levels above
	// White    = within 1 level (+/-)
	// Blue     = 2 to 4 levels below
	// Green    = 5 to 7 levels below
	// Sky blue = 8+ levels below (no EXP gained)
	
	D3DCOLOR crID = 0xffffffff;
	if(iLevelDiff >= 8)			crID = D3DCOLOR_ARGB(255, 255, 0, 255);
	else if(iLevelDiff >= 5)	crID = D3DCOLOR_ARGB(255, 255, 0, 0);
	else if(iLevelDiff >= 2)	crID = D3DCOLOR_ARGB(255, 255, 255, 0);
	else if(iLevelDiff >= -1)	crID = D3DCOLOR_ARGB(255, 255, 255, 255);
	else if(iLevelDiff >= -4)	crID = D3DCOLOR_ARGB(255, 0, 0, 255);
	else if(iLevelDiff >= -7)	crID = D3DCOLOR_ARGB(255, 0, 255, 0);
	else crID = D3DCOLOR_ARGB(255, 0, 255, 255);

	return crID;
}

// Generate requested resource filenames using the given item data
e_ItemType CGameBase::MakeResrcFileNameForUPC(	__TABLE_ITEM_BASIC* pItem,
												__TABLE_ITEM_EXT* pItemExt,
												std::string* pszResrcFN,
												std::string* pszIconFN,
												e_PartPosition& ePartPosition,
												e_PlugPosition& ePlugPosition,
												e_Race eRace /*= RACE_UNKNOWN*/)
{	
	ePartPosition = PART_POS_UNKNOWN;
	ePlugPosition = PLUG_POS_UNKNOWN;
	if(pszResrcFN) *pszResrcFN = "";
	if(pszIconFN) *pszIconFN = "";

	if(nullptr == pItem) return ITEM_TYPE_UNKNOWN;
	
	e_ItemType eType	= ITEM_TYPE_UNKNOWN;
	e_ItemPosition ePos	= (e_ItemPosition)pItem->byAttachPoint;

	int iPos = 0;
	std::string szExt; // File extension
	
	if(ePos >= ITEM_POS_DUAL && ePos <= ITEM_POS_TWOHANDLEFT)
	{
		if(ITEM_POS_DUAL == ePos || ITEM_POS_RIGHTHAND == ePos || ITEM_POS_TWOHANDRIGHT == ePos) ePlugPosition = PLUG_POS_RIGHTHAND;
		else if(ITEM_POS_LEFTHAND == ePos || ITEM_POS_TWOHANDLEFT == ePos) ePlugPosition = PLUG_POS_LEFTHAND;

		eType = ITEM_TYPE_PLUG;
		szExt = ".n3cplug";
	}
	else if(ePos >= ITEM_POS_UPPER && ePos <= ITEM_POS_SHOES)
	{
		if(ITEM_POS_UPPER == ePos)			ePartPosition = PART_POS_UPPER;
		else if(ITEM_POS_LOWER == ePos)		ePartPosition = PART_POS_LOWER;
		else if(ITEM_POS_HEAD == ePos)		ePartPosition = PART_POS_HAIR_HELMET;
		else if(ITEM_POS_GLOVES == ePos)	ePartPosition = PART_POS_HANDS;
		else if(ITEM_POS_SHOES == ePos)		ePartPosition = PART_POS_FEET;
		else { __ASSERT(0, "lll"); }
		
		eType = ITEM_TYPE_PART;
		szExt = ".n3cpart";
		iPos = ePartPosition + 1;
	}
	else if(ePos >= ITEM_POS_EAR && ePos <= ITEM_POS_INVENTORY)
	{
		eType = ITEM_TYPE_ICONONLY;
		szExt = ".dxt";
	}
	else if(ePos == ITEM_POS_GOLD)
	{
		eType = ITEM_TYPE_GOLD;
		szExt = ".dxt";
	}
	else if(ePos == ITEM_POS_SONGPYUN)
	{
		eType = ITEM_TYPE_SONGPYUN;
		szExt = ".dxt";
	}
	else
	{
		__ASSERT(0, "Invalid Item Position");
	}

	// replace icon/resource IDs if they're overridden by the item
	int iIDResrc = 0, iIDIcon = 0;
	
	if (pItemExt != nullptr && pItemExt->dwIDResrc != 0)
		iIDResrc = pItemExt->dwIDResrc;
	else
		iIDResrc = pItem->dwIDResrc;

	if (pItemExt != nullptr && pItemExt->dwIDIcon != 0)
		iIDIcon = pItemExt->dwIDIcon;
	else
		iIDIcon = pItem->dwIDIcon;

	if (pszResrcFN)
	{
		if (pItem->dwIDResrc)
		{
			// NOTE: no idea but perhaps this will work for now
			if (eRace != RACE_UNKNOWN && ePos >= /*ITEM_POS_DUAL*/ITEM_POS_UPPER && ePos <= ITEM_POS_SHOES)
			{
				*pszResrcFN = fmt::format("Item\\{:01}_{:04}_{:02}_{:01}{}",
					(iIDResrc / 10000000),
					((iIDResrc / 1000) % 10000) + eRace,
					(iIDResrc / 10) % 100,
					iIDResrc % 10,
					szExt);
			}
			else
			{
				*pszResrcFN = fmt::format("Item\\{:01}_{:04}_{:02}_{:01}{}",
					(iIDResrc / 10000000),
					(iIDResrc / 1000) % 10000,
					(iIDResrc / 10) % 100,
					iIDResrc % 10,
					szExt);
			}
		}
		// Some items don't have models -- only icons.
		else
		{
			pszResrcFN->clear();
		}
	}

	if (pszIconFN)
	{
		*pszIconFN = fmt::format("UI\\ItemIcon_{:01}_{:04}_{:02}_{:01}.dxt",
			(iIDIcon / 10000000),
			(iIDIcon / 1000) % 10000,
			(iIDIcon / 10) % 100,
			iIDIcon % 10);
	}
	
	return eType;
}

bool CGameBase::IsValidCharacter(CPlayerBase* pCharacter)
{
	if (pCharacter == nullptr)
		return false;

	// Requested character is the lcoal player.
	if (pCharacter == s_pPlayer)
		return true;

	// Verify that the player exists.
	// NOTE: The original comment claimed to check if they're alive,
	// but it does no such thing.
	return s_pOPMgr->IsValidCharacter(pCharacter);
}

CPlayerBase* CGameBase::CharacterGetByID(int iID, bool bFromAlive)
{
	if(iID < 0) return nullptr;
	if(iID == s_pPlayer->IDNumber()) return s_pPlayer;
	return s_pOPMgr->CharacterGetByID(iID, bFromAlive);
}

std::string CGameBase::FormatNumber(int iNumber)
{
	// Original unformatted number in string form
	const std::string szOrigNum = std::to_string(iNumber);

	// Where the digits actually start - if it has a sign, this will be at 1.
	// Otherwise, it will start at 0.
	size_t nDigitStart = (iNumber < 0 ? 1 : 0);

	// Full number of digits (excluding the sign).
	size_t nDigitCount = szOrigNum.size() - nDigitStart;

	// Number of commas that will be generated.
	size_t nCommaCount = (nDigitCount - 1) / 3;

	// Number of leading digits. 
	size_t nLeadingDigits = nDigitCount % 3;
	if (nLeadingDigits == 0)
		nLeadingDigits = 3;

	// Pre-reserve the buffer for us to append to.
	std::string szFormattedNum;
	szFormattedNum.reserve(szOrigNum.size() + nCommaCount);

	// Append sign (if applicable) and variable number of leading digits.
	size_t nStartPos = nDigitStart + nLeadingDigits;
	szFormattedNum.append(szOrigNum, 0, nStartPos);

	// The remaining groups of 3 are guaranteed, so we can append them in their full 3s.
	for (size_t i = nStartPos; i < szOrigNum.size(); i += 3)
	{
		szFormattedNum += ',';
		szFormattedNum.append(szOrigNum, i, 3);
	}

	return szFormattedNum;
}

std::string CGameBase::UnformatNumber(const std::string& input)
{
	std::string result = input;
	result.erase(std::remove(result.begin(), result.end(), ','), result.end());
	return result;
}

void CGameBase::ConvertPipesToNewlines(std::string& input)
{
	std::ranges::replace(input, '|', '\n');
}

bool CGameBase::IsItemClassWeapon(e_ItemClass itemClass)
{
	switch (itemClass)
	{
		case ITEM_CLASS_DAGGER:
		case ITEM_CLASS_SWORD:
		case ITEM_CLASS_SWORD_2H:
		case ITEM_CLASS_AXE:
		case ITEM_CLASS_AXE_2H:
		case ITEM_CLASS_MACE:
		case ITEM_CLASS_MACE_2H:
		case ITEM_CLASS_SPEAR:
		case ITEM_CLASS_POLEARM:
		case ITEM_CLASS_BOW:
		case ITEM_CLASS_BOW_CROSS:
		case ITEM_CLASS_BOW_LONG:
		case ITEM_CLASS_STAFF:
		case ITEM_CLASS_JAVELIN:
			return true;
	}

	return false;
}

e_ZoneAbilityType CGameBase::GetCurrentZoneAbilityType()
{
	return s_pPlayer->m_InfoExt.eZoneAbilityType;
}

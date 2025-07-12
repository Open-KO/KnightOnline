// GameBase.cpp: implementation of the CGameBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "GameBase.h"
#include "N3WorldManager.h"
#include "PlayerOtherMgr.h"
#include "PlayerMySelf.h"

#include <N3Base/N3ShapeMgr.h>

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
	if(0x0404 == iLangID) szLangTail = "_TW.tbl"; // Taiwan Language

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

	for(int i = 0; i < MAX_ITEM_EXTENSION; i++)
	{
		char szFNTmp[256] = "";
		sprintf(szFNTmp, "Data\\Item_Ext_%d", i);
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

bool CGameBase::GetText(uint32_t dwResourceID, std::string* szText)
{
	__TABLE_TEXTS* pText = s_pTbl_Texts.Find(dwResourceID);
	if (pText == nullptr)
	{
		szText->clear();
		return false;
	}

	*szText = pText->szText;
	return true;
}

bool CGameBase::GetTextF(uint32_t nResourceID, std::string* szText, ...)
{
	if (!GetText(nResourceID, szText))
	{
		szText->clear();
		return false;
	}

	char buffer[1024] = {};
	va_list args;
	va_start(args, szText);
	vsnprintf(buffer, sizeof(buffer), szText->c_str(), args);
	*szText = buffer;
	va_end(args);

	return true;
}

bool CGameBase::GetTextByClass(e_Class eClass, std::string& szText)
{
	switch(eClass)
	{
		case CLASS_KINDOF_WARRIOR:
			GetText(IDS_CLASS_KINDOF_WARRIOR, &szText);
			break;
		case CLASS_KINDOF_ROGUE:
			GetText(IDS_CLASS_KINDOF_ROGUE, &szText);
			break;
		case CLASS_KINDOF_WIZARD:
			GetText(IDS_CLASS_KINDOF_WIZARD, &szText);
			break;
		case CLASS_KINDOF_PRIEST:
			GetText(IDS_CLASS_KINDOF_PRIEST, &szText);
			break;
		case CLASS_KINDOF_ATTACK_WARRIOR:
			GetText(IDS_CLASS_KINDOF_ATTACK_WARRIOR, &szText);
			break;
		case CLASS_KINDOF_DEFEND_WARRIOR:
			GetText(IDS_CLASS_KINDOF_DEFEND_WARRIOR, &szText);
			break;
		case CLASS_KINDOF_ARCHER:
			GetText(IDS_CLASS_KINDOF_ARCHER, &szText);
			break;
		case CLASS_KINDOF_ASSASSIN:
			GetText(IDS_CLASS_KINDOF_ASSASSIN, &szText);
			break;
		case CLASS_KINDOF_ATTACK_WIZARD:
			GetText(IDS_CLASS_KINDOF_ATTACK_WIZARD, &szText);
			break;
		case CLASS_KINDOF_PET_WIZARD:
			GetText(IDS_CLASS_KINDOF_PET_WIZARD, &szText);
			break;
		case CLASS_KINDOF_HEAL_PRIEST:
			GetText(IDS_CLASS_KINDOF_HEAL_PRIEST, &szText);
			break;
		case CLASS_KINDOF_CURSE_PRIEST:
			GetText(IDS_CLASS_KINDOF_CURSE_PRIEST, &szText);
			break;

		case CLASS_EL_WARRIOR:
		case CLASS_KA_WARRIOR:
			GetText(IDS_CLASS_WARRIOR, &szText);
			break;
		case CLASS_EL_ROGUE:
		case CLASS_KA_ROGUE:
			GetText(IDS_CLASS_ROGUE, &szText);
			break;
		case CLASS_EL_WIZARD:
		case CLASS_KA_WIZARD:
			GetText(IDS_CLASS_WIZARD, &szText);
			break;
		case CLASS_EL_PRIEST:
		case CLASS_KA_PRIEST:
			GetText(IDS_CLASS_PRIEST, &szText);
			break;
		
		case CLASS_KA_BERSERKER:
			GetText(IDS_CLASS_KA_BERSERKER, &szText);
			break;
		case CLASS_KA_GUARDIAN:
			GetText(IDS_CLASS_KA_GUARDIAN, &szText);
			break;
		case CLASS_KA_HUNTER:
			GetText(IDS_CLASS_KA_HUNTER, &szText);
			break;
		case CLASS_KA_PENETRATOR:
			GetText(IDS_CLASS_KA_PENETRATOR, &szText);
			break;
		case CLASS_KA_SORCERER:
			GetText(IDS_CLASS_KA_SORCERER, &szText);
			break;
		case CLASS_KA_NECROMANCER:
			GetText(IDS_CLASS_KA_NECROMANCER, &szText);
			break;
		case CLASS_KA_SHAMAN:
			GetText(IDS_CLASS_KA_SHAMAN, &szText);
			break;
		case CLASS_KA_DARKPRIEST:
			GetText(IDS_CLASS_KA_DARKPRIEST, &szText);
			break;
		
		case CLASS_EL_BLADE:
			GetText(IDS_CLASS_EL_BLADE, &szText);
			break;
		case CLASS_EL_PROTECTOR:
			GetText(IDS_CLASS_EL_PROTECTOR, &szText);
			break;
		case CLASS_EL_RANGER:
			GetText(IDS_CLASS_EL_RANGER, &szText);
			break;
		case CLASS_EL_ASSASIN:
			GetText(IDS_CLASS_EL_ASSASIN, &szText);
			break;
		case CLASS_EL_MAGE:
			GetText(IDS_CLASS_EL_MAGE, &szText);
			break;
		case CLASS_EL_ENCHANTER:
			GetText(IDS_CLASS_EL_ENCHANTER, &szText);
			break;
		case CLASS_EL_CLERIC:
			GetText(IDS_CLASS_EL_CLERIC, &szText);
			break;
		case CLASS_EL_DRUID:
			GetText(IDS_CLASS_EL_DRUID, &szText);
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
		case KNIGHTS_DUTY_UNKNOWN:		GetText(IDS_KNIGHTS_DUTY_UNKNOWN, &szText); break;
		case KNIGHTS_DUTY_PUNISH:		GetText(IDS_KNIGHTS_DUTY_PUNISH, &szText); break;
		case KNIGHTS_DUTY_TRAINEE:		GetText(IDS_KNIGHTS_DUTY_TRAINEE, &szText); break;
		case KNIGHTS_DUTY_KNIGHT:		GetText(IDS_KNIGHTS_DUTY_KNIGHT, &szText); break;
		case KNIGHTS_DUTY_OFFICER:		GetText(IDS_KNIGHTS_DUTY_OFFICER, &szText); break;
		case KNIGHTS_DUTY_VICECHIEF:	GetText(IDS_KNIGHTS_DUTY_VICECHIEF, &szText); break;
		case KNIGHTS_DUTY_CHIEF:		GetText(IDS_KNIGHTS_DUTY_CHIEF, &szText); break;
		default: __ASSERT(0, "Invalid Knights Duty"); szText = "Unknown Duty"; return false;
	}

	return true;
}

bool CGameBase::GetTextByItemClass(e_ItemClass eItemClass, std::string& szText)
{
	switch(eItemClass)
	{
		case ITEM_CLASS_DAGGER:
			GetText(IDS_ITEM_CLASS_DAGGER, &szText);
			break;
		case ITEM_CLASS_SWORD:
			GetText(IDS_ITEM_CLASS_SWORD, &szText);
			break;
		case ITEM_CLASS_SWORD_2H:
			GetText(IDS_ITEM_CLASS_SWORD_2H, &szText);
			break;
		case ITEM_CLASS_AXE:
			GetText(IDS_ITEM_CLASS_AXE, &szText);
			break;
		case ITEM_CLASS_AXE_2H:
			GetText(IDS_ITEM_CLASS_AXE_2H, &szText);
			break;
		case ITEM_CLASS_MACE:
			GetText(IDS_ITEM_CLASS_MACE, &szText);
			break;
		case ITEM_CLASS_MACE_2H:
			GetText(IDS_ITEM_CLASS_MACE_2H, &szText);
			break;
		case ITEM_CLASS_SPEAR:
			GetText(IDS_ITEM_CLASS_SPEAR, &szText);
			break;
		case ITEM_CLASS_POLEARM:
			GetText(IDS_ITEM_CLASS_POLEARM, &szText);
			break;

		case ITEM_CLASS_SHIELD:
			GetText(IDS_ITEM_CLASS_SHIELD, &szText);
			break;

		case ITEM_CLASS_BOW:
			GetText(IDS_ITEM_CLASS_BOW, &szText);
			break;
		case ITEM_CLASS_BOW_CROSS:
			GetText(IDS_ITEM_CLASS_BOW_CROSS, &szText);
			break;
		case ITEM_CLASS_BOW_LONG:
			GetText(IDS_ITEM_CLASS_BOW_LONG, &szText);
			break;

		case ITEM_CLASS_EARRING:
			GetText(IDS_ITEM_CLASS_EARRING, &szText);
			break;
		case ITEM_CLASS_AMULET:
			GetText(IDS_ITEM_CLASS_AMULET, &szText);
			break;
		case ITEM_CLASS_RING:
			GetText(IDS_ITEM_CLASS_RING, &szText);
			break;
		case ITEM_CLASS_BELT:
			GetText(IDS_ITEM_CLASS_BELT, &szText);
			break;
		case ITEM_CLASS_CHARM:
			GetText(IDS_ITEM_CLASS_CHARM, &szText);
			break;
		case ITEM_CLASS_JEWEL:
			GetText(IDS_ITEM_CLASS_JEWEL, &szText);
			break;
		case ITEM_CLASS_POTION:
			GetText(IDS_ITEM_CLASS_POTION, &szText);
			break;
		case ITEM_CLASS_SCROLL:
			GetText(IDS_ITEM_CLASS_SCROLL, &szText);
			break;

		case ITEM_CLASS_LAUNCHER:
			GetText(IDS_ITEM_CLASS_LAUNCHER, &szText);
			break; 
						
		case ITEM_CLASS_STAFF:
			GetText(IDS_ITEM_CLASS_STAFF, &szText);
			break;
		case ITEM_CLASS_ARROW:
			GetText(IDS_ITEM_CLASS_ARROW, &szText);
			break;
		case ITEM_CLASS_JAVELIN:
			GetText(IDS_ITEM_CLASS_JAVELIN, &szText);
			break;
		
		case ITEM_CLASS_ARMOR_WARRIOR:
			GetText(IDS_ITEM_CLASS_ARMOR_WARRIOR, &szText);
			break;
		case ITEM_CLASS_ARMOR_ROGUE:
			GetText(IDS_ITEM_CLASS_ARMOR_ROGUE, &szText);
			break;
		case ITEM_CLASS_ARMOR_MAGE:
			GetText(IDS_ITEM_CLASS_ARMOR_MAGE, &szText);
			break;
		case ITEM_CLASS_ARMOR_PRIEST:
			GetText(IDS_ITEM_CLASS_ARMOR_PRIEST, &szText); 
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
		case ITEM_ATTRIB_GENERAL:		GetText(IDS_ITEM_ATTRIB_GENERAL, &szAttrib); break;
		case ITEM_ATTRIB_MAGIC:			GetText(IDS_ITEM_ATTRIB_MAGIC, &szAttrib); break;
		case ITEM_ATTRIB_LAIR:			GetText(IDS_ITEM_ATTRIB_LAIR, &szAttrib); break;
		case ITEM_ATTRIB_CRAFT:			GetText(IDS_ITEM_ATTRIB_CRAFT, &szAttrib); break;
		case ITEM_ATTRIB_UNIQUE:		GetText(IDS_ITEM_ATTRIB_UNIQUE, &szAttrib); break;
		case ITEM_ATTRIB_UPGRADE:		GetText(IDS_ITEM_ATTRIB_UPGRADE, &szAttrib); break;
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
		case NATION_ELMORAD:	GetText(IDS_NATION_ELMORAD, &szText); break;
		case NATION_KARUS:		GetText(IDS_NATION_KARUS, &szText); break;
		default: GetText(IDS_NATION_UNKNOWN, &szText); return false;
	}

	return true;
}

bool CGameBase::GetTextByRace(e_Race eRace, std::string& szText)
{
	switch(eRace)
	{
		case RACE_EL_BABARIAN:
			GetText(IDS_RACE_EL_BABARIAN, &szText);
			break;
		case RACE_EL_MAN:
			GetText(IDS_RACE_EL_MAN, &szText);
			break;
		case RACE_EL_WOMEN:
			GetText(IDS_RACE_EL_WOMEN, &szText);
			break;

		case RACE_KA_ARKTUAREK:
			GetText(IDS_RACE_KA_ARKTUAREK, &szText);
			break;
		case RACE_KA_TUAREK:
			GetText(IDS_RACE_KA_TUAREK, &szText);
			break;
		case RACE_KA_WRINKLETUAREK:
			GetText(IDS_RACE_KA_WRINKLETUAREK, &szText);
			break;
		case RACE_KA_PURITUAREK:
			GetText(IDS_RACE_KA_PURITUAREK, &szText);
			break;
			
		default:
			GetText(IDS_NATION_UNKNOWN, &szText); 
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

	if(NULL == pItem) return ITEM_TYPE_UNKNOWN;
	
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

	char buffer[256] = {};
	if(pszResrcFN)
	{
		if(pItem->dwIDResrc) 
		{
			if(eRace != RACE_UNKNOWN && ePos >= /*ITEM_POS_DUAL*/ITEM_POS_UPPER && ePos <= ITEM_POS_SHOES) {
				// NOTE: no idea but perhaps this will work for now
				sprintf(buffer, "Item\\%.1d_%.4d_%.2d_%.1d%s",
					(pItem->dwIDResrc / 10000000),
					((pItem->dwIDResrc / 1000) % 10000) + eRace,
					(pItem->dwIDResrc / 10) % 100,
					pItem->dwIDResrc % 10,
					szExt.c_str());
			} else {
				sprintf(buffer, "Item\\%.1d_%.4d_%.2d_%.1d%s",
					(pItem->dwIDResrc / 10000000),
					(pItem->dwIDResrc / 1000) % 10000,
					(pItem->dwIDResrc / 10) % 100,
					pItem->dwIDResrc % 10,
					szExt.c_str());
			}

			*pszResrcFN = buffer;
		}
		// Some items don't have models -- only icons.
		else
		{
			pszResrcFN->clear();
		}
	}
	if(pszIconFN)
	{
		sprintf(buffer,	"UI\\ItemIcon_%.1d_%.4d_%.2d_%.1d.dxt",
			(pItem->dwIDIcon / 10000000), 
			(pItem->dwIDIcon / 1000) % 10000, 
			(pItem->dwIDIcon / 10) % 100, 
			pItem->dwIDIcon % 10);
		*pszIconFN = &buffer[0];
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
	if(iID < 0) return NULL;
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

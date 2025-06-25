// MagicSkillMng.cpp: implementation of the CMagicSkillMng class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "GameProcMain.h"
#include "APISocket.h"
#include "PacketDef.h"
#include "PlayerMySelf.h"
#include "PlayerOtherMgr.h"

#include "N3FXMgr.h"
#include "N3WorldManager.h"

#include "UIStateBar.h"
#include "UIInventory.h"
#include "UIVarious.h"
#include "UIPartyOrForce.h"
#include "UIHotKeyDlg.h"
#include "MagicSkillMng.h"
#include "N3SndObj.h"
#include "N3SndObjStream.h"
#include "N3ShapeExtra.h"

//#include "StdAfxBase.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMagicSkillMng::CMagicSkillMng()
{
	m_pGameProcMain = NULL;
	m_dwRegionMagicState = 0;
	m_dwCastingStateNonAction = 0;
	m_fCastTimeNonAction = 0.0f;
	m_dwNonActionMagicID = 0;
	m_iNonActionMagicTarget = -1;
	//m_fRecastTimeNonAction = 0.0f;

	m_iMyRegionTargetFXID = 0;
	m_fZonePointerRadius = -1.0f;
	m_fZonePointerRadiusEffective = 0.0f;
	m_fZonePointerRotRad = 0.0f;

	Init();
}

CMagicSkillMng::CMagicSkillMng(CGameProcMain* pGameProcMain)
{
	m_pGameProcMain = pGameProcMain;
	m_dwRegionMagicState = 0;
	m_dwCastingStateNonAction = 0;
	m_fCastTimeNonAction = 0.0f;
	m_dwNonActionMagicID = 0;
	m_iNonActionMagicTarget = -1;
	//m_fRecastTimeNonAction = 0.0f;

	m_iMyRegionTargetFXID = 0;
	m_fZonePointerRadius = -1.0f;
	m_fZonePointerRadiusEffective = 0.0f;
	m_fZonePointerRotRad = 0.0f;

	Init();
}

void CMagicSkillMng::Init()
{
	m_pTbl_Type_1 = new CN3TableBase<struct __TABLE_UPC_SKILL_TYPE_1>;
	m_pTbl_Type_1->LoadFromFile("Data\\Skill_Magic_1.tbl");

	m_pTbl_Type_2 = new CN3TableBase<struct __TABLE_UPC_SKILL_TYPE_2>;
	m_pTbl_Type_2->LoadFromFile("Data\\Skill_Magic_2.tbl");

	m_pTbl_Type_3 = new CN3TableBase<struct __TABLE_UPC_SKILL_TYPE_3>;
	m_pTbl_Type_3->LoadFromFile("Data\\Skill_Magic_3.tbl");

	m_pTbl_Type_4 = new CN3TableBase<struct __TABLE_UPC_SKILL_TYPE_4>;
	m_pTbl_Type_4->LoadFromFile("Data\\Skill_Magic_4.tbl");

//	m_pTbl_Type_5 = new CN3TableBase<struct __TABLE_UPC_SKILL_TYPE_5>;
//	m_pTbl_Type_5->LoadFromFile("Data\\Skill_Magic_5.tbl");

//	m_pTbl_Type_6 = new CN3TableBase<struct __TABLE_UPC_SKILL_TYPE_6>;
//	m_pTbl_Type_6->LoadFromFile("Data\\Skill_Magic_6.tbl");

	m_pTbl_Type_7 = new CN3TableBase<struct __TABLE_UPC_SKILL_TYPE_7>;
	m_pTbl_Type_7->LoadFromFile("Data\\Skill_Magic_7.tbl");

//	m_pTbl_Type_8 = new CN3TableBase<struct __TABLE_UPC_SKILL_TYPE_8>;
//	m_pTbl_Type_8->LoadFromFile("Data\\Skill_Magic_8.tbl");

//	m_pTbl_Type_9 = new CN3TableBase<struct __TABLE_UPC_SKILL_TYPE_9>;
//	m_pTbl_Type_9->LoadFromFile("Data\\Skill_Magic_9.tbl");
	
//	m_pTbl_Type_10 = new CN3TableBase<struct __TABLE_UPC_SKILL_TYPE_10>;
//	m_pTbl_Type_10->LoadFromFile("Data\\Skill_Magic_10.tbl");

	m_MySelf.clear();

	m_iTarget = -1;
	m_vTargetPos.Set(0,0,0);

	//m_fRecastTime = 0.0f;
	m_fDelay = 0.0f;

	m_dwRegionMagicState = 0;

	InitType4();

	__InfoPlayerBase* pInfoBase = &(s_pPlayer->m_InfoBase);

	/*
	CLASS_KA_WARRIOR = 101, CLASS_KA_ROGUE, CLASS_KA_WIZARD, CLASS_KA_PRIEST, // 여기까지 기본 직업
	CLASS_KA_BERSERKER = 105, CLASS_KA_GUARDIAN, CLASS_KA_HUNTER = 107, CLASS_KA_PENETRATOR, 
	CLASS_KA_SORCERER = 109, CLASS_KA_NECROMANCER, CLASS_KA_SHAMAN = 111, CLASS_KA_DARKPRIEST, 
	
	CLASS_EL_WARRIOR = 201, CLASS_EL_ROGUE, CLASS_EL_WIZARD, CLASS_EL_PRIEST, // 여기까지 기본 직업 
	CLASS_EL_BLADE = 205, CLASS_EL_PROTECTOR, CLASS_EL_RANGER = 207, CLASS_EL_ASSASIN, 
	CLASS_EL_MAGE = 209, CLASS_EL_ENCHANTER, CLASS_EL_CLERIC = 211, CLASS_EL_DRUID,
	*/

	m_iMyRegionTargetFXID = 0;
	/*
	if( pInfoBase->eClass==CLASS_KA_ROGUE || pInfoBase->eClass==CLASS_KA_HUNTER || pInfoBase->eClass==CLASS_KA_PENETRATOR )
	{
		m_iMyRegionTargetFXID = FXID_REGION_TARGET_KA_ROGUE;
	}
	else if( pInfoBase->eClass==CLASS_KA_WIZARD || pInfoBase->eClass==CLASS_KA_SORCERER || pInfoBase->eClass==CLASS_KA_NECROMANCER )
	{
		m_iMyRegionTargetFXID = FXID_REGION_TARGET_KA_WIZARD;
	}
	else if( pInfoBase->eClass==CLASS_KA_PRIEST || pInfoBase->eClass==CLASS_KA_SHAMAN || pInfoBase->eClass==CLASS_KA_DARKPRIEST )
	{
		m_iMyRegionTargetFXID = FXID_REGION_TARGET_KA_PRIEST;
	}
	else if( pInfoBase->eClass==CLASS_EL_ROGUE || pInfoBase->eClass==CLASS_EL_RANGER || pInfoBase->eClass==CLASS_EL_ASSASIN )
	{
		m_iMyRegionTargetFXID = FXID_REGION_TARGET_EL_ROGUE;
	}
	else if( pInfoBase->eClass==CLASS_EL_WIZARD || pInfoBase->eClass==CLASS_EL_MAGE || pInfoBase->eClass==CLASS_EL_ENCHANTER )
	{
		m_iMyRegionTargetFXID = FXID_REGION_TARGET_EL_WIZARD;
	}
	else if( pInfoBase->eClass==CLASS_EL_PRIEST || pInfoBase->eClass==CLASS_EL_CLERIC || pInfoBase->eClass==CLASS_EL_DRUID )
	{
		m_iMyRegionTargetFXID = FXID_REGION_TARGET_EL_PRIEST;
	}
	*/
}

CMagicSkillMng::~CMagicSkillMng()
{
	m_pGameProcMain = NULL;

	if(m_pTbl_Type_1) { delete m_pTbl_Type_1; m_pTbl_Type_1 = NULL; }
	if(m_pTbl_Type_2) { delete m_pTbl_Type_2; m_pTbl_Type_2 = NULL; }
	if(m_pTbl_Type_3) { delete m_pTbl_Type_3; m_pTbl_Type_3 = NULL; }
	if(m_pTbl_Type_4) { delete m_pTbl_Type_4; m_pTbl_Type_4 = NULL; }
//	if(m_pTbl_Type_5) { delete m_pTbl_Type_5; m_pTbl_Type_5 = NULL; }
//	if(m_pTbl_Type_6) { delete m_pTbl_Type_6; m_pTbl_Type_6 = NULL; }
	if(m_pTbl_Type_7) { delete m_pTbl_Type_7; m_pTbl_Type_7 = NULL; }
//	if(m_pTbl_Type_8) { delete m_pTbl_Type_8; m_pTbl_Type_8 = NULL; }
//	if(m_pTbl_Type_9) { delete m_pTbl_Type_9; m_pTbl_Type_9 = NULL; }
//	if(m_pTbl_Type_10) { delete m_pTbl_Type_10; m_pTbl_Type_10 = NULL; }
}


//
//
//
bool CMagicSkillMng::IsCasting()
{
	if(s_pPlayer->State() == PSA_SPELLMAGIC ||
		s_pPlayer->m_dwMagicID != 0xffffffff ||
		s_pPlayer->m_bStun == true ||
		m_fDelay > 0.0f ) return true;	
	return false;
}

//Used to inform Hotkey and skill tree UI
bool CMagicSkillMng::CheckValidSkillMagic(__TABLE_UPC_SKILL* pSkill)
{
	__InfoPlayerBase* pInfoBase = &(s_pPlayer->m_InfoBase);
	__InfoPlayerMySelf* pInfoExt = &(s_pPlayer->m_InfoExt);

	e_Class_Represent Class = CGameProcedure::GetRepresentClass(pInfoBase->eClass);
	
	//mana check for all classes, no need to 
	//separate because error message is not required
	if(pInfoExt->iMSP < pSkill->iExhaustMSP)
		return false;

	

	if(pInfoBase->iHP < pSkill->iExhaustHP) return false;
	
	int LeftItem = s_pPlayer->ItemClass_LeftHand();
	int RightItem = s_pPlayer->ItemClass_RightHand();
	bool bHasSword = (LeftItem == ITEM_CLASS_SWORD || RightItem == ITEM_CLASS_SWORD);
	bool bHas2HSword = (LeftItem == ITEM_CLASS_SWORD_2H || RightItem == ITEM_CLASS_SWORD_2H);
	bool bHasAxe = (LeftItem == ITEM_CLASS_AXE || RightItem == ITEM_CLASS_AXE);
	bool bHas2HAxe = (LeftItem == ITEM_CLASS_AXE_2H || RightItem == ITEM_CLASS_AXE_2H);
	bool bHasMace = (LeftItem == ITEM_CLASS_MACE || RightItem == ITEM_CLASS_MACE);
	bool bHas2HMace = (LeftItem == ITEM_CLASS_MACE_2H || RightItem == ITEM_CLASS_MACE_2H);
	bool bHasSpear = (LeftItem == ITEM_CLASS_SPEAR || RightItem == ITEM_CLASS_SPEAR);
	bool bHasPolearm = (LeftItem == ITEM_CLASS_POLEARM || RightItem == ITEM_CLASS_POLEARM);
	bool bHasBow = (LeftItem == ITEM_CLASS_BOW || RightItem == ITEM_CLASS_BOW ||
						   LeftItem == ITEM_CLASS_BOW_CROSS || RightItem == ITEM_CLASS_BOW_CROSS ||
						   LeftItem == ITEM_CLASS_BOW_LONG || RightItem == ITEM_CLASS_BOW_LONG);
	bool bHasStaff = (LeftItem == ITEM_CLASS_STAFF || RightItem == ITEM_CLASS_STAFF);
	bool bHasDagger = (LeftItem == ITEM_CLASS_DAGGER || RightItem == ITEM_CLASS_DAGGER);
	bool bNotEquipedItem = (LeftItem == ITEM_CLASS_UNKNOWN && RightItem == ITEM_CLASS_UNKNOWN);
	
	switch (pSkill->dwNeedItem)
	{

		case 0: //dwNeedItem = 0 , all priest & warrior skills
		{
			//break if Bright Dew, Sleep Carpet, Sleep Wing (mage, priest) skills
			if (pSkill->dw1stTableType == 0 || pSkill->dw1stTableType == 7)
			{
				break;
			}

			if (bNotEquipedItem || bHasDagger || bHasStaff || bHasBow)
			{
				return false;
			}

			if (!bHasSword && !bHas2HSword && !bHasAxe && !bHas2HAxe &&
				!bHasMace && !bHas2HMace &&
				!bHasSpear && !bHasPolearm)
			{
				return false;
			}

			break;
		}
		case 1:	//dwNeedItem = 1 , only daggers
		{

			if (bNotEquipedItem || bHasSword || bHas2HSword || bHasAxe || bHas2HAxe ||
				bHasMace || bHas2HMace || bHasSpear || bHasPolearm || bHasBow || bHasStaff)
			{
				return false;
			}

			if (!bHasDagger)
			{
				return false;
			}

			break;
		}
		case 7: //dwNeedItem = 7 , only Bows - archery
		{

			if (bNotEquipedItem || bHasSword || bHas2HSword || bHasAxe || bHas2HAxe ||
				bHasMace || bHas2HMace || bHasSpear || bHasPolearm || bHasDagger || bHasStaff)
			{
				return false;
			}

			if (!bHasBow)
			{
				return false;
			}

			break;
		}
		case 9: //dwNeedItem = 9, no item requirement for skill (example : sprint)
		{
			break;
		}
		case 11: //dwNeedItem = 11, only staves , mage 42 - 72 staff skills
		{

			if (bNotEquipedItem || bHasSword || bHas2HSword || bHasAxe || bHas2HAxe ||
				bHasMace || bHas2HMace || bHasSpear || bHasPolearm || bHasDagger || bHasBow)
			{
				return false;
			}

			if (!bHasStaff)
			{
				return false;
			}

			break;
		}

		default:
		{
			//other skills that requires items in inventory
			//like guardian items, transformation totem, catapult
			int iNum = m_pGameProcMain->m_pUIInventory->GetCountInInvByID(pSkill->dwNeedItem);

			if (iNum <= 0)
			{
				return false;
			}

			break;
		}

	}

	/*
	//melee (attack) & (archery) & (mage staff) skills
	if (pSkill->dw1stTableType == 1 || pSkill->dw1stTableType == 2)
	{
		//no item equipped on hands, cannot cast attack skills
		if (bNotEquipedItem)
		{
			return false;
		}

		if (Class == CLASS_REPRESENT_WARRIOR || Class == CLASS_REPRESENT_PRIEST)
		{
			//warrior and priest cannot attack with staff and dagger
			if (bHasDagger || bHasStaff)
			{
				return false;
			}
		}
		else if (Class == CLASS_REPRESENT_ROGUE)
		{
			if (pSkill->dw1stTableType == 1) //assasin skills
			{
				if (bHasSword || bHas2HSword || bHasAxe || bHas2HAxe || bHasMace || bHas2HMace ||
					bHasSpear || bHasPolearm || bHasBow || bHasStaff)
				{
					return false;
				}
			}
			else //archery skills
			{
				if (bHasDagger || bHasSword || bHas2HSword || bHasAxe || bHas2HAxe || bHasMace ||
					bHas2HMace || bHasSpear || bHasPolearm || bHasStaff)
				{
					return false;
				}
			}
		}
		else if (Class == CLASS_REPRESENT_WIZARD) //staff skills 
		{
			if (bHasSword || bHas2HSword || bHasAxe || bHas2HAxe || bHasMace || bHas2HMace ||
				bHasSpear || bHasPolearm || bHasBow || bHasDagger)
			{
				return false;
			}
		}

	}
	*/

	if(pSkill->dwExhaustItem>0)
	{
		int NumItem = m_pGameProcMain->m_pUIInventory->GetCountInInvByID(pSkill->dwExhaustItem);
		
		if(pSkill->dw1stTableType==2 || pSkill->dw2ndTableType==2)
		{
			__TABLE_UPC_SKILL_TYPE_2* pType2 = m_pTbl_Type_2->Find(pSkill->dwID);
			if (!pType2) return false;
			if(NumItem < pType2->iNumArrow)
			{
				return false;
			}
		}
		else 
		{
			if(NumItem < 1) return false;
		}

		__TABLE_ITEM_BASIC* pItem = NULL;														// 아이템 테이블 구조체 포인터..	
		__TABLE_ITEM_EXT* pItemExt = NULL;														// 아이템 테이블 구조체 포인터..	

		pItem = s_pTbl_Items_Basic.Find(pSkill->dwExhaustItem/1000*1000);	// 열 데이터 얻기..
		if(pItem && pItem->byExtIndex >= 0 && pItem->byExtIndex < MAX_ITEM_EXTENSION)
			pItemExt = s_pTbl_Items_Exts[pItem->byExtIndex].Find(pSkill->dwExhaustItem%1000);	// 열 데이터 얻기..
		if ( NULL == pItem || NULL == pItemExt )
		{
			__ASSERT(0, "NULL Item");
			CLogWriter::Write("MyInfo - Inv - Unknown Item %d, IDNumber", pSkill->dwExhaustItem);
			return false;	// 아이템이 없으면..
		}

		if (pItem->byAttachPoint == ITEM_LIMITED_EXHAUST)
		{
			// 종족 체크..
			switch ( pItem->byNeedRace )
			{
				case 0:
					break;

				default:
					if ( pItem->byNeedRace != CGameBase::s_pPlayer->m_InfoBase.eRace )
						return false;
					break;
			}

			// 직업 체크..
			if (pItem->byNeedClass != 0)
			{
				switch (pItem->byNeedClass)
				{
					case CLASS_KINDOF_WARRIOR:
						switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
						{
							case CLASS_KA_WARRIOR:
							case CLASS_KA_BERSERKER:
							case CLASS_KA_GUARDIAN:
							case CLASS_EL_WARRIOR:
							case CLASS_EL_BLADE:
							case CLASS_EL_PROTECTOR:
								break;
							default:
								return false;
						}
						break;

					case CLASS_KINDOF_ROGUE:
						switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
						{
							case CLASS_KA_ROGUE:
							case CLASS_KA_HUNTER:
							case CLASS_KA_PENETRATOR:
							case CLASS_EL_ROGUE:
							case CLASS_EL_RANGER:
							case CLASS_EL_ASSASIN:
								break;
							default:
								return false;
						}
						break;

					case CLASS_KINDOF_WIZARD:
						switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
						{
							case CLASS_KA_WIZARD:
							case CLASS_KA_SORCERER:
							case CLASS_KA_NECROMANCER:
							case CLASS_EL_WIZARD:
							case CLASS_EL_MAGE:
							case CLASS_EL_ENCHANTER:
								break;
							default:
								return false;
						}
						break;

					case CLASS_KINDOF_PRIEST:
						switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
						{
							case CLASS_KA_PRIEST:
							case CLASS_KA_SHAMAN:
							case CLASS_KA_DARKPRIEST:
							case CLASS_EL_PRIEST:
							case CLASS_EL_CLERIC:
							case CLASS_EL_DRUID:
								break;
							default:
								return false;
						}
						break;

					case CLASS_KINDOF_ATTACK_WARRIOR:
						switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
						{
							case CLASS_KA_BERSERKER:
							case CLASS_EL_BLADE:
								break;
							default:
								return false;
						}
						break;

					case CLASS_KINDOF_DEFEND_WARRIOR:
						switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
						{
							case CLASS_KA_GUARDIAN:
							case CLASS_EL_PROTECTOR:
								break;
							default:
								return false;
						}
						break;

					case CLASS_KINDOF_ARCHER:
						switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
						{
							case CLASS_KA_HUNTER:
							case CLASS_EL_RANGER:
								break;
							default:
								return false;
						}
						break;

					case CLASS_KINDOF_ASSASSIN:
						switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
						{
							case CLASS_KA_PENETRATOR:
							case CLASS_EL_ASSASIN:
								break;
							default:
								return false;
						}
						break;

					case CLASS_KINDOF_ATTACK_WIZARD:
						switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
						{
							case CLASS_KA_SORCERER:
							case CLASS_EL_MAGE:
								break;
							default:
								return false;
						}
						break;

					case CLASS_KINDOF_PET_WIZARD:
						switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
						{
							case CLASS_KA_NECROMANCER:
							case CLASS_EL_ENCHANTER:
								break;
							default:
								return false;
						}
						break;

					case CLASS_KINDOF_HEAL_PRIEST:
						switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
						{
							case CLASS_KA_SHAMAN:
							case CLASS_EL_CLERIC:
								break;
							default:
								return false;
						}
						break;

					case CLASS_KINDOF_CURSE_PRIEST:
						switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
						{
							case CLASS_KA_DARKPRIEST:
							case CLASS_EL_DRUID:
								break;
							default:
								return false;
						}
						break;

					default:
						if (CGameBase::s_pPlayer->m_InfoBase.eClass != pItem->byNeedClass)
							return false;
						break;
				}						
			}

			// 요구레벨 체크..
			if ( CGameBase::s_pPlayer->m_InfoBase.iLevel < pItem->cNeedLevel+pItemExt->siNeedLevel )
				return false;

			// 요구 능력치 체크..
			int iNeedValue;
			iNeedValue = pItem->byNeedStrength;
			if (iNeedValue != 0)
				iNeedValue += pItemExt->siNeedStrength;
			if( iNeedValue > 0 && CGameBase::s_pPlayer->m_InfoExt.iStrength < iNeedValue )		
				return false;

			iNeedValue = pItem->byNeedStamina;
			if (iNeedValue != 0)
				iNeedValue += pItemExt->siNeedStamina;
			if( iNeedValue > 0 && CGameBase::s_pPlayer->m_InfoExt.iStamina < iNeedValue )		
				return false;

			iNeedValue = pItem->byNeedDexterity;
			if (iNeedValue != 0)
				iNeedValue += pItemExt->siNeedDexterity;
			if( iNeedValue > 0 && CGameBase::s_pPlayer->m_InfoExt.iDexterity < iNeedValue )		
				return false;

			iNeedValue = pItem->byNeedInteli;
			if (iNeedValue != 0)
				iNeedValue += pItemExt->siNeedInteli;
			if( iNeedValue > 0 && CGameBase::s_pPlayer->m_InfoExt.iIntelligence < iNeedValue )	
				return false;

			iNeedValue = pItem->byNeedMagicAttack;
			if (iNeedValue != 0)
				iNeedValue += pItemExt->siNeedMagicAttack;
			if( iNeedValue > 0 && CGameBase::s_pPlayer->m_InfoExt.iAttack < iNeedValue )				
				return false;
		}
	}

	return true;
}

bool CMagicSkillMng::CheckValidCondition(int iTargetID, __TABLE_UPC_SKILL* pSkill)
{
	__InfoPlayerBase* pInfoBase = &(s_pPlayer->m_InfoBase);
	__InfoPlayerMySelf* pInfoExt = &(s_pPlayer->m_InfoExt);

	//직업에 맞는 스킬인지 알아봐라...
	e_Class_Represent Class = CGameProcedure::GetRepresentClass(pInfoBase->eClass);
	
	if(pSkill->iNeedSkill!=0)
	{
		if(Class == CLASS_REPRESENT_WARRIOR)
		{
			int NeedSkill = pSkill->iNeedSkill / 10;
			if(NeedSkill != CLASS_KA_WARRIOR && NeedSkill != CLASS_KA_BERSERKER && NeedSkill != CLASS_KA_GUARDIAN &&
				NeedSkill != CLASS_EL_WARRIOR && NeedSkill != CLASS_EL_BLADE && NeedSkill != CLASS_EL_PROTECTOR)
			{
				std::string buff;
				GetText(IDS_SKILL_FAIL_DIFFURENTCLASS, &buff);
				m_pGameProcMain->MsgOutput(buff, 0xffffff00);
				return false;
			}
		}
		else if(Class == CLASS_REPRESENT_ROGUE)
		{
			int NeedSkill = pSkill->iNeedSkill / 10;
			if(NeedSkill != CLASS_KA_ROGUE && NeedSkill != CLASS_KA_HUNTER && NeedSkill != CLASS_KA_PENETRATOR &&
				NeedSkill != CLASS_EL_ROGUE && NeedSkill != CLASS_EL_RANGER && NeedSkill != CLASS_EL_ASSASIN)
			{
				std::string buff;
				GetText(IDS_SKILL_FAIL_DIFFURENTCLASS, &buff);
				m_pGameProcMain->MsgOutput(buff, 0xffffff00);
				return false;
			}
		}
		else if(Class == CLASS_REPRESENT_WIZARD)
		{
			int NeedSkill = pSkill->iNeedSkill / 10;
			if(NeedSkill != CLASS_KA_WIZARD && NeedSkill != CLASS_KA_SORCERER && NeedSkill != CLASS_KA_NECROMANCER &&
				NeedSkill != CLASS_EL_WIZARD && NeedSkill != CLASS_EL_MAGE && NeedSkill != CLASS_EL_ENCHANTER)
			{
				std::string buff;
				GetText(IDS_SKILL_FAIL_DIFFURENTCLASS, &buff);
				m_pGameProcMain->MsgOutput(buff, 0xffffff00);
				return false;
			}
		}
		else if(Class == CLASS_REPRESENT_PRIEST)
		{
			int NeedSkill = pSkill->iNeedSkill / 10;
			if(NeedSkill != CLASS_KA_PRIEST && NeedSkill != CLASS_KA_DARKPRIEST && NeedSkill != CLASS_KA_SHAMAN &&
				NeedSkill != CLASS_EL_PRIEST && NeedSkill != CLASS_EL_CLERIC && NeedSkill != CLASS_EL_DRUID)
			{
				std::string buff;
				GetText(IDS_SKILL_FAIL_DIFFURENTCLASS, &buff);
				m_pGameProcMain->MsgOutput(buff, 0xffffff00);
				return false;
			}
		}
	}

	if(pInfoExt->iMSP < pSkill->iExhaustMSP)
	{
		std::string buff;

		if(Class==CLASS_REPRESENT_PRIEST || Class==CLASS_REPRESENT_WIZARD)
		{
			GetText(IDS_MSG_CASTING_FAIL_LACK_MP, &buff);
			m_pGameProcMain->MsgOutput(buff, 0xffffff00);
		}
		else if (Class == CLASS_REPRESENT_WARRIOR || Class == CLASS_REPRESENT_ROGUE)
		{
			GetText(IDS_SKILL_FAIL_LACK_SP, &buff);
			m_pGameProcMain->MsgOutput(buff, 0xffffff00);
		}

		return false;
	}	


	if(pInfoBase->iHP < pSkill->iExhaustHP)
	{
		std::string buff;
		GetText(IDS_SKILL_FAIL_LACK_HP, &buff);
		m_pGameProcMain->MsgOutput(buff, 0xffffff00);
		return false;
	}

	int LeftItem = s_pPlayer->ItemClass_LeftHand();
	int RightItem = s_pPlayer->ItemClass_RightHand();
	bool bHasSword		= (LeftItem == ITEM_CLASS_SWORD || RightItem == ITEM_CLASS_SWORD);
	bool bHas2HSword	= (LeftItem == ITEM_CLASS_SWORD_2H || RightItem == ITEM_CLASS_SWORD_2H);
	bool bHasAxe		= (LeftItem == ITEM_CLASS_AXE || RightItem == ITEM_CLASS_AXE);
	bool bHas2HAxe		= (LeftItem == ITEM_CLASS_AXE_2H || RightItem == ITEM_CLASS_AXE_2H);
	bool bHasMace		= (LeftItem == ITEM_CLASS_MACE || RightItem == ITEM_CLASS_MACE);
	bool bHas2HMace		= (LeftItem == ITEM_CLASS_MACE_2H || RightItem == ITEM_CLASS_MACE_2H);
	bool bHasSpear		= (LeftItem == ITEM_CLASS_SPEAR || RightItem == ITEM_CLASS_SPEAR);
	bool bHasPolearm	= (LeftItem == ITEM_CLASS_POLEARM || RightItem == ITEM_CLASS_POLEARM);
	bool bHasBow		= (LeftItem == ITEM_CLASS_BOW || RightItem == ITEM_CLASS_BOW ||
						   LeftItem == ITEM_CLASS_BOW_CROSS || RightItem == ITEM_CLASS_BOW_CROSS ||
						   LeftItem == ITEM_CLASS_BOW_LONG || RightItem == ITEM_CLASS_BOW_LONG);
	bool bHasStaff		= (LeftItem == ITEM_CLASS_STAFF || RightItem == ITEM_CLASS_STAFF);
	bool bHasDagger		= (LeftItem == ITEM_CLASS_DAGGER || RightItem == ITEM_CLASS_DAGGER);
	bool bNotEquipedItem = (LeftItem == ITEM_CLASS_UNKNOWN && RightItem == ITEM_CLASS_UNKNOWN);

	
	switch (pSkill->dwNeedItem)
	{

		case 0: //dwNeedItem = 0 , all priest & warrior attack skills
		{

			//break if Bright Dew, Sleep Carpet, Sleep Wing (mage, priest) skills
			if (pSkill->dw1stTableType == 0 || pSkill->dw1stTableType == 7)
			{
				break;
			}

			if (bNotEquipedItem || bHasDagger || bHasStaff || bHasBow)
			{
				std::string buff;
				GetText(IDS_SKILL_FAIL_INVALID_ITEM, &buff);
				m_pGameProcMain->MsgOutput(buff, 0xffffff00);
				return false;
			}

			if (!bHasSword && !bHas2HSword && !bHasAxe && !bHas2HAxe && 
				!bHasMace && !bHas2HMace &&
				!bHasSpear && !bHasPolearm )
			{
				std::string buff;
				GetText(IDS_SKILL_FAIL_LACK_ITEM, &buff);
				m_pGameProcMain->MsgOutput(buff, 0xffffff00);
				return false;
			}

			break;
		}
		case 1:	//dwNeedItem = 1 , only daggers
		{

			if (bNotEquipedItem || bHasSword || bHas2HSword || bHasAxe || bHas2HAxe ||
				bHasMace || bHas2HMace || bHasSpear || bHasPolearm || bHasBow || bHasStaff)
			{
				std::string buff;
				GetText(IDS_SKILL_FAIL_INVALID_ITEM, &buff);
				m_pGameProcMain->MsgOutput(buff, 0xffffff00);
				return false;
			}

			if (!bHasDagger)
			{
				std::string buff;
				GetText(IDS_SKILL_FAIL_LACK_ITEM, &buff);
				m_pGameProcMain->MsgOutput(buff, 0xffffff00);
				return false;
			}

			break;
		}
		case 7: //dwNeedItem = 7 , only Bows - archery
		{

			if (bNotEquipedItem || bHasSword || bHas2HSword || bHasAxe || bHas2HAxe ||
				bHasMace || bHas2HMace || bHasSpear || bHasPolearm || bHasDagger || bHasStaff)
			{
				std::string buff;
				GetText(IDS_SKILL_FAIL_INVALID_ITEM, &buff);
				m_pGameProcMain->MsgOutput(buff, 0xffffff00);
				return false;
			}

			if (!bHasBow)
			{
				std::string buff;
				GetText(IDS_SKILL_FAIL_LACK_ITEM, &buff);
				m_pGameProcMain->MsgOutput(buff, 0xffffff00);
				return false;
			}

			break;
		}
		case 9: //dwNeedItem = 9, no item requirement for skill (example : sprint)
		{
			break;
		}
		case 11: //dwNeedItem = 11, only staves , mage 42 - 72 staff skills
		{

			if (bNotEquipedItem || bHasSword || bHas2HSword || bHasAxe || bHas2HAxe ||
				bHasMace || bHas2HMace || bHasSpear || bHasPolearm || bHasDagger || bHasBow)
			{
				std::string buff;
				GetText(IDS_SKILL_FAIL_INVALID_ITEM, &buff);
				m_pGameProcMain->MsgOutput(buff, 0xffffff00);
				return false;
			}

			if (!bHasStaff)
			{
				std::string buff;
				GetText(IDS_SKILL_FAIL_LACK_ITEM, &buff);
				m_pGameProcMain->MsgOutput(buff, 0xffffff00);
				return false;
			}
			
			break;
		}
		
		default: 
		{
			//other skills that requires items in inventory
			//like guardian items, transformation totem, catapult
			int iNum = m_pGameProcMain->m_pUIInventory->GetCountInInvByID(pSkill->dwNeedItem);

			if (iNum <= 0)
			{
				std::string buff;
				GetText(IDS_SKILL_FAIL_LACK_ITEM, &buff);
				m_pGameProcMain->MsgOutput(buff, 0xffffff00);
				return false;
			}

			break;
		}
			
	}

	
	

	if(pSkill->dwExhaustItem>0)
	{
		int NumItem = m_pGameProcMain->m_pUIInventory->GetCountInInvByID(pSkill->dwExhaustItem);
		if(pSkill->dw1stTableType==2 || pSkill->dw2ndTableType==2)
		{
			__TABLE_UPC_SKILL_TYPE_2* pType2 = m_pTbl_Type_2->Find(pSkill->dwID);
			if(NumItem < pType2->iNumArrow)
			{
				std::string szMsg;
				GetText(IDS_SKILL_FAIL_LACK_ITEM, &szMsg);
				m_pGameProcMain->MsgOutput(szMsg, 0xffffff00);
				return false;
			}
		}
		else 
		{
			if(NumItem < 1)
			{
				std::string szMsg;
				GetText(IDS_SKILL_FAIL_LACK_ITEM, &szMsg);
				m_pGameProcMain->MsgOutput(szMsg, 0xffffff00);
				return false;
			}
		}

		__TABLE_ITEM_BASIC* pItem = NULL;														// 아이템 테이블 구조체 포인터..	
		__TABLE_ITEM_EXT* pItemExt = NULL;														// 아이템 테이블 구조체 포인터..	

		pItem = s_pTbl_Items_Basic.Find(pSkill->dwExhaustItem/1000*1000);	// 열 데이터 얻기..
		if(pItem && pItem->byExtIndex >= 0 && pItem->byExtIndex < MAX_ITEM_EXTENSION)
			pItemExt = s_pTbl_Items_Exts[pItem->byExtIndex].Find(pSkill->dwExhaustItem%1000);	// 열 데이터 얻기..
		if ( NULL == pItem || NULL == pItemExt )
		{
			__ASSERT(0, "NULL Item");
			CLogWriter::Write("MyInfo - Inv - Unknown Item %d, IDNumber", pSkill->dwExhaustItem);
			return false;	// 아이템이 없으면..
		}

		if (pItem->byAttachPoint == ITEM_LIMITED_EXHAUST)
		{
			// 종족 체크..
			switch ( pItem->byNeedRace )
			{
				case 0:
					break;

				default:
					if ( pItem->byNeedRace != CGameBase::s_pPlayer->m_InfoBase.eRace )
						return false;
					break;
			}

			// 직업 체크..
			if (pItem->byNeedClass != 0)
			{
				switch (pItem->byNeedClass)
				{
					case CLASS_KINDOF_WARRIOR:
						switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
						{
							case CLASS_KA_WARRIOR:
							case CLASS_KA_BERSERKER:
							case CLASS_KA_GUARDIAN:
							case CLASS_EL_WARRIOR:
							case CLASS_EL_BLADE:
							case CLASS_EL_PROTECTOR:
								break;
							default:
								return false;
						}
						break;

					case CLASS_KINDOF_ROGUE:
						switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
						{
							case CLASS_KA_ROGUE:
							case CLASS_KA_HUNTER:
							case CLASS_KA_PENETRATOR:
							case CLASS_EL_ROGUE:
							case CLASS_EL_RANGER:
							case CLASS_EL_ASSASIN:
								break;
							default:
								return false;
						}
						break;

					case CLASS_KINDOF_WIZARD:
						switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
						{
							case CLASS_KA_WIZARD:
							case CLASS_KA_SORCERER:
							case CLASS_KA_NECROMANCER:
							case CLASS_EL_WIZARD:
							case CLASS_EL_MAGE:
							case CLASS_EL_ENCHANTER:
								break;
							default:
								return false;
						}
						break;

					case CLASS_KINDOF_PRIEST:
						switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
						{
							case CLASS_KA_PRIEST:
							case CLASS_KA_SHAMAN:
							case CLASS_KA_DARKPRIEST:
							case CLASS_EL_PRIEST:
							case CLASS_EL_CLERIC:
							case CLASS_EL_DRUID:
								break;
							default:
								return false;
						}
						break;

					case CLASS_KINDOF_ATTACK_WARRIOR:
						switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
						{
							case CLASS_KA_BERSERKER:
							case CLASS_EL_BLADE:
								break;
							default:
								return false;
						}
						break;

					case CLASS_KINDOF_DEFEND_WARRIOR:
						switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
						{
							case CLASS_KA_GUARDIAN:
							case CLASS_EL_PROTECTOR:
								break;
							default:
								return false;
						}
						break;

					case CLASS_KINDOF_ARCHER:
						switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
						{
							case CLASS_KA_HUNTER:
							case CLASS_EL_RANGER:
								break;
							default:
								return false;
						}
						break;

					case CLASS_KINDOF_ASSASSIN:
						switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
						{
							case CLASS_KA_PENETRATOR:
							case CLASS_EL_ASSASIN:
								break;
							default:
								return false;
						}
						break;

					case CLASS_KINDOF_ATTACK_WIZARD:
						switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
						{
							case CLASS_KA_SORCERER:
							case CLASS_EL_MAGE:
								break;
							default:
								return false;
						}
						break;

					case CLASS_KINDOF_PET_WIZARD:
						switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
						{
							case CLASS_KA_NECROMANCER:
							case CLASS_EL_ENCHANTER:
								break;
							default:
								return false;
						}
						break;

					case CLASS_KINDOF_HEAL_PRIEST:
						switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
						{
							case CLASS_KA_SHAMAN:
							case CLASS_EL_CLERIC:
								break;
							default:
								return false;
						}
						break;

					case CLASS_KINDOF_CURSE_PRIEST:
						switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
						{
							case CLASS_KA_DARKPRIEST:
							case CLASS_EL_DRUID:
								break;
							default:
								return false;
						}
						break;

					default:
						if (CGameBase::s_pPlayer->m_InfoBase.eClass != pItem->byNeedClass)
							return false;
						break;
				}						
			}

			// 요구레벨 체크..
			if ( CGameBase::s_pPlayer->m_InfoBase.iLevel < pItem->cNeedLevel+pItemExt->siNeedLevel )
				return false;

			// 요구 능력치 체크..
			int iNeedValue;
			iNeedValue = pItem->byNeedStrength;
			if (iNeedValue != 0)
				iNeedValue += pItemExt->siNeedStrength;
			if( iNeedValue > 0 && CGameBase::s_pPlayer->m_InfoExt.iStrength < iNeedValue )		
				return false;

			iNeedValue = pItem->byNeedStamina;
			if (iNeedValue != 0)
				iNeedValue += pItemExt->siNeedStamina;
			if( iNeedValue > 0 && CGameBase::s_pPlayer->m_InfoExt.iStamina < iNeedValue )		
				return false;

			iNeedValue = pItem->byNeedDexterity;
			if (iNeedValue != 0)
				iNeedValue += pItemExt->siNeedDexterity;
			if( iNeedValue > 0 && CGameBase::s_pPlayer->m_InfoExt.iDexterity < iNeedValue )		
				return false;

			iNeedValue = pItem->byNeedInteli;
			if (iNeedValue != 0)
				iNeedValue += pItemExt->siNeedInteli;
			if( iNeedValue > 0 && CGameBase::s_pPlayer->m_InfoExt.iIntelligence < iNeedValue )	
				return false;

			iNeedValue = pItem->byNeedMagicAttack;
			if (iNeedValue != 0)
				iNeedValue += pItemExt->siNeedMagicAttack;
			if( iNeedValue > 0 && CGameBase::s_pPlayer->m_InfoExt.iAttack < iNeedValue )				
				return false;
		}
	}
	
	if((pSkill->dw1stTableType==3 || pSkill->dw2ndTableType==3) &&
		pSkill->iTarget==SKILLMAGIC_TARGET_SELF)
	{
		__TABLE_UPC_SKILL_TYPE_3* pType3 = m_pTbl_Type_3->Find(pSkill->dwID);
		if(!pType3) return false;

		int key = 0;
		if(pType3->iStartDamage>0 || (pType3->iStartDamage==0 && pType3->iDuraDamage>0) ) key = DDTYPE_TYPE3_DUR_OUR;
		else key = DDTYPE_TYPE3_DUR_ENEMY;

		key += pType3->iDDType;

		if(key==DDTYPE_TYPE3_DUR_OUR)
		{
			std::multimap<int, uint32_t>::iterator it, itend;
			itend = m_ListBuffTypeID.end();
			it = m_ListBuffTypeID.find(key);
			if(it!=itend) return false;
		}
	}

	if( (pSkill->dw1stTableType==4 || pSkill->dw2ndTableType==4) && 
		( (pSkill->iTarget==SKILLMAGIC_TARGET_SELF) || (iTargetID==s_pPlayer->IDNumber()) ) )
	{
		__TABLE_UPC_SKILL_TYPE_4* pType4 = m_pTbl_Type_4->Find(pSkill->dwID);
		if(!pType4) return false;
		
		switch(pType4->iBuffType)
		{
		case BUFFTYPE_MAXHP:
			if(m_iMaxHP != 0) return false;
			break;
		case BUFFTYPE_AC:
			if(m_iAC != 0) return false;
			break;
		case BUFFTYPE_ATTACK:
			if(m_iAttack != 0) return false;
			break;
		case BUFFTYPE_ATTACKSPEED:
			if(m_fAttackSpeed != 1.0f) return false;
			break;
		case BUFFTYPE_SPEED:
			if(m_fSpeed != 1.0f) return false;
			break;
		case BUFFTYPE_ABILITY:
			if(	m_iStr != 0 || m_iSta != 0 || m_iDex != 0 || m_iInt != 0 || m_iMAP != 0) return false;
			break;
		case BUFFTYPE_RESIST:
			if(	m_iFireR != 0 || m_iColdR != 0 || m_iLightningR != 0 || m_iMagicR != 0 || m_iDeseaseR != 0 || m_iPoisonR != 0) return false;
			break;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// 스킬 사용시 오브젝트 체크
	CPlayerBase* pTarget = m_pGameProcMain->CharacterGetByID(iTargetID, false);
	if(pTarget == NULL) return true;

	__Vector3 vNormal, vMyPos, vGap, vDir, vSkillPos;

	vMyPos = s_pPlayer->Position();
	vMyPos.y += s_pPlayer->Height() / 2;

	vDir = (pTarget->Position() + pTarget->Height()/2) - vMyPos;
	vGap = vDir;
	vDir.Normalize();


	bool bColShape = ACT_WORLD->CheckCollisionWithShape(vMyPos, vDir, vGap.Magnitude(), &vSkillPos, &vNormal);

	switch(pSkill->iTarget)
	{
	case SKILLMAGIC_TARGET_SELF:
		{
			break;
		}
	case SKILLMAGIC_TARGET_FRIEND_WITHME:
		{
//			if(pTarget->m_InfoBase.eNation==pInfoBase->eNation)
//			{
//				if(bColShape)
//				{
//					std::string szMsg;
//					GetText(IDS_SKILL_FAIL_OBJECT_BLOCK, &szMsg);
//					m_pGameProcMain->MsgOutput(szMsg, 0xffffff00);
//					return false;
//				}
//			}
			break;
		}
	case SKILLMAGIC_TARGET_FRIEND_ONLY:
		{
//			if(pTarget->m_InfoBase.eNation==pInfoBase->eNation)
//			{
//				if(bColShape)
//				{
//					std::string szMsg;
//					GetText(IDS_SKILL_FAIL_OBJECT_BLOCK, &szMsg);
//					m_pGameProcMain->MsgOutput(szMsg, 0xffffff00);
//					return false;
//				}
//			}
			break;
		}
	case SKILLMAGIC_TARGET_PARTY:
		{
//			__InfoPartyOrForce* pInfo = (__InfoPartyOrForce*)m_pGameProcMain->m_pUIPartyOrForce->MemberInfoGetSelected();
//			if(!pInfo && iTargetID==-1)
//				return true;

//			int iMemberIndex  = -1;
//			if( m_pGameProcMain->m_pUIPartyOrForce->MemberInfoGetByID(pTarget->IDNumber(), iMemberIndex) )
//			{
//				if(bColShape)
//				{
//					std::string szMsg;
//					GetText(IDS_SKILL_FAIL_OBJECT_BLOCK, &szMsg);
//					m_pGameProcMain->MsgOutput(szMsg, 0xffffff00);
//					return false;
//				}
//			}
			break;
		}
	case SKILLMAGIC_TARGET_NPC_ONLY:
		{
			if(bColShape)
			{
				std::string szMsg;
				GetText(IDS_SKILL_FAIL_OBJECT_BLOCK, &szMsg);
				m_pGameProcMain->MsgOutput(szMsg, 0xffffff00);
				return false;
			}
			break;
		}
	case SKILLMAGIC_TARGET_PARTY_ALL:
		{
			break;
		}
	case SKILLMAGIC_TARGET_ENEMY_ONLY:
		{
			if(pTarget->m_InfoBase.eNation!=pInfoBase->eNation)
			{
				if(bColShape)
				{
					std::string szMsg;
					GetText(IDS_SKILL_FAIL_OBJECT_BLOCK, &szMsg);
					m_pGameProcMain->MsgOutput(szMsg, 0xffffff00);
					return false;
				}
			}
			break;
		}
	case SKILLMAGIC_TARGET_ALL:
		{
			if(bColShape)
			{
				std::string szMsg;
				GetText(IDS_SKILL_FAIL_OBJECT_BLOCK, &szMsg);
				m_pGameProcMain->MsgOutput(szMsg, 0xffffff00);
				return false;
			}
			break;
		}
	case SKILLMAGIC_TARGET_AREA:
	case SKILLMAGIC_TARGET_AREA_ENEMY:
	case SKILLMAGIC_TARGET_AREA_FRIEND:
	case SKILLMAGIC_TARGET_AREA_ALL:
		{
			break;
		}
	case SKILLMAGIC_TARGET_DEAD_FRIEND_ONLY:
		{
			if(pTarget->m_InfoBase.eNation==pInfoBase->eNation && pTarget->IsDead())
			{
				if(bColShape)
				{
					std::string szMsg;
					GetText(IDS_SKILL_FAIL_OBJECT_BLOCK, &szMsg);
					m_pGameProcMain->MsgOutput(szMsg, 0xffffff00);
					return false;
				}
			}
			break;
		}
	default:
		break;
	}

// 스킬 사용시 오브젝트 체크
/////////////////////////////////////////////////////////////////////////////////////////////////////

	return true;
}


//
///
//
bool CMagicSkillMng::MsgSend_MagicProcess(int iTargetID, __TABLE_UPC_SKILL* pSkill)
{
	//if(m_fRecastTime > 0.0f) return;//recast time이 아직 안되었네..^^
	if(s_pPlayer->IsDead()) return false; // 죽어 있네.. ^^

	///////////////////////////////////////////////////////////////////////////////////
	// 스킬 쓸 조건이 되는지 검사...
	// Existing validity checks
	if (!pSkill) return false;
	// Check cooldowns first
	auto itRecast = m_RecastTimes.find(pSkill->dwID);
	auto itNonAction = m_NonActionRecastTimes.find(pSkill->dwID);

	if (pSkill->iSelfAnimID1 > 0) // Casting skill with animation
	{
		if (IsCasting() || (itRecast != m_RecastTimes.end() && itRecast->second > 0))
			return false;
	}
	else // Instant/non-action skill
	{
		if (itNonAction != m_NonActionRecastTimes.end() && itNonAction->second > 0)
			return false;
		m_dwCastingStateNonAction = 0;
		m_fCastTimeNonAction = 0.0f;
		m_dwNonActionMagicID = 0;
		m_iNonActionMagicTarget = -1;
	}
	if(!CheckValidCondition(iTargetID, pSkill)) return false;

	//TRACE("마법성공 state : %d time %.2f\n", s_pPlayer->State(), CN3Base::TimeGet());
	// 스킬 쓸 조건이 되는지 검사 끝...
	///////////////////////////////////////////////////////////////////////////////////
	__InfoPlayerBase* pInfoBase = &(s_pPlayer->m_InfoBase);
	__InfoPlayerMySelf* pInfoExt = &(s_pPlayer->m_InfoExt);
	CPlayerBase* pTarget = m_pGameProcMain->CharacterGetByID(iTargetID, false);

	//지역마법타겟 초기화..
	CGameProcedure::s_pFX->Stop(s_pPlayer->IDNumber(), s_pPlayer->IDNumber(), m_iMyRegionTargetFXID, m_iMyRegionTargetFXID, true);

	CancelZonePointer();

	m_dwRegionMagicState = 0;	
	if(m_iMyRegionTargetFXID == 0)
	{
		if(pInfoBase->eNation == NATION_KARUS)
		{
			m_iMyRegionTargetFXID = FXID_REGION_TARGET_KA_WIZARD;
		}
		else if(pInfoBase->eNation == NATION_ELMORAD)
		{
			m_iMyRegionTargetFXID = FXID_REGION_TARGET_EL_WIZARD;
		}
	}
	//	

//	if(!pTarget) return false;//임시 일단 죽어 있다면 리턴을 한다.

	float fDist = s_pPlayer->Radius() + 1.0f; // 공격 거리제한..
	if(pTarget) fDist += pTarget->Radius();

	switch(pSkill->iTarget)
	{
	case SKILLMAGIC_TARGET_SELF:
		{
			StartSkillMagicAtTargetPacket(pSkill, (int16_t)s_pPlayer->IDNumber());
			return true;
		}
	case SKILLMAGIC_TARGET_FRIEND_WITHME:
		{
			if(!pTarget)
			{
				StartSkillMagicAtTargetPacket(pSkill, (int16_t)s_pPlayer->IDNumber());
				return true;
			}
			else if(pTarget->m_InfoBase.eNation==pInfoBase->eNation)
			{
				if( !CheckValidDistance(pSkill, pTarget->Position(), fDist) ) return false;
				StartSkillMagicAtTargetPacket(pSkill, (int16_t)pTarget->IDNumber());
				return true;
			}
			break;
		}
	case SKILLMAGIC_TARGET_FRIEND_ONLY:
		{
			if(pTarget && pTarget->m_InfoBase.eNation==pInfoBase->eNation)
			{
				if( !CheckValidDistance(pSkill, pTarget->Position(), fDist) ) return false;
				StartSkillMagicAtTargetPacket(pSkill, (int16_t)pTarget->IDNumber());
				return true;
			}
			break;
		}
	case SKILLMAGIC_TARGET_PARTY:
		{
			__InfoPartyOrForce* pInfo = (__InfoPartyOrForce*)m_pGameProcMain->m_pUIPartyOrForce->MemberInfoGetSelected();
			if(!pInfo && iTargetID==-1) pTarget = (CPlayerBase*)s_pPlayer;

			int iMemberIndex  = -1;
			if(pTarget && 
				( m_pGameProcMain->m_pUIPartyOrForce->MemberInfoGetByID(pTarget->IDNumber(), iMemberIndex) ||
				pTarget->IDNumber() == s_pPlayer->IDNumber() ) )
			{
				if( !CheckValidDistance(pSkill, pTarget->Position(), fDist) ) return false;
				StartSkillMagicAtTargetPacket(pSkill, (int16_t)pTarget->IDNumber());
				return true;
			}
			else if(pInfo)	//거리에 상관없이 파티원들에게 쓸때...
			{
				StartSkillMagicAtTargetPacket(pSkill, (int16_t)pInfo->iID);
				return true;
			}
			break;
		}
	case SKILLMAGIC_TARGET_NPC_ONLY:
		{
			if(pTarget && s_pOPMgr->NPCGetByID(pTarget->IDNumber(), true))
			{
				if( !CheckValidDistance(pSkill, pTarget->Position(), fDist) ) return false;
				StartSkillMagicAtTargetPacket(pSkill, (int16_t)pTarget->IDNumber());
				return true;
			}
			break;
		}
	case SKILLMAGIC_TARGET_PARTY_ALL:
		{
			StartSkillMagicAtPosPacket(pSkill, s_pPlayer->Position());
			return true;
		}
	case SKILLMAGIC_TARGET_ENEMY_ONLY:
		{
			if(pTarget && pTarget->m_InfoBase.eNation!=pInfoBase->eNation)
			{
				if( !CheckValidDistance(pSkill, pTarget->Position(), fDist) ) return false;
				StartSkillMagicAtTargetPacket(pSkill, (int16_t)pTarget->IDNumber());
				//CLogWriter::Write("send msg : %.4f", CN3Base::TimeGet());
				//TRACE("send msg : %.4f\n", CN3Base::TimeGet());
				return true;
			}
			break;
		}
	case SKILLMAGIC_TARGET_ALL:
		{
			if(pTarget)
			{
				if( !CheckValidDistance(pSkill, pTarget->Position(), fDist) ) return false;
				StartSkillMagicAtTargetPacket(pSkill, (int16_t)pTarget->IDNumber());
				return true;
			}
			break;
		}
	case SKILLMAGIC_TARGET_AREA:
		{
			StartSkillMagicAtPosPacket(pSkill, s_pPlayer->Position());
			return true;
		}
	case SKILLMAGIC_TARGET_AREA_ENEMY:
	case SKILLMAGIC_TARGET_AREA_FRIEND:
	case SKILLMAGIC_TARGET_AREA_ALL:
		{
			m_dwRegionMagicState = 1;
			m_dwRegionSkill = (*pSkill);
//			CGameProcedure::s_pFX->TriggerBundle(s_pPlayer->IDNumber(), 0, m_iMyRegionTargetFXID, m_pGameProcMain->m_vMouseLBClickedPos, m_iMyRegionTargetFXID);	//전격무기...
			CGameProcedure::s_pFX->TriggerBundle(s_pPlayer->IDNumber(), 0, m_iMyRegionTargetFXID, m_pGameProcMain->m_vMouseSkillPos, m_iMyRegionTargetFXID);	//전격무기...

			// Zone pointer circle FX with scaling

			switch (pSkill->dw1stTableType)
			{
				case 3:
				{
					__TABLE_UPC_SKILL_TYPE_3* pType3 = m_pTbl_Type_3->Find(pSkill->dwID);
					if (pType3 != nullptr)
						m_fZonePointerRadius = pType3->iRadius - 1.0f;
				} break;

				case 4:
				{
					__TABLE_UPC_SKILL_TYPE_4* pType4 = m_pTbl_Type_4->Find(pSkill->dwID);
					if (pType4 != nullptr)
						m_fZonePointerRadius = pType4->iRadius - 1.0f;
				} break;

				case 7:
				{
					__TABLE_UPC_SKILL_TYPE_7* pType7 = m_pTbl_Type_7->Find(pSkill->dwID);
					if (pType7 != nullptr)
						m_fZonePointerRadius = pType7->iRadius - 1.0f;
				} break;

				default:
					return true;
			}

			m_fZonePointerRotRad = 0.0f;
			m_fZonePointerRadiusEffective = 0.0f;

			// Start FX elements
			for (int j = 0; j < 8; j++)
			{
				CGameProcedure::s_pFX->TriggerBundle(
					s_pPlayer->IDNumber(),
					0,
					FXID_ZONE_POINTER,
					m_pGameProcMain->m_vMouseSkillPos,
					FXID_ZONE_POINTER + j);	// Unique index
			}

			return true;
		}
	case SKILLMAGIC_TARGET_DEAD_FRIEND_ONLY:
		{
			if(pTarget && pTarget->m_InfoBase.eNation==pInfoBase->eNation && pTarget->IsDead())
			{
				if( !CheckValidDistance(pSkill, pTarget->Position(), fDist) ) return false;
				StartSkillMagicAtTargetPacket(pSkill, (int16_t)pTarget->IDNumber());
				return true;
			}
			break;
		}
	default:
		break;
	}

	return false;
}

void CMagicSkillMng::SetSkillCooldown(__TABLE_UPC_SKILL* pSkill)
{
	// Convert iReCastTime (stored in tenths of seconds) to seconds
	const float fCooldown = static_cast<float>(pSkill->iReCastTime) / 10.0f;

	if (pSkill->iSelfAnimID1 > 0)
	{
		m_RecastTimes[pSkill->dwID] = fCooldown;
	}
	else
	{
		m_NonActionRecastTimes[pSkill->dwID] = fCooldown;
	}
}

bool CMagicSkillMng::CheckValidDistance(__TABLE_UPC_SKILL* pSkill, __Vector3 vTargetPos, float fTargetRadius)
{
	float fDist = (vTargetPos - s_pPlayer->Position()).Magnitude(); // 공격 거리를 구하고..

	if(pSkill->iValidDist > 0 && fDist <= (pSkill->iValidDist+fTargetRadius + 1.0f)) return true;

	//type1
	if(pSkill->dw1stTableType==1 || pSkill->dw2ndTableType==1)
	{
		__IconItemSkill* pItemIcon = m_pGameProcMain->m_pUIInventory->m_pMySlot[ITEM_SLOT_HAND_RIGHT];
		if(pItemIcon)
		{
			float fValidDist = (pItemIcon->pItemBasic->siAttackRange/10.0f) + fTargetRadius + 1.0f;
			if(fValidDist >= fDist) return true;
		}
	}

	//화살쏠때....
	if(pSkill->dw1stTableType==2 || pSkill->dw2ndTableType==2)
	{
		__IconItemSkill* pItemIcon1 = m_pGameProcMain->m_pUIInventory->m_pMySlot[ITEM_SLOT_HAND_LEFT];
		__IconItemSkill* pItemIcon2 = m_pGameProcMain->m_pUIInventory->m_pMySlot[ITEM_SLOT_HAND_RIGHT];
		float ItemDistance = 0.0f;

		if(pItemIcon2) ItemDistance = pItemIcon2->pItemBasic->siAttackRange/10.0f;
		if(pItemIcon1) ItemDistance = pItemIcon1->pItemBasic->siAttackRange/10.0f;

		float fValidDist = ItemDistance + fTargetRadius + 1.0f;
		__TABLE_UPC_SKILL_TYPE_2* pType2 = m_pTbl_Type_2->Find(pSkill->dwID);
		fValidDist *= (float)pType2->iAddDist;
		fValidDist /= 100.0f;

		if(fValidDist >= fDist) return true;
	}

	std::string szMsg;
	GetTextF(IDS_SKILL_FAIL_SOFAR, &szMsg, pSkill->szName.c_str());
	m_pGameProcMain->MsgOutput(szMsg, 0xffffff00);

	return false;
}

void CMagicSkillMng::StartSkillMagicAtPosPacket(__TABLE_UPC_SKILL* pSkill, __Vector3 vPos)
{
	if(!pSkill) return;
	int SourceID = s_pPlayer->IDNumber();

	if(pSkill->iSelfAnimID1<0)
	{
		m_dwCastingStateNonAction = 1;
		m_fCastTimeNonAction = (float)pSkill->iCastTime / 10.0f;
		m_dwNonActionMagicID = pSkill->dwID;
		m_iNonActionMagicTarget = -1;
		//m_fRecastTimeNonAction = (float)(pSkill->iReCastTime) / 10.0f;

		int spart1 = pSkill->iSelfPart1 % 1000;
		int spart2 = pSkill->iSelfPart1 / 1000;
		spart2 = abs(spart2);

		CGameProcedure::s_pFX->TriggerBundle(SourceID, spart1, pSkill->iSelfFX1, SourceID, spart1, -1);
		if(spart2!=0) CGameProcedure::s_pFX->TriggerBundle(SourceID, spart2, pSkill->iSelfFX1, SourceID, spart2, -2);
		SetSkillCooldown(pSkill);
		return;
	}
	m_pGameProcMain->CommandSitDown(false, false); // 혹시라도 앉아있음 일으켜 세운다..

	if (pSkill->iCastTime == 0)
	{
		std::string szMsg;
		GetTextF(IDS_SKILL_USE, &szMsg, pSkill->szName.c_str());
		m_pGameProcMain->MsgOutput(szMsg, 0xffffff00);

		uint8_t byBuff[32];
		int iOffset = 0;
		CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t) WIZ_MAGIC_PROCESS);
		CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t) N3_SP_MAGIC_EFFECTING);
		CAPISocket::MP_AddDword(byBuff, iOffset, (int) pSkill->dwID);
		CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t) SourceID);
		CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t) -1);

		CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t) vPos.x);
		CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t) vPos.y);
		CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t) vPos.z);

		CAPISocket::MP_AddShort(byBuff, iOffset, 0);
		CAPISocket::MP_AddShort(byBuff, iOffset, 0);
		CAPISocket::MP_AddShort(byBuff, iOffset, 0);

		CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보낸다..	
		return;
	}

	///////////////////////////////////////////////////////////////////////////////
	//	
	s_pPlayer->m_dwMagicID = pSkill->dwID;
	s_pPlayer->m_fCastingTime = 0.0f;
	m_iTarget = -1;
	m_vTargetPos = vPos;
		
	int spart1 = pSkill->iSelfPart1 % 1000;
	int spart2 = pSkill->iSelfPart1 / 1000;
	spart2 = abs(spart2);

	CGameProcedure::s_pFX->TriggerBundle(SourceID, spart1, pSkill->iSelfFX1, SourceID, spart1, -1);
	if(spart2!=0) CGameProcedure::s_pFX->TriggerBundle(SourceID, spart2, pSkill->iSelfFX1, SourceID, spart2, -2);

	s_pPlayer->m_iIDTarget = -1;
	//s_pPlayer->ActionMove(PSM_STOP);
	m_pGameProcMain->CommandMove(MD_STOP, true);

	s_pPlayer->m_iMagicAni = pSkill->iSelfAnimID1;
	if(pSkill->dw1stTableType==2 || pSkill->dw2ndTableType==2)
	{
		int LeftItem = s_pPlayer->ItemClass_LeftHand();
		int RightItem = s_pPlayer->ItemClass_RightHand();
		if(RightItem == ITEM_CLASS_BOW_CROSS || LeftItem==ITEM_CLASS_BOW_CROSS)
		{
			s_pPlayer->m_iMagicAni = ANI_SHOOT_QUARREL_A;
		}
	}
	s_pPlayer->m_fCastFreezeTime = 10.0f;
	s_pPlayer->Action(PSA_SPELLMAGIC, false, NULL);

	////////////////////////////////////////////////////////////////////////////////////////

	uint8_t byBuff[32];
	int iOffset=0;
	CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)WIZ_MAGIC_PROCESS);
	CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)N3_SP_MAGIC_CASTING);
	CAPISocket::MP_AddDword(byBuff, iOffset, (int)pSkill->dwID);
	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)SourceID);
	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)-1);

	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)vPos.x);
	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)vPos.y);
	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)vPos.z);
	
	CAPISocket::MP_AddShort(byBuff, iOffset, 0);
	CAPISocket::MP_AddShort(byBuff, iOffset, 0);
	CAPISocket::MP_AddShort(byBuff, iOffset, 0);
	
	CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보낸다..
	SetSkillCooldown(pSkill);

	if(pSkill->iTarget == SKILLMAGIC_TARGET_ENEMY_ONLY) m_pGameProcMain->PlayBGM_Battle();
}

void CMagicSkillMng::StartSkillMagicAtTargetPacket(__TABLE_UPC_SKILL* pSkill, int16_t TargetID)
{
	if(!pSkill) return;
	int SourceID = s_pPlayer->IDNumber();
	if(pSkill->iSelfAnimID1<0)
	{
		m_dwCastingStateNonAction = 1;
		m_fCastTimeNonAction = (float)(pSkill->iCastTime) / 10.0f;
		m_dwNonActionMagicID = pSkill->dwID;
		m_iNonActionMagicTarget = TargetID;
		//m_fRecastTimeNonAction = (float)(pSkill->iReCastTime) / 10.0f;

		int spart1 = pSkill->iSelfPart1 % 1000;
		int spart2 = pSkill->iSelfPart1 / 1000;
		spart2 = abs(spart2);

		CGameProcedure::s_pFX->TriggerBundle(SourceID, spart1, pSkill->iSelfFX1, SourceID, spart1, -1);
		if(spart2!=0) CGameProcedure::s_pFX->TriggerBundle(SourceID, spart2, pSkill->iSelfFX1, SourceID, spart2, -2);
		SetSkillCooldown(pSkill);
		return;
	}

	m_pGameProcMain->CommandSitDown(false, false); // 혹시라도 앉아있음 일으켜 세운다..
	
	if((pSkill->dw1stTableType==1 || pSkill->dw2ndTableType==1) && pSkill->iCastTime==0)
	{
		CPlayerBase* pTarget = m_pGameProcMain->CharacterGetByID(TargetID, true);
		if(!pTarget) return;

		//바로 skill로 들어가..^^
		//casting packet은 보내지 않고..바로 effect packet을 보낸다..

		//기술 애니메이션 드가...=^^=
		//효과있으면 같이 드가..
		__TABLE_UPC_SKILL_TYPE_1* pType1 = m_pTbl_Type_1->Find(pSkill->dwID);
		if(!pType1) return;

		// 검기 색을 바꾸어 준다..
//		D3DCOLOR crTrace = TraceColorGet(pSkill); // 스킬의 종류에 따라 검기의 색을 정한다..
//		s_pPlayer->PlugTraceColorRemake(crTrace); // 검기 색 적용..

		s_pPlayer->RotateTo(pTarget);

		m_iTarget = TargetID;

		for(int i=0;i<pType1->iNumCombo;i++)
		{
			bool bImmediately = ((0 == i) ? true : false); // 처음건 바로 넣는다..
			s_pPlayer->AnimationAdd((e_Ani)pType1->iAct[i], bImmediately);
		}			
		
		std::string szMsg;
		GetTextF(IDS_SKILL_USE, &szMsg, pSkill->szName.c_str());
		m_pGameProcMain->MsgOutput(szMsg, 0xffffff00);
		
		uint8_t byBuff[32];
		int iOffset = 0;
		CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t) WIZ_MAGIC_PROCESS);
		CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t) N3_SP_MAGIC_EFFECTING);
		CAPISocket::MP_AddDword(byBuff, iOffset, (int) pSkill->dwID);
		CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t) SourceID);
		CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t) TargetID);

		CAPISocket::MP_AddShort(byBuff, iOffset, 1);
		CAPISocket::MP_AddShort(byBuff, iOffset, 1);
		CAPISocket::MP_AddShort(byBuff, iOffset, 0);

		CAPISocket::MP_AddShort(byBuff, iOffset, 0);
		CAPISocket::MP_AddShort(byBuff, iOffset, 0);
		CAPISocket::MP_AddShort(byBuff, iOffset, 0);
		
		CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보낸다..	
		SetSkillCooldown(pSkill);
		return;
	}

	if (pSkill->iCastTime == 0)
	{
		std::string szMsg;
		GetTextF(IDS_SKILL_USE, &szMsg, pSkill->szName.c_str());
		m_pGameProcMain->MsgOutput(szMsg, 0xffffff00);

		uint8_t byBuff[32];
		int iOffset = 0;
		CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t) WIZ_MAGIC_PROCESS);
		CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t) N3_SP_MAGIC_EFFECTING);
		CAPISocket::MP_AddDword(byBuff, iOffset, (int) pSkill->dwID);
		CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t) SourceID);
		CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t) TargetID);

		CAPISocket::MP_AddShort(byBuff, iOffset, 0);
		CAPISocket::MP_AddShort(byBuff, iOffset, 0);
		CAPISocket::MP_AddShort(byBuff, iOffset, 0);
		
		CAPISocket::MP_AddShort(byBuff, iOffset, 0);
		CAPISocket::MP_AddShort(byBuff, iOffset, 0);
		CAPISocket::MP_AddShort(byBuff, iOffset, 0);
		
		CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보낸다..	
		SetSkillCooldown(pSkill);
		return;
	}

	/////////////////////////////////////////////////////////////
	//내껀 패킷 보내면서 그냥 처리..
	s_pPlayer->m_dwMagicID = pSkill->dwID;
	s_pPlayer->m_fCastingTime = 0.0f;
	m_iTarget = TargetID;
	
	CPlayerBase* pTargetPlayer = m_pGameProcMain->CharacterGetByID(TargetID, false);
			
	int spart1 = pSkill->iSelfPart1 % 1000;
	int spart2 = pSkill->iSelfPart1 / 1000;
	spart2 = abs(spart2);

	CGameProcedure::s_pFX->TriggerBundle(SourceID, spart1, pSkill->iSelfFX1, SourceID, spart1, -1);
	if(spart2!=0) CGameProcedure::s_pFX->TriggerBundle(SourceID, spart2, pSkill->iSelfFX1, SourceID, spart2, -2);

	s_pPlayer->m_iIDTarget = TargetID;			

	//s_pPlayer->ActionMove(PSM_STOP);
	m_pGameProcMain->CommandMove(MD_STOP, true);

	s_pPlayer->m_iMagicAni = pSkill->iSelfAnimID1;
	if(pSkill->dw1stTableType==2 || pSkill->dw2ndTableType==2)
	{
		int LeftItem = s_pPlayer->ItemClass_LeftHand();
		int RightItem = s_pPlayer->ItemClass_RightHand();
		if(RightItem == ITEM_CLASS_BOW_CROSS || LeftItem==ITEM_CLASS_BOW_CROSS)
		{
			s_pPlayer->m_iMagicAni = ANI_SHOOT_QUARREL_A;
		}
	}
	s_pPlayer->m_fCastFreezeTime = 10.0f;
	s_pPlayer->Action(PSA_SPELLMAGIC, false, pTargetPlayer);

	uint8_t byBuff[32];
	int iOffset=0;
	CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)WIZ_MAGIC_PROCESS);
	CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)N3_SP_MAGIC_CASTING);
	CAPISocket::MP_AddDword(byBuff, iOffset, (int)pSkill->dwID);
	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)SourceID);
	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)TargetID);

	CAPISocket::MP_AddShort(byBuff, iOffset, 0);
	CAPISocket::MP_AddShort(byBuff, iOffset, 0);
	CAPISocket::MP_AddShort(byBuff, iOffset, 0);
	
	CAPISocket::MP_AddShort(byBuff, iOffset, 0);
	CAPISocket::MP_AddShort(byBuff, iOffset, 0);
	CAPISocket::MP_AddShort(byBuff, iOffset, 0);
	
	CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보낸다..
	
	if(pSkill->iTarget == SKILLMAGIC_TARGET_ENEMY_ONLY) m_pGameProcMain->PlayBGM_Battle();
	//
	/////////////////////////////////////////////////////////////
}


//
//	내가 캐스팅 중이면 케스팅 처리해야되고, flying효과도 처리..
//
void CMagicSkillMng::Tick()
{
#ifdef _DEBUG
	m_fMsgUpdateTimer += CN3Base::s_fSecPerFrm;
#endif

	for (auto it = m_RecastTimes.begin(); it != m_RecastTimes.end(); )
	{
#ifdef _DEBUG
		if (m_fMsgUpdateTimer >= 0.2f)
		{
			char szMsg[100];
			std::snprintf(szMsg, sizeof(szMsg), "SkillID: %u - %.2f seconds", it->first, it->second);
			m_pGameProcMain->MsgOutput(szMsg, 0xffffff00);
			m_fMsgUpdateTimer = 0.0f;
		}
#endif
		it->second -= CN3Base::s_fSecPerFrm;

		if (it->second < 0)
			it = m_RecastTimes.erase(it);
		else
			++it;
	}

	for (auto it = m_NonActionRecastTimes.begin(); it != m_NonActionRecastTimes.end(); )
	{
#ifdef _DEBUG
		if (m_fMsgUpdateTimer >= 0.2f)
		{
			char szMsg[100];
			std::snprintf(szMsg, sizeof(szMsg), "SkillID: %u - skill %.2f seconds", it->first, it->second);
			m_pGameProcMain->MsgOutput(szMsg, 0xffffff00);
			m_fMsgUpdateTimer = 0.0f;
		}
#endif

		it->second -= CN3Base::s_fSecPerFrm;
		if (it->second < 0)
			it = m_NonActionRecastTimes.erase(it);
		else
			++it;
	}

	// Legacy Existing code for delay and casting
	m_fDelay -= CN3Base::s_fSecPerFrm;
	if (m_fDelay < 0.0f) m_fDelay = 0.0f;
	ProcessCasting();
	//TRACE("skillmagic tick state : %d time %.2f\n", s_pPlayer->State(), CN3Base::TimeGet());

	if (m_fZonePointerRadius > 0.0f)
	{
		// Initially grow radius per tick as per official.
		if (m_fZonePointerRadiusEffective <= m_fZonePointerRadius)
			m_fZonePointerRadiusEffective += 1.0f;

		// Get REAL-TIME mouse position
		m_fZonePointerRotRad += CN3Base::s_fSecPerFrm * D3DXToRadian(50.0f);

		UpdateZonePointerPositions();
	}

	if(m_dwRegionMagicState==2)
	{
		m_dwRegionMagicState = 0;
		CGameProcedure::s_pFX->Stop(s_pPlayer->IDNumber(), s_pPlayer->IDNumber(), m_iMyRegionTargetFXID, m_iMyRegionTargetFXID, true);
		CancelZonePointer();
//		if( !CheckValidDistance(&m_dwRegionSkill, m_pGameProcMain->m_vMouseLBClickedPos, 0) ) return;
//		StartSkillMagicAtPosPacket(&m_dwRegionSkill, m_pGameProcMain->m_vMouseLBClickedPos);		
		if( !CheckValidDistance(&m_dwRegionSkill, m_pGameProcMain->m_vMouseSkillPos, 0) ) return;
		StartSkillMagicAtPosPacket(&m_dwRegionSkill, m_pGameProcMain->m_vMouseSkillPos);		
	}

	if( m_dwCastingStateNonAction == 1)
	{
		m_fCastTimeNonAction -= CN3Base::s_fSecPerFrm;
		if(m_fCastTimeNonAction<0.0f)
		{
			__TABLE_UPC_SKILL* pSkill = s_pTbl_Skill.Find(m_dwNonActionMagicID);
			if(!pSkill) 
			{
				m_dwCastingStateNonAction = 0;
				m_fCastTimeNonAction = 0.0f;
				m_dwNonActionMagicID = 0;
				m_iNonActionMagicTarget = -1;
				return;
			}

			uint8_t byBuff[32];
			int iOffset = 0;
			CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t) WIZ_MAGIC_PROCESS);
			CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t) N3_SP_MAGIC_EFFECTING);
			CAPISocket::MP_AddDword(byBuff, iOffset, (int) m_dwNonActionMagicID);
			CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t) s_pPlayer->IDNumber());
			CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t) m_iNonActionMagicTarget);

			CAPISocket::MP_AddShort(byBuff, iOffset, 0);	//targetpos...
			CAPISocket::MP_AddShort(byBuff, iOffset, 0);
			CAPISocket::MP_AddShort(byBuff, iOffset, 0);

			CAPISocket::MP_AddShort(byBuff, iOffset, 0);
			CAPISocket::MP_AddShort(byBuff, iOffset, 0);
			CAPISocket::MP_AddShort(byBuff, iOffset, 0);
			
			CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보낸다..

			m_dwCastingStateNonAction = 0;
			m_fCastTimeNonAction = 0.0f;
			m_dwNonActionMagicID = 0;
			m_iNonActionMagicTarget = -1;

			//m_fRecastTimeNonAction = (float)(pSkill->iReCastTime) / 10.0f;

			std::string szMsg;
			GetTextF(IDS_SKILL_USE, &szMsg, pSkill->szName.c_str());
			m_pGameProcMain->MsgOutput(szMsg, 0xffffff00);
		}
	}
//	if(s_pPlayer->State()==PSA_SPELLMAGIC) 
}

void CMagicSkillMng::UpdateZonePointerPositions()
{
	// Update all FX positions
	for (int j = 0; j < 8; j++)
	{
		const float fRadians = m_fZonePointerRotRad + (D3DXToRadian(45.0f) * j);

		__Vector3 vNewPos = m_pGameProcMain->m_vMouseSkillPos;
		vNewPos.y = CGameBase::ACT_WORLD->GetHeightWithTerrain(vNewPos.x, vNewPos.z);

		vNewPos.x += cosf(fRadians) * m_fZonePointerRadiusEffective;
		vNewPos.z += sinf(fRadians) * m_fZonePointerRadiusEffective;

		// Update FX position using the index
		CGameProcedure::s_pFX->SetBundlePos(
			FXID_ZONE_POINTER,
			FXID_ZONE_POINTER + j,	// Index
			vNewPos);
	}
}

// When cancelling the skill or changing modes
void CMagicSkillMng::CancelZonePointer()
{
	for (int j = 0; j < 8; j++)
	{
		CGameProcedure::s_pFX->Stop(
			s_pPlayer->IDNumber(),
			0,
			FXID_ZONE_POINTER,
			FXID_ZONE_POINTER + j,
			true);
	}

	m_fZonePointerRadius = -1.0f;
	m_fZonePointerRadiusEffective = 0.0f;
	m_fZonePointerRotRad = 0.0f;
}

void CMagicSkillMng::SuccessCast(__TABLE_UPC_SKILL* pSkill, CPlayerBase* pTarget)
{
	s_pPlayer->m_dwMagicID = 0xffffffff;
	s_pPlayer->m_fCastingTime = 0.0f;

	uint8_t byBuff[32];
	int iOffset=0;
	CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)WIZ_MAGIC_PROCESS);

	int idx = 0;
	if(pSkill->iFlyingFX==0) 
		CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)N3_SP_MAGIC_EFFECTING);
	else
	{
		CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)N3_SP_MAGIC_FLYING);
		if(pSkill->dw1stTableType==2 || pSkill->dw2ndTableType==2)//화살쏘기..
		{
			int iNumArrow = 1;
			__TABLE_UPC_SKILL_TYPE_2* pType2 = m_pTbl_Type_2->Find(pSkill->dwID);
			if(pType2) iNumArrow = pType2->iNumArrow;

			idx = AddIdx(pSkill->dwID, iNumArrow);
		}
		else idx = AddIdx(pSkill->dwID);				
	}

	if(pSkill->dw1stTableType==1 || pSkill->dw2ndTableType==1)
	{		
		//바로 skill로 들어가..^^
		//casting packet은 보내지 않고..바로 effect packet을 보낸다..

		//기술 애니메이션 드가...=^^=
		//효과있으면 같이 드가..
		__TABLE_UPC_SKILL_TYPE_1* pType1 = m_pTbl_Type_1->Find(pSkill->dwID);
		if(!pType1) return;

		s_pPlayer->RotateTo(pTarget);

		for(int i=0;i<pType1->iNumCombo;i++)
		{
			bool bImmediately = ((0 == i) ? true : false); // 처음건 바로 넣는다..
			s_pPlayer->AnimationAdd((const e_Ani)pType1->iAct[i], bImmediately);				
		}			
		
		std::string szMsg;
		GetTextF(IDS_SKILL_USE, &szMsg, pSkill->szName.c_str());
		m_pGameProcMain->MsgOutput(szMsg, 0xffffff00);

		uint8_t byBuff[32];
		int iOffset = 0;
		//CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)WIZ_MAGIC_PROCESS);
		//CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)N3_SP_MAGIC_EFFECTING);
		CAPISocket::MP_AddDword(byBuff, iOffset, (int) pSkill->dwID);
		CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t) s_pPlayer->IDNumber());
		CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t) m_iTarget);

		CAPISocket::MP_AddShort(byBuff, iOffset, 1);
		CAPISocket::MP_AddShort(byBuff, iOffset, 1);
		CAPISocket::MP_AddShort(byBuff, iOffset, 0);

		CAPISocket::MP_AddShort(byBuff, iOffset, 0);
		CAPISocket::MP_AddShort(byBuff, iOffset, 0);
		CAPISocket::MP_AddShort(byBuff, iOffset, 0);

		CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보낸다..				
		SetSkillCooldown(pSkill);
	}
	else
	{
		std::string szMsg;
		GetTextF(IDS_SKILL_USE, &szMsg, pSkill->szName.c_str());
		m_pGameProcMain->MsgOutput(szMsg, 0xffffff00);
		//m_fRecastTime = (float) pSkill->iReCastTime / 10.0f;
		m_fDelay = 0.3f;

		CAPISocket::MP_AddDword(byBuff, iOffset, (int) pSkill->dwID);
		CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t) s_pPlayer->IDNumber());
		CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t) m_iTarget);

		CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t) m_vTargetPos.x);
		CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t) m_vTargetPos.y);
		CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t) m_vTargetPos.z);

		CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t) idx);//flying이라면 idx...
		CAPISocket::MP_AddShort(byBuff, iOffset, 0);
		CAPISocket::MP_AddShort(byBuff, iOffset, 0);

		CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보낸다..
		SetSkillCooldown(pSkill);

		if (pSkill->iFlyingFX != 0)
		{
			////////////////////////////////////////////////////
			//flying처리하기..	
			int SourceID = s_pPlayer->IDNumber();

			s_pPlayer->m_iMagicAni = pSkill->iSelfAnimID2;
			if(pSkill->dw1stTableType == 2 || pSkill->dw2ndTableType == 2)
			{
				m_fDelay = 0.1f;
				int LeftItem = s_pPlayer->ItemClass_LeftHand();
				int RightItem = s_pPlayer->ItemClass_RightHand();
				if (RightItem == ITEM_CLASS_BOW_CROSS || LeftItem == ITEM_CLASS_BOW_CROSS)
				{
					s_pPlayer->m_iMagicAni = ANI_SHOOT_QUARREL_B;
				}
			}
			s_pPlayer->m_fCastFreezeTime = 0.0f;
			s_pPlayer->Action(PSA_SPELLMAGIC, false);
			//s_pPlayer->Action(PSA_BASIC, false);

			CGameProcedure::s_pFX->Stop(SourceID, SourceID, pSkill->iSelfFX1, -1, true);
			CGameProcedure::s_pFX->Stop(SourceID, SourceID, pSkill->iSelfFX1, -2, true);

			if(pSkill->dw1stTableType == 2 || pSkill->dw2ndTableType == 2)//화살쏘기..
			{
				int16_t Data[6] = { (int16_t)m_vTargetPos.x, (int16_t)m_vTargetPos.y, (int16_t)m_vTargetPos.z, (int16_t)idx, 0, 0 };
				FlyingType2(pSkill, SourceID, m_iTarget, Data);
				return;
			}

			CPlayerBase* pTarget = m_pGameProcMain->CharacterGetByID(m_iTarget, false);
			int spart1 = pSkill->iSelfPart1 % 1000;
			if(!pTarget)
			{
				__Vector3 vTargetPos = s_pPlayer->Position() + s_pPlayer->Direction();
				CGameProcedure::s_pFX->TriggerBundle(SourceID, spart1, pSkill->iFlyingFX, m_vTargetPos,
															idx, FX_BUNDLE_MOVE_DIR_FIXEDTARGET);
			}
			else
			{
				CGameProcedure::s_pFX->TriggerBundle(SourceID, spart1, pSkill->iFlyingFX, m_iTarget,
												0, idx, FX_BUNDLE_MOVE_DIR_FLEXABLETARGET);
			}
			//
			////////////////////////////////////////////////////
		}
		else
		{
			if(pSkill->iSelfAnimID2>=0)
			{
				s_pPlayer->m_iMagicAni = pSkill->iSelfAnimID2;
				s_pPlayer->m_fCastFreezeTime = 0.0f;
				s_pPlayer->Action(PSA_SPELLMAGIC, false);
				//s_pPlayer->Action(PSA_BASIC, false);				
			}
		}

		if(pSkill->iSelfFX2>0)
			CGameProcedure::s_pFX->TriggerBundle(s_pPlayer->IDNumber(), pSkill->iSelfPart2, pSkill->iSelfFX2, s_pPlayer->IDNumber(), pSkill->iSelfPart2, -3);
	}
}

void CMagicSkillMng::FailCast(__TABLE_UPC_SKILL* pSkill)
{
	s_pPlayer->m_dwMagicID = 0xffffffff;
	s_pPlayer->m_fCastingTime = 0.0f;

	uint8_t byBuff[32];
	int iOffset=0;
	CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)WIZ_MAGIC_PROCESS);
	CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)N3_SP_MAGIC_FAIL);
	CAPISocket::MP_AddDword(byBuff, iOffset, (int)pSkill->dwID);
	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)s_pPlayer->IDNumber());
	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)s_pPlayer->IDNumber());

	CAPISocket::MP_AddShort(byBuff, iOffset, 0);
	CAPISocket::MP_AddShort(byBuff, iOffset, 0);
	CAPISocket::MP_AddShort(byBuff, iOffset, 0);

	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)SKILLMAGIC_FAIL_CASTING);
	CAPISocket::MP_AddShort(byBuff, iOffset, 0);
	CAPISocket::MP_AddShort(byBuff, iOffset, 0);

	CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보낸다..	
}

void CMagicSkillMng::ProcessCasting()
{
	//캐스팅 처리..
	if(s_pPlayer->m_dwMagicID != 0xffffffff)
	{
		__TABLE_UPC_SKILL* pSkill = s_pTbl_Skill.Find(s_pPlayer->m_dwMagicID);

		CPlayerBase* pTarget = m_pGameProcMain->CharacterGetByID(m_iTarget, true);
		if(pTarget) s_pPlayer->RotateTo(pTarget); // 일단 타겟을 향해 방향을 돌린다..

		//캐스팅 성공적으로 완료...
		float fCastingTime = ((float)pSkill->iCastTime) / 10.0f * s_pPlayer->m_fAttackDelta;

		if(pSkill)
		{
			bool bSuccess = false;
			if( s_pPlayer->m_fCastingTime >= fCastingTime && s_pPlayer->State()==PSA_SPELLMAGIC && s_pPlayer->StateMove()==PSM_STOP)
			{
				SuccessCast(pSkill, pTarget);
				bSuccess = true;
			}
			
			//캐스팅 실패...
			if(bSuccess == false && (s_pPlayer->State()!=PSA_SPELLMAGIC || s_pPlayer->StateMove()!=PSM_STOP))
				FailCast(pSkill);
		}
		else s_pPlayer->m_dwMagicID = 0xffffffff;
	}	
}

void CMagicSkillMng::MobCasting(__TABLE_UPC_SKILL* pSkill, int iSourceID)
{
	if(!pSkill) return;

	//캐스팅 성공적으로 완료...
	uint8_t byBuff[32];
	int iOffset=0;
	CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)WIZ_MAGIC_PROCESS);

	int idx = 0;
	if(pSkill->iFlyingFX==0) 
		CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)N3_SP_MAGIC_EFFECTING);
	else
	{
		CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)N3_SP_MAGIC_FLYING);
		if(pSkill->dw1stTableType==2 || pSkill->dw2ndTableType==2)//화살쏘기..
		{
			int iNumArrow = 1;
			__TABLE_UPC_SKILL_TYPE_2* pType2 = m_pTbl_Type_2->Find(pSkill->dwID);
			if(pType2) iNumArrow = pType2->iNumArrow;

			idx = AddIdx(pSkill->dwID, iNumArrow);
		}
		else idx = AddIdx(pSkill->dwID);
	}

	CAPISocket::MP_AddDword(byBuff, iOffset, (int)pSkill->dwID);
	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)iSourceID);
	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)s_pPlayer->IDNumber());

	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)m_vTargetPos.x);
	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)m_vTargetPos.y);
	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)m_vTargetPos.z);

	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)idx);//flying이라면 idx...
	CAPISocket::MP_AddShort(byBuff, iOffset, 0);
	CAPISocket::MP_AddShort(byBuff, iOffset, 0);

	CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보낸다..
}


void CMagicSkillMng::MsgRecv_Casting(Packet& pkt)
{
	////common.....//////////////////////////////////////////////////////////////
	//	
	uint32_t dwMagicID = pkt.read<uint32_t>();
	int	iSourceID = pkt.read<int16_t>();
	int	iTargetID = pkt.read<int16_t>();

	int16_t Data[6];
	for(int i=0;i<6;i++)
	{
		Data[i] = pkt.read<int16_t>();
	}

	//if(iSourceID<0) return;
	if(iSourceID<0 || iSourceID==s_pPlayer->IDNumber()) return;
	//	
	////common.....//////////////////////////////////////////////////////////////
	
	__Vector3	vTargetPos;
	if(iTargetID == -1) vTargetPos.Set((float)Data[0], (float)Data[1], (float)Data[2]);
	
	CPlayerBase* pPlayer = m_pGameProcMain->CharacterGetByID(iSourceID, true);
	if(!pPlayer) return;

	__TABLE_UPC_SKILL* pSkill = s_pTbl_Skill.Find(dwMagicID);
	if(!pSkill) return;

	//내가 쓸때...
	if(iSourceID==s_pPlayer->IDNumber())
	{
		m_pGameProcMain->CommandSitDown(false, false); // 혹시라도 앉아있음 일으켜 세운다..
		
		s_pPlayer->m_dwMagicID = dwMagicID;
		s_pPlayer->m_fCastingTime = 0.0f;
		m_iTarget = iTargetID;
		m_vTargetPos = vTargetPos;
	}
	
	//몬스터가 나를 향해 쏠때...
	if( s_pOPMgr->NPCGetByID(iSourceID, true) )
	{
		pPlayer->RotateTo((CPlayerBase*)s_pPlayer); // 이넘을 바라본다.
		pPlayer->m_iIDTarget = iTargetID;
		pPlayer->ActionMove(PSM_STOP);
		pPlayer->m_iMagicAni = pSkill->iSelfAnimID1;
		pPlayer->Action(PSA_ATTACK, false, (CPlayerBase*)s_pPlayer);

		if(iTargetID==s_pPlayer->IDNumber()) 
			MobCasting(pSkill, iSourceID);
		return;
	}

	CPlayerBase* pTargetPlayer = m_pGameProcMain->CharacterGetByID(iTargetID, false);
		
	int spart1 = pSkill->iSelfPart1 % 1000;
	int spart2 = pSkill->iSelfPart1 / 1000;
	spart2 = abs(spart2);

	CGameProcedure::s_pFX->TriggerBundle(iSourceID, spart1, pSkill->iSelfFX1, iSourceID, spart1, -1);
	if(spart2!=0) CGameProcedure::s_pFX->TriggerBundle(iSourceID, spart2, pSkill->iSelfFX1, iSourceID, spart2, -2);

	pPlayer->m_iIDTarget = iTargetID;			

	pPlayer->ActionMove(PSM_STOP);
	pPlayer->m_iMagicAni = pSkill->iSelfAnimID1;
	if(pSkill->dw1stTableType==2 || pSkill->dw2ndTableType==2)
	{
		int LeftItem = pPlayer->ItemClass_LeftHand();
		int RightItem = pPlayer->ItemClass_RightHand();
		if(RightItem == ITEM_CLASS_BOW_CROSS || LeftItem==ITEM_CLASS_BOW_CROSS)
		{
			pPlayer->m_iMagicAni = ANI_SHOOT_QUARREL_A;
		}
	}
	pPlayer->m_fCastFreezeTime = 10.0f;
	pPlayer->Action(PSA_SPELLMAGIC, false, pTargetPlayer);

	//CLogWriter::Write("send casting : %.4f", CN3Base::TimeGet());
	//TRACE("recv casting : %.4f\n", CN3Base::TimeGet());

	if(pSkill->iTarget == SKILLMAGIC_TARGET_ENEMY_ONLY) m_pGameProcMain->PlayBGM_Battle();
}

void CMagicSkillMng::MsgRecv_Flying(Packet& pkt)
{
	////common.....//////////////////////////////////////////////////////////////
	//	
	uint32_t dwMagicID = pkt.read<uint32_t>();
	int	iSourceID = pkt.read<int16_t>();
	int	iTargetID = pkt.read<int16_t>();

	int16_t Data[6];
	for(int i=0;i<6;i++)
	{
		Data[i] = pkt.read<int16_t>();
	}

	//if(iSourceID<0) return;
	if(iSourceID<0 || iSourceID==s_pPlayer->IDNumber()) return;

	CPlayerBase* pPlayer = m_pGameProcMain->CharacterGetByID(iSourceID, true);
	if(!pPlayer) return;

	__TABLE_UPC_SKILL* pSkill = s_pTbl_Skill.Find(dwMagicID);
	if(!pSkill) return;
	//
	////common.....//////////////////////////////////////////////////////////////

	//TRACE("recv flying : %.4f\n", CN3Base::TimeGet());

	if(pPlayer && pPlayer->State()==PSA_SPELLMAGIC)
	{
		pPlayer->m_iMagicAni = pSkill->iSelfAnimID2;
		if(pSkill->dw1stTableType==2 || pSkill->dw2ndTableType==2)
		{
			int LeftItem = pPlayer->ItemClass_LeftHand();
			int RightItem = pPlayer->ItemClass_RightHand();
			if(RightItem == ITEM_CLASS_BOW_CROSS || LeftItem==ITEM_CLASS_BOW_CROSS)
			{
				pPlayer->m_iMagicAni = ANI_SHOOT_QUARREL_B;
			}
		}
		pPlayer->m_fCastFreezeTime = 0.0f;
		pPlayer->Action(PSA_SPELLMAGIC, false);
	}

	CGameProcedure::s_pFX->Stop(iSourceID, iSourceID, pSkill->iSelfFX1, -1, true);
	CGameProcedure::s_pFX->Stop(iSourceID, iSourceID, pSkill->iSelfFX1, -2, true);

	if(pSkill->dw1stTableType==2 || pSkill->dw2ndTableType==2)//화살쏘기..
	{
		FlyingType2(pSkill, iSourceID, iTargetID, Data);
		return;
	}

	CPlayerBase* pTarget = m_pGameProcMain->CharacterGetByID(iTargetID, false);

	int spart1 = pSkill->iSelfPart1 % 1000;
	
	if(!pTarget)
	{
		if(pPlayer)
		{
			__Vector3 vTargetPos = pPlayer->Position() + pPlayer->Direction();
			CGameProcedure::s_pFX->TriggerBundle(iSourceID, spart1, pSkill->iFlyingFX, vTargetPos,
													Data[3], FX_BUNDLE_MOVE_DIR_FIXEDTARGET);
		}
	}
	else
	{
		CGameProcedure::s_pFX->TriggerBundle(iSourceID, spart1, pSkill->iFlyingFX, iTargetID,
										0/*pSkill->iTargetPart*/, Data[3], FX_BUNDLE_MOVE_DIR_FLEXABLETARGET);
	}
}

void CMagicSkillMng::MsgRecv_Effecting(Packet& pkt)
{
	////common.....//////////////////////////////////////////////////////////////
	//
	uint32_t dwMagicID = pkt.read<uint32_t>();
	int	iSourceID = pkt.read<int16_t>();
	int	iTargetID = pkt.read<int16_t>();

	int16_t Data[6];
	for(int i=0;i<6;i++)
	{
		Data[i] = pkt.read<int16_t>();
	}

	CPlayerBase* pPlayer = m_pGameProcMain->CharacterGetByID(iSourceID, false);
	if(!pPlayer) return;

	__TABLE_UPC_SKILL* pSkill = s_pTbl_Skill.Find(dwMagicID);
	if(!pSkill) return;
	//
	////common.....//////////////////////////////////////////////////////////////
		
	if(pPlayer && iSourceID!=s_pPlayer->IDNumber() && pPlayer->State()==PSA_SPELLMAGIC)
	{
		pPlayer->m_iMagicAni = pSkill->iSelfAnimID2;	//화살놓는 동작...
		pPlayer->m_fCastFreezeTime = 0.0f;
		pPlayer->Action(PSA_SPELLMAGIC, false);
	}

	CGameProcedure::s_pFX->Stop(iSourceID, iSourceID, pSkill->iSelfFX1, -1, true);
	CGameProcedure::s_pFX->Stop(iSourceID, iSourceID, pSkill->iSelfFX1, -2, true);

	if(pSkill->dw1stTableType==1 || pSkill->dw2ndTableType==1)	//	타입1인경우 걍 스킬이야..콤보도 껴있어..좀 특별하게 관리해야돼..
	{
		if(!EffectingType1(dwMagicID, iSourceID, iTargetID, Data)) return;
	}

	if(pSkill->dw1stTableType==4 || pSkill->dw2ndTableType==4)
	{
		if(iTargetID==s_pPlayer->IDNumber()) EffectingType4(dwMagicID);
	}

	if(pSkill->dw1stTableType==3 || pSkill->dw2ndTableType==3)
	{
		if(iTargetID==s_pPlayer->IDNumber()) EffectingType3(dwMagicID);
	}

	if(pSkill->iFlyingFX!=0 && 
		(pSkill->iTarget < SKILLMAGIC_TARGET_AREA_ENEMY || pSkill->iTarget > SKILLMAGIC_TARGET_AREA) ) return;	//플라잉이 있는 마법의 경우는 효과를 fail에서 처리한다..
	
	if(iTargetID==-1)
	{
		__Vector3 vTargetPos(Data[0],Data[1],Data[2]);
		CGameProcedure::s_pFX->TriggerBundle(iSourceID, 0, pSkill->iTargetFX, vTargetPos);
	}
	else
		CGameProcedure::s_pFX->TriggerBundle(iSourceID, 0, pSkill->iTargetFX, iTargetID, pSkill->iTargetPart);
}

void CMagicSkillMng::MsgRecv_Fail(Packet& pkt)
{
	////common.....//////////////////////////////////////////////////////////////
	//
	uint32_t dwMagicID = pkt.read<uint32_t>();
	int	iSourceID = pkt.read<int16_t>();
	int	iTargetID = pkt.read<int16_t>();

	int16_t Data[6];
	for(int i=0;i<6;i++)
	{
		Data[i] = pkt.read<int16_t>();
	}
	CPlayerBase* pPlayer = m_pGameProcMain->CharacterGetByID(iSourceID, false);
	if(!pPlayer) return;

	__TABLE_UPC_SKILL* pSkill = s_pTbl_Skill.Find(dwMagicID);
	if(!pSkill) return;
	//
	////common.....//////////////////////////////////////////////////////////////
		
	if(pPlayer && iSourceID != s_pPlayer->IDNumber() && pPlayer->State()==PSA_SPELLMAGIC) 
	{
		pPlayer->m_iMagicAni = pSkill->iSelfAnimID2;
		pPlayer->m_fCastFreezeTime = 0.0f;
		pPlayer->Action(PSA_SPELLMAGIC, false);

		CGameProcedure::s_pFX->Stop(iSourceID, iSourceID, pSkill->iSelfFX1, -1, true);
		CGameProcedure::s_pFX->Stop(iSourceID, iSourceID, pSkill->iSelfFX1, -2, true);
	}

	if(Data[3]==SKILLMAGIC_FAIL_ATTACKZERO)
	{
		CGameProcedure::s_pFX->Stop(iSourceID, iSourceID, pSkill->iSelfFX1, -1, true);
		CGameProcedure::s_pFX->Stop(iSourceID, iSourceID, pSkill->iSelfFX1, -2, true);
		
		if(iSourceID == s_pPlayer->IDNumber())
		{
			s_pPlayer->m_dwMagicID = 0xffffffff;
			m_pGameProcMain->CommandSitDown(false, false); 

			std::string szMsg;
			GetTextF(
				IDS_MSG_FMT_TARGET_ATTACK_FAILED,
				&szMsg,
				pSkill->szName.c_str());
			m_pGameProcMain->MsgOutput(szMsg, 0xffff3b3b);			
		}
		return;
	}

	if(Data[3]==SKILLMAGIC_FAIL_NOEFFECT)
	{
		CGameProcedure::s_pFX->Stop(iSourceID, iSourceID, pSkill->iSelfFX1, -1, true);
		CGameProcedure::s_pFX->Stop(iSourceID, iSourceID, pSkill->iSelfFX1, -2, true);
		
		if (pPlayer != nullptr)
			pPlayer->Action(PSA_BASIC, true);

		if(iSourceID == s_pPlayer->IDNumber())
		{
			s_pPlayer->m_dwMagicID = 0xffffffff;
			m_pGameProcMain->CommandSitDown(false, false); // 혹시라도 앉아있음 일으켜 세운다..

			std::string szMsg;
			GetTextF(
				IDS_SKILL_FAIL_EFFECTING,
				&szMsg,
				pSkill->szName.c_str());
			m_pGameProcMain->MsgOutput(szMsg, 0xffff3b3b);			
		}
		return;
	}	

	if(Data[3]==SKILLMAGIC_FAIL_CASTING)//	캐스팅 실패인 것이다..
	{
		CGameProcedure::s_pFX->Stop(iSourceID, iSourceID, pSkill->iSelfFX1, -1, true);
		CGameProcedure::s_pFX->Stop(iSourceID, iSourceID, pSkill->iSelfFX1, -2, true);
		
		if (pPlayer != nullptr)
			pPlayer->Action(PSA_BASIC, true);

		if(iSourceID == s_pPlayer->IDNumber())
		{
			s_pPlayer->m_dwMagicID = 0xffffffff;
			m_pGameProcMain->CommandSitDown(false, false); // 혹시라도 앉아있음 일으켜 세운다..
	
			std::string szMsg;
			GetTextF(IDS_SKILL_FAIL_CASTING, &szMsg);
			m_pGameProcMain->MsgOutput(szMsg, 0xffff3b3b);
		}
		return;
	}

	if(Data[3]==SKILLMAGIC_FAIL_KILLFLYING)//flying효과 죽이고..그자리에 타겟효과 해라..	
	{
		if(iSourceID == s_pPlayer->IDNumber() ||
			((iTargetID==s_pPlayer->IDNumber() && s_pOPMgr->NPCGetByID(iSourceID, false)!=NULL)))
		{
			RemoveIdx(Data[4]);
		}
		else CGameProcedure::s_pFX->Stop(iSourceID, iTargetID, pSkill->iFlyingFX, Data[4]);

		if(iTargetID==s_pPlayer->IDNumber())
		{
			if(pSkill->dw1stTableType==2 || pSkill->dw2ndTableType==2)
			{
				StopCastingByRatio();
			}
			else if(pSkill->dw1stTableType==3 || pSkill->dw2ndTableType==3)
			{
				__TABLE_UPC_SKILL_TYPE_3* pType3 = m_pTbl_Type_3->Find(dwMagicID);

				if(pType3->iAttribute==3)
				{
					StopCastingByRatio();
					StunMySelf(pType3);
				}
			}
		}

		CPlayerBase* pTarget = m_pGameProcMain->CharacterGetByID(iTargetID, false);
		if(pTarget)
		{
			CGameProcedure::s_pFX->TriggerBundle(iSourceID, pSkill->iTargetPart, pSkill->iTargetFX, iTargetID, pSkill->iTargetPart);
			
		}
		else
		{
			__Vector3 TargetPos(Data[0], Data[1], Data[2]);
			CGameProcedure::s_pFX->TriggerBundle(iSourceID, 0, pSkill->iTargetFX, TargetPos);
		}		
		return;
	}

	//그외 stop이 필요한게 있을꺼야..
	//그땐...
	//CGameProcedure::s_pFX->Stop(iSourceID, iTargetID, pSkill->iSelfFX, 0);
}


//type4 해제..
void CMagicSkillMng::MsgRecv_BuffType(Packet& pkt)
{
	int iBuffType = pkt.read<uint8_t>();
	__InfoPlayerBase* pInfoBase = &(s_pPlayer->m_InfoBase);
	__InfoPlayerMySelf* pInfoExt = &(s_pPlayer->m_InfoExt);

	std::multimap<int, uint32_t>::iterator it = m_ListBuffTypeID.find(iBuffType);
	if(it!= m_ListBuffTypeID.end())
	{
		__TABLE_UPC_SKILL* pSkill = s_pTbl_Skill.Find(it->second);
		m_pGameProcMain->m_pUIStateBarAndMiniMap->DelMagic(pSkill);
		m_ListBuffTypeID.erase(it);
	}

	switch(iBuffType)
	{
	case BUFFTYPE_MAXHP:
		if(pInfoBase->iHP < pInfoBase->iHPMax)
		{
			pInfoBase->iHPMax -= m_iMaxHP;
			m_pGameProcMain->m_pUIVar->m_pPageState->UpdateHP(pInfoBase->iHP, pInfoBase->iHPMax);
		}
		m_iMaxHP = 0;
		break;
	case BUFFTYPE_AC:
		pInfoExt->iGuard_Delta -= m_iAC;
		m_pGameProcMain->m_pUIVar->m_pPageState->UpdateGuardPoint(pInfoExt->iGuard, pInfoExt->iGuard_Delta);
		m_iAC = 0;
		break;
	case BUFFTYPE_ATTACK:
		if(m_iAttack) pInfoExt->iAttack_Delta -= m_iAttack;
		m_pGameProcMain->m_pUIVar->m_pPageState->UpdateAttackPoint(pInfoExt->iAttack, pInfoExt->iAttack_Delta);
		m_iAttack = 0;
		break;
	case BUFFTYPE_ATTACKSPEED:
		s_pPlayer->m_fAttackDelta /= m_fAttackSpeed; 
		m_fAttackSpeed = 1.0f;
		break;
	case BUFFTYPE_SPEED:
		s_pPlayer->m_fMoveDelta /= m_fSpeed;
		m_fSpeed = 1.0f;
		//TRACE("스프린트 해제. MoveDelta = %f\n", s_pPlayer->m_fMoveDelta);
		break;
	case BUFFTYPE_ABILITY:
		pInfoExt->iStrength_Delta -= m_iStr;
		pInfoExt->iStamina_Delta -= m_iSta;
		pInfoExt->iDexterity_Delta -= m_iDex;
		pInfoExt->iIntelligence_Delta -= m_iInt;
		pInfoExt->iMagicAttak_Delta -= m_iMAP;

		m_pGameProcMain->m_pUIVar->m_pPageState->UpdateStrength(pInfoExt->iStrength, pInfoExt->iStrength_Delta);
		m_pGameProcMain->m_pUIVar->m_pPageState->UpdateStamina(pInfoExt->iStamina, pInfoExt->iStamina_Delta);
		m_pGameProcMain->m_pUIVar->m_pPageState->UpdateDexterity(pInfoExt->iDexterity, pInfoExt->iDexterity_Delta);
		m_pGameProcMain->m_pUIVar->m_pPageState->UpdateIntelligence(pInfoExt->iIntelligence, pInfoExt->iIntelligence_Delta);
		m_pGameProcMain->m_pUIVar->m_pPageState->UpdateMagicAttak(pInfoExt->iMagicAttak, pInfoExt->iMagicAttak_Delta);

		m_iStr = 0;
		m_iSta = 0;
		m_iDex = 0;
		m_iInt = 0;
		m_iMAP = 0;
		break;
	case BUFFTYPE_RESIST:
		pInfoExt->iRegistFire_Delta -= m_iFireR;
		pInfoExt->iRegistCold_Delta -= m_iColdR;
		pInfoExt->iRegistLight_Delta -= m_iLightningR;
		pInfoExt->iRegistMagic_Delta -= m_iMagicR;
		pInfoExt->iRegistCurse_Delta -= m_iDeseaseR;
		pInfoExt->iRegistPoison_Delta -= m_iPoisonR;

		m_pGameProcMain->m_pUIVar->m_pPageState->UpdateRegistFire(pInfoExt->iRegistFire, pInfoExt->iRegistFire_Delta);
		m_pGameProcMain->m_pUIVar->m_pPageState->UpdateRegistCold(pInfoExt->iRegistCold, pInfoExt->iRegistCold_Delta);
		m_pGameProcMain->m_pUIVar->m_pPageState->UpdateRegistLight(pInfoExt->iRegistLight, pInfoExt->iRegistLight_Delta);
		m_pGameProcMain->m_pUIVar->m_pPageState->UpdateRegistMagic(pInfoExt->iRegistMagic, pInfoExt->iRegistMagic_Delta);
		m_pGameProcMain->m_pUIVar->m_pPageState->UpdateRegistCurse(pInfoExt->iRegistCurse, pInfoExt->iRegistCurse_Delta);
		m_pGameProcMain->m_pUIVar->m_pPageState->UpdateRegistPoison(pInfoExt->iRegistPoison, pInfoExt->iRegistPoison_Delta);

		m_iFireR = 0;
		m_iColdR = 0;
		m_iLightningR = 0;
		m_iMagicR = 0;
		m_iDeseaseR = 0;
		m_iPoisonR = 0;
		break;
	}
}


//
//
//
void CMagicSkillMng::FlyingType2(__TABLE_UPC_SKILL* pSkill, int iSourceID, int iTargetID, int16_t* pData)
{
	CPlayerBase* pPlayer = m_pGameProcMain->CharacterGetByID(iSourceID, true);
	if(!pPlayer) return;
	__TABLE_UPC_SKILL_TYPE_2* pType2 = m_pTbl_Type_2->Find(pSkill->dwID);
	if(!pType2) return;

	int LeftItem = pPlayer->ItemClass_LeftHand()/10;
	int RightItem = pPlayer->ItemClass_RightHand()/10;

	if(LeftItem == (ITEM_CLASS_BOW/10))//활이닷..
	{
		CN3Base::s_SndMgr.PlayOnceAndRelease(ID_SOUND_SKILL_THROW_ARROW, &(pPlayer->Position()));
	}
	else if(RightItem == (ITEM_CLASS_JAVELIN/10))//투창이닷...pla
	{
	}
	
	int spart1 = pSkill->iSelfPart1 % 1000;

	__Vector3 vTargetPos(0,0,0);

	CPlayerBase* pTarget = m_pGameProcMain->CharacterGetByID(iTargetID, false);
	if(!pTarget)
	{
		vTargetPos = pPlayer->Position() + pPlayer->Direction();
		CGameProcedure::s_pFX->TriggerBundle(iSourceID, spart1, pSkill->iFlyingFX, vTargetPos+pPlayer->Position(), pData[3], pType2->iSuccessType);
		
		int NumArrow = (pType2->iNumArrow - 1) >>1;
		int idx = pData[3];
		
		__Vector3 vTargetPos2 = vTargetPos - pPlayer->Position();
		__Vector3 vTargetPos3;
		__Matrix44 mtx;
		for(int i=1;i<=NumArrow;i++)
		{
			float fAng = (__PI * (float)i) / 12.0f;	// 15도 씩 나눠서...
				
			mtx.Identity();
			mtx.RotationY(-fAng);
			vTargetPos3 = vTargetPos2*mtx;
			CGameProcedure::s_pFX->TriggerBundle(iSourceID, spart1, pSkill->iFlyingFX, vTargetPos3+pPlayer->Position(), idx++, pType2->iSuccessType);

			mtx.Identity();
			mtx.RotationY(fAng);
			vTargetPos3 = vTargetPos2*mtx;
			CGameProcedure::s_pFX->TriggerBundle(iSourceID, spart1, pSkill->iFlyingFX, vTargetPos3+pPlayer->Position(), idx++, pType2->iSuccessType);
		}
	}
	else
	{
		vTargetPos = pTarget->Center();

		if(pType2->iSuccessType == FX_BUNDLE_MOVE_DIR_FIXEDTARGET)
		{
			CGameProcedure::s_pFX->TriggerBundle(iSourceID, spart1, pSkill->iFlyingFX, vTargetPos, pData[3], FX_BUNDLE_MOVE_DIR_FIXEDTARGET);
					
			int NumArrow = (pType2->iNumArrow - 1) >>1;

			int idx = pData[3];

			__Vector3 vTargetPos2 = vTargetPos - pPlayer->Position();
			__Vector3 vTargetPos3;
			__Matrix44 mtx;
			for(int i=1;i<=NumArrow;i++)
			{
				float fAng = (__PI * (float)i) / 12.0f;	// 15도 씩 나눠서...
					
				mtx.Identity();
				mtx.RotationY(-fAng);
				vTargetPos3 = vTargetPos2*mtx;
				CGameProcedure::s_pFX->TriggerBundle(iSourceID, spart1, pSkill->iFlyingFX, vTargetPos3+pPlayer->Position(), idx++, FX_BUNDLE_MOVE_DIR_FIXEDTARGET);
				
				mtx.Identity();
				mtx.RotationY(fAng);
				vTargetPos3 = vTargetPos2*mtx;
				CGameProcedure::s_pFX->TriggerBundle(iSourceID, spart1, pSkill->iFlyingFX, vTargetPos3+pPlayer->Position(), idx++, FX_BUNDLE_MOVE_DIR_FIXEDTARGET);
			}
		}

		else 
		{
			CGameProcedure::s_pFX->TriggerBundle(iSourceID, spart1, pSkill->iFlyingFX, iTargetID, pSkill->iTargetPart, pData[3], pType2->iSuccessType);
					
			int NumArrow = (pType2->iNumArrow - 1) >>1;

			int idx = pData[3];

			__Vector3 vTargetPos2 = vTargetPos - pPlayer->Position();
			__Vector3 vTargetPos3;
			__Matrix44 mtx;
			for(int i=1;i<=NumArrow;i++)
			{
				float fAng = (__PI * (float)i) / 12.0f;	// 15도 씩 나눠서...
					
				mtx.Identity();
				mtx.RotationY(-fAng);
				vTargetPos3 = vTargetPos2*mtx;
				CGameProcedure::s_pFX->TriggerBundle(iSourceID, spart1, pSkill->iFlyingFX, vTargetPos3+pPlayer->Position(), idx++, pType2->iSuccessType);


				mtx.Identity();
				mtx.RotationY(fAng);
				vTargetPos3 = vTargetPos2*mtx;
				CGameProcedure::s_pFX->TriggerBundle(iSourceID, spart1, pSkill->iFlyingFX, vTargetPos3+pPlayer->Position(), idx++, pType2->iSuccessType);
			}
		}
	}

	//__TABLE_UPC_SKILL_TYPE_2* pType2 = m_pTbl_Type_2->Find(pSkill->dwID);
	//if(pType2)	CGameProcedure::s_pFX->Trigger(iSourceID, spart1, pSkill->iFlyingFX, iTargetID, 	pSkill->iTargetPart, pData[3], pType2->iSuccessType);
}


//
//
//

bool CMagicSkillMng::EffectingType1(uint32_t dwMagicID, int iSourceID, int iTargetID, int16_t* pData)
{
	CPlayerBase* pTarget = m_pGameProcMain->CharacterGetByID(iTargetID, false);
	if(pTarget)
	{
		if(iSourceID != s_pPlayer->IDNumber()) // 내가 스킬을 쓸때..
		{
			__TABLE_UPC_SKILL_TYPE_1* pType1 = m_pTbl_Type_1->Find(dwMagicID);
			if(pType1)
			{
				CPlayerBase* pPlayer = m_pGameProcMain->CharacterGetByID(iSourceID, true);
				__ASSERT(pPlayer, "NULL Player Pointer!!");
				if(pPlayer)
				{
					// 검기 색을 바꾸어 준다..
//					__TABLE_UPC_SKILL* pSkill = s_pTbl_Skill.Find(dwMagicID);
//					D3DCOLOR crTrace = TraceColorGet(pSkill); // 스킬의 종류에 따라 검기의 색을 정한다..
//					pPlayer->PlugTraceColorRemake(crTrace); // 검기 색 적용..

					pPlayer->RotateTo(pTarget);

					// TODO: Update this. It's intended to apply based on the equipped weapon type,
					// and rather than manually queue, this behaviour will be passed through Action().
					e_Ani eAni = (e_Ani) pType1->iAct[0];
					pPlayer->Action(PSA_SPELLMAGIC, false, pTarget, false);
					pPlayer->AnimationAdd(eAni, true);
				}
			}
		}
	}
	return true;
}

void CMagicSkillMng::EffectingType3(uint32_t dwMagicID)
{
	__TABLE_UPC_SKILL_TYPE_3* pType3 = m_pTbl_Type_3->Find(dwMagicID);
	__ASSERT(pType3, "NULL type3 Pointer!!");
	if(!pType3) return;

	StunMySelf(pType3);

	int key = 0;
	if(pType3->iStartDamage>0 || (pType3->iStartDamage==0 && pType3->iDuraDamage>0) ) key = DDTYPE_TYPE3_DUR_OUR;
	else key = DDTYPE_TYPE3_DUR_ENEMY;

	if(key==DDTYPE_TYPE3_DUR_ENEMY && pType3->iAttribute==3) StopCastingByRatio();

	if(pType3->iDurationTime==0) return;
		
	__TABLE_UPC_SKILL* pSkill = s_pTbl_Skill.Find(dwMagicID);
	m_pGameProcMain->m_pUIStateBarAndMiniMap->AddMagic(pSkill, (float)pType3->iDurationTime);
	m_ListBuffTypeID.insert(stlmultimapVAL_INT_DWORD(key, dwMagicID));
}

//
//
//
void CMagicSkillMng::EffectingType4(uint32_t dwMagicID)
{
	__TABLE_UPC_SKILL_TYPE_4* pType4 = m_pTbl_Type_4->Find(dwMagicID);
	__ASSERT(pType4, "NULL type4 Pointer!!");
	if(!pType4) return;

	__InfoPlayerBase* pInfoBase = &(s_pPlayer->m_InfoBase);
	__InfoPlayerMySelf* pInfoExt = &(s_pPlayer->m_InfoExt);

	std::multimap<int, uint32_t>::iterator it = m_ListBuffTypeID.find(pType4->iBuffType);
	if(it!= m_ListBuffTypeID.end())
	{
		__TABLE_UPC_SKILL* pSkill = s_pTbl_Skill.Find(it->second);
		m_pGameProcMain->m_pUIStateBarAndMiniMap->DelMagic(pSkill);
		m_ListBuffTypeID.erase(it);
	}

	__TABLE_UPC_SKILL* pSkill = s_pTbl_Skill.Find(dwMagicID);
	m_pGameProcMain->m_pUIStateBarAndMiniMap->AddMagic(pSkill, (float)pType4->iDuration);
	m_ListBuffTypeID.insert(stlmultimapVAL_INT_DWORD(pType4->iBuffType,dwMagicID));

	//같은 버프타입의 마법은 중복사용할 수 없다...먼저 사용된 것만 유효..
	if(pType4)
	{
		switch(pType4->iBuffType)
		{
		case BUFFTYPE_MAXHP:
			pInfoBase->iHPMax -= m_iMaxHP;
									
			m_iMaxHP = pType4->iMaxHP;
			pInfoBase->iHPMax += m_iMaxHP;
			m_pGameProcMain->m_pUIVar->m_pPageState->UpdateHP(pInfoBase->iHP,
																pInfoBase->iHPMax);
			break;
		case BUFFTYPE_AC:
			pInfoExt->iGuard_Delta -= m_iAC;

			m_iAC = pType4->iAC;
			pInfoExt->iGuard_Delta += m_iAC;
			m_pGameProcMain->m_pUIVar->m_pPageState->UpdateGuardPoint(pInfoExt->iGuard, pInfoExt->iGuard_Delta);
			break;
		case BUFFTYPE_ATTACK:
			pInfoExt->iAttack_Delta -= m_iAttack;

			m_iAttack = (pInfoExt->iAttack * pType4->iAttack / 100) - pInfoExt->iAttack;
			pInfoExt->iAttack_Delta += m_iAttack;
			m_pGameProcMain->m_pUIVar->m_pPageState->UpdateAttackPoint(pInfoExt->iAttack, pInfoExt->iAttack_Delta);
			break;
		case BUFFTYPE_ATTACKSPEED:
			s_pPlayer->m_fAttackDelta /= m_fAttackSpeed; 

			m_fAttackSpeed = (float)pType4->iAttackSpeed / 100.0f;
			s_pPlayer->m_fAttackDelta *= m_fAttackSpeed; 
			break;
		case BUFFTYPE_SPEED:
			s_pPlayer->m_fMoveDelta /= m_fSpeed;

			m_fSpeed = (float)pType4->iMoveSpeed / 100.0f;
			s_pPlayer->m_fMoveDelta *= m_fSpeed; 
			break;
		case BUFFTYPE_ABILITY:
			pInfoExt->iStrength_Delta -= m_iStr;
			pInfoExt->iStamina_Delta -= m_iSta;
			pInfoExt->iDexterity_Delta -= m_iDex;
			pInfoExt->iIntelligence_Delta -= m_iInt;
			pInfoExt->iMagicAttak_Delta -= m_iMAP;

			m_iStr = pType4->iStr;
			m_iSta = pType4->iSta;
			m_iDex = pType4->iDex;
			m_iInt = pType4->iInt;
			m_iMAP = pType4->iMAP;
			pInfoExt->iStrength_Delta += m_iStr;
			pInfoExt->iStamina_Delta += m_iSta;
			pInfoExt->iDexterity_Delta += m_iDex;
			pInfoExt->iIntelligence_Delta += m_iInt;
			pInfoExt->iMagicAttak_Delta += m_iMAP;

			m_pGameProcMain->m_pUIVar->m_pPageState->UpdateStrength(pInfoExt->iStrength, pInfoExt->iStrength_Delta);
			m_pGameProcMain->m_pUIVar->m_pPageState->UpdateStamina(pInfoExt->iStamina, pInfoExt->iStamina_Delta);
			m_pGameProcMain->m_pUIVar->m_pPageState->UpdateDexterity(pInfoExt->iDexterity, pInfoExt->iDexterity_Delta);
			m_pGameProcMain->m_pUIVar->m_pPageState->UpdateIntelligence(pInfoExt->iIntelligence, pInfoExt->iIntelligence_Delta);
			m_pGameProcMain->m_pUIVar->m_pPageState->UpdateMagicAttak(pInfoExt->iMagicAttak, pInfoExt->iMagicAttak_Delta);
			break;
		case BUFFTYPE_RESIST:
			pInfoExt->iRegistFire_Delta -= m_iFireR;
			pInfoExt->iRegistCold_Delta -= m_iColdR;
			pInfoExt->iRegistLight_Delta -= m_iLightningR;
			pInfoExt->iRegistMagic_Delta -= m_iMagicR;
			pInfoExt->iRegistCurse_Delta -= m_iDeseaseR;
			pInfoExt->iRegistPoison_Delta -= m_iPoisonR;

			m_iFireR = pType4->iFireResist;
			m_iColdR = pType4->iColdResist;
			m_iLightningR = pType4->iLightningResist;
			m_iMagicR = pType4->iMagicResist;
			m_iDeseaseR = pType4->iDeseaseResist;
			m_iPoisonR = pType4->iPoisonResist;

			pInfoExt->iRegistFire_Delta += m_iFireR;
			pInfoExt->iRegistCold_Delta += m_iColdR;
			pInfoExt->iRegistLight_Delta += m_iLightningR;
			pInfoExt->iRegistMagic_Delta += m_iMagicR;
			pInfoExt->iRegistCurse_Delta += m_iDeseaseR;
			pInfoExt->iRegistPoison_Delta += m_iPoisonR;

			m_pGameProcMain->m_pUIVar->m_pPageState->UpdateRegistFire(pInfoExt->iRegistFire, pInfoExt->iRegistFire_Delta);
			m_pGameProcMain->m_pUIVar->m_pPageState->UpdateRegistCold(pInfoExt->iRegistCold, pInfoExt->iRegistCold_Delta);
			m_pGameProcMain->m_pUIVar->m_pPageState->UpdateRegistLight(pInfoExt->iRegistLight, pInfoExt->iRegistLight_Delta);
			m_pGameProcMain->m_pUIVar->m_pPageState->UpdateRegistMagic(pInfoExt->iRegistMagic, pInfoExt->iRegistMagic_Delta);
			m_pGameProcMain->m_pUIVar->m_pPageState->UpdateRegistCurse(pInfoExt->iRegistCurse, pInfoExt->iRegistCurse_Delta);
			m_pGameProcMain->m_pUIVar->m_pPageState->UpdateRegistPoison(pInfoExt->iRegistPoison, pInfoExt->iRegistPoison_Delta);
			break;
		}
	}
}


//
//	내가 쓰는 스킬이나 마법은 내가 인덱스를 넣어서 관리한다..
//	이건 인덱스 넣는 함수..
//
int CMagicSkillMng::AddIdx(uint32_t MagicID, int iNum)
{
	int idx = 0;
	std::map<int, uint32_t>::iterator it;
	
	//연결되는 index를 여러개 한꺼번에 만드는 경우..
	if(iNum>1)
	{
		if(m_MySelf.size()!=0)
		{
			it = m_MySelf.end();
			it--;
			idx = it->first + 1;
		}
		else idx = 1;						

		for(int i=0;i<iNum;i++)
		{
			m_MySelf.insert(stlmapVAL_INT_DWORD(idx+i, MagicID));
		}
		return idx;
	}

	//그냥 하나의 인덱스만 만드는 경우..
	for(it = m_MySelf.begin(); it!=m_MySelf.end(); it++)
	{
		if(it->first==idx)
		{
			idx++;
			continue;
		}
		else
		{
			m_MySelf.insert(stlmapVAL_INT_DWORD(idx, MagicID));
			break;
		}
	}
	if(it==m_MySelf.end())
	{
		m_MySelf.insert(stlmapVAL_INT_DWORD(idx, MagicID));
	}

	return idx;
}


//
//
//
void CMagicSkillMng::InitType4()
{
	m_iBuffType = 0;
	
	m_fAttackSpeed = 1.0f;
	m_fSpeed = 1.0f;

	m_iAC = 0;
	m_iAttack = 0;
	m_iMaxHP = 0;
	m_iStr = 0;
	m_iSta = 0;
	m_iDex = 0;
	m_iInt = 0;
	m_iMAP = 0;
	m_iFireR = 0;
	m_iColdR = 0;
	m_iLightningR = 0;
	m_iMagicR = 0;
	m_iDeseaseR = 0;
	m_iPoisonR = 0;


	std::multimap<int, uint32_t>::iterator its, ite;
	its = m_ListBuffTypeID.begin();
	ite = m_ListBuffTypeID.end();

	for(;its != ite; its++)
	{
		__TABLE_UPC_SKILL* pSkill = s_pTbl_Skill.Find((*its).second);
		m_pGameProcMain->m_pUIStateBarAndMiniMap->DelMagic(pSkill);		
	}
	m_ListBuffTypeID.clear();
}


//
//	이건 인덱스 제거하는 함수..
//
void CMagicSkillMng::RemoveIdx(int idx)
{
	m_MySelf.erase(idx);
}

uint32_t CMagicSkillMng::GetMagicID(int idx)
{
	std::map<int, uint32_t>::iterator it = m_MySelf.find(idx);
	return it->second;
}

D3DCOLOR CMagicSkillMng::TraceColorGet(__TABLE_UPC_SKILL* pSkill) // 스킬의 종류에 따라 검기의 색을 정한다..
{
	if(NULL == pSkill) return 0xff404040;
	
	D3DCOLOR crTrace = 0xffff4040;
	switch(pSkill->dwNeedItem) // 요구 아이템에 따라서...
	{
		case 1: crTrace = 0xff808080; // ITEM_CLASS_DAGGER = 11 // 단검(dagger)
		case 2: crTrace = 0xff909090; // ITEM_CLASS_SWORD = 21, // 한손검(onehandsword)
		//case : crTrace = ; // ITEM_CLASS_SWORD_2H = 22, // 3 : 양손검(twohandsword)
		case 3: crTrace = 0xff7070ff; // ITEM_CLASS_AXE = 31, // 한손도끼(onehandaxe)
		//case : crTrace = ; // ITEM_CLASS_AXE_2H = 32, // 두손도끼(twohandaxe)
		case 4: crTrace = 0xffa07070; // ITEM_CLASS_MACE = 41, // 한손타격무기(mace)
		//case : crTrace = ; // ITEM_CLASS_MACE_2H = 42, // 두손타격무기(twohandmace)
		case 5: crTrace = 0xffff7070; // ITEM_CLASS_SPEAR = 51, // 창(spear)
		//case : crTrace = ; // ITEM_CLASS_POLEARM = 52, // 폴암(polearm)
		default: crTrace = 0xff4040ff;
	}
	
	return crTrace;
}

bool CMagicSkillMng::IsPositiveMagic(uint32_t dwMagicID)
{
	__TABLE_UPC_SKILL* pSkill = CGameBase::s_pTbl_Skill.Find(dwMagicID);
	if(!pSkill) return true;

	if(pSkill->dw1stTableType==3 || pSkill->dw2ndTableType==3)
	{
		__TABLE_UPC_SKILL_TYPE_3* pType3 = m_pTbl_Type_3->Find(dwMagicID);
		if(!pType3) return true;

		int key = 0;
		if(pType3->iStartDamage>0 || (pType3->iStartDamage==0 && pType3->iDuraDamage>0) ) key = DDTYPE_TYPE3_DUR_OUR;
		else key = DDTYPE_TYPE3_DUR_ENEMY;

		key += pType3->iDDType;
		if(key==DDTYPE_TYPE3_DUR_OUR) return true;
		return false;
	}
	if(pSkill->dw1stTableType==4 || pSkill->dw2ndTableType==4)
	{
		__TABLE_UPC_SKILL_TYPE_4* pType4 = m_pTbl_Type_4->Find(dwMagicID);
		if(!pType4) return true;

		switch(pType4->iBuffType)
		{
		case BUFFTYPE_MAXHP:
			if(pType4->iMaxHP>0) return true;
			break;
		case BUFFTYPE_AC:
			if(pType4->iAC>0) return true;
			break;
		case BUFFTYPE_RESIZE:
			return true;
		case BUFFTYPE_ATTACK:
			if(pType4->iAttack>100) return true;
			break;
		case BUFFTYPE_ATTACKSPEED:
			if(pType4->iAttackSpeed>100) return true;
			break;
		case BUFFTYPE_SPEED:
			if(pType4->iMoveSpeed>100) return true;
			break;
		case BUFFTYPE_ABILITY:
			if(pType4->iStr>0 || pType4->iSta>0 || pType4->iDex>0 || pType4->iInt>0 || pType4->iMAP>0) return true;
			break;
		case BUFFTYPE_RESIST:
			if(pType4->iFireResist>0 || pType4->iColdResist>0 || pType4->iLightningResist>0 ||
				pType4->iMagicResist>0 || pType4->iDeseaseResist>0 || pType4->iPoisonResist>0) return true;
			break;
		}		
	}
	return false;
}

void CMagicSkillMng::ClearDurationalMagic()
{
	__InfoPlayerBase* pInfoBase = &(s_pPlayer->m_InfoBase);
	__InfoPlayerMySelf* pInfoExt = &(s_pPlayer->m_InfoExt);

	if(pInfoBase->iHP < pInfoBase->iHPMax)
	{
		pInfoBase->iHPMax -= m_iMaxHP;
		m_pGameProcMain->m_pUIVar->m_pPageState->UpdateHP(pInfoBase->iHP, pInfoBase->iHPMax);
	}
	m_iMaxHP = 0;

	pInfoExt->iGuard_Delta -= m_iAC;
	m_pGameProcMain->m_pUIVar->m_pPageState->UpdateGuardPoint(pInfoExt->iGuard, pInfoExt->iGuard_Delta);
	m_iAC = 0;

	if(m_iAttack) pInfoExt->iAttack_Delta -= m_iAttack;
	m_pGameProcMain->m_pUIVar->m_pPageState->UpdateAttackPoint(pInfoExt->iAttack, pInfoExt->iAttack_Delta);
	m_iAttack = 0;
	
	s_pPlayer->m_fAttackDelta /= m_fAttackSpeed; 
	m_fAttackSpeed = 1.0f;
	
	s_pPlayer->m_fMoveDelta /= m_fSpeed;
	m_fSpeed = 1.0f;
	
	pInfoExt->iStrength_Delta -= m_iStr;
	pInfoExt->iStamina_Delta -= m_iSta;
	pInfoExt->iDexterity_Delta -= m_iDex;
	pInfoExt->iIntelligence_Delta -= m_iInt;
	pInfoExt->iMagicAttak_Delta -= m_iMAP;
	m_pGameProcMain->m_pUIVar->m_pPageState->UpdateStrength(pInfoExt->iStrength, pInfoExt->iStrength_Delta);
	m_pGameProcMain->m_pUIVar->m_pPageState->UpdateStamina(pInfoExt->iStamina, pInfoExt->iStamina_Delta);
	m_pGameProcMain->m_pUIVar->m_pPageState->UpdateDexterity(pInfoExt->iDexterity, pInfoExt->iDexterity_Delta);
	m_pGameProcMain->m_pUIVar->m_pPageState->UpdateIntelligence(pInfoExt->iIntelligence, pInfoExt->iIntelligence_Delta);
	m_pGameProcMain->m_pUIVar->m_pPageState->UpdateMagicAttak(pInfoExt->iMagicAttak, pInfoExt->iMagicAttak_Delta);
	m_iStr = 0;
	m_iSta = 0;
	m_iDex = 0;
	m_iInt = 0;
	m_iMAP = 0;	
	
	pInfoExt->iRegistFire_Delta -= m_iFireR;
	pInfoExt->iRegistCold_Delta -= m_iColdR;
	pInfoExt->iRegistLight_Delta -= m_iLightningR;
	pInfoExt->iRegistMagic_Delta -= m_iMagicR;
	pInfoExt->iRegistCurse_Delta -= m_iDeseaseR;
	pInfoExt->iRegistPoison_Delta -= m_iPoisonR;
	m_pGameProcMain->m_pUIVar->m_pPageState->UpdateRegistFire(pInfoExt->iRegistFire, pInfoExt->iRegistFire_Delta);
	m_pGameProcMain->m_pUIVar->m_pPageState->UpdateRegistCold(pInfoExt->iRegistCold, pInfoExt->iRegistCold_Delta);
	m_pGameProcMain->m_pUIVar->m_pPageState->UpdateRegistLight(pInfoExt->iRegistLight, pInfoExt->iRegistLight_Delta);
	m_pGameProcMain->m_pUIVar->m_pPageState->UpdateRegistMagic(pInfoExt->iRegistMagic, pInfoExt->iRegistMagic_Delta);
	m_pGameProcMain->m_pUIVar->m_pPageState->UpdateRegistCurse(pInfoExt->iRegistCurse, pInfoExt->iRegistCurse_Delta);
	m_pGameProcMain->m_pUIVar->m_pPageState->UpdateRegistPoison(pInfoExt->iRegistPoison, pInfoExt->iRegistPoison_Delta);
	m_iFireR = 0;
	m_iColdR = 0;
	m_iLightningR = 0;
	m_iMagicR = 0;
	m_iDeseaseR = 0;
	m_iPoisonR = 0;

	InitType4();
}

void CMagicSkillMng::StopCastingByRatio()
{
	m_pGameProcMain->CommandSitDown(false, false); // 일으켜 세운다.
	if(IsCasting())
	{
		__TABLE_UPC_SKILL* pSkill = s_pTbl_Skill.Find(s_pPlayer->m_dwMagicID);
		if(pSkill)
		{
			int SuccessValue = rand()%100;
			if(SuccessValue >= pSkill->iPercentSuccess) // 스킬 테이블에 있는 확률대로 실패한다..
			{
				FailCast(pSkill);
				//if(	s_pPlayer->Action(PSA_BASIC, false, NULL, true); // 캐스팅 취소, 기본동작으로 강제 세팅..
			}				
		}
	}
}

void CMagicSkillMng::StunMySelf(__TABLE_UPC_SKILL_TYPE_3* pType3)
{
	if(pType3->iAttribute!=3) return;

	int sample = rand()%101;

	__InfoPlayerMySelf* pInfoExt = &(s_pPlayer->m_InfoExt);

	float Regist = (float)(pInfoExt->iRegistLight + pInfoExt->iRegistLight_Delta);
	if(Regist>80.0f) Regist = 80.0f;

	float Prob = (30.0f+(40.0f-( 40.0f*(Regist/80.0f) )));

	if(sample < (int)Prob) //얼어라...
	{
		m_pGameProcMain->CommandSitDown(false, false); // 일으켜 세운다.
		s_pPlayer->Stun(STUN_TIME);
	}
}

float CMagicSkillMng::GetCooldown(const __TABLE_UPC_SKILL* pSkill) const
{
	if (pSkill->iSelfAnimID1 > 0)
	{
		auto it = m_RecastTimes.find(pSkill->dwID);
		if (it == m_RecastTimes.end())
			return -1;

		return it->second;
	}
	else
	{
		auto it = m_NonActionRecastTimes.find(pSkill->dwID);
		if (it == m_NonActionRecastTimes.end())
			return -1;

		return it->second;
	}
}

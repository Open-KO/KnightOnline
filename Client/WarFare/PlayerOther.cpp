// PlayerOther.cpp: implementation of the CPlayerOther class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "text_resources.h"
#include "GameProcedure.h"
#include "PlayerOther.h"
#include "PlayerMySelf.h"

#include <N3Base/DFont.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlayerOther::CPlayerOther()
{
	m_ePlayerType = PLAYER_OTHER; // Player Type ... Base, NPC, OTher, MySelf

	m_InfoExt.Init();
	m_bSit = false;
}

CPlayerOther::~CPlayerOther()
{
}

void CPlayerOther::Tick()
{
	if( m_bSit )
		CPlayerBase::Tick();  // 회전, 지정된 에니메이션 Tick 및 색깔 지정 처리.. 등등..
	else
		CPlayerNPC::Tick();
}

bool CPlayerOther::Init(e_Race eRace, int iFace, int iHair, uint32_t* pdwItemIDs, int* piItenDurabilities)
{
	if(nullptr == pdwItemIDs || nullptr == piItenDurabilities) return false;

	m_InfoBase.eRace = eRace;
	m_InfoExt.iFace = iFace;
	m_InfoExt.iHair = iHair;

	// 이제 패킷에 따라 캐릭터를 치장..(?) 시켜준다.. 아이템장착, 무기 장착등...
	__TABLE_PLAYER_LOOKS* pLooks = s_pTbl_UPC_Looks.Find(eRace);	// 테이블에서 기본 스킨 ..
	if(nullptr == pLooks) 
	{
		CLogWriter::Write("CPlayerOther::Init() Basic Resource Pointer is NULL Race({})",
			static_cast<int>(eRace));
		return false;
	}
	this->InitChr(pLooks); // 관절 세팅..

	for(int i = 0; i < MAX_ITEM_SLOT_OPC; i++)
	{
		std::string szFN;
		e_PartPosition ePart = PART_POS_UNKNOWN;
		e_PlugPosition ePlug = PLUG_POS_UNKNOWN;
		e_ItemSlot eSlot = ITEM_SLOT_UNKNOWN;
		__TABLE_ITEM_BASIC* pItem = nullptr;
		__TABLE_ITEM_EXT* pItemExt = nullptr;

		if(0 == pdwItemIDs[i]) 
		{
			if(0 == i) { ePart = PART_POS_UPPER;			szFN = pLooks->szPartFNs[0]; }
			else if(1 == i) { ePart = PART_POS_LOWER;		szFN = pLooks->szPartFNs[1]; }
//			else if(2 == i) { ePart = PART_POS_HAIR_HELMET; szFN = pLooks->szPartFNs[5]; }
			else if(3 == i) { ePart = PART_POS_HANDS;		szFN = pLooks->szPartFNs[3]; }
			else if(4 == i) { ePart = PART_POS_FEET;		szFN = pLooks->szPartFNs[4]; }
			else if(5 == i) { } // 망토
//			else if(6 == i) { ePlug = PLUG_POS_RIGHTHAND; }
//			else if(7 == i) { ePlug = PLUG_POS_LEFTHAND; }
		}
		else
		{
			pItem = s_pTbl_Items_Basic.Find(pdwItemIDs[i]/1000*1000);	// 유저 플레이어 아이템 얻기..
			if(pItem && pItem->byExtIndex >= 0 && pItem->byExtIndex < MAX_ITEM_EXTENSION)
				pItemExt = s_pTbl_Items_Exts[pItem->byExtIndex].Find(pdwItemIDs[i]%1000);
			if(nullptr == pItem || nullptr == pItemExt)
			{
				__ASSERT(0, "NULL Item!!!");
				continue;
			}


			e_ItemType eType = CGameBase::MakeResrcFileNameForUPC(pItem, pItemExt, &szFN, nullptr, ePart, ePlug, m_InfoBase.eRace); // 리소스 파일 이름을 만들고..

			if(0 == i) { ePart = PART_POS_UPPER;			eSlot = ITEM_SLOT_UPPER; }
			else if(1 == i) { ePart = PART_POS_LOWER;		eSlot = ITEM_SLOT_LOWER; }
			else if(2 == i) { ePart = PART_POS_HAIR_HELMET; eSlot = ITEM_SLOT_HEAD; }
			else if(3 == i) { ePart = PART_POS_HANDS;		eSlot = ITEM_SLOT_GLOVES; }
			else if(4 == i) { ePart = PART_POS_FEET;		eSlot = ITEM_SLOT_SHOES; }
			else if(5 == i) { } // 망토
			else if(6 == i) { ePlug = PLUG_POS_RIGHTHAND;	eSlot = ITEM_SLOT_HAND_RIGHT; }
			else if(7 == i) { ePlug = PLUG_POS_LEFTHAND;	eSlot = ITEM_SLOT_HAND_LEFT; }
		}

		if(PART_POS_UPPER == ePart || PART_POS_LOWER == ePart || PART_POS_HANDS == ePart || PART_POS_FEET == ePart)
		{
			this->PartSet(ePart, szFN, pItem, pItemExt);
		}
		else if(PART_POS_HAIR_HELMET == ePart) // 머리카락 혹은 헬멧이면..
		{
			this->PartSet(ePart, szFN, pItem, pItemExt);
		}
		else if(5 == i) 
		{
		}
		else if((6 == i || 7 == i) && PLUG_POS_UNKNOWN != ePlug)
		{
			this->PlugSet(ePlug, szFN, pItem, pItemExt);
		}

		if(ITEM_SLOT_UNKNOWN != eSlot)
			this->DurabilitySet(eSlot, piItenDurabilities[i]);
	}

	// 얼굴은 따로하자..
	this->InitFace();
	CN3CPart* pPartHairHelmet = this->Part(PART_POS_HAIR_HELMET);
	if(pPartHairHelmet->FileName().empty()) // 헬멧에 해당되는게 없으면.. 머리카락 붙이기..
		this->InitHair();

	return true;
}

void CPlayerOther::InitFace()
{
	__TABLE_PLAYER_LOOKS* pItem = s_pTbl_UPC_Looks.Find(m_InfoBase.eRace);

	// 아이템이 있고 얼굴 이름이 있으면..
	if (pItem != nullptr
		&& !pItem->szPartFNs[PART_POS_FACE].empty())
	{
		char szDir[_MAX_DIR] = {}, szFName[_MAX_FNAME] = {}, szExt[_MAX_EXT] = {};
		_splitpath(pItem->szPartFNs[PART_POS_FACE].c_str(), nullptr, szDir, szFName, szExt);

		std::string szFN = fmt::format("{}{}{:02}{}", szDir, szFName, m_InfoExt.iFace, szExt);
		PartSet(PART_POS_FACE, szFN, nullptr, nullptr);
	}
}

void CPlayerOther::InitHair()
{
	__TABLE_PLAYER_LOOKS* pItem = s_pTbl_UPC_Looks.Find(m_InfoBase.eRace);
	if (pItem && !pItem->szPartFNs[PART_POS_HAIR_HELMET].empty()) // 아이템이 있고 얼굴 이름이 있으면..
	{
		char szDir[_MAX_DIR] = {}, szFName[_MAX_FNAME] = {}, szExt[_MAX_EXT] = {};
		_splitpath(pItem->szPartFNs[PART_POS_HAIR_HELMET].c_str(), nullptr, szDir, szFName, szExt);

		std::string szFN = fmt::format("{}{}{:02}{}", szDir, szFName, m_InfoExt.iHair, szExt);
		PartSet(PART_POS_HAIR_HELMET, szFN, nullptr, nullptr);
	}
	else
	{
		m_Chr.PartSet(PART_POS_HAIR_HELMET, "");
	}
}

void CPlayerOther::KnightsInfoSet(int iID, const std::string& szName, int iGrade, int iRank)
{
	CPlayerBase::KnightsInfoSet(iID, szName, iGrade, iRank);

	m_InfoExt.szKnights = szName;
	m_InfoExt.iKnightsGrade = iGrade;
	m_InfoExt.iKnightsRank = iRank;

	if (m_InfoExt.szKnights.empty())
	{
		delete m_pClanFont;
		m_pClanFont = nullptr;
	}

	float fDist = Distance(CGameBase::s_pPlayer->Position());
	if (fDist < SOUND_RANGE_TO_SET)
	{
		if (m_pClanFont == nullptr)
		{
			std::string szFontID = fmt::format_text_resource(IDS_FONT_ID);

			m_pClanFont = new CDFont(szFontID, 12);
			m_pClanFont->InitDeviceObjects(s_lpD3DDev);
			m_pClanFont->RestoreDeviceObjects();
		}

		m_pClanFont->SetText(m_InfoExt.szKnights.c_str(), D3DFONT_BOLD); // 폰트에 텍스트 지정.
		m_pClanFont->SetFontColor(KNIGHTS_FONT_COLOR);
	}
}

void CPlayerOther::SetSoundAndInitFont(uint32_t dwFontFlag)
{
	CPlayerBase::SetSoundAndInitFont();

	if (m_InfoExt.szKnights.empty())
	{
		delete m_pClanFont;
		m_pClanFont = nullptr;
		return;
	}

	if (m_pClanFont == nullptr)
	{
		std::string szFontID = fmt::format_text_resource(IDS_FONT_ID);

		m_pClanFont = new CDFont(szFontID, 12, D3DFONT_BOLD); // 좀 작게 만든다..
		m_pClanFont->InitDeviceObjects(s_lpD3DDev);
		m_pClanFont->RestoreDeviceObjects();
	}

	m_pClanFont->SetText(m_InfoExt.szKnights.c_str()); // 폰트에 텍스트 지정.
	m_pClanFont->SetFontColor(KNIGHTS_FONT_COLOR);
}

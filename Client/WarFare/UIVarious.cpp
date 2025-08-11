// UIState.cpp: implementation of the CUIState class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UIVarious.h"
#include "UIManager.h"
#include "UIInventory.h"
#include "UITransactionDlg.h"
#include "GameProcMain.h"
#include "PlayerMySelf.h"
#include "PlayerOtherMgr.h"
#include "PacketDef.h"
#include "APISocket.h"
#include "text_resources.h"

#include <N3Base/N3UIString.h>
#include <N3Base/N3UIImage.h>
#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIList.h>
#include <N3Base/N3SndObj.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUIState::CUIState()
{
	m_pText_ID = NULL;
	m_pText_Level = NULL;
	m_pText_RealmPoint = NULL;

	m_pText_Class = NULL;
	m_pText_Race = NULL;
	m_pText_Nation = NULL;

	m_pText_HP = NULL;
	m_pText_MP = NULL;
	m_pText_Exp = NULL;
	m_pText_AP = NULL;	// 공격 = NULL력
	m_pText_GP = NULL;			// 방어 = NULL력
	m_pText_Weight = NULL;

	
	m_pText_BonusPoint = NULL;

	m_pBtn_Strength = NULL;
	m_pBtn_Stamina = NULL;
	m_pBtn_Dexterity = NULL;
	m_pBtn_MagicAttak = NULL;
	m_pBtn_Intelligence = NULL;

	m_pText_Strength = NULL;
	m_pText_Stamina = NULL;
	m_pText_Dexterity = NULL;
	m_pText_MagicAttak = NULL;
	m_pText_Intelligence = NULL;

	m_pText_RegistFire = NULL;
	m_pText_RegistMagic = NULL;
	m_pText_RegistIce = NULL;
	m_pText_RegistCurse = NULL;
	m_pText_RegistLight = NULL;
	m_pText_RegistPoison = NULL;

	m_pImg_Str = NULL;
	m_pImg_Sta = NULL;
	m_pImg_Dex = NULL;
	m_pImg_Int = NULL;
	m_pImg_MAP = NULL;
}

CUIState::~CUIState()
{
}

void CUIState::Release()
{
	CN3UIBase::Release();

	m_pText_ID = NULL;
	m_pText_Level = NULL;
	m_pText_RealmPoint = NULL;

	m_pText_Class = NULL;
	m_pText_Race = NULL;
	m_pText_Nation = NULL;

	m_pText_HP = NULL;
	m_pText_MP = NULL;
	m_pText_Exp = NULL;
	m_pText_AP = NULL;	// 공격 = NULL력
	m_pText_GP = NULL;			// 방어 = NULL력
	m_pText_Weight = NULL;

	
	m_pText_BonusPoint = NULL;

	m_pBtn_Strength = NULL;
	m_pBtn_Stamina = NULL;
	m_pBtn_Dexterity = NULL;
	m_pBtn_MagicAttak = NULL;
	m_pBtn_Intelligence = NULL;

	m_pText_Strength = NULL;
	m_pText_Stamina = NULL;
	m_pText_Dexterity = NULL;
	m_pText_MagicAttak = NULL;
	m_pText_Intelligence = NULL;

	m_pText_RegistFire = NULL;
	m_pText_RegistMagic = NULL;
	m_pText_RegistIce = NULL;
	m_pText_RegistCurse = NULL;
	m_pText_RegistLight = NULL;
	m_pText_RegistPoison = NULL;

	m_pImg_Str = NULL;
	m_pImg_Sta = NULL;
	m_pImg_Dex = NULL;
	m_pImg_Int = NULL;
	m_pImg_MAP = NULL;
}

bool CUIState::Load(HANDLE hFile)
{
	if(CN3UIBase::Load(hFile)==false) return false;

	m_pText_ID			= (CN3UIString*)GetChildByID("Text_ID");			__ASSERT(m_pText_ID	, "NULL UI Component!!");
	m_pText_Level		= (CN3UIString*)GetChildByID("Text_Level");			__ASSERT(m_pText_Level	, "NULL UI Component!!");
	m_pText_RealmPoint	= (CN3UIString*)GetChildByID("Text_RealmPoint");	__ASSERT(m_pText_RealmPoint, "NULL UI Component!!");

	m_pText_Class		= (CN3UIString*)GetChildByID("Text_Class");		__ASSERT(m_pText_Class	, "NULL UI Component!!");
	m_pText_Race		= (CN3UIString*)GetChildByID("Text_Race");		__ASSERT(m_pText_Race		, "NULL UI Component!!");
	m_pText_Nation		= (CN3UIString*)GetChildByID("Text_Nation");	__ASSERT(m_pText_Nation	, "NULL UI Component!!");

	m_pText_HP			= (CN3UIString*)GetChildByID("Text_HP");		__ASSERT(m_pText_HP		, "NULL UI Component!!");
	m_pText_MP			= (CN3UIString*)GetChildByID("Text_MP");		__ASSERT(m_pText_MP		, "NULL UI Component!!");
	m_pText_Exp			= (CN3UIString*)GetChildByID("Text_Exp");		__ASSERT(m_pText_Exp		, "NULL UI Component!!");
	m_pText_AP			= (CN3UIString*)GetChildByID("Text_AP");		__ASSERT(m_pText_AP		, "NULL UI Component!!");
	m_pText_GP			= (CN3UIString*)GetChildByID("Text_GP");		__ASSERT(m_pText_GP		, "NULL UI Component!!");
	m_pText_Weight		= (CN3UIString*)GetChildByID("Text_Weight");	__ASSERT(m_pText_Weight	, "NULL UI Component!!");

	m_pText_BonusPoint		= (CN3UIString*)GetChildByID("Text_BonusPoint"); __ASSERT(m_pText_BonusPoint	, "NULL UI Component!!");

	m_pBtn_Strength		= (CN3UIButton*)GetChildByID("Btn_Strength");		__ASSERT(m_pBtn_Strength	, "NULL UI Component!!");
	m_pBtn_Stamina		= (CN3UIButton*)GetChildByID("Btn_Stamina");		__ASSERT(m_pBtn_Stamina	, "NULL UI Component!!");
	m_pBtn_Dexterity	= (CN3UIButton*)GetChildByID("Btn_Dexterity");		__ASSERT(m_pBtn_Dexterity	, "NULL UI Component!!");
	m_pBtn_MagicAttak	= (CN3UIButton*)GetChildByID("Btn_MagicAttack");	__ASSERT(m_pBtn_MagicAttak	, "NULL UI Component!!");
	m_pBtn_Intelligence	= (CN3UIButton*)GetChildByID("Btn_Intelligence");	__ASSERT(m_pBtn_Intelligence, "NULL UI Component!!");

	m_pText_Strength	= (CN3UIString*)GetChildByID("Text_Strength");		__ASSERT(m_pText_Strength	, "NULL UI Component!!");
	m_pText_Stamina		= (CN3UIString*)GetChildByID("Text_Stamina");		__ASSERT(m_pText_Stamina	, "NULL UI Component!!");
	m_pText_Dexterity	= (CN3UIString*)GetChildByID("Text_Dexterity");		__ASSERT(m_pText_Dexterity, "NULL UI Component!!");
	m_pText_MagicAttak	= (CN3UIString*)GetChildByID("Text_MagicAttack");	__ASSERT(m_pText_MagicAttak, "NULL UI Component!!");
	m_pText_Intelligence = (CN3UIString*)GetChildByID("Text_Intelligence");	__ASSERT(m_pText_Intelligence, "NULL UI Component!!");

	m_pText_RegistFire	= (CN3UIString*)GetChildByID("Text_RegistFire");	__ASSERT(m_pText_RegistFire, "NULL UI Component!!");
	m_pText_RegistMagic	= (CN3UIString*)GetChildByID("Text_RegistMagic");	__ASSERT(m_pText_RegistMagic, "NULL UI Component!!");
	m_pText_RegistIce	= (CN3UIString*)GetChildByID("Text_RegistIce");		__ASSERT(m_pText_RegistIce, "NULL UI Component!!");
	m_pText_RegistCurse	= (CN3UIString*)GetChildByID("Text_RegistCurse");	__ASSERT(m_pText_RegistCurse, "NULL UI Component!!");
	m_pText_RegistLight  = (CN3UIString*)GetChildByID("Text_RegistLightR");	__ASSERT(m_pText_RegistLight , "NULL UI Component!!");
	m_pText_RegistPoison = (CN3UIString*)GetChildByID("Text_RegistPoison");	__ASSERT(m_pText_RegistPoison, "NULL UI Component!!");

	m_pImg_Str  = GetChildByID("img_str");	__ASSERT(m_pImg_Str, "NULL UI Component!!");
	m_pImg_Sta	= GetChildByID("img_sta");	__ASSERT(m_pImg_Sta, "NULL UI Component!!");
	m_pImg_Dex  = GetChildByID("img_dex");	__ASSERT(m_pImg_Dex , "NULL UI Component!!");
	m_pImg_Int	= GetChildByID("img_int");	__ASSERT(m_pImg_Int, "NULL UI Component!!");
	m_pImg_MAP	= GetChildByID("img_map");	__ASSERT(m_pImg_MAP, "NULL UI Component!!");

	return true;
}

void CUIState::UpdateBonusPointAndButtons(int iBonusPointRemain) // 보너스 포인트 적용이 가능한가??
{
	bool bEnable = false;
	if(iBonusPointRemain > 0) bEnable = true;
	else bEnable = false;

	if(m_pText_BonusPoint) m_pText_BonusPoint->SetStringAsInt(iBonusPointRemain);

	if(m_pBtn_Strength)		m_pBtn_Strength->SetVisible(bEnable); // 경험치 체인지..
	if(m_pBtn_Stamina)		m_pBtn_Stamina->SetVisible(bEnable);
	if(m_pBtn_Dexterity)	m_pBtn_Dexterity->SetVisible(bEnable);
	if(m_pBtn_Intelligence)	m_pBtn_Intelligence->SetVisible(bEnable);
	if(m_pBtn_MagicAttak)	m_pBtn_MagicAttak->SetVisible(bEnable);
}

void CUIState::UpdateID(const std::string& szID)
{
	if (m_pText_ID != nullptr)
		m_pText_ID->SetString(szID);
}

void CUIState::UpdateLevel(int iVal)
{
	if (m_pText_Level != nullptr)
		m_pText_Level->SetStringAsInt(iVal);
}

void CUIState::UpdateRealmPoint(int iLoyalty, int iLoyaltyMonthly) // 국가 기여도는 10을 나누어서 표시
{
	if (m_pText_RealmPoint == nullptr)
		return;

	std::string buff = fmt::format("{} / {}", iLoyalty, iLoyaltyMonthly);
	m_pText_RealmPoint->SetString(buff); // 국가 기여도는 10을 나누어서 표시
}

void CUIState::UpdateHP(int iVal, int iValMax)
{
	__ASSERT(iVal >= 0 && iVal < 10000 && iValMax >= 0 && iValMax < 10000, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	if (m_pText_HP == nullptr)
		return;

	std::string buff = fmt::format("{} / {}", iVal, iValMax);
	m_pText_HP->SetString(buff);
}

void CUIState::UpdateMSP(int iVal, int iValMax)
{
	__ASSERT(iVal >= 0 && iValMax > 0, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	if (m_pText_MP == nullptr)
		return;

	std::string buff = fmt::format("{} / {}", iVal, iValMax);
	m_pText_MP->SetString(buff);
}

void CUIState::UpdateExp(int64_t iVal, int64_t iValMax)
{
	__ASSERT(iVal >= 0 && iValMax > 0, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	if (m_pText_Exp == nullptr)
		return;

	std::string buff = fmt::format("{} / {}", iVal, iValMax);
	m_pText_Exp->SetString(buff);
}

void CUIState::UpdateAttackPoint(int iVal, int iDelta)
{
	if (m_pText_AP == nullptr)
		return;

	if (iDelta != 0)
	{
		std::string buff = FormatWithDelta(iVal, iDelta);
		m_pText_AP->SetString(buff);
	}
	else
	{
		m_pText_AP->SetStringAsInt(iVal);
	}
}

void CUIState::UpdateGuardPoint(int iVal, int iDelta)
{
	if (m_pText_GP == nullptr)
		return;

	if (iDelta != 0)
	{
		std::string buff = FormatWithDelta(iVal, iDelta);
		m_pText_GP->SetString(buff);
	}
	else
	{
		m_pText_GP->SetStringAsInt(iVal);
	}
}

void CUIState::UpdateWeight(int iVal, int iValMax)
{
	if (m_pText_Weight == nullptr)
		return;

	std::string szVal = fmt::format("{:.1f}/{:.1f}", (iVal * 0.1f), (iValMax * 0.1f));
	m_pText_Weight->SetString(szVal);

	std::string szMsg = fmt::format_text_resource(IDS_INVEN_WEIGHT);
	std::string str = szMsg + szVal;

	CUIInventory* pInv = CGameProcedure::s_pProcMain->m_pUIInventory;
	if (pInv != nullptr)
		pInv->UpdateWeight(str);

	CUITransactionDlg* pUITransactionDlg = CGameProcedure::s_pProcMain->m_pUITransactionDlg;
	if (pUITransactionDlg != nullptr)
		pUITransactionDlg->UpdateWeight(str);
}

void CUIState::UpdateStrength(int iVal, int iDelta)
{
	if (m_pText_Strength == nullptr)
		return;

	if (iDelta != 0)
	{
		std::string buff = FormatWithDelta(iVal, iDelta);
		m_pText_Strength->SetString(buff);
	}
	else
	{
		m_pText_Strength->SetStringAsInt(iVal);
	}
}

void CUIState::UpdateStamina(int iVal, int iDelta)
{
	if (m_pText_Stamina == nullptr)
		return;

	if (iDelta != 0)
	{
		std::string buff = FormatWithDelta(iVal, iDelta);
		m_pText_Stamina->SetString(buff);
	}
	else
	{
		m_pText_Stamina->SetStringAsInt(iVal);
	}
}

void CUIState::UpdateDexterity(int iVal, int iDelta)
{
	if (m_pText_Dexterity == nullptr)
		return;

	if (iDelta != 0)
	{
		std::string buff = FormatWithDelta(iVal, iDelta);
		m_pText_Dexterity->SetString(buff);
	}
	else
	{
		m_pText_Dexterity->SetStringAsInt(iVal);
	}
}

void CUIState::UpdateIntelligence(int iVal, int iDelta)
{
	if (m_pText_Intelligence == nullptr)
		return;

	if (iDelta != 0)
	{
		std::string buff = FormatWithDelta(iVal, iDelta);
		m_pText_Intelligence->SetString(buff);
	}
	else
	{
		m_pText_Intelligence->SetStringAsInt(iVal);
	}
}

void CUIState::UpdateMagicAttak(int iVal, int iDelta)
{
	if (m_pText_MagicAttak == nullptr)
		return;

	if (iDelta != 0)
	{
		std::string buff = FormatWithDelta(iVal, iDelta);
		m_pText_MagicAttak->SetString(buff);
	}
	else
	{
		m_pText_MagicAttak->SetStringAsInt(iVal);
	}
}

void CUIState::UpdateRegistFire(int iVal, int iDelta)
{
	if (m_pText_RegistFire == nullptr)
		return;

	if (iDelta != 0)
	{
		std::string buff = FormatWithDelta(iVal, iDelta);
		m_pText_RegistFire->SetString(buff);
	}
	else
	{
		m_pText_RegistFire->SetStringAsInt(iVal);
	}
}

void CUIState::UpdateRegistCold(int iVal, int iDelta)
{
	if (m_pText_RegistIce == nullptr)
		return;

	if (iDelta != 0)
	{
		std::string buff = FormatWithDelta(iVal, iDelta);
		m_pText_RegistIce->SetString(buff);
	}
	else
	{
		m_pText_RegistIce->SetStringAsInt(iVal);
	}
}

void CUIState::UpdateRegistLight(int iVal, int iDelta)
{
	if (m_pText_RegistLight == nullptr)
		return;

	if (iDelta)
	{
		std::string buff = FormatWithDelta(iVal, iDelta);
		m_pText_RegistLight->SetString(buff);
	}
	else
	{
		m_pText_RegistLight->SetStringAsInt(iVal);
	}
}

void CUIState::UpdateRegistMagic(int iVal, int iDelta)
{
	if (m_pText_RegistMagic == nullptr)
		return;

	if (iDelta != 0)
	{
		std::string buff = FormatWithDelta(iVal, iDelta);
		m_pText_RegistMagic->SetString(buff);
	}
	else
	{
		m_pText_RegistMagic->SetStringAsInt(iVal);
	}
}

void CUIState::UpdateRegistCurse(int iVal, int iDelta)
{
	if (m_pText_RegistCurse == nullptr)
		return;

	if (iDelta != 0)
	{
		std::string buff = FormatWithDelta(iVal, iDelta);
		m_pText_RegistCurse->SetString(buff);
	}
	else
	{
		m_pText_RegistCurse->SetStringAsInt(iVal);
	}
}

void CUIState::UpdateRegistPoison(int iVal, int iDelta)
{
	if (m_pText_RegistPoison == nullptr)
		return;

	if (iDelta != 0)
	{
		std::string buff = FormatWithDelta(iVal, iDelta);
		m_pText_RegistPoison->SetString(buff);
	}
	else
	{
		m_pText_RegistPoison->SetStringAsInt(iVal);
	}
}

std::string CUIState::FormatWithDelta(int iVal, int iDelta)
{
	if (iDelta > 0)
		return fmt::format("{}(+{})", iVal, iDelta);

	return fmt::format("{}({})", iVal, iDelta);
}

bool CUIState::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)					
	{
		if(pSender == m_pBtn_Strength) // 경험치 체인지..
			this->MsgSendAblityPointChange(0x01, +1);
		else if(pSender == m_pBtn_Stamina)
			this->MsgSendAblityPointChange(0x02, +1);
		else if(pSender == m_pBtn_Dexterity)
			this->MsgSendAblityPointChange(0x03, +1);
		else if(pSender == m_pBtn_Intelligence)
			this->MsgSendAblityPointChange(0x04, +1);
		else if(pSender == m_pBtn_MagicAttak)
			this->MsgSendAblityPointChange(0x05, +1);
	}

	return true;
}

void CUIState::MsgSendAblityPointChange(uint8_t byType, int16_t siValueDelta)
{
	uint8_t byBuff[4];
	int iOffset = 0;
	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_POINT_CHANGE);
	CAPISocket::MP_AddByte(byBuff, iOffset, byType);
	CAPISocket::MP_AddShort(byBuff, iOffset, siValueDelta); // 0x00 - 점차 늘어나게끔.. 0x01 - 즉시 업데이트..

	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
}

CUIKnights::CUIKnights()
{
	m_iPageCur = 1;
	// m_MemberList = NULL;

	m_pText_Name = NULL;
	m_pText_Duty = NULL;
	m_pText_Page = NULL;
	m_pText_MemberCount = NULL;

	// m_pImage_Grade = NULL;

	m_pList_CharGrades = NULL;
	m_pList_CharIDs = NULL;
	m_pList_CharLevels = NULL;
	m_pList_CharJobs = NULL;

	m_pBtn_Admit = NULL;
	m_pBtn_Appoint = NULL;
	m_pBtn_Remove = NULL;
	m_pBtn_Refresh = NULL;

	m_fTimeLimit_Refresh = 0.0f;
	m_fTimeLimit_Appoint = 0.0f;
	m_fTimeLimit_Remove = 0.0f;
	m_fTimeLimit_Admit = 0.0f;
}

CUIKnights::~CUIKnights()
{
}

void CUIKnights::Release() // TODO: check memory leaks
{
	CN3UIBase::Release();

	m_iPageCur = 1;
	// m_MemberList = NULL;

	m_pText_Name = NULL;
	m_pText_Duty = NULL;
	m_pText_Page = NULL;
	m_pText_MemberCount = NULL;

	// m_pImage_Grade = NULL;

	m_pList_CharGrades = NULL;
	m_pList_CharIDs = NULL;
	m_pList_CharLevels = NULL;
	m_pList_CharJobs = NULL;

	m_pBtn_Admit = NULL;
	m_pBtn_Appoint = NULL;
	m_pBtn_Remove = NULL;
	m_pBtn_Refresh = NULL;
}

void CUIKnights::Clear()
{
	m_MemberList.clear();

	ClearLists();
	UpdatePageNumber(1);
	UpdateMemberCount(0);

	m_pText_Name->SetString("");
	m_pText_Duty->SetString("");
	m_pText_MemberCount->SetString("0");

	this->ChangeUIByDuty(CGameProcedure::s_pPlayer->m_InfoExt.eKnightsDuty);
}

void CUIKnights::SetVisible(bool bVisible)
{
	if (bVisible == this->IsVisible()) return;

	if (bVisible)
		RefreshButtonHandler(true);

	CN3UIBase::SetVisible(bVisible);
}

bool CUIKnights::Load(HANDLE hFile)
{
	if (false == CN3UIBase::Load(hFile)) return false;

	m_pText_Name = (CN3UIString*)this->GetChildByID("Text_ClansName");			__ASSERT(m_pText_Name, "NULL UI Component!!");
	m_pText_Duty = (CN3UIString*)this->GetChildByID("Text_clan_Duty");			__ASSERT(m_pText_Duty, "NULL UI Component!!");
	m_pText_Page = (CN3UIString*)this->GetChildByID("Text_clan_Page");			__ASSERT(m_pText_Page, "NULL UI Component!!");
	m_pText_MemberCount = (CN3UIString*)this->GetChildByID("Text_clan_MemberCount");	__ASSERT(m_pText_MemberCount, "NULL UI Component!!");

	m_pList_CharGrades = (CN3UIList*)this->GetChildByID("List_clan_Grade");		__ASSERT(m_pList_CharGrades, "NULL UI Component!!");
	m_pList_CharIDs = (CN3UIList*)this->GetChildByID("List_clan_ChrID");		__ASSERT(m_pList_CharIDs, "NULL UI Component!!");
	m_pList_CharLevels = (CN3UIList*)this->GetChildByID("List_clan_Level");		__ASSERT(m_pList_CharLevels, "NULL UI Component!!");
	m_pList_CharJobs = (CN3UIList*)this->GetChildByID("List_clan_Job");		__ASSERT(m_pList_CharJobs, "NULL UI Component!!");

	m_pBtn_Admit = (CN3UIButton*)(this->GetChildByID("btn_clan_admit"));		__ASSERT(m_pBtn_Admit, "NULL UI Component!!");
	m_pBtn_Appoint = (CN3UIButton*)(this->GetChildByID("btn_clan_Appoint"));		__ASSERT(m_pBtn_Appoint, "NULL UI Component!!");
	m_pBtn_Remove = (CN3UIButton*)(this->GetChildByID("btn_clan_Remove"));		__ASSERT(m_pBtn_Remove, "NULL UI Component!!");
	m_pBtn_Refresh = (CN3UIButton*)(this->GetChildByID("btn_clan_refresh"));		__ASSERT(m_pBtn_Refresh, "NULL UI Component!!");

	std::string szID;
	for (int i = 0; i < MAX_CLAN_GRADE; i++)
	{
		szID = fmt::format("image_grade{:02}", i);
		N3_VERIFY_UI_COMPONENT(m_pImage_Grade[i], (CN3UIImage*) GetChildByID(szID));

		if (m_pImage_Grade[i] != nullptr)
			m_pImage_Grade[i]->SetVisible(false);
	}

	UpdatePageNumber(1);

	m_pList_CharGrades->SetState(UI_STATE_LIST_DISABLE);
	m_pList_CharLevels->SetState(UI_STATE_LIST_DISABLE);
	m_pList_CharJobs->SetState(UI_STATE_LIST_DISABLE);
	return true;
}

void CUIKnights::UpdatePageNumber(int nNewPageNr) // @Demircivi: default value for nNewPageNr is -1.
{
	if (nNewPageNr != -1)
		m_iPageCur = nNewPageNr;

	m_pText_Page->SetStringAsInt(m_iPageCur);
}

void CUIKnights::UpdateMemberCount(int nMemberCount)
{
	std::string memberCount = std::to_string(nMemberCount); // TODO: @Demircivi: it should be %d/%d, second %d is clan max member count.
	m_pText_MemberCount->SetString(memberCount);
}

bool CUIKnights::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg != UIMSG_BUTTON_CLICK)
		return false; // @Demircivi: Since we don't care about anything else click.

	if (pSender->m_szID == "btn_clan_up")
		PreviousPageButtonHandler();
	else if (pSender->m_szID == "btn_clan_down")
		NextPageButtonHandler();
	else if (pSender->m_szID == "btn_clan_refresh")
		RefreshButtonHandler();
	else if (pSender->m_szID == "btn_clan_whisper")
		WhisperButtonHandler();
	else if (pSender->m_szID == "btn_clan_admit")
		AdmitButtonHandler();
	else if (pSender->m_szID == "Btn_clan_Remove")
		RemoveButtonHandler();
	else if (pSender->m_szID == "Btn_clan_Appoint")
		AppointButtonHandler();
	else
		return false;

	return true;
}

void CUIKnights::PreviousPageButtonHandler()
{
	m_iPageCur--;
	if (m_iPageCur < 1)
		m_iPageCur = 1;

	RefreshList();
}

void CUIKnights::NextPageButtonHandler()
{
	m_iPageCur++;
	int MaxPage = m_MemberList.size() / 10 + 1;
	if (m_iPageCur > MaxPage)
		m_iPageCur = MaxPage;

	UpdatePageNumber();
	RefreshList();
}

void CUIKnights::RefreshButtonHandler(bool blBypassTime)
{
	if (m_fTimeLimit_Refresh < 5.0f && !blBypassTime)
		return;

	m_fTimeLimit_Refresh = 0.0f;

	Clear();
	UpdateExceptList();

	MsgSend_MemberInfoAll();
}

void CUIKnights::WhisperButtonHandler()
{
	int index = m_pList_CharIDs->GetCurSel();

	if (index == -1)
		return;

	std::string szName;
	m_pList_CharIDs->GetString(index, szName);
	CGameProcedure::s_pProcMain->MsgSend_ChatSelectTarget(szName);
}

void CUIKnights::AdmitButtonHandler()
{
	if (m_fTimeLimit_Admit < 3.0f)
		return;

	m_fTimeLimit_Admit = 0.0f;

	CGameProcedure::s_pProcMain->MsgSend_KnightsJoin(CGameProcedure::s_pPlayer->m_iIDTarget);
	m_fTimeLimit_Admit = 0.0f;
}

void CUIKnights::RemoveButtonHandler() // TODO: @Demircivi, maybe add confirmation dialog in further development? just saying.
{
	if (m_fTimeLimit_Remove < 3.0f)
		return;

	m_fTimeLimit_Remove = 0.0f;

	int index = m_pList_CharIDs->GetCurSel();
	if (index == -1)
		return;

	std::string szName;
	m_pList_CharIDs->GetString(index, szName);
	CGameProcedure::s_pProcMain->MsgSend_KnightsLeave(szName);
}

void CUIKnights::AppointButtonHandler()
{
	if (m_fTimeLimit_Appoint < 3.0f)
		return;

	m_fTimeLimit_Appoint = 0.0f;

	int index = m_pList_CharIDs->GetCurSel();
	if (index == -1)
		return;

	std::string szName;
	m_pList_CharIDs->GetString(index, szName);
	CGameProcedure::s_pProcMain->MsgSend_KnightsAppointViceChief(szName);
}

void CUIKnights::ClearLists()
{
	m_pList_CharGrades->ResetContent();
	m_pList_CharIDs->ResetContent();
	m_pList_CharLevels->ResetContent();
	m_pList_CharJobs->ResetContent();
}

void CUIKnights::RefreshList()
{
	ClearLists();

	auto it = m_MemberList.begin();

	int i = 10;
	int e = m_iPageCur * 10;

	for (; i < e; i++)
	{
		if (it == m_MemberList.end()) break;
		it++;
	}

	for (i = 0; i < 10; i++)
	{
		if (it == m_MemberList.end()) break;

		__KnightsMemberInfo KMI = (*it);

		if (KMI.iConnected)
		{
			std::string szClass, szDuty;
			CGameProcedure::GetTextByKnightsDuty(KMI.eDuty, szDuty);
			CGameProcedure::GetTextByClass(KMI.eClass, szClass);

			std::string level = std::to_string(KMI.iLevel);

			int index = m_pList_CharGrades->AddString(szDuty); // TODO: @Demircivi, Char Grade is not loading from language files.
			m_pList_CharIDs->AddString(KMI.szName);
			m_pList_CharLevels->AddString(level);
			m_pList_CharJobs->AddString(szClass);

			m_pList_CharGrades->SetFontColor(index, 0xff00ff00);
			m_pList_CharIDs->SetFontColor(index, 0xff00ff00);
			m_pList_CharLevels->SetFontColor(index, 0xff00ff00);
			m_pList_CharJobs->SetFontColor(index, 0xff00ff00);
		}
		else
		{
			int index = m_pList_CharGrades->AddString("....");
			m_pList_CharIDs->AddString(KMI.szName);
			m_pList_CharLevels->AddString("....");
			m_pList_CharJobs->AddString("....");

			m_pList_CharGrades->SetFontColor(index, 0xff808080);
			m_pList_CharIDs->SetFontColor(index, 0xff808080);
			m_pList_CharLevels->SetFontColor(index, 0xff808080);
			m_pList_CharJobs->SetFontColor(index, 0xff808080);
		}
		it++;
	}

	m_pList_CharGrades->SetCurSel(-1);
	m_pList_CharLevels->SetCurSel(-1);
	m_pList_CharJobs->SetCurSel(-1);
}

void CUIKnights::MemberListSort()
{
	it_KMI it = m_MemberList.begin(), itEnd = m_MemberList.end();

	__KnightsMemberInfo Chief; Chief.eDuty = KNIGHTS_DUTY_UNKNOWN;
	__KnightsMemberInfo ViceChief[3];
	ViceChief[0].eDuty = KNIGHTS_DUTY_UNKNOWN;
	ViceChief[1].eDuty = KNIGHTS_DUTY_UNKNOWN;
	ViceChief[2].eDuty = KNIGHTS_DUTY_UNKNOWN;

	int iViceChiefCount = 0;
	while (it != itEnd)
	{
		__KnightsMemberInfo kmi = (*it);

		if (kmi.eDuty == KNIGHTS_DUTY_CHIEF)
		{
			Chief = kmi;
			it = m_MemberList.erase(it);
			continue;
		}

		if (kmi.eDuty == KNIGHTS_DUTY_VICECHIEF)
		{
			ViceChief[iViceChiefCount] = kmi;
			it = m_MemberList.erase(it);
			iViceChiefCount++;
			continue;
		}
		it++;
	}

	for (int i = 0; i<3; i++)
		if (ViceChief[i].eDuty != KNIGHTS_DUTY_UNKNOWN) m_MemberList.push_front(ViceChief[i]);

	if (Chief.eDuty != KNIGHTS_DUTY_UNKNOWN) m_MemberList.push_front(Chief);
}

void CUIKnights::MemberListUpdate()
{
	MemberListSort();
	RefreshList();
}

void CUIKnights::MsgSend_MemberInfoAll()
{
	int iOffset = 0;
	uint8_t byBuff[32];
	
	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_KNIGHTS_PROCESS);
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_KNIGHTS_MEMBER_INFO_ALL);
	
	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
}

bool CUIKnights::MsgRecv_MemberInfo(Packet& pkt)
{
	pkt.read<int16_t>(); // @Demircivi: packet sizes, which are unused.
	pkt.read<int16_t>(); // @Demircivi: packet sizes, which are unused.
	pkt.read<int16_t>(); // @Demircivi: packet sizes, which are unused.

	int iMemberCount = pkt.read<int16_t>();

	UpdateMemberCount(iMemberCount);

	for (int i = 0; i < iMemberCount; i++)
	{
		int iNameLength = pkt.read<int16_t>();
		__ASSERT(iNameLength > 0, "iNameLength was below 0!");

		__KnightsMemberInfo KMI;

		pkt.readString(KMI.szName, iNameLength);
		// KMI.szName = szName;
		KMI.eDuty = (e_KnightsDuty)pkt.read<uint8_t>();
		KMI.iLevel = pkt.read<uint8_t>();
		KMI.eClass = (e_Class)pkt.read<int16_t>();
		KMI.iConnected = pkt.read<uint8_t>();

		m_MemberList.push_back(KMI);
	}

	UpdatePageNumber(1);

	this->MemberListUpdate(); // List 에 다 넣었으면 UI Update!!
	
	return true;
}

void CUIKnights::UpdateKnightsName(const std::string& szName)
{
	if(NULL == m_pText_Name) return;
	m_pText_Name->SetString(szName);
}

void CUIKnights::UpdateKnightsDuty(e_KnightsDuty eDuty)
{
	if(NULL == m_pText_Duty) return;
	std::string szDuty;
	switch(eDuty)
	{
		case KNIGHTS_DUTY_CHIEF:		szDuty = fmt::format_text_resource(IDS_KNIGHTS_DUTY_CHIEF); break;
		case KNIGHTS_DUTY_VICECHIEF:	szDuty = fmt::format_text_resource(IDS_KNIGHTS_DUTY_VICECHIEF); break;
		case KNIGHTS_DUTY_OFFICER:		szDuty = fmt::format_text_resource(IDS_KNIGHTS_DUTY_OFFICER); break;
		case KNIGHTS_DUTY_KNIGHT:		szDuty = fmt::format_text_resource(IDS_KNIGHTS_DUTY_KNIGHT); break;
		case KNIGHTS_DUTY_TRAINEE:		szDuty = fmt::format_text_resource(IDS_KNIGHTS_DUTY_TRAINEE); break;
		case KNIGHTS_DUTY_PUNISH:		szDuty = fmt::format_text_resource(IDS_KNIGHTS_DUTY_PUNISH); break;
		case KNIGHTS_DUTY_UNKNOWN:		szDuty = fmt::format_text_resource(IDS_KNIGHTS_DUTY_UNKNOWN); break;
		default: __ASSERT(0, "Invalid Knights Duty"); break;
	}	
	m_pText_Duty->SetString(szDuty);	
}

void CUIKnights::UpdateKnightsGrade(int iVal)
{
	for(int i = 0; i < MAX_CLAN_GRADE; i++)
		if(m_pImage_Grade[i])
			m_pImage_Grade[i]->SetVisible(false);

	if(iVal <= MAX_CLAN_GRADE && iVal > 0)
		if(m_pImage_Grade[iVal - 1]) 
			m_pImage_Grade[iVal - 1]->SetVisible(true);
}

void CUIKnights::UpdateKnightsRank(int iVal)
{
	// TODO: @Demircivi.
}

void CUIKnights::ChangeUIByDuty(e_KnightsDuty eDuty) // 권한에 따라 UI 변경..
{
	if (KNIGHTS_DUTY_CHIEF == eDuty)
	{
		m_pBtn_Admit->SetVisible(true);
		m_pBtn_Appoint->SetVisible(true);
		m_pBtn_Remove->SetVisible(true);
	}
	else if (KNIGHTS_DUTY_VICECHIEF == eDuty)
	{
		m_pBtn_Admit->SetVisible(true);
		m_pBtn_Appoint->SetVisible(false);
		m_pBtn_Remove->SetVisible(false);
	}
	else
	{
		m_pBtn_Admit->SetVisible(false);
		m_pBtn_Appoint->SetVisible(false);
		m_pBtn_Remove->SetVisible(false);
	}
}

void CUIKnights::UpdateExceptList()
{
	UpdateKnightsDuty(CGameProcedure::s_pPlayer->m_InfoExt.eKnightsDuty);
	UpdateKnightsName(CGameProcedure::s_pPlayer->m_InfoExt.szKnights);
	UpdateKnightsGrade(CGameProcedure::s_pPlayer->m_InfoExt.iKnightsGrade);
	UpdateKnightsRank(CGameProcedure::s_pPlayer->m_InfoExt.iKnightsRank);
	ChangeUIByDuty(CGameProcedure::s_pPlayer->m_InfoExt.eKnightsDuty);
}





CUIFriends::CUIFriends()
{
	m_iPageCur = 0;

	m_pList_Friends = NULL;
	m_pText_Page = NULL;

	m_pBtn_NextPage = NULL;
	m_pBtn_PrevPage = NULL;
	
	m_pBtn_Refresh = NULL;
	m_pBtn_Party = NULL;
	m_pBtn_Whisper = NULL;
	m_pBtn_Add = NULL;
	m_pBtn_Delete = NULL;
}

CUIFriends::~CUIFriends()
{
	this->SaveListToTextFile(""); // 몽땅 저장..
}

bool CUIFriends::Load(HANDLE hFile)
{
	if(false == CN3UIBase::Load(hFile)) return false;

	m_pList_Friends		= (CN3UIList*)this->GetChildByID("List_Friends");		__ASSERT(m_pList_Friends, "NULL UI Component!!");
	m_pText_Page		= (CN3UIString*)this->GetChildByID("String_Page");		__ASSERT(m_pText_Page, "NULL UI Component!!");

	m_pBtn_NextPage		= (CN3UIButton*)(this->GetChildByID("Btn_Page_Down"));	__ASSERT(m_pBtn_NextPage, "NULL UI Component!!");
	m_pBtn_PrevPage		= (CN3UIButton*)(this->GetChildByID("Btn_Page_Up"));	__ASSERT(m_pBtn_PrevPage, "NULL UI Component!!");
	
	m_pBtn_Refresh		= (CN3UIButton*)this->GetChildByID("Btn_Refresh");	__ASSERT(m_pBtn_Refresh, "NULL UI Component!!");
	m_pBtn_Party		= (CN3UIButton*)this->GetChildByID("Btn_Party");	__ASSERT(m_pBtn_Party, "NULL UI Component!!");
	m_pBtn_Whisper		= (CN3UIButton*)this->GetChildByID("Btn_Whisper");	__ASSERT(m_pBtn_Whisper, "NULL UI Component!!");
	m_pBtn_Add			= (CN3UIButton*)this->GetChildByID("Btn_Add");		__ASSERT(m_pBtn_Add, "NULL UI Component!!");
	m_pBtn_Delete		= (CN3UIButton*)this->GetChildByID("Btn_Delete");	__ASSERT(m_pBtn_Delete, "NULL UI Component!!");

	std::string szFN = CGameProcedure::s_szAccount + "_" + CGameProcedure::s_szServer + ".txt"; // 파일이름은 계정_서버.txt 로 한다.
	FILE* pFile = fopen(szFN.c_str(), "r");
	if (pFile)
	{
		char szLine[256] = "";
		char* pszResult = fgets(szLine, 256, pFile); // 줄을 읽고..
		while(pszResult)
		{
			int iLen = lstrlen(szLine);
			if(iLen > 3 && iLen <= 22)
			{
				std::string szTmp = szLine;
				int iTmp = szTmp.find("\r");
				if(iTmp >= 0) szTmp = szTmp.substr(0, iTmp);
				iTmp = szTmp.find("\n");
				if(iTmp >= 0) szTmp = szTmp.substr(0, iTmp);

				if(!szTmp.empty())
					this->MemberAdd(szTmp, -1, false, false);
			}
			pszResult = fgets(szLine, 256, pFile); // 첫째 줄을 읽고..
		}
		fclose(pFile);

		this->UpdateList();
	}

	return true;
}

void CUIFriends::SaveListToTextFile(const std::string& szID) // 문자열이 있으면 추가하고.. 없으면 몽땅 저장..
{
	// TEMP: to avoid the "_.txt" file in the data directory
	return;

	std::string szFN = CGameProcedure::s_szAccount + "_" + CGameProcedure::s_szServer + ".txt"; // 파일이름은 계정_서버.txt 로 한다.
	char szFlags[4] = "w";
	if(!szID.empty()) lstrcpy(szFlags, "a");
	FILE* pFile = fopen(szFN.c_str(), szFlags);
	if (NULL == pFile) return;

	std::string szIDTmp;
	if(szID.empty())
	{
		it_FI it = m_MapFriends.begin(), itEnd = m_MapFriends.end();
		for(; it != itEnd; it++)
		{
			szIDTmp = it->second.szName + "\r\n";
			fputs(szIDTmp.c_str(), pFile);
		}
	}
	else
	{
		szIDTmp = szID + "\r\n";
		fputs(szIDTmp.c_str(), pFile);
	}

	fclose(pFile);
}

bool CUIFriends::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	CGameProcMain* pProcMain = CGameProcedure::s_pProcMain;

	if (dwMsg == UIMSG_BUTTON_CLICK)					
	{
		if (pSender == m_pBtn_PrevPage
			|| pSender == m_pBtn_NextPage)
		{
			int iPagePrev = m_iPageCur;

			if (pSender == m_pBtn_PrevPage)
				m_iPageCur--;
			else
				m_iPageCur++;

			if (m_iPageCur < 0)
			{
				m_iPageCur = 0;
			}
			else
			{
				int iLinePerPage = 0;
				if (m_pList_Friends != nullptr)
				{
//					RECT rc = m_pList_Friends->GetRegion();
//					uint32_t dwH = m_pList_Friends->FontHeight();
//					iLinePerPage = (rc.bottom - rc.top) / dwH;
					iLinePerPage = 10;
				}

				int iPageMax = 0;
				if (iLinePerPage > 0)
					iPageMax = (static_cast<int>(m_MapFriends.size()) / iLinePerPage) + 1;
				
				if (m_iPageCur >= iPageMax)
					m_iPageCur = iPageMax - 1;
			}
			
			if(iPagePrev != m_iPageCur) // 페이지가 변경될때 
			{
				this->UpdateList();
				this->MsgSend_MemberInfo(false);
			}
		}
		else if(pSender == m_pBtn_Refresh) // 새 화면으로 갱신...
		{
			this->MsgSend_MemberInfo(true);
		}
		else if(pSender == m_pBtn_Add) // 추가.
		{
			CPlayerOther* pUPC = CGameProcedure::s_pOPMgr->UPCGetByID(CGameProcedure::s_pPlayer->m_iIDTarget, false);
			if(pUPC)
			{
				if(this->MemberAdd(pUPC->IDString(), pUPC->IDNumber(), true, false)) // 추가 성공이면..
				{
					this->SaveListToTextFile(pUPC->IDString()); // 파일에 추가 저장..
					this->MsgSend_MemberInfo(pUPC->IDString());
				}
			}
		}
		else if(pSender == m_pBtn_Delete) // 멤버 삭제
		{
			if(m_pList_Friends)
			{
				int iSel = m_pList_Friends->GetCurSel();
				std::string szID;
				m_pList_Friends->GetString(iSel, szID);

				if(this->MemberDelete(szID))
					this->UpdateList(); // 리스트 업데이트..
			}
		}
		else if(pSender == m_pBtn_Whisper) // 귓속말
		{
			if(m_pList_Friends)
			{
				int iSel = m_pList_Friends->GetCurSel();
				std::string szID;
				m_pList_Friends->GetString(iSel, szID);
				pProcMain->MsgSend_ChatSelectTarget(szID);
			}
		}
		else if(pSender == m_pBtn_Party) // 파티 신청
		{
			int iSel = m_pList_Friends->GetCurSel();
			std::string szID;
			m_pList_Friends->GetString(iSel, szID);
			it_FI it = m_MapFriends.find(szID);
			if(it != m_MapFriends.end())
			{
				std::string szMsg;
				if (pProcMain->MsgSend_PartyOrForceCreate(0, szID))
					szMsg = fmt::format_text_resource(IDS_PARTY_INVITE); // 파티
				else
					szMsg = fmt::format_text_resource(IDS_PARTY_INVITE_FAILED); // 파티 초대 실패
				pProcMain->MsgOutput(it->second.szName + szMsg, 0xffffff00);
			}
		}
	}

	return false;
}

bool CUIFriends::MemberAdd(const std::string &szID, int iID, bool bOnLine, bool bIsParty)
{
	if(szID.empty()) return false;
	if(m_MapFriends.find(szID) != m_MapFriends.end()) return false;

	__FriendsInfo FI;
	FI.szName = szID;
	FI.iID = iID;
	FI.bOnLine = bOnLine;
	FI.bIsParty = bIsParty;
	m_MapFriends.insert(val_FI(FI.szName, FI));

	return true;
}

bool CUIFriends::MemberDelete(const std::string &szID)
{
	it_FI it = m_MapFriends.find(szID);
	if(it == m_MapFriends.end()) return false;

	m_MapFriends.erase(it);

	return true;
}

void CUIFriends::UpdateList()
{
	if(NULL == m_pList_Friends) return;
	int iSelPrev = m_pList_Friends->GetCurSel();

	m_pList_Friends->ResetContent();
	if(m_MapFriends.empty()) return;

//	RECT rc = m_pList_Friends->GetRegion();
//	uint32_t dwH = m_pList_Friends->FontHeight();
//	int iLinePerPage = (rc.bottom - rc.top) / dwH;
	int iLinePerPage = 10;
//	if(iLinePerPage <= 0) return;

	int iPageMax = static_cast<int>(m_MapFriends.size()) / iLinePerPage;
	if (m_iPageCur < 0
		|| m_iPageCur > iPageMax)
		return;

	size_t iSkip = static_cast<size_t>(m_iPageCur * iLinePerPage);
	if (iSkip >= m_MapFriends.size())
		return;

	if(m_pText_Page) m_pText_Page->SetStringAsInt(m_iPageCur+1); // 페이지 표시..

	auto it = m_MapFriends.begin();
	std::advance(it, iSkip);

	for (int i = 0; i < iLinePerPage && it != m_MapFriends.end(); i++, it++)
	{
		__FriendsInfo& FI = it->second;
		int iIndex = m_pList_Friends->AddString(FI.szName);
		
		D3DCOLOR crStatus;
		if(FI.bOnLine)
		{
			if(FI.bIsParty) crStatus = 0xffff0000;
			else crStatus = 0xff00ff00;
		}
		else crStatus = 0xff808080;

		m_pList_Friends->SetFontColor(iIndex, crStatus);		
	}

	m_pList_Friends->SetCurSel(iSelPrev); // 전의 선택으로 돌리기..	
}

void CUIFriends::MsgSend_MemberInfo(bool bDisableInterval)
{
	float fTime = CN3Base::TimeGet();
	static float fTimePrev = 0;
	if(bDisableInterval) if(fTime < fTimePrev + 3.0f) return;
	fTimePrev = fTime;


	if(m_MapFriends.empty()) return;
	if(NULL == m_pList_Friends) return;

	int iFC = m_pList_Friends->GetCount();
	if(iFC <= 0) return;

	int iOffset = 0;
	std::vector<uint8_t> buffers(iFC * 32, 0);

	CAPISocket::MP_AddByte(&(buffers[0]), iOffset, WIZ_FRIEND_PROCESS); // 친구 정보.. Send s1(이름길이), str1(유저이름) | Receive s1(이름길이), str1(유저이름), s1(ID), b2(접속, 파티)
	CAPISocket::MP_AddShort(&(buffers[0]), iOffset, iFC);
	for(int i = 0; i < iFC; i++)
	{
		std::string szID;
		m_pList_Friends->GetString(i, szID);
		CAPISocket::MP_AddShort(&(buffers[0]), iOffset, (int16_t)szID.size());
		CAPISocket::MP_AddString(&(buffers[0]), iOffset, szID);
	}

	CGameProcedure::s_pSocket->Send(&(buffers[0]), iOffset);	
}

void CUIFriends::MsgSend_MemberInfo(const std::string& szID)
{
	if(szID.empty() || szID.size() > 20) return;
	int iFC = 1;

	int iOffset = 0;
	uint8_t byBuff[32];

	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_FRIEND_PROCESS); // 친구 정보.. Send s1(이름길이), str1(유저이름) | Receive s1(이름길이), str1(유저이름), s1(ID), b2(접속, 파티)
	CAPISocket::MP_AddShort(byBuff, iOffset, iFC);

	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)szID.size());
	CAPISocket::MP_AddString(byBuff, iOffset, szID);

	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
}

void CUIFriends::MsgRecv_MemberInfo(Packet& pkt)
{
	std::string szID;
	int iLen = 0;
	int iID = 0;
	uint8_t bStatus = 0;

	int iFC = pkt.read<int16_t>(); 
	for(int i = 0; i < iFC; i++)
	{
		iLen = pkt.read<int16_t>(); // 친구 정보.. Send s1(이름길이), str1(유저이름) | Receive s1(이름길이), str1(유저이름), s1(ID), b2(접속, 파티)
		pkt.readString(szID, iLen);
		iID = pkt.read<int16_t>(); 
		bStatus = pkt.read<uint8_t>();

		it_FI it = m_MapFriends.find(szID);
		if(it == m_MapFriends.end()) continue;

		__FriendsInfo& FI = it->second;
		FI.iID = iID;
		(bStatus & 0x01) ? FI.bOnLine = true : FI.bOnLine = false;
		(bStatus & 0x02) ? FI.bIsParty = true : FI.bIsParty = false;
	}
	this->UpdateList();
}



bool CUIQuest::Load(HANDLE hFile)
{
	if (!CN3UIBase::Load(hFile))
		return false;

	return true;
}

CUIQuest::CUIQuest()
{
}


CUIQuest::~CUIQuest()
{
}



CUIVarious::CUIVarious()
{
	m_pBtn_Knights = NULL;
	m_pBtn_State = NULL;
	m_pBtn_Quest = NULL;
	m_pBtn_Friends = NULL;
	m_pBtn_Close = NULL;

	m_pPageState = NULL;
	m_pPageKnights = NULL;
	m_pPageQuest = NULL;
	m_pPageFriends = NULL;

	m_bOpenningNow = false; // 열리고 있다..
	m_bClosingNow = false;	// 닫히고 있다..
	m_fMoveDelta = 0; // 부드럽게 열리고 닫히게 만들기 위해서 현재위치 계산에 부동소수점을 쓴다..
}

CUIVarious::~CUIVarious()
{
}

void CUIVarious::Release()
{
	CN3UIBase::Release();
	
	m_pBtn_Knights = NULL;
	m_pBtn_State = NULL;
	m_pBtn_Quest = NULL;
	m_pBtn_Friends = NULL;
	m_pBtn_Close = NULL;

	m_pPageState = NULL;
	m_pPageKnights = NULL;
	m_pPageQuest = NULL;
	m_pPageFriends = NULL;

	m_bOpenningNow = false; // 열리고 있다..
	m_bClosingNow = false;	// 닫히고 있다..
	m_fMoveDelta = 0; // 부드럽게 열리고 닫히게 만들기 위해서 현재위치 계산에 부동소수점을 쓴다..
}

bool CUIVarious::Load(HANDLE hFile)
{
	if(CN3UIBase::Load(hFile)==false) return false;

	m_pBtn_Knights = (CN3UIButton*)(this->GetChildByID("btn_clan"));		__ASSERT(m_pBtn_Knights, "NULL UI Component!!");
	m_pBtn_State =	(CN3UIButton*)(this->GetChildByID("Btn_State"));		__ASSERT(m_pBtn_State, "NULL UI Component!!");
	m_pBtn_Quest =	(CN3UIButton*)(this->GetChildByID("Btn_Quest"));		__ASSERT(m_pBtn_Quest, "NULL UI Component!!");
	m_pBtn_Friends = (CN3UIButton*)(this->GetChildByID("Btn_Friends"));		__ASSERT(m_pBtn_Friends, "NULL UI Component!!");
	m_pBtn_Close =	(CN3UIButton*)(this->GetChildByID("Btn_Close"));		__ASSERT(m_pBtn_Close, "NULL UI Component!!");

	// 아직 UI 가 안되어 있으니 막자..
	if(m_pBtn_Quest) m_pBtn_Quest->SetState(UI_STATE_BUTTON_DISABLE);
	if(m_pBtn_Friends) m_pBtn_Friends->SetState(UI_STATE_BUTTON_DISABLE);

	if(NULL == m_pPageState) m_pPageState = new CUIState();
	else m_pPageState->Release();
	m_pPageState->Init(this);

	if(NULL == m_pPageKnights) m_pPageKnights = new CUIKnights();
	else m_pPageKnights->Release();
	m_pPageKnights->Init(this);
	
	m_pPageQuest = NULL;
	if(NULL == m_pPageQuest) m_pPageQuest = new CUIQuest();
	else m_pPageQuest->Release();
	m_pPageQuest->Init(this);
	
	m_pPageFriends = NULL;
	if(NULL == m_pPageFriends) m_pPageFriends = new CUIFriends();
	else m_pPageFriends->Release();
	m_pPageFriends->Init(this);
	
	this->UpdatePageButtons(m_pBtn_State);

	return true;
}

bool CUIVarious::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)					
	{
		if(pSender == m_pBtn_Close)			this->Close(); // 닫는다..
		else if(pSender == m_pBtn_State)	this->UpdatePageButtons(m_pBtn_State);
		else if(pSender == m_pBtn_Quest)	this->UpdatePageButtons(m_pBtn_Quest);		// 퀘스트...
		else if(pSender == m_pBtn_Knights)	this->UpdatePageButtons(m_pBtn_Knights);	// 기사단... 잠시 막자..
		else if(pSender == m_pBtn_Friends)	this->UpdatePageButtons(m_pBtn_Friends);
	}

	return true;
}

void CUIVarious::UpdatePageButtons(CN3UIButton* pButtonToActive)
{
	static CN3UIButton* pButtonPrev = NULL;
	if(NULL == pButtonToActive || pButtonToActive == pButtonPrev) return;
	pButtonPrev = pButtonToActive;

	CN3UIButton*	pBtns[4] = { m_pBtn_Knights, m_pBtn_State, m_pBtn_Quest, m_pBtn_Friends };
	CN3UIBase*		pPages[4] = { m_pPageKnights, m_pPageState, m_pPageQuest, m_pPageFriends };
	
	for(int i = 0; i < 4; i++)
	{
		if(NULL == pBtns[i]) continue;

		if(pBtns[i] == pButtonToActive)
		{
			pBtns[i]->SetState(UI_STATE_BUTTON_DOWN);
			if(pPages[i]) pPages[i]->SetVisible(true);
		}
		else
		{
			pBtns[i]->SetState(UI_STATE_BUTTON_NORMAL);
			if(pPages[i]) pPages[i]->SetVisible(false);
		}
	}

	if(pButtonToActive == m_pBtn_Friends && m_pPageFriends)
		m_pPageFriends->MsgSend_MemberInfo(false); // 이러면 친구리스트를 업데이트한다..
}

void CUIVarious::Open()
{
	// 스르륵 열린다!!
	this->SetVisible(true);
	RECT rc = this->GetRegion();
	this->SetPos(-(rc.right - rc.left), 80);
	m_fMoveDelta = 0;
	m_bOpenningNow = true;
	m_bClosingNow = false;

	// 기사단 리스트가 없으면 요청해서 받는다.
//	__InfoPlayerMySelf*	pInfoExt = &(CGameBase::s_pPlayer->m_InfoExt);
//	if(m_pPageKnights->NeedMemberListRequest() && pInfoExt->iKnightsID > 0)
//	{
//		m_pPageKnights->MsgSend_MemberInfoOnline(0);
//	}
//	// 기사단장이거나 간부급이면...UI 가 달라야 한다..
//	m_pPageKnights->ChangeUIByDuty(pInfoExt->eKnightsDuty); // 권한에 따라 UI 변경..
}

void CUIVarious::Close()
{
	// 스르륵 닫힌다..!!
//	SetVisible(false); // 다 닫히고 나서 해준다..
	this->SetPos(0, 80);
	m_fMoveDelta = 0;
	m_bOpenningNow = false;
	m_bClosingNow = true;

	if(m_pSnd_CloseUI) m_pSnd_CloseUI->Play(); // 닫는 소리..
}

void CUIVarious::Tick()
{
	if(m_pPageKnights)
	{
		m_pPageKnights->m_fTimeLimit_Admit += CN3Base::s_fSecPerFrm;
		m_pPageKnights->m_fTimeLimit_Appoint += CN3Base::s_fSecPerFrm;
		m_pPageKnights->m_fTimeLimit_Refresh += CN3Base::s_fSecPerFrm;
		m_pPageKnights->m_fTimeLimit_Remove += CN3Base::s_fSecPerFrm;
	}

	if(m_bOpenningNow) // 오른쪽에서 왼쪽으로 스르륵...열려야 한다면..
	{
		POINT ptCur = this->GetPos();
		RECT rc = this->GetRegion();
		float fWidth = (float)(rc.right - rc.left);

		float fDelta = 5000.0f * CN3Base::s_fSecPerFrm;
		fDelta *= (fWidth - m_fMoveDelta) / fWidth;
		if(fDelta < 2.0f) fDelta = 2.0f;
		m_fMoveDelta += fDelta;

		int iXLimit = 0;
		ptCur.x = (int)(m_fMoveDelta - fWidth);
		if(ptCur.x >= iXLimit) // 다열렸다!!
		{
			ptCur.x = iXLimit;
			m_bOpenningNow = false;
		}

		this->SetPos(ptCur.x, ptCur.y);
	}
	else if(m_bClosingNow) // 오른쪽에서 왼쪽으로 스르륵...열려야 한다면..
	{
		POINT ptCur = this->GetPos();
		RECT rc = this->GetRegion();
		float fWidth = (float)(rc.right - rc.left);

		float fDelta = 5000.0f * CN3Base::s_fSecPerFrm;
		fDelta *= (fWidth - m_fMoveDelta) / fWidth;
		if(fDelta < 2.0f) fDelta = 2.0f;
		m_fMoveDelta += fDelta;

		int iXLimit = (int)-fWidth;
		ptCur.x = (int)-m_fMoveDelta;
		if(ptCur.x <= iXLimit) // 다 닫혔다..!!
		{
			ptCur.x = iXLimit;
			m_bClosingNow = false;

			this->SetVisibleWithNoSound(false, false, true); // 다 닫혔으니 눈에서 안보이게 한다.
			CGameProcedure::s_pUIMgr->ReFocusUI();//this_ui
		}

		this->SetPos(ptCur.x, ptCur.y);
	}

	CN3UIBase::Tick();
}

void CUIVarious::UpdateAllStates(const __InfoPlayerBase* pInfoBase, const __InfoPlayerMySelf* pInfoExt)
{
	if(NULL == pInfoBase || NULL == pInfoExt) return;
	
	std::string szVal;
	
	if(m_pPageState->m_pText_Class) // 직업
	{
		CGameProcedure::GetTextByClass(pInfoBase->eClass, szVal);
		m_pPageState->m_pText_Class->SetString(szVal);
	}

	// 종족
	if(m_pPageState->m_pText_Race) 
	{
		CGameProcedure::GetTextByRace(pInfoBase->eRace, szVal);
		m_pPageState->m_pText_Race->SetString(szVal);
	}
	
	// 국가
	if(m_pPageState->m_pText_Nation)
	{
		CGameProcedure::GetTextByNation(pInfoBase->eNation, szVal);
		m_pPageState->m_pText_Nation->SetString(szVal);
	}

	m_pPageState->UpdateLevel(pInfoBase->iLevel);
	m_pPageState->UpdateExp(pInfoExt->iExp, pInfoExt->iExpNext);
	m_pPageState->UpdateHP(pInfoBase->iHP, pInfoBase->iHPMax);
	m_pPageState->UpdateMSP(pInfoExt->iMSP, pInfoExt->iMSPMax);
	m_pPageState->UpdateWeight(pInfoExt->iWeight, pInfoExt->iWeightMax);
	
	m_pPageState->UpdateAttackPoint(pInfoExt->iAttack, pInfoExt->iAttack_Delta);
	m_pPageState->UpdateGuardPoint(pInfoExt->iGuard, pInfoExt->iGuard_Delta);
	m_pPageState->UpdateBonusPointAndButtons(pInfoExt->iBonusPointRemain);  // 보너스 포인트 적용이 가능한가??
	
	m_pPageState->UpdateStrength(pInfoExt->iStrength, pInfoExt->iStrength_Delta);
	m_pPageState->UpdateStamina(pInfoExt->iStamina, pInfoExt->iStamina_Delta);
	m_pPageState->UpdateDexterity(pInfoExt->iDexterity, pInfoExt->iDexterity_Delta);
	m_pPageState->UpdateIntelligence(pInfoExt->iIntelligence, pInfoExt->iIntelligence_Delta);
	m_pPageState->UpdateMagicAttak(pInfoExt->iMagicAttak, pInfoExt->iMagicAttak_Delta);
	
	m_pPageState->UpdateRegistFire(pInfoExt->iRegistFire, pInfoExt->iRegistFire_Delta);
	m_pPageState->UpdateRegistCold(pInfoExt->iRegistCold, pInfoExt->iRegistCold_Delta);
	m_pPageState->UpdateRegistMagic(pInfoExt->iRegistMagic, pInfoExt->iRegistMagic_Delta);
	m_pPageState->UpdateRegistCurse(pInfoExt->iRegistCurse, pInfoExt->iRegistCurse_Delta);
	m_pPageState->UpdateRegistLight(pInfoExt->iRegistLight, pInfoExt->iRegistLight_Delta);
	m_pPageState->UpdateRegistPoison(pInfoExt->iRegistPoison, pInfoExt->iRegistPoison_Delta);

	// 기사단 관련 정보 업데이트...
	m_pPageState->UpdateRealmPoint(pInfoExt->iRealmPoint, pInfoExt->iRealmPointMonthly); // Edited by @Demircivi while integrating monthly np system.  // 국가 기여도는 10을 나누어서 표시

	// 캐릭터 능력치 포인트 이미지 업데이트..
	if (m_pPageState->m_pImg_Str) m_pPageState->m_pImg_Str->SetVisible(false);
	if (m_pPageState->m_pImg_Sta) m_pPageState->m_pImg_Sta->SetVisible(false);
	if (m_pPageState->m_pImg_Dex) m_pPageState->m_pImg_Dex->SetVisible(false);
	if (m_pPageState->m_pImg_Int) m_pPageState->m_pImg_Int->SetVisible(false);
	if (m_pPageState->m_pImg_MAP) m_pPageState->m_pImg_MAP->SetVisible(false);

	switch ( pInfoBase->eClass )
	{
		case CLASS_KA_WARRIOR:
		case CLASS_KA_BERSERKER:
		case CLASS_EL_WARRIOR:
		case CLASS_EL_BLADE:
			if (m_pPageState->m_pImg_Str) m_pPageState->m_pImg_Str->SetVisible(true);
			if (m_pPageState->m_pImg_Sta) m_pPageState->m_pImg_Sta->SetVisible(true);
			break;

		case CLASS_KA_ROGUE:
		case CLASS_KA_HUNTER:
		case CLASS_EL_ROGUE:
		case CLASS_EL_RANGER:
			if (m_pPageState->m_pImg_Sta) m_pPageState->m_pImg_Sta->SetVisible(true);
			if (m_pPageState->m_pImg_Dex) m_pPageState->m_pImg_Dex->SetVisible(true);
			break;

		case CLASS_KA_WIZARD:
		case CLASS_KA_SORCERER:
		case CLASS_EL_WIZARD:
		case CLASS_EL_MAGE:
			if (m_pPageState->m_pImg_Int) m_pPageState->m_pImg_Int->SetVisible(true);
			if (m_pPageState->m_pImg_MAP) m_pPageState->m_pImg_MAP->SetVisible(true);
			break;

		case CLASS_KA_PRIEST:
		case CLASS_KA_SHAMAN:
		case CLASS_EL_PRIEST:
		case CLASS_EL_CLERIC:
			if (m_pPageState->m_pImg_Str) m_pPageState->m_pImg_Str->SetVisible(true);
			if (m_pPageState->m_pImg_Int) m_pPageState->m_pImg_Int->SetVisible(true);
			break;
	}
}

void CUIVarious::UpdateKnightsInfo()
{
	if(NULL == m_pPageKnights) return;

	/*
		m_pPageKnights->UpdateKnightsDuty(CGameProcedure::s_pPlayer->m_InfoExt.eKnightsDuty);
		m_pPageKnights->UpdateKnightsName(CGameProcedure::s_pPlayer->m_InfoExt.szKnights);
		m_pPageKnights->UpdateKnightsGrade(CGameProcedure::s_pPlayer->m_InfoExt.iKnightsGrade);
		m_pPageKnights->UpdateKnightsRank(CGameProcedure::s_pPlayer->m_InfoExt.iKnightsRank);
	*/

	m_pPageKnights->UpdateExceptList();
}

bool CUIVarious::OnKeyPress(int iKey)
{
	if(DIK_ESCAPE == iKey)
	{
		if(!m_bClosingNow) this->Close();
		return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

void CUIVarious::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);
	if(bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
		CGameProcedure::s_pUIMgr->ReFocusUI();//this_ui
}

void CUIVarious::SetVisibleWithNoSound(bool bVisible, bool bWork, bool bReFocus)
{
	CN3UIBase::SetVisibleWithNoSound(bVisible, bWork, bReFocus);

	if(bReFocus)
	{
		if(bVisible)
			CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
		else
			CGameProcedure::s_pUIMgr->ReFocusUI();//this_ui
	}
}

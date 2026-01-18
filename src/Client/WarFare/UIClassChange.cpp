// UIClassChange.cpp: implementation of the CUIClassChange class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UIClassChange.h"
#include "PacketDef.h"
#include "PlayerMySelf.h"
#include "GameProcMain.h"
#include "UISkillTreeDlg.h"
#include "APISocket.h"
#include "UIVarious.h"
#include "UIHotKeyDlg.h"
#include "text_resources.h"

#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIString.h>

CUIClassChange::CUIClassChange()
{
	m_pBtn_Ok       = nullptr;
	m_pBtn_Cancel   = nullptr;
	m_pBtn_Class    = nullptr;

	m_pText_Warning = nullptr;
	m_pText_Info    = nullptr;
	m_pText_Title   = nullptr;
	m_pText_Message = nullptr;

	m_eClass        = CLASS_UNKNOWN;
}

CUIClassChange::~CUIClassChange()
{
}

bool CUIClassChange::Load(File& file)
{
	if (!CN3UIBase::Load(file))
		return false;

	N3_VERIFY_UI_COMPONENT(m_pBtn_Ok, GetChildByID<CN3UIButton>("Btn_Ok"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Cancel, GetChildByID<CN3UIButton>("Btn_Cancel"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Class, GetChildByID<CN3UIButton>("Btn_Class"));

	N3_VERIFY_UI_COMPONENT(m_pText_Warning, GetChildByID<CN3UIString>("Text_Waring"));
	N3_VERIFY_UI_COMPONENT(m_pText_Info, GetChildByID<CN3UIString>("Text_info"));
	N3_VERIFY_UI_COMPONENT(m_pText_Message, GetChildByID<CN3UIString>("Text_Message"));

	return true;
}

void CUIClassChange::Open(int iCode)
{
	SetVisible(true);

	__InfoPlayerBase* pInfoBase = &CGameBase::s_pPlayer->m_InfoBase;

	std::string szSuccess, szNotYet, szAlready, szItemInSlot;
	szSuccess    = fmt::format_text_resource(IDS_CLASS_CHANGE_SUCCESS);
	szNotYet     = fmt::format_text_resource(IDS_CLASS_CHANGE_NOT_YET);
	szAlready    = fmt::format_text_resource(IDS_CLASS_CHANGE_ALREADY);
	szItemInSlot = fmt::format_text_resource(IDS_MSG_HASITEMINSLOT);

	m_pBtn_Ok->SetVisible(false);
	m_pBtn_Cancel->SetVisible(false);
	m_pBtn_Class->SetVisible(false);

	m_pText_Warning->SetVisible(false);
	m_pText_Info->SetVisible(false);
	m_pText_Message->SetVisible(true);

	std::string szClassTmp;

	switch (iCode)
	{
		case N3_SP_CLASS_CHANGE_SUCCESS:
			m_pText_Message->SetString(szSuccess);
			m_pBtn_Class->SetVisible(true);
			m_pBtn_Cancel->SetVisible(true);

			m_pText_Info->SetVisible(true);

			m_eClass = pInfoBase->eClass;
			switch (pInfoBase->eClass)
			{
				case CLASS_KA_WARRIOR:
					CGameBase::GetTextByClass(CLASS_KA_BERSERKER, szClassTmp);
					m_pText_Info->SetString(szClassTmp);
					break;
				case CLASS_KA_ROGUE:
					CGameBase::GetTextByClass(CLASS_KA_HUNTER, szClassTmp);
					m_pText_Info->SetString(szClassTmp);
					break;
				case CLASS_KA_WIZARD:
					CGameBase::GetTextByClass(CLASS_KA_SORCERER, szClassTmp);
					m_pText_Info->SetString(szClassTmp);
					break;
				case CLASS_KA_PRIEST:
					CGameBase::GetTextByClass(CLASS_KA_SHAMAN, szClassTmp);
					m_pText_Info->SetString(szClassTmp);
					break;
				case CLASS_EL_WARRIOR:
					CGameBase::GetTextByClass(CLASS_EL_BLADE, szClassTmp);
					m_pText_Info->SetString(szClassTmp);
					break;
				case CLASS_EL_ROGUE:
					CGameBase::GetTextByClass(CLASS_EL_RANGER, szClassTmp);
					m_pText_Info->SetString(szClassTmp);
					break;
				case CLASS_EL_WIZARD:
					CGameBase::GetTextByClass(CLASS_EL_MAGE, szClassTmp);
					m_pText_Info->SetString(szClassTmp);
					break;
				case CLASS_EL_PRIEST:
					CGameBase::GetTextByClass(CLASS_EL_CLERIC, szClassTmp);
					m_pText_Info->SetString(szClassTmp);
					break;
				default:
					break;
			}
			break;

		case N3_SP_CLASS_CHANGE_NOT_YET:
			m_pText_Message->SetString(szNotYet);
			m_pBtn_Ok->SetVisible(true);
			break;

		case N3_SP_CLASS_CHANGE_ALREADY:
			m_pText_Message->SetString(szAlready);
			m_pBtn_Ok->SetVisible(true);
			break;

		case IDS_MSG_HASITEMINSLOT: // TODO: FIXME. This is not a valid subopcode!
			m_pText_Message->SetString(szItemInSlot);
			m_pBtn_Ok->SetVisible(true);
			break;

		default:
			break;
	}
}

bool CUIClassChange::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	__InfoPlayerBase* pInfoBase  = &(CGameBase::s_pPlayer->m_InfoBase);
	__InfoPlayerMySelf* pInfoExt = &(CGameBase::s_pPlayer->m_InfoExt);

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtn_Ok || pSender == m_pBtn_Cancel)
		{
			Close();
		}
		else if (pSender == m_pBtn_Class)
		{
			switch (pInfoBase->eClass)
			{
				case CLASS_KA_WARRIOR:
					pInfoBase->eClass = CLASS_KA_BERSERKER;
					break;
				case CLASS_KA_ROGUE:
					pInfoBase->eClass = CLASS_KA_HUNTER;
					break;
				case CLASS_KA_WIZARD:
					pInfoBase->eClass = CLASS_KA_SORCERER;
					break;
				case CLASS_KA_PRIEST:
					pInfoBase->eClass = CLASS_KA_SHAMAN;
					break;
				case CLASS_EL_WARRIOR:
					pInfoBase->eClass = CLASS_EL_BLADE;
					break;
				case CLASS_EL_ROGUE:
					pInfoBase->eClass = CLASS_EL_RANGER;
					break;
				case CLASS_EL_WIZARD:
					pInfoBase->eClass = CLASS_EL_MAGE;
					break;
				case CLASS_EL_PRIEST:
					pInfoBase->eClass = CLASS_EL_CLERIC;
					break;
				default:
					break;
			}

			CGameProcedure::s_pProcMain->m_pUIVar->UpdateAllStates(pInfoBase, pInfoExt); // 상태창 수치를 모두 적용

			uint8_t byBuff[4];
			int iOffset = 0;
			CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_CLASS_CHANGE);
			CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_CLASS_CHANGE_REQ);
			CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t) pInfoBase->eClass);
			CGameProcedure::s_pSocket->Send(byBuff, iOffset);

			CGameProcedure::s_pProcMain->m_pUISkillTreeDlg->InitIconUpdate();

			// 전직하는 순간..  핫키 정보를 모두 없앤다..
			CGameProcedure::s_pProcMain->m_pUIHotKeyDlg->ClassChangeHotkeyFlush();
			Close();
		}
	}

	return true;
}

void CUIClassChange::Close()
{
	SetVisible(false);
}

void CUIClassChange::RestorePrevClass()
{
	__InfoPlayerBase* pInfoBase  = &CGameBase::s_pPlayer->m_InfoBase;
	__InfoPlayerMySelf* pInfoExt = &CGameBase::s_pPlayer->m_InfoExt;

	pInfoBase->eClass            = m_eClass;
	CGameProcedure::s_pProcMain->m_pUISkillTreeDlg->InitIconUpdate();

	CGameProcedure::s_pProcMain->m_pUIVar->UpdateAllStates(pInfoBase, pInfoExt); // 상태창 수치를 모두 적용
}

void CUIClassChange::ChangeToNormalState()
{
	m_pBtn_Ok->SetVisible(false);
	m_pBtn_Cancel->SetVisible(true);
	m_pBtn_Class->SetVisible(true);

	m_pText_Warning->SetVisible(false);
	m_pText_Info->SetVisible(true);
	m_pText_Message->SetVisible(true);
}

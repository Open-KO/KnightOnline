#include "StdAfx.h"
#include "UIExitMenu.h"
#include "GameProcMain.h"
#include "UIManager.h"
#include "N3FXMgr.h"
#include "PlayerMySelf.h"
#include "UIChat.h"
#include "text_resources.h"
#include "PacketDef.h"
#include "APISocket.h"

#include <N3Base/N3UIButton.h>

#include <shellapi.h>

CUIExitMenu::CUIExitMenu()
{
	m_pBtn_Chr    = nullptr;
	m_pBtn_Option = nullptr;
	m_pBtn_Exit   = nullptr;
	m_pBtn_Cancel = nullptr;
}

CUIExitMenu::~CUIExitMenu()
{
}

void CUIExitMenu::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);

	if (bVisible)
	{
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	}
	else
	{
		CGameProcedure::s_pUIMgr->ReFocusUI();

		if (m_pBtn_Chr != nullptr)
			m_pBtn_Chr->SetState(UI_STATE_BUTTON_NORMAL);

		if (m_pBtn_Option != nullptr)
			m_pBtn_Option->SetState(UI_STATE_BUTTON_NORMAL);

		if (m_pBtn_Exit != nullptr)
			m_pBtn_Exit->SetState(UI_STATE_BUTTON_NORMAL);

		if (m_pBtn_Cancel != nullptr)
			m_pBtn_Cancel->SetState(UI_STATE_BUTTON_NORMAL);
	}
}

bool CUIExitMenu::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (pSender == nullptr)
		return false;

	if (dwMsg != UIMSG_BUTTON_CLICK)
		return true;

	if (pSender == m_pBtn_Chr)
	{
		if (CGameProcedure::s_pProcMain == nullptr)
			return true;

		if (CGameProcedure::s_pProcMain->m_fExitTimer == -1.0f)
		{
			/*
			if (CGameProcedure::s_pProcMain->m_pUIHotKeyDlg != nullptr)
			{
				CUIHotKeyDlg::MsgSend_SkillData_Save();
			}
			*/

			//CGameProcedure::s_pProcMain->m_bSeekingPartyChatEnabled = false;
			//CGameProcedure::s_pProcMain->m_bPendingPremiumCafeData = true;

			std::string szMsg = fmt::format_text_resource(IDS_CONNECTING_PLEASE_WAIT);
			CGameProcedure::MessageBoxPost(szMsg, "", MB_OK);
			ReturnToCharacterSelection();
		}
		else if (CGameProcedure::s_pProcMain->m_pUIChatDlg != nullptr)
		{
			std::string szMsg = fmt::format_text_resource(IDS_CANNOT_EXIT_DURING_A_BATTLE);
			CGameProcedure::s_pProcMain->m_pUIChatDlg->AddChatMsg(N3_CHAT_NORMAL, szMsg, 0xFFFF0000);
			CGameProcedure::s_pProcMain->m_eExitType = EXIT_TYPE_CHR_SELECT;
			SetVisible(false);
		}

		return true;
	}

	if (pSender == m_pBtn_Option)
	{
		if (CGameProcedure::s_pProcMain == nullptr)
			return true;

		if (CGameProcedure::s_pProcMain->m_fExitTimer == -1.0f)
		{
			/*
			if (CGameProcedure::s_pProcMain->m_pUIHotKeyDlg != nullptr)
				CUIHotKeyDlg::MsgSend_SkillData_Save();
			*/

			::ShellExecute(nullptr, "open", "Option.exe", nullptr, nullptr, SW_SHOWNORMAL);
			PostQuitMessage(0);
		}
		else if (CGameProcedure::s_pProcMain->m_pUIChatDlg != nullptr)
		{
			std::string szMsg = fmt::format_text_resource(IDS_CANNOT_EXIT_DURING_A_BATTLE);
			CGameProcedure::s_pProcMain->m_pUIChatDlg->AddChatMsg(N3_CHAT_NORMAL, szMsg, 0xFFFF0000);
			CGameProcedure::s_pProcMain->m_eExitType = EXIT_TYPE_QUIT;
			SetVisible(false);
		}
	}
	else if (pSender == m_pBtn_Exit)
	{
		if (CGameProcedure::s_pProcMain == nullptr)
			return true;

		if (CGameProcedure::s_pProcMain->m_fExitTimer == -1.0f)
		{
			/*
			if (CGameProcedure::s_pProcMain->_.m_pUIHotKeyDlg != nullptr)
				CUIHotKeyDlg::MsgSend_SkillData_Save();
			*/

			PostQuitMessage(0);
		}
		else if (CGameProcedure::s_pProcMain->m_pUIChatDlg != nullptr)
		{
			std::string szMsg = fmt::format_text_resource(IDS_CANNOT_EXIT_DURING_A_BATTLE);
			CGameProcedure::s_pProcMain->m_pUIChatDlg->AddChatMsg(N3_CHAT_NORMAL, szMsg, 0xFFFF0000);
			CGameProcedure::s_pProcMain->m_eExitType = EXIT_TYPE_QUIT;

			SetVisible(false);
		}
	}
	else if (pSender == m_pBtn_Cancel)
	{
		SetVisible(false);
	}

	return true;
}

void CUIExitMenu::ReturnToCharacterSelection()
{
	if (CGameBase::s_pPlayer != nullptr)
	{
		if (CGameProcedure::s_pFX != nullptr)
		{
			//CGameBase::s_pPlayer->m_bStopTargetPointerAfterTime = false;
			//CGameBase::s_pPlayer->m_fStopTargetPointerAfterTime = 0.0f;
			CGameProcedure::s_pFX->Stop(
				CGameBase::s_pPlayer->IDNumber(), CGameBase::s_pPlayer->IDNumber(), FXID_TARGET_POINTER, FXID_TARGET_POINTER, true);
		}

		/*
		if (CGameBase::s_pPlayer->m_bIsMerchanting)
		{
			CGameBase::s_pPlayer->InitMerchant(false, nullptr);

			if (CGameProcedure::s_pProcMain->m_pUITradeInventory != nullptr)
				CGameProcedure::s_pProcMain->m_pUITradeInventory->Close();
		}
		*/
	}

	std::string szIP                              = CGameProcedure::s_pSocket->GetCurrentIP();
	DWORD dwPort                                  = CGameProcedure::s_pSocket->GetCurrentPort();

	CGameProcedure::s_bNeedReportConnectionClosed = false;
	CGameProcedure::s_pSocket->Disconnect();
	Sleep(4000);

	CGameProcedure::s_pSocket->Connect(s_hWndBase, szIP, dwPort);
	CGameProcedure::s_bNeedReportConnectionClosed = true;
	CGameProcedure::s_bIsRestarting               = true;
	CGameProcedure::ProcActiveSet((CGameProcedure*) CGameProcedure::s_pProcCharacterSelect);
}

bool CUIExitMenu::Load(File& file)
{
	if (!CN3UIBase::Load(file))
		return false;

	N3_VERIFY_UI_COMPONENT(m_pBtn_Chr, GetChildByID<CN3UIButton>("btn_chr"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Option, GetChildByID<CN3UIButton>("btn_option"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Exit, GetChildByID<CN3UIButton>("btn_exit"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Cancel, GetChildByID<CN3UIButton>("btn_cancel"));

	return true;
}

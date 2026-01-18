// UIMessageBox.cpp: implementation of the CUIMessageBox class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UIMessageBox.h"
#include "GameProcMain.h"
#include "PlayerMySelf.h"
#include "GameProcCharacterSelect.h"
#include "UIKnightsOperation.h"
#include "UICreateClanName.h"
#include "UIPartyBBS.h"
#include "GameEng.h"
#include "GameProcLogIn.h"
#include "LocalInput.h"
#include "UIMessageBoxManager.h"
#include "SubProcPerTrade.h"
#include "APISocket.h"

#include <shellapi.h>

#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIString.h>
#include <N3Base/N3UIEdit.h>
#include <N3Base/N3Shape.h>

CUIMessageBox::CUIMessageBox()
{
	m_pBtn_OK       = nullptr;
	m_pBtn_Yes      = nullptr;
	m_pBtn_No       = nullptr;
	m_pBtn_Cancel   = nullptr;
	m_pText_Message = nullptr;
	m_pText_Title   = nullptr;
	m_pEdit_Common  = nullptr;

	m_eBehavior     = BEHAVIOR_NOTHING;
	m_iStyle        = MB_OK;
}

CUIMessageBox::~CUIMessageBox()
{
}

bool CUIMessageBox::Load(File& file)
{
	if (CN3UIBase::Load(file) == false)
		return false;

	N3_VERIFY_UI_COMPONENT(m_pBtn_OK, GetChildByID<CN3UIButton>("Btn_OK"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Yes, GetChildByID<CN3UIButton>("Btn_Yes"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_No, GetChildByID<CN3UIButton>("Btn_No"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Cancel, GetChildByID<CN3UIButton>("Btn_Cancel"));
	N3_VERIFY_UI_COMPONENT(m_pText_Message, GetChildByID<CN3UIString>("Text_Message"));
	N3_VERIFY_UI_COMPONENT(m_pText_Title, GetChildByID<CN3UIString>("Text_Title"));
	N3_VERIFY_UI_COMPONENT(m_pEdit_Common, GetChildByID<CN3UIEdit>("Edit_Common"));

	return true;
}

void CUIMessageBox::SetText(const std::string& szMsg)
{
	m_pText_Message->SetString(szMsg);
}

void CUIMessageBox::SetTitle(const std::string& szTitle)
{
	m_pText_Title->SetString(szTitle);
}

bool CUIMessageBox::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (nullptr == pSender)
		return false;

	//s_CameraData.vp;  //불러 오는 과정을 살펴본다
	//uint32_t mm = s_CameraData.vp.Height;
	//uint32_t ss = s_CameraData.vp.Width;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		// CPlayerMySelf* pPlayer                 = CGameBase::s_pPlayer;
		CGameProcMain* pProcMain                 = CGameProcedure::s_pProcMain;
		// CGameProcLogIn* pProcLogIn             = CGameProcedure::s_pProcLogIn;
		CGameProcCharacterSelect* pProcChrSelect = CGameProcedure::s_pProcCharacterSelect;
		CN3Shape* pShape                         = CGameBase::s_pPlayer->m_pObjectTarget;

		if (pSender == m_pBtn_OK || pSender == m_pBtn_Yes)
		{
			//this_ui
			if (m_pParentUI)
				m_pParentUI->CallBackProc(m_iChildID, 1);

			switch (m_eBehavior)
			{
				case BEHAVIOR_EXIT:
				{
					if (CGameProcedure::s_pProcActive == pProcMain) // 지금 메인 프로시저이면..
						pProcMain->ReleaseSound();

					::PostQuitMessage(0);
				}
				break;                                              // 끝낸다..
				case BEHAVIOR_RESTART_GAME:
				{
					if (CGameProcedure::s_pProcActive == pProcMain) // 지금 메인 프로시저이면..
					{
						std::string szIP                              = CGameProcedure::s_pSocket->GetCurrentIP();
						uint32_t dwPort                               = CGameProcedure::s_pSocket->GetCurrentPort();

						CGameProcedure::s_bNeedReportConnectionClosed = false; // 서버접속이 끊어진걸 보고해야 하는지..
						CGameProcedure::s_pSocket->Disconnect();
						Sleep(2000);                                           // 2초 딜레이.. 서버가 처리할 시간을 준다.
						CGameProcedure::s_pSocket->Connect(s_hWndBase, szIP, dwPort);
						CGameProcedure::s_bNeedReportConnectionClosed = true;  // 서버접속이 끊어진걸 보고해야 하는지..

						CGameProcedure::MsgSend_GameServerLogIn();
						CGameProcedure::ProcActiveSet((CGameProcedure*) CGameProcedure::s_pProcCharacterSelect); // 다시 캐릭터 고르자..
					}
				}
				break;
				case BEHAVIOR_REGENERATION:
					pProcMain->MsgSend_Regen();
					break; // 부활 메시지 날리기..
				case BEHAVIOR_PARTY_PERMIT:
					pProcMain->MsgSend_PartyOrForcePermit(true);
					break;
				case BEHAVIOR_PARTY_DISBAND:
					pProcMain->MsgSend_PartyOrForceLeave();
					break;
				case BEHAVIOR_REQUEST_BINDPOINT:
					if (pShape)
						pProcMain->MsgSend_ObjectEvent(pShape->m_iEventID, 0);
					break;
				case BEHAVIOR_KNIGHTS_CREATE:
					pProcMain->m_pUICreateClanName->MsgSend_MakeClan();
					break;
				case BEHAVIOR_KNIGHTS_DESTROY:
					pProcMain->m_pUIKnightsOp->MsgSend_KnightsDestroy();
					break;                 // 기사단 뽀개기..
				case BEHAVIOR_KNIGHTS_WITHDRAW:
					pProcMain->m_pUIKnightsOp->MsgSend_KnightsWithdraw();
					break;                 // 기사단 탈퇴하기..
				case BEHAVIOR_PERSONAL_TRADE_PERMIT:
					pProcMain->m_pSubProcPerTrade->ProcessProceed(PER_TRADE_RESULT_MY_AGREE);
					break;                 // 내가 허락..
				case BEHAVIOR_MGAME_LOGIN: /*pProcLogIn->MsgSend_AccountLogIn(LIC_MGAME);*/
					break;
				case BEHAVIOR_DELETE_CHR:  // 캐릭터 지우기..
				{
					std::string szKey;
					if (m_pEdit_Common)
						szKey = m_pEdit_Common->GetString();
					pProcChrSelect->MsgSend_DeleteChr(szKey);
				}
				break;
				case BEHAVIOR_CLAN_JOIN:
					pProcMain->MsgSend_KnightsJoinReq(true);
					break;
				case BEHAVIOR_PARTY_BBS_REGISTER:
					if (pProcMain->m_pUIPartyBBS)
						pProcMain->m_pUIPartyBBS->MsgSend_Register();
					break; // 파티 게시판에 등록
				case BEHAVIOR_PARTY_BBS_REGISTER_CANCEL:
					if (pProcMain->m_pUIPartyBBS)
						pProcMain->m_pUIPartyBBS->MsgSend_RegisterCancel();
					break;                                                                          // 파티 게시판에 등록 해제
				case BEHAVIOR_EXECUTE_OPTION:
				{
					::ShellExecute(nullptr, "open", "Option.exe", nullptr, nullptr, SW_SHOWNORMAL); // 홈페이지로 이동..
					PostQuitMessage(0);                                                             // 종료...
				}
				break;
				default:
					break;
			}
		}
		else if (pSender == m_pBtn_No || pSender == m_pBtn_Cancel)
		{
			if (m_pParentUI != nullptr)
				m_pParentUI->CallBackProc(m_iChildID, 2);

			SetVisible(false);
			SetText("");
			SetTitle("");

			switch (m_eBehavior)
			{
				case BEHAVIOR_PARTY_PERMIT:
					pProcMain->MsgSend_PartyOrForcePermit(false);
					break;

				case BEHAVIOR_PERSONAL_TRADE_PERMIT:
					pProcMain->m_pSubProcPerTrade->LeavePerTradeState(PER_TRADE_RESULT_MY_DISAGREE);
					break; // 내가 거절..

				case BEHAVIOR_PERSONAL_TRADE_FMT_WAIT:
					pProcMain->m_pSubProcPerTrade->LeavePerTradeState(PER_TRADE_RESULT_MY_CANCEL);
					break; // 내가 취소..

				case BEHAVIOR_CLAN_JOIN:
					pProcMain->MsgSend_KnightsJoinReq(false);
					break;

				default:
					break;
			}
		}

		SetVisible(false);
		SetText("");
		SetTitle("");
		SetVisibleEditControl(false);

		m_eBehavior = BEHAVIOR_NOTHING;
		pSender->SetState(UI_STATE_BUTTON_NORMAL);
	}

	return true;
}

void CUIMessageBox::SetBoxStyle(int iStyle)
{
	if (m_pBtn_OK)
		m_pBtn_OK->SetVisible(false);
	if (m_pBtn_Yes)
		m_pBtn_Yes->SetVisible(false);
	if (m_pBtn_No)
		m_pBtn_No->SetVisible(false);
	if (m_pBtn_Cancel)
		m_pBtn_Cancel->SetVisible(false);

	m_iStyle = iStyle;

	if (MB_YESNO == iStyle)
	{
		if (m_pBtn_Yes)
			m_pBtn_Yes->SetVisible(true);
		if (m_pBtn_No)
			m_pBtn_No->SetVisible(true);
	}
	else if (MB_CANCEL == iStyle)
	{
		if (m_pBtn_Cancel)
			m_pBtn_Cancel->SetVisible(true);
	}
	else if (MB_OK == iStyle)
	{
		if (m_pBtn_OK)
			m_pBtn_OK->SetVisible(true);
	}
	else
	{
		__ASSERT(0, "Not Supported MessageBox Type!!");
		if (m_pBtn_OK)
			m_pBtn_OK->SetVisible(true);
	}
}

void CUIMessageBox::SetVisibleEditControl(bool bVisible) // Edit Control Visible
{
	if (nullptr == m_pEdit_Common)
		return;

	m_pEdit_Common->SetString("");
	m_pEdit_Common->SetVisible(bVisible);
	if (bVisible)
		m_pEdit_Common->SetFocus();
	else
		m_pEdit_Common->KillFocus();
}

void CUIMessageBox::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);

	if (!bVisible)
	{
		if (CGameProcedure::s_pMsgBoxMgr)
			CGameProcedure::s_pMsgBoxMgr->ReFocusMsgBox();
	}
}

bool CUIMessageBox::OnKeyPress(int iKey)
{
	if (!IsVisible())
		return CN3UIBase::OnKeyPress(iKey);

	if (MB_YESNO == m_iStyle)
	{
		switch (iKey)
		{
			case DIK_ESCAPE:
				ReceiveMessage(m_pBtn_No, UIMSG_BUTTON_CLICK);
				return true;

			case DIK_RETURN:
				ReceiveMessage(m_pBtn_Yes, UIMSG_BUTTON_CLICK);
				return true;

			default:
				break;
		}
	}
	else if (MB_CANCEL == m_iStyle)
	{
		switch (iKey)
		{
			case DIK_ESCAPE:
			case DIK_RETURN:
				ReceiveMessage(m_pBtn_Cancel, UIMSG_BUTTON_CLICK);
				return true;

			default:
				break;
		}
	}
	else if (MB_OK == m_iStyle)
	{
		switch (iKey)
		{
			case DIK_ESCAPE:
			case DIK_RETURN:
				ReceiveMessage(m_pBtn_OK, UIMSG_BUTTON_CLICK);
				return true;

			default:
				break;
		}
	}

	return CN3UIBase::OnKeyPress(iKey);
}

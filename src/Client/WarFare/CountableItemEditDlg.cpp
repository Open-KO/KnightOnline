// CountableItemEditDlg.cpp: implementation of the CCountableItemEditDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "text_resources.h"
#include "CountableItemEditDlg.h"

#include "GameProcedure.h"
#include "GameProcMain.h"
#include "UIInventory.h"
#include "UITransactionDlg.h"

#include "SubProcPerTrade.h"
#include "UIPerTradeDlg.h"
#include "UIWareHouseDlg.h"
#include "UIManager.h"

#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIEdit.h>
#include <N3Base/N3UIString.h>

CCountableItemEditDlg::CCountableItemEditDlg()
{
	m_eCallerWnd         = UIWND_UNKNOWN;
	m_eCallerWndDistrict = UIWND_DISTRICT_UNKNOWN;
	m_pArea              = nullptr;
	m_pImageOfIcon       = nullptr;
	m_bLocked            = false;
	m_pEdit              = nullptr;
	m_bWareGold          = false;

	m_pBtnOk             = nullptr;
	m_pBtnCancel         = nullptr;
}

CCountableItemEditDlg::~CCountableItemEditDlg()
{
}

void CCountableItemEditDlg::Release()
{
	CN3UIBase::Release();
}

bool CCountableItemEditDlg::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (nullptr == pSender)
		return false;
	if (IsVisible() == false)
		return false;
	if (m_eCallerWnd == UIWND_UNKNOWN)
		return false;
	if (m_eCallerWndDistrict == UIWND_DISTRICT_UNKNOWN)
		return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtnOk)
		{
			switch (m_eCallerWnd)
			{
				case UIWND_TRANSACTION:
					switch (m_eCallerWndDistrict)
					{
						case UIWND_DISTRICT_TRADE_NPC:
						case UIWND_DISTRICT_TRADE_MY:
							CGameProcedure::s_pProcMain->m_pUITransactionDlg->ItemCountOK();
							break;

						default:
							break;
					}
					break;

				case UIWND_PER_TRADE:
					switch (m_eCallerWndDistrict)
					{
						case UIWND_DISTRICT_PER_TRADE_MY:
							CGameProcedure::s_pProcMain->m_pSubProcPerTrade->ItemCountOK();
							break;

						default:
							break;
					}
					break;

				case UIWND_WARE_HOUSE:
					switch (m_eCallerWndDistrict)
					{
						case UIWND_DISTRICT_TRADE_NPC:
							if (!m_bWareGold)
								CGameProcedure::s_pProcMain->m_pUIWareHouseDlg->ItemCountOK();
							else
								CGameProcedure::s_pProcMain->m_pUIWareHouseDlg->GoldCountFromWareOK();
							break;

						case UIWND_DISTRICT_TRADE_MY:
							if (!m_bWareGold)
								CGameProcedure::s_pProcMain->m_pUIWareHouseDlg->ItemCountOK();
							else
								CGameProcedure::s_pProcMain->m_pUIWareHouseDlg->GoldCountToWareOK();
							break;

						default:
							break;
					}
					break;

				default:
					break;
			}
		}

		if (pSender == m_pBtnCancel)
		{
			switch (m_eCallerWnd)
			{
				case UIWND_TRANSACTION:
					switch (m_eCallerWndDistrict)
					{
						case UIWND_DISTRICT_TRADE_NPC:
						case UIWND_DISTRICT_TRADE_MY:
							CGameProcedure::s_pProcMain->m_pUITransactionDlg->ItemCountCancel();
							break;

						default:
							break;
					}
					break;

				case UIWND_PER_TRADE:
					switch (m_eCallerWndDistrict)
					{
						case UIWND_DISTRICT_PER_TRADE_MY:
							CGameProcedure::s_pProcMain->m_pSubProcPerTrade->ItemCountCancel();
							break;

						default:
							break;
					}
					break;

				case UIWND_WARE_HOUSE:
					switch (m_eCallerWndDistrict)
					{
						case UIWND_DISTRICT_TRADE_NPC:
							if (!m_bWareGold)
								CGameProcedure::s_pProcMain->m_pUIWareHouseDlg->ItemCountCancel();
							else
								CGameProcedure::s_pProcMain->m_pUIWareHouseDlg->GoldCountFromWareCancel();
							break;

						case UIWND_DISTRICT_TRADE_MY:
							if (!m_bWareGold)
								CGameProcedure::s_pProcMain->m_pUIWareHouseDlg->ItemCountCancel();
							else
								CGameProcedure::s_pProcMain->m_pUIWareHouseDlg->GoldCountToWareCancel();
							break;

						default:
							break;
					}
					break;

				default:
					break;
			}
		}
	}

	return true;
}

void CCountableItemEditDlg::Open(e_UIWND eUW, e_UIWND_DISTRICT eUD, bool bCountGold, bool bWareGold)
{
	std::string szMsg;
	if (bCountGold)
		szMsg = fmt::format_text_resource(IDS_EDIT_BOX_GOLD);
	else
		szMsg = fmt::format_text_resource(IDS_EDIT_BOX_COUNT);

	CN3UIString* pString = nullptr;
	N3_VERIFY_UI_COMPONENT(pString, GetChildByID<CN3UIString>("String_PersonTradeEdit_Msg"));
	if (pString != nullptr)
		pString->SetString(szMsg);

	RECT rc {}, rcThis {};
	int iCX = 0, iCY = 0;

	m_bLocked = true;
	SetQuantity(-1);

	SetVisible(true);

	CN3UIEdit* pEdit = nullptr;
	N3_VERIFY_UI_COMPONENT(pEdit, GetChildByID<CN3UIEdit>("edit_trade"));
	if (pEdit != nullptr)
		pEdit->SetFocus();

	m_eCallerWnd         = eUW;
	m_eCallerWndDistrict = eUD;
	m_bWareGold          = bWareGold;

	switch (eUW)
	{
		case UIWND_TRANSACTION:
			rc = CGameProcedure::s_pProcMain->m_pUITransactionDlg->GetRegion();
			break;

		case UIWND_PER_TRADE:
			rc = CGameProcedure::s_pProcMain->m_pSubProcPerTrade->m_pUIPerTradeDlg->GetRegion();
			break;

		case UIWND_WARE_HOUSE:
			rc = CGameProcedure::s_pProcMain->m_pUIWareHouseDlg->GetRegion();
			break;

		default:
			return;
	}

	if ((eUW == UIWND_TRANSACTION) || (eUW == UIWND_PER_TRADE) || (eUW == UIWND_WARE_HOUSE))
	{
		iCX    = (rc.right + rc.left) / 2;
		iCY    = (rc.bottom + rc.top) / 2;
		rcThis = GetRegion();
		SetPos(iCX - (rcThis.right - rcThis.left) / 2, iCY - (rcThis.bottom - rcThis.top) / 2);
	}
}

void CCountableItemEditDlg::Close()
{
	m_bLocked = false;
	//	SetVisible(false);	//이거 쓰지 말고 SetVisibleWithNoSound 함수 써주세요...
	SetVisibleWithNoSound(false);

	CN3UIEdit* pEdit = GetFocusedEdit();
	if (pEdit)
		pEdit->KillFocus();
}

int CCountableItemEditDlg::GetQuantity() // "edit_trade" Edit Control 에서 정수값을 얻오온다..
{
	CN3UIEdit* pEdit = nullptr;
	N3_VERIFY_UI_COMPONENT(pEdit, GetChildByID<CN3UIEdit>("edit_trade"));

	return atoi(pEdit->GetString().c_str());
}

void CCountableItemEditDlg::SetQuantity(int iQuantity) // "edit_trade" Edit Control 에서 정수값을 문자열로 세팅한다..
{
	CN3UIEdit* pEdit = nullptr;
	N3_VERIFY_UI_COMPONENT(pEdit, GetChildByID<CN3UIEdit>("edit_trade"));

	std::string buff;
	if (iQuantity != -1)
		buff = std::to_string(iQuantity);

	pEdit->SetString(buff);
}

//this_ui_add_start
void CCountableItemEditDlg::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);
	if (bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
		CGameProcedure::s_pUIMgr->ReFocusUI(); //this_ui
}

void CCountableItemEditDlg::SetVisibleWithNoSound(bool bVisible, bool bWork, bool bReFocus)
{
	if (bWork)
	{ //여기서는 ReceiveMessage에서 유아이가 보이는지를 체크를 해서 일단 앞쪽에 둔다.
		ReceiveMessage(m_pBtnCancel, UIMSG_BUTTON_CLICK);
	}

	CN3UIBase::SetVisibleWithNoSound(bVisible, bWork, bReFocus);
}

bool CCountableItemEditDlg::Load(File& file)
{
	if (false == CN3UIBase::Load(file))
		return false;

	N3_VERIFY_UI_COMPONENT(m_pBtnOk, GetChildByID<CN3UIButton>("btn_ok"));
	N3_VERIFY_UI_COMPONENT(m_pBtnCancel, GetChildByID<CN3UIButton>("btn_cancel"));

	return true;
}

bool CCountableItemEditDlg::OnKeyPress(int iKey)
{
	switch (iKey)
	{
		case DIK_RETURN:
			ReceiveMessage(m_pBtnOk, UIMSG_BUTTON_CLICK);
			return true;

		case DIK_ESCAPE:
			ReceiveMessage(m_pBtnCancel, UIMSG_BUTTON_CLICK);
			return true;

		default:
			break;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

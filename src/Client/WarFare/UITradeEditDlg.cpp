// UITradeEditDlg.cpp: implementation of the CUITradeEditDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UITradeEditDlg.h"
#include "PacketDef.h"
#include "LocalInput.h"
#include "APISocket.h"
#include "GameProcMain.h"
#include "UIImageTooltipDlg.h"
#include "UIInventory.h"
#include "SubProcPerTrade.h"
#include "UIPerTradeDlg.h"
#include "text_resources.h"

#include <N3Base/N3UIEdit.h>
#include <N3Base/N3UIString.h>

CUITradeEditDlg::CUITradeEditDlg()
{
	m_pSubProcPerTrade = nullptr;
	m_pArea            = nullptr;
	m_pImageOfIcon     = nullptr;
}

CUITradeEditDlg::~CUITradeEditDlg()
{
}

///////////////////////////////////////////////////////////////////////

void CUITradeEditDlg::Release()
{
	CN3UIBase::Release();
}

int CUITradeEditDlg::GetQuantity() // "edit_trade" Edit Control 에서 정수값을 얻오온다..
{
	CN3UIEdit* pEdit = nullptr;
	N3_VERIFY_UI_COMPONENT(pEdit, GetChildByID<CN3UIEdit>("edit_trade"));

	return atoi(pEdit->GetString().c_str());
}

void CUITradeEditDlg::SetQuantity(int iQuantity) // "edit_trade" Edit Control 에서 정수값을 문자열로 세팅한다..
{
	CN3UIEdit* pEdit = nullptr;
	N3_VERIFY_UI_COMPONENT(pEdit, GetChildByID<CN3UIEdit>("edit_trade"));

	std::string buff = std::to_string(iQuantity);
	pEdit->SetString(buff);
}

bool CUITradeEditDlg::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (nullptr == pSender)
		return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender->m_szID == "btn_ok")
			m_pSubProcPerTrade->ItemCountEditOK();

		if (pSender->m_szID == "btn_cancel")
			m_pSubProcPerTrade->ItemCountEditCancel();
	}

	return true;
}

void CUITradeEditDlg::Open(bool bCountGold)
{
	std::string szMsg;
	if (bCountGold)
		szMsg = fmt::format_text_resource(IDS_EDIT_BOX_GOLD);
	else
		szMsg = fmt::format_text_resource(IDS_EDIT_BOX_COUNT);

	CN3UIString* pString = nullptr;
	N3_VERIFY_UI_COMPONENT(pString, GetChildByID<CN3UIString>("String_PersonTradeEdit_Msg"));
	__ASSERT(pString, "NULL UI Component!!");
	if (pString)
		pString->SetString(szMsg);

	SetVisible(true);

	CN3UIEdit* pEdit = nullptr;
	N3_VERIFY_UI_COMPONENT(pEdit, GetChildByID<CN3UIEdit>("edit_trade"));
	if (pEdit)
		pEdit->SetFocus();

	RECT rc, rcThis;
	int iCX = 0, iCY = 0;

	SetQuantity(0);

	rc     = CGameProcedure::s_pProcMain->m_pSubProcPerTrade->m_pUIPerTradeDlg->GetRegion();
	iCX    = (rc.right + rc.left) / 2;
	iCY    = (rc.bottom + rc.top) / 2;
	rcThis = GetRegion();
	SetPos(iCX - (rcThis.right - rcThis.left) / 2, iCY - (rcThis.bottom - rcThis.top) / 2);
}

void CUITradeEditDlg::Close()
{
	SetVisible(false);

	CN3UIEdit* pEdit = GetFocusedEdit();
	if (pEdit)
		pEdit->KillFocus();
}


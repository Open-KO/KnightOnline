// UITradeBBSSelector.cpp: implementation of the CUITradeBBSSelector class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UITradeBBSSelector.h"
#include "PacketDef.h"
#include "APISocket.h"
#include "GameProcMain.h"
#include "UITradeSellBBS.h"
#include "LocalInput.h"
#include "UIManager.h"

#include <N3Base/N3UIButton.h>

CUITradeBBSSelector::CUITradeBBSSelector()
{
	m_pBtn_BBSSell   = nullptr;
	m_pBtn_BBSBuy    = nullptr;
	m_pBtn_BBSCancel = nullptr;
}

CUITradeBBSSelector::~CUITradeBBSSelector()
{
}

bool CUITradeBBSSelector::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtn_BBSSell)
		{
			MsgSend_OpenTradeSellBBS();
			SetVisible(false);
			return true;
		}
		else if (pSender == m_pBtn_BBSBuy)
		{
			MsgSend_OpenTradeBuyBBS();
			SetVisible(false);
			return true;
		}
		else if (pSender == m_pBtn_BBSCancel)
		{
			SetVisible(false);
			return true;
		}
	}
	return true;
}

bool CUITradeBBSSelector::Load(File& file)
{
	if (CN3UIBase::Load(file) == false)
		return false;

	N3_VERIFY_UI_COMPONENT(m_pBtn_BBSSell, GetChildByID<CN3UIButton>("btn_sell"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_BBSBuy, GetChildByID<CN3UIButton>("btn_buy"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_BBSCancel, GetChildByID<CN3UIButton>("btn_cancel"));

	return true;
}

void CUITradeBBSSelector::MsgSend_OpenTradeSellBBS()
{
	uint8_t byBuff[5]; // 패킷 버퍼..
	int iOffset = 0;   // 패킷 오프셋..

	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_MARKET_BBS);
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_TYPE_BBS_OPEN);
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_TRADE_BBS_SELL);
	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
}

void CUITradeBBSSelector::MsgSend_OpenTradeBuyBBS()
{
	uint8_t byBuff[5]; // 패킷 버퍼..
	int iOffset = 0;   // 패킷 오프셋..

	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_MARKET_BBS);
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_TYPE_BBS_OPEN);
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_TRADE_BBS_BUY);
	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
}

bool CUITradeBBSSelector::OnKeyPress(int iKey)
{
	if (iKey == DIK_ESCAPE)
	{
		SetVisible(false);
		return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

void CUITradeBBSSelector::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);
	if (bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
		CGameProcedure::s_pUIMgr->ReFocusUI(); //this_ui
}

// UINPCEvent.cpp: implementation of the UINPCEvent class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "text_resources.h"
#include "UIInn.h"
#include "UICreateClanName.h"

#include "GameProcMain.h"
#include "GameProcedure.h"
#include "PacketDef.h"
#include "APISocket.h"
#include "PlayerMySelf.h"
#include "UITradeBBSSelector.h"
#include "UIManager.h"

//#include "StdAfxBase.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUIInn::CUIInn()
{
}

CUIInn::~CUIInn()
{
}

bool CUIInn::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender->m_szID == "btn_warehouse")	//창고이용..
		{
			MsgSend_OpenWareHouse();
			SetVisible(false);
			return true;
		}

		if (pSender->m_szID == "btn_makeclan")	//clan 생성..
		{
			__InfoPlayerBase* pInfo = &CGameBase::s_pPlayer->m_InfoBase;
			__InfoPlayerMySelf* pInfoExt = &CGameBase::s_pPlayer->m_InfoExt;

			if (pInfo->iLevel < CLAN_LEVEL_LIMIT)
			{
				Message(IDS_CLAN_DENY_LOWLEVEL);
				SetVisible(false);
				return true;
			}

			if (pInfoExt->iGold < CLAN_COST)
			{
				Message(IDS_CLAN_DENY_LOWGOLD);
				SetVisible(false);
				return true;
			}

			if (pInfo->iKnightsID != 0)
			{
				Message(IDS_CLAN_DENY_ALREADYJOINED);
				SetVisible(false);
				return true;
			}

			CGameProcedure::s_pProcMain->m_pUICreateClanName->Open(IDS_CLAN_INPUT_NAME);
			SetVisible(false);
			return true;
		}
		else if (pSender->m_szID == "btn_sale")
		{
			CGameProcedure::s_pProcMain->m_pUITradeBBSSelector->MsgSend_OpenTradeSellBBS();
			SetVisible(false);
			return true;
		}

	}
	return true;
}

void CUIInn::Message(int iMessageID)
{
	std::string szMsg;

	if (iMessageID == IDS_CLAN_DENY_LOWGOLD)
		szMsg = fmt::format_text_resource(iMessageID, CLAN_COST);
	else
		szMsg = fmt::format_text_resource(iMessageID);

	CGameProcedure::MessageBoxPost(szMsg, "", MB_OK, BEHAVIOR_NOTHING);	
}

void CUIInn::MsgSend_OpenWareHouse()
{
	uint8_t byBuff[2];		// 패킷 버퍼..
	int iOffset=0;		// 패킷 오프셋..

	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_WAREHOUSE);		
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_WARE_OPEN);
	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
}

void CUIInn::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);
	if(bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
		CGameProcedure::s_pUIMgr->ReFocusUI();//this_ui
}

bool CUIInn::OnKeyPress(int iChar)
{
	switch(iChar)
	{
	case DIK_ESCAPE:
		SetVisible(false);
		return true;
	}

	return CN3UIBase::OnKeyPress(iChar);
}

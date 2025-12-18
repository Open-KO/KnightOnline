// UINPCEvent.cpp: implementation of the UINPCEvent class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UINPCEvent.h"
#include "GameProcMain.h"
#include "UIInventory.h"
#include "UITransactionDlg.h"
#include "SubProcPerTrade.h"
#include "UISkillTreeDlg.h"
#include "UIManager.h"
#include "text_resources.h"

#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIString.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUINPCEvent::CUINPCEvent()
{

}

CUINPCEvent::~CUINPCEvent()
{

}

void CUINPCEvent::Release()
{
	CN3UIBase::Release();
}

bool CUINPCEvent::Load(File& file)
{
	if(CN3UIBase::Load(file)==false) return false;

	N3_VERIFY_UI_COMPONENT(m_pBtn_Repair, GetChildByID<CN3UIButton>("Btn_Repair"));
	N3_VERIFY_UI_COMPONENT(m_pText_Repair, GetChildByID<CN3UIString>("Text_Repair"));
	N3_VERIFY_UI_COMPONENT(m_pText_Title, GetChildByID<CN3UIString>("Text_Title"));

	return true;
}

bool CUINPCEvent::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)					
	{
		if(pSender->m_szID == "Btn_Sale")
		{
			if (CGameProcedure::s_pProcMain->m_pSubProcPerTrade->m_ePerTradeState != PER_TRADE_STATE_NONE) 	/* 개인간 상거래 중이면.. */
				return true;

			Close();
			if (CGameProcedure::s_pProcMain->m_pUIInventory->IsVisible())
				return true;
			if (CGameProcedure::s_pProcMain->m_pUITransactionDlg->IsVisible())
				return true;
			if (CGameProcedure::s_pProcMain->m_pUISkillTreeDlg->IsVisible())
				CGameProcedure::s_pProcMain->m_pUISkillTreeDlg->Close();
			CGameProcedure::s_pProcMain->DoCommercialTransaction(m_iTradeID);
		}

		if(pSender->m_szID == "Btn_Repair")
		{
			if (CGameProcedure::s_pProcMain->m_pSubProcPerTrade->m_ePerTradeState != PER_TRADE_STATE_NONE) 	/* 개인간 상거래 중이면.. */
				return true;

			Close();
			if (CGameProcedure::s_pProcMain->m_pUITransactionDlg->IsVisible())
				return true;
			if (CGameProcedure::s_pProcMain->m_pUIInventory->IsVisible())
				return true;
			if (CGameProcedure::s_pProcMain->m_pUISkillTreeDlg->IsVisible())
				CGameProcedure::s_pProcMain->m_pUISkillTreeDlg->Close();
			CGameProcedure::s_pProcMain->m_pUIInventory->Open(INV_STATE_REPAIR);

		}


		if(pSender->m_szID == "btn_close")
			Close();
	}

	return true;
}

void CUINPCEvent::Open(e_NpcEvent eNpcEvent, int iTradeId, int iIDTarget)
{
	SetVisible(true);
	m_iIDTarget = iIDTarget;
	m_iTradeID  = iTradeId;
	std::string szStr;

	switch(eNpcEvent)
	{
		case NPC_EVENT_ITEM_TRADE:
			szStr = fmt::format_text_resource(IDS_NPC_EVENT_TITLE_TRADE);
			m_pText_Title->SetString(szStr);
			if (m_pBtn_Repair)
			{
				if (m_pBtn_Repair->IsVisible())
				{
					m_pBtn_Repair->SetVisible(false);
					m_pText_Repair->SetVisible(false);
				}
			}
			break;

		case NPC_EVENT_TRADE_REPAIR:
			szStr = fmt::format_text_resource(IDS_NPCEVENT_TITLE_REPAIR);
			m_pText_Title->SetString(szStr);
			if (m_pBtn_Repair)
			{
				if (!m_pBtn_Repair->IsVisible())
				{
					m_pBtn_Repair->SetVisible(true);
					m_pText_Repair->SetVisible(true);
				}
			}
			break;
	}
}

void CUINPCEvent::Close()
{
	SetVisible(false);
}

bool CUINPCEvent::OnKeyPress(int iKey)
{
	if(DIK_ESCAPE == iKey)
	{
		this->Close();
		return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

void CUINPCEvent::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);
	if(bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
		CGameProcedure::s_pUIMgr->ReFocusUI();//this_ui
}

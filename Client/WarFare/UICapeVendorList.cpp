// UICapeVendorList.cpp: implementation of the CUICapeVendorList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GameDef.h"
#include "GameBase.h"
#include "GameProcMain.h"
#include "GameProcedure.h"
#include "UICapeVendorList.h"
#include "UICapeVendorShop.h"
#include "UICapeVendorSymbol.h"
#include "UIManager.h"
#include "APISocket.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUICapeVendorList::CUICapeVendorList()
{
}

CUICapeVendorList::~CUICapeVendorList()
{
}

bool CUICapeVendorList::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (pSender == nullptr) return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender->m_szID == "btn_close")
			SetVisible(false);

		if (pSender->m_szID == "btn_mantle")
		{
			SetVisible(false);
			CGameProcMain::s_pProcMain->m_pUICapeVendorShop->SetVisible(true);
		}

		if (pSender->m_szID == "btn_application")
		{
			SetVisible(false);
			CGameProcMain::s_pProcMain->m_pUICapeVendorSymbol->SetVisible(true);
		}
	}
	return CN3UIBase::ReceiveMessage(pSender, dwMsg);
}

bool CUICapeVendorList::OnKeyPress(int iKey)
{
	switch (iKey)
	{
		case DIK_ESCAPE:
			SetVisible(false);
			return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

void CUICapeVendorList::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);

	if (bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
		CGameProcedure::s_pUIMgr->ReFocusUI();
}

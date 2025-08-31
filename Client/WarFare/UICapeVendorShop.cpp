// UICapeVendorShop.cpp: implementation of the CUICapeVendorShop class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GameDef.h"
#include "GameBase.h"
#include "GameProcedure.h"
#include "UICapeVendorShop.h"
#include "UIManager.h"
#include "PlayerOtherMgr.h"
#include "APISocket.h"

#include <N3Base/N3UIString.h>
#include <N3Base/N3UIImage.h>
#include <N3Base/N3UIButton.h>
#include <N3Base/N3UITooltip.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUICapeVendorShop::CUICapeVendorShop()
{}

CUICapeVendorShop::~CUICapeVendorShop()
{}

bool CUICapeVendorShop::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (pSender == nullptr) return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender->m_szID == "btn_cancel")
			SetVisible(false);
	}
	return CN3UIBase::ReceiveMessage(pSender, dwMsg);
}

bool CUICapeVendorShop::OnKeyPress(int iKey)
{
	switch (iKey)
	{
		case DIK_ESCAPE:
			SetVisible(false);
			return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

void CUICapeVendorShop::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);

	if (bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
		CGameProcedure::s_pUIMgr->ReFocusUI();
}

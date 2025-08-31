// UICapeVendorSymbol.cpp: implementation of the UICapeVendorSymbol and CUICapeVendorPreview class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GameDef.h"
#include "GameBase.h"
#include "GameProcMain.h"
#include "GameProcedure.h"
#include "UICapeVendorSymbol.h"
#include "UIManager.h"
#include "APISocket.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction - Symbol
//////////////////////////////////////////////////////////////////////


CUICapeVendorSymbol::CUICapeVendorSymbol()
{}

CUICapeVendorSymbol::~CUICapeVendorSymbol()
{}

bool CUICapeVendorSymbol::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (pSender == nullptr) return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender->m_szID == "btn_cancel")
			SetVisible(false);

		if (pSender->m_szID == "btn_ok")
		{
			SetVisible(false);
			CGameProcMain::s_pProcMain->m_pUICapeVendorPreview->SetVisible(true);

		}
	}
	return CN3UIBase::ReceiveMessage(pSender, dwMsg);
}

bool CUICapeVendorSymbol::OnKeyPress(int iKey)
{
	switch (iKey)
	{
		case DIK_ESCAPE:
			SetVisible(false);
			return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

void CUICapeVendorSymbol::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);

	if (bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
		CGameProcedure::s_pUIMgr->ReFocusUI();
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction - Symbol Preview
//////////////////////////////////////////////////////////////////////


CUICapeVendorPreview::CUICapeVendorPreview()
{}

CUICapeVendorPreview::~CUICapeVendorPreview()
{}

bool CUICapeVendorPreview::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (pSender == nullptr) return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender->m_szID == "btn_cancel")
			SetVisible(false);

		if (pSender->m_szID == "btn_ok")
		{
			// TO DO: Build out
		}
	}
	return CN3UIBase::ReceiveMessage(pSender, dwMsg);
}

bool CUICapeVendorPreview::OnKeyPress(int iKey)
{
	switch (iKey)
	{
		case DIK_ESCAPE:
			SetVisible(false);
			return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

void CUICapeVendorPreview::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);

	if (bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
		CGameProcedure::s_pUIMgr->ReFocusUI();
}

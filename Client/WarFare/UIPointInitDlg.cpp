// UIPointInitDlg.cpp: implementation of the UIPointInitDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UIPointInitDlg.h"
#include "PacketDef.h"
#include "GameProcMain.h"
#include "PlayerMySelf.h"
#include "APISocket.h"
#include "N3UIWndBase.h"
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

CUIPointInitDlg::CUIPointInitDlg()
{
	m_pBtn_Ok			= nullptr;
	m_pBtn_Cancel		= nullptr;

	m_pText_NeedGold	= nullptr;
}

CUIPointInitDlg::~CUIPointInitDlg()
{

}

void CUIPointInitDlg::Release()
{
	CN3UIBase::Release();
}

bool CUIPointInitDlg::Load(File& file)
{
	if(CN3UIBase::Load(file)==false) return false;

	N3_VERIFY_UI_COMPONENT(m_pBtn_Ok, GetChildByID<CN3UIButton>("btn_ok"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Cancel, GetChildByID<CN3UIButton>("btn_cancel"));

	N3_VERIFY_UI_COMPONENT(m_pText_NeedGold, GetChildByID<CN3UIString>("string_gold"));

	return true;
}

bool CUIPointInitDlg::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)					
	{
		if(pSender == m_pBtn_Ok)
		{
			Close();
			PushOkButton();
		}
		else if(pSender == m_pBtn_Cancel)
			Close();
	}

	return true;
}

void CUIPointInitDlg::Close()
{
	SetVisible(false);
}

void CUIPointInitDlg::PushOkButton()
{
	uint8_t byBuff[32];
	int iOffset = 0;

	switch (m_bAllpoint)
	{
		case true:
			CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_CLASS_CHANGE);
			CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_CLASS_ALL_POINT);
			break;

		case false:
			CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_CLASS_CHANGE);
			CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_CLASS_SKILL_POINT);
			break;
	}

	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
}

//this_ui_add_start
bool CUIPointInitDlg::OnKeyPress(int iKey)
{
	switch(iKey)
	{
	case DIK_RETURN:
		ReceiveMessage(m_pBtn_Ok, UIMSG_BUTTON_CLICK);
		return true;
	case DIK_ESCAPE:
		ReceiveMessage(m_pBtn_Cancel, UIMSG_BUTTON_CLICK);
		return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

void CUIPointInitDlg::InitDlg(bool bAllpoint, int iGold)
{
	m_bAllpoint = bAllpoint;
	if (m_pText_NeedGold) 
	{
		switch (bAllpoint)
		{
			case true: 
				m_pText_NeedGold->SetStringAsInt(iGold);
				break;
			case false: 
				m_pText_NeedGold->SetStringAsInt(iGold);
				break;
		}
	}
}

//this_ui_add_end

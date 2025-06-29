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
#include "resource.h"

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
	m_pBtn_Ok			= NULL;
	m_pBtn_Cancel		= NULL;

	m_pText_NeedGold	= NULL;
}

CUIPointInitDlg::~CUIPointInitDlg()
{

}

void CUIPointInitDlg::Release()
{
	CN3UIBase::Release();
}

bool CUIPointInitDlg::Load(HANDLE hFile)
{
	if(CN3UIBase::Load(hFile)==false) return false;

	m_pBtn_Ok			= (CN3UIButton*)GetChildByID("btn_ok");			__ASSERT(m_pBtn_Ok,	"NULL UI Component!!");
	m_pBtn_Cancel		= (CN3UIButton*)GetChildByID("btn_cancel");		__ASSERT(m_pBtn_Cancel, "NULL UI Component!!");

	m_pText_NeedGold	= (CN3UIString*)GetChildByID("string_gold");	__ASSERT(m_pText_NeedGold,	"NULL UI Component!!");

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

// UICmdEdit.cpp: implementation of the UINPCEvent class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UICmdEdit.h"
#include "APISocket.h"
#include "GameProcMain.h"
#include "PacketDef.h"
#include "text_resources.h"

#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIEdit.h>
#include <N3Base/N3UIString.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUICmdEdit::CUICmdEdit()
{
}

CUICmdEdit::~CUICmdEdit()
{
}

bool CUICmdEdit::Load(HANDLE hFile)
{
	if (!CN3UIBase::Load(hFile)) 
		return false;
	
	N3_VERIFY_UI_COMPONENT(m_pText_Title,	GetChildByID<CN3UIString>("Text_cmd"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Ok,		GetChildByID<CN3UIButton>("btn_ok"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Cancel,	GetChildByID<CN3UIButton>("btn_cancel"));
	N3_VERIFY_UI_COMPONENT(m_pEdit_Box,		GetChildByID<CN3UIEdit>("edit_cmd"));

	return true;
}

bool CUICmdEdit::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtn_Ok)
		{
			m_szArg1 = m_pEdit_Box->GetString();
			std::string strTempCmd = "/" + m_pText_Title->GetString() + " " + m_szArg1;
			CGameProcedure::s_pProcMain->ParseChattingCommand(strTempCmd);

			SetVisible(false);
			return true;
		}

		if (pSender == m_pBtn_Cancel)
		{
			SetVisible(false);
			return true;
		}
	}
	return true;
}

void CUICmdEdit::Open(const std::string& msg)
{
	m_pText_Title->SetString(msg);
	m_pEdit_Box->SetFocus();
	SetVisible(true);
}

void CUICmdEdit::SetVisible(bool bVisible)
{
	if (bVisible == IsVisible()) 
		return;

	if (!bVisible)
		m_pEdit_Box->KillFocus();

	CN3UIBase::SetVisible(bVisible);
}

// UICmdEdit.cpp: implementation of the UINPCEvent class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UICmdEdit.h"
#include "APISocket.h"
#include "GameProcMain.h"
#include "PacketDef.h"
#include "text_resources.h"

#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIEdit.h>
#include <N3Base/N3UIString.h>

CUICmdEdit::CUICmdEdit()
{
	m_pText_Title = nullptr;
	m_pEdit_Box   = nullptr;
	m_pBtn_Cancel = nullptr;
	m_pBtn_Ok     = nullptr;
}

CUICmdEdit::~CUICmdEdit()
{
}

void CUICmdEdit::Release()
{
	CN3UIBase::Release();

	m_pText_Title = nullptr;
	m_pEdit_Box   = nullptr;
	m_pBtn_Cancel = nullptr;
	m_pBtn_Ok     = nullptr;

	m_szArg1.clear();
}

bool CUICmdEdit::Load(File& file)
{
	if (!CN3UIBase::Load(file))
		return false;

	N3_VERIFY_UI_COMPONENT(m_pText_Title, GetChildByID<CN3UIString>("Text_cmd"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Ok, GetChildByID<CN3UIButton>("btn_ok"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Cancel, GetChildByID<CN3UIButton>("btn_cancel"));
	N3_VERIFY_UI_COMPONENT(m_pEdit_Box, GetChildByID<CN3UIEdit>("edit_cmd"));

	return true;
}

bool CUICmdEdit::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtn_Ok)
		{
			m_szArg1               = m_pEdit_Box->GetString();
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

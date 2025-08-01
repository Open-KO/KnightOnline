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
	if (CN3UIBase::Load(hFile) == false) return false;

	m_pText_Title = (CN3UIString*)GetChildByID("Text_cmd");			__ASSERT(m_pText_Title, "NULL UI Component!!");
	m_pBtn_Ok = (CN3UIButton*)GetChildByID("btn_ok");				__ASSERT(m_pBtn_Ok, "NULL UI Component!!");
	m_pBtn_Cancel = (CN3UIButton*)GetChildByID("btn_cancel");		__ASSERT(m_pBtn_Cancel, "NULL UI Component!!");
	m_pEdit_Box = (CN3UIEdit*)GetChildByID("edit_cmd");				__ASSERT(m_pEdit_Box, "NULL UI Component!!");
	return true;
}

bool CUICmdEdit::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtn_Ok)
		{
			m_szArg1 = m_pEdit_Box->GetString();
			std::string tempCmdStr = "/" + m_pText_Title->GetString() + " " + m_szArg1;
			CGameProcedure::s_pProcMain->ParseChattingCommand(tempCmdStr);
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

void CUICmdEdit::Open(std::string msg)
{
	if (!msg.empty())
	{
		m_pText_Title->SetString(msg);
	}
	//m_pEdit_Box->SetString("");
	m_pEdit_Box->SetFocus();
	SetVisible(true);
}

void CUICmdEdit::SetVisible(bool bVisible)
{
	if (bVisible == this->IsVisible()) return;

	if (!bVisible)
	{
		m_pEdit_Box->KillFocus();
	}

	CN3UIBase::SetVisible(bVisible);
}

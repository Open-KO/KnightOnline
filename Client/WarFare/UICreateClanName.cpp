// UICreateClanName.cpp: implementation of the UINPCEvent class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UICreateClanName.h"
#include "APISocket.h"
#include "GameProcMain.h"
#include "PacketDef.h"
#include "text_resources.h"

#include <N3Base/N3UIEdit.h>
#include <N3Base/N3UIString.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUICreateClanName::CUICreateClanName()
{
}

CUICreateClanName::~CUICreateClanName()
{
}

bool CUICreateClanName::Load(HANDLE hFile)
{
	if(CN3UIBase::Load(hFile)==false) return false;

	m_pText_Title		= (CN3UIString*)GetChildByID("Text_Message");	__ASSERT(m_pText_Title,	"NULL UI Component!!");
	m_pEdit_ClanName	= (CN3UIEdit*)GetChildByID("Edit_Clan");		__ASSERT(m_pEdit_ClanName,	"NULL UI Component!!");
	return true;
}

bool CUICreateClanName::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)					
	{
		if(pSender->m_szID == "btn_yes")	
		{
			m_szClanName = m_pEdit_ClanName->GetString();
			if(!MakeClan()) return true;
			SetVisible(false);
			return true;
		}

		if(pSender->m_szID == "btn_no")	
		{
			SetVisible(false);
			return true;
		}
	}
	return true;
}

bool CUICreateClanName::MakeClan()
{
	if (m_szClanName.empty()) return false;
	if (m_szClanName.size() > 20)
		m_szClanName.resize(20);

	std::string szMsg = fmt::format_text_resource(IDS_CLAN_WARNING_COST, CLAN_COST);
	CGameProcedure::s_pProcMain->MessageBoxPost(szMsg, "", MB_YESNO, BEHAVIOR_KNIGHTS_CREATE);
	return true;
}

void CUICreateClanName::MsgSend_MakeClan()
{
	int iLn = (int) m_szClanName.size();
	uint8_t byBuff[40];									// 패킷 버퍼..
	int iOffset = 0;									// 패킷 오프셋..
	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_KNIGHTS_PROCESS);
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_KNIGHTS_CREATE);
	CAPISocket::MP_AddShort(byBuff, iOffset, (uint8_t) iLn);
	CAPISocket::MP_AddString(byBuff, iOffset, m_szClanName);

	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
}

void CUICreateClanName::Open(int msg)
{
	if (msg != 0)
	{
		std::string szMsg = fmt::format_text_resource(msg);
		m_pText_Title->SetString(szMsg);
	}
	m_pEdit_ClanName->SetString("");
	m_pEdit_ClanName->SetFocus();
	SetVisible(true);
}

void CUICreateClanName::SetVisible(bool bVisible)
{
	if(bVisible==this->IsVisible()) return;

	if(!bVisible)
	{
		m_pEdit_ClanName->KillFocus();
	}

	CN3UIBase::SetVisible(bVisible);
}

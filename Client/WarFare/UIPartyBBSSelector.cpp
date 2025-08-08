// UIPartyBBSSelector.cpp: implementation of the CUIPartyBBSSelector class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UIPartyBBSSelector.h"
#include "GameProcedure.h"
#include "PacketDef.h"
#include "APISocket.h"

#include <N3Base/N3UIButton.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUIPartyBBSSelector::CUIPartyBBSSelector()
{
	m_pBtn_WantPartyMember	= NULL;
	m_pBtn_WantParty		= NULL;
}

CUIPartyBBSSelector::~CUIPartyBBSSelector()
{

}

bool CUIPartyBBSSelector::ReceiveMessage(CN3UIBase *pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)					
	{
		if(pSender->m_szID == "Btn_WantParty")	
		{
			if(m_pBtn_WantParty && m_pBtn_WantParty->IsVisible())
			{
				MsgSend_PartyBBSKind(WANT_PARTY);
			}
			SetVisible(false);
			return true;
		}

		if(pSender->m_szID == "Btn_WantPartyMember")
		{
			if(m_pBtn_WantPartyMember && m_pBtn_WantPartyMember->IsVisible())
			{
				MsgSend_PartyBBSKind(WANT_PARTY_MEMBER);
			}
			SetVisible(false);
			return true;
		}

		if(pSender->m_szID == "Btn_Cancel")	
		{
			SetVisible(false);
			return true;
		}
	}
	return true;
}

bool CUIPartyBBSSelector::Load(HANDLE hFile)
{
	if(CN3UIBase::Load(hFile)==false) return false;

	m_pBtn_WantPartyMember	= (CN3UIButton*)GetChildByID("Btn_WantParty");			__ASSERT(m_pBtn_WantPartyMember,	"NULL UI Component!!");
	m_pBtn_WantParty		= (CN3UIButton*)GetChildByID("Btn_WantPartyMember");	__ASSERT(m_pBtn_WantParty,	"NULL UI Component!!");

	return true;
}

void CUIPartyBBSSelector::MsgSend_PartyBBSKind(uint8_t byKind)
{
	uint8_t byBuff[4];											
	int iOffset=0;											

	CAPISocket::MP_AddByte(byBuff, iOffset, 1);		// 패킷도 정해야 할듯
	CAPISocket::MP_AddByte(byBuff, iOffset, byKind);		// 파티구함인지 파티원 구함인지...
	CGameProcedure::s_pSocket->Send(byBuff, iOffset);		// 패킷을 보냄..
}

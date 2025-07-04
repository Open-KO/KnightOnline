﻿// UIPartyBBSSelector.h: interface for the CUIPartyBBSSelector class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIPARTYBBSSELECTOR_H__CF79ECBE_7FBE_4251_A740_E6AD08881785__INCLUDED_)
#define AFX_UIPARTYBBSSELECTOR_H__CF79ECBE_7FBE_4251_A740_E6AD08881785__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3UIBase.h>

#define WANT_PARTY			0
#define WANT_PARTY_MEMBER	1

class CUIPartyBBSSelector : public CN3UIBase
{
protected:
	CN3UIButton*	m_pBtn_WantPartyMember;
	CN3UIButton*	m_pBtn_WantParty;

public:
	void MsgSend_PartyBBSKind(uint8_t byKind);
	bool Load(HANDLE hFile);
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);

	CUIPartyBBSSelector();
	virtual ~CUIPartyBBSSelector();

};

#endif // !defined(AFX_UIPARTYBBSSELECTOR_H__CF79ECBE_7FBE_4251_A740_E6AD08881785__INCLUDED_)

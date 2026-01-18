// UIPartyBBSSelector.h: interface for the CUIPartyBBSSelector class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIPARTYBBSSELECTOR_H__CF79ECBE_7FBE_4251_A740_E6AD08881785__INCLUDED_)
#define AFX_UIPARTYBBSSELECTOR_H__CF79ECBE_7FBE_4251_A740_E6AD08881785__INCLUDED_

#pragma once

#include <N3Base/N3UIBase.h>

enum e_PartyBBSKind : uint8_t
{
	WANT_PARTY        = 0,
	WANT_PARTY_MEMBER = 1
};

class CUIPartyBBSSelector : public CN3UIBase
{
protected:
	CN3UIButton* m_pBtn_WantPartyMember;
	CN3UIButton* m_pBtn_WantParty;

public:
	void MsgSend_PartyBBSKind(uint8_t byKind);
	bool Load(File& file) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;

	CUIPartyBBSSelector();
	~CUIPartyBBSSelector() override;
};

#endif // !defined(AFX_UIPARTYBBSSELECTOR_H__CF79ECBE_7FBE_4251_A740_E6AD08881785__INCLUDED_)

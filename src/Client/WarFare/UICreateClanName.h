// UICreateClanName.h: interface for the UICreateClanName class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__UICREATECLANNAME_H__)
#define __UICREATECLANNAME_H__

#pragma once

#include "GameDef.h"
#include <N3Base/N3UIBase.h>

class CUICreateClanName : public CN3UIBase
{
public:
	CN3UIString* m_pText_Title;
	CN3UIEdit* m_pEdit_ClanName;
	std::string m_szClanName;

public:
	CUICreateClanName();
	~CUICreateClanName() override;
	void Release() override;
	bool Load(File& file) override;
	void SetVisible(bool bVisible) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	void Open(int msg = 0);
	bool MakeClan();
	void MsgSend_MakeClan() const;
};

#endif //#if !defined(__UICREATECLANNAME_H__)

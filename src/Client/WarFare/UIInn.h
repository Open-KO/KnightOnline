// UIInn.h: interface for the UIInn class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__UIINN_H__)
#define __UIINN_H__

#pragma once

#include <N3Base/N3UIBase.h>

class CUIInn : public CN3UIBase
{
public:
	bool OnKeyPress(int iChar) override;
	void SetVisible(bool bVisible) override;
	void Message(int iMessageID);

	void MsgSend_OpenWareHouse();

	CUIInn();
	~CUIInn() override;

	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
};

#endif //#if !defined(__UIINN_H__)

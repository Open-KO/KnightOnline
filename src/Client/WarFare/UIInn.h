// UIInn.h: interface for the UIInn class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__UIINN_H__)
#define __UIINN_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3UIBase.h>

//////////////////////////////////////////////////////////////////////

class CUIInn : public CN3UIBase
{	
public:
	bool OnKeyPress(int iChar);
	void SetVisible(bool bVisible);
	void Message(int iMessageID);

	void MsgSend_OpenWareHouse();

	CUIInn();
	virtual ~CUIInn();

	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);
};

#endif //#if !defined(__UIINN_H__)

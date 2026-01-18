// UIDead.h: interface for the CUIDead class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIDEAD_H__B5FAFAB6_04A4_4708_945E_731AE35257EB__INCLUDED_)
#define AFX_UIDEAD_H__B5FAFAB6_04A4_4708_945E_731AE35257EB__INCLUDED_

#pragma once

#include <N3Base/N3UIBase.h>
#include "UIMessageBox.h"

class CUIDead : public CN3UIBase
{
protected:
	class CN3UIString* m_pTextAlive;
	class CN3UIString* m_pTextTown;
	CUIMessageBox m_MsgBox;
	bool m_bProcessing;

public:
	CUIDead();
	~CUIDead() override;
	void SetVisibleWithNoSound(bool bVisible, bool bWork = false, bool bReFocus = false) override;
	void SetVisible(bool bVisible) override;
	void MsgRecv_Revival(Packet& pkt);
	void MsgSend_Revival(uint8_t byType);
	void CallBackProc(int iID, uint32_t dwFlag) override;
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	bool Load(File& file) override;
};

#endif // !defined(AFX_UIDEAD_H__B5FAFAB6_04A4_4708_945E_731AE35257EB__INCLUDED_)

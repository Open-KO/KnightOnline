// UINPCEvent.h: interface for the UINPCEvent class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UINPCEVENT_H__27F6610C_5D61_4A22_97F2_93211B77BF9C__INCLUDED_)
#define AFX_UINPCEVENT_H__27F6610C_5D61_4A22_97F2_93211B77BF9C__INCLUDED_

#pragma once

#include <N3Base/N3UIBase.h>

enum e_NpcEvent : uint8_t
{
	NPC_EVENT_ITEM_TRADE   = 0,
	NPC_EVENT_TRADE_REPAIR = 1,
	NPC_EVENT_KNIGHTS      = 2
};

class CUINPCEvent : public CN3UIBase
{
	int m_iTradeID;
	int m_iIDTarget;
	CN3UIButton* m_pBtn_Repair;
	CN3UIString* m_pText_Repair;
	CN3UIString* m_pText_Title;

public:
	void SetVisible(bool bVisible) override;
	bool OnKeyPress(int iKey) override;
	void Release() override;

	CUINPCEvent();
	~CUINPCEvent() override;

	bool Load(File& file) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;

	void Open(e_NpcEvent eNpcEvent, int iTradeId = -1, int iIDTarget = -1);
	void Close();
};

#endif // !defined(AFX_UINPCEVENT_H__27F6610C_5D61_4A22_97F2_93211B77BF9C__INCLUDED_)

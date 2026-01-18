// ItemRepairMgr.h: interface for the CItemRepairMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ITEMREPAIRMGR_H__773AD64F_2ADD_44CC_BCE8_1EF2F38C76FB__INCLUDED_)
#define AFX_ITEMREPAIRMGR_H__773AD64F_2ADD_44CC_BCE8_1EF2F38C76FB__INCLUDED_

#pragma once

#include "GameBase.h"

class CItemRepairMgr : CGameBase
{
	struct __IconItemSkill* m_pspItemBack;
	int m_iArm;
	int m_iOrder;

public:
	CItemRepairMgr();
	~CItemRepairMgr() override;

	void Tick();
	void ReceiveResultFromServer(int iResult, int iUserGold);

	void UpdateUserTotalGold(int iGold);
	int CalcRepairGold(struct __IconItemSkill* spItem);
};

#endif // !defined(AFX_ITEMREPAIRMGR_H__773AD64F_2ADD_44CC_BCE8_1EF2F38C76FB__INCLUDED_)

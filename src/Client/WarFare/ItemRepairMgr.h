// ItemRepairMgr.h: interface for the CItemRepairMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ITEMREPAIRMGR_H__773AD64F_2ADD_44CC_BCE8_1EF2F38C76FB__INCLUDED_)
#define AFX_ITEMREPAIRMGR_H__773AD64F_2ADD_44CC_BCE8_1EF2F38C76FB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GameBase.h"

class CItemRepairMgr : CGameBase
{
	struct __IconItemSkill*	m_pspItemBack;	
	int						m_iArm;
	int						m_iiOrder;

public:
	CItemRepairMgr();
	virtual ~CItemRepairMgr();

	void	Tick();
	void	ReceiveResultFromServer(int iResult, int iUserGold);

	void	UpdateUserTotalGold(int iGold);
	int		CalcRepairGold(struct __IconItemSkill* spItem);
};

#endif // !defined(AFX_ITEMREPAIRMGR_H__773AD64F_2ADD_44CC_BCE8_1EF2F38C76FB__INCLUDED_)

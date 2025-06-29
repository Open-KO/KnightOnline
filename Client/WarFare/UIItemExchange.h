// UIItemExchange.h: interface for the CUIItemRepairExchange class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIITEMREPAIREXCHANGE_H__DFB5AC89_C348_41B9_830C_A4FFE08B1E55__INCLUDED_)
#define AFX_UIITEMREPAIREXCHANGE_H__DFB5AC89_C348_41B9_830C_A4FFE08B1E55__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GameDef.h"

#include "N3UIWndBase.h"

//////////////////////////////////////////////////////////////////////


class CUIItemExchange : public CN3UIWndBase
{
	CN3UIImage*		m_pImage_Repair;
	CN3UIImage*		m_pImage_Exchange;

	__IconItemSkill*	m_pMyInvWnd[MAX_ITEM_INVENTORY];
	__IconItemSkill*	m_pMyNpcWnd[MAX_ITEM_EX_RE_NPC];
	int					m_pMyNpcWndOriginIndex[MAX_ITEM_EX_RE_NPC];
	int					m_pTotalPrice;
	CUIImageTooltipDlg*	m_pUITooltipDlg;

private:
	void Close();

protected:
	e_UIWND_DISTRICT	GetWndDistrict(__IconItemSkill* spItem);
	int					GetItemiOrder(__IconItemSkill* spItem, e_UIWND_DISTRICT eWndDist);
	RECT				GetSampleRect();

public:
	void Release();

	CUIItemExchange();
	virtual ~CUIItemExchange();

	bool Load(HANDLE hFile);

	virtual uint32_t		MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld);
	virtual bool		ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);

	void Render();

	void Open();

	void ItemMoveFromInvToThis();
	void ItemMoveFromThisToInv();

	void InitIconWnd(e_UIWND eWnd);	

	void InitIconUpdate() {}
	__IconItemSkill* GetHighlightIconItem(CN3UIIcon* pUIIcon);

	void IconRestore();	
	void CancelIconDrop(__IconItemSkill* spItem);
	bool ReceiveIconDrop(__IconItemSkill* spItem, POINT ptCur);

	int	 CalcRepairGold(__IconItemSkill* spItem);
	void UpdateGoldValue();
	void UpdateUserTotalGold(int iGold);

	void UserPressCancel();		// And User Press Close.. ^^
	void UserPressOK();
	void ReceiveResultFromServer(int iResult, int iUserGold);
};

#endif // !defined(AFX_UIITEMREPAIREXCHANGE_H__DFB5AC89_C348_41B9_830C_A4FFE08B1E55__INCLUDED_)

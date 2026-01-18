// UIWareHouseDlg.h: interface for the UIWareHouseDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIWAREHOUSEDLG_H__47E9608F_598A_49CF_B10E_3D95FA647768__INCLUDED_)
#define AFX_UIWAREHOUSEDLG_H__47E9608F_598A_49CF_B10E_3D95FA647768__INCLUDED_

#pragma once

#include "GameDef.h"
#include "N3UIWndBase.h"

class CUIWareHouseDlg : public CN3UIWndBase
{
	friend class CUIInventory;

public:
	// 직접 접근해야 할 객체 참조 포인터
	__IconItemSkill* m_pMyWare[MAX_ITEM_WARE_PAGE][MAX_ITEM_TRADE];
	__IconItemSkill* m_pMyWareInv[MAX_ITEM_INVENTORY];
	CN3UIString* m_pStrMyGold;
	CN3UIString* m_pStrWareGold;

	//this_ui_add_start
	CN3UIButton* m_pBtnGold;
	CN3UIButton* m_pBtnGoldWareHouse;
	CN3UIButton* m_pBtnClose;
	CN3UIButton* m_pBtnPageUp;
	CN3UIButton* m_pBtnPageDown;
	//this_ui_add_end

	bool m_bSendedItemGold;
	int m_iGoldOffsetBackup;

	int m_iCurPage;
	CUIImageTooltipDlg* m_pUITooltipDlg;

protected:
	int GetItemiOrder(__IconItemSkill* spItem, e_UIWND_DISTRICT eWndDist);
	RECT GetSampleRect();

	e_UIWND_DISTRICT GetWndDistrict(__IconItemSkill* spItem);

public:
	CUIWareHouseDlg();
	~CUIWareHouseDlg() override;
	void Release() override;

	//this_ui_add_start
	void SetVisibleWithNoSound(bool bVisible, bool bWork = false, bool bReFocus = false) override;
	void SetVisible(bool bVisible) override;
	bool Load(File& file) override;
	bool OnKeyPress(int iKey) override;
	//this_ui_add_end

	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	bool OnMouseWheelEvent(short delta) override;
	void Render() override;
	void LeaveWareHouseState();
	void EnterWareHouseStateStart(int iWareGold);
	void AddItemInWare(int iItem, int iDurability, int iCount, int iIndex);
	void EnterWareHouseStateEnd();

	void InitIconWnd(e_UIWND eWnd) override;
	void InitIconUpdate() override;

	__IconItemSkill* GetHighlightIconItem(CN3UIIcon* pUIIcon) override;

	void IconRestore() override;

	bool ReceiveIconDrop(__IconItemSkill* spItem, POINT ptCur) override;

	void CancelIconDrop(__IconItemSkill* spItem) override;
	void AcceptIconDrop(__IconItemSkill* spItem) override;

	void SendToServerToWareMsg(int iItemID, uint8_t page, uint8_t startpos, uint8_t pos, int iCount);
	void SendToServerFromWareMsg(int iItemID, uint8_t page, uint8_t startpos, uint8_t pos, int iCount);
	void SendToServerWareToWareMsg(int iItemID, uint8_t page, uint8_t startpos, uint8_t destpos);
	void SendToServerInvToInvMsg(int iItemID, uint8_t page, uint8_t startpos, uint8_t destpos);
	void ReceiveResultToWareMsg(uint8_t bResult);
	void ReceiveResultFromWareMsg(uint8_t bResult);
	void ReceiveResultWareToWareMsg(uint8_t bResult);
	void ReceiveResultInvToInvMsg(uint8_t bResult);
	void ReceiveResultGoldToWareFail();
	void ReceiveResultGoldFromWareFail();

	// Item Count OK..
	void ItemCountOK();
	void ItemCountCancel();

	void GoldCountToWareOK();
	void GoldCountFromWareOK();
	void GoldCountToWareCancel();
	void GoldCountFromWareCancel();

	void ItemMoveFromInvToThis();
	void ItemMoveFromThisToInv();
};

#endif // !defined(AFX_UIWAREHOUSEDLG_H__47E9608F_598A_49CF_B10E_3D95FA647768__INCLUDED_)

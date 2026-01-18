// UITransactionDlg.h: interface for the CUITransactionDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UITRANSACTIONDLG_H__42671245_FF4F_42FC_AF7B_DACEDA8734B7__INCLUDED_)
#define AFX_UITRANSACTIONDLG_H__42671245_FF4F_42FC_AF7B_DACEDA8734B7__INCLUDED_

#pragma once

#include "GameDef.h"
#include "N3UIWndBase.h"

enum e_NpcTrade : uint8_t
{
	UI_BLACKSMITH = 1,
	UI_STORE,
	UI_INN
};

//////////////////////////////////////////////////////////////////////

class CUIMsgBoxOkCancel;
class CUITransactionDlg : public CN3UIWndBase
{
	friend class CUIInventory;

public:
	// 직접 접근해야 할 객체 참조 포인터
	__IconItemSkill* m_pMyTrade[MAX_ITEM_TRADE_PAGE][MAX_ITEM_TRADE];
	__IconItemSkill* m_pMyTradeInv[MAX_ITEM_INVENTORY];
	CN3UIString* m_pStrMyGold;
	CN3UIString* m_pText_Weight;

	int m_iCurPage;
	int m_iTradeID;
	int m_iNpcID;
	CUIImageTooltipDlg* m_pUITooltipDlg;

	CN3UIImage* m_pUIInn;
	CN3UIImage* m_pUIBlackSmith;
	CN3UIImage* m_pUIStore;

	//this_ui_add_start
	CN3UIButton* m_pBtnClose;
	CN3UIButton* m_pBtnPageUp;
	CN3UIButton* m_pBtnPageDown;
	//this_ui_add_end

	CUIMsgBoxOkCancel* m_pUIMsgBoxOkCancel;

protected:
	int GetItemiOrder(__IconItemSkill* spItem, e_UIWND_DISTRICT eWndDist);
	RECT GetSampleRect();

	e_UIWND_DISTRICT GetWndDistrict(__IconItemSkill* spItem);

public:
	CUITransactionDlg();
	~CUITransactionDlg() override;
	void Release() override;

	//this_ui_add_start
	bool OnKeyPress(int iKey) override;
	bool Load(File& file) override;
	void SetVisibleWithNoSound(bool bVisible, bool bWork = false, bool bReFocus = false) override;
	void SetVisible(bool bVisible) override;
	//this_ui_add_end

	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	bool OnMouseWheelEvent(short delta) override;
	void Render() override;
	void LeaveTransactionState();
	void EnterTransactionState();

	void InitIconWnd(e_UIWND eWnd) override;
	void InitIconUpdate() override;

	__IconItemSkill* GetHighlightIconItem(CN3UIIcon* pUIIcon) override;

	void IconRestore() override;

	bool ReceiveIconDrop(__IconItemSkill* spItem, POINT ptCur) override;

	void CancelIconDrop(__IconItemSkill* spItem) override;
	void AcceptIconDrop(__IconItemSkill* spItem) override;

	void SendToServerSellMsg(int itemID, byte pos, int iCount);
	void SendToServerBuyMsg(int itemID, byte pos, int iCount);
	void SendToServerMoveMsg(int itemID, byte startpos, byte destpos);

	// 물건 구입이 서버에게 보내기전 성공..
	void ReceiveItemDropByTradeSuccess();

	CN3UIBase* GetChildButtonByName(const std::string& szFN);

	// Item Count OK..
	void ItemCountOK();
	void ItemCountCancel();

	// MsgBox Ok Cancel
	void CallBackProc(int iID, uint32_t dwFlag) override;
	void OnConfirm();
	void OnCancel();

	void ItemMoveFromInvToThis();
	void ItemMoveFromThisToInv();

	void ReceiveResultTradeFromServer(byte bResult, byte bType, int iMoney);
	void ReceiveResultTradeMoveSuccess();
	void ReceiveResultTradeMoveFail();

	// 타이틀 표시..
	void ShowTitle(e_NpcTrade eNT);

	void GoldUpdate();
	void UpdateWeight(const std::string& szWeight);
	static std::string GetItemName(const __IconItemSkill* spItem);
};

#endif // !defined(AFX_UITRANSACTIONDLG_H__42671245_FF4F_42FC_AF7B_DACEDA8734B7__INCLUDED_)

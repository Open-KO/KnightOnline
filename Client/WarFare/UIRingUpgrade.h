// CUIRingUpgrade.h: interface for the CUIRingUpgrade class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIRingUpgrade_H__fd9f5093_0ed3_4c08_9e31_19c40773b24d__INCLUDED_)
#define AFX_UIRingUpgrade_H__fd9f5093_0ed3_4c08_9e31_19c40773b24d__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "N3UIBase.h"
#include "N3UIArea.h"
#include "N3UIString.h"
#include "N3UIImage.h"
#include "N3UIButton.h"
#include "GameProcedure.h"

#include "GameDef.h"
#include "N3UIWndBase.h"



//////////////////////////////////////////////////////////////////////

class CUIRingUpgrade : public CN3UIWndBase
{
	friend class CUIInventory;

public:
	// 직접 접근해야 할 객체 참조 포인터
	__IconItemSkill* m_pRingUpgradeScrollSlot[MAX_RING_UPGRADE_SCROLL_SLOT];	// Ring Upgrade Scroll Slot
	__IconItemSkill* m_pRingUpgradeResultSlot;	//Result Slot after uprade
	__IconItemSkill* m_pMyUpgradeSLot[MAX_RING_UPGRADE_SLOT]; //3 Ring slot for merge
	__IconItemSkill* m_pMyUpgradeInv[MAX_ITEM_INVENTORY];
	__IconItemSkill* m_pBackupUpgradeInv[MAX_ITEM_INVENTORY];
	CN3UIString* m_pStrMyGold;


	CUIImageTooltipDlg* m_pUITooltipDlg;


	//this_ui_add_start
	CN3UIButton* m_pBtnClose;
	CN3UIButton* m_pBtnOk;
	CN3UIButton* m_pBtnCancel;
	CN3UIButton* m_pBtnConversation;
	CN3UIArea* m_pAreaUpgradeScroll;
	//this_ui_add_end


protected:
	int					GetItemiOrder(__IconItemSkill* spItem, e_UIWND_DISTRICT eWndDist);
	RECT GetSampleRect();
	e_UIWND_DISTRICT	GetWndDistrict(__IconItemSkill* spItem);
	uint32_t m_dwRingUpgradeSlotRefID = 0;

public:
	CUIRingUpgrade();
	~CUIRingUpgrade() override;
	void				Release() override;

	//this_ui_add_start
	bool				OnKeyPress(int iKey) override;
	bool				Load(HANDLE hFile) override;
	void				SetVisibleWithNoSound(bool bVisible, bool bWork = false, bool bReFocus = false) override;
	void				SetVisible(bool bVisible) override;
	//this_ui_add_end

	uint32_t			MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld) override;
	bool				ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	void				Render() override;
	void				Close();
	void				Open();

	void				InitIconWnd(e_UIWND eWnd) override;

	__IconItemSkill* GetHighlightIconItem(CN3UIIcon* pUIIcon) override;

	void				IconRestore();

	bool				ReceiveIconDrop(__IconItemSkill* spItem, POINT ptCur) override;
	void				CancelIconDrop(__IconItemSkill* spItem) override;
	void				AcceptIconDrop(__IconItemSkill* spItem) override;
	void				ItemMoveFromInvToThis();
	void				ItemMoveFromThisToInv();
	void				GoldUpdate();
	void				InitIconUpdate() {};
	void                RestoreInventoryFromBackup();
	bool				IsRingUpgradeScrollorBlueTrina(uint32_t dwID);
	bool IsAllowedRingItem(__IconItemSkill* spItem);
	void DeleteIconItemSkill(__IconItemSkill*& pItem);
};

#endif // !defined(AFX_UIRingUpgrade_H__fd9f5093_0ed3_4c08_9e31_19c40773b24d__INCLUDED_)

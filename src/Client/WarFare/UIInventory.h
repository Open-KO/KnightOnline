// UIInventory.h: interface for the CUIInventory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIInventory_H__863D2E52_65A8_47E1_BCBE_6281692DDCA2__INCLUDED_)
#define AFX_UIInventory_H__863D2E52_65A8_47E1_BCBE_6281692DDCA2__INCLUDED_

#pragma once

#include "GameDef.h"
#include "N3UIWndBase.h"
#include "ItemRepairMgr.h"

inline constexpr int ITEM_SLOT_POS_EAR_RIGHT       = 0;
inline constexpr int ITEM_SLOT_POS_HEAD            = 1;
inline constexpr int ITEM_SLOT_POS_EAR_LEFT        = 2;
inline constexpr int ITEM_SLOT_POS_NECK            = 3;
inline constexpr int ITEM_SLOT_POS_UPPER           = 4;
inline constexpr int ITEM_SLOT_POS_SHOULDER        = 5;
inline constexpr int ITEM_SLOT_POS_HAND_RIGHT      = 6;
inline constexpr int ITEM_SLOT_POS_BELT            = 7;
inline constexpr int ITEM_SLOT_POS_HAND_LEFT       = 8;
inline constexpr int ITEM_SLOT_POS_RING_RIGHT      = 9;
inline constexpr int ITEM_SLOT_POS_LOWER           = 10;
inline constexpr int ITEM_SLOT_POS_RING_LEFT       = 11;
inline constexpr int ITEM_SLOT_POS_GLOVES          = 12;
inline constexpr int ITEM_SLOT_POS_SHOES           = 13;

inline constexpr int ITEM_ATTACH_POS_DUAL          = 0;
inline constexpr int ITEM_ATTACH_POS_HAND_RIGHT    = 1;
inline constexpr int ITEM_ATTACH_POS_HAND_LEFT     = 2;
inline constexpr int ITEM_ATTACH_POS_TWOHAND_RIGHT = 3;
inline constexpr int ITEM_ATTACH_POS_TWOHAND_LEFT  = 4;
inline constexpr int ITEM_ATTACH_POS_UPPER         = 5;
inline constexpr int ITEM_ATTACH_POS_LOWER         = 6;
inline constexpr int ITEM_ATTACH_POS_HEAD          = 7;
inline constexpr int ITEM_ATTACH_POS_ARM           = 8;
inline constexpr int ITEM_ATTACH_POS_FOOT          = 9;
inline constexpr int ITEM_ATTACH_POS_EAR           = 10;
inline constexpr int ITEM_ATTACH_POS_NECK          = 11;
inline constexpr int ITEM_ATTACH_POS_FINGER        = 12;
inline constexpr int ITEM_ATTACH_POS_CLOAK         = 13;
inline constexpr int ITEM_ATTACH_POS_BELT          = 14;
inline constexpr int ITEM_ATTACH_POS_INVENTORY     = 15;

// 인벤토리 상태.. 기존의 상태와 구별하기 위해.. 수리 상태가 추가됐다..
enum e_InvenState : uint8_t
{
	INV_STATE_NORMAL = 1,
	INV_STATE_REPAIR,
};

//////////////////////////////////////////////////////////////////////

class CUIInventory : public CN3UIWndBase
{
	bool m_bDestoyDlgAlive;

protected:
	e_InvenState m_eInvenState;
	CItemRepairMgr m_cItemRepairMgr;

	bool m_bOpenningNow; // 열리고 있다..
	bool m_bClosingNow;  // 닫히고 있다..
	float m_fMoveDelta;  // 부드럽게 열리고 닫히게 만들기 위해서 현재위치 계산에 부동소수점을 쓴다..

	CN3UIArea* m_pArea_User;
	CN3UIArea* m_pArea_Destroy;

	CN3UIString* m_pText_Weight;

	CUIImageTooltipDlg* m_pUITooltipDlg;

	int m_iRBtnDownOffs;
	bool m_bRBtnProcessing;

public:
	__IconItemSkill* m_pMySlot[ITEM_SLOT_COUNT];
	__IconItemSkill* m_pMyInvWnd[MAX_ITEM_INVENTORY];

protected:
	e_UIWND_DISTRICT GetWndDistrict(__IconItemSkill* spItem);
	int GetItemiOrder(__IconItemSkill* spItem, e_UIWND_DISTRICT eWndDist);

	RECT GetSampleRect();

	// 검사하는 루틴..
	bool IsValidRaceAndClass(__TABLE_ITEM_BASIC* pItem, __TABLE_ITEM_EXT* pItemExt);
	bool IsValidPosFromInvToArm(int iOrder);
	bool IsValidPosFromArmToArm(int iOrder);
	bool IsValidPosFromArmToArmInverse(int iOrder);

	void ItemAdd(__TABLE_ITEM_BASIC* pItem, __TABLE_ITEM_EXT* pItemExt, e_ItemSlot eSlot);
	void ItemDelete(__TABLE_ITEM_BASIC* pItem, __TABLE_ITEM_EXT* pItemExt, e_ItemSlot eSlot);

	inline bool InvOpsSomething(__IconItemSkill* spItem);
	int GetArmDestinationIndex(__IconItemSkill* spItem);
	int GetInvDestinationIndex(__IconItemSkill* spItem);

	// Operations
public:
	//this_ui_add_start
	int GetIndexItemCount(uint32_t dwIndex);
	void SetVisibleWithNoSound(bool bVisible, bool bWork = false, bool bReFocus = false) override;
	void SetVisible(bool bVisible) override;
	bool OnKeyPress(int iKey) override;
	//this_ui_add_end

	void Release() override;
	void ReleaseItem();

	void Open(e_InvenState eIS = INV_STATE_NORMAL);
	void Close(bool bByKey = false);

	void Tick() override;
	void Render() override;

	void InitIconWnd(e_UIWND eWnd) override;
	void InitIconUpdate() override;

	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;

	__IconItemSkill* GetHighlightIconItem(CN3UIIcon* pUIIcon) override;

	bool CheckIconDropIfSuccessSendToServer(__IconItemSkill* spItem);
	void IconRestore() override;

	bool ReceiveIconDrop(__IconItemSkill* spItem, POINT ptCur) override;

	void CancelIconDrop(__IconItemSkill* spItem) override;
	void AcceptIconDrop(__IconItemSkill* spItem) override;

	void SendInvMsg(uint8_t bDir, int iItemID, int SrcPos, int DestPos);

	void ReceiveResultFromServer(uint8_t bResult = 0x01);
	void ReceiveResultItemRemoveFromServer(int iResult);

	// Duration Change..
	void DurabilityChange(e_ItemSlot eSlot, int iDurability);
	e_InvenState GetInvState()
	{
		return m_eInvenState;
	}

	void ReceiveResultFromServer(int iResult, int iUserGold);

	int GetCountInInvByID(int iID) const;

	// 소모성 아이템을 소비한 경우 or Not..
	void ItemCountChange(int iDistrict, int iIndex, int iCount, int iID, int iDurability);

	// child dlg func..
	void ItemDestroyOK();
	void ItemDestroyCancel();

	bool CheckWeightValidate(__IconItemSkill* spItem);
	void GoldUpdate();

	bool HasAnyItemInSlot();
	void UpdateWeight(const std::string& str);

	int GetIndexInArea(POINT pt);

	CUIInventory();
	~CUIInventory() override;
};

#endif // !defined(AFX_UIInventory_H__863D2E52_65A8_47E1_BCBE_6281692DDCA2__INCLUDED_)


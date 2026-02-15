#pragma once

#include <N3Base/N3UIBase.h>

#include "GameDef.h"

enum e_UIWND_DISTRICT : uint8_t;
struct __IconItemSkill;
class CN3UIIcon;
class CUIImageTooltipDlg;
class CUIMsgBoxOkCancel;
class CUIItemUpgrade : public CN3UIBase
{
private:
	static constexpr int FLIPFLOP_MAX_FRAMES      = 19;
	static constexpr int CHILD_UI_MSGBOX_OKCANCEL = 1;

	enum class AnimationState : uint8_t
	{
		None,
		Start,
		FlipFlop,
		Result,
		CoverOpening,
		Done
	};

	AnimationState m_eAnimationState                     = AnimationState::None;

	float m_fAnimationTimer                              = 0.0f;
	int m_iCurrentFrame                                  = 0;
	bool m_bUpgradeSucceeded                             = false;
	bool m_bUpgradeInProgress                            = false;
	int m_iNpcID                                         = 0;

	RECT m_rcCover1Original                              = {};
	RECT m_rcCover2Original                              = {};

	CN3UIButton* m_pBtnClose                             = nullptr;
	CN3UIButton* m_pBtnOk                                = nullptr;
	CN3UIButton* m_pBtnCancel                            = nullptr;
	CN3UIString* m_pStrMyGold                            = nullptr;
	CN3UIArea* m_pAreaUpgrade                            = nullptr;
	CN3UIArea* m_pAreaResult                             = nullptr;
	CN3UIArea* m_pAreaInv[MAX_ITEM_INVENTORY]            = {};
	CN3UIString* m_pStrInv[MAX_ITEM_INVENTORY]           = {};
	CN3UIArea* m_pSlotArea[ANVIL_REQ_MAX]                = {};
	CN3UIImage* m_pImgFail[FLIPFLOP_MAX_FRAMES]          = {};
	CN3UIImage* m_pImgSuccess[FLIPFLOP_MAX_FRAMES]       = {};
	CN3UIImage* m_pImageCover1                           = nullptr;
	CN3UIImage* m_pImageCover2                           = nullptr;

	CUIImageTooltipDlg* m_pUITooltipDlg                  = nullptr;
	CUIMsgBoxOkCancel* m_pUIMsgBoxOkCancel               = nullptr;

	__IconItemSkill* m_pItemBeingDragged                 = nullptr;
	int m_iItemBeingDraggedSourcePos                     = -1;

	__IconItemSkill* m_pMyUpgradeInv[MAX_ITEM_INVENTORY] = {};
	__IconItemSkill* m_pRequirementSlot[ANVIL_REQ_MAX]   = {};
	int8_t m_iRequirementSlotInvPos[ANVIL_REQ_MAX]       = {};
	int8_t m_iUpgradeItemSlotInvPos                      = -1;

public:
	CUIItemUpgrade();
	~CUIItemUpgrade() override;
	void Release() override;
	void Close();
	void SetVisibleWithNoSound(bool bVisible, bool bWork = false, bool bReFocus = false) override;
	void MsgRecv_ItemUpgrade(Packet& pkt);
	void SetNpcID(int iNpcID);
	void SetVisible(bool bVisible) override;

private:
	RECT GetSampleRect();
	e_UIWND_DISTRICT GetWndDistrict(const POINT ptCur) const;
	bool HandleInventoryIconRightClick(CN3UIBase* pUIIcon);
	bool OnKeyPress(int iKey) override;

	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld) override;
	bool Load(File& file) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	void Render() override;

	__IconItemSkill* GetHighlightIconItem(CN3UIBase* pUIIcon) const;
	bool GetSelectedIconInfo(
		CN3UIBase* pUIIcon, int* piOrder = nullptr, e_UIWND_DISTRICT* peDistrict = nullptr, __IconItemSkill** pspItem = nullptr) const;
	void CancelIconDrop();
	bool ReceiveIconDrop();
	void CopyInventoryItems();
	void ResetUpgradeInventory();
	void GoldUpdate();
	bool IsAllowedUpgradeItem(const __IconItemSkill* spItem) const;
	void SendToServerUpgradeMsg();
	void CallBackProc(int iID, uint32_t dwFlag) override;

	void FlipFlopAnim();
	void AnimClose();
	void StartUpgradeAnim();
	void UpdateCoverAnimation();
	void UpdateFlipFlopAnimation();
	void HideAllAnimationFrames();
	void SetRequirementItemSlot(__IconItemSkill* spItem, int iSrcOrder, int iDstOrder);
	bool IsValidRequirementItem(const __IconItemSkill* pSrc) const;
	void Tick() override;
	void ReduceInvItemStackSize(__IconItemSkill* spItem);
	void RestoreInvItemStackSize(__IconItemSkill* spItem);
};

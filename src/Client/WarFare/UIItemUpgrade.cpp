#include "StdAfx.h"
#include "UIItemUpgrade.h"
#include "APISocket.h"
#include "GameProcMain.h"
#include "IconItemSkill.h"
#include "LocalInput.h"
#include "N3UIIcon.h"
#include "N3UIWndBase.h"
#include "PlayerMySelf.h"
#include "UIImageTooltipDlg.h"
#include "UIInventory.h"
#include "UIManager.h"
#include "UIMsgBoxOkCancel.h"

#include "text_resources.h"

#include <N3Base/N3UIArea.h>
#include <N3Base/N3UIString.h>
#include <N3Base/N3UIImage.h>
#include <N3Base/N3UIButton.h>

#include <array>
#include <unordered_set>

CUIItemUpgrade::CUIItemUpgrade()
{
	for (int i = 0; i < ANVIL_REQ_MAX; i++)
		m_iRequirementSlotInvPos[i] = -1;
}

CUIItemUpgrade::~CUIItemUpgrade()
{
	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (m_pMyUpgradeInv[i] == nullptr)
			continue;

		// NOTE: As a child, this is freed by CN3UIBase
		m_pMyUpgradeInv[i]->pUIIcon = nullptr;
		delete m_pMyUpgradeInv[i];
		m_pMyUpgradeInv[i] = nullptr;
	}
}

void CUIItemUpgrade::Release()
{
	m_eAnimationState    = AnimationState::None;
	m_fAnimationTimer    = 0.0f;
	m_iCurrentFrame      = 0;
	m_bUpgradeSucceeded  = false;
	m_bUpgradeInProgress = false;
	m_iNpcID             = 0;

	m_rcCover1Original   = {};
	m_rcCover2Original   = {};

	m_pBtnClose          = nullptr;
	m_pBtnOk             = nullptr;
	m_pBtnCancel         = nullptr;
	m_pStrMyGold         = nullptr;
	m_pAreaUpgrade       = nullptr;
	m_pAreaResult        = nullptr;

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		m_pAreaInv[i] = nullptr;
		m_pStrInv[i]  = nullptr;

		if (m_pMyUpgradeInv[i] != nullptr)
		{
			m_pMyUpgradeInv[i]->pUIIcon = nullptr;
			delete m_pMyUpgradeInv[i]->pUIIcon;
			m_pMyUpgradeInv[i] = nullptr;
		}
	}

	for (int i = 0; i < ANVIL_REQ_MAX; i++)
	{
		m_pSlotArea[i]              = nullptr;
		m_iRequirementSlotInvPos[i] = -1;
	}

	for (int i = 0; i < FLIPFLOP_MAX_FRAMES; i++)
	{
		m_pImgFail[i]    = nullptr;
		m_pImgSuccess[i] = nullptr;
	}

	m_pImageCover1      = nullptr;
	m_pImageCover2      = nullptr;

	m_pUITooltipDlg     = nullptr;
	m_pUIMsgBoxOkCancel = nullptr;

	if (m_pItemBeingDragged != nullptr)
	{
		delete m_pItemBeingDragged->pUIIcon;
		m_pItemBeingDragged->pUIIcon = nullptr;
		delete m_pItemBeingDragged;
		m_pItemBeingDragged = nullptr;
	}

	m_iItemBeingDraggedSourcePos = -1;
	m_iUpgradeItemSlotInvPos     = -1;

	CN3UIBase::Release();
}

void CUIItemUpgrade::Tick()
{
	if (m_pImageCover1 != nullptr && m_pImageCover2 != nullptr)
	{
		switch (m_eAnimationState)
		{
			case AnimationState::Start:
				StartUpgradeAnim();
				break;

			case AnimationState::FlipFlop:
				UpdateFlipFlopAnimation();
				break;

			case AnimationState::Result:
				if (m_bUpgradeSucceeded && m_iUpgradeItemSlotInvPos >= 0 && m_iUpgradeItemSlotInvPos < MAX_ITEM_INVENTORY)
				{
					__IconItemSkill* spItem = m_pMyUpgradeInv[m_iUpgradeItemSlotInvPos];
					if (spItem != nullptr && m_pAreaResult != nullptr)
					{
						spItem->pUIIcon->SetRegion(m_pAreaResult->GetRegion());
						spItem->pUIIcon->SetMoveRect(m_pAreaResult->GetRegion());
					}
				}

				m_eAnimationState = AnimationState::CoverOpening;
				break;

			case AnimationState::CoverOpening:
				UpdateCoverAnimation();
				break;

			case AnimationState::None:
				m_fAnimationTimer = 0.0f;
				m_iCurrentFrame   = 0;
				break;

			case AnimationState::Done:
				break;
		}
	}

	CN3UIBase::Tick();
}

void CUIItemUpgrade::Render()
{
	if (!m_bVisible)
		return;

	POINT ptCur = CGameProcedure::s_pLocalInput->MouseGetPos();

	if (m_pUITooltipDlg != nullptr)
		m_pUITooltipDlg->DisplayTooltipsDisable();

	__IconItemSkill* spItemTooltip = nullptr;

	for (auto itor = m_Children.rbegin(); m_Children.rend() != itor; ++itor)
	{
		CN3UIBase* pChild = *itor;
		if (GetState() == UI_STATE_ICON_MOVING && pChild->UIType() == UI_TYPE_ICON && m_pItemBeingDragged != nullptr
			&& pChild == m_pItemBeingDragged->pUIIcon)
			continue;

		pChild->Render();

		if (GetState() == UI_STATE_COMMON_NONE && pChild->UIType() == UI_TYPE_ICON && (pChild->GetStyle() & UISTYLE_ICON_HIGHLIGHT))
			spItemTooltip = GetHighlightIconItem(pChild);
	}

	if (GetState() == UI_STATE_ICON_MOVING && m_pItemBeingDragged != nullptr && m_pItemBeingDragged->pUIIcon != nullptr)
		m_pItemBeingDragged->pUIIcon->Render();

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		CN3UIString* pStr = m_pStrInv[i];
		if (pStr == nullptr)
			continue;

		if (m_pMyUpgradeInv[i] == nullptr || m_pMyUpgradeInv[i]->iCount <= 0 || !m_pMyUpgradeInv[i]->IsStackable())
			continue;

		pStr->SetVisibleWithNoSound(true);
		pStr->SetStringAsInt(m_pMyUpgradeInv[i]->iCount);
		pStr->Render();
		pStr->SetVisibleWithNoSound(false);
	}

	if (spItemTooltip != nullptr)
		m_pUITooltipDlg->DisplayTooltipsEnable(ptCur.x, ptCur.y, spItemTooltip, false, false);
}

__IconItemSkill* CUIItemUpgrade::GetHighlightIconItem(CN3UIBase* pUIIcon) const
{
	for (int i = 0; i < ANVIL_REQ_MAX; i++)
	{
		if (m_pRequirementSlot[i] != nullptr && m_pRequirementSlot[i]->pUIIcon == pUIIcon)
			return m_pRequirementSlot[i];
	}

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (m_pMyUpgradeInv[i] != nullptr && m_pMyUpgradeInv[i]->pUIIcon == pUIIcon)
			return m_pMyUpgradeInv[i];
	}

	return nullptr;
}

bool CUIItemUpgrade::GetSelectedIconInfo(CN3UIBase* pUIIcon, int* piOrder, e_UIWND_DISTRICT* peDistrict, __IconItemSkill** pspItem) const
{
	if (pUIIcon == nullptr || m_pAreaResult == nullptr || m_pAreaUpgrade == nullptr)
		return false;

	for (int i = 0; i < ANVIL_REQ_MAX; i++)
	{
		__IconItemSkill* spItem = m_pRequirementSlot[i];
		if (spItem == nullptr || spItem->pUIIcon != pUIIcon)
			continue;

		if (piOrder != nullptr)
			*piOrder = i;

		if (peDistrict != nullptr)
			*peDistrict = UIWND_DISTRICT_UPGRADE_SLOT;

		if (pspItem != nullptr)
			*pspItem = spItem;

		return true;
	}

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		__IconItemSkill* spItem = m_pMyUpgradeInv[i];
		if (spItem == nullptr || spItem->pUIIcon != pUIIcon)
			continue;

		if (piOrder != nullptr)
			*piOrder = i;

		if (peDistrict != nullptr)
		{
			// Hacky result slot check; this should be placed, so it should match.
			// Ideally this would just be assigned to an icon slot and not need to do any of this.
			const RECT rcIcon    = spItem->pUIIcon->GetRegion();
			const RECT rcResult  = m_pAreaResult->GetRegion();
			const RECT rcUpgrade = m_pAreaUpgrade->GetRegion();
			if (rcIcon.left == rcResult.left && rcIcon.top == rcResult.top && rcIcon.right == rcResult.right
				&& rcIcon.bottom == rcResult.bottom)
				*peDistrict = UIWND_DISTRICT_UPGRADE_RESULT_SLOT;
			else if (rcIcon.left == rcUpgrade.left && rcIcon.top == rcUpgrade.top && rcIcon.right == rcUpgrade.right
					 && rcIcon.bottom == rcUpgrade.bottom)
				*peDistrict = UIWND_DISTRICT_UPGRADE_SLOT;
			else
				*peDistrict = UIWND_DISTRICT_UPGRADE_INV;
		}

		if (pspItem != nullptr)
			*pspItem = spItem;

		return true;
	}

	if (piOrder != nullptr)
		*piOrder = -1;

	if (peDistrict != nullptr)
		*peDistrict = UIWND_DISTRICT_UNKNOWN;

	if (pspItem != nullptr)
		*pspItem = nullptr;

	return false;
}

void CUIItemUpgrade::SetNpcID(int iNpcID)
{
	m_iNpcID = iNpcID;
}

void CUIItemUpgrade::GoldUpdate()
{
	if (m_pStrMyGold == nullptr)
		return;

	std::string formattedGold = CGameBase::FormatNumber(CGameBase::s_pPlayer->m_InfoExt.iGold);
	m_pStrMyGold->SetString(formattedGold);
}

void CUIItemUpgrade::CopyInventoryItems()
{
	CUIInventory* pInven = CGameProcedure::s_pProcMain->m_pUIInventory;
	if (pInven == nullptr)
		return;

	m_iUpgradeItemSlotInvPos = -1;

	for (int i = 0; i < ANVIL_REQ_MAX; i++)
		m_iRequirementSlotInvPos[i] = -1;

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (m_pMyUpgradeInv[i] != nullptr)
		{
			delete m_pMyUpgradeInv[i]->pUIIcon;
			m_pMyUpgradeInv[i]->pUIIcon = nullptr;
			delete m_pMyUpgradeInv[i];
			m_pMyUpgradeInv[i] = nullptr;
		}

		__IconItemSkill* spItemInv = pInven->m_pMyInvWnd[i];
		if (spItemInv != nullptr)
		{
			__IconItemSkill* spItem = spItemInv->Clone(this);

			CN3UIArea* pArea        = m_pAreaInv[i];
			if (pArea != nullptr && spItem->pUIIcon != nullptr)
			{
				spItem->pUIIcon->SetRegion(pArea->GetRegion());
				spItem->pUIIcon->SetMoveRect(pArea->GetRegion());
			}

			m_pMyUpgradeInv[i] = spItem;
		}
	}
}

void CUIItemUpgrade::Close()
{
	bool bWork = IsVisible();
	SetVisibleWithNoSound(false, bWork, false);
}

bool CUIItemUpgrade::ReceiveIconDrop()
{
	if (m_pItemBeingDragged == nullptr)
		return false;

	POINT ptCur = CGameProcedure::s_pLocalInput->MouseGetPos();
	if (IsAllowedUpgradeItem(m_pItemBeingDragged))
	{
		if (m_iUpgradeItemSlotInvPos == -1 && m_pAreaUpgrade->IsIn(ptCur.x, ptCur.y))
		{
			if (m_pItemBeingDragged->pUIIcon != nullptr && m_pAreaUpgrade != nullptr)
			{
				m_pItemBeingDragged->pUIIcon->SetRegion(m_pAreaUpgrade->GetRegion());
				m_pItemBeingDragged->pUIIcon->SetMoveRect(m_pAreaUpgrade->GetRegion());
			}

			m_iUpgradeItemSlotInvPos = m_iItemBeingDraggedSourcePos;
			return true;
		}
	}
	else if (IsValidRequirementItem(m_pItemBeingDragged))
	{
		for (int i = 0; i < ANVIL_REQ_MAX; i++)
		{
			CN3UIArea* pArea = m_pSlotArea[i];
			if (pArea == nullptr || !pArea->IsIn(ptCur.x, ptCur.y))
				continue;

			SetRequirementItemSlot(m_pItemBeingDragged, m_iItemBeingDraggedSourcePos, i);
			return true;
		}
	}

	return false;
}

void CUIItemUpgrade::CancelIconDrop()
{
	if (m_pItemBeingDragged == nullptr)
		return;

	// If stackable, restore stack size in inventory
	// Note that we assume this is from the inventory because it's the only draggable source.
	RestoreInvItemStackSize(m_pMyUpgradeInv[m_iItemBeingDraggedSourcePos]);

	delete m_pItemBeingDragged->pUIIcon;
	m_pItemBeingDragged->pUIIcon = nullptr;
	delete m_pItemBeingDragged;
	m_pItemBeingDragged          = nullptr;
	m_iItemBeingDraggedSourcePos = -1;
}

uint32_t CUIItemUpgrade::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;

	if (!m_bVisible)
		return dwRet;

	if (GetState() == UI_STATE_ICON_MOVING && m_pItemBeingDragged != nullptr && m_pItemBeingDragged->pUIIcon != nullptr)
	{
		RECT region = GetSampleRect();
		m_pItemBeingDragged->pUIIcon->SetRegion(region);
		m_pItemBeingDragged->pUIIcon->SetMoveRect(region);

		dwRet |= UI_MOUSEPROC_DONESOMETHING;
	}

	return CN3UIBase::MouseProc(dwFlags, ptCur, ptOld) | dwRet;
}

// Returns a rectangle centered at the mouse position, used for moving icons.
RECT CUIItemUpgrade::GetSampleRect()
{
	__ASSERT(m_pAreaResult, "m_pAreaResult not loaded");

	if (m_pAreaResult == nullptr)
		return {};

	POINT ptCur   = CGameProcedure::s_pLocalInput->MouseGetPos();
	RECT rect     = m_pAreaResult->GetRegion();

	float fWidth  = (float) (rect.right - rect.left) * 0.5f;
	float fHeight = (float) (rect.bottom - rect.top) * 0.5f;
	rect.left     = ptCur.x - (int) fWidth;
	rect.right    = ptCur.x + (int) fWidth;
	rect.top      = ptCur.y - (int) fHeight;
	rect.bottom   = ptCur.y + (int) fHeight;
	return rect;
}

e_UIWND_DISTRICT CUIItemUpgrade::GetWndDistrict(const POINT ptCur) const
{
	if (m_pAreaUpgrade != nullptr && m_pAreaUpgrade->IsIn(ptCur.x, ptCur.y))
		return UIWND_DISTRICT_UPGRADE_SLOT;

	if (m_pAreaResult != nullptr && m_pAreaResult->IsIn(ptCur.x, ptCur.y))
		return UIWND_DISTRICT_UPGRADE_RESULT_SLOT;

	for (int i = 0; i < ANVIL_REQ_MAX; i++)
	{
		if (m_pSlotArea[i] != nullptr && m_pSlotArea[i]->IsIn(ptCur.x, ptCur.y))
			return UIWND_DISTRICT_UPGRADE_SLOT;
	}

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (m_pAreaInv[i] != nullptr && m_pAreaInv[i]->IsIn(ptCur.x, ptCur.y))
			return UIWND_DISTRICT_UPGRADE_INV;
	}

	return UIWND_DISTRICT_UNKNOWN;
}

// Handles UI messages such as button clicks and icon events.
bool CUIItemUpgrade::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (pSender == nullptr)
		return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtnClose)
		{
			Close();
		}
		else if (pSender == m_pBtnCancel)
		{
			if (!m_bUpgradeInProgress)
				ResetUpgradeInventory();
		}
		else if (pSender == m_pBtnOk)
		{
			if (!m_bUpgradeInProgress && m_pUIMsgBoxOkCancel != nullptr)
				m_pUIMsgBoxOkCancel->ShowWindow(CHILD_UI_MSGBOX_OKCANCEL, this);
		}

		return true;
	}

	switch (dwMsg)
	{
		case UIMSG_ICON_DOWN_FIRST:
		{
			if (m_bUpgradeInProgress)
			{
				SetState(UI_STATE_COMMON_NONE);
				return false;
			}

			if (m_pItemBeingDragged != nullptr)
				CancelIconDrop();

			e_UIWND_DISTRICT eDistrict = UIWND_DISTRICT_UNKNOWN;
			int iOrder                 = -1;
			__IconItemSkill* spItem    = nullptr;

			if (!GetSelectedIconInfo(pSender, &iOrder, &eDistrict, &spItem) || iOrder < 0)
			{
				SetState(UI_STATE_COMMON_NONE);
				return false;
			}

			if (eDistrict == UIWND_DISTRICT_UPGRADE_RESULT_SLOT)
				ResetUpgradeInventory();

			if (eDistrict == UIWND_DISTRICT_UPGRADE_INV)
			{
				if (m_bUpgradeSucceeded)
					ResetUpgradeInventory();

				m_iItemBeingDraggedSourcePos = iOrder;
				m_pItemBeingDragged          = spItem->Clone(this);

				ReduceInvItemStackSize(m_pItemBeingDragged);

				// Set icon region for moving.
				RECT region = GetSampleRect();
				if (m_pItemBeingDragged->pUIIcon != nullptr)
				{
					m_pItemBeingDragged->pUIIcon->SetRegion(region);
					m_pItemBeingDragged->pUIIcon->SetMoveRect(region);
				}
			}
			else // if (eDistrict != UIWND_DISTRICT_UPGRADE_INV)
			{
				SetState(UI_STATE_COMMON_NONE);
				return false;
			}
		}
		break;

		case UIMSG_ICON_UP:
			if (m_pItemBeingDragged == nullptr)
				break;

			if (!ReceiveIconDrop())
			{
				// Restore the icon position to its original place if drop failed.
				CancelIconDrop();
			}

			m_pItemBeingDragged          = nullptr;
			m_iItemBeingDraggedSourcePos = -1;

			SetState(UI_STATE_COMMON_NONE);
			break;

		case UIMSG_ICON_RDOWN_FIRST:
			if (!m_bUpgradeInProgress && !HandleInventoryIconRightClick(pSender))
				return false;
			break;

		default:
			break;
	}

	return true;
}

void CUIItemUpgrade::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);

	if (bVisible)
	{
		CancelIconDrop();
		CopyInventoryItems();
		GoldUpdate();
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	}
	else
	{
		CGameProcedure::s_pUIMgr->ReFocusUI();
	}
}

void CUIItemUpgrade::SetVisibleWithNoSound(bool bVisible, bool bWork, bool bReFocus)
{
	CN3UIBase::SetVisibleWithNoSound(bVisible, bWork, bReFocus);

	if (bVisible)
	{
		SetVisible(true);
	}
	else
	{
		if (m_pUITooltipDlg != nullptr)
			m_pUITooltipDlg->DisplayTooltipsDisable();

		if (GetState() == UI_STATE_ICON_MOVING)
			CancelIconDrop();

		// Reset the item's inventory area.
		ResetUpgradeInventory();
		AnimClose();
	}

	if (bReFocus)
	{
		if (bVisible)
			CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
		else
			CGameProcedure::s_pUIMgr->ReFocusUI();
	}
}

// Loads the UI from file and initializes all required UI components.
bool CUIItemUpgrade::Load(File& file)
{
	if (!CN3UIBase::Load(file))
		return false;

	std::string szID;

	N3_VERIFY_UI_COMPONENT(m_pBtnClose, GetChildByID<CN3UIButton>("btn_close"));
	N3_VERIFY_UI_COMPONENT(m_pBtnOk, GetChildByID<CN3UIButton>("btn_ok"));
	N3_VERIFY_UI_COMPONENT(m_pBtnCancel, GetChildByID<CN3UIButton>("btn_cancel"));
	N3_VERIFY_UI_COMPONENT(m_pAreaUpgrade, GetChildByID<CN3UIArea>("a_upgrade"));
	N3_VERIFY_UI_COMPONENT(m_pAreaResult, GetChildByID<CN3UIArea>("a_result"));
	N3_VERIFY_UI_COMPONENT(m_pImageCover1, GetChildByID<CN3UIImage>("img_cover_01"));
	N3_VERIFY_UI_COMPONENT(m_pImageCover2, GetChildByID<CN3UIImage>("img_cover_02"));
	N3_VERIFY_UI_COMPONENT(m_pStrMyGold, GetChildByID<CN3UIString>("text_gold"));
	// TODO: Implement this UI later
	//N3_VERIFY_UI_COMPONENT(m_pBtnConversation,	GetChildByID<CN3UIButton>("btn_conversation"));

	if (m_pStrMyGold != nullptr)
		m_pStrMyGold->SetString("0");

	if (m_pImageCover1 != nullptr)
		m_pImageCover1->SetVisible(false);

	if (m_pImageCover2 != nullptr)
		m_pImageCover2->SetVisible(false);

	for (int i = 0; i < ANVIL_REQ_MAX; i++)
	{
		szID = fmt::format("a_m_{}", i);
		N3_VERIFY_UI_COMPONENT(m_pSlotArea[i], GetChildByID<CN3UIArea>(szID));
	}

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		szID = fmt::format("a_slot_{}", i);
		N3_VERIFY_UI_COMPONENT(m_pAreaInv[i], GetChildByID<CN3UIArea>(szID));

		szID = fmt::format("s_count_{}", i);
		N3_VERIFY_UI_COMPONENT(m_pStrInv[i], GetChildByID<CN3UIString>(szID));

		if (m_pStrInv[i] != nullptr)
			m_pStrInv[i]->SetVisible(false);
	}

	for (int i = 0; i < FLIPFLOP_MAX_FRAMES; i++)
	{
		szID = fmt::format("img_s_load_{}", i);
		N3_VERIFY_UI_COMPONENT(m_pImgSuccess[i], GetChildByID<CN3UIImage>(szID));

		szID = fmt::format("img_f_load_{}", i);
		N3_VERIFY_UI_COMPONENT(m_pImgFail[i], GetChildByID<CN3UIImage>(szID));

		if (m_pImgFail[i] != nullptr)
			m_pImgFail[i]->SetVisible(false);

		if (m_pImgSuccess[i] != nullptr)
			m_pImgSuccess[i]->SetVisible(false);
	}

	__TABLE_UI_RESRC* pTbl = CGameBase::s_pTbl_UI.Find(CGameBase::s_pPlayer->Nation());
	__ASSERT(pTbl != nullptr, "__TABLE_UI_RESRC: missing nation");
	if (pTbl != nullptr)
	{
		if (m_pUITooltipDlg == nullptr)
			m_pUITooltipDlg = new CUIImageTooltipDlg();

		m_pUITooltipDlg->Init(this);
		m_pUITooltipDlg->LoadFromFile(pTbl->szItemInfo);
		m_pUITooltipDlg->InitPos();
		m_pUITooltipDlg->SetVisible(false);

		if (m_pUIMsgBoxOkCancel == nullptr)
			m_pUIMsgBoxOkCancel = new CUIMsgBoxOkCancel();
		m_pUIMsgBoxOkCancel->Init(this);
		m_pUIMsgBoxOkCancel->LoadFromFile(pTbl->szMsgBoxOkCancel);
		m_pUIMsgBoxOkCancel->SetText(fmt::format_text_resource(IDS_ITEM_UPGRADE_CONFIRM));
		int iX = (m_rcRegion.right + m_rcRegion.left) / 2;
		int iY = (m_rcRegion.bottom + m_rcRegion.top) / 2;
		m_pUIMsgBoxOkCancel->SetPos(iX - (m_pUIMsgBoxOkCancel->GetWidth() / 2), iY - (m_pUIMsgBoxOkCancel->GetHeight() / 2) - 80);
		m_pUIMsgBoxOkCancel->SetVisible(false);
	}

	return true;
}

// Handles key press events, such as closing the UI with ESC.
bool CUIItemUpgrade::OnKeyPress(int iKey)
{
	if (iKey == DIK_ESCAPE)
	{
		Close();
		return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

// Restores the inventory and requirement slots.
void CUIItemUpgrade::ResetUpgradeInventory()
{
	if (m_iUpgradeItemSlotInvPos != -1)
	{
		__IconItemSkill* spItem = m_pMyUpgradeInv[m_iUpgradeItemSlotInvPos];
		CN3UIArea* pArea        = m_pAreaInv[m_iUpgradeItemSlotInvPos];
		if (spItem != nullptr && spItem->pUIIcon != nullptr && pArea != nullptr)
		{
			spItem->pUIIcon->SetRegion(pArea->GetRegion());
			spItem->pUIIcon->SetMoveRect(pArea->GetRegion());
		}

		m_iUpgradeItemSlotInvPos = -1;
	}

	for (int i = 0; i < ANVIL_REQ_MAX; i++)
	{
		int8_t iOrder = m_iRequirementSlotInvPos[i];
		if (iOrder == -1)
			continue;

		__IconItemSkill* spItem = m_pMyUpgradeInv[iOrder];
		if (spItem != nullptr)
		{
			CN3UIArea* pArea = m_pAreaInv[iOrder];
			if (spItem->pUIIcon != nullptr && pArea != nullptr)
			{
				spItem->pUIIcon->SetRegion(pArea->GetRegion());
				spItem->pUIIcon->SetMoveRect(pArea->GetRegion());
			}

			RestoreInvItemStackSize(spItem);

			delete m_pRequirementSlot[i]->pUIIcon;
			m_pRequirementSlot[i]->pUIIcon = nullptr;
			delete m_pRequirementSlot[i];
			m_pRequirementSlot[i] = nullptr;
		}

		m_iRequirementSlotInvPos[i] = -1;
	}

	m_bUpgradeSucceeded  = false;
	m_bUpgradeInProgress = false;
}

// Checks if the given item is allowed to be upgraded (unique or upgrade type).
bool CUIItemUpgrade::IsAllowedUpgradeItem(const __IconItemSkill* spItem) const
{
	if (m_iUpgradeItemSlotInvPos != -1)
		return false;

	if (spItem == nullptr || spItem->pItemBasic == nullptr)
		return false;

	switch (spItem->pItemBasic->byAttachPoint)
	{
		case ITEM_POS_DUAL:
		case ITEM_POS_RIGHTHAND:
		case ITEM_POS_LEFTHAND:
		case ITEM_POS_TWOHANDRIGHT:
		case ITEM_POS_TWOHANDLEFT:
		case ITEM_POS_SHOES:
		case ITEM_POS_GLOVES:
		case ITEM_POS_HEAD:
		case ITEM_POS_LOWER:
		case ITEM_POS_UPPER:
			break;

		default:
			return false;
	}

	e_ItemAttrib eItemAttribute = static_cast<e_ItemAttrib>(spItem->pItemExt->byMagicOrRare);
	return eItemAttribute == ITEM_ATTRIB_UNIQUE || eItemAttribute == ITEM_ATTRIB_UPGRADE;
}

void CUIItemUpgrade::SendToServerUpgradeMsg()
{
	if (m_iUpgradeItemSlotInvPos < 0 || m_iUpgradeItemSlotInvPos >= MAX_ITEM_INVENTORY
		|| m_pMyUpgradeInv[m_iUpgradeItemSlotInvPos] == nullptr)
		return;

	struct ItemPosPair
	{
		int ID     = 0;
		int8_t Pos = -1;
	};

	uint8_t byBuff[512];
	std::array<ItemPosPair, ANVIL_REQ_MAX> reqItems {};
	int iOffset = 0, iTotalSent = 0;

	m_bUpgradeInProgress = true;

	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_ITEM_UPGRADE);
	CAPISocket::MP_AddByte(byBuff, iOffset, ITEM_UPGRADE_PROCESS);
	CAPISocket::MP_AddWord(byBuff, iOffset, m_iNpcID);

	// Add item to upgrade
	CAPISocket::MP_AddDword(byBuff, iOffset, m_pMyUpgradeInv[m_iUpgradeItemSlotInvPos]->GetItemID());
	CAPISocket::MP_AddByte(byBuff, iOffset, m_iUpgradeItemSlotInvPos);

	// Add requirement items
	for (int i = 0; i < ANVIL_REQ_MAX; i++)
	{
		int8_t iOrder = m_iRequirementSlotInvPos[i];
		if (iOrder < 0 || iOrder >= MAX_ITEM_INVENTORY)
			continue;

		reqItems[iTotalSent++] = { .ID = m_pMyUpgradeInv[iOrder]->GetItemID(), .Pos = iOrder };
	}

	std::sort(reqItems.begin(), reqItems.end(), //
		[](const ItemPosPair& lhs, const ItemPosPair& rhs) { return lhs.ID > rhs.ID; });

	for (const ItemPosPair& reqItem : reqItems)
	{
		CAPISocket::MP_AddDword(byBuff, iOffset, reqItem.ID);
		CAPISocket::MP_AddByte(byBuff, iOffset, reqItem.Pos);
	}

	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
}

void CUIItemUpgrade::MsgRecv_ItemUpgrade(Packet& pkt)
{
	CUIInventory* pInven = CGameProcedure::s_pProcMain->m_pUIInventory;
	if (pInven == nullptr)
		return;

	struct ItemPair
	{
		int ID     = 0;
		int8_t Pos = -1;
	};

	ItemPair upgradeItem {};
	std::array<ItemPair, ANVIL_REQ_MAX> reqItems {};

	int8_t result   = pkt.read<int8_t>();

	upgradeItem.ID  = pkt.read<int32_t>();
	upgradeItem.Pos = pkt.read<int8_t>();

	for (int i = 0; i < ANVIL_REQ_MAX; i++)
	{
		reqItems[i].ID  = pkt.read<int32_t>();
		reqItems[i].Pos = pkt.read<int8_t>();
	}

	std::string szMsg;

	CancelIconDrop();

	// When failing (burning) or succeeding, all requirement items are consumed and should be removed.
	if (result == ITEM_UPGRADE_RESULT_FAILED || result == ITEM_UPGRADE_RESULT_SUCCEEDED)
	{
		for (int i = 0; i < ANVIL_REQ_MAX; i++)
		{
			// Remove requirement items first.
			// These are all copies, not references to existing data.
			if (m_pRequirementSlot[i] != nullptr)
			{
				delete m_pRequirementSlot[i]->pUIIcon;
				m_pRequirementSlot[i]->pUIIcon = nullptr;
				delete m_pRequirementSlot[i];
				m_pRequirementSlot[i] = nullptr;
			}

			m_iRequirementSlotInvPos[i] = -1;

			const ItemPair& reqItem     = reqItems[i];

			// Remove a stack from the requirement items in the base inventory.
			int8_t byInvOrder           = reqItem.Pos;
			if (byInvOrder < 0 || byInvOrder >= MAX_ITEM_INVENTORY)
				continue;

			__IconItemSkill* spItemInv = pInven->m_pMyInvWnd[byInvOrder];
			if (spItemInv == nullptr)
				continue;

			if (spItemInv->GetItemID() != reqItem.ID)
			{
				assert(!"Unexpected item");
				continue;
			}

			if (--spItemInv->iCount <= 0)
			{
				delete spItemInv->pUIIcon;
				spItemInv->pUIIcon = nullptr;
				delete spItemInv;
				pInven->m_pMyInvWnd[byInvOrder] = nullptr;
			}
		}
	}

	if (result == ITEM_UPGRADE_RESULT_FAILED)
	{
		if (upgradeItem.Pos < 0 || upgradeItem.Pos >= MAX_ITEM_INVENTORY)
			return;

		// The item burned. We should remove it from the inventory.
		__IconItemSkill* spItemInv = pInven->m_pMyInvWnd[upgradeItem.Pos];
		if (spItemInv != nullptr)
		{
			delete spItemInv->pUIIcon;
			spItemInv->pUIIcon = nullptr;
			delete spItemInv;
			pInven->m_pMyInvWnd[upgradeItem.Pos] = nullptr;
		}

		// Reset the inventory, copying it back from the original.
		// Note that in this case, we don't need the upgrade item to be displayed.
		// This will be reset at this point.
		if (IsVisible())
		{
			CopyInventoryItems();

			m_bUpgradeSucceeded = false;
			m_eAnimationState   = AnimationState::Start;
		}

		szMsg = fmt::format_text_resource(IDS_ITEM_UPGRADE_FAILED);
		CGameProcedure::s_pProcMain->MsgOutput(szMsg, D3DCOLOR_XRGB(255, 0, 255));
	}
	else if (result == ITEM_UPGRADE_RESULT_SUCCEEDED)
	{
		if (upgradeItem.Pos < 0 || upgradeItem.Pos >= MAX_ITEM_INVENTORY)
			return;

		if (IsVisible())
		{
			m_bUpgradeSucceeded = true;

			if (m_bUpgradeInProgress)
				m_eAnimationState = AnimationState::Start;
		}

		szMsg = fmt::format_text_resource(IDS_ITEM_UPGRADE_SUCCEEDED);
		CGameProcedure::s_pProcMain->MsgOutput(szMsg, D3DCOLOR_XRGB(128, 128, 255));

		e_PartPosition ePart = PART_POS_UNKNOWN;
		e_PlugPosition ePlug = PLUG_POS_UNKNOWN;
		e_ItemType eItemType = ITEM_TYPE_UNKNOWN;
		std::string szIconFN;

		__TABLE_ITEM_BASIC* pItemBasic = CGameBase::s_pTbl_Items_Basic.Find(upgradeItem.ID / 1000 * 1000);
		__TABLE_ITEM_EXT* pItemExt     = nullptr;

		if (pItemBasic != nullptr && pItemBasic->byExtIndex >= 0 && pItemBasic->byExtIndex < MAX_ITEM_EXTENSION)
			pItemExt = CGameBase::s_pTbl_Items_Exts[pItemBasic->byExtIndex].Find(upgradeItem.ID % 1000);

		eItemType = CGameBase::MakeResrcFileNameForUPC(
			pItemBasic, pItemExt, nullptr, &szIconFN, ePart, ePlug, CGameBase::s_pPlayer->Race());
		if (eItemType == ITEM_TYPE_UNKNOWN)
			return;

		// Upgrade the item in the inventory.
		__IconItemSkill* spItemInv = pInven->m_pMyInvWnd[upgradeItem.Pos];
		if (spItemInv != nullptr)
		{
			spItemInv->pItemBasic  = pItemBasic;
			spItemInv->pItemExt    = pItemExt;
			spItemInv->iDurability = pItemBasic->siMaxDurability + pItemExt->siMaxDurability;

			if (spItemInv->pUIIcon != nullptr)
				spItemInv->pUIIcon->SetTex(szIconFN);

			CN3UIArea* pArea = pInven->GetChildAreaByiOrder(UI_AREA_TYPE_INV, upgradeItem.Pos);
			if (pArea != nullptr && spItemInv->pUIIcon != nullptr)
			{
				spItemInv->pUIIcon->SetRegion(pArea->GetRegion());
				spItemInv->pUIIcon->SetMoveRect(pArea->GetRegion());
			}
		}

		if (IsVisible())
		{
			// Reset the inventory, copying it back from the original.
			CopyInventoryItems();

			m_iUpgradeItemSlotInvPos = upgradeItem.Pos;

			// Note that in this case, we need the upgrade item to be displayed in the result slot.
			// We should restore it.
			spItemInv                = m_pMyUpgradeInv[upgradeItem.Pos];
			if (spItemInv != nullptr && spItemInv->pUIIcon && m_pAreaResult != nullptr)
			{
				spItemInv->pUIIcon->SetRegion(m_pAreaResult->GetRegion());
				spItemInv->pUIIcon->SetMoveRect(m_pAreaResult->GetRegion());
			}
		}
	}
	else if (result == ITEM_UPGRADE_RESULT_TRADING)
	{
		if (IsVisible())
		{
			m_bUpgradeInProgress = false;
			ResetUpgradeInventory();
		}

		szMsg = fmt::format_text_resource(IDS_ITEM_UPGRADE_CANNOT_PERFORM);
		CGameProcedure::s_pProcMain->MsgOutput(szMsg, D3DCOLOR_XRGB(255, 0, 255));
	}
	else if (result == ITEM_UPGRADE_RESULT_NEED_COINS)
	{
		if (IsVisible())
		{
			m_bUpgradeInProgress = false;
			ResetUpgradeInventory();
		}

		szMsg = fmt::format_text_resource(IDS_ITEM_UPGRADE_NEED_COINS);
		CGameProcedure::s_pProcMain->MsgOutput(szMsg, D3DCOLOR_XRGB(255, 0, 255));
	}
	else if (result == ITEM_UPGRADE_RESULT_NO_MATCH)
	{
		if (IsVisible())
		{
			m_bUpgradeInProgress = false;
			ResetUpgradeInventory();
		}

		szMsg = fmt::format_text_resource(IDS_ITEM_UPGRADE_NO_MATCH);
		CGameProcedure::s_pProcMain->MsgOutput(szMsg, D3DCOLOR_XRGB(255, 0, 255));
	}

	GoldUpdate();
}

void CUIItemUpgrade::UpdateCoverAnimation()
{
	constexpr float COVER_ANIMATION_DURATION = 0.8f;

	if (m_pImageCover1 == nullptr || m_pImageCover2 == nullptr)
		return;

	m_fAnimationTimer += CN3Base::s_fSecPerFrm;

	float t            = m_fAnimationTimer / COVER_ANIMATION_DURATION;

	// Only handle opening (covers move outward and hide)
	float ease         = 1.0f - ((1.0f - t) * (1.0f - t));
	int coverShift     = m_rcCover1Original.bottom - m_rcCover1Original.top;

	// Calculate new Y positions for opening
	int y1             = m_rcCover1Original.top + (int) ((m_rcCover1Original.top - coverShift - m_rcCover1Original.top) * ease);
	int y2             = m_rcCover2Original.top + (int) ((m_rcCover2Original.top + coverShift - m_rcCover2Original.top) * ease);

	// Animation completed - hide covers and reset positions
	if (t >= 1.0f)
	{
		m_eAnimationState = AnimationState::None;

		m_pImageCover1->SetVisible(false);
		m_pImageCover2->SetVisible(false);
		m_pImageCover1->SetRegion(m_rcCover1Original);
		m_pImageCover2->SetRegion(m_rcCover2Original);

		m_bUpgradeInProgress = false;
		return;
	}

	// Update cover regions
	RECT rc1    = m_rcCover1Original;
	RECT rc2    = m_rcCover2Original;
	int height1 = rc1.bottom - rc1.top;
	int height2 = rc2.bottom - rc2.top;
	rc1.top     = y1;
	rc1.bottom  = y1 + height1;
	rc2.top     = y2;
	rc2.bottom  = y2 + height2;

	m_pImageCover1->SetRegion(rc1);
	m_pImageCover2->SetRegion(rc2);
}

void CUIItemUpgrade::UpdateFlipFlopAnimation()
{
	constexpr float FLIPFLOP_FRAME_DELAY  = 0.1f;

	m_fAnimationTimer                    += CN3Base::s_fSecPerFrm;

	if (m_fAnimationTimer >= FLIPFLOP_FRAME_DELAY)
	{
		m_fAnimationTimer -= FLIPFLOP_FRAME_DELAY;
		++m_iCurrentFrame;

		if (m_iCurrentFrame >= FLIPFLOP_MAX_FRAMES)
		{
			HideAllAnimationFrames();

			m_eAnimationState = AnimationState::Result;
			m_fAnimationTimer = 0.0f;
		}
		else
		{
			FlipFlopAnim();
		}
	}
}

void CUIItemUpgrade::HideAllAnimationFrames()
{
	for (int i = 0; i < FLIPFLOP_MAX_FRAMES; i++)
	{
		// Hide all img_s_load_X frames
		if (m_pImgSuccess[i] != nullptr)
			m_pImgSuccess[i]->SetVisible(false);

		// Hide all img_f_load_X frames
		if (m_pImgFail[i] != nullptr)
			m_pImgFail[i]->SetVisible(false);
	}
}

bool CUIItemUpgrade::IsValidRequirementItem(const __IconItemSkill* pSrc) const
{
	if (m_bUpgradeInProgress)
		return false;

	if (pSrc->pItemBasic->dwEffectID2 != ITEM_EFFECT2_ITEM_UPGRADE_REQ)
		return false;

	bool bHasConsumable = false;
	bool bHasScroll     = false;

	for (int i = 0; i < ANVIL_REQ_MAX; i++)
	{
		int8_t iOrder = m_iRequirementSlotInvPos[i];
		if (iOrder < 0)
			continue;

		if (m_pMyUpgradeInv[iOrder]->pItemBasic->byClass == ITEM_CLASS_CONSUMABLE)
			bHasConsumable = true;
		else
			bHasScroll = true;
	}

	if (bHasConsumable && pSrc->pItemBasic->byClass == ITEM_CLASS_CONSUMABLE)
		return false;

	if (bHasScroll && pSrc->pItemBasic->byClass != ITEM_CLASS_CONSUMABLE)
		return false;

	if (bHasConsumable && bHasScroll)
		return false;

	return true;
}

void CUIItemUpgrade::FlipFlopAnim()
{
	if (m_bUpgradeSucceeded)
	{
		// Hide previous frame
		if (m_iCurrentFrame > 0 && m_pImgSuccess[m_iCurrentFrame - 1] != nullptr)
			m_pImgSuccess[m_iCurrentFrame - 1]->SetVisible(false);

		// Show current frame
		if (m_pImgSuccess[m_iCurrentFrame] != nullptr)
		{
			m_pImgSuccess[m_iCurrentFrame]->SetVisible(true);
			m_pImgSuccess[m_iCurrentFrame]->SetParent(this); // Re-order for rendering
		}
	}
	else
	{
		// Hide previous frame
		if (m_iCurrentFrame > 0 && m_pImgFail[m_iCurrentFrame - 1] != nullptr)
			m_pImgFail[m_iCurrentFrame - 1]->SetVisible(false);

		// Show current frame
		if (m_pImgFail[m_iCurrentFrame] != nullptr)
		{
			m_pImgFail[m_iCurrentFrame]->SetVisible(true);
			m_pImgFail[m_iCurrentFrame]->SetParent(this); // Re-order for rendering
		}
	}
}

void CUIItemUpgrade::AnimClose()
{
	if (m_eAnimationState != AnimationState::None)
	{
		m_eAnimationState = AnimationState::None;
		m_fAnimationTimer = 0.0f;
		m_iCurrentFrame   = 0;

		if (m_pImageCover1 != nullptr)
			m_pImageCover1->SetVisible(false);

		if (m_pImageCover2 != nullptr)
			m_pImageCover2->SetVisible(false);
	}

	m_bUpgradeInProgress = false;
	HideAllAnimationFrames();
}

void CUIItemUpgrade::StartUpgradeAnim()
{
	if (!m_bUpgradeInProgress)
		return;

	if (m_pImageCover1 == nullptr || m_pImageCover2 == nullptr)
		return;

	m_fAnimationTimer = 0.0f;
	m_iCurrentFrame   = 0;

	// Make covers visible during animation
	m_pImageCover1->SetVisible(true);
	m_pImageCover2->SetVisible(true);
	m_pImageCover1->SetParent(this); // Re-order for rendering
	m_pImageCover2->SetParent(this);

	// save original positions
	m_rcCover1Original = m_pImageCover1->GetRegion();
	m_rcCover2Original = m_pImageCover2->GetRegion();
	m_eAnimationState  = AnimationState::FlipFlop;
}

bool CUIItemUpgrade::HandleInventoryIconRightClick(CN3UIBase* pUIIcon)
{
	if (pUIIcon == nullptr || !pUIIcon->IsVisible())
		return false;

	e_UIWND_DISTRICT eDistrict = UIWND_DISTRICT_UNKNOWN;
	int iSrcOrder              = -1;
	__IconItemSkill* spItem    = nullptr;
	if (!GetSelectedIconInfo(pUIIcon, &iSrcOrder, &eDistrict, &spItem) || iSrcOrder < 0)
		return false;

	if (spItem == nullptr || spItem->pUIIcon == nullptr)
		return false;

	if (eDistrict == UIWND_DISTRICT_UPGRADE_INV)
	{
		if (m_bUpgradeSucceeded)
			ResetUpgradeInventory();

		if (m_iUpgradeItemSlotInvPos == -1 && IsAllowedUpgradeItem(spItem))
		{
			if (m_pAreaUpgrade != nullptr)
			{
				spItem->pUIIcon->SetRegion(m_pAreaUpgrade->GetRegion());
				spItem->pUIIcon->SetMoveRect(m_pAreaUpgrade->GetRegion());
			}

			m_iUpgradeItemSlotInvPos = iSrcOrder;
			return true;
		}

		if (IsValidRequirementItem(spItem))
		{
			__IconItemSkill* spItemNew = spItem->Clone(this);

			for (int i = 0; i < ANVIL_REQ_MAX; i++)
			{
				if (m_iRequirementSlotInvPos[i] != -1)
					continue;

				// If stackable, reduce stack size in inventory
				ReduceInvItemStackSize(m_pMyUpgradeInv[iSrcOrder]);
				SetRequirementItemSlot(spItemNew, iSrcOrder, i);
				return true;
			}

			if (spItemNew != nullptr)
			{
				delete spItemNew->pUIIcon;
				spItemNew->pUIIcon = nullptr;
				delete spItemNew;
			}
		}
	}
	else if (eDistrict == UIWND_DISTRICT_UPGRADE_RESULT_SLOT)
	{
		// Move upgrade result to inv
		ResetUpgradeInventory();
		return true;
	}

	return false;
}

void CUIItemUpgrade::SetRequirementItemSlot(__IconItemSkill* spItem, int iSrcOrder, int iDstOrder)
{
	if (spItem == nullptr)
		return;

	CN3UIArea* pArea = m_pSlotArea[iDstOrder];

	if (spItem->pUIIcon != nullptr && pArea != nullptr)
	{
		spItem->pUIIcon->SetRegion(pArea->GetRegion());
		spItem->pUIIcon->SetMoveRect(pArea->GetRegion());
	}

	m_iRequirementSlotInvPos[iDstOrder] = iSrcOrder;
	m_pRequirementSlot[iDstOrder]       = spItem;
}

void CUIItemUpgrade::CallBackProc(int iID, uint32_t dwFlag)
{
	if (iID == CHILD_UI_MSGBOX_OKCANCEL)
	{
		if (dwFlag == CUIMsgBoxOkCancel::CALLBACK_OK)
		{
			SendToServerUpgradeMsg();
		}
		else if (dwFlag == CUIMsgBoxOkCancel::CALLBACK_CANCEL)
		{
			if (m_pUIMsgBoxOkCancel != nullptr)
				m_pUIMsgBoxOkCancel->SetVisible(false);
		}
	}
}

void CUIItemUpgrade::ReduceInvItemStackSize(__IconItemSkill* spItem)
{
	// If stackable, reduce stack size in inventory
	if (!spItem->IsStackable())
		return;

	if (--spItem->iCount == 0 && spItem->pUIIcon != nullptr)
		spItem->pUIIcon->SetVisibleWithNoSound(false);
}

void CUIItemUpgrade::RestoreInvItemStackSize(__IconItemSkill* spItem)
{
	// If stackable, restore stack size in inventory
	if (!spItem->IsStackable())
		return;

	if (++spItem->iCount == 1 && spItem->pUIIcon != nullptr)
		spItem->pUIIcon->SetVisibleWithNoSound(true);
}

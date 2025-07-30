// UIItemUpgrade.cpp: implementation of the CUIItemUpgrade class.
//Author : Monzantys(Mervan)
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "LogWriter.h"

#include "PacketDef.h"
#include "LocalInput.h"
#include "APISocket.h"
#include "GameProcMain.h"
#include "UIItemUpgrade.h"
#include "UIImageTooltipDlg.h"

#include "UIInventory.h"
#include "UIManager.h"
#include "PlayerMySelf.h"

#include "CountableItemEditDlg.h"

#include "UIHotKeyDlg.h"
#include "UISkillTreeDlg.h"

#include "N3UIString.h"
#include "N3UIEdit.h"
#include "N3SndObj.h"



#include "resource.h"
// Item ID Constants
static const uint32_t MIN_UPGRADE_ITEM_ID = 379000000;
static const uint32_t MAX_UPGRADE_ITEM_ID = 379257000;
static const uint32_t TRINA_ITEM_ID = 700002000;

// Animation Constants
static const float GUILLOTINE_ANIMATION_DURATION = 0.8f;
static const float FLIPFLOP_FRAME_DELAY = 0.1f;
static const int FLIPFLOP_MAX_FRAMES = 20;
static const float UV_ASPECT_RATIO = 45.0f / 64.0f;


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUIItemUpgrade::CUIItemUpgrade()
{
	int i;
	for (i = 0; i < MAX_ITEM_UPGRADE_SLOT; i++)
	{
		m_pMyUpgradeSLot[i] = NULL;
		m_iUpgradeSlotInvPos[i] = -1;
	}



	for (i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		m_pMyUpgradeInv[i] = NULL;
		m_pBackupUpgradeInv[i] = NULL;
	}

	m_pUpgradeItemSlot = NULL;
	m_iUpgradeSlotInvPos[9] = -1; //UpgradeItemSlot pozition
	m_pUpgradeResultSlot = NULL;
	m_pUITooltipDlg = NULL;
	m_pStrMyGold = NULL;


	this->SetVisible(false);
}

CUIItemUpgrade::~CUIItemUpgrade()
{
	Release();
}

void CUIItemUpgrade::Release()
{


	for (int i = 0; i < MAX_ITEM_UPGRADE_SLOT; i++)
	{
		DeleteIconItemSkill(m_pMyUpgradeSLot[i]);
		m_iUpgradeSlotInvPos[i] = -1;
	}

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		DeleteIconItemSkill(m_pMyUpgradeInv[i]);
	}
	DeleteIconItemSkill(m_pUpgradeItemSlot);
	m_iUpgradeSlotInvPos[9] = -1; //UpgradeItemSlot pozition
	DeleteIconItemSkill(m_pUpgradeResultSlot);

	if (m_pUITooltipDlg)
	{
		delete m_pUITooltipDlg;
		m_pUITooltipDlg = nullptr;
	}
	m_pStrMyGold = nullptr;

	CN3UIBase::Release();
}

void CUIItemUpgrade::Tick()
{

	if (m_bGuillotineActive && m_pImageCover1 && m_pImageCover2)
	{
		UpdateGuillotineAnimation();
	}


	if (m_bFlipFlopActive)
	{
		UpdateFlipFlopAnimation();
	}

	CN3UIBase::Tick();
}

void CUIItemUpgrade::Render()
{
	if (!m_bVisible) return;

	int i;

	POINT ptCur = CGameProcedure::s_pLocalInput->MouseGetPos();
	m_pUITooltipDlg->DisplayTooltipsDisable();

	bool bTooltipRender = false;
	__IconItemSkill* spItem = NULL;

	for (UIListReverseItor itor = m_Children.rbegin(); m_Children.rend() != itor; ++itor)
	{
		CN3UIBase* pChild = (*itor);
		if ((GetState() == UI_STATE_ICON_MOVING) && (pChild->UIType() == UI_TYPE_ICON) && (CN3UIWndBase::m_sSelectedIconInfo.pItemSelect) &&
			((CN3UIIcon*) pChild == CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pUIIcon))	continue;
		pChild->Render();

		if ((GetState() == UI_STATE_COMMON_NONE) &&
			(pChild->UIType() == UI_TYPE_ICON) && (pChild->GetStyle() & UISTYLE_ICON_HIGHLIGHT))
		{
			bTooltipRender = true;
			spItem = GetHighlightIconItem((CN3UIIcon*) pChild);
		}
	}

	// Display the count for items that should show a count.
	for (i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (m_pMyUpgradeInv[i] && ((m_pMyUpgradeInv[i]->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE) ||
			(m_pMyUpgradeInv[i]->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL)))
		{
			CN3UIString* pStr = GetChildStringByiOrderWithPrefix(i, "s_count_");
			if (pStr)
			{
				if ((GetState() == UI_STATE_ICON_MOVING) && (m_pMyUpgradeInv[i] == CN3UIWndBase::m_sSelectedIconInfo.pItemSelect))
				{
					pStr->SetVisible(false);
				}
				else
				{
					if (m_pMyUpgradeInv[i]->pUIIcon->IsVisible())
					{
						pStr->SetVisible(true);
						pStr->SetStringAsInt(m_pMyUpgradeInv[i]->iCount);
						pStr->Render();
					}
					else
					{
						pStr->SetVisible(false);
					}
				}
			}
		}
		else
		{
			CN3UIString* pStr = GetChildStringByiOrderWithPrefix(i, "s_count_");
			if (pStr)
				pStr->SetVisible(false);
		}
	}

	if ((GetState() == UI_STATE_ICON_MOVING) && (CN3UIWndBase::m_sSelectedIconInfo.pItemSelect))
		CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pUIIcon->Render();

	if (spItem)
	{
		m_pUITooltipDlg->DisplayTooltipsEnable(ptCur.x, ptCur.y, spItem, false, false);
	}

}

void CUIItemUpgrade::InitIconWnd(e_UIWND eWnd)
{
	__TABLE_UI_RESRC* pTbl = CGameBase::s_pTbl_UI.Find(CGameBase::s_pPlayer->m_InfoBase.eNation);

	m_pUITooltipDlg = new CUIImageTooltipDlg();
	m_pUITooltipDlg->Init(this);
	m_pUITooltipDlg->LoadFromFile(pTbl->szItemInfo);
	m_pUITooltipDlg->InitPos();
	m_pUITooltipDlg->SetVisible(FALSE);

	CN3UIWndBase::InitIconWnd(eWnd);

	m_pStrMyGold = (CN3UIString*) GetChildByID("text_gold"); __ASSERT(m_pStrMyGold, "NULL UI Component!!");
	if (m_pStrMyGold) m_pStrMyGold->SetString("0");

}


__IconItemSkill* CUIItemUpgrade::GetHighlightIconItem(CN3UIIcon* pUIIcon)
{
	int i;
	for (i = 0; i < MAX_ITEM_UPGRADE_SLOT; i++)
	{
		if ((m_pMyUpgradeSLot[i] != NULL) && (m_pMyUpgradeSLot[i]->pUIIcon == pUIIcon))
			return m_pMyUpgradeSLot[i];
	}

	for (i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if ((m_pMyUpgradeInv[i] != NULL) && (m_pMyUpgradeInv[i]->pUIIcon == pUIIcon))
			return m_pMyUpgradeInv[i];
	}
	if (m_pUpgradeItemSlot && m_pUpgradeItemSlot->pUIIcon == pUIIcon)
		return m_pUpgradeItemSlot;
	if (m_pUpgradeResultSlot && m_pUpgradeResultSlot->pUIIcon == pUIIcon)
		return m_pUpgradeResultSlot;

	return NULL;
}

void CUIItemUpgrade::Open()
{
	SetVisible(true);
	ItemMoveFromInvToThis();

	if (m_pStrMyGold)
	{
		__InfoPlayerMySelf* pInfoExt = &(CGameBase::s_pPlayer->m_InfoExt);
		m_pStrMyGold->SetStringAsInt(pInfoExt->iGold);
	}


}

void CUIItemUpgrade::GoldUpdate()
{
	if (m_pStrMyGold)
	{
		__InfoPlayerMySelf* pInfoExt = &(CGameBase::s_pPlayer->m_InfoExt);
		m_pStrMyGold->SetStringAsInt(pInfoExt->iGold);
	}
}

void CUIItemUpgrade::ItemMoveFromInvToThis()
{
	CUIInventory* pInven = CGameProcedure::s_pProcMain->m_pUIInventory;
	if (!pInven) return;


	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (pInven->m_pMyInvWnd[i])
		{
			__IconItemSkill* spItem = pInven->m_pMyInvWnd[i];
			spItem->pUIIcon->SetParent(this);

			pInven->m_pMyInvWnd[i] = NULL;
			CN3UIArea* pArea;

			pArea = GetChildAreaByiOrderWithPrefix(UI_AREA_TYPE_INV, i, "a_slot_");
			if (pArea)
			{
				spItem->pUIIcon->SetRegion(pArea->GetRegion());
				spItem->pUIIcon->SetMoveRect(pArea->GetRegion());
			}

			m_pMyUpgradeInv[i] = spItem;
		}
		// Backup the inventory state for restoration if needed.
		if (m_pMyUpgradeInv[i])
		{
			m_pBackupUpgradeInv[i] = new __IconItemSkill(*m_pMyUpgradeInv[i]);
		}
	}
}

void CUIItemUpgrade::Close()
{
	if (IsVisible())
		SetVisible(false);

	RestoreInventoryFromBackup();


	if (GetState() == UI_STATE_ICON_MOVING)
		IconRestore();
	SetState(UI_STATE_COMMON_NONE);
	CN3UIWndBase::AllHighLightIconFree();

	ItemMoveFromThisToInv();

	if (CGameProcedure::s_pProcMain->m_pUISkillTreeDlg) CGameProcedure::s_pProcMain->m_pUISkillTreeDlg->UpdateDisableCheck();
	if (CGameProcedure::s_pProcMain->m_pUIHotKeyDlg) CGameProcedure::s_pProcMain->m_pUIHotKeyDlg->UpdateDisableCheck();
}

void CUIItemUpgrade::ItemMoveFromThisToInv()
{
	CUIInventory* pInven = CGameProcedure::s_pProcMain->m_pUIInventory;
	if (!pInven) return;

	int i;
	for (i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (m_pMyUpgradeInv[i])
		{
			__IconItemSkill* spItem = m_pMyUpgradeInv[i];
			spItem->pUIIcon->SetParent(pInven);

			m_pMyUpgradeInv[i] = NULL;

			CN3UIArea* pArea;

			pArea = pInven->GetChildAreaByiOrder(UI_AREA_TYPE_INV, i);
			if (pArea)
			{
				spItem->pUIIcon->SetRegion(pArea->GetRegion());
				spItem->pUIIcon->SetMoveRect(pArea->GetRegion());
			}

			pInven->m_pMyInvWnd[i] = spItem;
		}
	}
}




bool CUIItemUpgrade::ReceiveIconDrop(__IconItemSkill* spItem, POINT ptCur)
{
	// Temp Define 
#define FAIL_RETURN {	\
		CN3UIWndBase::AllHighLightIconFree();	\
		SetState(UI_STATE_COMMON_NONE);	\
		return false;	\
	}

	CN3UIArea* pArea;
	e_UIWND_DISTRICT eUIWnd = UIWND_DISTRICT_UNKNOWN;
	if (!m_bVisible) return false;

	//  Check if the selected window is correct and the drop is valid.
	if (CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWnd != m_eUIWnd)
		FAIL_RETURN
		if ((CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWndDistrict != UIWND_DISTRICT_UPGRADE_SLOT) &&
			(CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWndDistrict != UIWND_DISTRICT_UPGRADE_INV))
			FAIL_RETURN

	// Find which slot or area the item is being dropped onto.
			int iDestiOrder = -1; bool bFound = false;
	for (int i = 0; i < MAX_ITEM_UPGRADE_SLOT; i++)
	{
		pArea = CN3UIWndBase::GetChildAreaByiOrderWithPrefix(UI_AREA_TYPE_SLOT, i, "a_m_");
		if ((pArea) && (pArea->IsIn(ptCur.x, ptCur.y)))
		{
			bFound = true;
			eUIWnd = UIWND_DISTRICT_UPGRADE_SLOT;
			iDestiOrder = i;
			break;
		}
	}

	// Handle dropping item into the main upgrade area (a_upgrade)	
	if (HandleUpgradeAreaDrop(spItem, ptCur))
	{
		CN3UIWndBase::AllHighLightIconFree();
		SetState(UI_STATE_COMMON_NONE);
		return true;
	}




	if (spItem != CN3UIWndBase::m_sSelectedIconInfo.pItemSelect)
		CN3UIWndBase::m_sSelectedIconInfo.pItemSelect = spItem;


	CN3UIWndBase::m_sRecoveryJobInfo.pItemSource = CN3UIWndBase::m_sSelectedIconInfo.pItemSelect;
	CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.UIWnd = CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWnd;
	CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.UIWndDistrict = CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWndDistrict;
	CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder = CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder;
	CN3UIWndBase::m_sRecoveryJobInfo.pItemTarget = NULL;

	CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.UIWnd = UIWND_UPGRADE;
	CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.UIWndDistrict = eUIWnd;


	switch (CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWndDistrict)
	{

		case UIWND_DISTRICT_UPGRADE_INV:
			if (eUIWnd == UIWND_DISTRICT_UPGRADE_SLOT)
			{

				if (iDestiOrder != -1 && m_pMyUpgradeSLot[iDestiOrder] == nullptr)
				{
					int iSourceOrder = GetItemiOrder(spItem, UIWND_DISTRICT_UPGRADE_INV);
					if (iSourceOrder != -1)
					{
						__IconItemSkill* pSrc = m_pMyUpgradeInv[iSourceOrder];
						if (!IsUpgradeScrollorTrina(pSrc->pItemBasic->dwID))
							FAIL_RETURN

						// If  item with the same dwID is already in the slot, do not add it again.
							bool bAlreadyInSlot = false;
						for (int k = 0; k < MAX_ITEM_UPGRADE_SLOT; ++k)
						{
							if (m_pMyUpgradeSLot[k])
							{
								uint32_t id = m_pMyUpgradeSLot[k]->pItemBasic->dwID;
								// If the 2nd trina is trying to be added
								if (id == TRINA_ITEM_ID && pSrc->pItemBasic->dwID == TRINA_ITEM_ID)
								{
									bAlreadyInSlot = true;
									break;
								}
								// If the 2nd Upgrade Scroll is trying to be added
								if (id >= MIN_UPGRADE_ITEM_ID && id <= MAX_UPGRADE_ITEM_ID &&
									pSrc->pItemBasic->dwID >= MIN_UPGRADE_ITEM_ID && pSrc->pItemBasic->dwID <= MAX_UPGRADE_ITEM_ID)
								{
									bAlreadyInSlot = true;
									break;
								}
								// If there is an upgrade scroll in the slot, only trina can be added.
								if ((id >= MIN_UPGRADE_ITEM_ID && id <= MAX_UPGRADE_ITEM_ID) && pSrc->pItemBasic->dwID != TRINA_ITEM_ID)
								{
									bAlreadyInSlot = true;
									break;
								}
								// If there is TRINA in the slot, only scroll can be added.
								if (id == TRINA_ITEM_ID && !(pSrc->pItemBasic->dwID >= MIN_UPGRADE_ITEM_ID && pSrc->pItemBasic->dwID <= MAX_UPGRADE_ITEM_ID))
								{
									bAlreadyInSlot = true;
									break;
								}
							}
						}
						if (bAlreadyInSlot)
							FAIL_RETURN

						// If it is countable, only 1 piece should be carried
							if (pSrc->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE ||
								pSrc->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL)
							{
								if (pSrc->iCount > 1)
								{
									// Create a new icon, put it in 1 slot, reduce the number in the inventory
									__IconItemSkill* pNew = CreateIconFromSource(pSrc, 1);

									CN3UIArea* pSlotArea = CN3UIWndBase::GetChildAreaByiOrderWithPrefix(UI_AREA_TYPE_SLOT, iDestiOrder, "a_m_");
									SetupIconArea(pNew, pSlotArea);

									m_pMyUpgradeSLot[iDestiOrder] = pNew;
									pSrc->iCount -= 1;
								}
								else
								{
									// If the last one, move directly
									m_pMyUpgradeSLot[iDestiOrder] = pSrc;
									m_pMyUpgradeInv[iSourceOrder] = nullptr;

									CN3UIArea* pSlotArea = CN3UIWndBase::GetChildAreaByiOrderWithPrefix(UI_AREA_TYPE_SLOT, iDestiOrder, "a_m_");
									if (pSlotArea)
									{
										pSrc->pUIIcon->SetRegion(pSlotArea->GetRegion());
										pSrc->pUIIcon->SetMoveRect(pSlotArea->GetRegion());
									}
								}
							}
							else
							{
								// if is not countable item, just move it
								m_pMyUpgradeSLot[iDestiOrder] = pSrc;
								m_pMyUpgradeInv[iSourceOrder] = nullptr;

								CN3UIArea* pSlotArea = CN3UIWndBase::GetChildAreaByiOrderWithPrefix(UI_AREA_TYPE_SLOT, iDestiOrder, "a_m_");
								if (pSlotArea)
								{
									pSrc->pUIIcon->SetRegion(pSlotArea->GetRegion());
									pSrc->pUIIcon->SetMoveRect(pSlotArea->GetRegion());
								}
							}
						m_iUpgradeSlotInvPos[iDestiOrder] = iSourceOrder;
					}
				}
				FAIL_RETURN
			}
			break;
	}

	CN3UIWndBase::AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);

	return false;
}


void CUIItemUpgrade::CancelIconDrop(__IconItemSkill* spItem)
{
	CN3UIWndBase::AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);
}

void CUIItemUpgrade::AcceptIconDrop(__IconItemSkill* spItem)
{
	CN3UIWndBase::AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);
}

// Restores the icon's position to its original inventory area.
void CUIItemUpgrade::IconRestore()
{
	CN3UIArea* pArea;

	if (CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWndDistrict == UIWND_DISTRICT_UPGRADE_INV)
	{
		if (m_pMyUpgradeInv[CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder] != NULL)
		{
			pArea = CN3UIWndBase::GetChildAreaByiOrderWithPrefix(UI_AREA_TYPE_INV, CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder, "a_slot_");
			if (pArea)
			{
				m_pMyUpgradeInv[CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder]->pUIIcon->SetRegion(pArea->GetRegion());
				m_pMyUpgradeInv[CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder]->pUIIcon->SetMoveRect(pArea->GetRegion());
			}
		}
	}
}

uint32_t CUIItemUpgrade::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (!m_bVisible) return dwRet;
	if (CN3UIWndBase::m_sRecoveryJobInfo.m_bWaitFromServer)
	{
		dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);  return dwRet;
	}

	if ((GetState() == UI_STATE_ICON_MOVING) &&
		(CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWnd == UIWND_UPGRADE))
	{
		CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pUIIcon->SetRegion(GetSampleRect());
		CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pUIIcon->SetMoveRect(GetSampleRect());
	}

	return CN3UIWndBase::MouseProc(dwFlags, ptCur, ptOld);
}

// Returns the index of the given item in the specified window district.
int CUIItemUpgrade::GetItemiOrder(__IconItemSkill* spItem, e_UIWND_DISTRICT eWndDist)
{
	int iReturn = -1;
	int i;

	switch (eWndDist)
	{
		case UIWND_DISTRICT_UPGRADE_SLOT:
			for (i = 0; i < MAX_ITEM_UPGRADE_SLOT; i++)
			{
				if ((m_pMyUpgradeSLot[i] != NULL) && (m_pMyUpgradeSLot[i] == spItem))
					return i;
			}
			break;

		case UIWND_DISTRICT_UPGRADE_INV:
			for (i = 0; i < MAX_ITEM_INVENTORY; i++)
			{
				if ((m_pMyUpgradeInv[i] != NULL) && (m_pMyUpgradeInv[i] == spItem))
					return i;
			}
			break;
	}

	return iReturn;
}

// Returns a rectangle centered at the mouse position, used for moving icons.
RECT CUIItemUpgrade::GetSampleRect()
{
	RECT rect;
	CN3UIArea* pArea;
	POINT ptCur = CGameProcedure::s_pLocalInput->MouseGetPos();
	pArea = CN3UIWndBase::GetChildAreaByiOrderWithPrefix(UI_AREA_TYPE_INV, 0, "a_slot_");
	rect = pArea->GetRegion();
	float fWidth = (float) (rect.right - rect.left);
	float fHeight = (float) (rect.bottom - rect.top);
	fWidth *= 0.5f; fHeight *= 0.5f;
	rect.left = ptCur.x - (int) fWidth;  rect.right = ptCur.x + (int) fWidth;
	rect.top = ptCur.y - (int) fHeight; rect.bottom = ptCur.y + (int) fHeight;
	return rect;
}

// Determines which window district (slot or inventory) the given item belongs to.
e_UIWND_DISTRICT CUIItemUpgrade::GetWndDistrict(__IconItemSkill* spItem)
{
	for (int i = 0; i < MAX_ITEM_UPGRADE_SLOT; i++)
	{
		if ((m_pMyUpgradeSLot[i] != NULL) && (m_pMyUpgradeSLot[i] == spItem))
			return UIWND_DISTRICT_UPGRADE_SLOT;
	}

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if ((m_pMyUpgradeInv[i] != NULL) && (m_pMyUpgradeInv[i] == spItem))
			return UIWND_DISTRICT_UPGRADE_INV;
	}
	return UIWND_DISTRICT_UNKNOWN;
}

// Handles UI messages such as button clicks and icon events.
bool CUIItemUpgrade::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	// Temp Define
#define FAIL_CODE {		\
				SetState(UI_STATE_COMMON_NONE);	\
				return false;	\
			}

	if (NULL == pSender) return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtnClose)
			Close();
		else if (pSender == m_pBtnCancel)
			RestoreInventoryFromBackup();
		else if (pSender == m_pBtnOk)
		{
			SendToServerUpgradeMsg();

		}
	}


	__IconItemSkill* spItem = NULL;
	e_UIWND_DISTRICT eUIWnd;
	int iOrder;

	uint32_t dwBitMask = 0x000f0000;

	switch (dwMsg & dwBitMask)
	{
		case UIMSG_ICON_DOWN_FIRST:
			CN3UIWndBase::AllHighLightIconFree();

			// Get the item being interacted with.
			spItem = GetHighlightIconItem((CN3UIIcon*) pSender);

			// Save Select Info..
			CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWnd = UIWND_UPGRADE;
			eUIWnd = GetWndDistrict(spItem);
			if (eUIWnd == UIWND_DISTRICT_UPGRADE_SLOT)
				FAIL_CODE
				if (eUIWnd == UIWND_DISTRICT_UNKNOWN)	FAIL_CODE
					CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWndDistrict = eUIWnd;
			iOrder = GetItemiOrder(spItem, eUIWnd);
			if (iOrder == -1)	FAIL_CODE
				CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder = iOrder;
			CN3UIWndBase::m_sSelectedIconInfo.pItemSelect = spItem;
			// Set icon region for moving.
			((CN3UIIcon*) pSender)->SetRegion(GetSampleRect());
			((CN3UIIcon*) pSender)->SetMoveRect(GetSampleRect());
			// Play item sound.
			if (spItem) PlayItemSound(spItem->pItemBasic);
			break;

		case UIMSG_ICON_UP:
			if (!CGameProcedure::s_pUIMgr->BroadcastIconDropMsg(CN3UIWndBase::m_sSelectedIconInfo.pItemSelect))

				// Restore the icon position to its original place if drop failed.
				IconRestore();

			break;

		case UIMSG_ICON_DOWN:
			if (GetState() == UI_STATE_ICON_MOVING)
			{
				CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pUIIcon->SetRegion(GetSampleRect());
				CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pUIIcon->SetMoveRect(GetSampleRect());
			}
			break;
	}

	return true;
}


void CUIItemUpgrade::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);
	if (bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
	{
		CGameProcedure::s_pUIMgr->ReFocusUI();//this_ui
		RestoreInventoryFromBackup();


	}
}

void CUIItemUpgrade::SetVisibleWithNoSound(bool bVisible, bool bWork, bool bReFocus)
{
	CN3UIBase::SetVisibleWithNoSound(bVisible, bWork, bReFocus);

	if (bWork && !bVisible)
	{

		if (GetState() == UI_STATE_ICON_MOVING)
			IconRestore();
		SetState(UI_STATE_COMMON_NONE);
		CN3UIWndBase::AllHighLightIconFree();


		//Move the items from this window's inventory area to the inventory area of this inventory window.

		RestoreInventoryFromBackup();

		ItemMoveFromThisToInv();

		if (m_pUITooltipDlg) m_pUITooltipDlg->DisplayTooltipsDisable();
	}
}

// Loads the UI from file and initializes all required UI components.
bool CUIItemUpgrade::Load(HANDLE hFile)
{
	if (CN3UIBase::Load(hFile) == false) return false;

	m_pBtnClose = (CN3UIButton*) (this->GetChildByID("btn_close"));		__ASSERT(m_pBtnClose, "NULL UI Component!!");
	m_pBtnOk = (CN3UIButton*) (this->GetChildByID("btn_ok"));	__ASSERT(m_pBtnOk, "NULL UI Component!!");
	m_pBtnCancel = (CN3UIButton*) (this->GetChildByID("btn_cancel"));	__ASSERT(m_pBtnCancel, "NULL UI Component!!");
	m_pBtnConversation = (CN3UIButton*) (this->GetChildByID("btn_conversation"));	__ASSERT(m_pBtnConversation, "NULL UI Component!!");
	m_pAreaUpgrade = (CN3UIArea*) (this->GetChildByID("a_upgrade"));				__ASSERT(m_pAreaUpgrade, "NULL UI Component!!");
	m_pAreaResult = (CN3UIArea*) (this->GetChildByID("a_result"));				__ASSERT(m_pAreaResult, "NULL UI Component!!");
	m_pImageCover1 = (CN3UIImage*) (this->GetChildByID("img_cover_01"));			__ASSERT(m_pImageCover1, "NULL UI Component!!");
	m_pImageCover2 = (CN3UIImage*) (this->GetChildByID("img_cover_02"));			__ASSERT(m_pImageCover2, "NULL UI Component!!");

	m_pImageCover1->SetVisible(false);
	m_pImageCover2->SetVisible(false);
	
	// Store original regions and positions for consistent animation reference
	m_rcCover1Original = m_pImageCover1->GetRegion();
	m_rcCover2Original = m_pImageCover2->GetRegion();
	m_iCover1OriginalX = m_rcCover1Original.left;
	m_iCover1OriginalY = m_rcCover1Original.top;
	m_iCover2OriginalX = m_rcCover2Original.left;
	m_iCover2OriginalY = m_rcCover2Original.top;
	
	for (int i = 0; i < 20; ++i)
	{
		char szID[32];
		sprintf(szID, "img_s_load_%d", i);
		if (CN3UIImage* pImg = (CN3UIImage*) GetChildByID(szID))
			pImg->SetVisible(false);
		sprintf(szID, "img_f_load_%d", i);
		if (CN3UIImage* pImg = (CN3UIImage*) GetChildByID(szID))
			pImg->SetVisible(false);
	}


	return true;
}

// Handles key press events, such as closing the UI with ESC.
bool CUIItemUpgrade::OnKeyPress(int iKey)
{
	switch (iKey)
	{

		case DIK_ESCAPE:
			ReceiveMessage(m_pBtnClose, UIMSG_BUTTON_CLICK);
			if (m_pUITooltipDlg) m_pUITooltipDlg->DisplayTooltipsDisable();
			return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

void CUIItemUpgrade::UpdateBackupUpgradeInv()
{
	// Clear existing backup
	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		m_pBackupUpgradeInv[i] = NULL;
	}

	// Create new backup from current inventory
	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (m_pMyUpgradeInv[i])
		{
			m_pBackupUpgradeInv[i] = new __IconItemSkill(*m_pMyUpgradeInv[i]);
		}
	}
}

// Restores the inventory and slots from the backup, recreating icons as needed.
void CUIItemUpgrade::RestoreInventoryFromBackup()
{

	//Clear existing slots first
	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		DeleteIconItemSkill(m_pMyUpgradeInv[i]);
	}

	DeleteIconItemSkill(m_pUpgradeItemSlot);
	DeleteIconItemSkill(m_pUpgradeResultSlot);


	for (int i = 0; i < MAX_ITEM_UPGRADE_SLOT; i++)
	{
		if (m_pMyUpgradeSLot[i])
			DeleteIconItemSkill(m_pMyUpgradeSLot[i]);

	}

	// Restore items from backup
	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (m_pBackupUpgradeInv[i])
		{
			m_pMyUpgradeInv[i] = new __IconItemSkill(*m_pBackupUpgradeInv[i]);

			//If the icon file name is not empty, create a new UI icon
			if (m_pMyUpgradeInv[i]->pUIIcon)
			{
				CreateUIIconForItem(m_pMyUpgradeInv[i]);

				//Set the UI position based on the inventory area
				CN3UIArea* pArea = GetChildAreaByiOrderWithPrefix(UI_AREA_TYPE_INV, i, "a_slot_");
				SetupIconArea(m_pMyUpgradeInv[i], pArea);
			}
		}
	}
}



// Checks if the given item ID is an upgrade scroll or Trina.
bool CUIItemUpgrade::IsUpgradeScrollorTrina(uint32_t dwID)
{

	return ((dwID >= MIN_UPGRADE_ITEM_ID && dwID <= MAX_UPGRADE_ITEM_ID) || dwID == TRINA_ITEM_ID);

}

// Checks if the given item is allowed to be upgraded (unique or upgrade type).
bool CUIItemUpgrade::IsAllowedUpgradeItem(__IconItemSkill* spItem)
{
	if (spItem && spItem->pItemBasic)
	{

		if (spItem->pItemBasic->byAttachPoint == ITEM_POS_FINGER // Ring
			|| spItem->pItemBasic->byAttachPoint == ITEM_POS_NECK // Necklace
			|| spItem->pItemBasic->byAttachPoint == ITEM_POS_BELT // Belt
			|| spItem->pItemBasic->byAttachPoint == ITEM_POS_EAR) // Earring
		{
			return false;
		}
	}
	e_ItemAttrib eTA = (e_ItemAttrib) (spItem->pItemExt->byMagicOrRare);
	return (eTA == ITEM_ATTRIB_UNIQUE || eTA == ITEM_ATTRIB_UPGRADE || eTA == ITEM_ATTRIB_UNIQUE_REVERSE || eTA == ITEM_ATTRIB_UPGRADE);
}

// Deletes the given icon item skill and its UI icon
void CUIItemUpgrade::DeleteIconItemSkill(__IconItemSkill*& pItem)
{
	if (pItem)
	{
		if (pItem->pUIIcon)
		{
			delete pItem->pUIIcon;
			pItem->pUIIcon = nullptr;
		}
		delete pItem;
		pItem = nullptr;
	}
}

void CUIItemUpgrade::SendToServerUpgradeMsg()
{
	if (!m_pUpgradeItemSlot || !m_pMyUpgradeSLot) return;

	uint8_t byBuff[512];
	int iOffset = 0;

	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_ITEM_UPGRADE);
	CAPISocket::MP_AddByte(byBuff, iOffset, ITEM_UPGRADE_PROCESS);
	CAPISocket::MP_AddByte(byBuff, iOffset, 1);
	uint16_t sNpcID = 1;
	CAPISocket::MP_AddByte(byBuff, iOffset, sNpcID);

	uint32_t nItemID[10] = { 0 };
	uint8_t bPos[10] = { NULL };

	// Which  Upgrade Item
	nItemID[0] = m_pUpgradeItemSlot->pItemBasic->dwID + m_pUpgradeItemSlot->pItemExt->dwID;
	bPos[0] = m_iUpgradeSlotInvPos[9]; // m_pUpgradeItemSlot pozition

	CAPISocket::MP_AddDword(byBuff, iOffset, nItemID[0]);
	CAPISocket::MP_AddByte(byBuff, iOffset, bPos[0]);

	// Add Upgrade Slots
	for (int i = 0; i < MAX_ITEM_UPGRADE_SLOT; ++i)
	{
		bPos[i + 1] = NULL;
		nItemID[i + 1] = 0;
		if (m_pMyUpgradeSLot[i] != nullptr)
		{
			nItemID[i + 1] = m_pMyUpgradeSLot[i]->pItemBasic->dwID +
				m_pMyUpgradeSLot[i]->pItemExt->dwID;
			bPos[i + 1] = m_iUpgradeSlotInvPos[i];
			CAPISocket::MP_AddDword(byBuff, iOffset, nItemID[i + 1]);
			CAPISocket::MP_AddByte(byBuff, iOffset, bPos[i + 1]);
		}


	}

	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
}


void CUIItemUpgrade::MsgRecv_ItemUpgrade(Packet& pkt)
{
	int8_t result = pkt.read<uint8_t>();
	uint32_t nItemID[10];
	uint8_t bPos[10];
	for (int i = 0; i < MAX_ITEM_UPGRADE_SLOT + 1; i++)
	{
		pkt >> nItemID[i];
		pkt >> bPos[i];
	}

	CN3UIWndBase::m_sRecoveryJobInfo.m_bWaitFromServer = false;
	std::string szMsg;
	__TABLE_ITEM_EXT* itemExt = NULL;
	__TABLE_ITEM_BASIC* itemBasic = NULL;
	e_PartPosition ePart;
	e_PlugPosition ePlug;
	e_ItemType eType;
	std::string szIconFN;
	float fUVAspect = (float) 45.0f / (float) 64.0f;


	if (result == 0 || result == 1)
	{
		DoAnimationGuillotine();

		DeleteIconItemSkill(m_pUpgradeItemSlot);
		for (int i = 0; i < MAX_ITEM_UPGRADE_SLOT + 1; i++)
		{
			if (bPos[i])
				DeleteIconItemSkill(m_pMyUpgradeSLot[bPos[i]]);
		}
		UpdateBackupUpgradeInv();
		m_bReceivedResultFromServer = true;
		if (result == 0)
		{
			m_bUpgradeSuccesfull = false;

			CGameBase::GetText(6701, &szMsg);
			CGameProcedure::s_pProcMain->MsgOutput(szMsg, D3DCOLOR_RGBA(255, 60, 60, 255));
		}
		else if (result == 1)
		{
			m_bUpgradeSuccesfull = true;
			CGameBase::GetText(6700, &szMsg);
			CGameProcedure::s_pProcMain->MsgOutput(szMsg, D3DCOLOR_RGBA(255, 255, 0, 255));


			itemBasic = CGameBase::s_pTbl_Items_Basic.Find(nItemID[0] / 1000 * 1000);
			if (itemBasic && itemBasic->byExtIndex >= 0 && itemBasic->byExtIndex <= MAX_ITEM_EXTENSION)
				itemExt = CGameBase::s_pTbl_Items_Exts[itemBasic->byExtIndex].Find(nItemID[0] % 1000);
			else
				itemExt = NULL;
			eType = CGameProcedure::MakeResrcFileNameForUPC(itemBasic, NULL, &szIconFN, ePart, ePlug, CGameBase::s_pPlayer->m_InfoBase.eRace);
			if (ITEM_TYPE_UNKNOWN == eType) return;
			__IconItemSkill* spItemNew;
			spItemNew = new __IconItemSkill;
			spItemNew->pItemBasic = itemBasic;
			spItemNew->pItemExt = itemExt;
			spItemNew->szIconFN = szIconFN;
			spItemNew->iCount = 1;
			CreateUIIconForItem(spItemNew, szIconFN);

			if (m_pAreaResult)
			{
				spItemNew->pUIIcon->SetRegion(m_pAreaResult->GetRegion());
				spItemNew->pUIIcon->SetMoveRect(m_pAreaResult->GetRegion());
				spItemNew->pUIIcon->SetParent(this);
			}

			m_pMyUpgradeInv[bPos[0]] = spItemNew;
			UpdateBackupUpgradeInv();

		}
	}
	else
	{
		//Upgrade Not Match or other
		m_bReceivedResultFromServer = false;
	}

	
	RestoreInventoryFromBackup();
	CN3UIWndBase::AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);


}
void CUIItemUpgrade::DoAnimationGuillotine()
{
	// Reset animation state completely
	m_fGuillotineTimer = 0.0f;
	m_bGuillotineActive = false;
	m_bFlipFlopActive = false;
	m_fFlipFlopTimer = 0.0f;
	m_iCurrentFlipFlopFrame = 0;
	m_bGuillotineClosing = true;

	if (!m_pImageCover1 || !m_pImageCover2) return;
	
	// Capture current correct positions as the true original positions
	m_rcCover1Original = m_pImageCover1->GetRegion();
	m_rcCover2Original = m_pImageCover2->GetRegion();
	m_iCover1OriginalX = m_rcCover1Original.left;
	m_iCover1OriginalY = m_rcCover1Original.top;
	m_iCover2OriginalX = m_rcCover2Original.left;
	m_iCover2OriginalY = m_rcCover2Original.top;
	
	// Calculate the height of the covers for displacement
	m_iCoverShift = m_rcCover1Original.bottom - m_rcCover1Original.top;
	
	// Create starting regions - move Y positions, keep X and size same
	RECT rc1Start = m_rcCover1Original;
	RECT rc2Start = m_rcCover2Original;
	rc1Start.top -= m_iCoverShift;
	rc1Start.bottom -= m_iCoverShift;
	rc2Start.top += m_iCoverShift;
	rc2Start.bottom += m_iCoverShift;
	
	// Initialize covers to starting regions
	m_pImageCover1->SetRegion(rc1Start);
	m_pImageCover2->SetRegion(rc2Start);
	m_pImageCover1->SetVisible(false);
	m_pImageCover2->SetVisible(false);
	
	m_bGuillotineActive = true;
}


void CUIItemUpgrade::DoAnimationUpgradeFail()
{

}

void CUIItemUpgrade::UpdateGuillotineAnimation()
{
	m_fGuillotineTimer += CN3Base::s_fSecPerFrm;
	float t = m_fGuillotineTimer / GUILLOTINE_ANIMATION_DURATION;
	
	if (t > 1.0f) t = 1.0f;

	// Make covers visible during animation
	m_pImageCover1->SetVisible(true);
	m_pImageCover2->SetVisible(true);

	// Calculate start and end Y positions
	int cover1StartY = m_iCover1OriginalY - m_iCoverShift;
	int cover2StartY = m_iCover2OriginalY + m_iCoverShift;
	int cover1EndY = m_iCover1OriginalY;
	int cover2EndY = m_iCover2OriginalY;

	int y1, y2;

	if (m_bGuillotineClosing)
	{
		// Closing phase: quadratic ease-in
		float ease = t * t;
		
		// Cover1 moves from start to center
		y1 = cover1StartY + (int)((cover1EndY - cover1StartY) * ease);
		// Cover2 moves from start to center  
		y2 = cover2StartY + (int)((cover2EndY - cover2StartY) * ease);

		// Animation completed - start flipflop
		if (t >= 1.0f)
		{
			m_bFlipFlopActive = true;
			m_fFlipFlopTimer = 0.0f;
			m_iCurrentFlipFlopFrame = 0;
			m_bGuillotineActive = false;
			
			// Ensure covers are at exactly center region before flipflop
			m_pImageCover1->SetRegion(m_rcCover1Original);
			m_pImageCover2->SetRegion(m_rcCover2Original);
			
			FlipFlopAnim();
			return; // Exit early to prevent further position updates
		}
	}
	else
	{
		// Opening phase: quadratic ease-out
		float ease = 1.0f - ((1.0f - t) * (1.0f - t));
		
		// Cover1 moves from center back to start
		y1 = cover1EndY + (int)((cover1StartY - cover1EndY) * ease);
		// Cover2 moves from center back to start
		y2 = cover2EndY + (int)((cover2StartY - cover2EndY) * ease);

		// Animation completed - hide covers and reset positions
		if (t >= 1.0f)
		{
			m_bGuillotineActive = false;
			m_pImageCover1->SetVisible(false);
			m_pImageCover2->SetVisible(false);
			
			// Force reset covers to their exact original regions for next animation
			m_pImageCover1->SetRegion(m_rcCover1Original);
			m_pImageCover2->SetRegion(m_rcCover2Original);
			return; // Exit early to prevent further position updates
		}
	}

	// Create regions with updated Y positions, keeping X and size identical to original
	RECT rc1 = m_rcCover1Original;
	RECT rc2 = m_rcCover2Original;
	
	// Update Y positions while maintaining original width and height
	int height1 = rc1.bottom - rc1.top;
	int height2 = rc2.bottom - rc2.top;
	rc1.top = y1;
	rc1.bottom = y1 + height1;
	rc2.top = y2;
	rc2.bottom = y2 + height2;
	
	// Apply the updated regions
	m_pImageCover1->SetRegion(rc1);
	m_pImageCover2->SetRegion(rc2);
}

void CUIItemUpgrade::UpdateFlipFlopAnimation()
{
	m_fFlipFlopTimer += CN3Base::s_fSecPerFrm;

	if (m_fFlipFlopTimer >= FLIPFLOP_FRAME_DELAY)
	{
		m_fFlipFlopTimer -= FLIPFLOP_FRAME_DELAY;
		m_iCurrentFlipFlopFrame++;

		if (m_iCurrentFlipFlopFrame >= FLIPFLOP_MAX_FRAMES)
		{
			m_bFlipFlopActive = false;
			HideAllAnimationFrames();
			
			// Start guillotine opening animation
			m_bGuillotineClosing = false;
			m_bGuillotineActive = true;
			m_fGuillotineTimer = 0.0f;
		}
		else
		{
			FlipFlopAnim();
		}
	}
}

void CUIItemUpgrade::HideAllAnimationFrames()
{
	for (int i = 0; i < FLIPFLOP_MAX_FRAMES; ++i)
	{
		char szID[32];
		sprintf(szID, m_bUpgradeSuccesfull ? "img_s_load_%d" : "img_f_load_%d", i);
		if (CN3UIImage* pImg = (CN3UIImage*)GetChildByID(szID))
			pImg->SetVisible(false);
	}
}

void CUIItemUpgrade::CreateUIIconForItem(__IconItemSkill* pItem, const std::string& szIconFN)
{
	if (!pItem) return;

	pItem->pUIIcon = new CN3UIIcon;
	pItem->pUIIcon->Init(this);
	
	std::string iconFile = szIconFN.empty() ? pItem->szIconFN : szIconFN;
	pItem->pUIIcon->SetTex(iconFile);
	pItem->pUIIcon->SetUVRect(0, 0, UV_ASPECT_RATIO, UV_ASPECT_RATIO);
	pItem->pUIIcon->SetUIType(UI_TYPE_ICON);
	pItem->pUIIcon->SetStyle(UISTYLE_ICON_ITEM | UISTYLE_ICON_CERTIFICATION_NEED);
	pItem->pUIIcon->SetVisible(true);
}

__IconItemSkill* CUIItemUpgrade::CreateIconFromSource(__IconItemSkill* pSrc, int count)
{
	if (!pSrc) return nullptr;

	__IconItemSkill* pNew = new __IconItemSkill(*pSrc);
	pNew->iCount = count;
	CreateUIIconForItem(pNew);
	return pNew;
}

void CUIItemUpgrade::SetupIconArea(__IconItemSkill* pItem, CN3UIArea* pArea)
{
	if (!pItem || !pItem->pUIIcon || !pArea) return;

	pItem->pUIIcon->SetRegion(pArea->GetRegion());
	pItem->pUIIcon->SetMoveRect(pArea->GetRegion());
}

bool CUIItemUpgrade::HandleUpgradeAreaDrop(__IconItemSkill* spItem, POINT ptCur)
{
	if (!m_pAreaUpgrade || !m_pAreaUpgrade->IsIn(ptCur.x, ptCur.y))
		return false;

	// Only Upgrade and Unique items can be dropped here
	if (!IsAllowedUpgradeItem(spItem))
		return false;

	// Only one item at a time
	if (m_pUpgradeItemSlot != nullptr)
		return false;

	// Move the item to the upgrade slot
	m_pUpgradeItemSlot = spItem;

	// Remove the item from inventory
	for (int i = 0; i < MAX_ITEM_INVENTORY; ++i)
	{
		if (m_pMyUpgradeInv[i] == spItem)
		{
			m_pMyUpgradeInv[i] = nullptr;
			m_iUpgradeSlotInvPos[9] = i; // m_pUpgradeItemSlot position
			break;
		}
	}

	// Update the item's UI position
	if (m_pAreaUpgrade)
	{
		spItem->pUIIcon->SetRegion(m_pAreaUpgrade->GetRegion());
		spItem->pUIIcon->SetMoveRect(m_pAreaUpgrade->GetRegion());
		spItem->pUIIcon->SetParent(this);
	}

	return true;
}

bool CUIItemUpgrade::IsSlotCompatible(__IconItemSkill* pSrc, int iDestiOrder)
{
	if (!IsUpgradeScrollorTrina(pSrc->pItemBasic->dwID))
		return false;

	// Check if item with the same dwID is already in the slot
	for (int k = 0; k < MAX_ITEM_UPGRADE_SLOT; ++k)
	{
		if (m_pMyUpgradeSLot[k])
		{
			uint32_t id = m_pMyUpgradeSLot[k]->pItemBasic->dwID;
			// If the 2nd trina is trying to be added
			if (id == TRINA_ITEM_ID && pSrc->pItemBasic->dwID == TRINA_ITEM_ID)
				return false;
			// If the 2nd Upgrade Scroll is trying to be added
			if (id >= MIN_UPGRADE_ITEM_ID && id <= MAX_UPGRADE_ITEM_ID &&
				pSrc->pItemBasic->dwID >= MIN_UPGRADE_ITEM_ID && pSrc->pItemBasic->dwID <= MAX_UPGRADE_ITEM_ID)
				return false;
			// If there is an upgrade scroll in the slot, only trina can be added
			if ((id >= MIN_UPGRADE_ITEM_ID && id <= MAX_UPGRADE_ITEM_ID) && pSrc->pItemBasic->dwID != TRINA_ITEM_ID)
				return false;
			// If there is TRINA in the slot, only scroll can be added
			if (id == TRINA_ITEM_ID && !(pSrc->pItemBasic->dwID >= MIN_UPGRADE_ITEM_ID && pSrc->pItemBasic->dwID <= MAX_UPGRADE_ITEM_ID))
				return false;
		}
	}
	return true;
}

bool CUIItemUpgrade::HandleSlotDrop(__IconItemSkill* spItem, int iDestiOrder)
{
	if (iDestiOrder == -1 || m_pMyUpgradeSLot[iDestiOrder] != nullptr)
		return false;

	int iSourceOrder = GetItemiOrder(spItem, UIWND_DISTRICT_UPGRADE_INV);
	if (iSourceOrder == -1)
		return false;

	__IconItemSkill* pSrc = m_pMyUpgradeInv[iSourceOrder];
	if (!IsSlotCompatible(pSrc, iDestiOrder))
		return false;

	// Handle countable items
	if (pSrc->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE ||
		pSrc->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL)
	{
		if (pSrc->iCount > 1)
		{
			// Create a new icon, put it in 1 slot, reduce the number in the inventory
			__IconItemSkill* pNew = CreateIconFromSource(pSrc, 1);
			CN3UIArea* pSlotArea = CN3UIWndBase::GetChildAreaByiOrderWithPrefix(UI_AREA_TYPE_SLOT, iDestiOrder, "a_m_");
			SetupIconArea(pNew, pSlotArea);
			m_pMyUpgradeSLot[iDestiOrder] = pNew;
			pSrc->iCount -= 1;
		}
		else
		{
			// If the last one, move directly
			m_pMyUpgradeSLot[iDestiOrder] = pSrc;
			m_pMyUpgradeInv[iSourceOrder] = nullptr;
			CN3UIArea* pSlotArea = CN3UIWndBase::GetChildAreaByiOrderWithPrefix(UI_AREA_TYPE_SLOT, iDestiOrder, "a_m_");
			SetupIconArea(pSrc, pSlotArea);
		}
	}
	else
	{
		// If is not countable item, just move it
		m_pMyUpgradeSLot[iDestiOrder] = pSrc;
		m_pMyUpgradeInv[iSourceOrder] = nullptr;
		CN3UIArea* pSlotArea = CN3UIWndBase::GetChildAreaByiOrderWithPrefix(UI_AREA_TYPE_SLOT, iDestiOrder, "a_m_");
		SetupIconArea(pSrc, pSlotArea);
	}
	m_iUpgradeSlotInvPos[iDestiOrder] = iSourceOrder;
	return true;
}

void CUIItemUpgrade::FlipFlopAnim()
{
	if (!m_bFlipFlopActive) return;

	// Hide before frame
	if (m_iCurrentFlipFlopFrame > 0)
	{
		char szID[32];
		sprintf(szID, m_bUpgradeSuccesfull ? "img_s_load_%d" : "img_f_load_%d", m_iCurrentFlipFlopFrame - 1);
		if (CN3UIImage* pImg = (CN3UIImage*)GetChildByID(szID))
			pImg->SetVisible(false);
	}

	// Show current frame
	char szID[32];
	sprintf(szID, m_bUpgradeSuccesfull ? "img_s_load_%d" : "img_f_load_%d", m_iCurrentFlipFlopFrame);
	if (CN3UIImage* pImg = (CN3UIImage*)GetChildByID(szID))
		pImg->SetVisible(true);
}





// CUIRingUpgrade.cpp: implementation of the CUIRingUpgrade class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "LogWriter.h"

#include "PacketDef.h"
#include "LocalInput.h"
#include "APISocket.h"
#include "GameProcMain.h"
#include "UIRingUpgrade.h"
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
#define MIN_UPGRADE_ITEM_ID 379160000
#define MAX_UPGRADE_ITEM_ID 379164000
#define ACCESSORY_COMPOUND_ID 379159000
#define SHADOW_PIECE			700009000


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUIRingUpgrade::CUIRingUpgrade()
{
	int i;
	for (i = 0; i < MAX_ACCESSORY_COMPOUND_SCROLL_SLOT; i++)
	{
		m_pAccessoryCompoundScrollSlot[i] = NULL;
		m_pAccessoryCompoundScrollSlot[i] = NULL;
	}
	for (i = 0; i < MAX_ACCESSORY_COMPOUND_SLOT; i++)
		m_pMyCompoundSLot[i] = NULL;

	for (i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		m_pMyAccessoryCompoundInv[i] = NULL;
		m_pBackupUpInv[i] = NULL;
	}

	m_pAccessoryCompoundResultSlot = NULL;
	m_pUITooltipDlg = NULL;
	m_pStrMyGold = NULL;


	this->SetVisible(false);
}

CUIRingUpgrade::~CUIRingUpgrade()
{
	Release();
}

void CUIRingUpgrade::Release()
{
	CN3UIBase::Release();


	for (int i = 0; i < MAX_ACCESSORY_COMPOUND_SLOT; i++)
	{
		DeleteIconItemSkill(m_pMyCompoundSLot[i]);
	}

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		DeleteIconItemSkill(m_pMyAccessoryCompoundInv[i]);
	}
	//DeleteIconItemSkill(m_pAccessoryCompoundScrollSlot);
	DeleteIconItemSkill(m_pAccessoryCompoundResultSlot);

	m_pUITooltipDlg = NULL;
	m_pStrMyGold = NULL;
}

void CUIRingUpgrade::Render()
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
		if (pChild->GetID().find("img_cover") && pChild->GetID().find("img_s_load") && pChild->GetID().find("img_f_load"))
		{
			if ((GetState() == UI_STATE_ICON_MOVING) && (pChild->UIType() == UI_TYPE_ICON) && (CN3UIWndBase::m_sSelectedIconInfo.pItemSelect) &&
				((CN3UIIcon*) pChild == CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pUIIcon))	continue;
			pChild->Render();
		}
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
		if (m_pMyAccessoryCompoundInv[i] && ((m_pMyAccessoryCompoundInv[i]->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE) ||
			(m_pMyAccessoryCompoundInv[i]->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL)))
		{
			char szID[32];
			sprintf(szID, "s_count_%d", i);
			CN3UIString* pStr = (CN3UIString*)GetChildByID(szID);
			if (pStr)
			{
				if ((GetState() == UI_STATE_ICON_MOVING) && (m_pMyAccessoryCompoundInv[i] == CN3UIWndBase::m_sSelectedIconInfo.pItemSelect))
				{
					pStr->SetVisible(false);
				}
				else
				{
					if (m_pMyAccessoryCompoundInv[i]->pUIIcon->IsVisible())
					{
						pStr->SetVisible(true);
						pStr->SetStringAsInt(m_pMyAccessoryCompoundInv[i]->iCount);
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
			char szID[32];
			sprintf(szID, "s_count_%d", i);
			CN3UIString* pStr = (CN3UIString*)GetChildByID(szID);
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

void CUIRingUpgrade::InitIconWnd(e_UIWND eWnd)
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


__IconItemSkill* CUIRingUpgrade::GetHighlightIconItem(CN3UIIcon* pUIIcon)
{
	int i;
	for (i = 0; i < MAX_ACCESSORY_COMPOUND_SLOT; i++)
	{
		if ((m_pMyCompoundSLot[i] != NULL) && (m_pMyCompoundSLot[i]->pUIIcon == pUIIcon))
			return m_pMyCompoundSLot[i];
	}

	for (i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if ((m_pMyAccessoryCompoundInv[i] != NULL) && (m_pMyAccessoryCompoundInv[i]->pUIIcon == pUIIcon))
			return m_pMyAccessoryCompoundInv[i];
	}


	//if (m_pAccessoryCompoundScrollSlot && m_pAccessoryCompoundScrollSlot->pUIIcon == pUIIcon)
	//	return m_pAccessoryCompoundScrollSlot;

	if (m_pAccessoryCompoundResultSlot && m_pAccessoryCompoundResultSlot->pUIIcon == pUIIcon)
		return m_pAccessoryCompoundResultSlot;

	return NULL;
}

void CUIRingUpgrade::Open()
{
	SetVisible(true);
	ItemMoveFromInvToThis();

	if (m_pStrMyGold)
	{
		__InfoPlayerMySelf* pInfoExt = &(CGameBase::s_pPlayer->m_InfoExt);
		m_pStrMyGold->SetStringAsInt(pInfoExt->iGold);
	}


}

void CUIRingUpgrade::GoldUpdate()
{
	if (m_pStrMyGold)
	{
		__InfoPlayerMySelf* pInfoExt = &(CGameBase::s_pPlayer->m_InfoExt);
		m_pStrMyGold->SetStringAsInt(pInfoExt->iGold);
	}
}

void CUIRingUpgrade::ItemMoveFromInvToThis()
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

			char szID[32];
			sprintf(szID, "a_slot_%d", i);
			pArea = (CN3UIArea*)GetChildByID(szID);
			if (pArea)
			{
				spItem->pUIIcon->SetRegion(pArea->GetRegion());
				spItem->pUIIcon->SetMoveRect(pArea->GetRegion());
			}

			m_pMyAccessoryCompoundInv[i] = spItem;
		}
		// Backup the inventory state for restoration if needed.
		if (m_pMyAccessoryCompoundInv[i])
		{
			m_pBackupUpInv[i] = new __IconItemSkill(*m_pMyAccessoryCompoundInv[i]);
		}
	}
}

void CUIRingUpgrade::Close()
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

void CUIRingUpgrade::ItemMoveFromThisToInv()
{
	CUIInventory* pInven = CGameProcedure::s_pProcMain->m_pUIInventory;
	if (!pInven) return;

	int i;
	for (i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (m_pMyAccessoryCompoundInv[i])
		{
			__IconItemSkill* spItem = m_pMyAccessoryCompoundInv[i];
			spItem->pUIIcon->SetParent(pInven);

			m_pMyAccessoryCompoundInv[i] = NULL;

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




bool CUIRingUpgrade::ReceiveIconDrop(__IconItemSkill* spItem, POINT ptCur)
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
		if ((CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWndDistrict != UIWND_DISTRICT_ACCESSORY_COMPOUND_SLOT) &&
			(CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWndDistrict != UIWND_DISTRICT_ACCESSORY_COMPOUND_INV))
			FAIL_RETURN

	// Find which slot or area the item is being dropped onto.
	int iDestiOrder = -1; bool bFound = false;
	for (int i = 0; i < MAX_ACCESSORY_COMPOUND_SLOT; i++)
	{
		char szID[32];
		sprintf(szID, "a_upgrade_%d", i);
		pArea = (CN3UIArea*)GetChildByID(szID);
		if ((pArea) && (pArea->IsIn(ptCur.x, ptCur.y)))
		{
			bFound = true;
			eUIWnd = UIWND_DISTRICT_ACCESSORY_COMPOUND_SLOT;
			iDestiOrder = i;
			break;
		}
	}

	for (int i = 0; i < MAX_ACCESSORY_COMPOUND_SCROLL_SLOT; i++)
	{
		char szID[32];
		sprintf(szID, "a_m_%d", i);
		pArea = (CN3UIArea*)GetChildByID(szID);
		if ((pArea) && (pArea->IsIn(ptCur.x, ptCur.y)))
		{
			bFound = true;
			eUIWnd = UIWND_DISTRICT_ACCESSORY_COMPOUND_SCROLL_SLOT;
			iDestiOrder = i;
			break;
		}
	}


	if (spItem != CN3UIWndBase::m_sSelectedIconInfo.pItemSelect)
		CN3UIWndBase::m_sSelectedIconInfo.pItemSelect = spItem;


	CN3UIWndBase::m_sRecoveryJobInfo.pItemSource = CN3UIWndBase::m_sSelectedIconInfo.pItemSelect;
	CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.UIWnd = CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWnd;
	CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.UIWndDistrict = CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWndDistrict;
	CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder = CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder;
	CN3UIWndBase::m_sRecoveryJobInfo.pItemTarget = NULL;

	CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.UIWnd = UIWND_RING_UPGRADE;
	CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.UIWndDistrict = eUIWnd;


	switch (CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWndDistrict)
	{

		case UIWND_DISTRICT_ACCESSORY_COMPOUND_INV:
			if (eUIWnd == UIWND_DISTRICT_ACCESSORY_COMPOUND_SCROLL_SLOT)
			{
				if (!IsAccessoryCompoundScroll(spItem->pItemBasic->dwID))
					FAIL_RETURN
				if (iDestiOrder != -1 && m_pAccessoryCompoundScrollSlot[iDestiOrder] == nullptr)
					{
					// Inventory'den itemi çıkar
					int iSourceOrder = GetItemiOrder(spItem, UIWND_DISTRICT_ACCESSORY_COMPOUND_INV);
					if (iSourceOrder != -1)
					{
						m_pAccessoryCompoundScrollSlot[iDestiOrder] = spItem;
						m_pMyAccessoryCompoundInv[iSourceOrder] = nullptr;
						// UI pozisyonunu güncelle
						char szID[32];
						sprintf(szID, "a_m_%d", iDestiOrder);
						CN3UIArea* pScrollArea = (CN3UIArea*)GetChildByID(szID);
						if (pScrollArea)
						{
							spItem->pUIIcon->SetRegion(pScrollArea->GetRegion());
							spItem->pUIIcon->SetMoveRect(pScrollArea->GetRegion());
							spItem->pUIIcon->SetParent(this);
						}
						CN3UIWndBase::AllHighLightIconFree();
						SetState(UI_STATE_COMMON_NONE);
						return true;
					}
				}
				FAIL_RETURN
			
			}
			else if (eUIWnd == UIWND_DISTRICT_ACCESSORY_COMPOUND_SLOT)
			{
				if (!IsAllowedAccessoryCompoundable(spItem))
					FAIL_RETURN
				if (iDestiOrder != -1 && m_pMyCompoundSLot[iDestiOrder] == nullptr)
				{
					// Inventory'den itemi çıkar
					int iSourceOrder = GetItemiOrder(spItem, UIWND_DISTRICT_ACCESSORY_COMPOUND_INV);
					if (iSourceOrder != -1)
					{
						m_pMyCompoundSLot[iDestiOrder] = spItem;
						m_pMyAccessoryCompoundInv[iSourceOrder] = nullptr;

						// UI pozisyonunu güncelle
						char szID[32];
						sprintf(szID, "a_upgrade_%d", iDestiOrder);
						CN3UIArea* pCompoundArea = (CN3UIArea*)GetChildByID(szID);
						if (pCompoundArea)
						{
							spItem->pUIIcon->SetRegion(pCompoundArea->GetRegion());
							spItem->pUIIcon->SetMoveRect(pCompoundArea->GetRegion());
							spItem->pUIIcon->SetParent(this);
						}

						CN3UIWndBase::AllHighLightIconFree();
						SetState(UI_STATE_COMMON_NONE);
						return true;
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


void CUIRingUpgrade::CancelIconDrop(__IconItemSkill* spItem)
{
	CN3UIWndBase::AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);
}

void CUIRingUpgrade::AcceptIconDrop(__IconItemSkill* spItem)
{
	CN3UIWndBase::AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);
}

// Restores the icon's position to its original inventory area.
void CUIRingUpgrade::IconRestore()
{
	CN3UIArea* pArea;

	if (CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWndDistrict == UIWND_DISTRICT_ACCESSORY_COMPOUND_INV)
	{
		if (m_pMyAccessoryCompoundInv[CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder] != NULL)
		{
			char szID[32];
			sprintf(szID, "a_slot_%d", CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder);
			pArea = (CN3UIArea*)GetChildByID(szID);
			if (pArea)
			{
				m_pMyAccessoryCompoundInv[CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder]->pUIIcon->SetRegion(pArea->GetRegion());
				m_pMyAccessoryCompoundInv[CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder]->pUIIcon->SetMoveRect(pArea->GetRegion());
			}
		}
	}
}

uint32_t CUIRingUpgrade::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (!m_bVisible) return dwRet;
	if (CN3UIWndBase::m_sRecoveryJobInfo.m_bWaitFromServer)
	{
		dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);  return dwRet;
	}

	if ((GetState() == UI_STATE_ICON_MOVING) &&
		(CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWnd == UIWND_RING_UPGRADE))
	{
		CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pUIIcon->SetRegion(GetSampleRect());
		CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pUIIcon->SetMoveRect(GetSampleRect());
	}

	return CN3UIWndBase::MouseProc(dwFlags, ptCur, ptOld);
}

// Returns the index of the given item in the specified window district.
int CUIRingUpgrade::GetItemiOrder(__IconItemSkill* spItem, e_UIWND_DISTRICT eWndDist)
{
	int iReturn = -1;
	int i;

	switch (eWndDist)
	{
		case UIWND_DISTRICT_ACCESSORY_COMPOUND_SLOT:
			for (i = 0; i < MAX_ACCESSORY_COMPOUND_SLOT; i++)
			{
				if ((m_pMyCompoundSLot[i] != NULL) && (m_pMyCompoundSLot[i] == spItem))
					return i;
			}
			break;
		case UIWND_DISTRICT_ACCESSORY_COMPOUND_SCROLL_SLOT:
			for (i = 0; i < MAX_ACCESSORY_COMPOUND_SCROLL_SLOT; i++)
			{
				if ((m_pAccessoryCompoundScrollSlot[i] != NULL) && (m_pAccessoryCompoundScrollSlot[i] == spItem))
					return i;
			}

		case UIWND_DISTRICT_ACCESSORY_COMPOUND_INV:
			for (i = 0; i < MAX_ITEM_INVENTORY; i++)
			{
				if ((m_pMyAccessoryCompoundInv[i] != NULL) && (m_pMyAccessoryCompoundInv[i] == spItem))
					return i;
			}
			break;
	}

	return iReturn;
}

// Returns a rectangle centered at the mouse position, used for moving icons.
RECT CUIRingUpgrade::GetSampleRect()
{
	RECT rect;
	CN3UIArea* pArea;
	POINT ptCur = CGameProcedure::s_pLocalInput->MouseGetPos();
	pArea = (CN3UIArea*)GetChildByID("a_slot_0");
	rect = pArea->GetRegion();
	float fWidth = (float) (rect.right - rect.left);
	float fHeight = (float) (rect.bottom - rect.top);
	fWidth *= 0.5f; fHeight *= 0.5f;
	rect.left = ptCur.x - (int) fWidth;  rect.right = ptCur.x + (int) fWidth;
	rect.top = ptCur.y - (int) fHeight; rect.bottom = ptCur.y + (int) fHeight;
	return rect;
}

// Determines which window district (slot or inventory) the given item belongs to.
e_UIWND_DISTRICT CUIRingUpgrade::GetWndDistrict(__IconItemSkill* spItem)
{
	for (int i = 0; i < MAX_ACCESSORY_COMPOUND_SLOT; i++)
	{
		if ((m_pMyCompoundSLot[i] != NULL) && (m_pMyCompoundSLot[i] == spItem))
			return UIWND_DISTRICT_ACCESSORY_COMPOUND_SLOT;
	}

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if ((m_pMyAccessoryCompoundInv[i] != NULL) && (m_pMyAccessoryCompoundInv[i] == spItem))
			return UIWND_DISTRICT_ACCESSORY_COMPOUND_INV;
	}
	return UIWND_DISTRICT_UNKNOWN;
}

// Handles UI messages such as button clicks and icon events.
bool CUIRingUpgrade::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
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
			CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWnd = UIWND_RING_UPGRADE;
			eUIWnd = GetWndDistrict(spItem);
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


void CUIRingUpgrade::SetVisible(bool bVisible)
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

void CUIRingUpgrade::SetVisibleWithNoSound(bool bVisible, bool bWork, bool bReFocus)
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
bool CUIRingUpgrade::Load(HANDLE hFile)
{
	if (CN3UIBase::Load(hFile) == false) return false;

	m_pBtnClose = (CN3UIButton*) (this->GetChildByID("btn_close"));		__ASSERT(m_pBtnClose, "NULL UI Component!!");
	m_pBtnOk = (CN3UIButton*) (this->GetChildByID("btn_ok"));	__ASSERT(m_pBtnOk, "NULL UI Component!!");
	m_pBtnCancel = (CN3UIButton*) (this->GetChildByID("btn_cancel"));	__ASSERT(m_pBtnCancel, "NULL UI Component!!");
	m_pBtnConversation = (CN3UIButton*) (this->GetChildByID("btn_conversation"));	__ASSERT(m_pBtnConversation, "NULL UI Component!!");



	return true;
}

// Handles key press events, such as closing the UI with ESC.
bool CUIRingUpgrade::OnKeyPress(int iKey)
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

// Restores the inventory and slots from the backup, recreating icons as needed.
void CUIRingUpgrade::RestoreInventoryFromBackup()
{

	//Clear existing slots first
	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		DeleteIconItemSkill(m_pMyAccessoryCompoundInv[i]);
	}


	for (int i = 0; i < MAX_ACCESSORY_COMPOUND_SLOT; i++)
	{
		DeleteIconItemSkill(m_pMyCompoundSLot[i]);

	}

	for (int i = 0; i < MAX_ACCESSORY_COMPOUND_SCROLL_SLOT; i++)
	{
		DeleteIconItemSkill(m_pAccessoryCompoundScrollSlot[i]);

	}

	// Restore items from backup
	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (m_pBackupUpInv[i])
		{
			m_pMyAccessoryCompoundInv[i] = new __IconItemSkill(*m_pBackupUpInv[i]);

			//If the icon file name is not empty, create a new UI icon
			if (!m_pMyAccessoryCompoundInv[i]->szIconFN.empty())
			{
				m_pMyAccessoryCompoundInv[i]->pUIIcon = new CN3UIIcon;
				m_pMyAccessoryCompoundInv[i]->pUIIcon->Init(this);
				m_pMyAccessoryCompoundInv[i]->pUIIcon->SetTex(m_pMyAccessoryCompoundInv[i]->szIconFN);
				float fUVAspect = 45.0f / 64.0f;
				m_pMyAccessoryCompoundInv[i]->pUIIcon->SetUVRect(0, 0, fUVAspect, fUVAspect);
				m_pMyAccessoryCompoundInv[i]->pUIIcon->SetUIType(UI_TYPE_ICON);
				m_pMyAccessoryCompoundInv[i]->pUIIcon->SetStyle(UISTYLE_ICON_ITEM | UISTYLE_ICON_CERTIFICATION_NEED);
				m_pMyAccessoryCompoundInv[i]->pUIIcon->SetVisible(true);


				//Set the UI position based on the inventory area
				char szID[32];
				sprintf(szID, "a_slot_%d", i);
				CN3UIArea* pArea = (CN3UIArea*)GetChildByID(szID);
				if (pArea)
				{
					m_pMyAccessoryCompoundInv[i]->pUIIcon->SetRegion(pArea->GetRegion());
					m_pMyAccessoryCompoundInv[i]->pUIIcon->SetMoveRect(pArea->GetRegion());
				}
			}
		}
	}
}

// Checks if the given item ID is an upgrade scroll or Trina.
bool CUIRingUpgrade::IsAccessoryCompoundScroll(uint32_t dwID)
{

	return (dwID == ACCESSORY_COMPOUND_ID);

}

// Checks if the given item is allowed to be upgraded (unique or upgrade type).
bool CUIRingUpgrade::IsAllowedAccessoryCompoundable(__IconItemSkill* spItem)
{
	if (!spItem || !spItem->pItemBasic || !spItem->pItemExt) return false;

	
	int bySlot = spItem->pItemBasic->byAttachPoint;
	bool isAccessory =
		bySlot == ITEM_POS_EAR ||
		bySlot == ITEM_POS_NECK ||
		bySlot == ITEM_POS_BELT ||
		bySlot== ITEM_POS_FINGER ;

	
	e_ItemAttrib eTA = (e_ItemAttrib) (spItem->pItemExt->byMagicOrRare);
	bool isUniqueOrUpgrade = (eTA == ITEM_ATTRIB_UNIQUE || eTA == ITEM_ATTRIB_UPGRADE);

	return isAccessory && isUniqueOrUpgrade;
}

// Deletes the given icon item skill and its UI icon
void CUIRingUpgrade::DeleteIconItemSkill(__IconItemSkill*& pItem)
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

void CUIRingUpgrade::SendToServerUpgradeMsg()
{
	uint8_t byBuff[8];
	int iOffset = 0;
	uint8_t upgradeType = 0;
	int itemID = 0; // Example item ID, should be set based on the item being upgraded
	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_ITEM_UPGRADE);
	CAPISocket::MP_AddDword(byBuff, iOffset, itemID);
	CAPISocket::MP_AddByte(byBuff, iOffset, upgradeType);
	CGameProcedure::s_pSocket->Send(byBuff, iOffset);



}

void CUIRingUpgrade::MsgRecv_RingUpgrade(Packet& pkt)
{

	//uint8_t upgradeType = pkt.ReadByte(); // Get upgrade type from the packet
	//int itemID = pkt.ReadDword(); // Get item ID from the packet		


}




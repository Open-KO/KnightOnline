// UIItemUpgrade.cpp: implementation of the CUIItemUpgrade class.
//
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
		m_pMyUpgradeSLot[i] = NULL;
	for (i = 0; i < MAX_ITEM_INVENTORY; i++)	m_pMyUpgradeInv[i] = NULL;

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
	CN3UIBase::Release();

	int i;
	for (i = 0; i < MAX_ITEM_UPGRADE_SLOT; i++)
	{
		if (m_pMyUpgradeSLot[i] != NULL)
		{
			delete m_pMyUpgradeSLot[i];
			m_pMyUpgradeSLot[i] = NULL;
		}
	}

	for (i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (m_pMyUpgradeInv[i] != NULL)
		{
			delete m_pMyUpgradeInv[i];
			m_pMyUpgradeInv[i] = NULL;
		}
	}
}

void CUIItemUpgrade::Render()
{
	if (!m_bVisible) return;	// 보이지 않으면 자식들을 render하지 않는다.

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

	// 갯수 표시되야 할 아이템 갯수 표시..
	for (i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (m_pMyUpgradeInv[i] && ((m_pMyUpgradeInv[i]->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE) ||
			(m_pMyUpgradeInv[i]->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL)))
		{
			// string 얻기..
			CN3UIString* pStr = GetChildStringByiOrder(i);
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
			// string 얻기..
			CN3UIString* pStr = GetChildStringByiOrder(i);
			if (pStr)
				pStr->SetVisible(false);
		}
	}

	if ((GetState() == UI_STATE_ICON_MOVING) && (CN3UIWndBase::m_sSelectedIconInfo.pItemSelect))
		CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pUIIcon->Render();

	if (bTooltipRender && spItem)
	{
		e_UIWND_DISTRICT eUD = GetWndDistrict(spItem);
		switch (eUD)
		{
			case UIWND_DISTRICT_UPGRADE:
				m_pUITooltipDlg->DisplayTooltipsEnable(ptCur.x, ptCur.y, spItem, true, true);
				break;
			case UIWND_DISTRICT_INVENTORY_INV:
				m_pUITooltipDlg->DisplayTooltipsEnable(ptCur.x, ptCur.y, spItem, true, false);
				break;
		}
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

	return NULL;
}

void CUIItemUpgrade::Open()
{
	int i;

	for (i = 0; i < MAX_ITEM_UPGRADE_SLOT; i++)
	{
		if (m_pMyUpgradeSLot[i] != NULL)
		{
			if (m_pMyUpgradeSLot[i]->pUIIcon)
			{
				RemoveChild(m_pMyUpgradeSLot[i]->pUIIcon);
				m_pMyUpgradeSLot[i]->pUIIcon->Release();
				delete m_pMyUpgradeSLot[i]->pUIIcon;
				m_pMyUpgradeSLot[i]->pUIIcon = NULL;
			}
			delete m_pMyUpgradeSLot[i];
			m_pMyUpgradeSLot[i] = NULL;
		}
	}

	for (i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (m_pMyUpgradeInv[i] != NULL)
		{
			if (m_pMyUpgradeInv[i]->pUIIcon)
			{
				RemoveChild(m_pMyUpgradeInv[i]->pUIIcon);
				m_pMyUpgradeInv[i]->pUIIcon->Release();
				delete m_pMyUpgradeInv[i]->pUIIcon;
				m_pMyUpgradeInv[i]->pUIIcon = NULL;
			}
			delete m_pMyUpgradeInv[i];
			m_pMyUpgradeInv[i] = NULL;
		}
	}

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

	int i;
	for (i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		m_pMyUpgradeInv[i] = NULL;
	}

	for (i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (pInven->m_pMyInvWnd[i])
		{
			__IconItemSkill* spItem = pInven->m_pMyInvWnd[i];
			spItem->pUIIcon->SetParent(this);

			pInven->m_pMyInvWnd[i] = NULL;
			CN3UIArea* pArea;

			pArea = GetChildAreaByiOrder(UI_AREA_TYPE_INV, i);
			if (pArea)
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
	if (IsVisible())
		SetVisible(false);

	if (GetState() == UI_STATE_ICON_MOVING)
		IconRestore();
	SetState(UI_STATE_COMMON_NONE);
	CN3UIWndBase::AllHighLightIconFree();

	// 이 윈도우의 inv 영역의 아이템을 이 인벤토리 윈도우의 inv영역으로 옮긴다..	
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

	// 내가 가졌던 아이콘이 아니면..
	if (CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWnd != m_eUIWnd)
		FAIL_RETURN
		if ((CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWndDistrict != UIWND_DISTRICT_UPGRADE) &&
			(CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWndDistrict != UIWND_DISTRICT_INVENTORY_INV))
			FAIL_RETURN

			// 내가 가졌던 아이콘이면.. npc영역인지 검사한다..
			int i, iDestiOrder = -1; bool bFound = false;
	for (i = 0; i < MAX_ITEM_UPGRADE_SLOT; i++)
	{
		pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_SLOT, i);
		if ((pArea) && (pArea->IsIn(ptCur.x, ptCur.y)))
		{
			bFound = true;
			eUIWnd = UIWND_DISTRICT_UPGRADE;
			break;
		}
	}

	if (!bFound)
	{
		for (i = 0; i < MAX_ITEM_INVENTORY; i++)
		{
			pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_INV, i);
			if ((pArea) && (pArea->IsIn(ptCur.x, ptCur.y)))
			{
				bFound = true;
				eUIWnd = UIWND_DISTRICT_INVENTORY_INV;
				break;
			}
		}
	}

	CN3UIWndBase::m_sRecoveryJobInfo.pItemSource = CN3UIWndBase::m_sSelectedIconInfo.pItemSelect;
	CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.UIWnd = CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWnd;
	CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.UIWndDistrict = CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWndDistrict;
	CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder = CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder;

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

void CUIItemUpgrade::IconRestore()
{
	CN3UIArea* pArea;
	switch (CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWndDistrict)
	{

		case UIWND_DISTRICT_UPGRADE:
			if (m_pMyUpgradeSLot[CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder] != NULL)
			{
				pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_SLOT, CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder);
				if (pArea)
				{
					m_pMyUpgradeSLot[CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder]->pUIIcon->SetRegion(pArea->GetRegion());
					m_pMyUpgradeSLot[CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder]->pUIIcon->SetMoveRect(pArea->GetRegion());
				}
			}
			break;

		case UIWND_DISTRICT_INVENTORY_INV:
			if (m_pMyUpgradeInv[CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder] != NULL)
			{
				pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_INV, CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder);
				if (pArea)
				{
					m_pMyUpgradeInv[CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder]->pUIIcon->SetRegion(pArea->GetRegion());
					m_pMyUpgradeInv[CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder]->pUIIcon->SetMoveRect(pArea->GetRegion());
				}
			}
			break;
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

// 드래그 되는 아이콘 갱신..
	if ((GetState() == UI_STATE_ICON_MOVING) &&
		(CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWnd == UIWND_UPGRADE))
	{
		CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pUIIcon->SetRegion(GetSampleRect());
		CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pUIIcon->SetMoveRect(GetSampleRect());
	}

	return CN3UIWndBase::MouseProc(dwFlags, ptCur, ptOld);
}

int CUIItemUpgrade::GetItemiOrder(__IconItemSkill* spItem, e_UIWND_DISTRICT eWndDist)
{
	int iReturn = -1;
	int i;

	switch (eWndDist)
	{
		case UIWND_DISTRICT_INVENTORY_SLOT:
			for (i = 0; i < MAX_ITEM_UPGRADE_SLOT; i++)
			{
				if ((m_pMyUpgradeSLot[i] != NULL) && (m_pMyUpgradeSLot[i] == spItem))
					return i;
			}
			break;

		case UIWND_DISTRICT_INVENTORY_INV:
			for (i = 0; i < MAX_ITEM_INVENTORY; i++)
			{
				if ((m_pMyUpgradeInv[i] != NULL) && (m_pMyUpgradeInv[i] == spItem))
					return i;
			}
			break;
	}

	return iReturn;
}

RECT CUIItemUpgrade::GetSampleRect()
{
	RECT rect;
	CN3UIArea* pArea;
	POINT ptCur = CGameProcedure::s_pLocalInput->MouseGetPos();
	pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_INV, 0);
	rect = pArea->GetRegion();
	float fWidth = (float) (rect.right - rect.left);
	float fHeight = (float) (rect.bottom - rect.top);
	fWidth *= 0.5f; fHeight *= 0.5f;
	rect.left = ptCur.x - (int) fWidth;  rect.right = ptCur.x + (int) fWidth;
	rect.top = ptCur.y - (int) fHeight; rect.bottom = ptCur.y + (int) fHeight;
	return rect;
}

e_UIWND_DISTRICT CUIItemUpgrade::GetWndDistrict(__IconItemSkill* spItem)
{
	for (int i = 0; i < MAX_ITEM_UPGRADE_SLOT; i++)
	{
		if ((m_pMyUpgradeSLot[i] != NULL) && (m_pMyUpgradeSLot[i] == spItem))
			return UIWND_DISTRICT_UPGRADE;
	}

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if ((m_pMyUpgradeInv[i] != NULL) && (m_pMyUpgradeInv[i] == spItem))
			return UIWND_DISTRICT_INVENTORY_INV;
	}
	return UIWND_DISTRICT_UNKNOWN;
}

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
	}

	__IconItemSkill* spItem = NULL;
	e_UIWND_DISTRICT eUIWnd;
	int iOrder;

	uint32_t dwBitMask = 0x000f0000;

	switch (dwMsg & dwBitMask)
	{
		case UIMSG_ICON_DOWN_FIRST:
			CN3UIWndBase::AllHighLightIconFree();

			// Get Item..
			spItem = GetHighlightIconItem((CN3UIIcon*) pSender);

			// Save Select Info..
			CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWnd = UIWND_UPGRADE;
			eUIWnd = GetWndDistrict(spItem);
			if (eUIWnd == UIWND_DISTRICT_UNKNOWN)	FAIL_CODE
				CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWndDistrict = eUIWnd;
			iOrder = GetItemiOrder(spItem, eUIWnd);
			if (iOrder == -1)	FAIL_CODE
				CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder = iOrder;
			CN3UIWndBase::m_sSelectedIconInfo.pItemSelect = spItem;
			// Do Ops..
			((CN3UIIcon*) pSender)->SetRegion(GetSampleRect());
			((CN3UIIcon*) pSender)->SetMoveRect(GetSampleRect());
			// Sound..
			if (spItem) PlayItemSound(spItem->pItemBasic);
			break;

		case UIMSG_ICON_UP:
			// 아이콘 매니저 윈도우들을 돌아 다니면서 검사..
			if (!CGameProcedure::s_pUIMgr->BroadcastIconDropMsg(CN3UIWndBase::m_sSelectedIconInfo.pItemSelect))
				// 아이콘 위치 원래대로..
				IconRestore();
			// Sound..
			if (CN3UIWndBase::m_sSelectedIconInfo.pItemSelect) PlayItemSound(CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pItemBasic);
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

//this_ui_add_start
void CUIItemUpgrade::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);
	if (bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
	{
		CGameProcedure::s_pUIMgr->ReFocusUI();//this_ui
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

		// 이 윈도우의 inv 영역의 아이템을 이 인벤토리 윈도우의 inv영역으로 옮긴다..	
		ItemMoveFromThisToInv();

		//if (CGameProcedure::s_pProcMain->m_pUISkillTreeDlg) CGameProcedure::s_pProcMain->m_pUISkillTreeDlg->UpdateDisableCheck();
		//if (CGameProcedure::s_pProcMain->m_pUIHotKeyDlg) CGameProcedure::s_pProcMain->m_pUIHotKeyDlg->UpdateDisableCheck();
		if (m_pUITooltipDlg) m_pUITooltipDlg->DisplayTooltipsDisable();
	}
}

bool CUIItemUpgrade::Load(HANDLE hFile)
{
	if (CN3UIBase::Load(hFile) == false) return false;

	m_pBtnClose = (CN3UIButton*) (this->GetChildByID("btn_close"));		__ASSERT(m_pBtnClose, "NULL UI Component!!");
	m_pBtnOk = (CN3UIButton*) (this->GetChildByID("btn_ok"));	__ASSERT(m_pBtnOk, "NULL UI Component!!");
	m_pBtnCancel = (CN3UIButton*) (this->GetChildByID("btn_cancel"));	__ASSERT(m_pBtnCancel, "NULL UI Component!!");
	m_pBtnConversation = (CN3UIButton*) (this->GetChildByID("btn_conversation"));	__ASSERT(m_pBtnConversation, "NULL UI Component!!");

	return true;
}

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


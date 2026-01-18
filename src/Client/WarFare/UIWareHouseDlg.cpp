// UIWareHouseDlg.cpp: implementation of the UIWareHouseDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UIWareHouseDlg.h"
#include "PacketDef.h"
#include "LocalInput.h"
#include "APISocket.h"
#include "GameProcMain.h"
#include "PlayerMySelf.h"
#include "UIImageTooltipDlg.h"
#include "UIInventory.h"
#include "UIManager.h"
#include "CountableItemEditDlg.h"
#include "SubProcPerTrade.h"
#include "UIHotKeyDlg.h"
#include "UISkillTreeDlg.h"
#include "text_resources.h"

#include <N3Base/LogWriter.h>

#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIString.h>
#include <N3Base/N3UIEdit.h>
#include <N3Base/N3SndObj.h>

CUIWareHouseDlg::CUIWareHouseDlg()
{
	m_iCurPage = 0;
	for (int j = 0; j < MAX_ITEM_WARE_PAGE; j++)
	{
		for (int i = 0; i < MAX_ITEM_TRADE; i++)
			m_pMyWare[j][i] = nullptr;
	}

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
		m_pMyWareInv[i] = nullptr;

	m_pUITooltipDlg     = nullptr;
	m_pStrMyGold        = nullptr;
	m_pStrWareGold      = nullptr;

	m_pBtnGold          = nullptr;
	m_pBtnGoldWareHouse = nullptr;
	m_pBtnClose         = nullptr;
	m_pBtnPageUp        = nullptr;
	m_pBtnPageDown      = nullptr;

	m_bSendedItemGold   = false;
	m_iGoldOffsetBackup = 0;

	SetVisible(false);
}

CUIWareHouseDlg::~CUIWareHouseDlg()
{
	CUIWareHouseDlg::Release();
}

void CUIWareHouseDlg::Release()
{
	CN3UIBase::Release();

	for (int j = 0; j < MAX_ITEM_WARE_PAGE; j++)
	{
		for (int i = 0; i < MAX_ITEM_TRADE; i++)
		{
			delete m_pMyWare[j][i];
			m_pMyWare[j][i] = nullptr;
		}
	}

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		delete m_pMyWareInv[i];
		m_pMyWareInv[i] = nullptr;
	}
}

void CUIWareHouseDlg::Render()
{
	// 보이지 않으면 자식들을 render하지 않는다.
	if (!m_bVisible)
		return;

	POINT ptCur = CGameProcedure::s_pLocalInput->MouseGetPos();
	m_pUITooltipDlg->DisplayTooltipsDisable();

	bool bTooltipRender     = false;
	__IconItemSkill* spItem = nullptr;

	for (UIListReverseItor itor = m_Children.rbegin(); m_Children.rend() != itor; ++itor)
	{
		CN3UIBase* pChild = (*itor);
		if (GetState() == UI_STATE_ICON_MOVING && pChild->UIType() == UI_TYPE_ICON && s_sSelectedIconInfo.pItemSelect != nullptr
			&& pChild == s_sSelectedIconInfo.pItemSelect->pUIIcon)
			continue;

		pChild->Render();

		if (GetState() == UI_STATE_COMMON_NONE && pChild->UIType() == UI_TYPE_ICON && (pChild->GetStyle() & UISTYLE_ICON_HIGHLIGHT))
		{
			bTooltipRender = true;
			spItem         = GetHighlightIconItem((CN3UIIcon*) pChild);
		}
	}

	// 갯수 표시되야 할 아이템 갯수 표시..
	for (int i = 0; i < MAX_ITEM_TRADE; i++)
	{
		__IconItemSkill* spItemWare = m_pMyWare[m_iCurPage][i];
		if (spItemWare != nullptr
			&& (spItemWare->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE
				|| spItemWare->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL))
		{
			// string 얻기..
			CN3UIString* pStr = GetChildStringByiOrder(i + 100);
			if (pStr != nullptr)
			{
				if ((GetState() != UI_STATE_ICON_MOVING || spItemWare != s_sSelectedIconInfo.pItemSelect)
					&& spItemWare->pUIIcon->IsVisible())
				{
					pStr->SetVisible(true);
					pStr->SetStringAsInt(spItemWare->iCount);
					pStr->Render();
				}
				else
				{
					pStr->SetVisible(false);
				}
			}
		}
		else
		{
			// string 얻기..
			CN3UIString* pStr = GetChildStringByiOrder(i + 100);
			if (pStr != nullptr)
				pStr->SetVisible(false);
		}
	}

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		__IconItemSkill* spItemInv = m_pMyWareInv[i];
		if (spItemInv != nullptr
			&& (spItemInv->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE
				|| spItemInv->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL))
		{
			// string 얻기..
			CN3UIString* pStr = GetChildStringByiOrder(i);
			if (pStr != nullptr)
			{
				if ((GetState() != UI_STATE_ICON_MOVING || spItemInv != s_sSelectedIconInfo.pItemSelect) && spItemInv->pUIIcon->IsVisible())
				{
					pStr->SetVisible(true);
					pStr->SetStringAsInt(spItemInv->iCount);
					pStr->Render();
				}
				else
				{
					pStr->SetVisible(false);
				}
			}
		}
		else
		{
			// string 얻기..
			CN3UIString* pStr = GetChildStringByiOrder(i);
			if (pStr != nullptr)
				pStr->SetVisible(false);
		}
	}

	if (GetState() == UI_STATE_ICON_MOVING && s_sSelectedIconInfo.pItemSelect != nullptr)
		s_sSelectedIconInfo.pItemSelect->pUIIcon->Render();

	if (bTooltipRender && spItem != nullptr)
		m_pUITooltipDlg->DisplayTooltipsEnable(ptCur.x, ptCur.y, spItem);
}

void CUIWareHouseDlg::InitIconWnd(e_UIWND eWnd)
{
	__TABLE_UI_RESRC* pTbl = CGameBase::s_pTbl_UI.Find(CGameBase::s_pPlayer->m_InfoBase.eNation);

	m_pUITooltipDlg        = new CUIImageTooltipDlg();
	m_pUITooltipDlg->Init(this);
	m_pUITooltipDlg->LoadFromFile(pTbl->szItemInfo);
	m_pUITooltipDlg->InitPos();
	m_pUITooltipDlg->SetVisible(false);

	CN3UIWndBase::InitIconWnd(eWnd);

	N3_VERIFY_UI_COMPONENT(m_pStrWareGold, GetChildByID<CN3UIString>("string_wareitem_name"));
	if (m_pStrWareGold != nullptr)
		m_pStrWareGold->SetString("0");

	N3_VERIFY_UI_COMPONENT(m_pStrMyGold, GetChildByID<CN3UIString>("string_item_name"));
	if (m_pStrMyGold)
		m_pStrMyGold->SetString("0");
}

void CUIWareHouseDlg::InitIconUpdate()
{
	float fUVAspect = (float) 45.0f / (float) 64.0f;

	for (int j = 0; j < MAX_ITEM_WARE_PAGE; j++)
	{
		for (int i = 0; i < MAX_ITEM_TRADE; i++)
		{
			if (m_pMyWare[j][i] != nullptr)
			{
				m_pMyWare[j][i]->pUIIcon = new CN3UIIcon();
				m_pMyWare[j][i]->pUIIcon->Init(this);
				m_pMyWare[j][i]->pUIIcon->SetTex(m_pMyWare[j][i]->szIconFN);
				m_pMyWare[j][i]->pUIIcon->SetUVRect(0, 0, fUVAspect, fUVAspect);
				m_pMyWare[j][i]->pUIIcon->SetUIType(UI_TYPE_ICON);
				m_pMyWare[j][i]->pUIIcon->SetStyle(UISTYLE_ICON_ITEM | UISTYLE_ICON_CERTIFICATION_NEED);

				CN3UIArea* pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_NPC, i);
				if (pArea != nullptr)
				{
					m_pMyWare[j][i]->pUIIcon->SetRegion(pArea->GetRegion());
					m_pMyWare[j][i]->pUIIcon->SetMoveRect(pArea->GetRegion());
				}
			}
		}
	}
}

int CUIWareHouseDlg::GetItemiOrder(__IconItemSkill* spItem, e_UIWND_DISTRICT eWndDist)
{
	switch (eWndDist)
	{
		case UIWND_DISTRICT_TRADE_NPC:
			for (int i = 0; i < MAX_ITEM_TRADE; i++)
			{
				if (m_pMyWare[m_iCurPage][i] != nullptr && m_pMyWare[m_iCurPage][i] == spItem)
					return i;
			}
			break;

		case UIWND_DISTRICT_TRADE_MY:
			for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
			{
				if (m_pMyWareInv[i] != nullptr && m_pMyWareInv[i] == spItem)
					return i;
			}
			break;

		default:
			break;
	}

	return -1;
}

RECT CUIWareHouseDlg::GetSampleRect()
{
	POINT ptCur      = CGameProcedure::s_pLocalInput->MouseGetPos();
	CN3UIArea* pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_NPC, 0);

	RECT rect        = pArea->GetRegion();

	float fWidth     = (float) (rect.right - rect.left) * 0.5f;
	float fHeight    = (float) (rect.bottom - rect.top) * 0.5f;

	rect.left        = ptCur.x - (int) fWidth;
	rect.right       = ptCur.x + (int) fWidth;
	rect.top         = ptCur.y - (int) fHeight;
	rect.bottom      = ptCur.y + (int) fHeight;

	return rect;
}

e_UIWND_DISTRICT CUIWareHouseDlg::GetWndDistrict(__IconItemSkill* spItem)
{
	for (int i = 0; i < MAX_ITEM_TRADE; i++)
	{
		if (m_pMyWare[m_iCurPage][i] != nullptr && m_pMyWare[m_iCurPage][i] == spItem)
			return UIWND_DISTRICT_TRADE_NPC;
	}

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (m_pMyWareInv[i] != nullptr && m_pMyWareInv[i] == spItem)
			return UIWND_DISTRICT_TRADE_MY;
	}
	return UIWND_DISTRICT_UNKNOWN;
}

uint32_t CUIWareHouseDlg::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (!m_bVisible)
		return dwRet;

	if (s_bWaitFromServer)
	{
		// NOLINTNEXTLINE(bugprone-parent-virtual-call)
		dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);
		return dwRet;
	}

	// 드래그 되는 아이콘 갱신..
	if (GetState() == UI_STATE_ICON_MOVING && s_sSelectedIconInfo.UIWndSelect.UIWnd == UIWND_WARE_HOUSE)
	{
		s_sSelectedIconInfo.pItemSelect->pUIIcon->SetRegion(GetSampleRect());
		s_sSelectedIconInfo.pItemSelect->pUIIcon->SetMoveRect(GetSampleRect());
	}

	return CN3UIWndBase::MouseProc(dwFlags, ptCur, ptOld);
}

bool CUIWareHouseDlg::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (pSender == nullptr)
		return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtnGold)
		{
			// 인벤토리만 떠 있을때..
			s_pCountableItemEdit->Open(UIWND_WARE_HOUSE, UIWND_DISTRICT_TRADE_MY, true, true);
			return true;
		}

		if (pSender == m_pBtnGoldWareHouse)
		{
			// 인벤토리만 떠 있을때..
			s_pCountableItemEdit->Open(UIWND_WARE_HOUSE, UIWND_DISTRICT_TRADE_NPC, true, true);
			return true;
		}

		if (pSender == m_pBtnClose)
			LeaveWareHouseState();

		CN3UIString* pStr = nullptr;

		if (pSender == m_pBtnPageUp)
		{
			m_iCurPage--;
			if (m_iCurPage < 0)
				m_iCurPage = 0;

			N3_VERIFY_UI_COMPONENT(pStr, GetChildByID<CN3UIString>("string_page"));
			if (pStr != nullptr)
			{
				std::string pageNo = std::to_string(m_iCurPage + 1);
				pStr->SetString(pageNo);
			}

			for (int j = 0; j < MAX_ITEM_WARE_PAGE; j++)
			{
				if (j == m_iCurPage)
				{
					for (int i = 0; i < MAX_ITEM_TRADE; i++)
					{
						if (m_pMyWare[j][i] != nullptr)
							m_pMyWare[j][i]->pUIIcon->SetVisible(true);
					}
				}
				else
				{
					for (int i = 0; i < MAX_ITEM_TRADE; i++)
					{
						if (m_pMyWare[j][i] != nullptr)
							m_pMyWare[j][i]->pUIIcon->SetVisible(false);
					}
				}
			}
		}

		if (pSender == m_pBtnPageDown)
		{
			m_iCurPage++;
			if (m_iCurPage >= MAX_ITEM_WARE_PAGE)
				m_iCurPage = MAX_ITEM_WARE_PAGE - 1;

			N3_VERIFY_UI_COMPONENT(pStr, GetChildByID<CN3UIString>("string_page"));
			if (pStr != nullptr)
			{
				std::string pageNo = std::to_string(m_iCurPage + 1);
				pStr->SetString(pageNo);
			}

			for (int j = 0; j < MAX_ITEM_WARE_PAGE; j++)
			{
				if (j == m_iCurPage)
				{
					for (int i = 0; i < MAX_ITEM_TRADE; i++)
					{
						if (m_pMyWare[j][i] != nullptr)
							m_pMyWare[j][i]->pUIIcon->SetVisible(true);
					}
				}
				else
				{
					for (int i = 0; i < MAX_ITEM_TRADE; i++)
					{
						if (m_pMyWare[j][i] != nullptr)
							m_pMyWare[j][i]->pUIIcon->SetVisible(false);
					}
				}
			}
		}
	}

	__IconItemSkill* spItem = nullptr;
	e_UIWND_DISTRICT eUIWnd = UIWND_DISTRICT_UNKNOWN;
	int iOrder              = -1;

	uint32_t dwBitMask      = 0x000f0000;

	switch (dwMsg & dwBitMask)
	{
		case UIMSG_ICON_DOWN_FIRST:
			AllHighLightIconFree();

			// Get Item..
			spItem                                = GetHighlightIconItem((CN3UIIcon*) pSender);

			// Save Select Info..
			s_sSelectedIconInfo.UIWndSelect.UIWnd = UIWND_WARE_HOUSE;
			eUIWnd                                = GetWndDistrict(spItem);
			if (eUIWnd == UIWND_DISTRICT_UNKNOWN)
			{
				SetState(UI_STATE_COMMON_NONE);
				return false;
			}

			s_sSelectedIconInfo.UIWndSelect.UIWndDistrict = eUIWnd;
			iOrder                                        = GetItemiOrder(spItem, eUIWnd);
			if (iOrder == -1)
			{
				SetState(UI_STATE_COMMON_NONE);
				return false;
			}

			s_sSelectedIconInfo.UIWndSelect.iOrder = iOrder;
			s_sSelectedIconInfo.pItemSelect        = spItem;

			// Do Ops..
			pSender->SetRegion(GetSampleRect());
			pSender->SetMoveRect(GetSampleRect());

			// Sound..
			if (spItem != nullptr)
				PlayItemSound(spItem->pItemBasic);
			break;

		case UIMSG_ICON_UP:
			// 아이콘 매니저 윈도우들을 돌아 다니면서 검사..
			// 아이콘 위치 원래대로..
			if (!CGameProcedure::s_pUIMgr->BroadcastIconDropMsg(s_sSelectedIconInfo.pItemSelect))
				IconRestore();

			// Sound..
			if (s_sSelectedIconInfo.pItemSelect != nullptr)
				PlayItemSound(s_sSelectedIconInfo.pItemSelect->pItemBasic);
			break;

		case UIMSG_ICON_DOWN:
			if (GetState() == UI_STATE_ICON_MOVING)
			{
				s_sSelectedIconInfo.pItemSelect->pUIIcon->SetRegion(GetSampleRect());
				s_sSelectedIconInfo.pItemSelect->pUIIcon->SetMoveRect(GetSampleRect());
			}
			break;

		default:
			break;
	}

	return true;
}

bool CUIWareHouseDlg::OnMouseWheelEvent(short delta)
{
	if (delta > 0)
		ReceiveMessage(m_pBtnPageUp, UIMSG_BUTTON_CLICK);
	else
		ReceiveMessage(m_pBtnPageDown, UIMSG_BUTTON_CLICK);

	return true;
}

void CUIWareHouseDlg::LeaveWareHouseState()
{
	if (IsVisible())
		SetVisible(false);

	if (GetState() == UI_STATE_ICON_MOVING)
		IconRestore();

	SetState(UI_STATE_COMMON_NONE);
	AllHighLightIconFree();

	// 이 윈도우의 inv 영역의 아이템을 이 인벤토리 윈도우의 inv영역으로 옮긴다..
	ItemMoveFromThisToInv();

	if (CGameProcedure::s_pProcMain->m_pUISkillTreeDlg != nullptr)
		CGameProcedure::s_pProcMain->m_pUISkillTreeDlg->UpdateDisableCheck();

	if (CGameProcedure::s_pProcMain->m_pUIHotKeyDlg != nullptr)
		CGameProcedure::s_pProcMain->m_pUIHotKeyDlg->UpdateDisableCheck();
}

void CUIWareHouseDlg::EnterWareHouseStateStart(int iWareGold)
{
	for (int j = 0; j < MAX_ITEM_WARE_PAGE; j++)
	{
		for (int i = 0; i < MAX_ITEM_TRADE; i++)
		{
			if (m_pMyWare[j][i] != nullptr)
			{
				if (m_pMyWare[j][i]->pUIIcon != nullptr)
				{
					RemoveChild(m_pMyWare[j][i]->pUIIcon);
					m_pMyWare[j][i]->pUIIcon->Release();
					delete m_pMyWare[j][i]->pUIIcon;
					m_pMyWare[j][i]->pUIIcon = nullptr;
				}

				delete m_pMyWare[j][i];
				m_pMyWare[j][i] = nullptr;
			}
		}
	}

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (m_pMyWareInv[i] != nullptr)
		{
			if (m_pMyWareInv[i]->pUIIcon != nullptr)
			{
				RemoveChild(m_pMyWareInv[i]->pUIIcon);
				m_pMyWareInv[i]->pUIIcon->Release();
				delete m_pMyWareInv[i]->pUIIcon;
				m_pMyWareInv[i]->pUIIcon = nullptr;
			}

			delete m_pMyWareInv[i];
			m_pMyWareInv[i] = nullptr;
		}
	}

	if (m_pStrWareGold != nullptr)
		m_pStrWareGold->SetString(CGameBase::FormatNumber(iWareGold));
}

void CUIWareHouseDlg::EnterWareHouseStateEnd()
{
	InitIconUpdate();

	m_iCurPage        = 0;

	CN3UIString* pStr = nullptr;
	N3_VERIFY_UI_COMPONENT(pStr, GetChildByID<CN3UIString>("string_page"));
	if (pStr != nullptr)
	{
		std::string pageNo = std::to_string(m_iCurPage + 1);
		pStr->SetString(pageNo);
	}

	for (int j = 0; j < MAX_ITEM_WARE_PAGE; j++)
	{
		if (j == m_iCurPage)
		{
			for (int i = 0; i < MAX_ITEM_TRADE; i++)
			{
				if (m_pMyWare[j][i] != nullptr)
					m_pMyWare[j][i]->pUIIcon->SetVisible(true);
			}
		}
		else
		{
			for (int i = 0; i < MAX_ITEM_TRADE; i++)
			{
				if (m_pMyWare[j][i] != nullptr)
					m_pMyWare[j][i]->pUIIcon->SetVisible(false);
			}
		}
	}

	ItemMoveFromInvToThis();

	if (m_pStrMyGold != nullptr)
	{
		__InfoPlayerMySelf* pInfoExt = &CGameBase::s_pPlayer->m_InfoExt;
		m_pStrMyGold->SetString(CGameBase::FormatNumber(pInfoExt->iGold));
	}
}

__IconItemSkill* CUIWareHouseDlg::GetHighlightIconItem(CN3UIIcon* pUIIcon)
{
	for (int i = 0; i < MAX_ITEM_TRADE; i++)
	{
		if (m_pMyWare[m_iCurPage][i] != nullptr && m_pMyWare[m_iCurPage][i]->pUIIcon == pUIIcon)
			return m_pMyWare[m_iCurPage][i];
	}

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (m_pMyWareInv[i] != nullptr && m_pMyWareInv[i]->pUIIcon == pUIIcon)
			return m_pMyWareInv[i];
	}

	return nullptr;
}

void CUIWareHouseDlg::IconRestore()
{
	switch (s_sSelectedIconInfo.UIWndSelect.UIWndDistrict)
	{
		case UIWND_DISTRICT_TRADE_NPC:
			if (m_pMyWare[m_iCurPage][s_sSelectedIconInfo.UIWndSelect.iOrder] != nullptr)
			{
				CN3UIArea* pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_NPC, s_sSelectedIconInfo.UIWndSelect.iOrder);
				if (pArea != nullptr)
				{
					m_pMyWare[m_iCurPage][s_sSelectedIconInfo.UIWndSelect.iOrder]->pUIIcon->SetRegion(pArea->GetRegion());
					m_pMyWare[m_iCurPage][s_sSelectedIconInfo.UIWndSelect.iOrder]->pUIIcon->SetMoveRect(pArea->GetRegion());
				}
			}
			break;

		case UIWND_DISTRICT_TRADE_MY:
			if (m_pMyWareInv[s_sSelectedIconInfo.UIWndSelect.iOrder] != nullptr)
			{
				CN3UIArea* pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, s_sSelectedIconInfo.UIWndSelect.iOrder);
				if (pArea != nullptr)
				{
					m_pMyWareInv[s_sSelectedIconInfo.UIWndSelect.iOrder]->pUIIcon->SetRegion(pArea->GetRegion());
					m_pMyWareInv[s_sSelectedIconInfo.UIWndSelect.iOrder]->pUIIcon->SetMoveRect(pArea->GetRegion());
				}
			}
			break;

		default:
			break;
	}
}

bool CUIWareHouseDlg::ReceiveIconDrop(__IconItemSkill* spItem, POINT ptCur)
{
	CN3UIArea* pArea        = nullptr;
	e_UIWND_DISTRICT eUIWnd = UIWND_DISTRICT_UNKNOWN;
	if (!m_bVisible)
		return false;

	// 내가 가졌던 아이콘이 아니면..
	if ((s_sSelectedIconInfo.UIWndSelect.UIWnd != m_eUIWnd)
		|| (s_sSelectedIconInfo.UIWndSelect.UIWndDistrict != UIWND_DISTRICT_TRADE_NPC
			&& s_sSelectedIconInfo.UIWndSelect.UIWndDistrict != UIWND_DISTRICT_TRADE_MY))
	{
		return false;
	}

	// 내가 가졌던 아이콘이면.. npc영역인지 검사한다..
	int iDestiOrder = -1;
	bool bFound     = false;
	for (int i = 0; i < MAX_ITEM_TRADE; i++)
	{
		pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_NPC, i);
		if (pArea != nullptr && pArea->IsIn(ptCur.x, ptCur.y))
		{
			bFound      = true;
			eUIWnd      = UIWND_DISTRICT_TRADE_NPC;
			iDestiOrder = i;
			break;
		}
	}

	if (!bFound)
	{
		for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
		{
			pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, i);
			if (pArea != nullptr && pArea->IsIn(ptCur.x, ptCur.y))
			{
				bFound      = true;
				eUIWnd      = UIWND_DISTRICT_TRADE_MY;
				iDestiOrder = i;
				break;
			}
		}
	}

	if (!bFound)
		return false;

	// 본격적으로 Recovery Info를 활용하기 시작한다..
	// 먼저 WaitFromServer를 On으로 하고.. Select Info를 Recovery Info로 복사.. 이때 Dest는 팰요없다..
	if (spItem != s_sSelectedIconInfo.pItemSelect)
		s_sSelectedIconInfo.pItemSelect = spItem;

	s_bWaitFromServer                                 = true;
	s_sRecoveryJobInfo.pItemSource                    = s_sSelectedIconInfo.pItemSelect;
	s_sRecoveryJobInfo.UIWndSourceStart.UIWnd         = s_sSelectedIconInfo.UIWndSelect.UIWnd;
	s_sRecoveryJobInfo.UIWndSourceStart.UIWndDistrict = s_sSelectedIconInfo.UIWndSelect.UIWndDistrict;
	s_sRecoveryJobInfo.UIWndSourceStart.iOrder        = s_sSelectedIconInfo.UIWndSelect.iOrder;
	s_sRecoveryJobInfo.pItemTarget                    = nullptr;

	s_sRecoveryJobInfo.UIWndSourceEnd.UIWnd           = UIWND_WARE_HOUSE;
	s_sRecoveryJobInfo.UIWndSourceEnd.UIWndDistrict   = eUIWnd;

	switch (s_sSelectedIconInfo.UIWndSelect.UIWndDistrict)
	{
		case UIWND_DISTRICT_TRADE_NPC:
			// 빼는 경우..
			if (eUIWnd == UIWND_DISTRICT_TRADE_MY)
			{
				if (s_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE
					|| s_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL)
				{
					// 활이나 물약등 아이템인 경우..
					// 면저 인벤토리에 해당 아이콘이 있는지 알아본다..
					bFound = false;

					for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
					{
						if (m_pMyWareInv[i] != nullptr
							&& m_pMyWareInv[i]->pItemBasic->dwID == s_sSelectedIconInfo.pItemSelect->pItemBasic->dwID
							&& m_pMyWareInv[i]->pItemExt->dwID == s_sSelectedIconInfo.pItemSelect->pItemExt->dwID)
						{
							bFound                                   = true;
							iDestiOrder                              = i;
							s_sRecoveryJobInfo.UIWndSourceEnd.iOrder = iDestiOrder;
							break;
						}
					}

					// 못찾았으면..
					if (!bFound)
					{
						// 해당 위치에 아이콘이 있으면..
						if (m_pMyWareInv[iDestiOrder] != nullptr)
						{
							// 인벤토리 빈슬롯을 찾아 들어간다..
							for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
							{
								if (m_pMyWareInv[i] == nullptr)
								{
									bFound      = true;
									iDestiOrder = i;
									break;
								}
							}

							// 빈 슬롯을 찾지 못했으면..
							if (!bFound)
							{
								s_bWaitFromServer              = false;
								s_sRecoveryJobInfo.pItemSource = nullptr;
								s_sRecoveryJobInfo.pItemTarget = nullptr;

								return false;
							}
						}
					}

					s_sRecoveryJobInfo.UIWndSourceEnd.iOrder = iDestiOrder;
					s_bWaitFromServer                        = false;

					s_pCountableItemEdit->Open(UIWND_WARE_HOUSE, s_sSelectedIconInfo.UIWndSelect.UIWndDistrict, false);
				}
				else
				{
					// 일반 아이템인 경우..
					// 해당 위치에 아이콘이 있으면..
					if (m_pMyWareInv[iDestiOrder] != nullptr)
					{
						// 인벤토리 빈슬롯을 찾아 들어간다..
						bFound = false;
						for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
						{
							if (m_pMyWareInv[i] == nullptr)
							{
								bFound      = true;
								iDestiOrder = i;
								break;
							}
						}

						// 빈 슬롯을 찾지 못했으면..
						if (!bFound)
						{
							s_bWaitFromServer              = false;
							s_sRecoveryJobInfo.pItemSource = nullptr;
							s_sRecoveryJobInfo.pItemTarget = nullptr;

							return false;
						}
					}

					s_sRecoveryJobInfo.UIWndSourceEnd.iOrder = iDestiOrder;

					// 무게 체크..
					__InfoPlayerMySelf* pInfoExt             = &CGameBase::s_pPlayer->m_InfoExt;
					if ((pInfoExt->iWeight + s_sRecoveryJobInfo.pItemSource->pItemBasic->siWeight) > pInfoExt->iWeightMax)
					{
						std::string szMsg = fmt::format_text_resource(IDS_ITEM_WEIGHT_OVERFLOW);
						CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);

						s_bWaitFromServer              = false;
						s_sRecoveryJobInfo.pItemSource = nullptr;
						s_sRecoveryJobInfo.pItemTarget = nullptr;

						return false;
					}

					SendToServerFromWareMsg(
						s_sRecoveryJobInfo.pItemSource->pItemBasic->dwID + s_sRecoveryJobInfo.pItemSource->pItemExt->dwID, m_iCurPage,
						s_sRecoveryJobInfo.UIWndSourceStart.iOrder, iDestiOrder, s_sRecoveryJobInfo.pItemSource->iCount);

					m_pMyWareInv[iDestiOrder] = m_pMyWare[m_iCurPage][s_sRecoveryJobInfo.UIWndSourceStart.iOrder];
					m_pMyWare[m_iCurPage][s_sRecoveryJobInfo.UIWndSourceStart.iOrder] = nullptr;

					pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, iDestiOrder);
					if (pArea != nullptr)
					{
						m_pMyWareInv[iDestiOrder]->pUIIcon->SetRegion(pArea->GetRegion());
						m_pMyWareInv[iDestiOrder]->pUIIcon->SetMoveRect(pArea->GetRegion());
					}
				}
			}
			else
			{
				// 이동..
				__IconItemSkill *spItemSource = nullptr, *spItemTarget = nullptr;
				spItemSource = s_sRecoveryJobInfo.pItemSource;

				pArea        = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_NPC, iDestiOrder);
				if (pArea != nullptr)
				{
					spItemSource->pUIIcon->SetRegion(pArea->GetRegion());
					spItemSource->pUIIcon->SetMoveRect(pArea->GetRegion());
				}

				s_sRecoveryJobInfo.UIWndSourceEnd.iOrder = iDestiOrder;

				// 창고 내에서 이동..	(모두 일반 아이템으로 취급한다..)
				// 해당 위치에 아이콘이 있으면..
				if (m_pMyWare[m_iCurPage][iDestiOrder] != nullptr)
				{
					s_bWaitFromServer              = false;
					s_sRecoveryJobInfo.pItemSource = nullptr;
					s_sRecoveryJobInfo.pItemTarget = nullptr;

					return false;
				}

				s_sRecoveryJobInfo.pItemTarget                                    = nullptr;

				m_pMyWare[m_iCurPage][iDestiOrder]                                = spItemSource;
				m_pMyWare[m_iCurPage][s_sRecoveryJobInfo.UIWndSourceStart.iOrder] = spItemTarget;

				// 이동 메시지를 보낸다..
				SendToServerWareToWareMsg(s_sRecoveryJobInfo.pItemSource->pItemBasic->dwID + s_sRecoveryJobInfo.pItemSource->pItemExt->dwID,
					m_iCurPage, s_sRecoveryJobInfo.UIWndSourceStart.iOrder, iDestiOrder);
			}
			break;

		case UIWND_DISTRICT_TRADE_MY:
			// 넣는 경우..
			if (eUIWnd == UIWND_DISTRICT_TRADE_NPC)
			{
				if (s_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE
					|| s_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL)
				{
					// 활이나 물약등 아이템인 경우..
					// 면저 Ware에 아이콘이 있는지 알아본다..
					bFound = false;

					// 10개의 폐이지를 다 뒤진다..
					for (int iPage = 0; iPage < MAX_ITEM_WARE_PAGE; iPage++)
					{
						if (bFound)
							break;

						for (int i = 0; i < MAX_ITEM_TRADE; i++)
						{
							if (m_pMyWare[iPage][i] != nullptr
								&& m_pMyWare[iPage][i]->pItemBasic->dwID == s_sSelectedIconInfo.pItemSelect->pItemBasic->dwID
								&& m_pMyWare[iPage][i]->pItemExt->dwID == s_sSelectedIconInfo.pItemSelect->pItemExt->dwID)
							{
								bFound                                   = true;
								iDestiOrder                              = i;
								s_sRecoveryJobInfo.UIWndSourceEnd.iOrder = iDestiOrder;
								s_sRecoveryJobInfo.m_iPage               = iPage;
								break;
							}
						}
					}

					// 못찾았으면..
					if (!bFound)
					{
						// 해당 위치에 아이콘이 있으면..
						if (m_pMyWare[m_iCurPage][iDestiOrder] != nullptr)
						{
							// 빈슬롯을 찾아 들어간다..
							for (int iPage = 0; iPage < MAX_ITEM_WARE_PAGE; iPage++)
							{
								if (bFound)
									break;

								for (int i = 0; i < MAX_ITEM_TRADE; i++)
								{
									if (bFound)
									{
										s_sRecoveryJobInfo.UIWndSourceEnd.iOrder = iDestiOrder;
										s_sRecoveryJobInfo.m_iPage               = iPage;
										break;
									}

									if (m_pMyWare[iPage][i] == nullptr)
									{
										bFound      = true;
										iDestiOrder = i;
									}
								}

								if (!bFound) // 빈 슬롯을 찾지 못했으면..
								{
									s_bWaitFromServer              = false;
									s_sRecoveryJobInfo.pItemSource = nullptr;
									s_sRecoveryJobInfo.pItemTarget = nullptr;

									return false;
								}
							}
						}
						else
						{
							s_sRecoveryJobInfo.UIWndSourceEnd.iOrder = iDestiOrder;
							s_sRecoveryJobInfo.m_iPage               = m_iCurPage;
						}
					}

					s_bWaitFromServer = false;

					s_pCountableItemEdit->Open(UIWND_WARE_HOUSE, s_sSelectedIconInfo.UIWndSelect.UIWndDistrict, false);
					return false;
				}

				// 일반 아이템인 경우..
				if (m_pMyWare[m_iCurPage][iDestiOrder] != nullptr) // 해당 위치에 아이콘이 있으면..
				{
					// 인벤토리 빈슬롯을 찾아 들어간다..
					bFound = false;

					// 10개의 폐이지를 다 뒤진다..
					for (int iPage = 0; iPage < MAX_ITEM_WARE_PAGE; iPage++)
					{
						if (bFound)
							break;

						for (int i = 0; i < MAX_ITEM_TRADE; i++)
						{
							if (bFound)
							{
								s_sRecoveryJobInfo.UIWndSourceEnd.iOrder = iDestiOrder;
								s_sRecoveryJobInfo.m_iPage               = iPage;
								break;
							}

							if (m_pMyWare[iPage][i] == nullptr)
							{
								bFound      = true;
								iDestiOrder = i;
							}
						}
					}

					// 빈 슬롯을 찾지 못했으면..
					if (!bFound)
					{
						s_bWaitFromServer              = false;
						s_sRecoveryJobInfo.pItemSource = nullptr;
						s_sRecoveryJobInfo.pItemTarget = nullptr;

						return false;
					}
				}
				else
				{
					s_sRecoveryJobInfo.UIWndSourceEnd.iOrder = iDestiOrder;
					s_sRecoveryJobInfo.m_iPage               = m_iCurPage;
				}

				SendToServerToWareMsg(s_sRecoveryJobInfo.pItemSource->pItemBasic->dwID + s_sRecoveryJobInfo.pItemSource->pItemExt->dwID,
					s_sRecoveryJobInfo.m_iPage, s_sRecoveryJobInfo.UIWndSourceStart.iOrder, iDestiOrder,
					s_sRecoveryJobInfo.pItemSource->iCount);

				m_pMyWare[s_sRecoveryJobInfo.m_iPage][iDestiOrder]       = m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceStart.iOrder];
				m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceStart.iOrder] = nullptr;

				pArea                                                    = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_NPC, iDestiOrder);
				if (pArea != nullptr)
				{
					m_pMyWare[s_sRecoveryJobInfo.m_iPage][iDestiOrder]->pUIIcon->SetRegion(pArea->GetRegion());
					m_pMyWare[s_sRecoveryJobInfo.m_iPage][iDestiOrder]->pUIIcon->SetMoveRect(pArea->GetRegion());
				}

				if (s_sRecoveryJobInfo.m_iPage != m_iCurPage)
					m_pMyWare[s_sRecoveryJobInfo.m_iPage][iDestiOrder]->pUIIcon->SetVisibleWithNoSound(false);
			}
			else
			{
				// 이동..
				__IconItemSkill *spItemSource = nullptr, *spItemTarget = nullptr;
				spItemSource = s_sRecoveryJobInfo.pItemSource;

				pArea        = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, iDestiOrder);
				if (pArea != nullptr)
				{
					spItemSource->pUIIcon->SetRegion(pArea->GetRegion());
					spItemSource->pUIIcon->SetMoveRect(pArea->GetRegion());
				}

				s_sRecoveryJobInfo.UIWndSourceEnd.iOrder = iDestiOrder;

				// Inv 내에서 이동..	(모두 일반 아이템으로 취급한다..)
				// 해당 위치에 아이콘이 있으면..
				if (m_pMyWareInv[iDestiOrder] != nullptr)
				{
					s_sRecoveryJobInfo.pItemTarget                    = m_pMyWareInv[iDestiOrder];
					s_sRecoveryJobInfo.UIWndTargetStart.UIWnd         = UIWND_WARE_HOUSE;
					s_sRecoveryJobInfo.UIWndTargetStart.UIWndDistrict = UIWND_DISTRICT_TRADE_MY;
					s_sRecoveryJobInfo.UIWndTargetStart.iOrder        = iDestiOrder;
					s_sRecoveryJobInfo.UIWndTargetEnd.UIWnd           = UIWND_WARE_HOUSE;
					s_sRecoveryJobInfo.UIWndTargetEnd.UIWndDistrict   = UIWND_DISTRICT_TRADE_MY;
					s_sRecoveryJobInfo.UIWndTargetEnd.iOrder          = s_sRecoveryJobInfo.UIWndSourceStart.iOrder;

					spItemTarget                                      = s_sRecoveryJobInfo.pItemTarget;

					pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, s_sRecoveryJobInfo.UIWndSourceStart.iOrder);
					if (pArea != nullptr)
					{
						spItemTarget->pUIIcon->SetRegion(pArea->GetRegion());
						spItemTarget->pUIIcon->SetMoveRect(pArea->GetRegion());
					}
				}
				else
				{
					s_sRecoveryJobInfo.pItemTarget = nullptr;
				}

				m_pMyWareInv[iDestiOrder]                                = spItemSource;
				m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceStart.iOrder] = spItemTarget;

				// 이동 메시지를 보낸다..
				SendToServerInvToInvMsg(s_sRecoveryJobInfo.pItemSource->pItemBasic->dwID + s_sRecoveryJobInfo.pItemSource->pItemExt->dwID,
					m_iCurPage, s_sRecoveryJobInfo.UIWndSourceStart.iOrder, iDestiOrder);
			}
			break;

		default:
			break;
	}

	return false;
}

void CUIWareHouseDlg::CancelIconDrop(__IconItemSkill* /*spItem*/)
{
	AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);
}

void CUIWareHouseDlg::AcceptIconDrop(__IconItemSkill* /*spItem*/)
{
	AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);
}

void CUIWareHouseDlg::SendToServerToWareMsg(int iItemID, uint8_t page, uint8_t startpos, uint8_t pos, int iCount)
{
	uint8_t byBuff[32];
	int iOffset = 0;
	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_WAREHOUSE);
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_WARE_GET_IN);
	CAPISocket::MP_AddDword(byBuff, iOffset, iItemID);
	CAPISocket::MP_AddByte(byBuff, iOffset, page);
	CAPISocket::MP_AddByte(byBuff, iOffset, startpos);
	CAPISocket::MP_AddByte(byBuff, iOffset, pos);
	CAPISocket::MP_AddDword(byBuff, iOffset, iCount);

	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
}

void CUIWareHouseDlg::SendToServerFromWareMsg(int iItemID, uint8_t page, uint8_t startpos, uint8_t pos, int iCount)
{
	uint8_t byBuff[32];
	int iOffset = 0;
	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_WAREHOUSE);
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_WARE_GET_OUT);
	CAPISocket::MP_AddDword(byBuff, iOffset, iItemID);
	CAPISocket::MP_AddByte(byBuff, iOffset, page);
	CAPISocket::MP_AddByte(byBuff, iOffset, startpos);
	CAPISocket::MP_AddByte(byBuff, iOffset, pos);
	CAPISocket::MP_AddDword(byBuff, iOffset, iCount);

	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
}

void CUIWareHouseDlg::SendToServerWareToWareMsg(int iItemID, uint8_t page, uint8_t startpos, uint8_t destpos)
{
	uint8_t byBuff[32];
	int iOffset = 0;
	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_WAREHOUSE);
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_WARE_WARE_MOVE);
	CAPISocket::MP_AddDword(byBuff, iOffset, iItemID);
	CAPISocket::MP_AddByte(byBuff, iOffset, page);
	CAPISocket::MP_AddByte(byBuff, iOffset, startpos);
	CAPISocket::MP_AddByte(byBuff, iOffset, destpos);

	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
}

void CUIWareHouseDlg::SendToServerInvToInvMsg(int iItemID, uint8_t page, uint8_t startpos, uint8_t destpos)
{
	uint8_t byBuff[32];
	int iOffset = 0;
	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_WAREHOUSE);
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_WARE_INV_MOVE);
	CAPISocket::MP_AddDword(byBuff, iOffset, iItemID);
	CAPISocket::MP_AddByte(byBuff, iOffset, page);
	CAPISocket::MP_AddByte(byBuff, iOffset, startpos);
	CAPISocket::MP_AddByte(byBuff, iOffset, destpos);

	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
}

// 넣는 경우..
void CUIWareHouseDlg::ReceiveResultToWareMsg(uint8_t bResult)
{
	s_bWaitFromServer       = false;

	int iGold               = s_pCountableItemEdit->GetQuantity();
	__IconItemSkill* spItem = nullptr;
	CN3UIArea* pArea        = nullptr;

	// 실패..
	if (bResult != 0x01)
	{
		if (m_bSendedItemGold)
		{
			ReceiveResultGoldToWareFail();
			return;
		}

		// 활이나 물약등 아이템인 경우..
		if (s_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE
			|| s_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL)
		{
			// Ware Side..
			if ((m_pMyWare[s_sRecoveryJobInfo.m_iPage][s_sRecoveryJobInfo.UIWndSourceEnd.iOrder]->iCount - iGold) > 0)
			{
				//  숫자 업데이트..
				m_pMyWare[s_sRecoveryJobInfo.m_iPage][s_sRecoveryJobInfo.UIWndSourceEnd.iOrder]->iCount -= iGold;
			}
			else
			{
				// 아이템 삭제.. 현재 인벤토리 윈도우만..
				spItem = m_pMyWare[s_sRecoveryJobInfo.m_iPage][s_sRecoveryJobInfo.UIWndSourceEnd.iOrder];

				// 인벤토리에서도 지운다..
				m_pMyWare[s_sRecoveryJobInfo.m_iPage][s_sRecoveryJobInfo.UIWndSourceEnd.iOrder] = nullptr;

				// iOrder로 내 매니저의 아이템을 리스트에서 삭제한다..
				RemoveChild(spItem->pUIIcon);

				// 아이콘 리소스 삭제...
				spItem->pUIIcon->Release();
				delete spItem->pUIIcon;
				spItem->pUIIcon = nullptr;
				delete spItem;
				spItem = nullptr;
			}

			// Inv Side..	//////////////////////////////////////////////////////

			if (!m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceStart.iOrder]->pUIIcon->IsVisible())
			{
				m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceStart.iOrder]->pUIIcon->SetVisible(true);
				m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceStart.iOrder]->iCount = iGold;
			}
			else
			{
				m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceStart.iOrder]->iCount += iGold;
			}
		}
		// 일반 아이템인 경우..
		else
		{
			m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceStart.iOrder] = m_pMyWare[m_iCurPage][s_sRecoveryJobInfo.UIWndSourceEnd.iOrder];

			pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, s_sRecoveryJobInfo.UIWndSourceStart.iOrder);
			if (pArea != nullptr)
			{
				m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceStart.iOrder]->pUIIcon->SetRegion(pArea->GetRegion());
				m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceStart.iOrder]->pUIIcon->SetMoveRect(pArea->GetRegion());
			}

			m_pMyWare[m_iCurPage][s_sRecoveryJobInfo.UIWndSourceEnd.iOrder] = nullptr;
		}
	}
	// 성공..
	else
	{
		// 원래 대로..
		if (m_bSendedItemGold)
		{
			m_bSendedItemGold = false;
			return;
		}

		// 활이나 물약등 아이템인 경우..
		if ((s_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE
				|| s_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL)
			&& !m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceStart.iOrder]->pUIIcon->IsVisible())
		{
			spItem                                                   = m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceStart.iOrder];

			// 인벤토리에서도 지운다..
			m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceStart.iOrder] = nullptr;

			// iOrder로 내 매니저의 아이템을 리스트에서 삭제한다..
			RemoveChild(spItem->pUIIcon);

			// 아이콘 리소스 삭제...
			spItem->pUIIcon->Release();
			delete spItem->pUIIcon;
			spItem->pUIIcon = nullptr;
			delete spItem;
			spItem = nullptr;
		}
	}
}

// 빼는 경우..
void CUIWareHouseDlg::ReceiveResultFromWareMsg(uint8_t bResult)
{
	s_bWaitFromServer       = false;

	int iGold               = s_pCountableItemEdit->GetQuantity();
	__IconItemSkill* spItem = nullptr;
	CN3UIArea* pArea        = nullptr;

	if (bResult != 0x01) // 실패..
	{
		if (m_bSendedItemGold)
		{
			ReceiveResultGoldFromWareFail();
			return;
		}

		if (s_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE
			|| s_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL)
		{
			// Inv Side..	//////////////////////////////////////////////////////
			if ((m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceEnd.iOrder]->iCount - iGold) > 0)
			{
				// 숫자 업데이트..
				m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceEnd.iOrder]->iCount -= iGold;
			}
			else
			{
				// 아이템 삭제.. 현재 인벤토리 윈도우만..
				spItem                                                 = m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceEnd.iOrder];

				// 인벤토리에서도 지운다..
				m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceEnd.iOrder] = nullptr;

				// iOrder로 내 매니저의 아이템을 리스트에서 삭제한다..
				RemoveChild(spItem->pUIIcon);

				// 아이콘 리소스 삭제...
				spItem->pUIIcon->Release();
				delete spItem->pUIIcon;
				spItem->pUIIcon = nullptr;
				delete spItem;
				spItem = nullptr;
			}

			// Ware Side..

			if (!m_pMyWare[m_iCurPage][s_sRecoveryJobInfo.UIWndSourceStart.iOrder]->pUIIcon->IsVisible())
			{
				m_pMyWare[m_iCurPage][s_sRecoveryJobInfo.UIWndSourceStart.iOrder]->pUIIcon->SetVisible(true);
				m_pMyWare[m_iCurPage][s_sRecoveryJobInfo.UIWndSourceStart.iOrder]->iCount = iGold;
			}
			else
			{
				m_pMyWare[m_iCurPage][s_sRecoveryJobInfo.UIWndSourceStart.iOrder]->iCount += iGold;
			}
		}
		else
		{
			// 일반 아이템..
			m_pMyWare[m_iCurPage][s_sRecoveryJobInfo.UIWndSourceStart.iOrder] = m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceEnd.iOrder];

			pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_NPC, s_sRecoveryJobInfo.UIWndSourceStart.iOrder);
			if (pArea != nullptr)
			{
				m_pMyWare[m_iCurPage][s_sRecoveryJobInfo.UIWndSourceStart.iOrder]->pUIIcon->SetRegion(pArea->GetRegion());
				m_pMyWare[m_iCurPage][s_sRecoveryJobInfo.UIWndSourceStart.iOrder]->pUIIcon->SetMoveRect(pArea->GetRegion());
			}

			m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceEnd.iOrder] = nullptr;
		}

		std::string szMsg = fmt::format_text_resource(IDS_ITEM_TOOMANY_OR_HEAVY);
		CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);
	}
	else
	{
		// 성공..
		if (m_bSendedItemGold)
		{
			// 원래 대로..
			m_bSendedItemGold = false;
			return;
		}

		// 활이나 물약등 아이템인 경우..
		if ((s_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE
				|| s_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL)
			&& !m_pMyWare[m_iCurPage][s_sRecoveryJobInfo.UIWndSourceStart.iOrder]->pUIIcon->IsVisible())
		{
			spItem = m_pMyWare[m_iCurPage][s_sRecoveryJobInfo.UIWndSourceStart.iOrder];

			// 인벤토리에서도 지운다..
			m_pMyWare[m_iCurPage][s_sRecoveryJobInfo.UIWndSourceStart.iOrder] = nullptr;

			// iOrder로 내 매니저의 아이템을 리스트에서 삭제한다..
			RemoveChild(spItem->pUIIcon);

			// 아이콘 리소스 삭제...
			spItem->pUIIcon->Release();
			delete spItem->pUIIcon;
			spItem->pUIIcon = nullptr;
			delete spItem;
			spItem = nullptr;
		}
	}
}

void CUIWareHouseDlg::ReceiveResultWareToWareMsg(uint8_t bResult)
{
	s_bWaitFromServer = false;

	// 실패..
	if (bResult != 0x01)
	{
		__IconItemSkill* spItemSource = s_sRecoveryJobInfo.pItemSource;
		__IconItemSkill* spItemTarget = s_sRecoveryJobInfo.pItemTarget;

		if (spItemSource != nullptr)
		{
			CN3UIArea* pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_NPC, s_sRecoveryJobInfo.UIWndSourceStart.iOrder);
			if (pArea != nullptr)
			{
				spItemSource->pUIIcon->SetRegion(pArea->GetRegion());
				spItemSource->pUIIcon->SetMoveRect(pArea->GetRegion());
			}

			m_pMyWare[m_iCurPage][s_sRecoveryJobInfo.UIWndSourceStart.iOrder] = spItemSource;
		}

		if (spItemTarget != nullptr)
		{
			CN3UIArea* pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_NPC, s_sRecoveryJobInfo.UIWndSourceEnd.iOrder);
			if (pArea != nullptr)
			{
				spItemTarget->pUIIcon->SetRegion(pArea->GetRegion());
				spItemTarget->pUIIcon->SetMoveRect(pArea->GetRegion());
			}

			m_pMyWare[m_iCurPage][s_sRecoveryJobInfo.UIWndSourceEnd.iOrder] = spItemTarget;
		}
		else
		{
			m_pMyWare[m_iCurPage][s_sRecoveryJobInfo.UIWndSourceEnd.iOrder] = nullptr;
		}
	}

	AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);
}

void CUIWareHouseDlg::ReceiveResultInvToInvMsg(uint8_t bResult)
{
	s_bWaitFromServer = false;

	// 실패..
	if (bResult != 0x01)
	{
		__IconItemSkill* spItemSource = s_sRecoveryJobInfo.pItemSource;
		__IconItemSkill* spItemTarget = s_sRecoveryJobInfo.pItemTarget;

		if (spItemSource != nullptr)
		{
			CN3UIArea* pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, s_sRecoveryJobInfo.UIWndSourceStart.iOrder);
			if (pArea != nullptr)
			{
				spItemSource->pUIIcon->SetRegion(pArea->GetRegion());
				spItemSource->pUIIcon->SetMoveRect(pArea->GetRegion());
			}

			m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceStart.iOrder] = spItemSource;
		}

		if (spItemTarget != nullptr)
		{
			CN3UIArea* pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, s_sRecoveryJobInfo.UIWndSourceEnd.iOrder);
			if (pArea != nullptr)
			{
				spItemTarget->pUIIcon->SetRegion(pArea->GetRegion());
				spItemTarget->pUIIcon->SetMoveRect(pArea->GetRegion());
			}

			m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceEnd.iOrder] = spItemTarget;
		}
		else
		{
			m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceEnd.iOrder] = nullptr;
		}
	}

	AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);
}

void CUIWareHouseDlg::ItemCountOK()
{
	constexpr float UVAspect     = 45.0f / 64.0f;
	int iGold                    = s_pCountableItemEdit->GetQuantity();
	__IconItemSkill* spItem      = nullptr;
	CN3UIArea* pArea             = nullptr;
	int iWeight                  = 0;
	__InfoPlayerMySelf* pInfoExt = &CGameBase::s_pPlayer->m_InfoExt;

	switch (s_pCountableItemEdit->GetCallerWndDistrict())
	{
		// 빼는 경우..
		case UIWND_DISTRICT_TRADE_NPC:
			spItem = m_pMyWare[m_iCurPage][s_sRecoveryJobInfo.UIWndSourceStart.iOrder];
			if (iGold > spItem->iCount)
				return;

			switch (spItem->pItemBasic->byContable)
			{
				case UIITEM_TYPE_ONLYONE:
				case UIITEM_TYPE_SOMOONE:
					iWeight = spItem->pItemBasic->siWeight;

					// 무게 체크..
					if ((pInfoExt->iWeight + iWeight) > pInfoExt->iWeightMax)
					{
						std::string szMsg = fmt::format_text_resource(IDS_ITEM_WEIGHT_OVERFLOW);
						CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);
						return;
					}
					break;

				case UIITEM_TYPE_COUNTABLE:
					if (iGold <= 0)
						return;

					// int16_t 범위이상은 살수 없다..
					if (iGold > UIITEM_COUNT_MANY)
					{
						std::string szMsg = fmt::format_text_resource(IDS_MANY_COUNTABLE_ITEM_GET_MANY);
						CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);
						return;
					}

					if (m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceEnd.iOrder] != nullptr)
					{
						spItem = m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceEnd.iOrder];
						if (spItem->iCount + iGold > UIITEM_COUNT_MANY)
						{
							std::string szMsg = fmt::format_text_resource(IDS_MANY_COUNTABLE_ITEM_GET_MANY);
							CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);
							return;
						}
					}

					// 무게 체크..
					iWeight = iGold * spItem->pItemBasic->siWeight;
					if ((pInfoExt->iWeight + iWeight) > pInfoExt->iWeightMax)
					{
						std::string szMsg = fmt::format_text_resource(IDS_ITEM_WEIGHT_OVERFLOW);
						CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);
						return;
					}
					break;

				case UIITEM_TYPE_COUNTABLE_SMALL:
					if (iGold <= 0)
						return;

					// int16_t 범위이상은 살수 없다..
					if (iGold > UIITEM_COUNT_FEW)
					{
						std::string szMsg = fmt::format_text_resource(IDS_SMALL_COUNTABLE_ITEM_GET_MANY);
						CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);
						return;
					}

					if (m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceEnd.iOrder] != nullptr)
					{
						spItem = m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceEnd.iOrder];
						if (spItem->iCount + iGold > UIITEM_COUNT_FEW)
						{
							std::string szMsg = fmt::format_text_resource(IDS_SMALL_COUNTABLE_ITEM_GET_MANY);
							CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);
							return;
						}
					}

					// 무게 체크..
					iWeight = iGold * spItem->pItemBasic->siWeight;
					if ((pInfoExt->iWeight + iWeight) > pInfoExt->iWeightMax)
					{
						std::string szMsg = fmt::format_text_resource(IDS_ITEM_WEIGHT_OVERFLOW);
						CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);
						return;
					}
					break;

				default:
					break;
			}

			spItem            = m_pMyWare[m_iCurPage][s_sRecoveryJobInfo.UIWndSourceStart.iOrder];

			s_bWaitFromServer = true;

			// 해당 위치에 아이콘이 있으면..
			if (m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceEnd.iOrder] != nullptr)
			{
				// 숫자 업데이트..
				m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceEnd.iOrder]->iCount += iGold;
			}
			else // 없으면 아이콘을 만든다...
			{
				__IconItemSkill* spItemNew = new __IconItemSkill();
				spItemNew->pItemBasic      = spItem->pItemBasic;
				spItemNew->pItemExt        = spItem->pItemExt;
				spItemNew->szIconFN        = spItem->szIconFN;
				spItemNew->iCount          = iGold;
				spItemNew->iDurability     = spItem->iDurability;
				spItemNew->pUIIcon         = new CN3UIIcon();
				spItemNew->pUIIcon->Init(this);
				spItemNew->pUIIcon->SetTex(spItem->szIconFN);
				spItemNew->pUIIcon->SetUVRect(0, 0, UVAspect, UVAspect);
				spItemNew->pUIIcon->SetUIType(UI_TYPE_ICON);
				spItemNew->pUIIcon->SetStyle(UISTYLE_ICON_ITEM | UISTYLE_ICON_CERTIFICATION_NEED);
				spItemNew->pUIIcon->SetVisible(true);

				pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, s_sRecoveryJobInfo.UIWndSourceEnd.iOrder);
				if (pArea != nullptr)
				{
					spItemNew->pUIIcon->SetRegion(pArea->GetRegion());
					spItemNew->pUIIcon->SetMoveRect(pArea->GetRegion());
				}

				m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceEnd.iOrder] = spItemNew;
			}

			// 숫자 업데이트..
			if ((spItem->iCount - iGold) > 0)
				spItem->iCount -= iGold;
			else
				spItem->pUIIcon->SetVisible(false);

			// 표시는 아이콘 렌더링할때.. Inventory의 Render에서..
			// 서버에게 보냄..
			SendToServerFromWareMsg(s_sRecoveryJobInfo.pItemSource->pItemBasic->dwID + s_sRecoveryJobInfo.pItemSource->pItemExt->dwID,
				m_iCurPage, s_sRecoveryJobInfo.UIWndSourceStart.iOrder, s_sRecoveryJobInfo.UIWndSourceEnd.iOrder, iGold);

			// Sound..
			if (s_sRecoveryJobInfo.pItemSource != nullptr)
				PlayItemSound(s_sRecoveryJobInfo.pItemSource->pItemBasic);
			break;

		// 넣는 경우..
		case UIWND_DISTRICT_TRADE_MY:
			spItem = m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceStart.iOrder];
			if (iGold > spItem->iCount)
				return;

			switch (spItem->pItemBasic->byContable)
			{
				case UIITEM_TYPE_COUNTABLE:
					if (iGold <= 0)
						return;

					// int16_t 범위이상은 살수 없다..
					if (iGold > UIITEM_COUNT_MANY)
					{
						std::string szMsg = fmt::format_text_resource(IDS_MANY_COUNTABLE_ITEM_GET_MANY);
						CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);
						return;
					}

					if (m_pMyWare[s_sRecoveryJobInfo.m_iPage][s_sRecoveryJobInfo.UIWndSourceEnd.iOrder] != nullptr)
					{
						spItem = m_pMyWare[s_sRecoveryJobInfo.m_iPage][s_sRecoveryJobInfo.UIWndSourceEnd.iOrder];
						if (spItem->iCount + iGold > UIITEM_COUNT_MANY)
						{
							std::string szMsg = fmt::format_text_resource(IDS_MANY_COUNTABLE_ITEM_GET_MANY);
							CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);
							return;
						}
					}
					break;

				case UIITEM_TYPE_COUNTABLE_SMALL:
					if (iGold <= 0)
						return;

					// int16_t 범위이상은 살수 없다..
					if (iGold > UIITEM_COUNT_FEW)
					{
						std::string szMsg = fmt::format_text_resource(IDS_SMALL_COUNTABLE_ITEM_GET_MANY);
						CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);
						return;
					}

					if (m_pMyWare[s_sRecoveryJobInfo.m_iPage][s_sRecoveryJobInfo.UIWndSourceEnd.iOrder] != nullptr)
					{
						spItem = m_pMyWare[s_sRecoveryJobInfo.m_iPage][s_sRecoveryJobInfo.UIWndSourceEnd.iOrder];
						if (spItem->iCount + iGold > UIITEM_COUNT_FEW)
						{
							std::string szMsg = fmt::format_text_resource(IDS_SMALL_COUNTABLE_ITEM_GET_MANY);
							CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);
							return;
						}
					}
					break;

				default:
					break;
			}

			spItem            = m_pMyWareInv[s_sRecoveryJobInfo.UIWndSourceStart.iOrder];

			s_bWaitFromServer = true;

			// 해당 위치에 아이콘이 있으면..
			if (m_pMyWare[s_sRecoveryJobInfo.m_iPage][s_sRecoveryJobInfo.UIWndSourceEnd.iOrder])
			{
				// 숫자 업데이트..
				m_pMyWare[s_sRecoveryJobInfo.m_iPage][s_sRecoveryJobInfo.UIWndSourceEnd.iOrder]->iCount += iGold;
			}
			// 없으면 아이콘을 만든다..
			else
			{
				__IconItemSkill* spItemNew = new __IconItemSkill();
				spItemNew->pItemBasic      = spItem->pItemBasic;
				spItemNew->pItemExt        = spItem->pItemExt;
				spItemNew->szIconFN        = spItem->szIconFN;
				spItemNew->iCount          = iGold;
				spItemNew->iDurability     = spItem->iDurability;
				spItemNew->pUIIcon         = new CN3UIIcon();
				spItemNew->pUIIcon->Init(this);
				spItemNew->pUIIcon->SetTex(spItem->szIconFN);
				spItemNew->pUIIcon->SetUVRect(0, 0, UVAspect, UVAspect);
				spItemNew->pUIIcon->SetUIType(UI_TYPE_ICON);
				spItemNew->pUIIcon->SetStyle(UISTYLE_ICON_ITEM | UISTYLE_ICON_CERTIFICATION_NEED);
				spItemNew->pUIIcon->SetVisible(true);

				pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_NPC, s_sRecoveryJobInfo.UIWndSourceEnd.iOrder);
				if (pArea != nullptr)
				{
					spItemNew->pUIIcon->SetRegion(pArea->GetRegion());
					spItemNew->pUIIcon->SetMoveRect(pArea->GetRegion());
				}

				if (s_sRecoveryJobInfo.m_iPage != m_iCurPage)
					spItemNew->pUIIcon->SetVisibleWithNoSound(false);

				m_pMyWare[s_sRecoveryJobInfo.m_iPage][s_sRecoveryJobInfo.UIWndSourceEnd.iOrder] = spItemNew;
			}

			//  숫자 업데이트..
			if ((spItem->iCount - iGold) > 0)
				spItem->iCount -= iGold;
			else
				spItem->pUIIcon->SetVisible(false);

			// 표시는 아이콘 렌더링할때.. Inventory의 Render에서..
			// 서버에게 보냄..
			SendToServerToWareMsg(s_sRecoveryJobInfo.pItemSource->pItemBasic->dwID + s_sRecoveryJobInfo.pItemSource->pItemExt->dwID,
				s_sRecoveryJobInfo.m_iPage, s_sRecoveryJobInfo.UIWndSourceStart.iOrder, s_sRecoveryJobInfo.UIWndSourceEnd.iOrder, iGold);

			break;

		default:
			break;
	}

	s_pCountableItemEdit->Close();
}

void CUIWareHouseDlg::ItemCountCancel()
{
	// Sound..
	if (s_sRecoveryJobInfo.pItemSource != nullptr)
		PlayItemSound(s_sRecoveryJobInfo.pItemSource->pItemBasic);

	// 취소..
	s_bWaitFromServer              = false;
	s_sRecoveryJobInfo.pItemSource = nullptr;
	s_sRecoveryJobInfo.pItemTarget = nullptr;

	s_pCountableItemEdit->Close();
}

void CUIWareHouseDlg::ItemMoveFromInvToThis()
{
	CUIInventory* pInven = CGameProcedure::s_pProcMain->m_pUIInventory;
	if (pInven == nullptr)
		return;

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		m_pMyWareInv[i] = nullptr;

		if (pInven->m_pMyInvWnd[i] != nullptr)
		{
			__IconItemSkill* spItem = pInven->m_pMyInvWnd[i];
			spItem->pUIIcon->SetParent(this);

			pInven->m_pMyInvWnd[i] = nullptr;

			CN3UIArea* pArea       = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, i);
			if (pArea != nullptr)
			{
				spItem->pUIIcon->SetRegion(pArea->GetRegion());
				spItem->pUIIcon->SetMoveRect(pArea->GetRegion());
			}

			m_pMyWareInv[i] = spItem;
		}
	}
}

void CUIWareHouseDlg::ItemMoveFromThisToInv()
{
	CUIInventory* pInven = CGameProcedure::s_pProcMain->m_pUIInventory;
	if (pInven == nullptr)
		return;

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (m_pMyWareInv[i] != nullptr)
		{
			__IconItemSkill* spItem = m_pMyWareInv[i];
			spItem->pUIIcon->SetParent(pInven);

			m_pMyWareInv[i]  = nullptr;

			CN3UIArea* pArea = pInven->GetChildAreaByiOrder(UI_AREA_TYPE_INV, i);
			if (pArea != nullptr)
			{
				spItem->pUIIcon->SetRegion(pArea->GetRegion());
				spItem->pUIIcon->SetMoveRect(pArea->GetRegion());
			}

			pInven->m_pMyInvWnd[i] = spItem;
		}
	}
}

void CUIWareHouseDlg::AddItemInWare(int iItem, int iDurability, int iCount, int iIndex)
{
	if (iItem <= 0)
		return;

	std::string szIconFN;
	__IconItemSkill* spItem    = nullptr;
	// 아이템 테이블 구조체 포인터..
	__TABLE_ITEM_BASIC* pItem  = nullptr;
	__TABLE_ITEM_EXT* pItemExt = nullptr;
	// 열 데이터 얻기..
	pItem                      = CGameBase::s_pTbl_Items_Basic.Find(iItem / 1000 * 1000);
	if (pItem && pItem->byExtIndex >= 0 && pItem->byExtIndex < MAX_ITEM_EXTENSION)
		pItemExt = CGameBase::s_pTbl_Items_Exts[pItem->byExtIndex].Find(iItem % 1000); // 열 데이터 얻기..
	if (pItem == nullptr || pItemExt == nullptr)
	{
		__ASSERT(0, "NULL Item!!!");
		CLogWriter::Write("WareHouse - Ware - Unknown Item {}, IDNumber", iItem);
		// 아이템이 없으면..
		return;
	}

	e_PartPosition ePart = PART_POS_UNKNOWN;
	e_PlugPosition ePlug = PLUG_POS_UNKNOWN;
	// 아이템에 따른 파일 이름을 만들어서
	e_ItemType eType     = CGameBase::MakeResrcFileNameForUPC(pItem, pItemExt, nullptr, &szIconFN, ePart, ePlug);
	if (ITEM_TYPE_UNKNOWN == eType)
		CLogWriter::Write("MyInfo - slot - Unknown Item");
	__ASSERT(ITEM_TYPE_UNKNOWN != eType, "Unknown Item");

	spItem                                                      = new __IconItemSkill();
	spItem->pItemBasic                                          = pItem;
	spItem->pItemExt                                            = pItemExt;
	spItem->szIconFN                                            = szIconFN; // 아이콘 파일 이름 복사..
	spItem->iCount                                              = iCount;
	spItem->iCount                                              = iCount;
	spItem->iDurability                                         = iDurability;

	m_pMyWare[iIndex / MAX_ITEM_TRADE][iIndex % MAX_ITEM_TRADE] = spItem;
}

// 돈을 넣는 경우..
void CUIWareHouseDlg::GoldCountToWareOK()
{
	// 인벤토리의 값..
	int iGold = 0, iMyMoney = 0, iWareMoney = 0;
	std::string str;

	// 돈을 보관함에 보관하는 경우..
	iGold               = s_pCountableItemEdit->GetQuantity();

	// Gold Offset Backup..
	m_iGoldOffsetBackup = iGold;

	// 현재 내가 가진 돈을 얻어 온다..
	iMyMoney            = CGameBase::s_pPlayer->m_InfoExt.iGold;

	// 보관함의 돈을 얻어온다..
	CN3UIString* pStr   = GetChildByID<CN3UIString>("string_wareitem_name");
	__ASSERT(pStr, "NULL UI Component!!");
	str        = CGameBase::UnformatNumber(pStr->GetString());
	iWareMoney = atoi(str.c_str());

	if (iGold <= 0)
		return;

	if (iGold > iMyMoney)
		return;

	// 보낸 아이템이 돈이다.. !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	m_bSendedItemGold                      = true;

	// 돈을 감소 시킨다..
	iMyMoney                              -= iGold;
	CGameBase::s_pPlayer->m_InfoExt.iGold  = iMyMoney;

	iWareMoney                            += iGold;

	// 돈 표시.. Ware..
	pStr->SetString(CGameBase::FormatNumber(iWareMoney));

	// 돈 표시.. 인벤토리..
	CGameProcedure::s_pProcMain->m_pUIInventory->GoldUpdate();

	// 돈 표시.. Inv..
	N3_VERIFY_UI_COMPONENT(pStr, GetChildByID<CN3UIString>("string_item_name"));
	if (pStr != nullptr)
		pStr->SetString(CGameBase::FormatNumber(iMyMoney));

	// 서버에게 패킷 만들어서 날림..
	SendToServerToWareMsg(dwGold, 0xff, 0xff, 0xff, iGold);

	// 상태를 변화시키고.. 창을 닫고..
	s_bWaitFromServer = true;
	s_pCountableItemEdit->Close();

	PlayGoldSound();
}

// 돈을 빼는 경우..
void CUIWareHouseDlg::GoldCountFromWareOK()
{
	int iGold = 0, iMyMoney = 0, iWareMoney = 0; // 인벤토리의 값..
	std::string str;

	// 돈을 보관함에서 빼는 경우..
	iGold               = s_pCountableItemEdit->GetQuantity();

	// Gold Offset Backup..
	m_iGoldOffsetBackup = iGold;

	// 현재 내가 가진 돈을 얻어 온다..
	iMyMoney            = CGameBase::s_pPlayer->m_InfoExt.iGold;

	// 보관함의 돈을 얻어온다..
	CN3UIString* pStr   = nullptr;
	N3_VERIFY_UI_COMPONENT(pStr, GetChildByID<CN3UIString>("string_wareitem_name"));
	str        = CGameBase::UnformatNumber(pStr->GetString());
	iWareMoney = atoi(str.c_str());

	if (iGold <= 0)
		return;

	if (iGold > iWareMoney)
		return;

	// 보낸 아이템이 돈이다.. !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	m_bSendedItemGold                      = true;

	// 돈을 감소 시킨다..
	iMyMoney                              += iGold;
	CGameBase::s_pPlayer->m_InfoExt.iGold  = iMyMoney;

	iWareMoney                            -= iGold;

	// 돈 표시.. Ware..
	pStr->SetString(CGameBase::FormatNumber(iWareMoney));

	// 돈 표시.. 인벤토리..
	CGameProcedure::s_pProcMain->m_pUIInventory->GoldUpdate();

	// 돈 표시.. Inv..
	N3_VERIFY_UI_COMPONENT(pStr, GetChildByID<CN3UIString>("string_item_name"));
	if (pStr != nullptr)
		pStr->SetString(CGameBase::FormatNumber(iMyMoney));

	// 서버에게 패킷 만들어서 날림..
	SendToServerFromWareMsg(dwGold, 0xff, 0xff, 0xff, iGold);

	// 상태를 변화시키고.. 창을 닫고..
	s_bWaitFromServer = true;
	s_pCountableItemEdit->Close();

	PlayGoldSound();
}

void CUIWareHouseDlg::GoldCountToWareCancel()
{
	// 돈을 보관함에 보관하는 경우 취소..
	// Sound..
	PlayGoldSound();

	// 취소..
	s_bWaitFromServer              = false;
	s_sRecoveryJobInfo.pItemSource = nullptr;
	s_sRecoveryJobInfo.pItemTarget = nullptr;

	s_pCountableItemEdit->Close();
}

void CUIWareHouseDlg::GoldCountFromWareCancel()
{
	// 돈을 보관함에서 빼오는 경우 취소..
	// Sound..
	PlayGoldSound();

	// 취소..
	s_bWaitFromServer              = false;
	s_sRecoveryJobInfo.pItemSource = nullptr;
	s_sRecoveryJobInfo.pItemTarget = nullptr;

	s_pCountableItemEdit->Close();
}

void CUIWareHouseDlg::ReceiveResultGoldToWareFail()
{
	// 원래 대로..
	m_bSendedItemGold = false;

	// 인벤토리의 값..
	int iGold = 0, iMyMoney = 0, iWareMoney = 0;
	std::string str;

	// 돈을 보관함에서 빼는 경우..
	iGold               = s_pCountableItemEdit->GetQuantity();

	// Gold Offset Backup..
	m_iGoldOffsetBackup = iGold;

	// 현재 내가 가진 돈을 얻어 온다..
	iMyMoney            = CGameBase::s_pPlayer->m_InfoExt.iGold;

	// 보관함의 돈을 얻어온다..
	CN3UIString* pStr   = nullptr;
	N3_VERIFY_UI_COMPONENT(pStr, GetChildByID<CN3UIString>("string_wareitem_name"));
	str                                    = CGameBase::UnformatNumber(pStr->GetString());
	iWareMoney                             = atoi(str.c_str());

	// 돈을 감소 시킨다..
	iMyMoney                              += iGold;
	CGameBase::s_pPlayer->m_InfoExt.iGold  = iMyMoney;

	iWareMoney                            -= iGold;

	// 돈 표시.. Ware..
	pStr->SetStringAsInt(iWareMoney);

	// 돈 표시.. 인벤토리..
	CGameProcedure::s_pProcMain->m_pUIInventory->GoldUpdate();

	// 돈 표시.. Inv..
	N3_VERIFY_UI_COMPONENT(pStr, GetChildByID<CN3UIString>("string_item_name"));
	if (pStr != nullptr)
		pStr->SetStringAsInt(iMyMoney);
}

void CUIWareHouseDlg::ReceiveResultGoldFromWareFail()
{
	// 원래 대로..
	m_bSendedItemGold = false;

	// 인벤토리의 값..
	int iGold = 0, iMyMoney = 0, iWareMoney = 0;
	std::string str;

	// 돈을 보관함에 보관하는 경우..
	iGold               = s_pCountableItemEdit->GetQuantity();

	// Gold Offset Backup..
	m_iGoldOffsetBackup = iGold;

	// 현재 내가 가진 돈을 얻어 온다..
	iMyMoney            = CGameBase::s_pPlayer->m_InfoExt.iGold;

	// 보관함의 돈을 얻어온다..
	CN3UIString* pStr   = nullptr;
	N3_VERIFY_UI_COMPONENT(pStr, GetChildByID<CN3UIString>("string_wareitem_name"));
	str                                    = CGameBase::UnformatNumber(pStr->GetString());
	iWareMoney                             = atoi(str.c_str());

	// 돈을 감소 시킨다..
	iMyMoney                              -= iGold;
	CGameBase::s_pPlayer->m_InfoExt.iGold  = iMyMoney;

	iWareMoney                            += iGold;

	// 돈 표시.. Ware..
	pStr->SetStringAsInt(iWareMoney);

	// 돈 표시.. 인벤토리..
	CGameProcedure::s_pProcMain->m_pUIInventory->GoldUpdate();

	// 돈 표시.. Inv..
	N3_VERIFY_UI_COMPONENT(pStr, GetChildByID<CN3UIString>("string_item_name"));
	if (pStr != nullptr)
		pStr->SetStringAsInt(iMyMoney);
}

bool CUIWareHouseDlg::OnKeyPress(int iKey)
{
	switch (iKey)
	{
		case DIK_PRIOR:
			ReceiveMessage(m_pBtnPageUp, UIMSG_BUTTON_CLICK);
			return true;

		case DIK_NEXT:
			ReceiveMessage(m_pBtnPageDown, UIMSG_BUTTON_CLICK);
			return true;

		case DIK_ESCAPE:
			ReceiveMessage(m_pBtnClose, UIMSG_BUTTON_CLICK);
			return true;

		default:
			break;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

bool CUIWareHouseDlg::Load(File& file)
{
	if (!CN3UIBase::Load(file))
		return false;

	N3_VERIFY_UI_COMPONENT(m_pBtnGold, GetChildByID<CN3UIButton>("btn_gold"));
	N3_VERIFY_UI_COMPONENT(m_pBtnGoldWareHouse, GetChildByID<CN3UIButton>("btn_gold_warehouse"));
	N3_VERIFY_UI_COMPONENT(m_pBtnClose, GetChildByID<CN3UIButton>("btn_close"));
	N3_VERIFY_UI_COMPONENT(m_pBtnPageUp, GetChildByID<CN3UIButton>("btn_page_up"));
	N3_VERIFY_UI_COMPONENT(m_pBtnPageDown, GetChildByID<CN3UIButton>("btn_page_down"));

	return true;
}

void CUIWareHouseDlg::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);
	if (bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
		CGameProcedure::s_pUIMgr->ReFocusUI();
}

void CUIWareHouseDlg::SetVisibleWithNoSound(bool bVisible, bool bWork, bool bReFocus)
{
	CN3UIBase::SetVisibleWithNoSound(bVisible, bWork, bReFocus);

	if (bWork)
	{
		if (s_pCountableItemEdit != nullptr && s_pCountableItemEdit->IsVisible())
			s_pCountableItemEdit->SetVisibleWithNoSound(bVisible, bWork, bReFocus);

		if (GetState() == UI_STATE_ICON_MOVING)
			IconRestore();

		SetState(UI_STATE_COMMON_NONE);
		AllHighLightIconFree();

		// 이 윈도우의 inv 영역의 아이템을 이 인벤토리 윈도우의 inv영역으로 옮긴다..
		ItemMoveFromThisToInv();

		if (CGameProcedure::s_pProcMain->m_pUISkillTreeDlg != nullptr)
			CGameProcedure::s_pProcMain->m_pUISkillTreeDlg->UpdateDisableCheck();

		if (CGameProcedure::s_pProcMain->m_pUIHotKeyDlg != nullptr)
			CGameProcedure::s_pProcMain->m_pUIHotKeyDlg->UpdateDisableCheck();
	}
}

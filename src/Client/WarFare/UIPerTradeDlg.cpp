// UIPerTradeDlg.cpp: implementation of the CUIPerTradeDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UIPerTradeDlg.h"
#include "PacketDef.h"
#include "LocalInput.h"
#include "APISocket.h"
#include "GameProcMain.h"
#include "PlayerMySelf.h"
#include "N3UIWndBase.h"
#include "UIImageTooltipDlg.h"
#include "UIInventory.h"
#include "SubProcPerTrade.h"
#include "CountableItemEditDlg.h"
#include "UIManager.h"
#include "text_resources.h"

#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIEdit.h>
#include <N3Base/N3UIString.h>
#include <N3Base/N3SndObj.h>

CUIPerTradeDlg::CUIPerTradeDlg()
{
	m_pSubProcPerTrade = nullptr;

	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
	{
		m_pPerTradeMy[i]    = nullptr;
		m_pPerTradeOther[i] = nullptr;
	}

	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
		m_iBackupiOrder[i] = -1;

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
		m_pPerTradeInv[i] = nullptr;

	m_iBackupiCount = 0;
	m_pUITooltipDlg = nullptr;
	m_pStrMyGold    = nullptr;

	SetVisible(false);
}

CUIPerTradeDlg::~CUIPerTradeDlg()
{
	CUIPerTradeDlg::Release();
}

///////////////////////////////////////////////////////////////////////

void CUIPerTradeDlg::Release()
{
	CN3UIBase::Release();

	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
	{
		if (m_pPerTradeMy[i] != nullptr)
		{
			delete m_pPerTradeMy[i];
			m_pPerTradeMy[i] = nullptr;
		}
	}

	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
	{
		if (m_pPerTradeOther[i] != nullptr)
		{
			delete m_pPerTradeOther[i];
			m_pPerTradeOther[i] = nullptr;
		}
	}

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (m_pPerTradeInv[i] != nullptr)
		{
			delete m_pPerTradeInv[i];
			m_pPerTradeInv[i] = nullptr;
		}
	}
}

void CUIPerTradeDlg::Render()
{
	if (!m_bVisible)
		return; // 보이지 않으면 자식들을 render하지 않는다.
	POINT ptCur = CGameProcedure::s_pLocalInput->MouseGetPos();
	m_pUITooltipDlg->DisplayTooltipsDisable();

	bool bTooltipRender     = false;
	__IconItemSkill* spItem = nullptr;

	for (UIListReverseItor itor = m_Children.rbegin(); m_Children.rend() != itor; ++itor)
	{
		CN3UIBase* pChild = (*itor);
		if ((GetState() == UI_STATE_ICON_MOVING) && (pChild->UIType() == UI_TYPE_ICON) && (CN3UIWndBase::s_sSelectedIconInfo.pItemSelect)
			&& ((CN3UIIcon*) pChild == CN3UIWndBase::s_sSelectedIconInfo.pItemSelect->pUIIcon))
			continue;
		pChild->Render();
		if ((GetState() == UI_STATE_COMMON_NONE) && (pChild->UIType() == UI_TYPE_ICON) && (pChild->GetStyle() & UISTYLE_ICON_HIGHLIGHT))
		{
			bTooltipRender = true;
			spItem         = GetHighlightIconItem((CN3UIIcon*) pChild);
		}
	}

	// 갯수 표시되야 할 아이템 갯수 표시..
	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
	{
		if (m_pPerTradeMy[i]
			&& ((m_pPerTradeMy[i]->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE)
				|| (m_pPerTradeMy[i]->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL)))
		{
			// string 얻기..
			CN3UIString* pStr = GetChildStringByiOrder(i);
			if (pStr)
			{
				pStr->SetVisible(true);
				pStr->SetStringAsInt(m_pPerTradeMy[i]->iCount);
				pStr->Render();
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

	// 갯수 표시되야 할 아이템 갯수 표시..
	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
	{
		if (m_pPerTradeOther[i]
			&& ((m_pPerTradeOther[i]->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE)
				|| (m_pPerTradeOther[i]->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL)))
		{
			// string 얻기..
			CN3UIString* pStr = GetChildStringByiOrder(i + 100);
			if (pStr)
			{
				pStr->SetVisible(true);
				pStr->SetStringAsInt(m_pPerTradeOther[i]->iCount);
				pStr->Render();
			}
		}
		else
		{
			// string 얻기..
			CN3UIString* pStr = GetChildStringByiOrder(i + 100);
			if (pStr)
				pStr->SetVisible(false);
		}
	}

	// 갯수 표시되야 할 아이템 갯수 표시..
	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (m_pPerTradeInv[i]
			&& ((m_pPerTradeInv[i]->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE)
				|| (m_pPerTradeInv[i]->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL)))
		{
			// string 얻기..
			CN3UIString* pStr = GetChildStringByiOrder(i + 200);
			if (pStr)
			{
				if ((GetState() == UI_STATE_ICON_MOVING) && (m_pPerTradeInv[i] == CN3UIWndBase::s_sSelectedIconInfo.pItemSelect))
				{
					pStr->SetVisible(false);
				}
				else
				{
					if (m_pPerTradeInv[i]->pUIIcon->IsVisible())
					{
						pStr->SetVisible(true);
						pStr->SetStringAsInt(m_pPerTradeInv[i]->iCount);
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
			CN3UIString* pStr = GetChildStringByiOrder(i + 200);
			if (pStr)
				pStr->SetVisible(false);
		}
	}

	if ((GetState() == UI_STATE_ICON_MOVING) && (CN3UIWndBase::s_sSelectedIconInfo.pItemSelect))
		CN3UIWndBase::s_sSelectedIconInfo.pItemSelect->pUIIcon->Render();

	if (bTooltipRender && spItem)
		m_pUITooltipDlg->DisplayTooltipsEnable(ptCur.x, ptCur.y, spItem);
}

///////////////////////////////////////////////////////////////////////

void CUIPerTradeDlg::InitIconWnd(e_UIWND eWnd)
{
	__TABLE_UI_RESRC* pTblUI = CGameBase::s_pTbl_UI.Find(CGameBase::s_pPlayer->m_InfoBase.eNation);

	m_pUITooltipDlg          = new CUIImageTooltipDlg();
	m_pUITooltipDlg->Init(this);
	m_pUITooltipDlg->LoadFromFile(pTblUI->szItemInfo);
	m_pUITooltipDlg->InitPos();
	m_pUITooltipDlg->SetVisible(FALSE);

	CN3UIWndBase::InitIconWnd(eWnd);

	// 내 결정 버튼 보통 상태로..
	std::string szFN     = "btn_trade_my";
	CN3UIButton* pButton = nullptr;
	pButton              = (CN3UIButton*) GetChildButtonByName(szFN);
	if (pButton != nullptr)
		pButton->SetState(UI_STATE_BUTTON_NORMAL);

	N3_VERIFY_UI_COMPONENT(m_pStrMyGold, GetChildByID<CN3UIString>("string_money_inv"));
	if (m_pStrMyGold)
		m_pStrMyGold->SetString("0");
}

void CUIPerTradeDlg::InitIconUpdate()
{
}

__IconItemSkill* CUIPerTradeDlg::GetHighlightIconItem(CN3UIIcon* pUIIcon)
{
	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
	{
		if ((m_pPerTradeMy[i] != nullptr) && (m_pPerTradeMy[i]->pUIIcon == pUIIcon))
			return m_pPerTradeMy[i];
	}

	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
	{
		if ((m_pPerTradeOther[i] != nullptr) && (m_pPerTradeOther[i]->pUIIcon == pUIIcon))
			return m_pPerTradeOther[i];
	}

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if ((m_pPerTradeInv[i] != nullptr) && (m_pPerTradeInv[i]->pUIIcon == pUIIcon))
			return m_pPerTradeInv[i];
	}

	return nullptr;
}

void CUIPerTradeDlg::LeavePerTradeState()
{
	// 변수 클리어..
	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
		m_iBackupiOrder[i] = -1;

	// 내 결정 버튼 보통 상태로..
	std::string szFN     = "btn_trade_my";
	CN3UIButton* pButton = nullptr;
	pButton              = (CN3UIButton*) GetChildButtonByName(szFN);
	if (pButton != nullptr)
		pButton->SetState(UI_STATE_BUTTON_NORMAL);

	ItemMoveFromThisToInv();

	if (GetState() == UI_STATE_ICON_MOVING)
		IconRestore();
	SetState(UI_STATE_COMMON_NONE);
	CN3UIWndBase::AllHighLightIconFree();
}

void CUIPerTradeDlg::EnterPerTradeState()
{
	// 변수 초기화..
	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
		m_iBackupiOrder[i] = -1;

	// 내 결정 버튼 보통 상태로..
	std::string szFN     = "btn_trade_my";
	CN3UIButton* pButton = nullptr;
	pButton              = (CN3UIButton*) GetChildButtonByName(szFN);
	if (pButton != nullptr)
		pButton->SetState(UI_STATE_BUTTON_NORMAL);

	for (int i = 0; i < MAX_ITEM_TRADE; i++)
	{
		if (m_pPerTradeMy[i] != nullptr)
		{
			if (m_pPerTradeMy[i]->pUIIcon)
			{
				RemoveChild(m_pPerTradeMy[i]->pUIIcon);
				m_pPerTradeMy[i]->pUIIcon->Release();
				delete m_pPerTradeMy[i]->pUIIcon;
				m_pPerTradeMy[i]->pUIIcon = nullptr;
			}
			delete m_pPerTradeMy[i];
			m_pPerTradeMy[i] = nullptr;
		}
	}

	for (int i = 0; i < MAX_ITEM_TRADE; i++)
	{
		if (m_pPerTradeOther[i] != nullptr)
		{
			if (m_pPerTradeOther[i]->pUIIcon)
			{
				RemoveChild(m_pPerTradeOther[i]->pUIIcon);
				m_pPerTradeOther[i]->pUIIcon->Release();
				delete m_pPerTradeOther[i]->pUIIcon;
				m_pPerTradeOther[i]->pUIIcon = nullptr;
			}
			delete m_pPerTradeOther[i];
			m_pPerTradeOther[i] = nullptr;
		}
	}

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (m_pPerTradeInv[i] != nullptr)
		{
			if (m_pPerTradeInv[i]->pUIIcon)
			{
				RemoveChild(m_pPerTradeInv[i]->pUIIcon);
				m_pPerTradeInv[i]->pUIIcon->Release();
				delete m_pPerTradeInv[i]->pUIIcon;
				m_pPerTradeInv[i]->pUIIcon = nullptr;
			}
			delete m_pPerTradeInv[i];
			m_pPerTradeInv[i] = nullptr;
		}
	}

	ItemMoveFromInvToThis();
	GoldUpdate();
}

void CUIPerTradeDlg::GoldUpdate()
{
	if (m_pStrMyGold != nullptr)
		m_pStrMyGold->SetStringAsInt(CGameBase::s_pPlayer->m_InfoExt.iGold);
}

void CUIPerTradeDlg::ItemMoveFromInvToThis()
{
	CUIInventory* pInven = CGameProcedure::s_pProcMain->m_pUIInventory;
	if (pInven == nullptr)
		return;

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (pInven->m_pMyInvWnd[i] == nullptr)
			continue;

		__IconItemSkill* spItem = pInven->m_pMyInvWnd[i];
		spItem->pUIIcon->SetParent(this);

		pInven->m_pMyInvWnd[i] = nullptr;

		CN3UIArea* pArea       = GetChildAreaByiOrder(UI_AREA_TYPE_PER_TRADE_INV, i);
		if (pArea != nullptr)
		{
			spItem->pUIIcon->SetRegion(pArea->GetRegion());
			spItem->pUIIcon->SetMoveRect(pArea->GetRegion());
		}

		m_pPerTradeInv[i] = spItem;
	}
}

void CUIPerTradeDlg::ItemMoveFromThisToInv()
{
	CUIInventory* pInven = CGameProcedure::s_pProcMain->m_pUIInventory;
	if (!pInven)
		return;

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (m_pPerTradeInv[i] == nullptr)
			continue;

		__IconItemSkill* spItem = m_pPerTradeInv[i];
		spItem->pUIIcon->SetParent(pInven);

		m_pPerTradeInv[i] = nullptr;

		CN3UIArea* pArea  = pInven->GetChildAreaByiOrder(UI_AREA_TYPE_INV, i);
		if (pArea != nullptr)
		{
			spItem->pUIIcon->SetRegion(pArea->GetRegion());
			spItem->pUIIcon->SetMoveRect(pArea->GetRegion());
		}

		pInven->m_pMyInvWnd[i] = spItem;
	}
}

void CUIPerTradeDlg::ItemCountOK()
{
	__IconItemSkill* spItem = nullptr;

	int iGold               = s_pCountableItemEdit->GetQuantity();
	spItem                  = m_pPerTradeInv[s_sRecoveryJobInfo.UIWndSourceStart.iOrder];
	if (spItem == nullptr)
		return;

	if (iGold <= 0)
		return;

	if (iGold > spItem->iCount)
		return;

	s_bWaitFromServer = true;
	m_iBackupiCount   = iGold; // 전에 옮긴 화살등의 갯수..

	int iOffset       = spItem->iCount - iGold;

	// Sound..
	if (spItem != nullptr)
		PlayItemSound(spItem->pItemBasic);

	if (iOffset > 0)
	{
		// 숫자 업데이트..
		spItem->iCount = iOffset;

		// 해당 위치에 아이콘이 존재 한다면..
		if (m_pPerTradeMy[s_sRecoveryJobInfo.UIWndSourceEnd.iOrder] != nullptr)
			m_pPerTradeMy[s_sRecoveryJobInfo.UIWndSourceEnd.iOrder]->iCount += iGold;
	}
	else
	{
		// 인벤토리의 아이콘을 삭제한다..
		spItem = m_pPerTradeInv[s_sRecoveryJobInfo.UIWndSourceStart.iOrder];

		// 해당 위치에 아이콘이 존재 한다면..
		if (m_pPerTradeMy[s_sRecoveryJobInfo.UIWndSourceEnd.iOrder] != nullptr)
			m_pPerTradeMy[s_sRecoveryJobInfo.UIWndSourceEnd.iOrder]->iCount += iGold;

		// 인벤토리에서도 지운다..
		m_pPerTradeInv[s_sRecoveryJobInfo.UIWndSourceStart.iOrder] = nullptr;

		if (spItem != nullptr)
		{
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

	// Server에게 보낸다..
	__IconItemSkill* spItemEnd = m_pPerTradeMy[CN3UIWndBase::s_sRecoveryJobInfo.UIWndSourceEnd.iOrder];
	if (spItemEnd != nullptr)
	{
		SendToServerItemAddMsg(
			CN3UIWndBase::s_sRecoveryJobInfo.UIWndSourceStart.iOrder, spItemEnd->pItemBasic->dwID + spItemEnd->pItemExt->dwID, iGold);
	}

	CN3UIWndBase::s_sRecoveryJobInfo.pItemTarget = nullptr;
	CN3UIWndBase::s_pCountableItemEdit->Close();
}

void CUIPerTradeDlg::ItemCountCancel()
{
	// Sound..
	if (CN3UIWndBase::s_sRecoveryJobInfo.pItemSource)
		PlayItemSound(CN3UIWndBase::s_sRecoveryJobInfo.pItemSource->pItemBasic);

	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
	{
		if ((m_pPerTradeMy[i])
			&& ((m_pPerTradeMy[i]->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE)
				|| (m_pPerTradeMy[i]->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL)))
		{
			if (m_pPerTradeMy[i]->iCount == 0)
			{
				// 인벤토리의 아이콘을 삭제한다..
				__IconItemSkill* spItem = m_pPerTradeMy[i];

				// 인벤토리에서도 지운다..
				m_pPerTradeMy[i]        = nullptr;

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

	// 취소..
	s_bWaitFromServer              = false;
	s_sRecoveryJobInfo.pItemSource = nullptr;
	s_sRecoveryJobInfo.pItemTarget = nullptr;

	s_pCountableItemEdit->Close();
}

void CUIPerTradeDlg::SendToServerItemAddMsg(byte pos, int itemID, int iCount)
{
	// 서버에게 전송한다..
	uint8_t byBuff[16]; // 패킷 버퍼..
	int iOffset = 0;    // 패킷 오프셋..

	// 서버에게 패킷 만들어서 날림..
	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_EXCHANGE);
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_PER_TRADE_ADD);
	CAPISocket::MP_AddByte(byBuff, iOffset, pos);
	CAPISocket::MP_AddDword(byBuff, iOffset, itemID);
	CAPISocket::MP_AddDword(byBuff, iOffset, iCount);

	CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보냄..
}

bool CUIPerTradeDlg::ReceiveIconDrop(__IconItemSkill* spItem, POINT ptCur)
{
	CN3UIArea* pArea = nullptr;
	if (!m_bVisible)
		return false;

	// 내가 가졌던 아이콘이 아니면..
	if (s_sSelectedIconInfo.UIWndSelect.UIWnd != m_eUIWnd || s_sSelectedIconInfo.UIWndSelect.UIWndDistrict != UIWND_DISTRICT_PER_TRADE_INV)
	{
		AllHighLightIconFree();
		SetState(UI_STATE_COMMON_NONE);
		return false;
	}

	// 내가 가졌던 아이콘이면.. npc영역인지 검사한다..
	int i = 0, iDestiOrder = -1;
	bool bFound = false;

	for (i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		pArea = GetChildAreaByiOrder(UI_AREA_TYPE_PER_TRADE_MY, i);
		if (pArea != nullptr && pArea->IsIn(ptCur.x, ptCur.y))
		{
			bFound = true;
			break;
		}
	}

	if (!bFound)
	{
		AllHighLightIconFree();
		SetState(UI_STATE_COMMON_NONE);
		return false;
	}

	s_bWaitFromServer                                 = true;
	m_pSubProcPerTrade->m_ePerTradeItemKindBackup     = PER_TRADE_ITEM_OTHER;

	s_sRecoveryJobInfo.pItemSource                    = s_sSelectedIconInfo.pItemSelect;
	s_sRecoveryJobInfo.UIWndSourceStart.UIWnd         = s_sSelectedIconInfo.UIWndSelect.UIWnd;
	s_sRecoveryJobInfo.UIWndSourceStart.UIWndDistrict = s_sSelectedIconInfo.UIWndSelect.UIWndDistrict;
	s_sRecoveryJobInfo.UIWndSourceStart.iOrder        = s_sSelectedIconInfo.UIWndSelect.iOrder;
	s_sRecoveryJobInfo.UIWndSourceEnd.UIWnd           = UIWND_PER_TRADE;
	s_sRecoveryJobInfo.UIWndSourceEnd.UIWndDistrict   = UIWND_DISTRICT_PER_TRADE_MY;

	bFound                                            = false;
	if ((s_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE)
		|| (s_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL))
	{
		// 활이나 물약등 아이템인 경우..
		// 면저 슬롯에 해당 아이콘이 있는지 알아본다..
		for (i = 0; i < MAX_ITEM_PER_TRADE; i++)
		{
			if (bFound)
				break;

			if (m_pPerTradeMy[i] != nullptr && (m_pPerTradeMy[i]->pItemBasic->dwID == s_sSelectedIconInfo.pItemSelect->pItemBasic->dwID)
				&& (m_pPerTradeMy[i]->pItemExt->dwID == s_sSelectedIconInfo.pItemSelect->pItemExt->dwID))
			{
				bFound      = true;
				iDestiOrder = i;
			}
		}

		// 못찾았으면..
		if (!bFound)
		{
			// 빈슬롯을 찾아 들어간다..
			for (i = 0; i < MAX_ITEM_PER_TRADE; i++)
			{
				if (!m_pPerTradeMy[i])
				{
					bFound      = true;
					iDestiOrder = i;
					break;
				}
			}

			if (!bFound) // 빈 슬롯을 찾지 못했으면..
			{
				s_bWaitFromServer              = false;
				s_sRecoveryJobInfo.pItemSource = nullptr;
				s_sRecoveryJobInfo.pItemTarget = nullptr;

				AllHighLightIconFree();
				SetState(UI_STATE_COMMON_NONE);
				return false;
			}
		}

		s_sRecoveryJobInfo.UIWndSourceEnd.iOrder = iDestiOrder;

		// 아이콘이 있는지 없는지 살펴보고..
		if (m_pPerTradeMy[iDestiOrder] == nullptr)
		{
			__IconItemSkill* spItemNew = nullptr;
			spItem                     = s_sSelectedIconInfo.pItemSelect;

			// 아이콘이 없으면 아이콘을 만드록 갯수는 0으로..
			spItemNew                  = new __IconItemSkill;
			spItemNew->pItemBasic      = spItem->pItemBasic;
			spItemNew->pItemExt        = spItem->pItemExt;
			spItemNew->szIconFN        = spItem->szIconFN; // 아이콘 파일 이름 복사..
			spItemNew->iCount          = 0;
			spItemNew->iDurability     = spItem->iDurability;

			// 아이콘 리소스 만들기..
			spItemNew->pUIIcon         = new CN3UIIcon;

			float fUVAspect            = (float) 45.0f / (float) 64.0f;
			spItemNew->pUIIcon->Init(this);
			spItemNew->pUIIcon->SetTex(spItemNew->szIconFN);
			spItemNew->pUIIcon->SetUVRect(0, 0, fUVAspect, fUVAspect);
			spItemNew->pUIIcon->SetUIType(UI_TYPE_ICON);
			spItemNew->pUIIcon->SetStyle(UISTYLE_ICON_ITEM | UISTYLE_ICON_CERTIFICATION_NEED);
			spItemNew->pUIIcon->SetVisible(true);

			CN3UIArea* pArea = GetChildAreaByiOrder(UI_AREA_TYPE_PER_TRADE_MY, s_sRecoveryJobInfo.UIWndSourceEnd.iOrder);
			if (pArea != nullptr)
			{
				spItemNew->pUIIcon->SetRegion(pArea->GetRegion());
				// 움직일 수 없다..
				RECT rect = { 0, 0, 0, 0 };
				spItemNew->pUIIcon->SetMoveRect(rect);
			}

			m_pPerTradeMy[iDestiOrder] = spItemNew;
		}

		m_iBackupiOrder[i] = CN3UIWndBase::s_sSelectedIconInfo.UIWndSelect.iOrder;

		// 활이나 물약등 아이템인 경우..
		s_bWaitFromServer  = false;
		s_pCountableItemEdit->Open(UIWND_PER_TRADE, UIWND_DISTRICT_PER_TRADE_MY, false);

		AllHighLightIconFree();
		SetState(UI_STATE_COMMON_NONE);
		return false;
	}
	else
	{
		// 아이템이 들어갈 수 있는지 확인, 아이템이 들어 가는 자리 계산..
		for (i = 0; i < MAX_ITEM_PER_TRADE; i++)
		{
			if (m_pPerTradeMy[i] == nullptr)
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			s_bWaitFromServer = false;

			// 못 찾았으므로.. 실패..
			AllHighLightIconFree();
			SetState(UI_STATE_COMMON_NONE);
			return false;
		}

		CN3UIWndBase::s_sRecoveryJobInfo.UIWndSourceEnd.iOrder = i;
		m_iBackupiOrder[i]                                     = CN3UIWndBase::s_sSelectedIconInfo.UIWndSelect.iOrder;

		// Server에게 보낸다..	현재 아이템 갯수는 1..^^
		SendToServerItemAddMsg(
			m_iBackupiOrder[i], s_sSelectedIconInfo.pItemSelect->pItemBasic->dwID + s_sSelectedIconInfo.pItemSelect->pItemExt->dwID, 1);
		s_sRecoveryJobInfo.pItemTarget = nullptr;
	}

	// 내 거래 영역에 세팅하고..
	m_pPerTradeMy[s_sRecoveryJobInfo.UIWndSourceEnd.iOrder]    = spItem;

	// 영역 윈도우에서 클리어..
	m_pPerTradeInv[s_sRecoveryJobInfo.UIWndSourceStart.iOrder] = nullptr;

	pArea = GetChildAreaByiOrder(UI_AREA_TYPE_PER_TRADE_MY, s_sRecoveryJobInfo.UIWndSourceEnd.iOrder);
	if (pArea != nullptr)
	{
		spItem->pUIIcon->SetRegion(pArea->GetRegion());
		spItem->pUIIcon->SetMoveRect(pArea->GetRegion());
	}

	AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);

	return true;
}

void CUIPerTradeDlg::CancelIconDrop(__IconItemSkill* /*spItem*/)
{
	AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);
}

void CUIPerTradeDlg::AcceptIconDrop(__IconItemSkill* /*spItem*/)
{
	AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);
}

void CUIPerTradeDlg::IconRestore()
{
	CN3UIArea* pArea = nullptr;

	switch (s_sSelectedIconInfo.UIWndSelect.UIWndDistrict)
	{
		case UIWND_DISTRICT_PER_TRADE_INV:
			if (m_pPerTradeInv[s_sSelectedIconInfo.UIWndSelect.iOrder] != nullptr)
			{
				pArea = GetChildAreaByiOrder(UI_AREA_TYPE_PER_TRADE_INV, s_sSelectedIconInfo.UIWndSelect.iOrder);
				if (pArea != nullptr)
				{
					m_pPerTradeInv[s_sSelectedIconInfo.UIWndSelect.iOrder]->pUIIcon->SetRegion(pArea->GetRegion());
					m_pPerTradeInv[s_sSelectedIconInfo.UIWndSelect.iOrder]->pUIIcon->SetMoveRect(pArea->GetRegion());
				}
			}
			break;

		default:
			break;
	}
}

uint32_t CUIPerTradeDlg::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (!IsVisible() || s_bWaitFromServer)
	{
		// NOLINTNEXTLINE(bugprone-parent-virtual-call)
		dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);
		return dwRet;
	}

	// 드래그 되는 아이콘 갱신..
	if ((GetState() == UI_STATE_ICON_MOVING) && (CN3UIWndBase::s_sSelectedIconInfo.UIWndSelect.UIWnd == UIWND_PER_TRADE))
	{
		CN3UIWndBase::s_sSelectedIconInfo.pItemSelect->pUIIcon->SetRegion(GetSampleRect());
		CN3UIWndBase::s_sSelectedIconInfo.pItemSelect->pUIIcon->SetMoveRect(GetSampleRect());
	}

	return CN3UIWndBase::MouseProc(dwFlags, ptCur, ptOld);
}

int CUIPerTradeDlg::GetItemiOrder(__IconItemSkill* spItem, e_UIWND_DISTRICT eWndDist)
{
	if (eWndDist == UIWND_DISTRICT_PER_TRADE_INV)
	{
		for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
		{
			if (m_pPerTradeInv[i] != nullptr && m_pPerTradeInv[i] == spItem)
				return i;
		}
	}

	return -1;
}

RECT CUIPerTradeDlg::GetSampleRect()
{
	CN3UIArea* pArea = GetChildAreaByiOrder(UI_AREA_TYPE_PER_TRADE_MY, 0);
	if (pArea == nullptr)
		return {};

	POINT ptCur    = CGameProcedure::s_pLocalInput->MouseGetPos();
	RECT rect      = pArea->GetRegion();
	float fWidth   = (float) (rect.right - rect.left);
	float fHeight  = (float) (rect.bottom - rect.top);
	fWidth        *= 0.5f;
	fHeight       *= 0.5f;
	rect.left      = ptCur.x - (int) fWidth;
	rect.right     = ptCur.x + (int) fWidth;
	rect.top       = ptCur.y - (int) fHeight;
	rect.bottom    = ptCur.y + (int) fHeight;
	return rect;
}

bool CUIPerTradeDlg::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (pSender == nullptr)
		return false;

	if ((dwMsg == UIMSG_BUTTON_CLICK) && (pSender->m_szID == "btn_close") && (m_pSubProcPerTrade != nullptr)
		&& ((m_pSubProcPerTrade->m_ePerTradeState == PER_TRADE_STATE_NORMAL)
			|| (m_pSubProcPerTrade->m_ePerTradeState == PER_TRADE_STATE_MY_TRADE_DECISION_DONE)))
		// 정상 상태와 내가 거래 결정 버튼을 누른 상태에서만 취소할 수 있다..
		m_pSubProcPerTrade->LeavePerTradeState(PER_TRADE_RESULT_MY_CANCEL);

	if ((dwMsg == UIMSG_BUTTON_CLICK) && (pSender->m_szID == "btn_trade_my") && (m_pSubProcPerTrade != nullptr)
		&& (m_pSubProcPerTrade->m_ePerTradeState == PER_TRADE_STATE_NORMAL)) // 정상 상태에서만 결정할 수 있다..
		m_pSubProcPerTrade->PerTradeMyDecision();

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender->m_szID == "btn_gold")
		{
			// 인벤토리만 떠 있을때..
			CGameProcedure::s_pProcMain->m_pSubProcPerTrade->RequestItemCountEdit();
		}
	}

	__IconItemSkill* spItem = nullptr;
	int iOrder              = -1;

	uint32_t dwBitMask      = 0x000f0000;

	switch (dwMsg & dwBitMask)
	{
		case UIMSG_ICON_DOWN_FIRST:
			//..
			AllHighLightIconFree();

			// Get Item..
			spItem                                        = GetHighlightIconItem((CN3UIIcon*) pSender);

			// Save Select Info..
			s_sSelectedIconInfo.UIWndSelect.UIWnd         = UIWND_PER_TRADE;
			s_sSelectedIconInfo.UIWndSelect.UIWndDistrict = UIWND_DISTRICT_PER_TRADE_INV;
			iOrder                                        = GetItemiOrder(spItem, UIWND_DISTRICT_PER_TRADE_INV);
			if (iOrder == -1)
			{
				SetState(UI_STATE_COMMON_NONE);
				return false;
			}

			s_sSelectedIconInfo.UIWndSelect.iOrder = iOrder;
			s_sSelectedIconInfo.pItemSelect        = spItem;

			// Do Ops..
			((CN3UIIcon*) pSender)->SetRegion(GetSampleRect());
			((CN3UIIcon*) pSender)->SetMoveRect(GetSampleRect());

			// Sound..
			if (spItem != nullptr)
				PlayItemSound(spItem->pItemBasic);
			break;

		case UIMSG_ICON_DOWN:
			if (GetState() == UI_STATE_ICON_MOVING)
			{
				s_sSelectedIconInfo.pItemSelect->pUIIcon->SetRegion(GetSampleRect());
				s_sSelectedIconInfo.pItemSelect->pUIIcon->SetMoveRect(GetSampleRect());
			}
			break;

		case UIMSG_ICON_UP:
			// 아이콘 매니저 윈도우들을 돌아 다니면서 검사..
			if (!CGameProcedure::s_pUIMgr->BroadcastIconDropMsg(s_sSelectedIconInfo.pItemSelect))
				// 아이콘 위치 원래대로..
				IconRestore();

			// Sound..
			if (s_sSelectedIconInfo.pItemSelect != nullptr)
				PlayItemSound(s_sSelectedIconInfo.pItemSelect->pItemBasic);
			break;

		default:
			break;
	}

	return true;
}

CN3UIBase* CUIPerTradeDlg::GetChildButtonByName(const std::string& szFN)
{
	for (UIListItor itor = m_Children.begin(); m_Children.end() != itor; ++itor)
	{
		CN3UIBase* pChild = (CN3UIBase*) (*itor);
		if ((pChild->UIType() == UI_TYPE_BUTTON) && (szFN.compare(pChild->m_szID) == 0))
			return pChild;
	}

	return nullptr;
}


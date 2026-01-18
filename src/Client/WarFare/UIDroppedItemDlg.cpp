// UIDroppedItemDlg.cpp: implementation of the UIDroppedItemDlg class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "UIDroppedItemDlg.h"
#include "PacketDef.h"
#include "LocalInput.h"
#include "APISocket.h"
#include "GameProcMain.h"
#include "PlayerMySelf.h"
#include "N3UIWndBase.h"
#include "UIImageTooltipDlg.h"
#include "UIInventory.h"
#include "UITransactionDlg.h"
#include "SubProcPerTrade.h"
#include "PlayerOtherMgr.h"
#include "PlayerNPC.h"
#include "UIHotKeyDlg.h"
#include "UISkillTreeDlg.h"
#include "text_resources.h"

#include <N3Base/N3UIArea.h>
#include <N3Base/N3UIString.h>

CUIDroppedItemDlg::CUIDroppedItemDlg()
{
	for (int i = 0; i < MAX_ITEM_BUNDLE_DROP_PIECE; i++)
	{
		m_pMyDroppedItem[i]   = nullptr;
		m_bSendedIconArray[i] = false;
	}

	m_iItemBundleID = 0;
	m_pUITooltipDlg = nullptr;
	m_iBackupiOrder = 0;
}

CUIDroppedItemDlg::~CUIDroppedItemDlg()
{
	CUIDroppedItemDlg::Release();
}

void CUIDroppedItemDlg::Release()
{
	CN3UIWndBase::Release();

	for (int i = 0; i < MAX_ITEM_BUNDLE_DROP_PIECE; i++)
	{
		delete m_pMyDroppedItem[i];
		m_pMyDroppedItem[i] = nullptr;
	}
}

void CUIDroppedItemDlg::Render()
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
		pChild->Render();

		if (pChild->UIType() == UI_TYPE_ICON && (pChild->GetStyle() & UISTYLE_ICON_HIGHLIGHT))
		{
			bTooltipRender = true;
			spItem         = GetHighlightIconItem((CN3UIIcon*) pChild);
		}
	}

	if (bTooltipRender)
		m_pUITooltipDlg->DisplayTooltipsEnable(ptCur.x, ptCur.y, spItem);

	// 갯수 표시되야 할 아이템 갯수 표시..
	CN3UIString* pStr = nullptr;
	for (int i = 0; i < MAX_ITEM_BUNDLE_DROP_PIECE; i++)
	{
		if (m_pMyDroppedItem[i] != nullptr
			&& ((m_pMyDroppedItem[i]->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE)
				|| (m_pMyDroppedItem[i]->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL)))
		{
			// string 얻기..
			pStr = GetChildStringByiOrder(i);
			if (pStr != nullptr)
			{
				if ((GetState() != UI_STATE_ICON_MOVING || m_pMyDroppedItem[i] != s_sSelectedIconInfo.pItemSelect)
					&& m_pMyDroppedItem[i]->pUIIcon->IsVisible())
				{
					pStr->SetVisible(true);
					pStr->SetStringAsInt(m_pMyDroppedItem[i]->iCount);
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
			pStr = GetChildStringByiOrder(i);
			if (pStr != nullptr)
				pStr->SetVisible(false);
		}
	}
}

void CUIDroppedItemDlg::InitIconWnd(e_UIWND eWnd)
{
	__TABLE_UI_RESRC* pTblUI = CGameBase::s_pTbl_UI.Find(CGameBase::s_pPlayer->m_InfoBase.eNation);

	m_pUITooltipDlg          = new CUIImageTooltipDlg();
	m_pUITooltipDlg->Init(this);
	m_pUITooltipDlg->LoadFromFile(pTblUI->szItemInfo);
	m_pUITooltipDlg->InitPos();
	m_pUITooltipDlg->SetVisible(false);

	CN3UIWndBase::InitIconWnd(eWnd);
}

void CUIDroppedItemDlg::InitIconUpdate()
{
	float fUVAspect = 45.0f / 64.0f;

	for (int i = 0; i < MAX_ITEM_BUNDLE_DROP_PIECE; i++)
	{
		if (m_pMyDroppedItem[i] != nullptr)
		{
			m_pMyDroppedItem[i]->pUIIcon = new CN3UIIcon();
			m_pMyDroppedItem[i]->pUIIcon->Init(this);
			m_pMyDroppedItem[i]->pUIIcon->SetTex(m_pMyDroppedItem[i]->szIconFN);
			m_pMyDroppedItem[i]->pUIIcon->SetUVRect(0, 0, fUVAspect, fUVAspect);
			m_pMyDroppedItem[i]->pUIIcon->SetUIType(UI_TYPE_ICON);
			m_pMyDroppedItem[i]->pUIIcon->SetStyle(UISTYLE_ICON_ITEM | UISTYLE_ICON_CERTIFICATION_NEED);

			CN3UIArea* pArea = GetChildAreaByiOrder(UI_AREA_TYPE_DROP_ITEM, i);
			if (pArea != nullptr)
			{
				m_pMyDroppedItem[i]->pUIIcon->SetRegion(pArea->GetRegion());
				m_pMyDroppedItem[i]->pUIIcon->SetMoveRect(pArea->GetRegion());
			}
		}
	}
}

__IconItemSkill* CUIDroppedItemDlg::GetHighlightIconItem(CN3UIIcon* pUIIcon)
{
	for (int i = 0; i < MAX_ITEM_BUNDLE_DROP_PIECE; i++)
	{
		if (m_pMyDroppedItem[i] != nullptr && m_pMyDroppedItem[i]->pUIIcon == pUIIcon)
			return m_pMyDroppedItem[i];
	}

	return nullptr;
}

void CUIDroppedItemDlg::EnterDroppedState(int xpos, int ypos)
{
	if (!IsVisible())
		SetVisible(true);

	SetPos(xpos, ypos - 150);

	for (int i = 0; i < MAX_ITEM_BUNDLE_DROP_PIECE; i++)
	{
		m_bSendedIconArray[i] = false;

		if (m_pMyDroppedItem[i] != nullptr)
		{
			if (m_pMyDroppedItem[i]->pUIIcon)
			{
				RemoveChild(m_pMyDroppedItem[i]->pUIIcon);
				m_pMyDroppedItem[i]->pUIIcon->Release();
				delete m_pMyDroppedItem[i]->pUIIcon;
				m_pMyDroppedItem[i]->pUIIcon = nullptr;
			}

			delete m_pMyDroppedItem[i];
			m_pMyDroppedItem[i] = nullptr;
		}
	}

	AllHighLightIconFree();
}

void CUIDroppedItemDlg::LeaveDroppedState()
{
	if (IsVisible())
		SetVisible(false);

	for (int i = 0; i < MAX_ITEM_BUNDLE_DROP_PIECE; i++)
		m_bSendedIconArray[i] = false;
}

uint32_t CUIDroppedItemDlg::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (!IsVisible())
		return dwRet;

	// NOLINTNEXTLINE(bugprone-parent-virtual-call)
	dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);
	return dwRet;
}

void CUIDroppedItemDlg::AddToItemTable(int iItemID, int iItemCount, int iOrder)
{
	__IconItemSkill* spItem    = nullptr;
	__TABLE_ITEM_BASIC* pItem  = nullptr;                              // 아이템 테이블 구조체 포인터..
	__TABLE_ITEM_EXT* pItemExt = nullptr;                              // 아이템 테이블 구조체 포인터..
	std::string szIconFN;

	pItem = CGameBase::s_pTbl_Items_Basic.Find(iItemID / 1000 * 1000); // 열 데이터 얻기..
	if (pItem != nullptr && pItem->byExtIndex >= 0 && pItem->byExtIndex < MAX_ITEM_EXTENSION)
		pItemExt = CGameBase::s_pTbl_Items_Exts[pItem->byExtIndex].Find(iItemID % 1000);
	if (pItem == nullptr || pItemExt == nullptr)
	{
		__ASSERT(0, "아이템 포인터 테이블에 없음!!");
		CLogWriter::Write("CUIDroppedItemDlg::AddToItemTable - Invalidate ItemID : {}", iItemID);
		return;
	}

	e_PartPosition ePart = PART_POS_UNKNOWN;
	e_PlugPosition ePlug = PLUG_POS_UNKNOWN;

	// 아이템에 따른 파일 이름을 만들어서
	e_ItemType eType     = CGameBase::MakeResrcFileNameForUPC(pItem, pItemExt, nullptr, &szIconFN, ePart, ePlug, RACE_UNKNOWN);
	if (ITEM_TYPE_UNKNOWN == eType)
		return;

	spItem                   = new __IconItemSkill();
	spItem->pItemBasic       = pItem;
	spItem->pItemExt         = pItemExt;
	spItem->szIconFN         = szIconFN; // 아이콘 파일 이름 복사..
	spItem->iCount           = iItemCount;
	spItem->iDurability      = pItem->siMaxDurability + pItemExt->siMaxDurability;

	m_pMyDroppedItem[iOrder] = spItem;
}

void CUIDroppedItemDlg::AddToItemTableToInventory(int iItemID, int iItemCount, int iOrder)
{
	__IconItemSkill* spItem    = nullptr;
	// 아이템 테이블 구조체 포인터..
	__TABLE_ITEM_BASIC* pItem  = nullptr;
	// 아이템 테이블 구조체 포인터..
	__TABLE_ITEM_EXT* pItemExt = nullptr;
	std::string szIconFN;
	float fUVAspect = 45.0f / 64.0f;

	pItem           = CGameBase::s_pTbl_Items_Basic.Find(iItemID / 1000 * 1000); // 열 데이터 얻기..
	if (pItem != nullptr && pItem->byExtIndex >= 0 && pItem->byExtIndex < MAX_ITEM_EXTENSION)
		pItemExt = CGameBase::s_pTbl_Items_Exts[pItem->byExtIndex].Find(iItemID % 1000);

	if (pItem == nullptr || pItemExt == nullptr)
	{
		__ASSERT(0, "아이템 포인터 테이블에 없음!!");
		CLogWriter::Write("CUIDroppedItemDlg::AddToItemTableToInventory - Invalidate ItemID : {}", iItemID);
		return;
	}

	e_PartPosition ePart = PART_POS_UNKNOWN;
	e_PlugPosition ePlug = PLUG_POS_UNKNOWN;
	// 아이템에 따른 파일 이름을 만들어서
	e_ItemType eType     = CGameBase::MakeResrcFileNameForUPC(pItem, pItemExt, nullptr, &szIconFN, ePart, ePlug);
	if (ITEM_TYPE_UNKNOWN == eType)
		return;

	spItem              = new __IconItemSkill();
	spItem->pItemBasic  = pItem;
	spItem->pItemExt    = pItemExt;
	spItem->szIconFN    = szIconFN; // 아이콘 파일 이름 복사..
	spItem->iCount      = iItemCount;
	spItem->iDurability = pItem->siMaxDurability + pItemExt->siMaxDurability;
	spItem->pUIIcon     = new CN3UIIcon();

	spItem->pUIIcon->Init(CGameProcedure::s_pProcMain->m_pUIInventory);
	spItem->pUIIcon->SetTex(spItem->szIconFN);
	spItem->pUIIcon->SetUVRect(0, 0, fUVAspect, fUVAspect);
	spItem->pUIIcon->SetUIType(UI_TYPE_ICON);
	spItem->pUIIcon->SetStyle(UISTYLE_ICON_ITEM | UISTYLE_ICON_CERTIFICATION_NEED);
	spItem->pUIIcon->SetVisible(true);

	CN3UIArea* pArea = CGameProcedure::s_pProcMain->m_pUIInventory->GetChildAreaByiOrder(UI_AREA_TYPE_INV, iOrder);
	if (pArea != nullptr)
	{
		spItem->pUIIcon->SetRegion(pArea->GetRegion());
		spItem->pUIIcon->SetMoveRect(pArea->GetRegion());
	}

	CGameProcedure::s_pProcMain->m_pUIInventory->m_pMyInvWnd[iOrder] = spItem;
	PlayItemSound(pItem);
}

bool CUIDroppedItemDlg::ReceiveIconDrop(__IconItemSkill* spItem, POINT /*ptCur*/)
{
	if (!m_bVisible)
		return false;

	// 검사해서 선택된 아이콘을 가진 윈도우에게 결과를 알려줘야 한다..
	switch (s_sSelectedIconInfo.UIWndSelect.UIWnd)
	{
		// 인벤토리 윈도우로부터 온 것이라면..
		case UIWND_INVENTORY:
			CGameProcedure::s_pProcMain->m_pUIInventory->CancelIconDrop(spItem);
			break;

		// 상거래 윈도우로부터 온 것이라면...
		case UIWND_TRANSACTION:
			CGameProcedure::s_pProcMain->m_pUITransactionDlg->CancelIconDrop(spItem);
			break;

		default:
			break;
	}

	return false;
}

int CUIDroppedItemDlg::GetInventoryEmptyInviOrder(__IconItemSkill* spItem)
{
	if (spItem == nullptr)
	{
		for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
		{
			__IconItemSkill* spItemInv = CGameProcedure::s_pProcMain->m_pUIInventory->m_pMyInvWnd[i];
			if (spItemInv == nullptr)
				return i;
		}
	}
	else
	{
		for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
		{
			__IconItemSkill* spItemInv = CGameProcedure::s_pProcMain->m_pUIInventory->m_pMyInvWnd[i];
			if (spItemInv != nullptr && spItemInv->pItemBasic->dwID == spItem->pItemBasic->dwID)
				return i;
		}

		for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
		{
			__IconItemSkill* spItemInv = CGameProcedure::s_pProcMain->m_pUIInventory->m_pMyInvWnd[i];
			if (spItemInv == nullptr)
				return i;
		}
	}

	return -1;
}

int CUIDroppedItemDlg::GetItemiOrder(__IconItemSkill* spItem)
{
	for (int i = 0; i < MAX_ITEM_BUNDLE_DROP_PIECE; i++)
	{
		if (m_pMyDroppedItem[i] != nullptr && m_pMyDroppedItem[i] == spItem)
			return i;
	}

	return -1;
}

bool CUIDroppedItemDlg::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	// Code Begin
	if (pSender == nullptr)
		return false;

	int iOrder = -1, iOrderInv = -1;

	uint32_t dwBitMask        = 0x000f0000;

	__TABLE_ITEM_BASIC* pItem = nullptr;
	__IconItemSkill* spItem   = nullptr;
	std::string szIconFN;
	e_PartPosition ePart = PART_POS_UNKNOWN;
	e_PlugPosition ePlug = PLUG_POS_UNKNOWN;
	e_ItemType eType     = ITEM_TYPE_UNKNOWN;

	// 서버에 보내지 않은 아이템이니까.. 서버에 보낸다..
	uint8_t byBuff[16];
	int iOffset = 0;

	switch (dwMsg & dwBitMask)
	{
		case UIMSG_ICON_DOWN_FIRST:
			spItem = GetHighlightIconItem((CN3UIIcon*) pSender);
			if (spItem == nullptr)
				break;

			m_iBackupiOrder = GetItemiOrder(spItem);
			break;

		case UIMSG_ICON_UP:
			SetState(UI_STATE_COMMON_NONE);

			// 아이템이 돈인지 검사..
			pItem  = nullptr; // 아이템 테이블 구조체 포인터..
			spItem = GetHighlightIconItem((CN3UIIcon*) pSender);
			if (spItem == nullptr)
				break;

			pItem = CGameBase::s_pTbl_Items_Basic.Find(spItem->pItemBasic->dwID); // 열 데이터 얻기..
			if (pItem == nullptr)
			{
				__ASSERT(0, "NULL Item!!!");
				CLogWriter::Write("CUIDroppedItemDlg::ReceiveMessage - UIMSG_ICON_UP - NULL Icon : {}", spItem->pItemBasic->dwID);
				break;
			}

			// 아이템에 따른 파일 이름을 만들어서
			eType  = CGameBase::MakeResrcFileNameForUPC(pItem, spItem->pItemExt, nullptr, &szIconFN, ePart, ePlug);

			// 보낸 아이콘 배열이랑 비교..
			iOrder = GetItemiOrder(spItem);
			if (m_iBackupiOrder != iOrder)
				break;

			// 한번 보냈던 패킷이면 break..
			if (m_bSendedIconArray[iOrder])
				break;

			m_bSendedIconArray[iOrder] = true;

			CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_ITEM_GET);
			CAPISocket::MP_AddDword(byBuff, iOffset, m_iItemBundleID);

			// 돈이 아니면 인벤토리 리스트에 추가....
			if (ITEM_TYPE_GOLD != eType)
				CAPISocket::MP_AddDword(byBuff, iOffset, spItem->pItemBasic->dwID + spItem->pItemExt->dwID);
			else
				CAPISocket::MP_AddDword(byBuff, iOffset, spItem->pItemBasic->dwID);

			CGameProcedure::s_pSocket->Send(byBuff, iOffset);

			// 보낸 아이콘 정보 셋팅..
			s_sRecoveryJobInfo.pItemSource                    = spItem;
			s_sRecoveryJobInfo.UIWndSourceStart.UIWnd         = UIWND_DROPITEM;
			s_sRecoveryJobInfo.UIWndSourceStart.UIWndDistrict = UIWND_DISTRICT_DROPITEM;
			s_sRecoveryJobInfo.UIWndSourceStart.iOrder        = iOrder;
			s_sRecoveryJobInfo.UIWndSourceEnd.UIWnd           = UIWND_INVENTORY;
			s_sRecoveryJobInfo.UIWndSourceEnd.UIWndDistrict   = UIWND_DISTRICT_INVENTORY_INV;
			s_sRecoveryJobInfo.UIWndSourceEnd.iOrder          = iOrderInv;
			break;

		case UIMSG_ICON_DOWN:
			break;

		case UIMSG_ICON_DBLCLK:
			SetState(UI_STATE_COMMON_NONE);
			break;

		default:
			break;
	}

	return true;
}

void CUIDroppedItemDlg::GetItemByIDToInventory(
	uint8_t bResult, int iItemID, int iGold, int iPos, int iItemCount, const std::string& characterName)
{
	// 아이템 리스트에서 아이템을 찾고..
	bool bFound                  = false;
	// 아이템 테이블 구조체 포인터..
	__TABLE_ITEM_BASIC* pItem    = nullptr;
	__TABLE_ITEM_EXT* pItemExt   = nullptr;
	__IconItemSkill* spItem      = nullptr;
	int i                        = 0;
	__InfoPlayerMySelf* pInfoExt = nullptr;
	std::string stdMsg;

	// 실패..
	if (bResult == 0)
	{
		int iOrderInv = GetInventoryEmptyInviOrder();
		if (iOrderInv == -1)
		{
			// 인벤토리가 꽉 차있으면.. break.. ^^
			stdMsg = fmt::format_text_resource(IDS_INV_ITEM_FULL);
			CGameProcedure::s_pProcMain->MsgOutput(stdMsg, 0xff9b9bff);
		}

		if (CGameProcedure::s_pProcMain->m_pUISkillTreeDlg != nullptr)
			CGameProcedure::s_pProcMain->m_pUISkillTreeDlg->UpdateDisableCheck();

		if (CGameProcedure::s_pProcMain->m_pUIHotKeyDlg != nullptr)
			CGameProcedure::s_pProcMain->m_pUIHotKeyDlg->UpdateDisableCheck();

		return;
	}

	// 파티 상태에서 노아를 얻는다..
	if (bResult == 2)
	{
		// 돈 갱신..
		pInfoExt = &CGameBase::s_pPlayer->m_InfoExt;

		// 돈 업데이트..
		stdMsg   = fmt::format_text_resource(IDS_DROPPED_NOAH_GET, iGold - pInfoExt->iGold);
		CGameProcedure::s_pProcMain->MsgOutput(stdMsg, 0xff9b9bff);

		pInfoExt->iGold = iGold;
		CGameProcedure::s_pProcMain->m_pUIInventory->GoldUpdate();

		if (!IsVisible())
			return;

		// 돈 아이콘이 있으면 없앤다..
		bFound = false;
		for (i = 0; i < MAX_ITEM_BUNDLE_DROP_PIECE; i++)
		{
			if (m_pMyDroppedItem[i] != nullptr && m_pMyDroppedItem[i]->pItemBasic->dwID == dwGold)
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)
			return;

		spItem = m_pMyDroppedItem[i];
		if (spItem == nullptr)
			return;

		// 매니저에서 제거..
		RemoveChild(spItem->pUIIcon);

		// 리소스 제거..
		spItem->pUIIcon->Release();
		delete spItem->pUIIcon;
		spItem->pUIIcon = nullptr;
		delete spItem;
		spItem              = nullptr;
		m_pMyDroppedItem[i] = nullptr;

		PlayGoldSound();

		if (CGameProcedure::s_pProcMain->m_pUISkillTreeDlg != nullptr)
			CGameProcedure::s_pProcMain->m_pUISkillTreeDlg->UpdateDisableCheck();

		if (CGameProcedure::s_pProcMain->m_pUIHotKeyDlg != nullptr)
			CGameProcedure::s_pProcMain->m_pUIHotKeyDlg->UpdateDisableCheck();
	}

	// 파티상태에서 내가 아이템을 습득..
	if (bResult == 3)
	{
		pItem    = CGameBase::s_pTbl_Items_Basic.Find(iItemID / 1000 * 1000); // 열 데이터 얻기..
		pItemExt = nullptr;

		if (pItem != nullptr && pItem->byExtIndex >= 0 && pItem->byExtIndex < MAX_ITEM_EXTENSION)
			pItemExt = CGameBase::s_pTbl_Items_Exts[pItem->byExtIndex].Find(iItemID % 1000);

		if (pItem == nullptr || pItemExt == nullptr)
		{
			__ASSERT(0, "아이템 포인터 테이블에 없음!!");
			CLogWriter::Write("CUIDroppedItemDlg::GetItemByIDToInventory - NULL Icon : {}", iItemID);
			return;
		}

		std::string szMsg = fmt::format_text_resource(IDS_PARTY_ITEM_GET, characterName, pItem->szName);
		CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xff9b9bff);

		if (!IsVisible())
			return;

		// 아이템 아이콘이 있으면 없앤다..
		bFound = false;
		for (i = 0; i < MAX_ITEM_BUNDLE_DROP_PIECE; i++)
		{
			if (m_pMyDroppedItem[i] != nullptr && (m_pMyDroppedItem[i]->pItemBasic->dwID + m_pMyDroppedItem[i]->pItemExt->dwID == iItemID))
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)
			return;

		if (m_pMyDroppedItem[i] != nullptr)
			PlayItemSound(m_pMyDroppedItem[i]->pItemBasic);

		spItem = m_pMyDroppedItem[i];
		if (spItem == nullptr)
			return;

		// 매니저에서 제거..
		RemoveChild(spItem->pUIIcon);

		// 리소스 제거..
		spItem->pUIIcon->Release();
		delete spItem->pUIIcon;
		spItem->pUIIcon = nullptr;
		delete spItem;
		spItem              = nullptr;
		m_pMyDroppedItem[i] = nullptr;

		if (CGameProcedure::s_pProcMain->m_pUISkillTreeDlg != nullptr)
			CGameProcedure::s_pProcMain->m_pUISkillTreeDlg->UpdateDisableCheck();

		if (CGameProcedure::s_pProcMain->m_pUIHotKeyDlg != nullptr)
			CGameProcedure::s_pProcMain->m_pUIHotKeyDlg->UpdateDisableCheck();

		return;
	}

	// 파티 상태에서 다른 멤버가 아이템을 습득..
	if (bResult == 4)
	{
		spItem = m_pMyDroppedItem[s_sRecoveryJobInfo.UIWndSourceStart.iOrder];
		if (spItem != nullptr)
		{
			// 매니저에서 제거..
			RemoveChild(spItem->pUIIcon);

			// 리소스 제거..
			spItem->pUIIcon->Release();
			delete spItem->pUIIcon;
			spItem->pUIIcon = nullptr;
			delete spItem;
			spItem = nullptr;
		}

		m_pMyDroppedItem[s_sRecoveryJobInfo.UIWndSourceStart.iOrder] = nullptr;

		if (CGameProcedure::s_pProcMain->m_pUISkillTreeDlg != nullptr)
			CGameProcedure::s_pProcMain->m_pUISkillTreeDlg->UpdateDisableCheck();

		if (CGameProcedure::s_pProcMain->m_pUIHotKeyDlg)
			CGameProcedure::s_pProcMain->m_pUIHotKeyDlg->UpdateDisableCheck();

		return;
	}

	// 파티 상태에서 일반적인 아이템 습득..
	if (bResult == 5)
	{
		if (iItemID == dwGold)
		{
			__ASSERT(0, "Invalidate Item ID From Server.. ");
			CLogWriter::Write("CUIDroppedItemDlg::GetItemByIDToInventory - ID Pos : {}", iPos);
			return;
		}

		if (iPos < 0 || iPos >= MAX_ITEM_INVENTORY)
		{
			__ASSERT(0, "Invalidate Item Pos From Server.. ");
			CLogWriter::Write("CUIDroppedItemDlg::GetItemByIDToInventory - Invalidate Pos : {}", iPos);
			return;
		}

		__IconItemSkill* spItemDest = CGameProcedure::s_pProcMain->m_pUIInventory->m_pMyInvWnd[iPos];

		// 아이템이 있다..
		if (spItemDest != nullptr)
		{
			if (iItemID != (spItemDest->pItemBasic->dwID + spItemDest->pItemExt->dwID))
			{
				// 기존 이이템을 클리어..
				if (spItemDest == nullptr)
					return;

				RemoveChild(spItemDest->pUIIcon);

				// 아이콘 리소스 삭제...
				spItemDest->pUIIcon->Release();
				delete spItemDest->pUIIcon;
				spItemDest->pUIIcon = nullptr;
				delete spItemDest;
				spItemDest                                                     = nullptr;

				CGameProcedure::s_pProcMain->m_pUIInventory->m_pMyInvWnd[iPos] = nullptr;

				//  아이템을 새로 만듬.. 갯수 셋팅..
				AddToItemTableToInventory(iItemID, iItemCount, iPos);
			}
			else
			{
				// 갯수 셋팅..
				CGameProcedure::s_pProcMain->m_pUIInventory->m_pMyInvWnd[iPos]->iCount = iItemCount;
				PlayItemSound(CGameProcedure::s_pProcMain->m_pUIInventory->m_pMyInvWnd[iPos]->pItemBasic);
			}
		}
		else
		{
			// 아이템이 없는 경우 .. 새로 만든다.. 갯수 셋팅..
			AddToItemTableToInventory(iItemID, iItemCount, iPos);
		}

		// 열 데이터 얻기..
		pItem = CGameBase::s_pTbl_Items_Basic.Find(iItemID / 1000 * 1000);
		if (pItem == nullptr)
		{
			__ASSERT(0, "아이템 포인터 테이블에 없음!!");
			CLogWriter::Write("CUIDroppedItemDlg::GetItemByIDToInventory - NULL Icon : {}", iItemID);
			return;
		}

		stdMsg = fmt::format_text_resource(IDS_ITEM_GET_BY_RULE, pItem->szName);
		CGameProcedure::s_pProcMain->MsgOutput(stdMsg, 0xff9b9bff);

		if (CGameProcedure::s_pProcMain->m_pUISkillTreeDlg != nullptr)
			CGameProcedure::s_pProcMain->m_pUISkillTreeDlg->UpdateDisableCheck();

		if (CGameProcedure::s_pProcMain->m_pUIHotKeyDlg != nullptr)
			CGameProcedure::s_pProcMain->m_pUIHotKeyDlg->UpdateDisableCheck();
	}

	if (bResult == 6)
	{
		// 메시지 박스 텍스트 표시..
		std::string szMsg = fmt::format_text_resource(IDS_ITEM_TOOMANY_OR_HEAVY);
		CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);
	}

	if (bResult == 7)
	{
		// 메시지 박스 텍스트 표시..
		std::string szMsg = fmt::format_text_resource(IDS_INV_ITEM_FULL);
		CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);
	}

	if (bResult == 1)
	{
		if (iItemID != dwGold)
		{
			pItem = CGameBase::s_pTbl_Items_Basic.Find(iItemID / 1000 * 1000);
			if (pItem == nullptr)
			{
				__ASSERT(0, "Invalidate Item ID From Server.. ");
				CLogWriter::Write("CUIDroppedItemDlg::GetItemByIDToInventory - Invalidate Item ID : {}", iItemID);
				return;
			}

			if (iPos < 0 || iPos >= MAX_ITEM_INVENTORY)
			{
				__ASSERT(0, "Invalidate Item Pos From Server.. ");
				CLogWriter::Write("CUIDroppedItemDlg::GetItemByIDToInventory - Invalidate Pos : {}", iPos);
				return;
			}

			spItem                      = nullptr;
			__IconItemSkill* spItemDest = CGameProcedure::s_pProcMain->m_pUIInventory->m_pMyInvWnd[iPos];
			// 아이템이 있다..
			if (spItemDest != nullptr)
			{
				if (iItemID != (spItemDest->pItemBasic->dwID + spItemDest->pItemExt->dwID))
				{
					// 기존 이이템을 클리어..
					if (spItemDest == nullptr)
						return;

					RemoveChild(spItemDest->pUIIcon);

					// 아이콘 리소스 삭제...
					spItemDest->pUIIcon->Release();
					delete spItemDest->pUIIcon;
					spItemDest->pUIIcon = nullptr;
					delete spItemDest;
					spItemDest                                                     = nullptr;

					CGameProcedure::s_pProcMain->m_pUIInventory->m_pMyInvWnd[iPos] = nullptr;

					//  아이템을 새로 만듬.. 갯수 셋팅..
					AddToItemTableToInventory(iItemID, iItemCount, iPos);
				}
				else
				{
					// 갯수 셋팅..
					// Picking up countable item which the user already have
					CGameProcedure::s_pProcMain->m_pUIInventory->m_pMyInvWnd[iPos]->iCount = iItemCount;
					PlayItemSound(CGameProcedure::s_pProcMain->m_pUIInventory->m_pMyInvWnd[iPos]->pItemBasic);
				}
			}
			else
			{
				// 아이템이 없는 경우 .. 새로 만든다.. 갯수 셋팅..
				// Picking up countable item for the first time or just a regular non-countable item
				AddToItemTableToInventory(iItemID, iItemCount, iPos);
			}

			std::string szMsg = fmt::format_text_resource(IDS_ITEM_GET_BY_RULE, pItem->szName);
			CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xff9b9bff);

			spItem = m_pMyDroppedItem[s_sRecoveryJobInfo.UIWndSourceStart.iOrder];
			if (spItem != nullptr)
			{
				// 매니저에서 제거..
				RemoveChild(spItem->pUIIcon);

				// 리소스 제거..
				spItem->pUIIcon->Release();
				delete spItem->pUIIcon;
				spItem->pUIIcon = nullptr;
				delete spItem;
				spItem = nullptr;
			}

			m_pMyDroppedItem[s_sRecoveryJobInfo.UIWndSourceStart.iOrder] = nullptr;
		}
		else
		{
			pInfoExt = &CGameBase::s_pPlayer->m_InfoExt;

			// 돈 업데이트..
			stdMsg   = fmt::format_text_resource(IDS_DROPPED_NOAH_GET, iGold - pInfoExt->iGold);
			CGameProcedure::s_pProcMain->MsgOutput(stdMsg, 0xff9b9bff);

			pInfoExt->iGold = iGold;
			CGameProcedure::s_pProcMain->m_pUIInventory->GoldUpdate();

			spItem = s_sRecoveryJobInfo.pItemSource;
			if (spItem == nullptr)
				return;

			// 매니저에서 제거..
			RemoveChild(spItem->pUIIcon);

			// 리소스 제거..
			spItem->pUIIcon->Release();
			delete spItem->pUIIcon;
			spItem->pUIIcon = nullptr;
			delete spItem;
			spItem                                                       = nullptr;
			m_pMyDroppedItem[s_sRecoveryJobInfo.UIWndSourceStart.iOrder] = nullptr;

			PlayGoldSound();
		}

		if (CGameProcedure::s_pProcMain->m_pUISkillTreeDlg != nullptr)
			CGameProcedure::s_pProcMain->m_pUISkillTreeDlg->UpdateDisableCheck();

		if (CGameProcedure::s_pProcMain->m_pUIHotKeyDlg != nullptr)
			CGameProcedure::s_pProcMain->m_pUIHotKeyDlg->UpdateDisableCheck();
	}

	bFound = false;
	for (int i = 0; i < MAX_ITEM_BUNDLE_DROP_PIECE; i++)
	{
		if (m_pMyDroppedItem[i] != nullptr)
			bFound = true;
	}

	if (!bFound)
		LeaveDroppedState();
}

void CUIDroppedItemDlg::SetVisibleWithNoSound(bool bVisible, bool bWork, bool bReFocus)
{
	CN3UIWndBase::SetVisibleWithNoSound(bVisible, bWork, bReFocus);

	for (int i = 0; i < MAX_ITEM_BUNDLE_DROP_PIECE; i++)
		m_bSendedIconArray[i] = false;
}

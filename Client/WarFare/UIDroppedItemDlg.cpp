// UIDroppedItemDlg.cpp: implementation of the UIDroppedItemDlg class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "UIDroppedItemDlg.h"
#include "PacketDef.h"
#include "LocalInput.h"
#include "APISocket.h"
#include "GameProcMain.h"
#include "PlayerMyself.h"
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

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUIDroppedItemDlg::CUIDroppedItemDlg()
{
	for( int i = 0; i < MAX_ITEM_BUNDLE_DROP_PIECE; i++ )	m_pMyDroppedItem[i] = NULL;
	for( int i = 0; i < MAX_ITEM_BUNDLE_DROP_PIECE; i++ )	m_bSendedIconArray[i] = false;

	m_iItemBundleID = 0;
	m_pUITooltipDlg = NULL;
}

CUIDroppedItemDlg::~CUIDroppedItemDlg()
{
	Release();
}

void CUIDroppedItemDlg::Release()
{
	CN3UIBase::Release();

	for( int i = 0; i < MAX_ITEM_BUNDLE_DROP_PIECE; i++ )
	{
		if ( m_pMyDroppedItem[i] != NULL )
		{
			delete m_pMyDroppedItem[i];
			m_pMyDroppedItem[i] = NULL;
		}
	}
}
void CUIDroppedItemDlg::Init(CN3UIBase* pParent)
{
	CN3UIBase::Init(pParent);

}

void CUIDroppedItemDlg::Render()
{
	if (!m_bVisible) return;	// 보이지 않으면 자식들을 render하지 않는다.
	POINT ptCur = CGameProcedure::s_pLocalInput->MouseGetPos();
	m_pUITooltipDlg->DisplayTooltipsDisable();

	bool bTooltipRender = false;
	__IconItemSkill* spItem;

	for(UIListReverseItor itor = m_Children.rbegin(); m_Children.rend() != itor; ++itor)
	{
		CN3UIBase* pChild = (*itor);
		pChild->Render();
		if ( (pChild->UIType() == UI_TYPE_ICON) && (pChild->GetStyle() & UISTYLE_ICON_HIGHLIGHT) )
		{
			bTooltipRender = true;
			spItem = GetHighlightIconItem( (CN3UIIcon* )pChild );
		}
	}

	if ( bTooltipRender )
		m_pUITooltipDlg->DisplayTooltipsEnable(ptCur.x, ptCur.y, spItem );

	// 갯수 표시되야 할 아이템 갯수 표시..
	CN3UIString *pStr = NULL;
	for( int i = 0; i < MAX_ITEM_BUNDLE_DROP_PIECE; i++ )
	{
		if ( m_pMyDroppedItem[i] && ( (m_pMyDroppedItem[i]->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE) || 
			(m_pMyDroppedItem[i]->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL) ) )
		{
			// string 얻기..
			pStr = GetChildStringByiOrder(i);
			if(pStr) 
			{
				if ( (GetState() == UI_STATE_ICON_MOVING) && (m_pMyDroppedItem[i] == CN3UIWndBase::m_sSelectedIconInfo.pItemSelect) )
				{
					pStr->SetVisible(false);
				}
				else
				{
					if ( m_pMyDroppedItem[i]->pUIIcon->IsVisible() )
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
		}
		else
		{
			// string 얻기..
			pStr = GetChildStringByiOrder(i);
			if(pStr) 
				pStr->SetVisible(false);
		}
	}
}

void CUIDroppedItemDlg::InitIconWnd(e_UIWND eWnd)
{
	__TABLE_UI_RESRC* pTblUI = CGameBase::s_pTbl_UI.Find(CGameBase::s_pPlayer->m_InfoBase.eNation);

	m_pUITooltipDlg = new CUIImageTooltipDlg();
	m_pUITooltipDlg->Init(this);
	m_pUITooltipDlg->LoadFromFile(pTblUI->szItemInfo);
	m_pUITooltipDlg->InitPos();
	m_pUITooltipDlg->SetVisible(FALSE);	

	CN3UIWndBase::InitIconWnd(eWnd);
}

void CUIDroppedItemDlg::InitIconUpdate()
{
	CN3UIArea* pArea;
	float fUVAspect = (float)45.0f/(float)64.0f;
	int i;

	for( i = 0; i < MAX_ITEM_BUNDLE_DROP_PIECE; i++ )
	{
		if ( m_pMyDroppedItem[i] )
		{
			m_pMyDroppedItem[i]->pUIIcon = new CN3UIIcon;
			m_pMyDroppedItem[i]->pUIIcon->Init(this);
			m_pMyDroppedItem[i]->pUIIcon->SetTex(m_pMyDroppedItem[i]->szIconFN);
			m_pMyDroppedItem[i]->pUIIcon->SetUVRect(0,0, fUVAspect, fUVAspect);
			m_pMyDroppedItem[i]->pUIIcon->SetUIType(UI_TYPE_ICON);
			m_pMyDroppedItem[i]->pUIIcon->SetStyle(UISTYLE_ICON_ITEM|UISTYLE_ICON_CERTIFICATION_NEED);
			pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_DROP_ITEM, i);
			if ( pArea )
			{
				m_pMyDroppedItem[i]->pUIIcon->SetRegion(pArea->GetRegion());
				m_pMyDroppedItem[i]->pUIIcon->SetMoveRect(pArea->GetRegion());
			}
		}
	}
}

__IconItemSkill* CUIDroppedItemDlg::GetHighlightIconItem(CN3UIIcon* pUIIcon)
{
	for( int i = 0; i < MAX_ITEM_BUNDLE_DROP_PIECE; i++ )
	{
		if ( (m_pMyDroppedItem[i] != NULL) && (m_pMyDroppedItem[i]->pUIIcon == pUIIcon) )
			return m_pMyDroppedItem[i];
	}
	return NULL;
}

void CUIDroppedItemDlg::EnterDroppedState(int xpos, int ypos)
{
	if ( !IsVisible() )
		SetVisible(true);

	SetPos(xpos, ypos-150);

	int i;
	for( i = 0; i < MAX_ITEM_BUNDLE_DROP_PIECE; i++ )
	{
		m_bSendedIconArray[i] = false;

		if ( m_pMyDroppedItem[i] )
		{
			if ( m_pMyDroppedItem[i]->pUIIcon )
			{
				RemoveChild(m_pMyDroppedItem[i]->pUIIcon);
				m_pMyDroppedItem[i]->pUIIcon->Release();
				delete m_pMyDroppedItem[i]->pUIIcon;
				m_pMyDroppedItem[i]->pUIIcon = NULL;
			}
			delete m_pMyDroppedItem[i];	
			m_pMyDroppedItem[i] = NULL;
		}
	}

	CN3UIWndBase::AllHighLightIconFree();
}


void CUIDroppedItemDlg::LeaveDroppedState()
{
	if ( IsVisible() )
		SetVisible(false);

	for( int i = 0; i < MAX_ITEM_BUNDLE_DROP_PIECE; i++ )	m_bSendedIconArray[i] = false;
}

uint32_t CUIDroppedItemDlg::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if ( !IsVisible() ) { dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);  return dwRet; }

	dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);
	return dwRet;
}

void CUIDroppedItemDlg::AddToItemTable(int iItemID, int iItemCount, int iOrder)
{
	__IconItemSkill*	spItem;
	__TABLE_ITEM_BASIC*	pItem = NULL;								// 아이템 테이블 구조체 포인터..
	__TABLE_ITEM_EXT*	pItemExt = NULL;							// 아이템 테이블 구조체 포인터..
	std::string szIconFN;

	pItem = CGameBase::s_pTbl_Items_Basic.Find(iItemID/1000*1000);	// 열 데이터 얻기..
	if(pItem && pItem->byExtIndex >= 0 && pItem->byExtIndex < MAX_ITEM_EXTENSION)
		pItemExt = CGameBase::s_pTbl_Items_Exts[pItem->byExtIndex].Find(iItemID%1000);
	if(NULL == pItem || NULL == pItemExt)
	{
		__ASSERT(0, "아이템 포인터 테이블에 없음!!");
		CLogWriter::Write("CUIDroppedItemDlg::AddToItemTable - Invalidate ItemID : {}", iItemID);
		return;
	}

	//TRACE("Dropped item from server to ItemDlg %d \n", iItemID);
	e_PartPosition ePart;
	e_PlugPosition ePlug;
	e_ItemType eType = CGameBase::MakeResrcFileNameForUPC(pItem, pItemExt, nullptr, &szIconFN, ePart, ePlug, RACE_UNKNOWN); // 아이템에 따른 파일 이름을 만들어서
	if ( ITEM_TYPE_UNKNOWN == eType ) return;
	
	spItem = new __IconItemSkill;
	spItem->pItemBasic = pItem;
	spItem->pItemExt = pItemExt;
	spItem->szIconFN = szIconFN; // 아이콘 파일 이름 복사..
	spItem->iCount	= iItemCount;
	spItem->iDurability = pItem->siMaxDurability+pItemExt->siMaxDurability;

	m_pMyDroppedItem[iOrder] = spItem;
}

void CUIDroppedItemDlg::AddToItemTableToInventory(int iItemID, int iItemCount, int iOrder)
{
	CN3UIArea* pArea;
	__IconItemSkill*	spItem;
	__TABLE_ITEM_BASIC*	pItem = NULL;								// 아이템 테이블 구조체 포인터..
	__TABLE_ITEM_EXT*	pItemExt = NULL;							// 아이템 테이블 구조체 포인터..
	std::string szIconFN;
	float fUVAspect = (float)45.0f/(float)64.0f;

	pItem = CGameBase::s_pTbl_Items_Basic.Find(iItemID/1000*1000);	// 열 데이터 얻기..
	if(pItem && pItem->byExtIndex >= 0 && pItem->byExtIndex < MAX_ITEM_EXTENSION)
		pItemExt = CGameBase::s_pTbl_Items_Exts[pItem->byExtIndex].Find(iItemID%1000);
	if(NULL == pItem || NULL == pItemExt)
	{
		__ASSERT(0, "아이템 포인터 테이블에 없음!!");
		CLogWriter::Write("CUIDroppedItemDlg::AddToItemTableToInventory - Invalidate ItemID : {}", iItemID);
		return;
	}

	//TRACE("Dropped item from server to ItemDlg %d \n", iItemID);
	e_PartPosition ePart;
	e_PlugPosition ePlug;
	e_ItemType eType = CGameBase::MakeResrcFileNameForUPC(pItem, pItemExt, nullptr, &szIconFN, ePart, ePlug); // 아이템에 따른 파일 이름을 만들어서
	if ( ITEM_TYPE_UNKNOWN == eType ) return;
	
	spItem = new __IconItemSkill;
	spItem->pItemBasic = pItem;
	spItem->pItemExt = pItemExt;
	spItem->szIconFN = szIconFN; // 아이콘 파일 이름 복사..
	spItem->iCount	= iItemCount;
	spItem->iDurability = pItem->siMaxDurability+pItemExt->siMaxDurability;

	spItem->pUIIcon = new CN3UIIcon;
	spItem->pUIIcon->Init(CGameProcedure::s_pProcMain->m_pUIInventory);
	spItem->pUIIcon->SetTex(spItem->szIconFN);
	spItem->pUIIcon->SetUVRect(0,0, fUVAspect, fUVAspect);
	spItem->pUIIcon->SetUIType(UI_TYPE_ICON);
	spItem->pUIIcon->SetStyle(UISTYLE_ICON_ITEM|UISTYLE_ICON_CERTIFICATION_NEED);
	spItem->pUIIcon->SetVisible(true);
	pArea = NULL;
	pArea = CGameProcedure::s_pProcMain->m_pUIInventory->GetChildAreaByiOrder(UI_AREA_TYPE_INV, iOrder);
	if ( pArea )
	{
		spItem->pUIIcon->SetRegion(pArea->GetRegion());
		spItem->pUIIcon->SetMoveRect(pArea->GetRegion());
	}

	CGameProcedure::s_pProcMain->m_pUIInventory->m_pMyInvWnd[iOrder] = spItem;
	PlayItemSound(pItem);
}

bool CUIDroppedItemDlg::ReceiveIconDrop(__IconItemSkill* spItem, POINT ptCur)
{
	CN3UIArea* pArea = NULL;

	if (!m_bVisible) return false;

	// 검사해서 선택된 아이콘을 가진 윈도우에게 결과를 알려줘야 한다..
	switch ( CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWnd )
	{
		// 인벤토리 윈도우로부터 온 것이라면..
		case UIWND_INVENTORY:
			CGameProcedure::s_pProcMain->m_pUIInventory->CancelIconDrop(spItem);
			break;

		// 상거래 윈도우로부터 온 것이라면...
		case UIWND_TRANSACTION:
			CGameProcedure::s_pProcMain->m_pUITransactionDlg->CancelIconDrop(spItem);
			break;
	}

	return false;
}

int CUIDroppedItemDlg::GetInventoryEmptyInviOrder(__IconItemSkill* spItem)
{
	int i;
	if (spItem == NULL)
	{
		for ( i = 0; i < MAX_ITEM_INVENTORY; i++ )
		{
			if ( !CGameProcedure::s_pProcMain->m_pUIInventory->m_pMyInvWnd[i] )
				return i;
		}
	}
	else
	{
		for ( i = 0; i < MAX_ITEM_INVENTORY; i++ )
		{
			if ( CGameProcedure::s_pProcMain->m_pUIInventory->m_pMyInvWnd[i] && 
				(CGameProcedure::s_pProcMain->m_pUIInventory->m_pMyInvWnd[i]->pItemBasic->dwID == spItem->pItemBasic->dwID) )
				return i;
		}

		for ( i = 0; i < MAX_ITEM_INVENTORY; i++ )
		{
			if ( !CGameProcedure::s_pProcMain->m_pUIInventory->m_pMyInvWnd[i] )
				return i;
		}
	}

	return -1;
}

int	CUIDroppedItemDlg::GetItemiOrder(__IconItemSkill* spItem)
{
	int i;

	for( i = 0; i < MAX_ITEM_BUNDLE_DROP_PIECE; i++ )
	{
		if ( (m_pMyDroppedItem[i] != NULL) && (m_pMyDroppedItem[i] == spItem) )
			return i;
	}

	return -1;
}

bool CUIDroppedItemDlg::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
// Temp Define
#define FAIL_CODE {		\
				CN3UIWndBase::m_sSelectedIconInfo.eIconSelectState = UIICON_SELECT_NONE;	\
				return false;	\
			}

// Code Begin
	if(NULL == pSender) return false;

	int iOrder, iOrderInv = -1;

	uint32_t dwBitMask = 0x000f0000;

	__TABLE_ITEM_BASIC*	pItem;
	__IconItemSkill*	spItem;
	std::string szIconFN;
	e_PartPosition ePart;
	e_PlugPosition ePlug;
	e_ItemType eType;

	// 서버에 보내지 않은 아이템이니까.. 서버에 보낸다..
	uint8_t byBuff[16];
	int iOffset = 0;

	switch (dwMsg & dwBitMask)
	{
		case UIMSG_ICON_DOWN_FIRST:
			spItem = NULL;
			spItem = GetHighlightIconItem((CN3UIIcon* )pSender);
			if ( !spItem )	break;
			m_iBackupiOrder = GetItemiOrder( spItem );
			break;

		case UIMSG_ICON_UP:
			SetState(UI_STATE_COMMON_NONE);

			// 아이템이 돈인지 검사..
			pItem = NULL;								// 아이템 테이블 구조체 포인터..
			spItem = NULL;
			spItem = GetHighlightIconItem((CN3UIIcon* )pSender);
			if ( !spItem )	break;
			pItem = CGameBase::s_pTbl_Items_Basic.Find(spItem->pItemBasic->dwID);	// 열 데이터 얻기..
			if ( pItem == NULL )
			{
				__ASSERT(0, "NULL Item!!!");
				CLogWriter::Write("CUIDroppedItemDlg::ReceiveMessage - UIMSG_ICON_UP - NULL Icon : {}", spItem->pItemBasic->dwID);
				break;
			}

			eType = CGameBase::MakeResrcFileNameForUPC(pItem, spItem->pItemExt, nullptr, &szIconFN, ePart, ePlug); // 아이템에 따른 파일 이름을 만들어서

			// 보낸 아이콘 배열이랑 비교.. 
			iOrder = GetItemiOrder( spItem );
			if ( m_iBackupiOrder != iOrder )	break;

			if ( m_bSendedIconArray[iOrder] == true )	// 한번 보냈던 패킷이면 break.. 
				break;
			else
				m_bSendedIconArray[iOrder] = true;

			CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_ITEM_GET);
			CAPISocket::MP_AddDword(byBuff, iOffset, m_iItemBundleID);

			// 돈이 아니면 인벤토리 리스트에 추가.... 
			if ( ITEM_TYPE_GOLD != eType ) 
			{
				CAPISocket::MP_AddDword(byBuff, iOffset, spItem->pItemBasic->dwID+spItem->pItemExt->dwID);
//				CAPISocket::MP_AddDword(byBuff, iOffset, (uint8_t)iOrderInv);
			}
			else
			{
				CAPISocket::MP_AddDword(byBuff, iOffset, spItem->pItemBasic->dwID);
//				CAPISocket::MP_AddDword(byBuff, iOffset, 0xff);
			}
			CGameProcedure::s_pSocket->Send(byBuff, iOffset);

			// 보낸 아이콘 정보 셋팅..	
			CN3UIWndBase::m_sRecoveryJobInfo.pItemSource = spItem;
			CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.UIWnd = UIWND_DROPITEM;
			CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.UIWndDistrict = UIWND_DISTRICT_DROPITEM;
			CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder = iOrder;
			CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.UIWnd = UIWND_INVENTORY;
			CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.UIWndDistrict = UIWND_DISTRICT_INVENTORY_INV;
			CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder = iOrderInv;
			break;

		case UIMSG_ICON_DOWN:
			break;

		case UIMSG_ICON_DBLCLK:
			SetState(UI_STATE_COMMON_NONE);
			break;
	}

	return true;
}

void CUIDroppedItemDlg::GetItemByIDToInventory(uint8_t bResult, int iItemID, int iGold, int iPos, int iItemCount, int iStrLen, 
											   std::string strString)
{
	// 아이템 리스트에서 아이템을 찾고..
	bool				bFound = false;
	__TABLE_ITEM_BASIC*	pItem = NULL;									// 아이템 테이블 구조체 포인터..
	__TABLE_ITEM_EXT*	pItemExt = NULL;
	__IconItemSkill*	spItem = NULL;
	int i;
	CN3UIString* pStatic = NULL;
	__InfoPlayerMySelf*	pInfoExt = NULL;
	std::string stdMsg;

	if (bResult == 0x00)			// 실패..
	{
		int iOrderInv = GetInventoryEmptyInviOrder();
		if ( iOrderInv == -1 )
		{
			// 인벤토리가 꽉 차있으면.. break.. ^^
			std::string stdMsg = fmt::format_text_resource(IDS_INV_ITEM_FULL);
			CGameProcedure::s_pProcMain->MsgOutput(stdMsg, 0xff9b9bff);
		}

		if (CGameProcedure::s_pProcMain->m_pUISkillTreeDlg) CGameProcedure::s_pProcMain->m_pUISkillTreeDlg->UpdateDisableCheck();
		if (CGameProcedure::s_pProcMain->m_pUIHotKeyDlg) CGameProcedure::s_pProcMain->m_pUIHotKeyDlg->UpdateDisableCheck();

		return;
	}
	
	if (bResult == 0x02)		// 파티 상태에서 노아를 얻는다..
	{
		// 돈 갱신..
		pStatic = NULL;
		pInfoExt = &CGameBase::s_pPlayer->m_InfoExt;

		// 돈 업데이트..
		stdMsg = fmt::format_text_resource(IDS_DROPPED_NOAH_GET,
			iGold - pInfoExt->iGold);
		CGameProcedure::s_pProcMain->MsgOutput(stdMsg, 0xff9b9bff);

		pInfoExt->iGold = iGold;
		CGameProcedure::s_pProcMain->m_pUIInventory->GoldUpdate();

		if (!IsVisible()) 
			return;

		// 돈 아이콘이 있으면 없앤다..
		bFound = false;
		for( i = 0; i < MAX_ITEM_BUNDLE_DROP_PIECE; i++ )
		{
			if( m_pMyDroppedItem[i] && m_pMyDroppedItem[i]->pItemBasic->dwID == dwGold )
			{
				bFound = true;
				break;
			}
		}
		if (!bFound)	return;

		spItem = m_pMyDroppedItem[i];
		if (!spItem)	return;
		// 매니저에서 제거..
		RemoveChild(spItem->pUIIcon);

		// 리소스 제거..
		spItem->pUIIcon->Release();
		delete spItem->pUIIcon;
		spItem->pUIIcon = NULL;
		delete spItem;
		spItem = NULL;
		m_pMyDroppedItem[i] = NULL;

		PlayGoldSound();

		if (CGameProcedure::s_pProcMain->m_pUISkillTreeDlg) CGameProcedure::s_pProcMain->m_pUISkillTreeDlg->UpdateDisableCheck();
		if (CGameProcedure::s_pProcMain->m_pUIHotKeyDlg) CGameProcedure::s_pProcMain->m_pUIHotKeyDlg->UpdateDisableCheck();
	}

	if (bResult == 0x03)		// 파티상태에서 내가 아이템을 습득..
	{
		pItem = NULL;
		pItemExt = NULL;
		pItem = CGameBase::s_pTbl_Items_Basic.Find(iItemID/1000*1000);	// 열 데이터 얻기..
		if(pItem && pItem->byExtIndex >= 0 && pItem->byExtIndex < MAX_ITEM_EXTENSION)
			pItemExt = CGameBase::s_pTbl_Items_Exts[pItem->byExtIndex].Find(iItemID%1000);
		if(NULL == pItem || NULL == pItemExt)
		{
			__ASSERT(0, "아이템 포인터 테이블에 없음!!");
			CLogWriter::Write("CUIDroppedItemDlg::GetItemByIDToInventory - NULL Icon : {}", iItemID);
			return;
		}

		std::string szMsg = fmt::format_text_resource(IDS_PARTY_ITEM_GET,
			strString, pItem->szName);
		CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xff9b9bff);

		if (!IsVisible()) 
			return;

		// 아이템 아이콘이 있으면 없앤다..
		bFound = false;
		for( i = 0; i < MAX_ITEM_BUNDLE_DROP_PIECE; i++ )
		{
			if( (m_pMyDroppedItem[i]) && (m_pMyDroppedItem[i]->pItemBasic->dwID+m_pMyDroppedItem[i]->pItemExt->dwID == iItemID) )
			{
				bFound = true;
				break;
			}
		}
		if (!bFound)	return;

		if (m_pMyDroppedItem[i])
			PlayItemSound(m_pMyDroppedItem[i]->pItemBasic);

		spItem = m_pMyDroppedItem[i];
		if (!spItem)	return;
		// 매니저에서 제거..
		RemoveChild(spItem->pUIIcon);

		// 리소스 제거..
		spItem->pUIIcon->Release();
		delete spItem->pUIIcon;
		spItem->pUIIcon = NULL;
		delete spItem;
		spItem = NULL;
		m_pMyDroppedItem[i] = NULL;

		if (CGameProcedure::s_pProcMain->m_pUISkillTreeDlg) CGameProcedure::s_pProcMain->m_pUISkillTreeDlg->UpdateDisableCheck();
		if (CGameProcedure::s_pProcMain->m_pUIHotKeyDlg) CGameProcedure::s_pProcMain->m_pUIHotKeyDlg->UpdateDisableCheck();

		return;
	}

	if (bResult == 0x04)		// 파티 상태에서 다른 멤버가 아이템을 습득..
	{
		spItem = m_pMyDroppedItem[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder];
		if (spItem)
		{
			// 매니저에서 제거..
			RemoveChild(spItem->pUIIcon);

			// 리소스 제거..
			spItem->pUIIcon->Release();
			delete spItem->pUIIcon;
			spItem->pUIIcon = NULL;
			delete spItem;
			spItem = NULL;
		}

		m_pMyDroppedItem[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder] = NULL;

		if (CGameProcedure::s_pProcMain->m_pUISkillTreeDlg) CGameProcedure::s_pProcMain->m_pUISkillTreeDlg->UpdateDisableCheck();
		if (CGameProcedure::s_pProcMain->m_pUIHotKeyDlg) CGameProcedure::s_pProcMain->m_pUIHotKeyDlg->UpdateDisableCheck();

		return;	
	}

	if (bResult == 0x05)		// 파티 상태에서 일반적인 아이템 습득..
	{
		if (iItemID == dwGold)
		{
			__ASSERT(0, "Invalidate Item ID From Server.. ");
			CLogWriter::Write("CUIDroppedItemDlg::GetItemByIDToInventory - ID Pos : {}", iPos);
			return;
		}

		if ( (iPos < 0) || (iPos > (MAX_ITEM_INVENTORY-1)) )
		{
			__ASSERT(0, "Invalidate Item Pos From Server.. ");
			CLogWriter::Write("CUIDroppedItemDlg::GetItemByIDToInventory - Invalidate Pos : {}", iPos);
			return;
		}

		__IconItemSkill	*spItemDest = CGameProcedure::s_pProcMain->m_pUIInventory->m_pMyInvWnd[iPos];
		if (spItemDest)		// 아이템이 있다..
		{
			if ( iItemID != (spItemDest->pItemBasic->dwID+spItemDest->pItemExt->dwID) )
			{
				// 기존 이이템을 클리어..
				if (!spItemDest) return;
				RemoveChild(spItemDest->pUIIcon);

				// 아이콘 리소스 삭제...
				spItemDest->pUIIcon->Release();
				delete spItemDest->pUIIcon;
				spItemDest->pUIIcon = NULL;
				delete spItemDest;
				spItemDest = NULL;

				CGameProcedure::s_pProcMain->m_pUIInventory->m_pMyInvWnd[iPos] = NULL;

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

		pItem = NULL;
		pItem = CGameBase::s_pTbl_Items_Basic.Find(iItemID/1000*1000);	// 열 데이터 얻기..
		if(NULL == pItem)
		{
			__ASSERT(0, "아이템 포인터 테이블에 없음!!");
			CLogWriter::Write("CUIDroppedItemDlg::GetItemByIDToInventory - NULL Icon : {}", iItemID);
			return;
		}

		stdMsg = fmt::format_text_resource(IDS_ITEM_GET_BY_RULE,
			pItem->szName);
		CGameProcedure::s_pProcMain->MsgOutput(stdMsg, 0xff9b9bff);

		if (CGameProcedure::s_pProcMain->m_pUISkillTreeDlg) CGameProcedure::s_pProcMain->m_pUISkillTreeDlg->UpdateDisableCheck();
		if (CGameProcedure::s_pProcMain->m_pUIHotKeyDlg) CGameProcedure::s_pProcMain->m_pUIHotKeyDlg->UpdateDisableCheck();
	}

	if (bResult == 0x06)
	{
		// 메시지 박스 텍스트 표시..
		std::string szMsg = fmt::format_text_resource(IDS_ITEM_TOOMANY_OR_HEAVY);
		CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);
	}

	if (bResult == 0x07)
	{
		// 메시지 박스 텍스트 표시..
		std::string szMsg = fmt::format_text_resource(IDS_INV_ITEM_FULL);
		CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);
	}

	if (bResult == 0x01)
	{
		if (iItemID != dwGold)
		{
			pItem = CGameBase::s_pTbl_Items_Basic.Find(iItemID / 1000 * 1000);
			if (NULL == pItem)
			{ 
				__ASSERT(0, "Invalidate Item ID From Server.. ");
				CLogWriter::Write("CUIDroppedItemDlg::GetItemByIDToInventory - Invalidate Item ID : {}", iItemID);
				return;
			}
			if ( (iPos < 0) || (iPos > (MAX_ITEM_INVENTORY-1)) )
			{
				__ASSERT(0, "Invalidate Item Pos From Server.. ");
				CLogWriter::Write("CUIDroppedItemDlg::GetItemByIDToInventory - Invalidate Pos : {}", iPos);
				return;
			}

			spItem = NULL;
			__IconItemSkill	*spItemDest = CGameProcedure::s_pProcMain->m_pUIInventory->m_pMyInvWnd[iPos];
			if (spItemDest)		// 아이템이 있다..
			{
				if ( iItemID != (spItemDest->pItemBasic->dwID+spItemDest->pItemExt->dwID) )
				{
					// 기존 이이템을 클리어..
					if (!spItemDest) return;
					RemoveChild(spItemDest->pUIIcon);

					// 아이콘 리소스 삭제...
					spItemDest->pUIIcon->Release();
					delete spItemDest->pUIIcon;
					spItemDest->pUIIcon = NULL;
					delete spItemDest;
					spItemDest = NULL;

					CGameProcedure::s_pProcMain->m_pUIInventory->m_pMyInvWnd[iPos] = NULL;

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

			std::string szMsg = fmt::format_text_resource(IDS_ITEM_GET_BY_RULE,
				pItem->szName);
			CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xff9b9bff);

			spItem = m_pMyDroppedItem[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder];
			if (spItem)
			{
				// 매니저에서 제거..
				RemoveChild(spItem->pUIIcon);

				// 리소스 제거..
				spItem->pUIIcon->Release();
				delete spItem->pUIIcon;
				spItem->pUIIcon = NULL;
				delete spItem;
				spItem = NULL;
			}
			m_pMyDroppedItem[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder] = NULL;
		}
		else
		{
			pStatic = NULL;
			pInfoExt = &(CGameBase::s_pPlayer->m_InfoExt);

			// 돈 업데이트..
			stdMsg = fmt::format_text_resource(IDS_DROPPED_NOAH_GET,
				iGold - pInfoExt->iGold);
			CGameProcedure::s_pProcMain->MsgOutput(stdMsg, 0xff9b9bff);

			pInfoExt->iGold = iGold;
			//TRACE("돈 업데이트 %d \n", iGold);
			CGameProcedure::s_pProcMain->m_pUIInventory->GoldUpdate();

			spItem = CN3UIWndBase::m_sRecoveryJobInfo.pItemSource;
			if ( NULL == spItem ) return;

			// 매니저에서 제거..
			RemoveChild(spItem->pUIIcon);

			// 리소스 제거..
			spItem->pUIIcon->Release();
			delete spItem->pUIIcon;
			spItem->pUIIcon = NULL;
			delete spItem;
			spItem = NULL;
			m_pMyDroppedItem[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder] = NULL;

			PlayGoldSound();
		}

		if (CGameProcedure::s_pProcMain->m_pUISkillTreeDlg) CGameProcedure::s_pProcMain->m_pUISkillTreeDlg->UpdateDisableCheck();
		if (CGameProcedure::s_pProcMain->m_pUIHotKeyDlg) CGameProcedure::s_pProcMain->m_pUIHotKeyDlg->UpdateDisableCheck();
	}

	bFound = false;
	for( i = 0; i < MAX_ITEM_BUNDLE_DROP_PIECE; i++ )
	{
		if( m_pMyDroppedItem[i] != NULL )
			bFound = true;
	}

	if ( !bFound )
		LeaveDroppedState();
}

//this_ui_add_start
void CUIDroppedItemDlg::SetVisibleWithNoSound(bool bVisible, bool bWork, bool bReFocus)
{
	CN3UIBase::SetVisibleWithNoSound(bVisible, bWork, bReFocus);

	for( int i = 0; i < MAX_ITEM_BUNDLE_DROP_PIECE; i++ )	m_bSendedIconArray[i] = false;
}
//this_ui_add_end

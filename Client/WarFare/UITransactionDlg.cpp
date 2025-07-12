// UITransactionDlg.cpp: implementation of the CUITransactionDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UITransactionDlg.h"
#include "PacketDef.h"
#include "LocalInput.h"
#include "APISocket.h"
#include "GameProcMain.h"
#include "UIImageTooltipDlg.h"
#include "UIInventory.h"
#include "UIManager.h"
#include "PlayerMySelf.h"
#include "CountableItemEditDlg.h"
#include "UIHotKeyDlg.h"
#include "UISkillTreeDlg.h"
#include "resource.h"

#include <N3Base/LogWriter.h>

#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIString.h>
#include <N3Base/N3UIEdit.h>
#include <N3Base/N3SndObj.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUITransactionDlg::CUITransactionDlg()
{
	m_iCurPage = 0;
	
	for(int i = 0; i < MAX_ITEM_TRADE_PAGE; i++ )
		for(int j = 0; j < MAX_ITEM_TRADE; j++ )		
			m_pMyTrade[i][j] = nullptr;

	for(int i = 0; i < MAX_ITEM_INVENTORY; i++ )	
		m_pMyTradeInv[i] = nullptr;

	m_pUITooltipDlg = nullptr;
	m_pStrMyGold    = nullptr;
	m_pUIInn		= nullptr;
	m_pUIBlackSmith	= nullptr;
	m_pUIStore		= nullptr;
	m_pText_Weight	= nullptr;

	this->SetVisible(false);
}

CUITransactionDlg::~CUITransactionDlg()
{
	Release();
}

void CUITransactionDlg::Release()
{
	CN3UIBase::Release();

	for(int i = 0; i < MAX_ITEM_TRADE_PAGE; i++ )
		for(int j = 0; j < MAX_ITEM_TRADE; j++ )
		{
			if ( m_pMyTrade[i][j] != nullptr )
			{
				delete m_pMyTrade[i][j];
				m_pMyTrade[i][j] = nullptr;
			}
		}

	for(int i = 0; i < MAX_ITEM_INVENTORY; i++ )
	{
		if ( m_pMyTradeInv[i] != nullptr)
		{
			delete m_pMyTradeInv[i];
			m_pMyTradeInv[i] = nullptr;
		}
	}
}

void CUITransactionDlg::Render()
{
	if (!m_bVisible) return;	// If not visible, do not render the children.

	POINT ptCur = CGameProcedure::s_pLocalInput->MouseGetPos();
	m_pUITooltipDlg->DisplayTooltipsDisable();

	bool bTooltipRender = false;
	__IconItemSkill* spItem = nullptr;

	for(UIListReverseItor itor = m_Children.rbegin(); m_Children.rend() != itor; ++itor)
	{
		CN3UIBase* pChild = (*itor);
		if ( (GetState() == UI_STATE_ICON_MOVING) && (pChild->UIType() == UI_TYPE_ICON) && (CN3UIWndBase::m_sSelectedIconInfo.pItemSelect) &&
			((CN3UIIcon *)pChild == CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pUIIcon) )	continue;
			pChild->Render();
		if ( (GetState() == UI_STATE_COMMON_NONE) && 
				(pChild->UIType() == UI_TYPE_ICON) && (pChild->GetStyle() & UISTYLE_ICON_HIGHLIGHT) )
		{
			bTooltipRender = true;
			spItem = GetHighlightIconItem( (CN3UIIcon* )pChild );
		}
	}

	// Display the number of items that should be shown.
	for(int i = 0; i < MAX_ITEM_INVENTORY; i++ )
	{
		if ( m_pMyTradeInv[i] && ( (m_pMyTradeInv[i]->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE) || 
			(m_pMyTradeInv[i]->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL) ) )
		{
			// Get the string.
			CN3UIString* pStr = GetChildStringByiOrder(i);
			if(pStr) 
			{
				if ( (GetState() == UI_STATE_ICON_MOVING) && (m_pMyTradeInv[i] == CN3UIWndBase::m_sSelectedIconInfo.pItemSelect) )
				{
					pStr->SetVisible(false);
				}
				else
				{
					if ( m_pMyTradeInv[i]->pUIIcon->IsVisible() )
					{
						pStr->SetVisible(true);
						pStr->SetStringAsInt(m_pMyTradeInv[i]->iCount);
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
			// Get the string.
			CN3UIString* pStr = GetChildStringByiOrder(i);
			if(pStr) 
				pStr->SetVisible(false);
		}
	}

	if ( (GetState() == UI_STATE_ICON_MOVING) && (CN3UIWndBase::m_sSelectedIconInfo.pItemSelect) ) 
		CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pUIIcon->Render();		

	if ( bTooltipRender && spItem )
	{
		e_UIWND_DISTRICT eUD = GetWndDistrict(spItem);
		switch (eUD)
		{
			case UIWND_DISTRICT_TRADE_NPC:
				m_pUITooltipDlg->DisplayTooltipsEnable(ptCur.x, ptCur.y, spItem, true, true );
				break;
			case UIWND_DISTRICT_TRADE_MY:
				m_pUITooltipDlg->DisplayTooltipsEnable(ptCur.x, ptCur.y, spItem, true, false );
				break;
		}
	}
}

void CUITransactionDlg::InitIconWnd(e_UIWND eWnd)
{
	__TABLE_UI_RESRC* pTbl = CGameBase::s_pTbl_UI.Find(CGameBase::s_pPlayer->m_InfoBase.eNation);

	m_pUITooltipDlg = new CUIImageTooltipDlg();
	m_pUITooltipDlg->Init(this);
	m_pUITooltipDlg->LoadFromFile(pTbl->szItemInfo);
	m_pUITooltipDlg->InitPos();
	m_pUITooltipDlg->SetVisible(false);	

	CN3UIWndBase::InitIconWnd(eWnd);
	N3_VERIFY_UI_COMPONENT(m_pStrMyGold, (CN3UIString*) GetChildByID("string_item_name"));

	if(m_pStrMyGold != nullptr) m_pStrMyGold->SetString("0");

	N3_VERIFY_UI_COMPONENT(m_pUIInn, (CN3UIImage*) GetChildByID("img_inn"));
	N3_VERIFY_UI_COMPONENT(m_pUIBlackSmith, (CN3UIImage*) GetChildByID("img_blacksmith"));
	N3_VERIFY_UI_COMPONENT(m_pUIStore, (CN3UIImage*) GetChildByID("img_store"));
	N3_VERIFY_UI_COMPONENT(m_pText_Weight, (CN3UIString*) GetChildByID("text_weight"));
}

void CUITransactionDlg::InitIconUpdate()
{
	CN3UIArea* pArea;
	float fUVAspect = (float)45.0f/(float)64.0f;

	for(int i = 0; i < MAX_ITEM_TRADE_PAGE; i++ )
		for(int j = 0; j < MAX_ITEM_TRADE; j++ )
		{
			if ( m_pMyTrade[i][j] != nullptr )
			{
				m_pMyTrade[i][j]->pUIIcon = new CN3UIIcon;
				m_pMyTrade[i][j]->pUIIcon->Init(this);
				m_pMyTrade[i][j]->pUIIcon->SetTex(m_pMyTrade[i][j]->szIconFN);
				m_pMyTrade[i][j]->pUIIcon->SetUVRect(0,0,fUVAspect,fUVAspect);
				m_pMyTrade[i][j]->pUIIcon->SetUIType(UI_TYPE_ICON);
				m_pMyTrade[i][j]->pUIIcon->SetStyle(UISTYLE_ICON_ITEM|UISTYLE_ICON_CERTIFICATION_NEED);
				pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_NPC, j);
				if ( pArea )
				{
					m_pMyTrade[i][j]->pUIIcon->SetRegion(pArea->GetRegion());
					m_pMyTrade[i][j]->pUIIcon->SetMoveRect(pArea->GetRegion());
				}
			}
		}
}

__IconItemSkill* CUITransactionDlg::GetHighlightIconItem(CN3UIIcon* pUIIcon)
{

	for(int i = 0; i < MAX_ITEM_TRADE; i++)
	{
		if ( (m_pMyTrade[m_iCurPage][i] != nullptr) && (m_pMyTrade[m_iCurPage][i]->pUIIcon == pUIIcon) )
			return m_pMyTrade[m_iCurPage][i];
	}

	for(int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if ( (m_pMyTradeInv[i] != nullptr) && (m_pMyTradeInv[i]->pUIIcon == pUIIcon) ) 
			return m_pMyTradeInv[i];
	}

	return nullptr;
}

void CUITransactionDlg::EnterTransactionState()
{
	
	for(int j = 0; j < MAX_ITEM_TRADE_PAGE; j++ )
		for(int i = 0; i < MAX_ITEM_TRADE; i++ )
		{
			if ( m_pMyTrade[j][i] != nullptr )
			{
				if ( m_pMyTrade[j][i]->pUIIcon )
				{
					RemoveChild(m_pMyTrade[j][i]->pUIIcon);
					m_pMyTrade[j][i]->pUIIcon->Release();
					delete m_pMyTrade[j][i]->pUIIcon;
					m_pMyTrade[j][i]->pUIIcon = nullptr;
				}
				delete m_pMyTrade[j][i];	
				m_pMyTrade[j][i] = nullptr;
			}
		}

	for(int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if ( m_pMyTradeInv[i] != nullptr )
		{
			if ( m_pMyTradeInv[i]->pUIIcon )
			{
				RemoveChild(m_pMyTradeInv[i]->pUIIcon);
				m_pMyTradeInv[i]->pUIIcon->Release();
				delete m_pMyTradeInv[i]->pUIIcon;
				m_pMyTradeInv[i]->pUIIcon = nullptr;
			}
			delete m_pMyTradeInv[i];	
			m_pMyTradeInv[i] = nullptr;
		}
	}

	std::string szIconFN;
	__IconItemSkill*	spItem = nullptr;
	__TABLE_ITEM_BASIC*	pItem = nullptr; //Pointer to the item table structure
	__TABLE_ITEM_EXT*	pItemExt = nullptr;

	int iOrg = m_iTradeID/1000;
	int iExt = m_iTradeID%1000;
	int iSize = CGameBase::s_pTbl_Items_Basic.GetSize();

	int j = 0, k = 0;
	for (int i = 0; i < iSize; i++ )
	{
		if (k >= MAX_ITEM_TRADE)
		{
			j++;	k = 0;
		}

		if (j >= MAX_ITEM_TRADE_PAGE)
			break;

		pItem = CGameBase::s_pTbl_Items_Basic.GetIndexedData(i);
		if(pItem == nullptr) // If there are no items...
		{
			__ASSERT(0, "Item pointer not found in the table!!");
			CLogWriter::Write("CUITransactionDlg::EnterTransactionState - Invalid Item ID : %d, %d", iOrg, iExt);
			continue;
		}

		if(pItem->byExtIndex < 0 || pItem->byExtIndex >= MAX_ITEM_EXTENSION)
			continue;

		if (pItem->bySellGroup != iOrg)
			continue;

		pItemExt = CGameBase::s_pTbl_Items_Exts[pItem->byExtIndex].Find(iExt);	
		if(pItemExt == nullptr) // If there is no item
		{
			__ASSERT(0, "Item pointer not found in the table!!");
			CLogWriter::Write("CUITransactionDlg::EnterTransactionState - Invalid Item ID : %d, %d", iOrg, iExt);
			continue;
		}

		if ( pItemExt->dwID != iExt )
			continue;

		e_PartPosition ePart;
		e_PlugPosition ePlug;
		e_ItemType eType = CGameProcedure::MakeResrcFileNameForUPC(pItem, nullptr, &szIconFN, ePart, ePlug); // 아이템에 따른 파일 이름을 만들어서
		__ASSERT(ITEM_TYPE_UNKNOWN != eType, "Unknown Item");

		spItem = new __IconItemSkill;
		spItem->pItemBasic	= pItem;
		spItem->pItemExt	= pItemExt;
		spItem->szIconFN	= szIconFN; // Copy the icon file name...
		spItem->iCount		= 1;
		spItem->iDurability = pItem->siMaxDurability+pItemExt->siMaxDurability;

		m_pMyTrade[j][k] = spItem; 

		k++;
	}

	InitIconUpdate();
	m_iCurPage = 0;
	CN3UIString* pStr = (CN3UIString* )GetChildByID("string_page");
	if (pStr)
	{
		char pszID[32];
		sprintf(pszID, "%d",m_iCurPage+1);
		pStr->SetString(pszID);
	}

	for(int i = 0; i < MAX_ITEM_TRADE_PAGE; i++ )
	{
		if (i == m_iCurPage)
		{
			for(int j = 0; j < MAX_ITEM_TRADE; j++ )
			{
				if ( m_pMyTrade[i][j] != nullptr )
					m_pMyTrade[i][j]->pUIIcon->SetVisible(true);
			}	
		}
		else
		{
			for(int j = 0; j < MAX_ITEM_TRADE; j++ )
			{
				if ( m_pMyTrade[i][j] != nullptr )
					m_pMyTrade[i][j]->pUIIcon->SetVisible(false);
			}	
		}
	}

	ItemMoveFromInvToThis();

	GoldUpdate();

	switch ((int)(m_iTradeID/1000))
	{
		case 122:
		case 222:
			ShowTitle(UI_BLACKSMITH);
			break;
		default:
			ShowTitle(UI_STORE);
			break;
	}
}

void CUITransactionDlg::UpdateWeight(const std::string& szWeight)
{
	if (m_pText_Weight != nullptr)
		m_pText_Weight->SetString(szWeight);
}

void CUITransactionDlg::GoldUpdate()
{
	if (m_pStrMyGold == nullptr)
		return;

	std::string strGold = CGameBase::FormatNumber(CGameBase::s_pPlayer->m_InfoExt.iGold);
	m_pStrMyGold->SetString(strGold);
}

void CUITransactionDlg::ItemMoveFromInvToThis()
{
	CUIInventory* pInven = CGameProcedure::s_pProcMain->m_pUIInventory;
	if(!pInven) return;

	for(int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		m_pMyTradeInv[i] = nullptr;
	}

	for(int i = 0; i < MAX_ITEM_INVENTORY; i++ )
	{
		if(pInven->m_pMyInvWnd[i])
		{
			__IconItemSkill* spItem = pInven->m_pMyInvWnd[i];
			spItem->pUIIcon->SetParent(this);

			pInven->m_pMyInvWnd[i] = nullptr;
			CN3UIArea* pArea;

			pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, i);
			if ( pArea )
			{
				spItem->pUIIcon->SetRegion(pArea->GetRegion());
				spItem->pUIIcon->SetMoveRect(pArea->GetRegion());
			}			

			m_pMyTradeInv[i] = spItem;
		}
	}
}

void CUITransactionDlg::LeaveTransactionState()
{
	if ( IsVisible() )
		SetVisible(false);

	if (GetState() == UI_STATE_ICON_MOVING)
		IconRestore();
	SetState(UI_STATE_COMMON_NONE);
	CN3UIWndBase::AllHighLightIconFree();

	// Move the item from the inv area of this window to the inv area of this inventory window...
	ItemMoveFromThisToInv();

	if (CGameProcedure::s_pProcMain->m_pUISkillTreeDlg) CGameProcedure::s_pProcMain->m_pUISkillTreeDlg->UpdateDisableCheck();
	if (CGameProcedure::s_pProcMain->m_pUIHotKeyDlg) CGameProcedure::s_pProcMain->m_pUIHotKeyDlg->UpdateDisableCheck();
}

void CUITransactionDlg::ItemMoveFromThisToInv()
{
	CUIInventory* pInven = CGameProcedure::s_pProcMain->m_pUIInventory;
	if(!pInven) return;

	
	for(int i = 0; i < MAX_ITEM_INVENTORY; i++ )
	{
		if(m_pMyTradeInv[i])
		{
			__IconItemSkill* spItem = m_pMyTradeInv[i];
			spItem->pUIIcon->SetParent(pInven);

			m_pMyTradeInv[i] = nullptr;

			CN3UIArea* pArea;

			pArea = pInven->GetChildAreaByiOrder(UI_AREA_TYPE_INV, i);
			if ( pArea )
			{
				spItem->pUIIcon->SetRegion(pArea->GetRegion());
				spItem->pUIIcon->SetMoveRect(pArea->GetRegion());
			}			

			pInven->m_pMyInvWnd[i] = spItem;
		}
	}
}

void CUITransactionDlg::ItemCountOK()
{
	int iGold = CN3UIWndBase::m_pCountableItemEdit->GetQuantity();
	__IconItemSkill* spItem, *spItemNew = nullptr;
	__InfoPlayerMySelf*	pInfoExt = &(CGameBase::s_pPlayer->m_InfoExt);
	int iWeight;

	switch (CN3UIWndBase::m_pCountableItemEdit->GetCallerWndDistrict())
	{
		case UIWND_DISTRICT_TRADE_NPC:		// In case of purchase...
			spItem = m_pMyTrade[m_iCurPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder];

			switch (spItem->pItemBasic->byContable)
			{
				case UIITEM_TYPE_ONLYONE:
				case UIITEM_TYPE_SOMOONE:
					iWeight = spItem->pItemBasic->siWeight;

					// Weight check.
					if ( (pInfoExt->iWeight + iWeight) > pInfoExt->iWeightMax)
					{	 
						std::string szMsg;
						CGameBase::GetText(IDS_ITEM_WEIGHT_OVERFLOW, &szMsg);	
						CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);	
						return;
					}
					break;

				case UIITEM_TYPE_COUNTABLE:
					if ( iGold <= 0 ) return;
					if ( iGold > UIITEM_COUNT_MANY ) 
					{
						std::string szMsg;
						CGameBase::GetText(IDS_MANY_COUNTABLE_ITEM_BUY_FAIL, &szMsg);
						CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);				
						return;
					}

					if (spItem->iCount + iGold > UIITEM_COUNT_MANY)
					{
						std::string szMsg;
						CGameBase::GetText(IDS_MANY_COUNTABLE_ITEM_GET_MANY, &szMsg);
						CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);				
						return;
					}

					if ( (iGold * spItem->pItemBasic->iPrice)	> pInfoExt->iGold )	
					{
						std::string szMsg;
						CGameBase::GetText(IDS_COUNTABLE_ITEM_BUY_NOT_ENOUGH_MONEY, &szMsg);
						CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);				
						return;
					}

					iWeight = iGold * spItem->pItemBasic->siWeight;

					if ( (pInfoExt->iWeight + iWeight) > pInfoExt->iWeightMax)
					{	 
						std::string szMsg;
						CGameBase::GetText(IDS_ITEM_WEIGHT_OVERFLOW, &szMsg);	
						CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);	
						return;
					}
					break;

				case UIITEM_TYPE_COUNTABLE_SMALL:
					if ( iGold <= 0 ) return;

					if ( iGold > UIITEM_COUNT_FEW ) 
					{
						std::string szMsg;
						CGameBase::GetText(IDS_SMALL_COUNTABLE_ITEM_BUY_FAIL, &szMsg);
						CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);				
						return;
					}

					if (spItem->iCount + iGold > UIITEM_COUNT_FEW)
					{
						std::string szMsg;
						CGameBase::GetText(IDS_SMALL_COUNTABLE_ITEM_GET_MANY, &szMsg);
						CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);				
						return;
					}

					if ( (iGold * spItem->pItemBasic->iPrice)	> pInfoExt->iGold )	
					{
						std::string szMsg;
						CGameBase::GetText(IDS_COUNTABLE_ITEM_BUY_NOT_ENOUGH_MONEY, &szMsg);
						CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);				
						return;
					}

					iWeight = iGold * spItem->pItemBasic->siWeight;

					// Check weight.
					if ( (pInfoExt->iWeight + iWeight) > pInfoExt->iWeightMax)
					{	 
						std::string szMsg;
						CGameBase::GetText(IDS_ITEM_WEIGHT_OVERFLOW, &szMsg);	
						CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);	
						return;
					}
					break;
			}

			s_bWaitFromServer = true;

			if ( m_pMyTradeInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder] )	//If there is an icon at the specified position...
			{
				// Update the number.
				m_pMyTradeInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder]->iCount += iGold;

				// Display happens during icon rendering... in Inventory's Render.
				// Send to the server
				SendToServerBuyMsg(CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->dwID+
					CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemExt->dwID, 
					CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder, iGold);
			}
			else
			{
				spItem = m_pMyTrade[m_iCurPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder];
				CN3UIArea* pArea;
				spItemNew				= new __IconItemSkill;
				spItemNew->pItemBasic	= spItem->pItemBasic;
				spItemNew->pItemExt		= spItem->pItemExt;
				spItemNew->szIconFN		= spItem->szIconFN; // Copy the icon file name
				spItemNew->iCount		= iGold;
				spItemNew->iDurability  = spItem->pItemBasic->siMaxDurability+spItem->pItemExt->siMaxDurability;

				// Create the icon resource.
				spItemNew->pUIIcon = new CN3UIIcon;
				float fUVAspect		= (float)45.0f/(float)64.0f;
				spItemNew->pUIIcon->Init(this); 
				spItemNew->pUIIcon->SetTex(spItemNew->szIconFN);
				spItemNew->pUIIcon->SetUVRect(0,0, fUVAspect, fUVAspect);
				spItemNew->pUIIcon->SetUIType(UI_TYPE_ICON);
				spItemNew->pUIIcon->SetStyle(UISTYLE_ICON_ITEM|UISTYLE_ICON_CERTIFICATION_NEED);
				spItemNew->pUIIcon->SetVisible(true);
				pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder);
				if ( pArea )
				{
					spItemNew->pUIIcon->SetRegion(pArea->GetRegion());
					spItemNew->pUIIcon->SetMoveRect(pArea->GetRegion());
				}

				m_pMyTradeInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder] = spItemNew;

				// Send to the server
				SendToServerBuyMsg(CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->dwID+
					CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemExt->dwID, 
					CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder, iGold);
			}
			// Sound..
			if (CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic) PlayItemSound(CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic);
			break;

		case UIWND_DISTRICT_TRADE_MY:		// In case of selling
			spItem = m_pMyTradeInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder];

			if ( iGold <= 0 ) return;
			if ( iGold > spItem->iCount ) return;

			s_bWaitFromServer = true;

			if ( (spItem->iCount - iGold) > 0 )
			{	
				// Update number..
				spItem->iCount -= iGold;
			}
			else
			{
				spItem->pUIIcon->SetVisible(false);
			}

			// Send to the server..
			SendToServerSellMsg(CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->dwID+
				CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemExt->dwID, 
				CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder, iGold);
			break;
	}

	CN3UIWndBase::m_pCountableItemEdit->Close();
}

void CUITransactionDlg::ItemCountCancel()
{
	// Sound
	if (CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic) PlayItemSound(CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic);

	// Cancel
	s_bWaitFromServer				= false;
	m_sRecoveryJobInfo.pItemSource	= nullptr;
	m_sRecoveryJobInfo.pItemTarget	= nullptr;

	m_pCountableItemEdit->Close();
}

void CUITransactionDlg::SendToServerSellMsg(int itemID, byte pos, int iCount)
{
	uint8_t byBuff[32];
	int iOffset = 0;
	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_ITEM_TRADE);
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_TRADE_SELL);
	CAPISocket::MP_AddDword(byBuff, iOffset, itemID);	
	CAPISocket::MP_AddByte(byBuff, iOffset, pos);
	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)iCount);

	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
}

void CUITransactionDlg::SendToServerBuyMsg(int itemID, byte pos, int iCount)
{
	uint8_t byBuff[32];
	int iOffset = 0;
	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_ITEM_TRADE);
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_TRADE_BUY);
	CAPISocket::MP_AddDword(byBuff, iOffset, m_iTradeID);
	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)m_iNpcID);
	CAPISocket::MP_AddDword(byBuff, iOffset, itemID);	
	CAPISocket::MP_AddByte(byBuff, iOffset, pos);
	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)iCount);

	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
}

void CUITransactionDlg::SendToServerMoveMsg(int itemID, byte startpos, byte destpos)
{
	uint8_t byBuff[32];
	int iOffset = 0;
	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_ITEM_TRADE);
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_TRADE_MOVE);
	CAPISocket::MP_AddDword(byBuff, iOffset, itemID);	
	CAPISocket::MP_AddByte(byBuff, iOffset, startpos);
	CAPISocket::MP_AddByte(byBuff, iOffset, destpos);

	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
}

void CUITransactionDlg::ReceiveResultTradeMoveSuccess()
{
	s_bWaitFromServer = false;
	CN3UIWndBase::AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);
}

void CUITransactionDlg::ReceiveResultTradeMoveFail()
{
	CN3UIArea* pArea;
	s_bWaitFromServer = false;

	__IconItemSkill *spItemSource = nullptr, *spItemTarget = nullptr;
	spItemSource = CN3UIWndBase::m_sRecoveryJobInfo.pItemSource;
	spItemTarget = CN3UIWndBase::m_sRecoveryJobInfo.pItemTarget;

	if (spItemSource)
	{
		pArea = nullptr;
		pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder);
		if ( pArea )
		{
			spItemSource->pUIIcon->SetRegion(pArea->GetRegion());
			spItemSource->pUIIcon->SetMoveRect(pArea->GetRegion());
		}

		m_pMyTradeInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder] = spItemSource;
	}

	if (spItemTarget)
	{
		pArea = nullptr;
		pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder);
		if ( pArea )
		{
			spItemTarget->pUIIcon->SetRegion(pArea->GetRegion());
			spItemTarget->pUIIcon->SetMoveRect(pArea->GetRegion());
		}

		m_pMyTradeInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder] = spItemTarget;
	}
	else
	{
		m_pMyTradeInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder] = nullptr;
	}

	CN3UIWndBase::AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);
}

void CUITransactionDlg::ReceiveItemDropByTradeSuccess()
{
	// The original item should be deleted, 
	// but since there is no way to restore it, move it back to the original position 
	__IconItemSkill* spItem;
	spItem = CN3UIWndBase::m_sRecoveryJobInfo.pItemSource;

	CN3UIArea* pArea;

	pArea = CGameProcedure::s_pProcMain->m_pUIInventory->GetChildAreaByiOrder(UI_AREA_TYPE_INV, CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder);
	if ( pArea )
	{
		spItem->pUIIcon->SetRegion(pArea->GetRegion());
		spItem->pUIIcon->SetMoveRect(pArea->GetRegion());
	}

	// Set to invisible and delete only when the server confirms success
	spItem->pUIIcon->SetVisible(false);

	if( (spItem->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE) || (spItem->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL) )
	{
		// In case of items like bows or potions
		spItem->pUIIcon->SetVisible(true);
	}
}

bool CUITransactionDlg::ReceiveIconDrop(__IconItemSkill* spItem, POINT ptCur)
{
	// Temp Define 
	auto FAIL_RETURN = [&]() -> bool
		{
			CN3UIWndBase::AllHighLightIconFree();
			SetState(UI_STATE_COMMON_NONE);
			return false;
		};

	CN3UIArea* pArea;
	e_UIWND_DISTRICT eUIWnd = UIWND_DISTRICT_UNKNOWN;
	if (!m_bVisible) return false;

	// If it is not an icon I owned
	if (CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWnd != m_eUIWnd)
		FAIL_RETURN();

	if ( (CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWndDistrict != UIWND_DISTRICT_TRADE_NPC) &&
			(CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWndDistrict != UIWND_DISTRICT_TRADE_MY) )
		FAIL_RETURN();

	// If it is an icon I owned, check whether it is in the NPC area
	int iDestiOrder = -1; bool bFound = false;
	for(int i = 0; i < MAX_ITEM_TRADE; i++ )
	{
		pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_NPC, i);
		if ( (pArea) && (pArea->IsIn(ptCur.x, ptCur.y)) )
		{
			bFound = true;
			eUIWnd = UIWND_DISTRICT_TRADE_NPC;
			break;
		}
	}

	if (!bFound)
	{
		for(int i = 0; i < MAX_ITEM_INVENTORY; i++ )
		{
			pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, i);
			if ( (pArea) && (pArea->IsIn(ptCur.x, ptCur.y)) )
			{
				bFound = true;
				eUIWnd = UIWND_DISTRICT_TRADE_MY;
				break;
			}
		}
	}

	if (!bFound) FAIL_RETURN();

	//  Movement within the same window is a fail!
	if ((eUIWnd == CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWndDistrict) && 
		(eUIWnd != UIWND_DISTRICT_TRADE_MY))	
		FAIL_RETURN();

	// Start using Recovery Info in earnest
	// First, set WaitFromServer to On, then copy Select Info to Recovery Info 
	// Destination is not needed at this time.
	if ( spItem != CN3UIWndBase::m_sSelectedIconInfo.pItemSelect )
		CN3UIWndBase::m_sSelectedIconInfo.pItemSelect = spItem;

	s_bWaitFromServer									= true;
	m_sRecoveryJobInfo.pItemSource						= m_sSelectedIconInfo.pItemSelect;
	m_sRecoveryJobInfo.UIWndSourceStart.UIWnd			= m_sSelectedIconInfo.UIWndSelect.UIWnd;
	m_sRecoveryJobInfo.UIWndSourceStart.UIWndDistrict	= m_sSelectedIconInfo.UIWndSelect.UIWndDistrict;
	m_sRecoveryJobInfo.UIWndSourceStart.iOrder			= m_sSelectedIconInfo.UIWndSelect.iOrder;
	m_sRecoveryJobInfo.pItemTarget						= nullptr;

	m_sRecoveryJobInfo.UIWndSourceEnd.UIWnd				= UIWND_TRANSACTION;
	m_sRecoveryJobInfo.UIWndSourceEnd.UIWndDistrict		= eUIWnd;

	for(int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, i);
		if ( pArea && pArea->IsIn(ptCur.x, ptCur.y) )
		{
			iDestiOrder = i;
			break;
		}
	}

	switch (CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWndDistrict)
	{
		case UIWND_DISTRICT_TRADE_NPC:
			if (eUIWnd == UIWND_DISTRICT_TRADE_MY)		// In case of buying
			{
				if( (CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE) ||
					(CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL) )
				{
					// In case of items like bows or potions
					// First, check if the icon exists in the inventory.
					bFound = false;

					for(int i = 0; i < MAX_ITEM_INVENTORY; i++ )
					{
						if ( bFound )
						{
							CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder	= iDestiOrder;
							break;
						}

						if( (m_pMyTradeInv[i]) && (m_pMyTradeInv[i]->pItemBasic->dwID == CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pItemBasic->dwID) &&
							(m_pMyTradeInv[i]->pItemExt->dwID == CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pItemExt->dwID) )
						{
							bFound = true;
							iDestiOrder = i;
						}
					}

					// If not found 
					if ( !bFound )
					{
						if ( m_pMyTradeInv[iDestiOrder] )	// If there is an icon at the specified position
						{
							// Find an empty slot in the inventory and insert it
							for(int i = 0; i < MAX_ITEM_INVENTORY; i++ )
							{
								if ( !m_pMyTradeInv[i] )
								{
									bFound = true;
									iDestiOrder = i;
									break;
								}
							}

							if ( !bFound )	// If an empty slot was not found
							{
								s_bWaitFromServer				= false;
								m_sRecoveryJobInfo.pItemSource	= nullptr;
								m_sRecoveryJobInfo.pItemTarget	= nullptr;
								FAIL_RETURN();
							}
						}
					}

					m_sRecoveryJobInfo.UIWndSourceEnd.iOrder	= iDestiOrder;
					s_bWaitFromServer							= false;

					m_pCountableItemEdit->Open(UIWND_TRANSACTION, m_sSelectedIconInfo.UIWndSelect.UIWndDistrict, false);
					FAIL_RETURN();
				}
				else
				{
					__InfoPlayerMySelf*	pInfoExt = &(CGameBase::s_pPlayer->m_InfoExt);

					// If the purchase quantity X is more than the money I have... just return.
					if ( (CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->iPrice)	> pInfoExt->iGold )	
					{
						std::string szMsg;
						CGameBase::GetText(IDS_COUNTABLE_ITEM_BUY_NOT_ENOUGH_MONEY, &szMsg);
						CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);	

						s_bWaitFromServer				= false;
						m_sRecoveryJobInfo.pItemSource	= nullptr;
						m_sRecoveryJobInfo.pItemTarget	= nullptr;
						FAIL_RETURN();
					}

					// Weight check.
					if ( (pInfoExt->iWeight + CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->siWeight) > pInfoExt->iWeightMax)
					{	 
						std::string szMsg;
						CGameBase::GetText(IDS_ITEM_WEIGHT_OVERFLOW, &szMsg);	
						CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);	

						s_bWaitFromServer				= false;
						m_sRecoveryJobInfo.pItemSource	= nullptr;
						m_sRecoveryJobInfo.pItemTarget	= nullptr;
						FAIL_RETURN();
					}

					// In case of a normal item...
					if ( m_pMyTradeInv[iDestiOrder] )	// If there is an icon at the specified position...
					{
						// Find an empty slot in the inventory and place it there...
						bFound = false;
						for(int i = 0; i < MAX_ITEM_INVENTORY; i++ )
						{
							if ( !m_pMyTradeInv[i] )
							{
								bFound = true;
								iDestiOrder = i;
								break;
							}
						}

						if ( !bFound )	// If no empty slot is found...
						{
							s_bWaitFromServer				= false;
							m_sRecoveryJobInfo.pItemSource	= nullptr;
							m_sRecoveryJobInfo.pItemTarget	= nullptr;
							FAIL_RETURN();
						}

						CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder	= iDestiOrder;
					}
					else
						CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder	= iDestiOrder;

					SendToServerBuyMsg(CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->dwID+
						CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemExt->dwID, iDestiOrder, 
						CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->iCount);

					std::string szIconFN;
					e_PartPosition ePart;
					e_PlugPosition ePlug;
					CGameProcedure::MakeResrcFileNameForUPC(m_pMyTrade[m_iCurPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder]->pItemBasic, 
						nullptr, &szIconFN, ePart, ePlug); // Create the file name according to the item

					__IconItemSkill* spItemNew;
					spItemNew				= new __IconItemSkill;
					spItemNew->pItemBasic	= m_pMyTrade[m_iCurPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder]->pItemBasic;
					spItemNew->pItemExt		= m_pMyTrade[m_iCurPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder]->pItemExt;
					spItemNew->szIconFN		= szIconFN; // Copy the icon file name.
					spItemNew->iCount		= 1;
					spItemNew->iDurability	= m_pMyTrade[m_iCurPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder]->pItemBasic->siMaxDurability
						+m_pMyTrade[m_iCurPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder]->pItemExt->siMaxDurability;

					// Create the icon resource.
					spItemNew->pUIIcon = new CN3UIIcon;
					float fUVAspect		= (float)45.0f/(float)64.0f;
					spItemNew->pUIIcon->Init(this); 
					spItemNew->pUIIcon->SetTex(szIconFN);
					spItemNew->pUIIcon->SetUVRect(0,0, fUVAspect, fUVAspect);
					spItemNew->pUIIcon->SetUIType(UI_TYPE_ICON);
					spItemNew->pUIIcon->SetStyle(UISTYLE_ICON_ITEM|UISTYLE_ICON_CERTIFICATION_NEED);
					spItemNew->pUIIcon->SetVisible(true);
					pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, iDestiOrder);
					if ( pArea )
					{
						spItemNew->pUIIcon->SetRegion(pArea->GetRegion());
						spItemNew->pUIIcon->SetMoveRect(pArea->GetRegion());
					}

					m_pMyTradeInv[iDestiOrder] = spItemNew;
					FAIL_RETURN();
				}
			}
			else
			{
				s_bWaitFromServer				= false;
				m_sRecoveryJobInfo.pItemSource	= nullptr;
				m_sRecoveryJobInfo.pItemTarget	= nullptr;
				FAIL_RETURN();
			}
			break;

		case UIWND_DISTRICT_TRADE_MY:
			if (eUIWnd == UIWND_DISTRICT_TRADE_NPC)		// In case of selling...
			{
				if( (CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE) ||
					(CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL) )
				{
					// In case of items like bows or potions
					s_bWaitFromServer = false;
					m_pCountableItemEdit->Open(UIWND_TRANSACTION, m_sSelectedIconInfo.UIWndSelect.UIWndDistrict, false);
				}
				else
				{
					// Send to the server.
					SendToServerSellMsg(CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->dwID+
						CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemExt->dwID, 
						CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder, 
						CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->iCount);

					// The original item should be deleted, but since there is no way to restore it,
					// move it back to the original position
					pArea = nullptr;
					pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder);
					if ( pArea )
					{
						spItem->pUIIcon->SetRegion(pArea->GetRegion());
						spItem->pUIIcon->SetMoveRect(pArea->GetRegion());
					}

					// Set to invisible and delete only when the server confirms success.
					spItem->pUIIcon->SetVisible(false);
				}
				FAIL_RETURN();
			}
			else	
			{
				// Move 
				__IconItemSkill *spItemSource, *spItemTarget = nullptr;
				spItemSource = CN3UIWndBase::m_sRecoveryJobInfo.pItemSource;

				pArea = nullptr;
				pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, iDestiOrder);
				if ( pArea )
				{
					spItemSource->pUIIcon->SetRegion(pArea->GetRegion());
					spItemSource->pUIIcon->SetMoveRect(pArea->GetRegion());
				}

				CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder				= iDestiOrder;
				if ( m_pMyTradeInv[iDestiOrder] )	// If there is an icon at the specified position
				{
					CN3UIWndBase::m_sRecoveryJobInfo.pItemTarget = m_pMyTradeInv[iDestiOrder];
					CN3UIWndBase::m_sRecoveryJobInfo.UIWndTargetStart.UIWnd	= UIWND_TRANSACTION;
					CN3UIWndBase::m_sRecoveryJobInfo.UIWndTargetStart.UIWndDistrict = UIWND_DISTRICT_TRADE_MY;
					CN3UIWndBase::m_sRecoveryJobInfo.UIWndTargetStart.iOrder = iDestiOrder;
					CN3UIWndBase::m_sRecoveryJobInfo.UIWndTargetEnd.UIWnd = UIWND_TRANSACTION;
					CN3UIWndBase::m_sRecoveryJobInfo.UIWndTargetEnd.UIWndDistrict = UIWND_DISTRICT_TRADE_MY;
					CN3UIWndBase::m_sRecoveryJobInfo.UIWndTargetEnd.iOrder = CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder;

					spItemTarget = CN3UIWndBase::m_sRecoveryJobInfo.pItemTarget;

					pArea = nullptr;
					pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder);
					if ( pArea )
					{
						spItemTarget->pUIIcon->SetRegion(pArea->GetRegion());
						spItemTarget->pUIIcon->SetMoveRect(pArea->GetRegion());
					}
				}
				else
					CN3UIWndBase::m_sRecoveryJobInfo.pItemTarget = nullptr;

				m_pMyTradeInv[iDestiOrder] = spItemSource;
				m_pMyTradeInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder] = spItemTarget;

				SendToServerMoveMsg(CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->dwID+
						CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemExt->dwID, 
						CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder, 
						iDestiOrder );
				//TRACE("Source %d, Target %d \n", CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder, iDestiOrder);
				FAIL_RETURN();
			}				
			break;
		}

	CN3UIWndBase::AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);

	return false;
}

void CUITransactionDlg::ReceiveResultTradeFromServer(byte bResult, byte bType, int	iMoney)
{
	s_bWaitFromServer = false;
	__IconItemSkill* spItem = nullptr;
	__InfoPlayerMySelf*	pInfoExt = &(CGameBase::s_pPlayer->m_InfoExt);

	// If the source area is UIWND_DISTRICT_TRADE_NPC, it means buying an item
	switch ( CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.UIWndDistrict )
	{
		case UIWND_DISTRICT_TRADE_NPC:
			if ( bResult != 0x01 )	// If it is a failure
			{	
				if( (CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE) ||
					(CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL) )
				{
					int iGold = CN3UIWndBase::m_pCountableItemEdit->GetQuantity();

					if ( (m_pMyTradeInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder]->iCount - iGold) > 0 )
					{	
						// Update the number
						m_pMyTradeInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder]->iCount -= iGold;
					}
					else
					{
						// Delete item... only in the current inventory window.
						__IconItemSkill* spItem;
						spItem = m_pMyTradeInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder];

						// Also remove it from the inventory.
						m_pMyTradeInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder] = nullptr;

						// Remove my manager's item from the list using iOrder.
						RemoveChild(spItem->pUIIcon);

						// Delete the icon resource
						spItem->pUIIcon->Release();
						delete spItem->pUIIcon;
						spItem->pUIIcon = nullptr;
						delete spItem;
						spItem = nullptr;
					}
				}
				else
				{
					// Delete the item... only from the current inventory window
					__IconItemSkill* spItem;
					spItem = m_pMyTradeInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder];

					// Also delete it from the inventory
					m_pMyTradeInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder] = nullptr;

					// Delete my manager's item from the list using iOrder.
					RemoveChild(spItem->pUIIcon);

					// Delete the icon resource.
					spItem->pUIIcon->Release();
					delete spItem->pUIIcon;
					spItem->pUIIcon = nullptr;
					delete spItem;
					spItem = nullptr;
				}

				if (bType == 0x04)
				{
					std::string szMsg;
					CGameBase::GetText(IDS_ITEM_TOOMANY_OR_HEAVY, &szMsg);
					CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);
				}
			}
			else
			{
				pInfoExt->iGold = iMoney;

				CGameProcedure::s_pProcMain->m_pUIInventory->GoldUpdate();
				GoldUpdate();
			}
			
			CN3UIWndBase::AllHighLightIconFree();
			SetState(UI_STATE_COMMON_NONE);
			break;

		case UIWND_DISTRICT_TRADE_MY:
			if ( bResult != 0x01 )	// If it fails 
			{	
				if( (CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE) ||
					(CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL) )
				{
					int iGold = CN3UIWndBase::m_pCountableItemEdit->GetQuantity();

					if (m_pMyTradeInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder]->pUIIcon->IsVisible()) // 기존 아이콘이 보인다면..
					{
						// Only update the number.
						m_pMyTradeInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder]->iCount += iGold;
					}
					else
					{
						// If the existing icon is not visible
						m_pMyTradeInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder]->iCount = iGold;

						// Make the icon visible
						m_pMyTradeInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder]->pUIIcon->SetVisible(true);
					}
				}
				else
				{
					// Change from Invisible to Icon Visible.					
					spItem = m_pMyTradeInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder];
					spItem->pUIIcon->SetVisible(true);
				}
			}
			else
			{
				// In case of items like bows or potions, if the existing icon is not visible... delete the item.
				if( ( ((CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE) || 
					(CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL))
					&&	!m_pMyTradeInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder]->pUIIcon->IsVisible()) ||
					((CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->byContable != UIITEM_TYPE_COUNTABLE) &&
					(CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->byContable != UIITEM_TYPE_COUNTABLE_SMALL)) )
				{
					// Delete the item. only in my current area window.
					__IconItemSkill* spItem;
					spItem = m_pMyTradeInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder];

					// Also delete it from my area.
					m_pMyTradeInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder] = nullptr;

					// Remove my manager's item from the list using iOrder.
					RemoveChild(spItem->pUIIcon);

					// Delete the icon resource.
					spItem->pUIIcon->Release();
					delete spItem->pUIIcon;
					spItem->pUIIcon = nullptr;
					delete spItem;
					spItem = nullptr;
				}

				// If successful... update the money.
				pInfoExt->iGold = iMoney;

				CGameProcedure::s_pProcMain->m_pUIInventory->GoldUpdate();
				GoldUpdate();
			}

			CN3UIWndBase::AllHighLightIconFree();
			SetState(UI_STATE_COMMON_NONE);
			break;
	}
}

void CUITransactionDlg::CancelIconDrop(__IconItemSkill* spItem)
{
	CN3UIWndBase::AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);
}

void CUITransactionDlg::AcceptIconDrop(__IconItemSkill* spItem)
{
	CN3UIWndBase::AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);
}

void CUITransactionDlg::IconRestore()
{
	CN3UIArea* pArea;

	switch ( CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWndDistrict )
	{
		case UIWND_DISTRICT_TRADE_NPC:
			if ( m_pMyTrade[m_iCurPage][CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder] != nullptr)
			{
				pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_NPC, CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder);
				if ( pArea )
				{
					m_pMyTrade[m_iCurPage][CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder]->pUIIcon->SetRegion(pArea->GetRegion());
					m_pMyTrade[m_iCurPage][CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder]->pUIIcon->SetMoveRect(pArea->GetRegion());
				}
			}
			break;

		case UIWND_DISTRICT_TRADE_MY:
			if ( m_pMyTradeInv[CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder] != nullptr)
			{
				pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder);
				if ( pArea )
				{
					m_pMyTradeInv[CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder]->pUIIcon->SetRegion(pArea->GetRegion());
					m_pMyTradeInv[CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder]->pUIIcon->SetMoveRect(pArea->GetRegion());
				}
			}
			break;
	}
}

uint32_t CUITransactionDlg::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (!m_bVisible) return dwRet;
	if (s_bWaitFromServer) { dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);  return dwRet; }

	// Update the dragged icon.
	if ( (GetState() == UI_STATE_ICON_MOVING) && 
			(CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWnd == UIWND_TRANSACTION) )
	{
		CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pUIIcon->SetRegion(GetSampleRect());
		CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pUIIcon->SetMoveRect(GetSampleRect());
	}

	return CN3UIWndBase::MouseProc(dwFlags, ptCur, ptOld);
}

int CUITransactionDlg::GetItemiOrder(__IconItemSkill* spItem, e_UIWND_DISTRICT eWndDist)
{
	int iReturn = -1;
	int i;

	switch ( eWndDist )
	{
		case UIWND_DISTRICT_TRADE_NPC:
			for( i = 0; i < MAX_ITEM_TRADE; i++ )
			{
				if ( (m_pMyTrade[m_iCurPage][i] != nullptr) && (m_pMyTrade[m_iCurPage][i] == spItem) )
					return i;
			}
			break;

		case UIWND_DISTRICT_TRADE_MY:
			for( i = 0; i < MAX_ITEM_INVENTORY; i++ )
			{
				if ( (m_pMyTradeInv[i] != nullptr) && (m_pMyTradeInv[i] == spItem) )
					return i;
			}
			break;
	}

	return iReturn;
}

RECT CUITransactionDlg::GetSampleRect()
{
	RECT rect;
	CN3UIArea* pArea;
	POINT ptCur = CGameProcedure::s_pLocalInput->MouseGetPos();
	pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_NPC, 0);
	rect = pArea->GetRegion();
	float fWidth = (float)(rect.right - rect.left);
	float fHeight = (float)(rect.bottom - rect.top);
	fWidth *= 0.5f; fHeight *= 0.5f;
	rect.left = ptCur.x - (int)fWidth;  rect.right  = ptCur.x + (int)fWidth;
	rect.top  = ptCur.y - (int)fHeight; rect.bottom = ptCur.y + (int)fHeight;
	return rect;
}

e_UIWND_DISTRICT CUITransactionDlg::GetWndDistrict(__IconItemSkill* spItem)
{
	for( int i = 0; i < MAX_ITEM_TRADE; i++ )
	{
		if ( (m_pMyTrade[m_iCurPage][i] != nullptr) && (m_pMyTrade[m_iCurPage][i] == spItem) )
			return UIWND_DISTRICT_TRADE_NPC;
	}

	for(int i = 0; i < MAX_ITEM_INVENTORY; i++ )
	{
		if ( (m_pMyTradeInv[i] != nullptr) && (m_pMyTradeInv[i] == spItem) )
			return UIWND_DISTRICT_TRADE_MY;
	}
	return UIWND_DISTRICT_UNKNOWN;
}

bool CUITransactionDlg::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
// Temp Define
#define FAIL_CODE {		\
				SetState(UI_STATE_COMMON_NONE);	\
				return false;	\
			}

	if(pSender == nullptr) return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)					
	{
		if(pSender == m_pBtnClose)
			LeaveTransactionState();

		CN3UIString* pStr;

		if(pSender == m_pBtnPageUp)
		{
			m_iCurPage--;
			if(m_iCurPage<0) m_iCurPage = 0;

			pStr = (CN3UIString* )GetChildByID("string_page");
			if (pStr)
			{
				char pszID[32];
				sprintf(pszID, "%d",m_iCurPage+1);
				pStr->SetString(pszID);
			}

			for(int i = 0; i < MAX_ITEM_TRADE_PAGE; i++ )
			{
				if (i == m_iCurPage)
				{
					for(int j = 0; j < MAX_ITEM_TRADE; j++ )
					{
						if ( m_pMyTrade[i][j] != nullptr )
							m_pMyTrade[i][j]->pUIIcon->SetVisible(true);
					}	
				}
				else
				{
					for(int j = 0; j < MAX_ITEM_TRADE; j++ )
					{
						if ( m_pMyTrade[i][j] != nullptr )
							m_pMyTrade[i][j]->pUIIcon->SetVisible(false);
					}	
				}
			}
		}

		if(pSender == m_pBtnPageDown)
		{
			m_iCurPage++;
			if (m_iCurPage >= MAX_ITEM_TRADE_PAGE)
				m_iCurPage = MAX_ITEM_TRADE_PAGE-1;

			pStr = (CN3UIString* )GetChildByID("string_page");
			if (pStr)
			{
				char pszID[32];
				sprintf(pszID, "%d",m_iCurPage+1);
				pStr->SetString(pszID);
			}

			for(int i = 0; i < MAX_ITEM_TRADE_PAGE; i++)
			{
				if (i == m_iCurPage)
				{
					for(int j = 0; j < MAX_ITEM_TRADE; j++ )
					{
						if ( m_pMyTrade[i][j] != nullptr )
							m_pMyTrade[i][j]->pUIIcon->SetVisible(true);
					}	
				}
				else
				{
					for(int j = 0; j < MAX_ITEM_TRADE; j++ )
					{
						if ( m_pMyTrade[i][j] != nullptr )
							m_pMyTrade[i][j]->pUIIcon->SetVisible(false);
					}	
				}
			}
		}
	}

	__IconItemSkill* spItem = nullptr;
	e_UIWND_DISTRICT eUIWnd;
	int iOrder;

	uint32_t dwBitMask = 0x000f0000;

	switch (dwMsg & dwBitMask)
	{
		case UIMSG_ICON_DOWN_FIRST:
			CN3UIWndBase::AllHighLightIconFree();

			// Get Item..
			spItem = GetHighlightIconItem((CN3UIIcon* )pSender);

			// Save Select Info..
			CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWnd = UIWND_TRANSACTION;
			eUIWnd = GetWndDistrict(spItem);
			if ( eUIWnd == UIWND_DISTRICT_UNKNOWN )	FAIL_CODE
			CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWndDistrict = eUIWnd;
			iOrder = GetItemiOrder(spItem, eUIWnd);
			if ( iOrder == -1 )	FAIL_CODE
			CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder = iOrder;
			CN3UIWndBase::m_sSelectedIconInfo.pItemSelect = spItem;
			// Do Ops..
			((CN3UIIcon* )pSender)->SetRegion(GetSampleRect());
			((CN3UIIcon* )pSender)->SetMoveRect(GetSampleRect());
			// Sound..
			if (spItem) PlayItemSound(spItem->pItemBasic);
			break;

		case UIMSG_ICON_UP:
			// Iterate through icon manager windows and check...
			if ( !CGameProcedure::s_pUIMgr->BroadcastIconDropMsg(CN3UIWndBase::m_sSelectedIconInfo.pItemSelect) )
				// Reset icon position to original.
				IconRestore();
			// Sound..
			if (CN3UIWndBase::m_sSelectedIconInfo.pItemSelect) PlayItemSound(CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pItemBasic);
			break;

		case UIMSG_ICON_DOWN:
			if ( GetState()  == UI_STATE_ICON_MOVING )
			{
				CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pUIIcon->SetRegion(GetSampleRect());
				CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pUIIcon->SetMoveRect(GetSampleRect());
			}
			break;
	}

	return true;
}

bool CUITransactionDlg::OnMouseWheelEvent(
	short delta)
{
	if (delta > 0)
		ReceiveMessage(m_pBtnPageUp, UIMSG_BUTTON_CLICK);
	else
		ReceiveMessage(m_pBtnPageDown, UIMSG_BUTTON_CLICK);

	return true;
}

CN3UIBase* CUITransactionDlg::GetChildButtonByName(const std::string& szFN)
{
	for(UIListItor itor = m_Children.begin(); m_Children.end() != itor; ++itor)
	{
		CN3UIBase* pChild = (CN3UIBase* )(*itor);
		if ( (pChild->UIType() == UI_TYPE_BUTTON) && (szFN.compare(pChild->m_szID) == 0) )
			return pChild;
	}

	return nullptr;
}

void CUITransactionDlg::ShowTitle(e_NpcTrade eNT)
{
	m_pUIInn->SetVisible(false);
	m_pUIBlackSmith->SetVisible(false);
	m_pUIStore->SetVisible(false);

	switch (eNT)
	{
		case UI_BLACKSMITH:
			m_pUIBlackSmith->SetVisible(true);
			break;
		case UI_STORE:
			m_pUIStore->SetVisible(true);
			break;
		case UI_INN:
			m_pUIInn->SetVisible(true);
			break;
	}
}

//this_ui_add_start
void CUITransactionDlg::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);
	if(bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
	{
		if(CN3UIWndBase::m_pCountableItemEdit && CN3UIWndBase::m_pCountableItemEdit->IsVisible())
			ItemCountCancel();

		CGameProcedure::s_pUIMgr->ReFocusUI();//this_ui
	}
}

void CUITransactionDlg::SetVisibleWithNoSound(bool bVisible, bool bWork, bool bReFocus)
{
	CN3UIBase::SetVisibleWithNoSound(bVisible, bWork, bReFocus);

	if(bWork && !bVisible)
	{
		if(CN3UIWndBase::m_pCountableItemEdit && CN3UIWndBase::m_pCountableItemEdit->IsVisible())
			ItemCountCancel();

		if (GetState() == UI_STATE_ICON_MOVING)
			IconRestore();
		SetState(UI_STATE_COMMON_NONE);
		CN3UIWndBase::AllHighLightIconFree();

		// Move the items from the inv area of this window to the inv area of this inventory window.
		ItemMoveFromThisToInv();

		if (CGameProcedure::s_pProcMain->m_pUISkillTreeDlg) CGameProcedure::s_pProcMain->m_pUISkillTreeDlg->UpdateDisableCheck();
		if (CGameProcedure::s_pProcMain->m_pUIHotKeyDlg) CGameProcedure::s_pProcMain->m_pUIHotKeyDlg->UpdateDisableCheck();
		if(m_pUITooltipDlg) m_pUITooltipDlg->DisplayTooltipsDisable();
	}
}

bool CUITransactionDlg::Load(HANDLE hFile)
{
	if(CN3UIBase::Load(hFile)==false) return false;

	N3_VERIFY_UI_COMPONENT(m_pBtnClose, (CN3UIButton*) GetChildByID("btn_close"));
	N3_VERIFY_UI_COMPONENT(m_pBtnPageUp, (CN3UIButton*) GetChildByID("btn_page_up"));
	N3_VERIFY_UI_COMPONENT(m_pBtnPageDown, (CN3UIButton*) GetChildByID("btn_page_down"));

	return true;
}

bool CUITransactionDlg::OnKeyPress(int iKey)
{
	switch(iKey)
	{
	case DIK_PRIOR:
		ReceiveMessage(m_pBtnPageUp, UIMSG_BUTTON_CLICK);
		return true;
	case DIK_NEXT:
		ReceiveMessage(m_pBtnPageDown, UIMSG_BUTTON_CLICK);
		return true;
	case DIK_ESCAPE:
		ReceiveMessage(m_pBtnClose, UIMSG_BUTTON_CLICK);
		if(m_pUITooltipDlg) m_pUITooltipDlg->DisplayTooltipsDisable();
		return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}
//this_ui_add_end

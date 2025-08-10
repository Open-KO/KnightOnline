// UIWareHouseDlg.cpp: implementation of the UIWareHouseDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
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

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUIWareHouseDlg::CUIWareHouseDlg()
{
	int i, j;
	m_iCurPage = 0;
	for( j = 0; j < MAX_ITEM_WARE_PAGE; j++ )
		for( i = 0; i < MAX_ITEM_TRADE; i++ )		
			m_pMyWare[j][i] = NULL;
	for( i = 0; i < MAX_ITEM_INVENTORY; i++ )	m_pMyWareInv[i] = NULL;

	m_pUITooltipDlg		= NULL;
	m_pStrMyGold		= NULL;
	m_pStrWareGold		= NULL;

	m_pBtnGold			= NULL;
	m_pBtnGoldWareHouse = NULL;
	m_pBtnClose			= NULL;
	m_pBtnPageUp		= NULL;
	m_pBtnPageDown		= NULL;

	m_bSendedItemGold = false;
	m_iGoldOffsetBackup = 0;

	this->SetVisible(false);
}

CUIWareHouseDlg::~CUIWareHouseDlg()
{
	Release();
}

void CUIWareHouseDlg::Release()
{
	CN3UIBase::Release();

	int i, j;
	for( j = 0; j < MAX_ITEM_WARE_PAGE; j++ )
		for( i = 0; i < MAX_ITEM_TRADE; i++ )
		{
			if ( m_pMyWare[j][i] != NULL )
			{
				delete m_pMyWare[j][i];
				m_pMyWare[j][i] = NULL;
			}
		}

	for( i = 0; i < MAX_ITEM_INVENTORY; i++ )
	{
		if ( m_pMyWareInv[i] != NULL )
		{
			delete m_pMyWareInv[i];
			m_pMyWareInv[i] = NULL;
		}
	}
}

void CUIWareHouseDlg::Render()
{
	if (!m_bVisible) return;	// 보이지 않으면 자식들을 render하지 않는다.

	int i;

	POINT ptCur = CGameProcedure::s_pLocalInput->MouseGetPos();
	m_pUITooltipDlg->DisplayTooltipsDisable();

	bool bTooltipRender = false;
	__IconItemSkill* spItem = NULL;

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

	// 갯수 표시되야 할 아이템 갯수 표시..
	for( i = 0; i < MAX_ITEM_TRADE; i++ )
	{
		if ( m_pMyWare[m_iCurPage][i] && ( (m_pMyWare[m_iCurPage][i]->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE) || 
			(m_pMyWare[m_iCurPage][i]->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL) ) )
		{
			// string 얻기..
			CN3UIString* pStr = GetChildStringByiOrder(i+100);
			if(pStr) 
			{
				if ( (GetState() == UI_STATE_ICON_MOVING) && (m_pMyWare[m_iCurPage][i] == CN3UIWndBase::m_sSelectedIconInfo.pItemSelect) )
				{
					pStr->SetVisible(false);
				}
				else
				{
					if ( m_pMyWare[m_iCurPage][i]->pUIIcon->IsVisible() )
					{
						pStr->SetVisible(true);
						pStr->SetStringAsInt(m_pMyWare[m_iCurPage][i]->iCount);
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
			CN3UIString* pStr = GetChildStringByiOrder(i+100);
			if(pStr) 
				pStr->SetVisible(false);
		}
	}

	for( i = 0; i < MAX_ITEM_INVENTORY; i++ )
	{
		if ( m_pMyWareInv[i] && ( (m_pMyWareInv[i]->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE)  || (m_pMyWareInv[i]->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL)  ) )
		{
			// string 얻기..
			CN3UIString* pStr = GetChildStringByiOrder(i);
			if(pStr) 
			{
				if ( (GetState() == UI_STATE_ICON_MOVING) && (m_pMyWareInv[i] == CN3UIWndBase::m_sSelectedIconInfo.pItemSelect) )
				{
					pStr->SetVisible(false);
				}
				else
				{
					if ( m_pMyWareInv[i]->pUIIcon->IsVisible() )
					{
						pStr->SetVisible(true);
						pStr->SetStringAsInt(m_pMyWareInv[i]->iCount);
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
			if(pStr) 
				pStr->SetVisible(false);
		}
	}

	if ( (GetState() == UI_STATE_ICON_MOVING) && (CN3UIWndBase::m_sSelectedIconInfo.pItemSelect) ) 
		CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pUIIcon->Render();		

	if ( bTooltipRender && spItem )
		m_pUITooltipDlg->DisplayTooltipsEnable(ptCur.x, ptCur.y, spItem);
}

void CUIWareHouseDlg::InitIconWnd(e_UIWND eWnd)
{
	__TABLE_UI_RESRC* pTbl = CGameBase::s_pTbl_UI.Find(CGameBase::s_pPlayer->m_InfoBase.eNation);

	m_pUITooltipDlg = new CUIImageTooltipDlg();
	m_pUITooltipDlg->Init(this);
	m_pUITooltipDlg->LoadFromFile(pTbl->szItemInfo);
	m_pUITooltipDlg->InitPos();
	m_pUITooltipDlg->SetVisible(FALSE);	

	CN3UIWndBase::InitIconWnd(eWnd);

	m_pStrWareGold    = (CN3UIString* )GetChildByID("string_wareitem_name"); __ASSERT(m_pStrWareGold, "NULL UI Component!!");
	if(m_pStrWareGold) m_pStrWareGold->SetString("0");
	m_pStrMyGold    = (CN3UIString* )GetChildByID("string_item_name"); __ASSERT(m_pStrMyGold, "NULL UI Component!!");
	if(m_pStrMyGold) m_pStrMyGold->SetString("0");
}

void CUIWareHouseDlg::InitIconUpdate()
{
	CN3UIArea* pArea;
	float fUVAspect = (float)45.0f/(float)64.0f;
	int i, j;

	for( j = 0; j < MAX_ITEM_WARE_PAGE; j++ )
		for( i = 0; i < MAX_ITEM_TRADE; i++ )
		{
			if ( m_pMyWare[j][i] != NULL )
			{
				m_pMyWare[j][i]->pUIIcon = new CN3UIIcon;
				m_pMyWare[j][i]->pUIIcon->Init(this);
				m_pMyWare[j][i]->pUIIcon->SetTex(m_pMyWare[j][i]->szIconFN);
				m_pMyWare[j][i]->pUIIcon->SetUVRect(0,0,fUVAspect,fUVAspect);
				m_pMyWare[j][i]->pUIIcon->SetUIType(UI_TYPE_ICON);
				m_pMyWare[j][i]->pUIIcon->SetStyle(UISTYLE_ICON_ITEM|UISTYLE_ICON_CERTIFICATION_NEED);
				pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_NPC, i);
				if ( pArea )
				{
					m_pMyWare[j][i]->pUIIcon->SetRegion(pArea->GetRegion());
					m_pMyWare[j][i]->pUIIcon->SetMoveRect(pArea->GetRegion());
				}
			}
		}
}

int CUIWareHouseDlg::GetItemiOrder(__IconItemSkill* spItem, e_UIWND_DISTRICT eWndDist)
{
	int iReturn = -1;
	int i;

	switch ( eWndDist )
	{
		case UIWND_DISTRICT_TRADE_NPC:
			for( i = 0; i < MAX_ITEM_TRADE; i++ )
			{
				if ( (m_pMyWare[m_iCurPage][i] != NULL) && (m_pMyWare[m_iCurPage][i] == spItem) )
					return i;
			}
			break;

		case UIWND_DISTRICT_TRADE_MY:
			for( i = 0; i < MAX_ITEM_INVENTORY; i++ )
			{
				if ( (m_pMyWareInv[i] != NULL) && (m_pMyWareInv[i] == spItem) )
					return i;
			}
			break;
	}

	return iReturn;
}

RECT CUIWareHouseDlg::GetSampleRect()
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

e_UIWND_DISTRICT CUIWareHouseDlg::GetWndDistrict(__IconItemSkill* spItem)
{
	for( int i = 0; i < MAX_ITEM_TRADE; i++ )
	{
		if ( (m_pMyWare[m_iCurPage][i] != NULL) && (m_pMyWare[m_iCurPage][i] == spItem) )
			return UIWND_DISTRICT_TRADE_NPC;
	}

	for(int i = 0; i < MAX_ITEM_INVENTORY; i++ )
	{
		if ( (m_pMyWareInv[i] != NULL) && (m_pMyWareInv[i] == spItem) )
			return UIWND_DISTRICT_TRADE_MY;
	}
	return UIWND_DISTRICT_UNKNOWN;
}

uint32_t CUIWareHouseDlg::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (!m_bVisible) return dwRet;
	if (s_bWaitFromServer) { dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);  return dwRet; }

	// 드래그 되는 아이콘 갱신..
	if ( (GetState() == UI_STATE_ICON_MOVING) && 
			(CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWnd == UIWND_WARE_HOUSE) )
	{
		CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pUIIcon->SetRegion(GetSampleRect());
		CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pUIIcon->SetMoveRect(GetSampleRect());
	}

	return CN3UIWndBase::MouseProc(dwFlags, ptCur, ptOld);
}

bool CUIWareHouseDlg::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
// Temp Define
#define FAIL_CODE {		\
				SetState(UI_STATE_COMMON_NONE);	\
				return false;	\
			}

	if(NULL == pSender) return false;
	int i, j;

	if (dwMsg == UIMSG_BUTTON_CLICK)					
	{
		if(pSender == m_pBtnGold)
		{
			// 인벤토리만 떠 있을때..
			CN3UIWndBase::m_pCountableItemEdit->Open(UIWND_WARE_HOUSE, UIWND_DISTRICT_TRADE_MY, true, true);
			return true;
		}

		if(pSender == m_pBtnGoldWareHouse)
		{
			// 인벤토리만 떠 있을때..
			CN3UIWndBase::m_pCountableItemEdit->Open(UIWND_WARE_HOUSE, UIWND_DISTRICT_TRADE_NPC, true, true);
			return true;
		}

		if(pSender == m_pBtnClose)
			LeaveWareHouseState();

		CN3UIString* pStr;

		if(pSender == m_pBtnPageUp)
		{
			m_iCurPage--;
			if(m_iCurPage<0)
				m_iCurPage = 0;

			pStr = (CN3UIString* )GetChildByID("string_page");
			if (pStr)
			{
				std::string pageNo = std::to_string(m_iCurPage + 1);
				pStr->SetString(pageNo);
			}

			for( j = 0; j < MAX_ITEM_WARE_PAGE; j++ )
			{
				if (j == m_iCurPage)
				{
					for( i = 0; i < MAX_ITEM_TRADE; i++ )
					{
						if ( m_pMyWare[j][i] != NULL )
							m_pMyWare[j][i]->pUIIcon->SetVisible(true);
					}	
				}
				else
				{
					for( i = 0; i < MAX_ITEM_TRADE; i++ )
					{
						if ( m_pMyWare[j][i] != NULL )
							m_pMyWare[j][i]->pUIIcon->SetVisible(false);
					}	
				}
			}
		}

		if(pSender == m_pBtnPageDown)
		{
			m_iCurPage++;
			if (m_iCurPage >= MAX_ITEM_WARE_PAGE)
				m_iCurPage = MAX_ITEM_WARE_PAGE-1;

			pStr = (CN3UIString* )GetChildByID("string_page");
			if (pStr)
			{
				std::string pageNo = std::to_string(m_iCurPage + 1);
				pStr->SetString(pageNo);
			}

			for( j = 0; j < MAX_ITEM_WARE_PAGE; j++ )
			{
				if (j == m_iCurPage)
				{
					for( i = 0; i < MAX_ITEM_TRADE; i++ )
					{
						if ( m_pMyWare[j][i] != NULL )
							m_pMyWare[j][i]->pUIIcon->SetVisible(true);
					}	
				}
				else
				{
					for( i = 0; i < MAX_ITEM_TRADE; i++ )
					{
						if ( m_pMyWare[j][i] != NULL )
							m_pMyWare[j][i]->pUIIcon->SetVisible(false);
					}	
				}
			}
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

			// Get Item..
			spItem = GetHighlightIconItem((CN3UIIcon* )pSender);

			// Save Select Info..
			CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWnd = UIWND_WARE_HOUSE;
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
			// 아이콘 매니저 윈도우들을 돌아 다니면서 검사..
			if ( !CGameProcedure::s_pUIMgr->BroadcastIconDropMsg(CN3UIWndBase::m_sSelectedIconInfo.pItemSelect) )
				// 아이콘 위치 원래대로..
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

bool CUIWareHouseDlg::OnMouseWheelEvent(
	short delta)
{
	if (delta > 0)
		ReceiveMessage(m_pBtnPageUp, UIMSG_BUTTON_CLICK);
	else
		ReceiveMessage(m_pBtnPageDown, UIMSG_BUTTON_CLICK);

	return true;
}

void CUIWareHouseDlg::LeaveWareHouseState()
{
	if ( IsVisible() )
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

void CUIWareHouseDlg::EnterWareHouseStateStart(int iWareGold)
{
	int i, j;
	for( j = 0; j < MAX_ITEM_WARE_PAGE; j++ )
		for( i = 0; i < MAX_ITEM_TRADE; i++ )
		{
			if ( m_pMyWare[j][i] != NULL )
			{
				if ( m_pMyWare[j][i]->pUIIcon )
				{
					RemoveChild(m_pMyWare[j][i]->pUIIcon);
					m_pMyWare[j][i]->pUIIcon->Release();
					delete m_pMyWare[j][i]->pUIIcon;
					m_pMyWare[j][i]->pUIIcon = NULL;
				}
				delete m_pMyWare[j][i];	
				m_pMyWare[j][i] = NULL;
			}
		}

	for( i = 0; i < MAX_ITEM_INVENTORY; i++ )
	{
		if ( m_pMyWareInv[i] != NULL )
		{
			if ( m_pMyWareInv[i]->pUIIcon )
			{
				RemoveChild(m_pMyWareInv[i]->pUIIcon);
				m_pMyWareInv[i]->pUIIcon->Release();
				delete m_pMyWareInv[i]->pUIIcon;
				m_pMyWareInv[i]->pUIIcon = NULL;
			}
			delete m_pMyWareInv[i];	
			m_pMyWareInv[i] = NULL;
		}
	}

	if(m_pStrWareGold)
		m_pStrWareGold->SetString(CGameBase::FormatNumber(iWareGold));
}

void CUIWareHouseDlg::EnterWareHouseStateEnd()
{
	InitIconUpdate();

	m_iCurPage = 0;
	CN3UIString* pStr = (CN3UIString* )GetChildByID("string_page");
	if (pStr)
	{
		std::string pageNo = std::to_string(m_iCurPage + 1);
		pStr->SetString(pageNo);
	}

	int i, j;
	for( j = 0; j < MAX_ITEM_WARE_PAGE; j++ )
	{
		if (j == m_iCurPage)
		{
			for( i = 0; i < MAX_ITEM_TRADE; i++ )
			{
				if ( m_pMyWare[j][i] != NULL )
					m_pMyWare[j][i]->pUIIcon->SetVisible(true);
			}	
		}
		else
		{
			for( i = 0; i < MAX_ITEM_TRADE; i++ )
			{
				if ( m_pMyWare[j][i] != NULL )
					m_pMyWare[j][i]->pUIIcon->SetVisible(false);
			}	
		}
	}

	ItemMoveFromInvToThis();

	if(m_pStrMyGold)
	{
		__InfoPlayerMySelf*	pInfoExt = &(CGameBase::s_pPlayer->m_InfoExt);
		m_pStrMyGold->SetString(CGameBase::FormatNumber(pInfoExt->iGold));
	}
}

__IconItemSkill* CUIWareHouseDlg::GetHighlightIconItem(CN3UIIcon* pUIIcon)
{
	int i;
	for( i = 0; i < MAX_ITEM_TRADE; i++ )
	{
		if ( (m_pMyWare[m_iCurPage][i] != NULL) && (m_pMyWare[m_iCurPage][i]->pUIIcon == pUIIcon) )
			return m_pMyWare[m_iCurPage][i];
	}

	for( i = 0; i < MAX_ITEM_INVENTORY; i++ )
	{
		if ( (m_pMyWareInv[i] != NULL) && (m_pMyWareInv[i]->pUIIcon == pUIIcon) ) 
			return m_pMyWareInv[i];
	}

	return NULL;
}

void CUIWareHouseDlg::IconRestore()
{
	CN3UIArea* pArea;

	switch ( CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWndDistrict )
	{
		case UIWND_DISTRICT_TRADE_NPC:
			if ( m_pMyWare[m_iCurPage][CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder] != NULL )
			{
				pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_NPC, CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder);
				if ( pArea )
				{
					m_pMyWare[m_iCurPage][CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder]->pUIIcon->SetRegion(pArea->GetRegion());
					m_pMyWare[m_iCurPage][CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder]->pUIIcon->SetMoveRect(pArea->GetRegion());
				}
			}
			break;

		case UIWND_DISTRICT_TRADE_MY:
			if ( m_pMyWareInv[CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder] != NULL )
			{
				pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder);
				if ( pArea )
				{
					m_pMyWareInv[CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder]->pUIIcon->SetRegion(pArea->GetRegion());
					m_pMyWareInv[CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder]->pUIIcon->SetMoveRect(pArea->GetRegion());
				}
			}
			break;
	}
}

bool CUIWareHouseDlg::ReceiveIconDrop(__IconItemSkill* spItem, POINT ptCur)
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
	if ( CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWnd != m_eUIWnd )
		FAIL_RETURN
	if ( (CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWndDistrict != UIWND_DISTRICT_TRADE_NPC) &&
			(CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWndDistrict != UIWND_DISTRICT_TRADE_MY) )
		FAIL_RETURN

	// 내가 가졌던 아이콘이면.. npc영역인지 검사한다..
	int i, iDestiOrder = -1; bool bFound = false;
	for( i = 0; i < MAX_ITEM_TRADE; i++ )
	{
		pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_NPC, i);
		if ( (pArea) && (pArea->IsIn(ptCur.x, ptCur.y)) )
		{
			bFound = true;
			eUIWnd = UIWND_DISTRICT_TRADE_NPC;
			iDestiOrder = i;
			break;
		}
	}

	if (!bFound)
	{
		for( i = 0; i < MAX_ITEM_INVENTORY; i++ )
		{
			pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, i);
			if ( (pArea) && (pArea->IsIn(ptCur.x, ptCur.y)) )
			{
				bFound = true;
				eUIWnd = UIWND_DISTRICT_TRADE_MY;
				iDestiOrder = i;
				break;
			}
		}
	}

	if (!bFound)	FAIL_RETURN

	// 본격적으로 Recovery Info를 활용하기 시작한다..
	// 먼저 WaitFromServer를 On으로 하고.. Select Info를 Recovery Info로 복사.. 이때 Dest는 팰요없다..
	if ( spItem != m_sSelectedIconInfo.pItemSelect )
		m_sSelectedIconInfo.pItemSelect = spItem;

	s_bWaitFromServer									= true;
	m_sRecoveryJobInfo.pItemSource						= m_sSelectedIconInfo.pItemSelect;
	m_sRecoveryJobInfo.UIWndSourceStart.UIWnd			= m_sSelectedIconInfo.UIWndSelect.UIWnd;
	m_sRecoveryJobInfo.UIWndSourceStart.UIWndDistrict	= m_sSelectedIconInfo.UIWndSelect.UIWndDistrict;
	m_sRecoveryJobInfo.UIWndSourceStart.iOrder			= m_sSelectedIconInfo.UIWndSelect.iOrder;
	m_sRecoveryJobInfo.pItemTarget						= nullptr;

	m_sRecoveryJobInfo.UIWndSourceEnd.UIWnd				= UIWND_WARE_HOUSE;
	m_sRecoveryJobInfo.UIWndSourceEnd.UIWndDistrict		= eUIWnd;

	switch (m_sSelectedIconInfo.UIWndSelect.UIWndDistrict)
	{
		case UIWND_DISTRICT_TRADE_NPC:
			if (eUIWnd == UIWND_DISTRICT_TRADE_MY)		// 빼는 경우..
			{
				if( (CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE) ||
					(CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL) )
				{
					// 활이나 물약등 아이템인 경우..
					// 면저 인벤토리에 해당 아이콘이 있는지 알아본다..
					bFound = false;

					for( i = 0; i < MAX_ITEM_INVENTORY; i++ )
					{
						if( (m_pMyWareInv[i]) && (m_pMyWareInv[i]->pItemBasic->dwID == CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pItemBasic->dwID) &&
							(m_pMyWareInv[i]->pItemExt->dwID == CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pItemExt->dwID) )
						{
							bFound = true;
							iDestiOrder = i;
							CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder	= iDestiOrder;
							break;
						}
					}

					// 못찾았으면.. 
					if ( !bFound )
					{
						if ( m_pMyWareInv[iDestiOrder] )	// 해당 위치에 아이콘이 있으면..
						{
							// 인벤토리 빈슬롯을 찾아 들어간다..
							for( i = 0; i < MAX_ITEM_INVENTORY; i++ )
							{
								if ( !m_pMyWareInv[i] )
								{
									bFound = true;
									iDestiOrder = i;
									break;
								}
							}

							if ( !bFound )	// 빈 슬롯을 찾지 못했으면..
							{
								s_bWaitFromServer				= false;
								m_sRecoveryJobInfo.pItemSource	= nullptr;
								m_sRecoveryJobInfo.pItemTarget	= nullptr;
								FAIL_RETURN
							}
						}
					}

					m_sRecoveryJobInfo.UIWndSourceEnd.iOrder	= iDestiOrder;
					s_bWaitFromServer							= false;

					m_pCountableItemEdit->Open(UIWND_WARE_HOUSE, m_sSelectedIconInfo.UIWndSelect.UIWndDistrict, false);
					FAIL_RETURN
				}
				else
				{
					// 일반 아이템인 경우..
					if ( m_pMyWareInv[iDestiOrder] )	// 해당 위치에 아이콘이 있으면..
					{
						// 인벤토리 빈슬롯을 찾아 들어간다..
						bFound = false;
						for( i = 0; i < MAX_ITEM_INVENTORY; i++ )
						{
							if ( !m_pMyWareInv[i] )
							{
								bFound = true;
								iDestiOrder = i;
								break;
							}
						}

						if ( !bFound )	// 빈 슬롯을 찾지 못했으면..
						{
							s_bWaitFromServer				= false;
							m_sRecoveryJobInfo.pItemSource	= nullptr;
							m_sRecoveryJobInfo.pItemTarget	= nullptr;
							FAIL_RETURN
						}

						CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder	= iDestiOrder;
					}
					else
						CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder	= iDestiOrder;

					// 무게 체크..
					__InfoPlayerMySelf*	pInfoExt = &(CGameBase::s_pPlayer->m_InfoExt);
					if ( (pInfoExt->iWeight + CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->siWeight) > pInfoExt->iWeightMax)
					{	 
						std::string szMsg = fmt::format_text_resource(IDS_ITEM_WEIGHT_OVERFLOW);	
						CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);

						s_bWaitFromServer				= false;
						m_sRecoveryJobInfo.pItemSource	= nullptr;
						m_sRecoveryJobInfo.pItemTarget	= nullptr;
						FAIL_RETURN	
					}

					SendToServerFromWareMsg(CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->dwID+
						CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemExt->dwID,
						m_iCurPage,
						CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder, 
						iDestiOrder, 
						CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->iCount);

					m_pMyWareInv[iDestiOrder] = m_pMyWare[m_iCurPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder];
					m_pMyWare[m_iCurPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder] = NULL;
					pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, iDestiOrder);
					if ( pArea )
					{
						m_pMyWareInv[iDestiOrder]->pUIIcon->SetRegion(pArea->GetRegion());
						m_pMyWareInv[iDestiOrder]->pUIIcon->SetMoveRect(pArea->GetRegion());
					}			
					FAIL_RETURN
				}
			}
			else
			{
				// 이동.. 
				__IconItemSkill *spItemSource, *spItemTarget = NULL;
				spItemSource = CN3UIWndBase::m_sRecoveryJobInfo.pItemSource;

				pArea = NULL;
				pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_NPC, iDestiOrder);
				if ( pArea )
				{
					spItemSource->pUIIcon->SetRegion(pArea->GetRegion());
					spItemSource->pUIIcon->SetMoveRect(pArea->GetRegion());
				}

				CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder				= iDestiOrder;

				// 창고 내에서 이동..	(모두 일반 아이템으로 취급한다..)
				if ( m_pMyWare[m_iCurPage][iDestiOrder] )	// 해당 위치에 아이콘이 있으면..
				{
					s_bWaitFromServer				= false;
					m_sRecoveryJobInfo.pItemSource	= nullptr;
					m_sRecoveryJobInfo.pItemTarget	= nullptr;
					FAIL_RETURN
				}
				else
				{
					m_sRecoveryJobInfo.pItemTarget	= nullptr;
				}

				m_pMyWare[m_iCurPage][iDestiOrder] = spItemSource;
				m_pMyWare[m_iCurPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder] = spItemTarget;

				// 이동 메시지를 보낸다..
				SendToServerWareToWareMsg(CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->dwID+
					CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemExt->dwID,
					m_iCurPage,
					CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder, iDestiOrder
					);

				FAIL_RETURN					
			}
			break;

		case UIWND_DISTRICT_TRADE_MY:
			if (eUIWnd == UIWND_DISTRICT_TRADE_NPC)		// 넣는 경우..
			{
				if( (CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE) || 
					(CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL) )
				{
					// 활이나 물약등 아이템인 경우..
					// 면저 Ware에 아이콘이 있는지 알아본다..
					bFound = false;

					// 10개의 폐이지를 다 뒤진다..
					for( int iPage = 0; iPage < MAX_ITEM_WARE_PAGE; iPage++ )
					{
						if ( bFound )
							break;

						for( i = 0; i < MAX_ITEM_TRADE; i++ )
						{
							if( (m_pMyWare[iPage][i]) && (m_pMyWare[iPage][i]->pItemBasic->dwID == CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pItemBasic->dwID) &&
								(m_pMyWare[iPage][i]->pItemExt->dwID == CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pItemExt->dwID) )
							{
								bFound = true;
								iDestiOrder = i;
								CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder	= iDestiOrder;
								CN3UIWndBase::m_sRecoveryJobInfo.m_iPage = iPage;
								break;
							}
						}
					}

					// 못찾았으면.. 
					if ( !bFound )
					{
						if ( m_pMyWare[m_iCurPage][iDestiOrder] )	// 해당 위치에 아이콘이 있으면..
						{
							// 빈슬롯을 찾아 들어간다..
							for( int iPage = 0; iPage < MAX_ITEM_WARE_PAGE; iPage++ )
							{
								if ( bFound )
									break;

								for( i = 0; i < MAX_ITEM_TRADE; i++ )
								{
									if ( bFound )
									{
										CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder	= iDestiOrder;
										CN3UIWndBase::m_sRecoveryJobInfo.m_iPage = iPage;
										break;
									}

									if ( !m_pMyWare[iPage][i] )
									{
										bFound = true;
										iDestiOrder = i;
									}
								}

								if ( !bFound )	// 빈 슬롯을 찾지 못했으면..
								{
									s_bWaitFromServer				= false;
									m_sRecoveryJobInfo.pItemSource	= nullptr;
									m_sRecoveryJobInfo.pItemTarget	= nullptr;
									FAIL_RETURN
								}
							}
						}
						else
						{
							m_sRecoveryJobInfo.UIWndSourceEnd.iOrder	= iDestiOrder;
							m_sRecoveryJobInfo.m_iPage = m_iCurPage;
						}
					}

					s_bWaitFromServer = false;

					m_pCountableItemEdit->Open(UIWND_WARE_HOUSE, m_sSelectedIconInfo.UIWndSelect.UIWndDistrict, false);
					FAIL_RETURN
				}
				else
				{
					// 일반 아이템인 경우..
					if ( m_pMyWare[m_iCurPage][iDestiOrder] )	// 해당 위치에 아이콘이 있으면..
					{
						// 인벤토리 빈슬롯을 찾아 들어간다..
						bFound = false;

						// 10개의 폐이지를 다 뒤진다..
						for( int iPage = 0; iPage < MAX_ITEM_WARE_PAGE; iPage++ )
						{
							if ( bFound )
								break;

							for( i = 0; i < MAX_ITEM_TRADE; i++ )
							{
								if ( bFound )
								{
									CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder	= iDestiOrder;
									CN3UIWndBase::m_sRecoveryJobInfo.m_iPage = iPage;
									break;
								}

								if ( !m_pMyWare[iPage][i] )
								{
									bFound = true;
									iDestiOrder = i;
								}
							}
						}

						if ( !bFound )	// 빈 슬롯을 찾지 못했으면..
						{
							s_bWaitFromServer				= false;
							m_sRecoveryJobInfo.pItemSource	= nullptr;
							m_sRecoveryJobInfo.pItemTarget	= nullptr;
							FAIL_RETURN
						}
					}
					else
					{
						m_sRecoveryJobInfo.UIWndSourceEnd.iOrder	= iDestiOrder;
						m_sRecoveryJobInfo.m_iPage					= m_iCurPage;
					}

					SendToServerToWareMsg(CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->dwID+
						CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemExt->dwID,
						CN3UIWndBase::m_sRecoveryJobInfo.m_iPage,
						CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder, 
						iDestiOrder, 
						CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->iCount);

					m_pMyWare[CN3UIWndBase::m_sRecoveryJobInfo.m_iPage][iDestiOrder] = m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder];
					m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder] = NULL;

					pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_NPC, iDestiOrder);
					if ( pArea )
					{
						m_pMyWare[CN3UIWndBase::m_sRecoveryJobInfo.m_iPage][iDestiOrder]->pUIIcon->SetRegion(pArea->GetRegion());
						m_pMyWare[CN3UIWndBase::m_sRecoveryJobInfo.m_iPage][iDestiOrder]->pUIIcon->SetMoveRect(pArea->GetRegion());
					}
					
					if (CN3UIWndBase::m_sRecoveryJobInfo.m_iPage != m_iCurPage)
						m_pMyWare[CN3UIWndBase::m_sRecoveryJobInfo.m_iPage][iDestiOrder]->pUIIcon->SetVisibleWithNoSound(false);

					FAIL_RETURN
				}
			}
			else
			{
				// 이동.. 
				__IconItemSkill *spItemSource, *spItemTarget = NULL;
				spItemSource = CN3UIWndBase::m_sRecoveryJobInfo.pItemSource;

				pArea = NULL;
				pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, iDestiOrder);
				if ( pArea )
				{
					spItemSource->pUIIcon->SetRegion(pArea->GetRegion());
					spItemSource->pUIIcon->SetMoveRect(pArea->GetRegion());
				}

				CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder				= iDestiOrder;

				// Inv 내에서 이동..	(모두 일반 아이템으로 취급한다..)
				if ( m_pMyWareInv[iDestiOrder] )	// 해당 위치에 아이콘이 있으면..
				{
					CN3UIWndBase::m_sRecoveryJobInfo.pItemTarget					= m_pMyWareInv[iDestiOrder];
					CN3UIWndBase::m_sRecoveryJobInfo.UIWndTargetStart.UIWnd			= UIWND_WARE_HOUSE;
					CN3UIWndBase::m_sRecoveryJobInfo.UIWndTargetStart.UIWndDistrict = UIWND_DISTRICT_TRADE_MY;
					CN3UIWndBase::m_sRecoveryJobInfo.UIWndTargetStart.iOrder		= iDestiOrder;
					CN3UIWndBase::m_sRecoveryJobInfo.UIWndTargetEnd.UIWnd			= UIWND_WARE_HOUSE;
					CN3UIWndBase::m_sRecoveryJobInfo.UIWndTargetEnd.UIWndDistrict	= UIWND_DISTRICT_TRADE_MY;
					CN3UIWndBase::m_sRecoveryJobInfo.UIWndTargetEnd.iOrder			= 
						CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder;

					spItemTarget = CN3UIWndBase::m_sRecoveryJobInfo.pItemTarget;

					pArea = NULL;
					pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder);
					if ( pArea )
					{
						spItemTarget->pUIIcon->SetRegion(pArea->GetRegion());
						spItemTarget->pUIIcon->SetMoveRect(pArea->GetRegion());
					}
				}
				else
					CN3UIWndBase::m_sRecoveryJobInfo.pItemTarget					= NULL;

				m_pMyWareInv[iDestiOrder] = spItemSource;
				m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder] = spItemTarget;

				// 이동 메시지를 보낸다..
				SendToServerInvToInvMsg(CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->dwID+
					CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemExt->dwID,
					m_iCurPage,
					CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder, 
					iDestiOrder
					);

				FAIL_RETURN
			}
			break;
	}


	CN3UIWndBase::AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);

	return false;
}

void CUIWareHouseDlg::CancelIconDrop(__IconItemSkill* spItem)
{
	CN3UIWndBase::AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);
}

void CUIWareHouseDlg::AcceptIconDrop(__IconItemSkill* spItem)
{
	CN3UIWndBase::AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);
}

void CUIWareHouseDlg::SendToServerToWareMsg(int iItemID, byte page, byte startpos, byte pos, int iCount)
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

void CUIWareHouseDlg::SendToServerFromWareMsg(int iItemID, byte page, byte startpos, byte pos, int iCount)
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

void CUIWareHouseDlg::SendToServerWareToWareMsg(int iItemID, byte page, byte startpos, byte destpos)
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

void CUIWareHouseDlg::SendToServerInvToInvMsg(int iItemID, byte page, byte startpos, byte destpos)
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

void CUIWareHouseDlg::ReceiveResultToWareMsg(uint8_t bResult)	// 넣는 경우..
{
	s_bWaitFromServer = false;

	int iGold = CN3UIWndBase::m_pCountableItemEdit->GetQuantity();
	__IconItemSkill* spItem;
	CN3UIArea* pArea = NULL;

	if (bResult != 0x01)	// 실패..
	{
		if (m_bSendedItemGold) {	ReceiveResultGoldToWareFail();	return;		}
		if( (CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE) ||
			(CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL) )			// 활이나 물약등 아이템인 경우..
		{
			// Ware Side..	//////////////////////////////////////////////////////

			if ( (m_pMyWare[CN3UIWndBase::m_sRecoveryJobInfo.m_iPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder]->iCount - iGold) > 0 )
			{	
				//  숫자 업데이트..
				m_pMyWare[CN3UIWndBase::m_sRecoveryJobInfo.m_iPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder]->iCount -= iGold;
			}
			else
			{
				// 아이템 삭제.. 현재 인벤토리 윈도우만.. 
				spItem = m_pMyWare[CN3UIWndBase::m_sRecoveryJobInfo.m_iPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder];

				// 인벤토리에서도 지운다..
				m_pMyWare[CN3UIWndBase::m_sRecoveryJobInfo.m_iPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder] = NULL;

				// iOrder로 내 매니저의 아이템을 리스트에서 삭제한다..
				RemoveChild(spItem->pUIIcon);

				// 아이콘 리소스 삭제...
				spItem->pUIIcon->Release();
				delete spItem->pUIIcon;
				spItem->pUIIcon = NULL;
				delete spItem;
				spItem = NULL;
			}

			// Inv Side..	//////////////////////////////////////////////////////

			if ( !m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder]->pUIIcon->IsVisible() )
			{
				m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder]->pUIIcon->SetVisible(true);
				m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder]->iCount = iGold;
			}
			else
			{
				m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder]->iCount += iGold;
			}
		}
		else	// 일반 아이템인 경우..														
		{
			m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder] = 
				m_pMyWare[m_iCurPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder]; 

			pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder);
			if ( pArea )
			{
				m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder]->pUIIcon->SetRegion(pArea->GetRegion());
				m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder]->pUIIcon->SetMoveRect(pArea->GetRegion());
			}			

			m_pMyWare[m_iCurPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder] = NULL;
		}
	}

	// 성공.. 
	else
	{
		if (m_bSendedItemGold) 
		{
			m_bSendedItemGold = false;		// 원래 대로..
			return;
		}
		if( ( (CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE) || 
			(CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL) ) &&
			!m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder]->pUIIcon->IsVisible() )			// 활이나 물약등 아이템인 경우..
		{				
			spItem = m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder];

			// 인벤토리에서도 지운다..
			m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder] = NULL;

			// iOrder로 내 매니저의 아이템을 리스트에서 삭제한다..
			RemoveChild(spItem->pUIIcon);

			// 아이콘 리소스 삭제...
			spItem->pUIIcon->Release();
			delete spItem->pUIIcon;
			spItem->pUIIcon = NULL;
			delete spItem;
			spItem = NULL;
		}
	}
}

void CUIWareHouseDlg::ReceiveResultFromWareMsg(uint8_t bResult)	// 빼는 경우..
{
	s_bWaitFromServer = false;

	int iGold = CN3UIWndBase::m_pCountableItemEdit->GetQuantity();
	__IconItemSkill* spItem;
	CN3UIArea* pArea = NULL;

	if (bResult != 0x01)	// 실패..
	{
		if (m_bSendedItemGold) {	ReceiveResultGoldFromWareFail();	return;		}
		if( (CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE) || 
			(CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL) )
		{
			// Inv Side..	//////////////////////////////////////////////////////

			if ( (m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder]->iCount - iGold) > 0 )
			{	
				//  숫자 업데이트..
				m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder]->iCount -= iGold;
			}
			else
			{
				// 아이템 삭제.. 현재 인벤토리 윈도우만.. 
				spItem = m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder];

				// 인벤토리에서도 지운다..
				m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder] = NULL;

				// iOrder로 내 매니저의 아이템을 리스트에서 삭제한다..
				RemoveChild(spItem->pUIIcon);

				// 아이콘 리소스 삭제...
				spItem->pUIIcon->Release();
				delete spItem->pUIIcon;
				spItem->pUIIcon = NULL;
				delete spItem;
				spItem = NULL;
			}

			// Ware Side..	//////////////////////////////////////////////////////

			if ( !m_pMyWare[m_iCurPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder]->pUIIcon->IsVisible() )
			{
				m_pMyWare[m_iCurPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder]->pUIIcon->SetVisible(true);
				m_pMyWare[m_iCurPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder]->iCount = iGold;
			}
			else
			{
				m_pMyWare[m_iCurPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder]->iCount+= iGold;
			}
		}
		else	// 일반 아이템..
		{
			m_pMyWare[m_iCurPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder] = 
				m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder]; 

			pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_NPC, CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder);
			if ( pArea )
			{
				m_pMyWare[m_iCurPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder]->pUIIcon->SetRegion(pArea->GetRegion());
				m_pMyWare[m_iCurPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder]->pUIIcon->SetMoveRect(pArea->GetRegion());
			}			

			m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder] = NULL;
		}

		std::string szMsg = fmt::format_text_resource(IDS_ITEM_TOOMANY_OR_HEAVY);
		CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);
	}

	// 성공.. 
	else
	{
		if (m_bSendedItemGold) 
		{
			m_bSendedItemGold = false;		// 원래 대로..
			return;
		}
		if( ( (CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE) || 
			(CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL) ) &&
			!m_pMyWare[m_iCurPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder]->pUIIcon->IsVisible() )			// 활이나 물약등 아이템인 경우..
		{				
			spItem = m_pMyWare[m_iCurPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder];

			// 인벤토리에서도 지운다..
			m_pMyWare[m_iCurPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder] = NULL;

			// iOrder로 내 매니저의 아이템을 리스트에서 삭제한다..
			RemoveChild(spItem->pUIIcon);

			// 아이콘 리소스 삭제...
			spItem->pUIIcon->Release();
			delete spItem->pUIIcon;
			spItem->pUIIcon = NULL;
			delete spItem;
			spItem = NULL;
		}
	}
}

void CUIWareHouseDlg::ReceiveResultWareToWareMsg(uint8_t bResult)
{
	s_bWaitFromServer = false;

	CN3UIArea* pArea = NULL;

	if (bResult != 0x01)	// 실패..
	{
		__IconItemSkill *spItemSource = NULL, *spItemTarget = NULL;
		spItemSource = CN3UIWndBase::m_sRecoveryJobInfo.pItemSource;
		spItemTarget = CN3UIWndBase::m_sRecoveryJobInfo.pItemTarget;

		if (spItemSource)
		{
			pArea = NULL;
			pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_NPC, CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder);
			if ( pArea )
			{
				spItemSource->pUIIcon->SetRegion(pArea->GetRegion());
				spItemSource->pUIIcon->SetMoveRect(pArea->GetRegion());
			}

			m_pMyWare[m_iCurPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder] = spItemSource;
		}

		if (spItemTarget)
		{
			pArea = NULL;
			pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_NPC, CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder);
			if ( pArea )
			{
				spItemTarget->pUIIcon->SetRegion(pArea->GetRegion());
				spItemTarget->pUIIcon->SetMoveRect(pArea->GetRegion());
			}

			m_pMyWare[m_iCurPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder] = spItemTarget;
		}
		else
		{
			m_pMyWare[m_iCurPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder] = NULL;
		}
	}

	CN3UIWndBase::AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);
}

void CUIWareHouseDlg::ReceiveResultInvToInvMsg(uint8_t bResult)
{
	s_bWaitFromServer = false;

	CN3UIArea* pArea = NULL;

	if (bResult != 0x01)	// 실패..
	{
		__IconItemSkill *spItemSource = NULL, *spItemTarget = NULL;
		spItemSource = CN3UIWndBase::m_sRecoveryJobInfo.pItemSource;
		spItemTarget = CN3UIWndBase::m_sRecoveryJobInfo.pItemTarget;

		if (spItemSource)
		{
			pArea = NULL;
			pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder);
			if ( pArea )
			{
				spItemSource->pUIIcon->SetRegion(pArea->GetRegion());
				spItemSource->pUIIcon->SetMoveRect(pArea->GetRegion());
			}

			m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder] = spItemSource;
		}

		if (spItemTarget)
		{
			pArea = NULL;
			pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder);
			if ( pArea )
			{
				spItemTarget->pUIIcon->SetRegion(pArea->GetRegion());
				spItemTarget->pUIIcon->SetMoveRect(pArea->GetRegion());
			}

			m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder] = spItemTarget;
		}
		else
		{
			m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder] = NULL;
		}
	}

	CN3UIWndBase::AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);
}

void CUIWareHouseDlg::ItemCountOK()
{
	int iGold = CN3UIWndBase::m_pCountableItemEdit->GetQuantity();
	__IconItemSkill* spItem;
	CN3UIArea* pArea = NULL;
	float fUVAspect		= (float)45.0f/(float)64.0f;
	int iWeight;
	__InfoPlayerMySelf*	pInfoExt = &(CGameBase::s_pPlayer->m_InfoExt);

	switch (CN3UIWndBase::m_pCountableItemEdit->GetCallerWndDistrict())
	{
		case UIWND_DISTRICT_TRADE_NPC:		// 빼는 경우..
			spItem = m_pMyWare[m_iCurPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder];
			if ( iGold > spItem->iCount ) return;

			switch (spItem->pItemBasic->byContable)
			{
				case UIITEM_TYPE_ONLYONE:
				case UIITEM_TYPE_SOMOONE:
					iWeight = spItem->pItemBasic->siWeight;

					// 무게 체크..
					if ( (pInfoExt->iWeight + iWeight) > pInfoExt->iWeightMax)
					{	 
						std::string szMsg = fmt::format_text_resource(IDS_ITEM_WEIGHT_OVERFLOW);	
						CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);	
						return;
					}
					break;

				case UIITEM_TYPE_COUNTABLE:
					if ( iGold <= 0 ) return;
					// int16_t 범위이상은 살수 없다..
					if ( iGold > UIITEM_COUNT_MANY ) 
					{
						std::string szMsg = fmt::format_text_resource(IDS_MANY_COUNTABLE_ITEM_GET_MANY);
						CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);				
						return;
					}

					if (m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder])				
					{
						spItem = m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder];
						if (spItem->iCount + iGold > UIITEM_COUNT_MANY)
						{
							std::string szMsg = fmt::format_text_resource(IDS_MANY_COUNTABLE_ITEM_GET_MANY);
							CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);				
							return;
						}
					}

					// 무게 체크..
					iWeight = iGold * spItem->pItemBasic->siWeight;
					if ( (pInfoExt->iWeight + iWeight) > pInfoExt->iWeightMax)
					{	 
						std::string szMsg = fmt::format_text_resource(IDS_ITEM_WEIGHT_OVERFLOW);	
						CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);	
						return;
					}
					break;

				case UIITEM_TYPE_COUNTABLE_SMALL:
					if ( iGold <= 0 ) return;
					// int16_t 범위이상은 살수 없다..
					if ( iGold > UIITEM_COUNT_FEW ) 
					{
						std::string szMsg = fmt::format_text_resource(IDS_SMALL_COUNTABLE_ITEM_GET_MANY);
						CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);				
						return;
					}

					if (m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder])				
					{
						spItem = m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder];
						if (spItem->iCount + iGold > UIITEM_COUNT_FEW)
						{
							std::string szMsg = fmt::format_text_resource(IDS_SMALL_COUNTABLE_ITEM_GET_MANY);
							CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);				
							return;
						}
					}

					// 무게 체크..
					iWeight = iGold * spItem->pItemBasic->siWeight;
					if ( (pInfoExt->iWeight + iWeight) > pInfoExt->iWeightMax)
					{	 
						std::string szMsg = fmt::format_text_resource(IDS_ITEM_WEIGHT_OVERFLOW);	
						CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);	
						return;
					}
					break;
			}

			spItem = m_pMyWare[m_iCurPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder];

			s_bWaitFromServer = true;

			if ( m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder] )	// 해당 위치에 아이콘이 있으면..
			{
				//  숫자 업데이트..
				m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder]->iCount += iGold;
			}
			else	// 없으면 아이콘을 만든다...
			{
				__IconItemSkill* spItemNew;
				spItemNew				= new __IconItemSkill;
				spItemNew->pItemBasic	= spItem->pItemBasic;
				spItemNew->pItemExt		= spItem->pItemExt;
				spItemNew->szIconFN		= spItem->szIconFN;
				spItemNew->iCount		= iGold;
				spItemNew->iDurability	= spItem->iDurability;
				spItemNew->pUIIcon		= new CN3UIIcon;
				spItemNew->pUIIcon->Init(this);
				spItemNew->pUIIcon->SetTex(spItem->szIconFN);
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

				m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder] = spItemNew;
			}

			if ( (spItem->iCount - iGold) > 0 )
			{	
				//  숫자 업데이트..
				spItem->iCount -= iGold;
			}
			else
			{
				spItem->pUIIcon->SetVisible(false);
			}

			// 표시는 아이콘 렌더링할때.. Inventory의 Render에서..
			// 서버에게 보냄..
			SendToServerFromWareMsg(CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->dwID+
				CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemExt->dwID,
				m_iCurPage,
				CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder,
				CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder, 
				iGold);

			// Sound..
			if (CN3UIWndBase::m_sRecoveryJobInfo.pItemSource) PlayItemSound(CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic);
			break;

		case UIWND_DISTRICT_TRADE_MY:		//  넣는 경우..
			spItem = m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder];
			if ( iGold > spItem->iCount ) return;

			switch (spItem->pItemBasic->byContable)
			{
				case UIITEM_TYPE_COUNTABLE:
					if ( iGold <= 0 ) return;
					// int16_t 범위이상은 살수 없다..
					if ( iGold > UIITEM_COUNT_MANY ) 
					{
						std::string szMsg = fmt::format_text_resource(IDS_MANY_COUNTABLE_ITEM_GET_MANY);
						CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);				
						return;
					}

					if (m_pMyWare[CN3UIWndBase::m_sRecoveryJobInfo.m_iPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder])
					{
						spItem = m_pMyWare[CN3UIWndBase::m_sRecoveryJobInfo.m_iPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder];
						if (spItem->iCount + iGold > UIITEM_COUNT_MANY)
						{
							std::string szMsg = fmt::format_text_resource(IDS_MANY_COUNTABLE_ITEM_GET_MANY);
							CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);				
							return;
						}
					}
					break;

				case UIITEM_TYPE_COUNTABLE_SMALL:
					if ( iGold <= 0 ) return;
					// int16_t 범위이상은 살수 없다..
					if ( iGold > UIITEM_COUNT_FEW ) 
					{
						std::string szMsg = fmt::format_text_resource(IDS_SMALL_COUNTABLE_ITEM_GET_MANY);
						CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);				
						return;
					}

					if (m_pMyWare[CN3UIWndBase::m_sRecoveryJobInfo.m_iPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder])
					{
						spItem = m_pMyWare[CN3UIWndBase::m_sRecoveryJobInfo.m_iPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder];
						if (spItem->iCount + iGold > UIITEM_COUNT_FEW)
						{
							std::string szMsg = fmt::format_text_resource(IDS_SMALL_COUNTABLE_ITEM_GET_MANY);
							CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);				
							return;
						}
					}
					break;
			}

			spItem = m_pMyWareInv[CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder];

			s_bWaitFromServer = true;

			if ( m_pMyWare[CN3UIWndBase::m_sRecoveryJobInfo.m_iPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder] )	// 해당 위치에 아이콘이 있으면..
			{
				//  숫자 업데이트..
				m_pMyWare[CN3UIWndBase::m_sRecoveryJobInfo.m_iPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder]->iCount += iGold;
			}
			else	// 없으면 아이콘을 만든다..
			{
				__IconItemSkill* spItemNew;
				spItemNew				= new __IconItemSkill;
				spItemNew->pItemBasic	= spItem->pItemBasic;
				spItemNew->pItemExt		= spItem->pItemExt;
				spItemNew->szIconFN		= spItem->szIconFN;
				spItemNew->iCount		= iGold;
				spItemNew->iDurability	= spItem->iDurability;
				spItemNew->pUIIcon		= new CN3UIIcon;
				spItemNew->pUIIcon->Init(this);
				spItemNew->pUIIcon->SetTex(spItem->szIconFN);
				spItemNew->pUIIcon->SetUVRect(0,0, fUVAspect, fUVAspect);
				spItemNew->pUIIcon->SetUIType(UI_TYPE_ICON);
				spItemNew->pUIIcon->SetStyle(UISTYLE_ICON_ITEM|UISTYLE_ICON_CERTIFICATION_NEED);
				spItemNew->pUIIcon->SetVisible(true);
				pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_NPC, CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder);
				if ( pArea )
				{
					spItemNew->pUIIcon->SetRegion(pArea->GetRegion());
					spItemNew->pUIIcon->SetMoveRect(pArea->GetRegion());
				}

				if (CN3UIWndBase::m_sRecoveryJobInfo.m_iPage != m_iCurPage)
					spItemNew->pUIIcon->SetVisibleWithNoSound(false);

				m_pMyWare[CN3UIWndBase::m_sRecoveryJobInfo.m_iPage][CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder] = spItemNew;
			}

			if ( (spItem->iCount - iGold) > 0 )
			{	
				//  숫자 업데이트..
				spItem->iCount -= iGold;
			}
			else
			{
				spItem->pUIIcon->SetVisible(false);
			}

			// 표시는 아이콘 렌더링할때.. Inventory의 Render에서..
			// 서버에게 보냄..
			SendToServerToWareMsg(CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemBasic->dwID+
				CN3UIWndBase::m_sRecoveryJobInfo.pItemSource->pItemExt->dwID,
				CN3UIWndBase::m_sRecoveryJobInfo.m_iPage,
				CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceStart.iOrder,
				CN3UIWndBase::m_sRecoveryJobInfo.UIWndSourceEnd.iOrder, iGold);

			break;
	}

	CN3UIWndBase::m_pCountableItemEdit->Close();
}

void CUIWareHouseDlg::ItemCountCancel()
{
	// Sound..
	if (m_sRecoveryJobInfo.pItemSource) PlayItemSound(m_sRecoveryJobInfo.pItemSource->pItemBasic);

	// 취소..
	s_bWaitFromServer				= false;
	m_sRecoveryJobInfo.pItemSource	= nullptr;
	m_sRecoveryJobInfo.pItemTarget	= nullptr;

	m_pCountableItemEdit->Close();
}

void CUIWareHouseDlg::ItemMoveFromInvToThis()
{
	CUIInventory* pInven = CGameProcedure::s_pProcMain->m_pUIInventory;
	if(!pInven) return;

	int i;
	for( i = 0; i < MAX_ITEM_INVENTORY; i++ )
	{
		m_pMyWareInv[i] = NULL;
	}

	for( i = 0; i < MAX_ITEM_INVENTORY; i++ )
	{
		if(pInven->m_pMyInvWnd[i])
		{
			__IconItemSkill* spItem = pInven->m_pMyInvWnd[i];
			spItem->pUIIcon->SetParent(this);

			pInven->m_pMyInvWnd[i] = NULL;
			CN3UIArea* pArea;

			pArea = GetChildAreaByiOrder(UI_AREA_TYPE_TRADE_MY, i);
			if ( pArea )
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
	if(!pInven) return;

	int i;
	for( i = 0; i < MAX_ITEM_INVENTORY; i++ )
	{
		if(m_pMyWareInv[i])
		{
			__IconItemSkill* spItem = m_pMyWareInv[i];
			spItem->pUIIcon->SetParent(pInven);

			m_pMyWareInv[i] = NULL;

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

void CUIWareHouseDlg::AddItemInWare(int iItem, int iDurability, int iCount, int iIndex)
{

	if (!iItem) return;
	std::string szIconFN;
	__IconItemSkill*	spItem = NULL;
	__TABLE_ITEM_BASIC*	pItem = NULL;													// 아이템 테이블 구조체 포인터..
	__TABLE_ITEM_EXT*	pItemExt = NULL;

	pItem = CGameBase::s_pTbl_Items_Basic.Find(iItem/1000*1000);	// 열 데이터 얻기..
	if(pItem && pItem->byExtIndex >= 0 && pItem->byExtIndex < MAX_ITEM_EXTENSION)
		pItemExt = CGameBase::s_pTbl_Items_Exts[pItem->byExtIndex].Find(iItem%1000);	// 열 데이터 얻기..
	if ( NULL == pItem || NULL == pItemExt )
	{
		__ASSERT(0, "NULL Item!!!");
		CLogWriter::Write("WareHouse - Ware - Unknown Item {}, IDNumber", iItem);
		return; // 아이템이 없으면..
	}

	e_PartPosition ePart;
	e_PlugPosition ePlug;
	e_ItemType eType = CGameBase::MakeResrcFileNameForUPC(pItem, pItemExt, nullptr, &szIconFN, ePart, ePlug); // 아이템에 따른 파일 이름을 만들어서
	if(ITEM_TYPE_UNKNOWN == eType) CLogWriter::Write("MyInfo - slot - Unknown Item");
	__ASSERT(ITEM_TYPE_UNKNOWN != eType, "Unknown Item");
	
	spItem = new __IconItemSkill;
	spItem->pItemBasic	= pItem;
	spItem->pItemExt	= pItemExt;
	spItem->szIconFN	= szIconFN; // 아이콘 파일 이름 복사..
	spItem->iCount		= iCount;
	spItem->iDurability = iDurability;

	m_pMyWare[iIndex/MAX_ITEM_TRADE][iIndex%MAX_ITEM_TRADE] = spItem;
	//TRACE("Init Inv Msg Inve %d, iOrder %d \n", iItem, iIndex);
}

void CUIWareHouseDlg::GoldCountToWareOK()	//돈을 넣는 경우..
{
	int iGold, iMyMoney, iWareMoney;			// 인벤토리의 값..
	std::string str;

	// 돈을 보관함에 보관하는 경우..
	iGold = CN3UIWndBase::m_pCountableItemEdit->GetQuantity();

	// Gold Offset Backup..
	m_iGoldOffsetBackup = iGold;

	// 현재 내가 가진 돈을 얻어 온다..
	iMyMoney = CGameBase::s_pPlayer->m_InfoExt.iGold;

	// 보관함의 돈을 얻어온다..
	CN3UIString* pStr = NULL;
	pStr = (CN3UIString* )GetChildByID("string_wareitem_name");	 
	__ASSERT(pStr, "NULL UI Component!!");
	str = CGameBase::UnformatNumber(pStr->GetString());
	iWareMoney = atoi(str.c_str());

	if ( iGold <= 0 ) return;
	if ( iGold > iMyMoney ) return;

	m_bSendedItemGold = true;		// 보낸 아이템이 돈이다.. !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// 돈을 감소 시킨다..
	iMyMoney -= iGold;
	CGameBase::s_pPlayer->m_InfoExt.iGold = iMyMoney;

	iWareMoney += iGold;

	// 돈 표시.. Ware..
	pStr->SetString(CGameBase::FormatNumber(iWareMoney));
	// 돈 표시.. 인벤토리..
	CGameProcedure::s_pProcMain->m_pUIInventory->GoldUpdate();
	// 돈 표시.. Inv..
	pStr = (CN3UIString* )GetChildByID("string_item_name"); __ASSERT(pStr, "NULL UI Component!!");
	if (pStr) pStr->SetString(CGameBase::FormatNumber(iMyMoney));

	// 서버에게 패킷 만들어서 날림..
	SendToServerToWareMsg(dwGold, 0xff, 0xff, 0xff, iGold);

	// 상태를 변화시키고.. 창을 닫고..
	s_bWaitFromServer = true;
	m_pCountableItemEdit->Close();

	PlayGoldSound();
}

void CUIWareHouseDlg::GoldCountFromWareOK()		// 돈을 빼는 경우..
{
	int iGold, iMyMoney, iWareMoney;			// 인벤토리의 값..
	std::string str;

	// 돈을 보관함에서 빼는 경우..
	iGold = CN3UIWndBase::m_pCountableItemEdit->GetQuantity();

	// Gold Offset Backup..
	m_iGoldOffsetBackup = iGold;

	// 현재 내가 가진 돈을 얻어 온다..
	iMyMoney = CGameBase::s_pPlayer->m_InfoExt.iGold;

	// 보관함의 돈을 얻어온다..
	CN3UIString* pStr = NULL;
	pStr = (CN3UIString* )GetChildByID("string_wareitem_name");	 
	__ASSERT(pStr, "NULL UI Component!!");
	str = CGameBase::UnformatNumber(pStr->GetString());
	iWareMoney = atoi(str.c_str());

	if ( iGold <= 0 ) return;
	if ( iGold > iWareMoney ) return;

	m_bSendedItemGold = true;		// 보낸 아이템이 돈이다.. !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// 돈을 감소 시킨다..
	iMyMoney += iGold;
	CGameBase::s_pPlayer->m_InfoExt.iGold = iMyMoney;

	iWareMoney -= iGold;

	// 돈 표시.. Ware..
	pStr->SetString(CGameBase::FormatNumber(iWareMoney));
	// 돈 표시.. 인벤토리..
	CGameProcedure::s_pProcMain->m_pUIInventory->GoldUpdate();
	// 돈 표시.. Inv..
	pStr = (CN3UIString* )GetChildByID("string_item_name"); __ASSERT(pStr, "NULL UI Component!!");
	if (pStr) pStr->SetString(CGameBase::FormatNumber(iMyMoney));

	// 서버에게 패킷 만들어서 날림..
	SendToServerFromWareMsg(dwGold, 0xff, 0xff, 0xff, iGold);

	// 상태를 변화시키고.. 창을 닫고..
	s_bWaitFromServer = true;
	m_pCountableItemEdit->Close();

	PlayGoldSound();
}

void CUIWareHouseDlg::GoldCountToWareCancel()
{
	// 돈을 보관함에 보관하는 경우 취소..
	// Sound..
	PlayGoldSound();

	// 취소..
	s_bWaitFromServer					= false;
	m_sRecoveryJobInfo.pItemSource		= nullptr;
	m_sRecoveryJobInfo.pItemTarget		= nullptr;

	CN3UIWndBase::m_pCountableItemEdit->Close();
}

void CUIWareHouseDlg::GoldCountFromWareCancel()
{
	// 돈을 보관함에서 빼오는 경우 취소..
	// Sound..
	PlayGoldSound();

	// 취소..
	s_bWaitFromServer					= false;
	m_sRecoveryJobInfo.pItemSource		= nullptr;
	m_sRecoveryJobInfo.pItemTarget		= nullptr;

	CN3UIWndBase::m_pCountableItemEdit->Close();
}

void CUIWareHouseDlg::ReceiveResultGoldToWareFail()
{
	m_bSendedItemGold = false;			// 원래 대로..

	int iGold, iMyMoney, iWareMoney;			// 인벤토리의 값..
	std::string str;

	// 돈을 보관함에서 빼는 경우..
	iGold = CN3UIWndBase::m_pCountableItemEdit->GetQuantity();

	// Gold Offset Backup..
	m_iGoldOffsetBackup = iGold;

	// 현재 내가 가진 돈을 얻어 온다..
	iMyMoney = CGameBase::s_pPlayer->m_InfoExt.iGold;

	// 보관함의 돈을 얻어온다..
	CN3UIString* pStr = NULL;
	pStr = (CN3UIString* )GetChildByID("string_wareitem_name");	 
	__ASSERT(pStr, "NULL UI Component!!");
	str = CGameBase::UnformatNumber(pStr->GetString());
	iWareMoney = atoi(str.c_str());

	// 돈을 감소 시킨다..
	iMyMoney += iGold;
	CGameBase::s_pPlayer->m_InfoExt.iGold = iMyMoney;

	iWareMoney -= iGold;

	// 돈 표시.. Ware..
	pStr->SetStringAsInt(iWareMoney);
	// 돈 표시.. 인벤토리..
	CGameProcedure::s_pProcMain->m_pUIInventory->GoldUpdate();
	// 돈 표시.. Inv..
	pStr = (CN3UIString* )GetChildByID("string_item_name"); __ASSERT(pStr, "NULL UI Component!!");
	if(pStr) pStr->SetStringAsInt(iMyMoney);
}

void CUIWareHouseDlg::ReceiveResultGoldFromWareFail()
{
	m_bSendedItemGold = false;		// 원래 대로..

	int iGold, iMyMoney, iWareMoney;			// 인벤토리의 값..
	std::string str;

	// 돈을 보관함에 보관하는 경우..
	iGold = CN3UIWndBase::m_pCountableItemEdit->GetQuantity();

	// Gold Offset Backup..
	m_iGoldOffsetBackup = iGold;

	// 현재 내가 가진 돈을 얻어 온다..
	iMyMoney = CGameBase::s_pPlayer->m_InfoExt.iGold;

	// 보관함의 돈을 얻어온다..
	CN3UIString* pStr = NULL;
	pStr = (CN3UIString* )GetChildByID("string_wareitem_name");	 
	__ASSERT(pStr, "NULL UI Component!!");
	str = CGameBase::UnformatNumber(pStr->GetString());
	iWareMoney = atoi(str.c_str());

	// 돈을 감소 시킨다..
	iMyMoney -= iGold;
	CGameBase::s_pPlayer->m_InfoExt.iGold = iMyMoney;

	iWareMoney += iGold;

	// 돈 표시.. Ware..
	pStr->SetStringAsInt(iWareMoney);
	// 돈 표시.. 인벤토리..
	CGameProcedure::s_pProcMain->m_pUIInventory->GoldUpdate();
	// 돈 표시.. Inv..
	pStr = (CN3UIString* )GetChildByID("string_item_name"); __ASSERT(pStr, "NULL UI Component!!");
	if(pStr) pStr->SetStringAsInt(iMyMoney);
}

//this_ui_add_start
bool CUIWareHouseDlg::OnKeyPress(int iKey)
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
		return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

bool CUIWareHouseDlg::Load(HANDLE hFile)
{
	if(CN3UIBase::Load(hFile)==false) return false;

	m_pBtnGold			= (CN3UIButton*)(this->GetChildByID("btn_gold"));			__ASSERT(m_pBtnGold, "NULL UI Component!!");
	m_pBtnGoldWareHouse = (CN3UIButton*)(this->GetChildByID("btn_gold_warehouse"));	__ASSERT(m_pBtnGoldWareHouse, "NULL UI Component!!");
	m_pBtnClose			= (CN3UIButton*)(this->GetChildByID("btn_close"));			__ASSERT(m_pBtnClose, "NULL UI Component!!");
	m_pBtnPageUp		= (CN3UIButton*)(this->GetChildByID("btn_page_up"));		__ASSERT(m_pBtnPageUp, "NULL UI Component!!");
	m_pBtnPageDown		= (CN3UIButton*)(this->GetChildByID("btn_page_down"));		__ASSERT(m_pBtnPageDown, "NULL UI Component!!");

	return true;
}

void CUIWareHouseDlg::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);
	if(bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
		CGameProcedure::s_pUIMgr->ReFocusUI();//this_ui
}

void CUIWareHouseDlg::SetVisibleWithNoSound(bool bVisible, bool bWork, bool bReFocus)
{
	CN3UIBase::SetVisibleWithNoSound(bVisible, bWork, bReFocus);

	if(bWork)
	{
		if(CN3UIWndBase::m_pCountableItemEdit && CN3UIWndBase::m_pCountableItemEdit->IsVisible())
			CN3UIWndBase::m_pCountableItemEdit->SetVisibleWithNoSound(bVisible, bWork, bReFocus);

		if (GetState() == UI_STATE_ICON_MOVING)
			IconRestore();
		SetState(UI_STATE_COMMON_NONE);
		CN3UIWndBase::AllHighLightIconFree();

		// 이 윈도우의 inv 영역의 아이템을 이 인벤토리 윈도우의 inv영역으로 옮긴다..	
		ItemMoveFromThisToInv();

		if (CGameProcedure::s_pProcMain->m_pUISkillTreeDlg) CGameProcedure::s_pProcMain->m_pUISkillTreeDlg->UpdateDisableCheck();
		if (CGameProcedure::s_pProcMain->m_pUIHotKeyDlg) CGameProcedure::s_pProcMain->m_pUIHotKeyDlg->UpdateDisableCheck();
	}
}
//this_ui_add_end

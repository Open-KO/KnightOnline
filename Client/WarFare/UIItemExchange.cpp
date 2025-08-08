// UIItemExchange.cpp: implementation of the CUIItemRepairExchange class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UIItemExchange.h"
#include "PacketDef.h"
#include "APISocket.h"
#include "LocalInput.h"
#include "PlayerMySelf.h"
#include "GameProcMain.h"
#include "UIImageTooltipDlg.h"
#include "UIInventory.h"
#include "UIManager.h"
#include "text_resources.h"

#include <N3Base/N3UIString.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUIItemExchange::CUIItemExchange()
{
	m_pUITooltipDlg = NULL;

	int i;
	for( i = 0; i < MAX_ITEM_INVENTORY; i++ )
	{
		m_pMyInvWnd[i] = NULL;
	}

	for( i = 0; i < MAX_ITEM_EX_RE_NPC; i++ )
	{
		m_pMyNpcWnd[i] = NULL;
	}

	for( i = 0; i < MAX_ITEM_EX_RE_NPC; i++ )
	{
		m_pMyNpcWndOriginIndex[i] = -1;
	}

	m_pTotalPrice = 0;
}

CUIItemExchange::~CUIItemExchange()
{
	Release();
}

void CUIItemExchange::Release()
{
	CN3UIBase::Release();

	for( int i = 0; i < MAX_ITEM_INVENTORY; i++ )
	{
		if ( m_pMyInvWnd[i] != NULL )
		{
			delete m_pMyInvWnd[i];
			m_pMyInvWnd[i] = NULL;
		}
	}

	for(int i = 0; i < MAX_ITEM_EX_RE_NPC; i++ )
	{
		if ( m_pMyNpcWnd[i] != NULL )
		{
			delete m_pMyNpcWnd[i];
			m_pMyNpcWnd[i] = NULL;
		}
	}
}

bool CUIItemExchange::Load(HANDLE hFile)
{
	if(CN3UIBase::Load(hFile)==false) return false;

	m_pImage_Repair		= (CN3UIImage*)GetChildByID("img_repair");
	m_pImage_Exchange	= (CN3UIImage*)GetChildByID("img_change");

	return true;
}

void CUIItemExchange::Render()
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
}

void CUIItemExchange::InitIconWnd(e_UIWND eWnd)
{
	__TABLE_UI_RESRC* pTblUI = CGameBase::s_pTbl_UI.Find(CGameBase::s_pPlayer->m_InfoBase.eNation);

	m_pUITooltipDlg = new CUIImageTooltipDlg();
	m_pUITooltipDlg->Init(this);
	m_pUITooltipDlg->LoadFromFile(pTblUI->szItemInfo);
	m_pUITooltipDlg->InitPos();
	m_pUITooltipDlg->SetVisible(FALSE);	

	CN3UIWndBase::InitIconWnd(eWnd);
}

__IconItemSkill* CUIItemExchange::GetHighlightIconItem(CN3UIIcon* pUIIcon)
{
	int i;
	for( i = 0; i < MAX_ITEM_INVENTORY; i++ )
	{
		if ( (m_pMyInvWnd[i] != NULL) && (m_pMyInvWnd[i]->pUIIcon == pUIIcon) )
			return m_pMyInvWnd[i];
	}

	return NULL;
}

int	CUIItemExchange::GetItemiOrder(__IconItemSkill* spItem, e_UIWND_DISTRICT eWndDist)
{
	int i;

	switch ( eWndDist )
	{
		case UIWND_DISTRICT_EX_RE_INV:
			for( i = 0; i < MAX_ITEM_INVENTORY; i++ )
			{
				if ( (m_pMyInvWnd[i] != NULL) && (m_pMyInvWnd[i] == spItem) )
					return i;
			}
			break;
	}

	return -1;
}

e_UIWND_DISTRICT CUIItemExchange::GetWndDistrict(__IconItemSkill* spItem)
{
	for( int i = 0; i < MAX_ITEM_INVENTORY; i++ )
	{
		if ( (m_pMyInvWnd[i] != NULL) && (m_pMyInvWnd[i] == spItem) )
			return UIWND_DISTRICT_EX_RE_INV;
	}

	for(int i = 0; i < MAX_ITEM_EX_RE_NPC; i++ )
	{
		if ( (m_pMyNpcWnd[i] != NULL) && (m_pMyNpcWnd[i] == spItem) )
			return UIWND_DISTRICT_EX_RE_NPC;
	}
	return UIWND_DISTRICT_UNKNOWN;
}

RECT CUIItemExchange::GetSampleRect()
{
	RECT rect;
	CN3UIArea* pArea;
	POINT ptCur = CGameProcedure::s_pLocalInput->MouseGetPos();
	pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_REPAIR_INV, 0);
	rect = pArea->GetRegion();
	float fWidth = (float)(rect.right - rect.left);
	float fHeight = (float)(rect.bottom - rect.top);
	fWidth *= 0.5f; fHeight *= 0.5f;
	rect.left = ptCur.x - (int)fWidth;  rect.right  = ptCur.x + (int)fWidth;
	rect.top  = ptCur.y - (int)fHeight; rect.bottom = ptCur.y + (int)fHeight;
	return rect;
}

void CUIItemExchange::IconRestore()
{
	CN3UIArea* pArea;

	switch ( CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWndDistrict )
	{
		case UIWND_DISTRICT_EX_RE_INV:
			if ( m_pMyInvWnd[CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder] != NULL )
			{
				pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_REPAIR_INV, CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder);
				if ( pArea )
				{
					m_pMyInvWnd[CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder]->pUIIcon->SetRegion(pArea->GetRegion());
					m_pMyInvWnd[CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder]->pUIIcon->SetMoveRect(pArea->GetRegion());
				}
			}
			break;
	}
}

void CUIItemExchange::CancelIconDrop(__IconItemSkill* spItem)
{
	CN3UIWndBase::AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);
}

bool CUIItemExchange::ReceiveIconDrop(__IconItemSkill* spItem, POINT ptCur)
{
// Temp Define 
#define FAIL_RETURN {	\
		CN3UIWndBase::AllHighLightIconFree();	\
		SetState(UI_STATE_COMMON_NONE);	\
		return false;	\
	}

	CN3UIArea* pArea;
	if (!m_bVisible) return false;

	// 내가 가졌던 아이콘이 아니면..
	if ( CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWnd != m_eUIWnd )
		FAIL_RETURN
	if ( CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWndDistrict != UIWND_DISTRICT_EX_RE_INV )
		FAIL_RETURN

	// 내가 가졌던 아이콘이면.. npc영역인지 검사한다..
	int i; bool bFound = false;
	for( i = 0; i < MAX_ITEM_EX_RE_NPC; i++ )
	{
		pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_REPAIR_NPC, i);
		if ( (pArea) && (pArea->IsIn(ptCur.x, ptCur.y)) )
		{
			bFound = true;
			break;
		}
	}

	if (!bFound)	FAIL_RETURN

	// 아이템이 꽉 차있는지 검사한다..
	bFound = false;
	for( i = 0; i < MAX_ITEM_EX_RE_NPC; i++ )
	{
		if(!m_pMyNpcWnd[i])
		{ 
			bFound = true;
			break;
		}
	}

	// 아이템이 꽉 차있지 않다면.. 가장 첫번째 빈 슬롯을 검사한다..
	if (!bFound)	FAIL_RETURN

	// 받아들인다..
	m_pMyInvWnd[CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder] = NULL;

	spItem->pUIIcon->SetRegion(pArea->GetRegion());
	spItem->pUIIcon->SetMoveRect(pArea->GetRegion());

	m_pMyNpcWnd[i] = spItem;

	// 백업 정보..
	m_pMyNpcWndOriginIndex[i] = CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder;

	// 수리비용 업그레이드..
	m_pTotalPrice += CalcRepairGold(spItem);
	UpdateGoldValue();

	CN3UIWndBase::AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);

	return true;
}

void CUIItemExchange::UpdateGoldValue()
{
	CN3UIString* pStrGold = (CN3UIString*) GetChildByID("string_gold");
	__ASSERT(pStrGold, "NULL UI Component!!");

	// 돈 업데이트..	
	if (pStrGold != nullptr)
		pStrGold->SetStringAsInt(m_pTotalPrice);
}

void CUIItemExchange::UpdateUserTotalGold(int iGold)
{
	// 돈 업데이트..
	CGameBase::s_pPlayer->m_InfoExt.iGold = iGold;
	CGameProcedure::s_pProcMain->m_pUIInventory->GoldUpdate();
}

int	 CUIItemExchange::CalcRepairGold(__IconItemSkill* spItem)
{
	int iMaxDurability = spItem->pItemBasic->siMaxDurability + spItem->pItemExt->siMaxDurability;
	if (iMaxDurability <= 0) return 0;
	float fAllPrice = (float)spItem->pItemBasic->iPrice*(float)spItem->pItemExt->siPriceMultiply;
	float fTemp = ((fAllPrice-10.0f)/(10000.0f))+pow(fAllPrice, 0.75f);
	float fValue = fTemp*( (float)(iMaxDurability - spItem->iDurability) / (float)(iMaxDurability) );

	return (int)fValue;
}

bool CUIItemExchange::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
// Temp Define
#define FAIL_CODE {		\
				SetState(UI_STATE_COMMON_NONE);	\
				return false;	\
			}

	if(NULL == pSender) return false;


	if (dwMsg == UIMSG_BUTTON_CLICK)					
	{
		if(pSender->m_szID == "btn_cancel")
			UserPressCancel();
		if(pSender->m_szID == "btn_close")
			UserPressCancel();
		if(pSender->m_szID == "btn_ok")
			UserPressOK();
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
			if (!spItem) FAIL_CODE

			if ( (spItem->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE) || (spItem->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL) ) FAIL_CODE

			// Save Select Info..
			CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWnd = UIWND_EXCHANGE_REPAIR;
			eUIWnd = GetWndDistrict(spItem);
			if ( eUIWnd != UIWND_DISTRICT_EX_RE_INV )	FAIL_CODE
			CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWndDistrict = UIWND_DISTRICT_EX_RE_INV;
			iOrder = GetItemiOrder(spItem, UIWND_DISTRICT_EX_RE_INV);
			if (iOrder == -1) FAIL_CODE
			CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.iOrder = iOrder;
			CN3UIWndBase::m_sSelectedIconInfo.pItemSelect = spItem;
			// Do Ops..
			((CN3UIIcon* )pSender)->SetRegion(GetSampleRect());
			break;

		case UIMSG_ICON_DOWN:
			if ( GetState()  == UI_STATE_ICON_MOVING )
			{
				CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pUIIcon->SetRegion(GetSampleRect());
				CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pUIIcon->SetMoveRect(GetSampleRect());
			}
			break;

		case UIMSG_ICON_UP:
			// 아이콘 매니저 윈도우들을 돌아 다니면서 검사..
			if ( !CGameProcedure::s_pUIMgr->BroadcastIconDropMsg(CN3UIWndBase::m_sSelectedIconInfo.pItemSelect) )
				// 아이콘 위치 원래대로..
				IconRestore();				
			break;
	}

	return true;
}

uint32_t CUIItemExchange::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (!m_bVisible) return dwRet;
	if (s_bWaitFromServer) { dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);  return dwRet; }

	// 드래그 되는 아이콘 갱신..
	if ( (GetState() == UI_STATE_ICON_MOVING) && 
			(CN3UIWndBase::m_sSelectedIconInfo.UIWndSelect.UIWnd == UIWND_EXCHANGE_REPAIR) )
	{
		CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pUIIcon->SetRegion(GetSampleRect());
		CN3UIWndBase::m_sSelectedIconInfo.pItemSelect->pUIIcon->SetMoveRect(GetSampleRect());
	}

	return CN3UIWndBase::MouseProc(dwFlags, ptCur, ptOld);
}

void CUIItemExchange::Open()
{
	SetVisible(true);

	// 보호코드가 필요하다.. 예를 들면, 인벤토리, npc와의 상거래, 개인 거래 금지..


	// 기타 작업..
	int i;
	for( i = 0; i < MAX_ITEM_EX_RE_NPC; i++ )
	{
		m_pMyNpcWnd[i] = NULL;
	}

	for( i = 0; i < MAX_ITEM_EX_RE_NPC; i++ )
	{
		m_pMyNpcWndOriginIndex[i] = -1;
	}

	m_pTotalPrice = 0;
	UpdateGoldValue();

	// 인벤토리 inv 영역의 아이템을 이 윈도우의 inv영역으로 옮긴다..
	ItemMoveFromInvToThis();
}

void CUIItemExchange::UserPressOK()
{
	// 갯수 세기..
	int iCount = 0;
	for( int i = 0; i < MAX_ITEM_EX_RE_NPC; i++ )
	{
		if(m_pMyNpcWnd[i])	iCount++;
	}

	if (iCount == 0) 
		UserPressCancel();

	// 서버에게 보내고.. 
	uint8_t byBuff[16];											// 패킷 버퍼..
	int iOffset=0;											// 패킷 오프셋..

	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_ITEM_REPAIR);			// 게임 스타트 패킷 커멘드..
	CAPISocket::MP_AddShort(byBuff, iOffset, iCount);		// 아이디 길이 패킷에 넣기..
	for( int i = 0; i < iCount; i++ )
	{
		CAPISocket::MP_AddByte(byBuff, iOffset, m_pMyNpcWndOriginIndex[i]);		// 아이디 길이 패킷에 넣기..
		CAPISocket::MP_AddDword(byBuff, iOffset, m_pMyNpcWnd[i]->pItemBasic->dwID+m_pMyNpcWnd[i]->pItemExt->dwID);	// 아이디 문자열 패킷에 넣기..
	}
	CGameProcedure::s_pSocket->Send(byBuff, iOffset);	

	// 응답을 기다림..
	s_bWaitFromServer = true;
}

void CUIItemExchange::ReceiveResultFromServer(int iResult, int iUserGold)
{
	// 성공이면 npc영역의 Durability를 최대값으로..
	if(iResult == 0x01)
	{
		for( int i = 0; i < MAX_ITEM_EX_RE_NPC; i++ )
		{
			if(m_pMyNpcWnd[i])	
			{
				m_pMyNpcWnd[i]->iDurability = m_pMyNpcWnd[i]->pItemBasic->siMaxDurability+m_pMyNpcWnd[i]->pItemExt->siMaxDurability;

				// 아이콘 상태가 UISTYLE_DURABILITY_EXHAUST 이면..
				m_pMyNpcWnd[i]->pUIIcon->SetStyle(m_pMyNpcWnd[i]->pUIIcon->GetStyle() & (~UISTYLE_DURABILITY_EXHAUST));
			}
		}
	}

	// 돈 업데이트..
	UpdateUserTotalGold(iUserGold);

	// 응답 기다림 해제..
	s_bWaitFromServer = false;

	// 이 윈도우의 npc 영역의 아이템을 이 윈도우의 inv 영역으로 옮긴다..
	CN3UIArea* pArea = NULL;
	for( int i = 0; i < MAX_ITEM_EX_RE_NPC; i++ )
	{
		if(m_pMyNpcWnd[i])
		{
			pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_REPAIR_INV, m_pMyNpcWndOriginIndex[i]);			
			if ( pArea )
			{
				m_pMyNpcWnd[i]->pUIIcon->SetRegion(pArea->GetRegion());
				m_pMyNpcWnd[i]->pUIIcon->SetMoveRect(pArea->GetRegion());
			}			

			m_pMyInvWnd[m_pMyNpcWndOriginIndex[i]] = m_pMyNpcWnd[i];
			m_pMyNpcWnd[i] = NULL;
		}
	}

	// 창을 닫는다..	
	Close();
}

void CUIItemExchange::UserPressCancel()
{
	CN3UIArea* pArea = NULL;

	// 이 윈도우의 npc 영역의 아이템을 이 윈도우의 inv 영역으로 옮긴다..
	int i;
	for( i = 0; i < MAX_ITEM_EX_RE_NPC; i++ )
	{
		if(m_pMyNpcWnd[i])
		{
			pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_REPAIR_INV, m_pMyNpcWndOriginIndex[i]);			
			if ( pArea )
			{
				m_pMyNpcWnd[i]->pUIIcon->SetRegion(pArea->GetRegion());
				m_pMyNpcWnd[i]->pUIIcon->SetMoveRect(pArea->GetRegion());
			}			

			m_pMyInvWnd[m_pMyNpcWndOriginIndex[i]] = m_pMyNpcWnd[i];
			m_pMyNpcWnd[i] = NULL;
		}
	}

	// 창을 닫는다..	
	Close();
}

void CUIItemExchange::Close()
{
	SetVisible(false);

	// 이 윈도우의 inv 영역의 아이템을 이 인벤토리 윈도우의 inv영역으로 옮긴다..	
	ItemMoveFromThisToInv();
}

void CUIItemExchange::ItemMoveFromInvToThis()
{
	CUIInventory* pInven = CGameProcedure::s_pProcMain->m_pUIInventory;
	if(!pInven) return;

	int i;
	for( i = 0; i < MAX_ITEM_INVENTORY; i++ )
	{
		m_pMyInvWnd[i] = NULL;
	}

	for( i = 0; i < MAX_ITEM_INVENTORY; i++ )
	{
		if(pInven->m_pMyInvWnd[i])
		{
			__IconItemSkill* spItem = pInven->m_pMyInvWnd[i];
			spItem->pUIIcon->SetParent(this);

			pInven->m_pMyInvWnd[i] = NULL;
			CN3UIArea* pArea;

			pArea = GetChildAreaByiOrder(UI_AREA_TYPE_REPAIR_INV, i);
			if ( pArea )
			{
				spItem->pUIIcon->SetRegion(pArea->GetRegion());
				spItem->pUIIcon->SetMoveRect(pArea->GetRegion());
			}			

			m_pMyInvWnd[i] = spItem;
		}
	}
}

void CUIItemExchange::ItemMoveFromThisToInv()
{
	CUIInventory* pInven = CGameProcedure::s_pProcMain->m_pUIInventory;
	if(!pInven) return;

	int i;
	for( i = 0; i < MAX_ITEM_INVENTORY; i++ )
	{
		if(m_pMyInvWnd[i])
		{
			__IconItemSkill* spItem = m_pMyInvWnd[i];
			spItem->pUIIcon->SetParent(pInven);

			m_pMyInvWnd[i] = NULL;

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



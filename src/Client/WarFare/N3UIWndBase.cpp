// N3UIWndBase.cpp: implementation of the CN3UIWndBase class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "N3UIWndBase.h"

#include "GameProcMain.h"
#include "UIImageTooltipDlg.h"
#include "UITransactionDlg.h"
#include "UIManager.h"
#include "CountableItemEditDlg.h"

#include <N3Base/N3UITooltip.h>
#include <N3Base/N3SndObj.h>

#include <N3Base/N3UIString.h>

__InfoSelectedIcon CN3UIWndBase::s_sSelectedIconInfo;
__RecoveryJobInfo CN3UIWndBase::s_sRecoveryJobInfo;
__SkillSelectInfo CN3UIWndBase::s_sSkillSelectInfo;

CN3UIImage* CN3UIWndBase::s_pSelectionImage               = nullptr;
CCountableItemEditDlg* CN3UIWndBase::s_pCountableItemEdit = nullptr;

CN3SndObj* CN3UIWndBase::s_pSnd_Item_Etc                  = nullptr;
CN3SndObj* CN3UIWndBase::s_pSnd_Item_Weapon               = nullptr;
CN3SndObj* CN3UIWndBase::s_pSnd_Item_Armor                = nullptr;
CN3SndObj* CN3UIWndBase::s_pSnd_Gold                      = nullptr;
CN3SndObj* CN3UIWndBase::s_pSnd_Repair                    = nullptr;
int CN3UIWndBase::s_iRefCount                             = 0;

CN3UIWndBase::CN3UIWndBase()
{
	m_eUIWnd          = UIWND_UNKNOWN;

	s_pSelectionImage = new CN3UIImage();
	s_pSelectionImage->Init(CGameProcedure::s_pUIMgr);
	s_pSelectionImage->SetUVRect(0.0f, 0.0f, 1.0f, 1.0f);

	if (s_iRefCount == 0)
	{
		s_pSnd_Item_Etc    = s_SndMgr.CreateObj(ID_SOUND_ITEM_ETC_IN_INVENTORY);
		s_pSnd_Item_Weapon = s_SndMgr.CreateObj(ID_SOUND_ITEM_WEAPON_IN_INVENTORY);
		s_pSnd_Item_Armor  = s_SndMgr.CreateObj(ID_SOUND_ITEM_ARMOR_IN_INVENTORY);
		s_pSnd_Gold        = s_SndMgr.CreateObj(ID_SOUND_GOLD_IN_INVENTORY);
		s_pSnd_Repair      = s_SndMgr.CreateObj(ID_SOUND_ITEM_IN_REPAIR);
	}
	s_iRefCount++; // 참조 카운트
}

CN3UIWndBase::~CN3UIWndBase()
{
	s_iRefCount--; // 참조 카운트
	if (s_iRefCount == 0)
	{
		CN3Base::s_SndMgr.ReleaseObj(&s_pSnd_Item_Etc);
		CN3Base::s_SndMgr.ReleaseObj(&s_pSnd_Item_Weapon);
		CN3Base::s_SndMgr.ReleaseObj(&s_pSnd_Item_Armor);
		CN3Base::s_SndMgr.ReleaseObj(&s_pSnd_Gold);
		CN3Base::s_SndMgr.ReleaseObj(&s_pSnd_Repair);
	}
}

void CN3UIWndBase::InitIconWnd(e_UIWND eWnd)
{
	m_eUIWnd = eWnd;
}

CN3UIArea* CN3UIWndBase::GetChildAreaByiOrder(eUI_AREA_TYPE eUAT, int iOrder)
{
	std::string szID = std::to_string(iOrder);
	for (CN3UIBase* pChild : m_Children)
	{
		if (pChild->UIType() == UI_TYPE_AREA && static_cast<CN3UIArea*>(pChild)->m_eAreaType == eUAT && pChild->GetID() == szID)
			return static_cast<CN3UIArea*>(pChild);
	}

	return nullptr;
}

CN3UIString* CN3UIWndBase::GetChildStringByiOrder(int iOrder)
{
	std::string szID = std::to_string(iOrder);
	for (CN3UIBase* pChild : m_Children)
	{
		if (pChild->UIType() == UI_TYPE_STRING && pChild->GetID() == szID)
			return static_cast<CN3UIString*>(pChild);
	}

	return nullptr;
}

void CN3UIWndBase::AllHighLightIconFree()
{
	for (UIListReverseItor itor = m_Children.rbegin(); m_Children.rend() != itor; ++itor)
	{
		CN3UIBase* pChild = (*itor);
		if (pChild->UIType() == UI_TYPE_ICON)
			pChild->SetStyle(pChild->GetStyle() & (~UISTYLE_ICON_HIGHLIGHT));
	}
}

uint32_t CN3UIWndBase::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (!m_bVisible)
		return dwRet;

	// 아이콘을 가진 윈도우는 자기 윈도우 영역을 벗어 났을때도 자식의 메시지를 받을수 있어야 한다.. ^^

	// UI 움직이는 코드
	if (UI_STATE_COMMON_MOVE == m_eState)
	{
		if (dwFlags & UI_MOUSE_LBCLICKED)
		{
			SetState(UI_STATE_COMMON_NONE);
		}
		else
		{
			MoveOffset(ptCur.x - ptOld.x, ptCur.y - ptOld.y);
		}
		dwRet |= UI_MOUSEPROC_DONESOMETHING;
		return dwRet;
	}

	if (false == IsIn(ptCur.x, ptCur.y)) // 영역 밖이면
	{
		if (false == IsIn(ptOld.x, ptOld.y))
		{
			if (GetState() != UI_STATE_ICON_MOVING)
				return dwRet;               // 이전 좌표도 영역 밖이면
		}
		dwRet |= UI_MOUSEPROC_PREVINREGION; // 이전 좌표는 영역 안이었다.
	}
	else
	{
		// tool tip 관련
		if (s_pTooltipCtrl != nullptr)
			s_pTooltipCtrl->SetText(m_szToolTip, m_crToolTip);
	}

	dwRet |= UI_MOUSEPROC_INREGION; // 이번 좌표는 영역 안이다.

	// child에게 메세지 전달
	for (UIListItor itor = m_Children.begin(); m_Children.end() != itor; ++itor)
	{
		CN3UIBase* pChild   = (*itor);
		uint32_t dwChildRet = pChild->MouseProc(dwFlags, ptCur, ptOld);
		if (UI_MOUSEPROC_DONESOMETHING & dwChildRet)
		{
			// 이경우에는 먼가 포커스를 받은 경우이다.
			dwRet |= (UI_MOUSEPROC_CHILDDONESOMETHING | UI_MOUSEPROC_DONESOMETHING);
			return dwRet;
		}
	}

	// UI 움직이는 코드
	if (UI_STATE_COMMON_MOVE != m_eState && PtInRect(&m_rcMovable, ptCur) && (dwFlags & UI_MOUSE_LBCLICK))
	{
		// 인벤토리 윈도우이고 상거래 중이면..
		if ((UIType() == UI_TYPE_ICON_MANAGER) && (m_eUIWnd == UIWND_INVENTORY)
			&& (CGameProcedure::s_pProcMain->m_pUITransactionDlg->IsVisible()))
			return dwRet;
		SetState(UI_STATE_COMMON_MOVE);
		dwRet |= UI_MOUSEPROC_DONESOMETHING;
		return dwRet;
	}

	return dwRet;
}

void CN3UIWndBase::PlayItemEtcSound()
{
	if (s_pSnd_Item_Etc)
		s_pSnd_Item_Etc->Play();
}

void CN3UIWndBase::PlayItemWeaponSound()
{
	if (s_pSnd_Item_Weapon)
		s_pSnd_Item_Weapon->Play();
}

void CN3UIWndBase::PlayItemArmorSound()
{
	if (s_pSnd_Item_Armor)
		s_pSnd_Item_Armor->Play();
}

void CN3UIWndBase::PlayGoldSound()
{
	if (s_pSnd_Gold)
		s_pSnd_Gold->Play();
}

void CN3UIWndBase::PlayRepairSound()
{
	if (s_pSnd_Repair)
		s_pSnd_Repair->Play();
}

void CN3UIWndBase::PlayItemSound(__TABLE_ITEM_BASIC* pBasic)
{
	if (!pBasic)
		return;
	switch (pBasic->dwID / 100000000)
	{
		case 1:
			PlayItemWeaponSound();
			break;

		case 2:
			PlayItemArmorSound();
			break;

		default:
			PlayItemEtcSound();
			break;
	}
}

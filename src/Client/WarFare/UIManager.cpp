// UIManager.cpp: implementation of the CUIManager class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UIManager.h"
#include "LocalInput.h"
#include "N3UIWndBase.h"
#include "GameProcedure.h"
#include "GameProcMain.h"
#include "UIInventory.h"
#include "UITransactionDlg.h"
#include "SubProcPerTrade.h"
#include "CountableItemEditDlg.h"
#include "UIMsgBoxOkCancel.h"
#include "UIWareHouseDlg.h"

#include <N3Base/DFont.h>
#include <N3Base/N3UITooltip.h>

#include <iterator>

CUIManager::__RenderStateForUI CUIManager::s_sRSFU; // RenderStateForUI

CUIManager::CUIManager()
{
	m_dwMouseFlagsCur  = 0;
	m_bEnableOperation = true; // UI 조작이 가능한 상태인가?
	m_pUIFocused       = nullptr;
#ifdef _DEBUG
	m_pDFont = nullptr;
#endif

	m_bDoneSomething = false; // UI 에서 조작을 했다...
}

CUIManager::~CUIManager()
{
#ifdef _DEBUG
	delete m_pDFont;
#endif
}

void CUIManager::Release()
{
	CN3UIBase::Release();
}

uint32_t CUIManager::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	m_dwMouseFlagsCur = UI_MOUSEPROC_NONE;
	if (!m_bVisible || !m_bEnableOperation)
		return m_dwMouseFlagsCur;

	if (s_pTooltipCtrl)
		s_pTooltipCtrl->MouseProc(dwFlags, ptCur, ptOld); // 툴팁에게 마우스 메세지 전달.

	// child에게 메세지 전달
	for (UIListItor itor = m_Children.begin(); m_Children.end() != itor;)
	{
		CN3UIBase* pChild = (*itor);
		// 상거래 중이면 아이콘 매니저 윈도우만 작동..
		if (CGameProcedure::s_pProcMain && CGameProcedure::s_pProcMain->m_pUITransactionDlg
			&& (CGameProcedure::s_pProcMain->m_pUITransactionDlg->IsVisible())) // && (pChild->UIType() != UI_TYPE_ICON_MANAGER) )
		{
			if (CN3UIWndBase::s_pCountableItemEdit->IsLocked())
			{
				if (pChild->m_szID.compare("base_tradeedit") != 0)
				{
					++itor;
					continue;
				}
			}
		}
		// 보관함에 보관중이면 아이콘 매니저 윈도우만 작동..
		if (CGameProcedure::s_pProcMain && CGameProcedure::s_pProcMain->m_pUIWareHouseDlg
			&& (CGameProcedure::s_pProcMain->m_pUIWareHouseDlg->IsVisible())) // && (pChild->UIType() != UI_TYPE_ICON_MANAGER) )
		{
			if (CN3UIWndBase::s_pCountableItemEdit->IsLocked())
			{
				if (pChild->m_szID.compare("base_tradeedit") != 0)
				{
					++itor;
					continue;
				}
			}
		}
		// 개인간 거래중이면 아이콘 매니저 윈도우만 작동.. 돈 액수나 화살 갯수등을 입력하는 중이면.. 입력 윈도우만 작동..
		if (CGameProcedure::s_pProcMain && CGameProcedure::s_pProcMain->m_pSubProcPerTrade
			&& (CGameProcedure::s_pProcMain->m_pSubProcPerTrade->m_ePerTradeState != PER_TRADE_STATE_NONE))
		{
			if (CGameProcedure::s_pProcMain->m_pSubProcPerTrade->m_ePerTradeState == PER_TRARE_STATE_EDITTING)
			{
				if (pChild->m_szID.compare("base_tradeedit") != 0)
				{
					++itor;
					continue;
				}
			}

			if (CN3UIWndBase::s_pCountableItemEdit->IsLocked())
			{
				if (pChild->m_szID.compare("base_tradeedit") != 0)
				{
					++itor;
					continue;
				}
			}
		}

		if (pChild->m_pChildUI && pChild->m_pChildUI->IsVisible())
		{
			uint32_t dwRet = pChild->m_pChildUI->MouseProc(dwFlags, ptCur, ptOld);
			if (UI_MOUSEPROC_DONESOMETHING & dwRet)
			{ // 이경우에는 먼가 포커스를 받은 경우이다.
				pChild->MouseProc(0, ptCur, ptOld);
				m_dwMouseFlagsCur |= (UI_MOUSEPROC_DONESOMETHING | UI_MOUSEPROC_CHILDDONESOMETHING);

				SetFocusedUI(pChild); //this_ui

				return m_dwMouseFlagsCur;
			}
			else if ((UI_MOUSE_LBCLICK & dwFlags) && (UI_MOUSEPROC_INREGION & dwRet))
			{ // 영역 안을 클릭 했을때 먼가 일을 했다고 하고 리턴해버린다.
				pChild->MouseProc(0, ptCur, ptOld);
				m_dwMouseFlagsCur |= (UI_MOUSEPROC_DIALOGFOCUS);

				SetFocusedUI(pChild); //this_ui

				return m_dwMouseFlagsCur;
			}
		}

		uint32_t dwChildRet = pChild->MouseProc(dwFlags, ptCur, ptOld);
		if (UI_MOUSEPROC_DONESOMETHING & dwChildRet)
		{                         // 이경우에는 먼가 포커스를 받은 경우이다.
			m_dwMouseFlagsCur |= (UI_MOUSEPROC_DONESOMETHING | UI_MOUSEPROC_CHILDDONESOMETHING);

			SetFocusedUI(pChild); //this_ui

			return m_dwMouseFlagsCur;
		}
		else if ((UI_MOUSE_LBCLICK & dwFlags) && (UI_MOUSEPROC_INREGION & dwChildRet))
		{                         // 영역 안을 클릭 했을때 먼가 일을 했다고 하고 리턴해버린다.
			m_dwMouseFlagsCur |= (UI_MOUSEPROC_DIALOGFOCUS);

			SetFocusedUI(pChild); //this_ui

			return m_dwMouseFlagsCur;
		}
		else
			++itor;
		//else if (UI_MOUSE_LBCLICKED|UI_MOUSE_MBCLICK|UI_MOUSE_MBCLICKED|UI_MOUSE_RBCLICK|UI_MOUSE_RBCLICKED)

		m_dwMouseFlagsCur |= dwChildRet;
	}

	//	if(UI_MOUSE_LBCLICK & dwFlags) m_pUIFocused = nullptr; // 포커스 받은 UI 기록.. 아무것도 안하면.. 널이다..

	return m_dwMouseFlagsCur;
}

void CUIManager::ReorderChildList() // 다이알로그 순서 재배치
{
	int iChildCount = static_cast<int>(m_Children.size());
	if (iChildCount <= 0)
		return;

	CN3UIBase** ppBuffer     = new CN3UIBase*[iChildCount];
	int iAlwaysTopChildCount = 0;

	for (auto itor = m_Children.begin(); m_Children.end() != itor;)
	{
		CN3UIBase* pChild = (*itor);
		if (pChild->GetStyle() & UISTYLE_ALWAYSTOP)
		{
			itor                             = m_Children.erase(itor); // 우선 리스트에서 지우고
			ppBuffer[iAlwaysTopChildCount++] = pChild;

			if (iAlwaysTopChildCount >= iChildCount)
				break;
		}
		else
			++itor;
	}
	for (int i = iAlwaysTopChildCount - 1; i >= 0; --i)
		m_Children.push_front(ppBuffer[i]); // 맨앞에 넣는다. 그리는 순서를 맨 나중에 그리도록 하고 메세지를 맨 먼저 받게 하려고

	delete[] ppBuffer;
}

void CUIManager::Tick()
{
	CN3UIBase::Tick();
	if (s_pTooltipCtrl)
		s_pTooltipCtrl->Tick();
}

void CUIManager::Render()
{
	this->RenderStateSet();

	CN3UIBase::Render();          // 자식들 render
	if (s_pTooltipCtrl)
		s_pTooltipCtrl->Render(); // tooltip render

								  /*
	NOTE: there is a very weird issue with setting the render state and displaying text.
	- when the debug info is being displayed and you change window focus weird shit happens
	- currently just sticking this in here. it will work here but will end up showing during
	every game procedure which is somewhat unwanted right now...
	*/
	////////////////////////////////////////////////////////
#ifdef _DEBUG
	if (m_pDFont == nullptr)
	{
		m_pDFont = new CDFont("굴림", 10);
		m_pDFont->InitDeviceObjects(CN3Base::s_lpD3DDev);
		m_pDFont->RestoreDeviceObjects();
	}

	static std::string szDebugs[4];

	fmt::format_to(std::back_inserter(szDebugs[0]),
		"nTerrain_Polygon({}), nTerrain_Tile_Polygon({}), nShape({}), nShape_Part({}), nShape_Polygon({})",
		CN3Base::s_RenderInfo.nTerrain_Polygon, CN3Base::s_RenderInfo.nTerrain_Tile_Polygon, CN3Base::s_RenderInfo.nShape,
		CN3Base::s_RenderInfo.nShape_Part, CN3Base::s_RenderInfo.nShape_Polygon);

	fmt::format_to(std::back_inserter(szDebugs[1]), "nChr({}), nChr_Part({}), nChr_Polygon({}), nChr_Plug({}), nChr_Plug_Polygon({})",
		CN3Base::s_RenderInfo.nChr, CN3Base::s_RenderInfo.nChr_Part, CN3Base::s_RenderInfo.nChr_Polygon, CN3Base::s_RenderInfo.nChr_Plug,
		CN3Base::s_RenderInfo.nChr_Plug_Polygon);

	fmt::format_to(std::back_inserter(szDebugs[2]), "Camera : FieldOfView({:.1f}), NearPlane({:.1f}) FarPlane({:.1f})",
		RadiansToDegrees(CN3Base::s_CameraData.fFOV), CN3Base::s_CameraData.fNP, CN3Base::s_CameraData.fFP);

	if (CGameProcedure::s_pProcMain != nullptr && CGameBase::ACT_WORLD != nullptr && CGameBase::ACT_WORLD->GetSkyRef() != nullptr)
	{
		int iYear = 0, iMonth = 0, iDay = 0, iH = 0, iM = 0;
		CGameBase::ACT_WORLD->GetSkyRef()->GetGameTime(&iYear, &iMonth, &iDay, &iH, &iM);
		fmt::format_to(std::back_inserter(szDebugs[3]), "{:.2f}:FPS, {}/{}/{} : {}:{}", CN3Base::s_fFrmPerSec, iYear, iMonth, iDay, iH, iM);
	}
	else
	{
		szDebugs[3].clear();
	}

	for (int i = 0; i < 4; i++)
	{
		if (szDebugs[i].empty())
			continue;

		m_pDFont->SetText(szDebugs[i]);
		m_pDFont->DrawText(0.0f, 0.0f + i * 18, 0xFFFFFFFF, 0);
		szDebugs[i].clear();
	}
#endif
	////////////////////////////////////////////////////////

	this->RenderStateRestore();
}

void CUIManager::RenderStateSet()
{
	if (nullptr == s_lpD3DDev)
		return;

#ifdef _DEBUG
	__ASSERT(FALSE == s_sRSFU.bSet, "이전에 RenderStateSet()함수를 호출하고 RenderStateRestore()함수가 호출되지 않은 상태입니다.");
	s_sRSFU.bSet = TRUE;
#endif

	// back up old state
	s_lpD3DDev->GetRenderState(D3DRS_ZENABLE, &(s_sRSFU.dwZEnable));
	s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &(s_sRSFU.dwAlphaBlend));
	s_lpD3DDev->GetRenderState(D3DRS_SRCBLEND, &(s_sRSFU.dwSrcBlend));
	s_lpD3DDev->GetRenderState(D3DRS_DESTBLEND, &(s_sRSFU.dwDestBlend));
	s_lpD3DDev->GetRenderState(D3DRS_FOGENABLE, &(s_sRSFU.dwFog));
	/*
	s_lpD3DDev->GetTextureStageState(0, D3DTSS_MAGFILTER,   &(s_sRSFU.dwMagFilter));
	s_lpD3DDev->GetTextureStageState(0, D3DTSS_MINFILTER,   &(s_sRSFU.dwMinFilter));
	s_lpD3DDev->GetTextureStageState(0, D3DTSS_MIPFILTER,   &(s_sRSFU.dwMipFilter));
	*/
	s_lpD3DDev->GetSamplerState(0, D3DSAMP_MAGFILTER, &(s_sRSFU.dwMagFilter));
	s_lpD3DDev->GetSamplerState(0, D3DSAMP_MINFILTER, &(s_sRSFU.dwMinFilter));
	s_lpD3DDev->GetSamplerState(0, D3DSAMP_MIPFILTER, &(s_sRSFU.dwMipFilter));

	// set state
	if (D3DZB_FALSE != s_sRSFU.dwZEnable)
		s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	if (TRUE != s_sRSFU.dwAlphaBlend)
		s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	if (D3DBLEND_SRCALPHA != s_sRSFU.dwSrcBlend)
		s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	if (D3DBLEND_INVSRCALPHA != s_sRSFU.dwDestBlend)
		s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	if (FALSE != s_sRSFU.dwFog)
		s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, FALSE); // 2d도 fog를 먹는다 ㅡ.ㅡ;
	if (D3DTEXF_POINT != s_sRSFU.dwMagFilter)
		s_lpD3DDev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	if (D3DTEXF_POINT != s_sRSFU.dwMinFilter)
		s_lpD3DDev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	if (D3DTEXF_NONE != s_sRSFU.dwMipFilter)
		s_lpD3DDev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
}

void CUIManager::RenderStateRestore()
{
#ifdef _DEBUG
	__ASSERT(TRUE == s_sRSFU.bSet, "이전에 RenderStateSet()함수를 호출하지 않았습니다.");
	s_sRSFU.bSet = FALSE;
#endif

	// restore
	if (D3DZB_FALSE != s_sRSFU.dwZEnable)
		s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, s_sRSFU.dwZEnable);
	if (TRUE != s_sRSFU.dwAlphaBlend)
		s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, s_sRSFU.dwAlphaBlend);
	if (D3DBLEND_SRCALPHA != s_sRSFU.dwSrcBlend)
		s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, s_sRSFU.dwSrcBlend);
	if (D3DBLEND_INVSRCALPHA != s_sRSFU.dwDestBlend)
		s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, s_sRSFU.dwDestBlend);
	if (FALSE != s_sRSFU.dwFog)
		s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, s_sRSFU.dwFog);
	if (D3DTEXF_POINT != s_sRSFU.dwMagFilter)
		s_lpD3DDev->SetSamplerState(0, D3DSAMP_MAGFILTER, s_sRSFU.dwMagFilter);
	if (D3DTEXF_POINT != s_sRSFU.dwMinFilter)
		s_lpD3DDev->SetSamplerState(0, D3DSAMP_MINFILTER, s_sRSFU.dwMinFilter);
	if (D3DTEXF_NONE != s_sRSFU.dwMipFilter)
		s_lpD3DDev->SetSamplerState(0, D3DSAMP_MIPFILTER, s_sRSFU.dwMipFilter);
}

bool CUIManager::BroadcastIconDropMsg(__IconItemSkill* spItem)
{
	bool bFound = false;
	POINT ptCur = CGameProcedure::s_pLocalInput->MouseGetPos();

	// 윈도우들을 돌아 다니면서 검사..
	for (CN3UIBase* pChild : m_Children)
	{
		if (bFound)
			break;

		if (pChild->UIType() == UI_TYPE_ICON_MANAGER)
		{
			// 해당 윈도우가 보이고(활성화 되어 있고), 그 윈도우 영역 안에 있으면..
			if (((CN3UIWndBase*) pChild)->IsVisible() && ((CN3UIWndBase*) pChild)->IsIn(ptCur.x, ptCur.y))
			{
				// 해당 윈도우에 아이콘 드롭 메시지 함수를 호출..
				if (((CN3UIWndBase*) pChild)->ReceiveIconDrop(spItem, ptCur))
					return true;

				bFound = true;
			}
		}
	}

	// 어느 누구의 영역에도 속하지 않으면.. 해당 아이콘을 가진 윈도우에게 Cancel 메시지를 날려 준다..
	if (!bFound)
	{
		switch (CN3UIWndBase::s_sSelectedIconInfo.UIWndSelect.UIWnd)
		{
			case UIWND_INVENTORY:
				CGameProcedure::s_pProcMain->m_pUIInventory->CancelIconDrop(spItem);
				break;

			case UIWND_TRANSACTION:
				CGameProcedure::s_pProcMain->m_pUITransactionDlg->CancelIconDrop(spItem);
				break;

			case UIWND_WARE_HOUSE:
				CGameProcedure::s_pProcMain->m_pUIWareHouseDlg->CancelIconDrop(spItem);
				break;

			default:
				break;
		}
	}

	return false;
}

CN3UIBase* CUIManager::GetTopUI(bool bVisible)
{
	if (!bVisible)
	{
		if (m_Children.empty())
			return nullptr;
		else
			return *(m_Children.begin());
	}

	UIListItor it = m_Children.begin(), itEnd = m_Children.end();
	for (; it != itEnd; it++)
	{
		CN3UIBase* pUI = *(it);
		if (pUI->IsVisible())
			return pUI;
	}

	return nullptr;
}

void CUIManager::SetFocusedUI(CN3UIBase* pUI)
{
	if (nullptr == pUI)
	{
		m_pUIFocused = nullptr;
		return;
	}

	UIListItor it = m_Children.begin(), itEnd = m_Children.end();
	it = m_Children.begin();
	for (; it != itEnd; it++)
	{
		if (pUI == *it)
			break;
	}
	if (it == itEnd)
		return;

	it = m_Children.erase(it);  // 우선 리스트에서 지우고
	m_Children.push_front(pUI); // 맨앞에 넣는다. 그리는 순서를 맨 나중에 그리도록 하고 메세지를 맨 먼저 받게 하려고
	ReorderChildList();         // child list 재정렬(항상 위에 뜨는 dialog 때문에 다시 정렬한다.)

	m_pUIFocused = this->GetTopUI(true);
}

CN3UIBase* CUIManager::ReFocusUI()
{
	SetVisibleFocusedUI(GetEnableFocusTopUI(true));
	return m_pUIFocused;
}

void CUIManager::SetVisibleFocusedUI(CN3UIBase* pUI)
{
	if (nullptr == pUI)
	{
		m_pUIFocused = nullptr;
		return;
	}

	if (!pUI->IsVisible())
		return;

	UIListItor it = m_Children.begin(), itEnd = m_Children.end();

	uint32_t dwUIStyle = 0, dwUIHideStyle = 0;
	CN3UIBase* pUIHide = nullptr;

	dwUIStyle          = pUI->GetStyle();
	if (dwUIStyle & UISTYLE_SHOW_ME_ALONE)
	{
		for (; it != itEnd;)
		{
			pUIHide = *it;
			if (pUIHide == nullptr)
			{
				it = m_Children.erase(it);
				continue;
			}

			dwUIHideStyle = pUIHide->GetStyle();
			if (pUIHide->IsVisible() && pUI != pUIHide && !(dwUIHideStyle & UISTYLE_HIDE_UNABLE))
				pUIHide->SetVisibleWithNoSound(false, true);

			it++;
		} //
	}
	else if (!(dwUIStyle & UISTYLE_HIDE_UNABLE))
	{
		for (; it != itEnd;)
		{
			pUIHide = *it;
			if (pUIHide == nullptr)
			{
				it = m_Children.erase(it);
				continue;
			}

			dwUIHideStyle = pUIHide->GetStyle();

			if (pUIHide->IsVisible() && pUI != pUIHide)
			{
				if (((dwUIHideStyle & UISTYLE_SHOW_ME_ALONE) != 0)
					|| ((dwUIStyle & UISTYLE_POS_LEFT) != 0 && (dwUIHideStyle & UISTYLE_POS_LEFT) != 0)
					|| ((dwUIStyle & UISTYLE_POS_RIGHT) != 0 && (dwUIHideStyle & UISTYLE_POS_RIGHT) != 0))
					pUIHide->SetVisibleWithNoSound(false, true);
			}

			it++;
		}
	}

	it    = m_Children.begin();
	itEnd = m_Children.end();

	for (; it != itEnd; it++)
	{
		if (pUI == *it)
			break;
	}
	if (it == itEnd)
		return;

	if (!(dwUIStyle & UISTYLE_FOCUS_UNABLE))
	{
		it = m_Children.erase(it);  // 우선 리스트에서 지우고
		m_Children.push_front(pUI); // 맨앞에 넣는다. 그리는 순서를 맨 나중에 그리도록 하고 메세지를 맨 먼저 받게 하려고
		ReorderChildList();         // child list 재정렬(항상 위에 뜨는 dialog 때문에 다시 정렬한다.)
	}

	m_pUIFocused = this->GetEnableFocusTopUI(true);
}

CN3UIBase* CUIManager::GetEnableFocusTopUI(bool bVisible)
{
	if (!bVisible)
	{
		if (m_Children.empty())
			return nullptr;
		else
			return *(m_Children.begin());
	}

	UIListItor it = m_Children.begin(), itEnd = m_Children.end();
	for (; it != itEnd; it++)
	{
		CN3UIBase* pUI = *(it);
		if (pUI && pUI->IsVisible() &&                   //보이고
			!(pUI->GetStyle() & UISTYLE_FOCUS_UNABLE) && //포커스가 가능하고
			!(pUI->GetStyle() & UISTYLE_HIDE_UNABLE))    //닫힐수 있는
			return pUI;
	}

	return nullptr;
}

void CUIManager::UserMoveHideUIs()
{
	bool bHide    = false;
	UIListItor it = m_Children.begin(), itEnd = m_Children.end();
	for (; it != itEnd; it++)
	{
		CN3UIBase* pUI = *(it);

		if (pUI == nullptr)
			continue;

		if (pUI->IsVisible() && (pUI->GetStyle() & UISTYLE_USER_MOVE_HIDE))
		{
			bHide = true;
			pUI->SetVisibleWithNoSound(false, true);
		}
	}

	if (bHide)
		ReFocusUI();
}

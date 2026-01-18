// UICharacterSelect.cpp: implementation of the UICharacterSelect class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UICharacterSelect.h"
#include "APISocket.h"
#include "GameProcCharacterSelect.h"
#include "UIManager.h"
#include "text_resources.h"

#include <N3Base/N3UIString.h>
#include <N3Base/N3UITooltip.h>

CUICharacterSelect::CUICharacterSelect()
{
	m_eType        = UI_TYPE_BASE;

	m_pBtnLeft     = nullptr;
	m_pBtnRight    = nullptr;
	m_pBtnExit     = nullptr;
	m_pBtnDelete   = nullptr;
	m_pBtnBack     = nullptr;
	m_pUserInfoStr = nullptr;
}

CUICharacterSelect::~CUICharacterSelect()
{
}

void CUICharacterSelect::Release()
{
	m_pBtnLeft     = nullptr;
	m_pBtnRight    = nullptr;
	m_pBtnExit     = nullptr;
	m_pBtnDelete   = nullptr;
	m_pBtnBack     = nullptr;
	m_pUserInfoStr = nullptr;

	CN3UIBase::Release();
}

bool CUICharacterSelect::Load(File& file)
{
	if (!CN3UIBase::Load(file))
		return false;

	N3_VERIFY_UI_COMPONENT(m_pBtnLeft, GetChildByID("bt_left"));
	N3_VERIFY_UI_COMPONENT(m_pBtnRight, GetChildByID("bt_right"));
	N3_VERIFY_UI_COMPONENT(m_pBtnExit, GetChildByID("bt_exit"));
	N3_VERIFY_UI_COMPONENT(m_pBtnDelete, GetChildByID("bt_delete"));
	N3_VERIFY_UI_COMPONENT(m_pBtnBack, GetChildByID("bt_back"));
	N3_VERIFY_UI_COMPONENT(m_pUserInfoStr, GetChildByID<CN3UIString>("text00"));

	// 위치를 화면 해상도에 맞게 바꾸기...
	POINT pt {};
	RECT rc      = GetRegion();
	float fRatio = (float) s_CameraData.vp.Width / (rc.right - rc.left);

	if (m_pBtnLeft != nullptr)
	{
		pt   = m_pBtnLeft->GetPos();
		pt.x = (int) (pt.x * fRatio);
		pt.y = s_CameraData.vp.Height - 10 - m_pBtnLeft->GetHeight();
		m_pBtnLeft->SetPos(pt.x, pt.y);
	}

	if (m_pBtnRight != nullptr)
	{
		pt   = m_pBtnRight->GetPos();
		pt.x = (int) (pt.x * fRatio);
		pt.y = s_CameraData.vp.Height - 10 - m_pBtnRight->GetHeight();
		m_pBtnRight->SetPos(pt.x, pt.y);
	}

	if (m_pBtnExit != nullptr)
	{
		pt   = m_pBtnExit->GetPos();
		pt.x = (int) (pt.x * fRatio);
		pt.y = s_CameraData.vp.Height - 10 - m_pBtnExit->GetHeight();
		m_pBtnExit->SetPos(pt.x, pt.y);
	}

	if (m_pBtnBack != nullptr)
	{
		// Previous point in sane cases should be be the exit button.
		// There's a 15 pixel gap between them in the UIF's layout.
		POINT ptPrev = pt;
		pt           = m_pBtnBack->GetPos();
		pt.x         = (int) (pt.x * fRatio);
		pt.y         = ptPrev.y - 15 - m_pBtnBack->GetHeight();
		m_pBtnBack->SetPos(pt.x, pt.y);
	}

	if (m_pBtnDelete != nullptr)
	{
		pt   = m_pBtnDelete->GetPos();
		pt.x = (int) (pt.x * fRatio);
		pt.y = 20;
		m_pBtnDelete->SetPos(pt.x, pt.y);
	}

	SetRect(&rc, 0, 0, s_CameraData.vp.Width, s_CameraData.vp.Height);
	SetRegion(rc);

	return true;
}

void CUICharacterSelect::Tick()
{
	CN3UIBase::Tick();
}

bool CUICharacterSelect::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (pSender == nullptr)
		return false;

	if (!CGameProcedure::s_pUIMgr->EnableOperation())
		return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		// Rotate Left..
		if (pSender == m_pBtnLeft)
		{
			CGameProcedure::s_pProcCharacterSelect->DoJobLeft();
		}
		// Rotate Right..
		else if (pSender == m_pBtnRight)
		{
			CGameProcedure::s_pProcCharacterSelect->DojobRight();
		}
		else if (pSender == m_pBtnExit)
		{
			//			CGameProcedure::ProcActiveSet((CGameProcedure*)CGameProcedure::s_pProcLogIn); // 로그인으로 돌아간다..
			std::string szMsg = fmt::format_text_resource(IDS_CONFIRM_EXIT_GAME);
			CGameProcedure::MessageBoxPost(szMsg, "", MB_YESNO, BEHAVIOR_EXIT);
		}
		else if (pSender == m_pBtnBack)
		{
			CGameProcedure::s_bNeedReportConnectionClosed = false;
			CGameProcedure::s_pSocket->Disconnect();
			CGameProcedure::s_bNeedReportConnectionClosed = true;

			CGameProcedure::ProcActiveSet((CGameProcedure*) CGameProcedure::s_pProcLogIn); // 로그인으로 돌아간다..
		}
		else if (pSender == m_pBtnDelete)
		{
			std::string szMsg = fmt::format_text_resource(IDS_CONFIRM_DELETE_CHR);

			// NOTE: Character deletion is disabled and this resource is changed appropriately.
			// As such, rather than prompt to delete, we should simply show the new message.
#if 0
			CGameProcedure::MessageBoxPost(szMsg, "", MB_YESNO, BEHAVIOR_DELETE_CHR);
#else
			CGameProcedure::MessageBoxPost(szMsg, "", MB_OK);
#endif
		}
	}

	return true;
}

void CUICharacterSelect::DisplayChrInfo(__CharacterSelectInfo* pCSInfo)
{
	std::string szTotal;

	if (!pCSInfo->szID.empty())
	{
		std::string szClass;
		CGameBase::GetTextByClass(pCSInfo->eClass, szClass);

		// Level: %d\nSpecialty: %s\nID: %s
		szTotal = fmt::format_text_resource(IDS_CHR_SELECT_FMT_INFO, pCSInfo->iLevel, szClass, pCSInfo->szID);
	}
	else
	{
		szTotal = fmt::format_text_resource(IDS_CHR_SELECT_HINT);
	}

	if (m_pUserInfoStr != nullptr)
	{
		m_pUserInfoStr->SetVisible(true);
		m_pUserInfoStr->SetString(szTotal);
	}
}

void CUICharacterSelect::DontDisplayInfo()
{
	if (m_pUserInfoStr != nullptr)
		m_pUserInfoStr->SetVisible(false);
}

bool CUICharacterSelect::OnKeyPress(int iKey)
{
	if (CGameProcedure::s_pUIMgr->EnableOperation())
	{
		switch (iKey)
		{
			case DIK_ESCAPE:
				ReceiveMessage(m_pBtnExit, UIMSG_BUTTON_CLICK);
				return true;
			case DIK_LEFT:
				ReceiveMessage(m_pBtnLeft, UIMSG_BUTTON_CLICK);
				return true;
			case DIK_RIGHT:
				ReceiveMessage(m_pBtnRight, UIMSG_BUTTON_CLICK);
				return true;
			case DIK_NUMPADENTER:
			case DIK_RETURN:
				CGameProcedure::s_pProcCharacterSelect->CharacterSelectOrCreate();
				return true;

			default:
				break;
		}
	}

	return CN3UIBase::OnKeyPress(iKey);
}

uint32_t CUICharacterSelect::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (!m_bVisible)
		return dwRet;

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
			return dwRet; // 이전 좌표도 영역 밖이면
		}
	}
	else
	{
		// tool tip 관련
		if (s_pTooltipCtrl != nullptr)
			s_pTooltipCtrl->SetText(m_szToolTip, m_crToolTip);
	}

	if (m_pChildUI && m_pChildUI->IsVisible())
		return dwRet;

	// child에게 메세지 전달
	for (UIListItor itor = m_Children.begin(); m_Children.end() != itor; ++itor)
	{
		CN3UIBase* pChild   = (*itor);
		uint32_t dwChildRet = pChild->MouseProc(dwFlags, ptCur, ptOld);
		if (UI_MOUSEPROC_DONESOMETHING & dwChildRet)
		{ // 이경우에는 먼가 포커스를 받은 경우이다.

			dwRet |= (UI_MOUSEPROC_CHILDDONESOMETHING | UI_MOUSEPROC_DONESOMETHING);
			return dwRet;
		}
	}

	// UI 움직이는 코드
	if (UI_STATE_COMMON_MOVE != m_eState && PtInRect(&m_rcMovable, ptCur) && (dwFlags & UI_MOUSE_LBCLICK))
	{
		SetState(UI_STATE_COMMON_MOVE);
		dwRet |= UI_MOUSEPROC_DONESOMETHING;
		return dwRet;
	}

	return dwRet;
}

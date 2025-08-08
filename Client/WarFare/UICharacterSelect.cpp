// UICharacterSelect.cpp: implementation of the UICharacterSelect class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "text_resources.h"

#include "GameProcCharacterSelect.h"
#include "UICharacterSelect.h"
#include "UIManager.h"

#include <N3Base/N3UIString.h>
#include <N3Base/N3UITooltip.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUICharacterSelect::CUICharacterSelect()
{
	m_eType = UI_TYPE_BASE;
	CUICharacterSelect::Release();

	m_pBtnLeft		= NULL;
	m_pBtnRight		= NULL;
	m_pBtnExit		= NULL;
	m_pBtnDelete	= NULL;
}

CUICharacterSelect::~CUICharacterSelect()
{
}

void CUICharacterSelect::Release()
{
	m_pBtnLeft		= NULL;
	m_pBtnRight		= NULL;
	m_pBtnExit		= NULL;
	m_pBtnDelete	= NULL;

	CN3UIBase::Release();
}

bool CUICharacterSelect::Load(HANDLE hFile)
{
	CN3UIBase::Load(hFile);

	//PrintChildIDs();

	m_pBtnLeft = this->GetChildByID("bt_left");		__ASSERT(m_pBtnLeft, "NULL UI Component!!");
	m_pBtnRight = this->GetChildByID("bt_right");	__ASSERT(m_pBtnRight, "NULL UI Component!!");
	m_pBtnExit = this->GetChildByID("bt_exit");		__ASSERT(m_pBtnExit, "NULL UI Component!!");
	m_pBtnDelete = this->GetChildByID("bt_delete");	__ASSERT(m_pBtnDelete, "NULL UI Component!!");

	GetChildByID("bt_back")->SetVisible(false); // will want to add this

	// 위치를 화면 해상도에 맞게 바꾸기...
	POINT pt;
	RECT rc = this->GetRegion();
	float fRatio = (float)s_CameraData.vp.Width / (rc.right - rc.left);
//	if(pBtnLeft) { pt = pBtnLeft->GetPos(); pt.x *= fRatio; pt.y *= fRatio; pBtnLeft->SetPos(pt.x, pt.y); }
//	if(pBtnRight) { pt = pBtnRight->GetPos(); pt.x *= fRatio; pt.y *= fRatio; pBtnRight->SetPos(pt.x, pt.y); }
//	if(pBtnExit) { pt = pBtnExit->GetPos(); pt.x *= fRatio; pt.y *= fRatio; pBtnExit->SetPos(pt.x, pt.y); }
//	if(pBtnDelete) { pt = pBtnDelete->GetPos(); pt.x *= fRatio; pt.y *= fRatio; pBtnDelete->SetPos(pt.x, pt.y); }
	if(m_pBtnLeft) { pt = m_pBtnLeft->GetPos(); pt.x = (int)(pt.x * fRatio); pt.y = s_CameraData.vp.Height - 10 - m_pBtnLeft->GetHeight(); m_pBtnLeft->SetPos(pt.x, pt.y); }
	if(m_pBtnRight) { pt = m_pBtnRight->GetPos(); pt.x = (int)(pt.x * fRatio); pt.y = s_CameraData.vp.Height - 10 - m_pBtnRight->GetHeight(); m_pBtnRight->SetPos(pt.x, pt.y); }
	if(m_pBtnExit) { pt = m_pBtnExit->GetPos(); pt.x = (int)(pt.x * fRatio); pt.y = s_CameraData.vp.Height - 10 - m_pBtnExit->GetHeight(); m_pBtnExit->SetPos(pt.x, pt.y); }
	if(m_pBtnDelete) { pt = m_pBtnDelete->GetPos(); pt.x = (int)(pt.x * fRatio); pt.y = 20; m_pBtnDelete->SetPos(pt.x, pt.y); }

	SetRect(&rc, 0, 0, s_CameraData.vp.Width, s_CameraData.vp.Height);
	this->SetRegion(rc);

	return true;
}

void CUICharacterSelect::Tick()
{
	CN3UIBase::Tick();
}

bool CUICharacterSelect::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if(NULL == pSender) return false;
	if(!CGameProcedure::s_pUIMgr->EnableOperation()) return false;

	if( dwMsg == UIMSG_BUTTON_CLICK )
	{
		if ( pSender->m_szID == "bt_left" )	// Karus
		{
			// Rotate Left..
			CGameProcedure::s_pProcCharacterSelect->DoJobLeft();
		}
		else
		if ( pSender->m_szID == "bt_right" )	// Elmorad
		{
			// Rotate Right..
			CGameProcedure::s_pProcCharacterSelect->DojobRight();
		}
		else
		if ( pSender->m_szID == "bt_exit" )	// Elmorad
		{
//			CGameProcedure::ProcActiveSet((CGameProcedure*)CGameProcedure::s_pProcLogIn); // 로그인으로 돌아간다..
			std::string szMsg = fmt::format_text_resource(IDS_CONFIRM_EXIT_GAME);
			CGameProcedure::MessageBoxPost(szMsg, "", MB_YESNO, BEHAVIOR_EXIT);
		}
		else
		if ( pSender->m_szID == "bt_delete" )	// Elmorad
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
	CN3UIBase*	m_pUserInfoStr;
	std::string szTotal;

	m_pUserInfoStr = GetChildByID("text00"); __ASSERT(m_pUserInfoStr, "NULL UI Component!!");

	if (!pCSInfo->szID.empty())
	{
		std::string szClass;
		CGameBase::GetTextByClass(pCSInfo->eClass, szClass);

		// Level: %d\nSpecialty: %s\nID: %s
		szTotal = fmt::format_text_resource(IDS_CHR_SELECT_FMT_INFO,
			pCSInfo->iLevel, szClass, pCSInfo->szID);
	}
	else
	{
		szTotal = fmt::format_text_resource(IDS_CHR_SELECT_HINT);
	}

	if (m_pUserInfoStr != nullptr)
	{
		m_pUserInfoStr->SetVisible(true);
		((CN3UIString*) m_pUserInfoStr)->SetString(szTotal);
	}
}

void CUICharacterSelect::DontDisplayInfo()
{
	CN3UIBase*	m_pUserInfoStr;
	m_pUserInfoStr = GetChildByID("text00"); __ASSERT(m_pUserInfoStr, "NULL UI Component!!");

	if ( m_pUserInfoStr ) m_pUserInfoStr->SetVisible(false);
}

bool CUICharacterSelect::OnKeyPress(int iKey)
{
	if(CGameProcedure::s_pUIMgr->EnableOperation())
	{
		switch(iKey)
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
		}
	}

	return CN3UIBase::OnKeyPress(iKey);
}

uint32_t CUICharacterSelect::MouseProc(uint32_t dwFlags, const POINT &ptCur, const POINT &ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (!m_bVisible) return dwRet;

	// UI 움직이는 코드
	if (UI_STATE_COMMON_MOVE == m_eState)
	{
		if (dwFlags&UI_MOUSE_LBCLICKED)
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

	if(false == IsIn(ptCur.x, ptCur.y))	// 영역 밖이면
	{
		if(false == IsIn(ptOld.x, ptOld.y))
		{
			return dwRet;// 이전 좌표도 영역 밖이면 
		}
	}
	else
	{
		// tool tip 관련
		if (s_pTooltipCtrl != nullptr)
			s_pTooltipCtrl->SetText(m_szToolTip, m_crToolTip);
	}

	if(m_pChildUI && m_pChildUI->IsVisible())
		return dwRet;

	// child에게 메세지 전달
	for(UIListItor itor = m_Children.begin(); m_Children.end() != itor; ++itor)
	{
		CN3UIBase* pChild = (*itor);
		uint32_t dwChildRet = pChild->MouseProc(dwFlags, ptCur, ptOld);
		if (UI_MOUSEPROC_DONESOMETHING & dwChildRet)
		{	// 이경우에는 먼가 포커스를 받은 경우이다.

			dwRet |= (UI_MOUSEPROC_CHILDDONESOMETHING|UI_MOUSEPROC_DONESOMETHING);
			return dwRet;
		}
	}

	// UI 움직이는 코드
	if (UI_STATE_COMMON_MOVE != m_eState && 
			PtInRect(&m_rcMovable, ptCur) && (dwFlags&UI_MOUSE_LBCLICK) )
	{
		SetState(UI_STATE_COMMON_MOVE);
		dwRet |= UI_MOUSEPROC_DONESOMETHING;
		return dwRet;
	}

	return dwRet;
}

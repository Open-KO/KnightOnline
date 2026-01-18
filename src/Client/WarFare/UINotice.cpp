// UINotice.cpp: implementation of the CUINotice class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UINotice.h"
#include "GameProcedure.h"
#include "UIManager.h"

#include <N3Base/N3UIString.h>
#include <N3Base/N3UIScrollBar.h>
#include <N3Base/N3UIButton.h>

CUINotice::CUINotice()
{
	m_pText_Notice = nullptr;
	m_pScrollBar   = nullptr;
	m_pBtn_OK      = nullptr;
}

CUINotice::~CUINotice()
{
	m_Texts.clear();
}

void CUINotice::Release()
{
	m_Texts.clear();
	CN3UIBase::Release();
}

bool CUINotice::Load(File& file)
{
	if (CN3UIBase::Load(file) == false)
		return false;

	m_pText_Notice = GetChildByID<CN3UIString>("Text_Notice");
	m_pScrollBar   = GetChildByID<CN3UIScrollBar>("ScrollBar");
	m_pBtn_OK      = GetChildByID<CN3UIButton>("Btn_Quit");

	if (m_pScrollBar)
	{
		m_pScrollBar->SetRange(0, 100);
	}

	return true;
}

bool CUINotice::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (nullptr == pSender)
		return false;

	//s_CameraData.vp;  //불러 오는 과정을 살펴본다
	//uint32_t mm = s_CameraData.vp.Height;
	//uint32_t ss = s_CameraData.vp.Width;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtn_OK)
		{
			if (m_pText_Notice)
				m_pText_Notice->SetString("");
			SetVisible(false);
		}
	}
	else if (dwMsg == UIMSG_SCROLLBAR_POS)
	{
		if (pSender == m_pScrollBar)
		{
			// 스크롤바에 맞는 채팅 Line 설정
			// int iCurLinePos = m_pScrollBar->GetCurrentPos();
		}
	}

	return true;
}

void CUINotice::GenerateText()
{
	if (m_pText_Notice == nullptr)
		return;

	// 글자수를 센다..
	size_t textLength = 0;
	auto it = m_Texts.begin(), itEnd = m_Texts.end();
	for (; it != itEnd; it++)
		textLength += it->size() + 3; // LineFeed, Carriage return

	if (textLength == 0)
		return;

	std::string szBuff;
	szBuff.reserve(textLength * 2);

	// 글자들을 붙이고  // LineFeed, Carriage return 을 붙인다.
	it    = m_Texts.begin();
	itEnd = m_Texts.end();
	for (; it != itEnd; it++)
	{
		szBuff += *it;
		szBuff += "\n";
	}

	m_pText_Notice->SetString(szBuff); // 글자 적용..
}

bool CUINotice::OnKeyPress(int iKey)
{
	if (iKey == DIK_ESCAPE || iKey == DIK_RETURN)
	{
		ReceiveMessage(m_pBtn_OK, UIMSG_BUTTON_CLICK);
		return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

void CUINotice::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);
	if (bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
		CGameProcedure::s_pUIMgr->ReFocusUI(); //this_ui
}

void CUINotice::RemoveNotice()
{
	m_Texts.clear();
}

// UIHelp.cpp: implementation of the CUIHelp class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UIHelp.h"
#include "GameProcedure.h"
#include "UIManager.h"

#include <N3Base/N3UIButton.h>

CUIHelp::CUIHelp()
{
	m_pBtn_Close = nullptr;
	m_pBtn_Prev  = nullptr;
	m_pBtn_Next  = nullptr;

	for (int i = 0; i < MAX_HELP_PAGE; i++)
		m_pPages[i] = nullptr;
}

CUIHelp::~CUIHelp()
{
}

bool CUIHelp::Load(File& file)
{
	if (!CN3UIBase::Load(file))
		return false;

	int iPageCount = 0;
	std::string szID;
	for (int i = 0; i < MAX_HELP_PAGE; i++)
	{
		szID = fmt::format("Page{}", i);
		N3_VERIFY_UI_COMPONENT(m_pPages[i], GetChildByID(szID));
		if (m_pPages[i] != nullptr)
		{
			m_pPages[i]->SetVisible(0 == iPageCount);
			iPageCount++;
		}
	}

	N3_VERIFY_UI_COMPONENT(m_pBtn_Close, GetChildByID<CN3UIButton>("Btn_Close"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Prev, GetChildByID<CN3UIButton>("Btn_Left"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Next, GetChildByID<CN3UIButton>("Btn_Right"));

	return true;
}

bool CUIHelp::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		int iPage = -1;
		for (int i = 0; i < MAX_HELP_PAGE; i++)
		{
			if (m_pPages[i] != nullptr && m_pPages[i]->IsVisible())
			{
				iPage = i;
				break;
			}
		}

		int iPagePrev = iPage;

		if (pSender == m_pBtn_Prev)
		{
			iPage--;
			if (iPage < 0)
				iPage = 0;
		}
		else if (pSender == m_pBtn_Next)
		{
			iPage++;
			if (iPage >= MAX_HELP_PAGE)
				iPage = 0;
		}
		else if (pSender == m_pBtn_Close)
		{
			SetVisible(false);
		}

		if (iPagePrev != iPage)
		{
			for (int i = 0; i < MAX_HELP_PAGE; i++)
			{
				if (m_pPages[i] == nullptr)
					continue;

				m_pPages[i]->SetVisible(false);
				if (i == iPage)
					m_pPages[i]->SetVisible(true);
			}
		}
	}

	return false;
}

void CUIHelp::Release()
{
	CN3UIBase::Release();

	m_pBtn_Close = nullptr;
	m_pBtn_Prev  = nullptr;
	m_pBtn_Next  = nullptr;

	for (int i = 0; i < MAX_HELP_PAGE; i++)
		m_pPages[i] = nullptr;
}

bool CUIHelp::OnKeyPress(int iKey)
{
	switch (iKey)
	{
		case DIK_PRIOR:
			ReceiveMessage(m_pBtn_Prev, UIMSG_BUTTON_CLICK);
			return true;

		case DIK_NEXT:
			ReceiveMessage(m_pBtn_Next, UIMSG_BUTTON_CLICK);
			return true;

		case DIK_ESCAPE:
			ReceiveMessage(m_pBtn_Close, UIMSG_BUTTON_CLICK);
			return true;

		default:
			break;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

void CUIHelp::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);
	if (bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
		CGameProcedure::s_pUIMgr->ReFocusUI(); //this_ui
}

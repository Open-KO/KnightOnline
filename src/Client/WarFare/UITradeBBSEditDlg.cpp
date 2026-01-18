// UITradeBBSEditDlg.cpp: implementation of the CUITradeBBSEditDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UITradeBBSEditDlg.h"
#include "LocalInput.h"

#include <N3Base/N3UIEdit.h>
#include <N3Base/N3UIButton.h>

CUITradeBBSEditDlg::CUITradeBBSEditDlg()
{
	m_pEditTitle       = nullptr;
	m_pEditPrice       = nullptr;
	m_pEditExplanation = nullptr;

	m_pBtn_Ok          = nullptr;
	m_pBtn_Cancel      = nullptr;
}

CUITradeBBSEditDlg::~CUITradeBBSEditDlg()
{
}

std::string CUITradeBBSEditDlg::GetTradeTitle()
{
	if (m_pEditTitle)
		return m_pEditTitle->GetString();
	return "";
}

std::string CUITradeBBSEditDlg::GetTradeExplanation()
{
	if (m_pEditExplanation)
		return m_pEditExplanation->GetString();
	return "";
}

int CUITradeBBSEditDlg::GetPrice()
{
	if (m_pEditPrice)
		return atoi(m_pEditPrice->GetString().c_str());

	return 0;
}

void CUITradeBBSEditDlg::ShowWindow(int iID, CN3UIBase* pParent)
{
	CN3UIBase::ShowWindow(iID, pParent);
}

bool CUITradeBBSEditDlg::Load(File& file)
{
	if (CN3UIBase::Load(file) == false)
		return false;

	N3_VERIFY_UI_COMPONENT(m_pEditTitle, GetChildByID<CN3UIEdit>("edit_name"));
	N3_VERIFY_UI_COMPONENT(m_pEditPrice, GetChildByID<CN3UIEdit>("edit_price"));
	N3_VERIFY_UI_COMPONENT(m_pEditExplanation, GetChildByID<CN3UIEdit>("edit_memo"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Ok, GetChildByID<CN3UIButton>("btn_Ok"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Cancel, GetChildByID<CN3UIButton>("btn_Cancel"));

	m_pEditTitle->SetMaxString(18);
	m_pEditPrice->SetMaxString(8);
	m_pEditExplanation->SetMaxString(40);

	return true;
}

bool CUITradeBBSEditDlg::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (nullptr == pSender)
		return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtn_Ok)
		{
			if (m_pParentUI)
				m_pParentUI->CallBackProc(m_iChildID, 1);
			this->SetVisible(false);
		}
		else if (pSender == m_pBtn_Cancel)
		{
			if (m_pParentUI)
				m_pParentUI->CallBackProc(m_iChildID, 2);
			this->SetVisible(false);
		}
	}

	return true;
}

void CUITradeBBSEditDlg::SetVisible(bool bVisible)
{
	if (bVisible)
	{
		if (m_pEditTitle)
		{
			m_pEditTitle->SetString("");
			m_pEditTitle->SetFocus();
		}
		if (m_pEditPrice)
			m_pEditPrice->SetString("");
		if (m_pEditExplanation)
			m_pEditExplanation->SetString("");
	}
	else
	{
		if (m_pEditTitle)
			m_pEditTitle->SetString("");
		if (m_pEditPrice)
			m_pEditPrice->SetString("");
		if (m_pEditExplanation)
			m_pEditExplanation->SetString("");
		if (m_pEditTitle)
			m_pEditTitle->KillFocus();
		if (m_pEditPrice)
			m_pEditPrice->KillFocus();
		if (m_pEditExplanation)
			m_pEditExplanation->KillFocus();
	}
	CN3UIBase::SetVisible(bVisible);
}

bool CUITradeBBSEditDlg::OnKeyPress(int iKey)
{
	switch (iKey)
	{
		case DIK_ESCAPE:
			ReceiveMessage(m_pBtn_Cancel, UIMSG_BUTTON_CLICK);
			return true;

		case DIK_RETURN:
			ReceiveMessage(m_pBtn_Ok, UIMSG_BUTTON_CLICK);
			return true;

		case DIK_TAB:
			if (m_pEditTitle->HaveFocus())
				m_pEditPrice->SetFocus();
			else if (m_pEditPrice->HaveFocus())
				m_pEditExplanation->SetFocus();
			else if (m_pEditExplanation->HaveFocus())
				m_pEditTitle->SetFocus();
			return true;

		default:
			break;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

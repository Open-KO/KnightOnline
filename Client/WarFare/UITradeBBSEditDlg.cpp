// UITradeBBSEditDlg.cpp: implementation of the CUITradeBBSEditDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UITradeBBSEditDlg.h"
#include "LocalInput.h"

#include <N3Base/N3UIEdit.h>
#include <N3Base/N3UIButton.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUITradeBBSEditDlg::CUITradeBBSEditDlg()
{
	m_pEditTitle		= NULL;
	m_pEditPrice		= NULL;
	m_pEditExplanation	= NULL;

	m_pBtn_Ok			= NULL;
	m_pBtn_Cancel		= NULL;
}

CUITradeBBSEditDlg::~CUITradeBBSEditDlg()
{

}

std::string CUITradeBBSEditDlg::GetTradeTitle()
{
	if(m_pEditTitle)
		return m_pEditTitle->GetString();
	return "";
}

std::string CUITradeBBSEditDlg::GetTradeExplanation()
{
	if(m_pEditExplanation)
		return m_pEditExplanation->GetString();
	return "";
}

int	CUITradeBBSEditDlg::GetPrice()
{
	if(m_pEditPrice)
		return atoi(m_pEditPrice->GetString().c_str());

	return 0;
}

void CUITradeBBSEditDlg::ShowWindow(int iID, CN3UIBase *pParent)
{
	CN3UIBase::ShowWindow(iID, pParent);
}

bool CUITradeBBSEditDlg::Load(HANDLE hFile)
{
	if(CN3UIBase::Load(hFile)==false) return false;

	m_pEditTitle		= (CN3UIEdit*)(this->GetChildByID("edit_name"));	__ASSERT(m_pEditTitle, "NULL UI Component!!!");
	m_pEditPrice		= (CN3UIEdit*)(this->GetChildByID("edit_price"));	__ASSERT(m_pEditPrice, "NULL UI Component!!!");
	m_pEditExplanation	= (CN3UIEdit*)(this->GetChildByID("edit_memo"));	__ASSERT(m_pEditExplanation, "NULL UI Component!!!");
	m_pBtn_Ok			= (CN3UIButton*)(this->GetChildByID("btn_Ok"));		__ASSERT(m_pBtn_Ok, "NULL UI Component!!!");
	m_pBtn_Cancel		= (CN3UIButton*)(this->GetChildByID("btn_Cancel"));	__ASSERT(m_pBtn_Cancel, "NULL UI Component!!!");

	m_pEditTitle->SetMaxString(18);
	m_pEditPrice->SetMaxString(8);
	m_pEditExplanation->SetMaxString(40);

	return true;
}

bool CUITradeBBSEditDlg::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if(NULL == pSender) return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if(pSender == m_pBtn_Ok)
		{
			if(m_pParentUI) m_pParentUI->CallBackProc(m_iChildID, 1);
			this->SetVisible(false);
		}
		else if(pSender == m_pBtn_Cancel)
		{
			if(m_pParentUI) m_pParentUI->CallBackProc(m_iChildID, 2);
			this->SetVisible(false);
		}
	}

	return true;
}

void CUITradeBBSEditDlg::SetVisible(bool bVisible)
{
	if(bVisible)
	{
		if(m_pEditTitle)
		{
			m_pEditTitle->SetString("");
			m_pEditTitle->SetFocus();
		}
		if(m_pEditPrice)		m_pEditPrice->SetString("");
		if(m_pEditExplanation)	m_pEditExplanation->SetString("");
	}
	else
	{
		if(m_pEditTitle)		m_pEditTitle->SetString("");
		if(m_pEditPrice)		m_pEditPrice->SetString("");
		if(m_pEditExplanation)	m_pEditExplanation->SetString("");
		if(m_pEditTitle)		m_pEditTitle->KillFocus();
		if(m_pEditPrice)		m_pEditPrice->KillFocus();
		if(m_pEditExplanation)	m_pEditExplanation->KillFocus();
	}
	CN3UIBase::SetVisible(bVisible);
}

bool CUITradeBBSEditDlg::OnKeyPress(int iKey)
{
	switch(iKey)
	{
	case DIK_ESCAPE:
		ReceiveMessage(m_pBtn_Cancel, UIMSG_BUTTON_CLICK);
		return true;
	case DIK_RETURN:
		ReceiveMessage(m_pBtn_Ok, UIMSG_BUTTON_CLICK);
		return true;
	case DIK_TAB:
		{
			if(m_pEditTitle->HaveFocus())
				m_pEditPrice->SetFocus();
			else if(m_pEditPrice->HaveFocus())
				m_pEditExplanation->SetFocus();
			else if(m_pEditExplanation->HaveFocus())
				m_pEditTitle->SetFocus();
		}
		return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

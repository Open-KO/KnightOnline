// UIWarp.cpp: implementation of the UIWarp class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UIWarp.h"
#include "UIManager.h"
#include "GameProcMain.h"
#include "text_resources.h"

#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIString.h>
#include <N3Base/N3UIList.h>

CUIWarp::CUIWarp()
{
	m_pBtn_Ok         = nullptr;
	m_pBtn_Cancel     = nullptr;
	m_pList_Infos     = nullptr;
	m_pText_Agreement = nullptr; // 동의 사항..
}

CUIWarp::~CUIWarp()
{
}

bool CUIWarp::Load(File& file)
{
	if (!CN3UIBase::Load(file))
		return false;

	N3_VERIFY_UI_COMPONENT(m_pBtn_Ok, GetChildByID<CN3UIButton>("Btn_Ok"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Cancel, GetChildByID<CN3UIButton>("Btn_Cancel"));
	N3_VERIFY_UI_COMPONENT(m_pList_Infos, GetChildByID<CN3UIList>("List_Infos"));
	N3_VERIFY_UI_COMPONENT(m_pText_Agreement, GetChildByID<CN3UIString>("Text_Agreement"));

	return true;
}

bool CUIWarp::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg & UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtn_Ok)
		{
			CGameProcedure::s_pProcMain->MsgSend_Warp();
		}
		else if (pSender == m_pBtn_Cancel)
		{
		}
		this->SetVisible(false);
	}
	else if (dwMsg & UIMSG_LIST_SELCHANGE)
	{
		if (pSender == m_pList_Infos)
		{
			this->UpdateAgreement(); // 동의문 업데이트..
		}
	}
	else if (dwMsg & UIMSG_LIST_DBLCLK)
	{
		CGameProcedure::s_pProcMain->MsgSend_Warp();
		this->SetVisible(false);
	}

	return true;
}

void CUIWarp::InfoAdd(__WarpInfo&& WI)
{
	m_ListInfos.push_back(std::move(WI));
}

bool CUIWarp::InfoGetCur(__WarpInfo& WI)
{
	if (m_pList_Infos == nullptr)
		return false;

	int iSel = m_pList_Infos->GetCurSel();
	if (iSel < 0 || iSel >= static_cast<int>(m_ListInfos.size()))
		return false;

	auto it = m_ListInfos.begin();
	std::advance(it, iSel);
	WI = *it;

	return true;
}

void CUIWarp::UpdateList()
{
	if (m_pList_Infos == nullptr)
		return;

	m_pList_Infos->ResetContent();
	it_WI it = m_ListInfos.begin(), itEnd = m_ListInfos.end();
	for (; it != itEnd; it++)
	{
		m_pList_Infos->AddString(it->szName);
	}

	m_pList_Infos->SetCurSel(0);
	this->UpdateAgreement();
}

void CUIWarp::UpdateAgreement()
{
	if (m_pList_Infos == nullptr || m_pText_Agreement == nullptr)
		return;

	int iSel = m_pList_Infos->GetCurSel();
	if (iSel < 0 || iSel >= static_cast<int>(m_ListInfos.size()))
		return;

	auto it = m_ListInfos.begin();
	std::advance(it, iSel);
	m_pText_Agreement->SetString(it->szAgreement);
}

void CUIWarp::Reset()
{
	m_ListInfos.clear();
	this->UpdateList();
}

void CUIWarp::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);
	if (bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
		CGameProcedure::s_pUIMgr->ReFocusUI(); //this_ui
}

bool CUIWarp::OnKeyPress(int iKey)
{
	switch (iKey)
	{
		case DIK_ESCAPE:
			ReceiveMessage(m_pBtn_Cancel, UIMSG_BUTTON_CLICK);
			return true;

		case DIK_RETURN:
			ReceiveMessage(m_pBtn_Ok, UIMSG_BUTTON_CLICK);
			return true;

		default:
			break;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

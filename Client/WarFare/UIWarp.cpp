// UIWarp.cpp: implementation of the UIWarp class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UIWarp.h"
#include "UIManager.h"
#include "GameProcMain.h"
#include "text_resources.h"

#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIString.h>
#include <N3Base/N3UIList.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUIWarp::CUIWarp()
{
	m_pBtn_Ok = NULL;
	m_pBtn_Cancel = NULL;
	
	m_pList_Infos = NULL;
	m_pText_Agreement = NULL; // 동의 사항..
}

CUIWarp::~CUIWarp()
{

}

bool CUIWarp::Load(HANDLE hFile)
{
	if(CN3UIBase::Load(hFile)==false) return false;

	m_pBtn_Ok = (CN3UIButton*)GetChildByID("Btn_Ok");							__ASSERT(m_pBtn_Ok, "NULL UI Component!!");
	m_pBtn_Cancel = (CN3UIButton*)GetChildByID("Btn_Cancel");					__ASSERT(m_pBtn_Cancel, "NULL UI Component!!");
	m_pList_Infos = (CN3UIList*)GetChildByID("List_Infos");						__ASSERT(m_pList_Infos, "NULL UI Component!!");
	m_pText_Agreement = (CN3UIString*)(this->GetChildByID("Text_Agreement"));	__ASSERT(m_pText_Agreement, "NULL UI Component!!!");

	return true;
}

bool CUIWarp::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg & UIMSG_BUTTON_CLICK)
	{
		if(pSender == m_pBtn_Ok)
		{
			CGameProcedure::s_pProcMain->MsgSend_Warp();
		}
		else if(pSender == m_pBtn_Cancel)
		{
		}
		this->SetVisible(false);
	}
	else if(dwMsg & UIMSG_LIST_SELCHANGE)
	{
		if(pSender == m_pList_Infos)
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

void CUIWarp::InfoAdd(const __WarpInfo& WI)
{
	m_ListInfos.push_back(WI);
}

bool CUIWarp::InfoGetCur(__WarpInfo& WI)
{
	if (NULL == m_pList_Infos)
		return false;
	
	int iSel = m_pList_Infos->GetCurSel();
	if (iSel < 0
		|| iSel >= static_cast<int>(m_ListInfos.size()))
		return false;
	
	auto it = m_ListInfos.begin();
	std::advance(it, iSel);
	WI = *it;

	return true;
}

void CUIWarp::UpdateList()
{
	if(NULL == m_pList_Infos) return;

	m_pList_Infos->ResetContent();
	it_WI it = m_ListInfos.begin(), itEnd = m_ListInfos.end();
	for(; it != itEnd; it++)
	{
		m_pList_Infos->AddString(it->szName);
	}

	m_pList_Infos->SetCurSel(0);
	this->UpdateAgreement();
}

void CUIWarp::UpdateAgreement()
{
	if(NULL == m_pList_Infos || NULL == m_pText_Agreement) return;
	int iSel = m_pList_Infos->GetCurSel();
	if (iSel < 0
		|| iSel >= static_cast<int>(m_ListInfos.size()))
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
	if(bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
		CGameProcedure::s_pUIMgr->ReFocusUI();//this_ui
}

bool CUIWarp::OnKeyPress(int iKey)
{
	switch(iKey)
	{
	case DIK_ESCAPE:
		ReceiveMessage(m_pBtn_Cancel, UIMSG_BUTTON_CLICK);
		return true;
	case DIK_RETURN:
		ReceiveMessage(m_pBtn_Ok, UIMSG_BUTTON_CLICK);
		return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

// UIQuestTalk.cpp: implementation of the CUIQuestTalk class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UIQuestTalk.h"
#include "GameDef.h"
#include "GameProcedure.h"
#include "UIManager.h"
#include "APISocket.h"

#include <N3Base/N3UIString.h>
#include <N3Base/N3UIButton.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUIQuestTalk::CUIQuestTalk()
{
	m_pTextTalk			= nullptr;
	m_pBtnOk			= nullptr;
	m_pBtnClose			= nullptr;
	m_pBtnUpperEvent	= nullptr;
	m_pBtnNext			= nullptr;
	m_pBtnOkRight		= nullptr;
	m_pBtnPre			= nullptr;
	m_iNumTalk			= 0;
	m_iCurTalk			= 0;
}

CUIQuestTalk::~CUIQuestTalk()
{
}

void CUIQuestTalk::Open(Packet& pkt)
{
	m_iNumTalk = 0;
	m_iCurTalk = 0;

	// NOTE(srmeier): two -1s before text ids
	int index = pkt.read<uint32_t>();
	index = pkt.read<uint32_t>();

	for(int i=0;i<MAX_STRING_TALK;i++)
	{
		m_szTalk[i] = "";

		index = pkt.read<uint32_t>();
		__TABLE_QUEST_TALK* pTbl_Quest_Talk = CGameBase::s_pTbl_QuestTalk.Find(index);
		if(pTbl_Quest_Talk)
		{
			m_szTalk[i] = pTbl_Quest_Talk->szTalk;
			m_iNumTalk++;
		}
	}

	m_pTextTalk->SetString(m_szTalk[m_iCurTalk]);
	SetVisible(true);
}

bool CUIQuestTalk::ReceiveMessage(CN3UIBase *pSender, uint32_t dwMsg)
{
	if( dwMsg == UIMSG_BUTTON_CLICK )
	{
		if(pSender == m_pBtnOk)
		{
			m_iCurTalk++;
			if(m_iCurTalk>=m_iNumTalk)
			{
				m_iCurTalk = 0;
				SetVisible(false);
			}
			else
			{
				m_pTextTalk->SetString(m_szTalk[m_iCurTalk]);
			}
		}
		else if (pSender == m_pBtnClose)
		{
			SetVisible(false);
		}
	}

	return true;
}

bool CUIQuestTalk::Load(HANDLE hFile)
{
	if (!CN3UIBase::Load(hFile))
		return false;

	N3_VERIFY_UI_COMPONENT(m_pTextTalk,			(CN3UIString*) GetChildByID("Text_Talk"));
	N3_VERIFY_UI_COMPONENT(m_pBtnOk,			(CN3UIButton*) GetChildByID("btn_Ok_center"));

	// NOTE(srmeier): new stuff:
	N3_VERIFY_UI_COMPONENT(m_pBtnClose,			(CN3UIButton*) GetChildByID("btn_close"));
	N3_VERIFY_UI_COMPONENT(m_pBtnUpperEvent,	(CN3UIButton*) GetChildByID("btn_UpperEvent"));
	N3_VERIFY_UI_COMPONENT(m_pBtnNext,			(CN3UIButton*) GetChildByID("btn_Next"));
	N3_VERIFY_UI_COMPONENT(m_pBtnOkRight,		(CN3UIButton*) GetChildByID("btn_Ok_right"));
	N3_VERIFY_UI_COMPONENT(m_pBtnPre,			(CN3UIButton*) GetChildByID("btn_Pre"));

	if (m_pBtnUpperEvent != nullptr)
		m_pBtnUpperEvent->SetVisible(false);

	if (m_pBtnNext != nullptr)
		m_pBtnNext->SetVisible(false);

	if (m_pBtnOkRight != nullptr)
		m_pBtnOkRight->SetVisible(false);

	if (m_pBtnPre != nullptr)
		m_pBtnPre->SetVisible(false);

	return true;
}

bool CUIQuestTalk::OnKeyPress(int iKey)
{
	switch(iKey)
	{
	case DIK_ESCAPE:
		SetVisible(false);
		return true;
	case DIK_RETURN:
		ReceiveMessage(m_pBtnOk, UIMSG_BUTTON_CLICK);
		return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

void CUIQuestTalk::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);
	if(bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
		CGameProcedure::s_pUIMgr->ReFocusUI();//this_ui
}

void CUIQuestTalk::Release()
{
	CN3UIBase::Release();

	m_pTextTalk			= nullptr;
	m_pBtnOk			= nullptr;
	m_pBtnClose			= nullptr;
	m_pBtnUpperEvent	= nullptr;
	m_pBtnNext			= nullptr;
	m_pBtnOkRight		= nullptr;
	m_pBtnPre			= nullptr;
	m_iNumTalk			= 0;
	m_iCurTalk			= 0;
}

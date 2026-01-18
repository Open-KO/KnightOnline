// UILevelGuide.cpp: implementation of the CUILevelGuide class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UILevelGuide.h"
#include "GameDef.h"
#include "GameProcMain.h"
#include "PlayerMySelf.h"
#include "UIManager.h"
#include "text_resources.h"

#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIEdit.h>
#include <N3Base/N3UIScrollBar.h>
#include <N3Base/N3UIString.h>

#include <algorithm>

CUILevelGuide::CUILevelGuide()
{
	m_pEdit_Level  = nullptr;
	m_pText_Page   = nullptr;
	m_pBtn_Check   = nullptr;
	m_pText_Level  = nullptr;
	m_pBtn_Up      = nullptr;
	m_pBtn_Down    = nullptr;
	m_pBtn_Cancel  = nullptr;

	m_iSearchLevel = 0;
	m_iPageNo      = 0;

	for (int i = 0; i < MAX_QUESTS_PER_PAGE; i++)
	{
		m_pScroll_Guide[i] = nullptr;
		m_pText_Guide[i]   = nullptr;
		m_pText_Title[i]   = nullptr;
	}
}

CUILevelGuide::~CUILevelGuide()
{
}

void CUILevelGuide::Release()
{
	CN3UIBase::Release();

	m_pEdit_Level  = nullptr;
	m_pText_Page   = nullptr;
	m_pBtn_Check   = nullptr;
	m_pText_Level  = nullptr;
	m_pBtn_Up      = nullptr;
	m_pBtn_Down    = nullptr;
	m_pBtn_Cancel  = nullptr;

	m_iSearchLevel = 0;
	m_iPageNo      = 0;

	for (int i = 0; i < MAX_QUESTS_PER_PAGE; i++)
	{
		m_pScroll_Guide[i] = nullptr;
		m_pText_Guide[i]   = nullptr;
		m_pText_Title[i]   = nullptr;
	}
}

bool CUILevelGuide::Load(File& file)
{
	if (!CN3UIBase::Load(file))
		return false;

	std::string szID;

	N3_VERIFY_UI_COMPONENT(m_pEdit_Level, GetChildByID<CN3UIEdit>("edit_level"));
	N3_VERIFY_UI_COMPONENT(m_pText_Page, GetChildByID<CN3UIString>("text_page"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Check, GetChildByID<CN3UIButton>("btn_check"));
	N3_VERIFY_UI_COMPONENT(m_pText_Level, GetChildByID<CN3UIString>("text_level"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Up, GetChildByID<CN3UIButton>("btn_up"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Down, GetChildByID<CN3UIButton>("btn_down"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Cancel, GetChildByID<CN3UIButton>("btn_cancel"));

	for (int i = 0; i < MAX_QUESTS_PER_PAGE; i++)
	{
		szID = "scroll_guide" + std::to_string(i);
		N3_VERIFY_UI_COMPONENT(m_pScroll_Guide[i], GetChildByID<CN3UIScrollBar>(szID));

		szID = "text_guide" + std::to_string(i);
		N3_VERIFY_UI_COMPONENT(m_pText_Guide[i], GetChildByID<CN3UIString>(szID));

		szID = "text_title" + std::to_string(i);
		N3_VERIFY_UI_COMPONENT(m_pText_Title[i], GetChildByID<CN3UIString>(szID));
	}

	return true;
}

void CUILevelGuide::SearchQuests()
{
	if (m_pEdit_Level == nullptr)
		return;

	const std::string& szSearchLevel = m_pEdit_Level->GetString();

	int iSearchLevel                 = std::atoi(szSearchLevel.c_str());
	if (iSearchLevel == 0)
		return;

	// NOTE: This officially only checks the one way.
	if ((CGameBase::s_pPlayer->m_InfoBase.iLevel + MAX_SEARCH_LEVEL_RANGE) < iSearchLevel)
	{
		std::string szMsg = fmt::format_text_resource(IDS_QUEST_SEARCH_LEVEL_ERROR, MAX_SEARCH_LEVEL_RANGE);
		CGameProcedure::MessageBoxPost(szMsg, "", MB_OK);

		iSearchLevel = CGameBase::s_pPlayer->m_InfoBase.iLevel + MAX_SEARCH_LEVEL_RANGE;
	}

	m_iSearchLevel = iSearchLevel;

	SetPageNo(0);

	m_pEdit_Level->SetString("");
	m_pEdit_Level->SetFocus();
}

void CUILevelGuide::SetPageNo(int iPageNo)
{
	int iSearchLevel = 0;

	// if user entered a search level we should use it, otherwise use the user's current level
	if (m_iSearchLevel <= 0)
		iSearchLevel = CGameBase::s_pPlayer->m_InfoBase.iLevel;
	else
		iSearchLevel = m_iSearchLevel;

	// officially shows searched level
	if (m_pText_Level != nullptr)
		m_pText_Level->SetStringAsInt(iSearchLevel);

	// entered level cannot be bigger than max, cannot be smaller than min level
	iSearchLevel = std::clamp(iSearchLevel, 1, MAX_LEVEL);

	// set focus to edit on open
	if (m_pEdit_Level != nullptr)
		m_pEdit_Level->SetFocus();

	e_Class_Represent eCR = CGameBase::GetRepresentClass(CGameBase::s_pPlayer->m_InfoBase.eClass);

	// Build list of eligible quests
	std::vector<const __TABLE_HELP*> eligibleQuests;

	auto& questDataMap = CGameBase::s_pTbl_Help.GetMap();
	for (const auto& [_, questData] : questDataMap)
	{
		if (iSearchLevel < questData.iMinLevel || iSearchLevel > questData.iMaxLevel)
			continue;

		if (questData.iReqClass == CLASS_REPRESENT_UNKNOWN || questData.iReqClass == eCR)
			eligibleQuests.push_back(&questData);
	}

	int iPageCount = (static_cast<int>(eligibleQuests.size()) + MAX_QUESTS_PER_PAGE - 1) / MAX_QUESTS_PER_PAGE;

	// If we're ahead, we should roll it back to the last visible page.
	if (iPageNo >= iPageCount)
		iPageNo = iPageCount - 1;

	if (iPageNo < 0)
		iPageNo = 0;

	m_iPageNo         = iPageNo;

	int iStartIndex   = iPageNo * MAX_QUESTS_PER_PAGE;
	int iVisibleIndex = 0;

	// Skip straight to the first eligible quest for this page.
	auto itr          = eligibleQuests.begin();
	std::advance(itr, iStartIndex);

	// Attempt to display all 3, assuming 3 are present.
	while (itr != eligibleQuests.end())
	{
		const __TABLE_HELP* pQuestData = *itr;

		if (m_pText_Title[iVisibleIndex] != nullptr)
			m_pText_Title[iVisibleIndex]->SetString(pQuestData->szQuestName);

		if (m_pText_Guide[iVisibleIndex] != nullptr)
			m_pText_Guide[iVisibleIndex]->SetString(pQuestData->szQuestDesc);

		if (++iVisibleIndex >= MAX_QUESTS_PER_PAGE)
			break;

		++itr;
	}

	// Reset remaining quests if there weren't enough quests.
	for (int i = iVisibleIndex; i < MAX_QUESTS_PER_PAGE; i++)
	{
		if (m_pText_Title[i] != nullptr)
			m_pText_Title[i]->SetString("");

		if (m_pText_Guide[i] != nullptr)
			m_pText_Guide[i]->SetString("");
	}

	if (m_pText_Page != nullptr)
		m_pText_Page->SetStringAsInt(m_iPageNo + 1);
}

void CUILevelGuide::SetTopLine(CN3UIScrollBar* pScroll, CN3UIString* pTextGuide)
{
	if (pTextGuide == nullptr || pScroll == nullptr)
		return;

	// total number of lines of text
	const int iTotalLineCount   = pTextGuide->GetLineCount();

	// max number of lines visible in text area
	const int iVisibleLineCount = 4;

	// return if text is shorter than or equal to 4 lines
	if (iTotalLineCount <= iVisibleLineCount)
		return;

	// limit check for the line which displayed first, topline
	int iTopLine = std::clamp(pScroll->GetCurrentPos(), 0, iTotalLineCount - iVisibleLineCount);
	pTextGuide->SetStartLine(iTopLine);
}

void CUILevelGuide::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);

	if (bVisible)
	{
		SetPageNo(0);
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	}
	else
	{
		if (m_pEdit_Level != nullptr && m_pEdit_Level->HaveFocus())
			m_pEdit_Level->KillFocus();

		CGameProcedure::s_pUIMgr->ReFocusUI();
	}
}

bool CUILevelGuide::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtn_Cancel)
		{
			SetVisible(false);
			return true;
		}
		else if (pSender == m_pBtn_Up)
		{
			SetPageNo(m_iPageNo + 1);
			return true;
		}
		else if (pSender == m_pBtn_Down)
		{
			SetPageNo(m_iPageNo - 1);
			return true;
		}
		else if (pSender == m_pBtn_Check)
		{
			SearchQuests();
			return true;
		}
	}
	else if (dwMsg == UIMSG_SCROLLBAR_POS)
	{
		for (int i = 0; i < MAX_QUESTS_PER_PAGE; i++)
		{
			if (pSender != m_pScroll_Guide[i])
				continue;

			if (m_pText_Guide[i] != nullptr)
				SetTopLine(m_pScroll_Guide[i], m_pText_Guide[i]);

			break;
		}

		return true;
	}

	return false;
}

bool CUILevelGuide::OnKeyPress(int iKey)
{
	if (iKey == DIK_ESCAPE)
	{
		SetVisible(false);
		return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

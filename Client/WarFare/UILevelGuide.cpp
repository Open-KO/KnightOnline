// UILevelGuide.cpp: implementation of the CUILevelGuide class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UILevelGuide.h"
#include "GameDef.h"
#include "LocalInput.h"
#include "GameProcMain.h"
#include "APISocket.h"
#include "PacketDef.h"
#include "PlayerMySelf.h"
#include "MagicSkillMng.h"
#include "UIManager.h"

#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIEdit.h>
#include <N3Base/N3UIImage.h>
#include <N3Base/N3UIProgress.h>
#include <N3Base/N3UIScrollBar.h>
#include <N3Base/N3UIString.h>

#include <algorithm>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CUILevelGuide::CUILevelGuide()
{
	m_pEdit_Level			= nullptr;
	m_pText_Page			= nullptr;
	m_pBtn_Check			= nullptr;
	m_pText_Level			= nullptr;
	m_pBtn_Up				= nullptr;
	m_pBtn_Down				= nullptr;
	m_pBtn_Cancel			= nullptr;

	m_iSearchLevel			= 0;
	m_iCurrentPage			= 1;

	for (int i = 0; i < VISIBLE_ENTRY_COUNT; i++)
	{
		m_pScroll_Guide[i]	= nullptr;
		m_pText_Guide[i]	= nullptr;
		m_pText_Title[i]	= nullptr;
	}
}

CUILevelGuide::~CUILevelGuide()
{
}

void CUILevelGuide::Release()
{
	CN3UIBase::Release();

	m_pEdit_Level			= nullptr;
	m_pText_Page			= nullptr;
	m_pBtn_Check			= nullptr;
	m_pText_Level			= nullptr;
	m_pBtn_Up				= nullptr;
	m_pBtn_Down				= nullptr;
	m_pBtn_Cancel			= nullptr;

	m_iSearchLevel			= 0;
	m_iCurrentPage			= 1;

	for (int i = 0; i < VISIBLE_ENTRY_COUNT; i++)
	{
		m_pScroll_Guide[i]	= nullptr;
		m_pText_Guide[i]	= nullptr;
		m_pText_Title[i]	= nullptr;
	}

	for (int i = 0; i < MAX_LIST_SIZE; i++)
	{
		m_saQuestTitle[i].clear();
		m_saQuestText[i].clear();
	}
}

bool CUILevelGuide::Load(HANDLE hFile)
{
	if (!CN3UIBase::Load(hFile)) 
		return false;

	std::string szID;

	N3_VERIFY_UI_COMPONENT(m_pEdit_Level,			(CN3UIEdit*)   GetChildByID("edit_level"));
	N3_VERIFY_UI_COMPONENT(m_pText_Page,			(CN3UIString*) GetChildByID("text_page"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Check,			(CN3UIButton*) GetChildByID("btn_check"));
	N3_VERIFY_UI_COMPONENT(m_pText_Level,			(CN3UIString*) GetChildByID("text_level"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Up,				(CN3UIButton*) GetChildByID("btn_up"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Down,				(CN3UIButton*) GetChildByID("btn_down"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Cancel,			(CN3UIButton*) GetChildByID("btn_cancel"));

	for (int i = 0; i < VISIBLE_ENTRY_COUNT; i++)
	{
		szID = "scroll_guide" + std::to_string(i);
		N3_VERIFY_UI_COMPONENT(m_pScroll_Guide[i],	(CN3UIScrollBar*) GetChildByID(szID));

		szID = "text_guide" + std::to_string(i);
		N3_VERIFY_UI_COMPONENT(m_pText_Guide[i],	(CN3UIString*) GetChildByID(szID));

		szID = "text_title" + std::to_string(i);
		N3_VERIFY_UI_COMPONENT(m_pText_Title[i],	(CN3UIString*) GetChildByID(szID));
	}

	return true;
}

void CUILevelGuide::LoadContent()
{
	int iSearchLevel;

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

	// search by starting 5 levels lower and end 5 levels higher than user level.
	int iLowerLevelLimit = std::clamp(iSearchLevel - SEARCH_RANGE, 1, MAX_LEVEL);
	int iUpperLevelLimit = std::clamp(iSearchLevel + SEARCH_RANGE, 1, MAX_LEVEL);

	int iCounter = 0;

	size_t TableSize = CGameBase::s_pTbl_QuestContent.GetSize();

	std::fill(std::begin(m_saQuestTitle), std::end(m_saQuestTitle), "");
	std::fill(std::begin(m_saQuestText), std::end(m_saQuestText), "");

	for (size_t i = 0; i < TableSize && iCounter < MAX_LIST_SIZE; i++)
	{
		__TABLE_QUEST_CONTENT* pQuestContent = CGameBase::s_pTbl_QuestContent.GetIndexedData(i);
		if (pQuestContent == nullptr)
			continue;

		int iQuestLevel = pQuestContent->iLevel;
		if (iQuestLevel < iLowerLevelLimit
			|| iQuestLevel > iUpperLevelLimit)
			continue;

		m_saQuestTitle[iCounter]	= pQuestContent->szTitle;
		m_saQuestText[iCounter]		= pQuestContent->szQuestText + "\n" + pQuestContent->szHintText;

		++iCounter;
	}

	m_iCurrentPage = std::clamp(m_iCurrentPage, 1, MAX_PAGE_COUNT);

	int iStartIndex = (m_iCurrentPage - 1) * VISIBLE_ENTRY_COUNT;

	for (int i = 0; i < VISIBLE_ENTRY_COUNT; i++)
	{
		// last page 2 empty items
		if (iStartIndex >= iCounter)
		{
			if (m_pText_Title[i] != nullptr)
				m_pText_Title[i]->SetString("");

			if (m_pText_Guide[i] != nullptr)
				m_pText_Guide[i]->SetString("");

			continue;
		}

		if (m_pText_Title[i] != nullptr)
			m_pText_Title[i]->SetString(m_saQuestTitle[iStartIndex]);

		if (m_pText_Guide[i] != nullptr)
			m_pText_Guide[i]->SetString(m_saQuestText[iStartIndex]);

		++iStartIndex;
	}

	if (m_pText_Page != nullptr)
		m_pText_Page->SetStringAsInt(m_iCurrentPage);
}

void CUILevelGuide::SetTopLine(CN3UIScrollBar* pScroll, CN3UIString* pTextGuide)
{
	if (pTextGuide == nullptr
		|| pScroll == nullptr)
		return;

	// total number of lines of text
	const int iTotalLineCount = pTextGuide->GetLineCount();

	// max number of lines visible in text area
	const int iVisibleLineCount = 4;

	// return if text is shorter than or equal to 4 lines
	if (iTotalLineCount <= iVisibleLineCount)
		return;

	// limit check for the line which displayed first, topline
	int iTopLine = std::clamp(
		static_cast<int>(pScroll->GetCurrentPos()),
		0,
		iTotalLineCount - iVisibleLineCount);

	pTextGuide->SetStartLine(iTopLine);
}

void CUILevelGuide::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);

	if (bVisible)
	{
		// display initial user info with respect to user level
		m_iCurrentPage = 1;

		LoadContent();
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	}
	else
	{
		CGameProcedure::s_pUIMgr->ReFocusUI();

		if (m_pEdit_Level != nullptr
			&& m_pEdit_Level->HaveFocus())
			m_pEdit_Level->KillFocus();
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
			m_iCurrentPage++;
			LoadContent();
			return true;
		}
		else if (pSender == m_pBtn_Down)
		{
			m_iCurrentPage--;
			LoadContent();
			return true;
		}
		// Search
		else if (pSender == m_pBtn_Check)
		{
			if (m_pEdit_Level != nullptr)
			{
				const std::string& szSearchLevel = m_pEdit_Level->GetString();

				// kill focus edit, after user press search button
				m_pEdit_Level->KillFocus();

				m_iSearchLevel = std::atoi(szSearchLevel.c_str());
				m_iCurrentPage = 1;
				LoadContent();
			}
			
			return true;
		}
	}
	else if (dwMsg == UIMSG_SCROLLBAR_POS)
	{
		for (int i = 0; i < VISIBLE_ENTRY_COUNT; i++)
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

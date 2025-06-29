// UILevelGuide.cpp: implementation of the CUILevelGuide class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GameDef.h"
#include "UILevelGuide.h"
#include "GameProcedure.h"
#include "LocalInput.h"

#include "N3UIProgress.h"
#include "N3UIString.h"
#include "N3UIImage.h"
#include "N3UIEdit.h"
#include "N3UIScrollBar.h"

#include "GameProcMain.h"
#include "APISocket.h"
#include "PacketDef.h"
#include "PlayerMySelf.h"
#include "MagicSkillMng.h"
#include "UIManager.h"

#include "N3Texture.h"
#include "N3UIButton.h"
#include "N3UIDBCLButton.h"
#include <algorithm>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CUILevelGuide::CUILevelGuide()
{
	m_pEditLevel = nullptr;
	m_pScrollGuide2 = nullptr;
	m_pScrollGuide1 = nullptr;
	m_pScrollGuide0 = nullptr;
	m_pTextPage = nullptr;
	m_pButtonCheck = nullptr;
	m_pTextGuide2 = nullptr;
	m_pTextGuide1 = nullptr;
	m_pTextGuide0 = nullptr;
	m_pTextTitle2 = nullptr;
	m_pTextTitle1 = nullptr;
	m_pTextTitle0 = nullptr;
	m_pTextLevel = nullptr;
	m_pButtonUp = nullptr;
	m_pButtonDown = nullptr;
	m_pButtonCancel = nullptr;

	m_iSearchLevel = 0;
	m_iCurrentPage = 1;

	for (int i = 0; i < 10; i++)
		m_saQuestTitle[i].clear();

	for (int i = 0; i < 10; i++)
		m_saQuestText[i].clear();

}

CUILevelGuide::~CUILevelGuide()
{
	Release();
}

void CUILevelGuide::Release()
{
	CN3UIBase::Release();

	m_pEditLevel = nullptr;
	m_pScrollGuide2 = nullptr;
	m_pScrollGuide1 = nullptr;
	m_pScrollGuide0 = nullptr;
	m_pTextPage = nullptr;
	m_pButtonCheck = nullptr;
	m_pTextGuide2 = nullptr;
	m_pTextGuide1 = nullptr;
	m_pTextGuide0 = nullptr;
	m_pTextTitle2 = nullptr;
	m_pTextTitle1 = nullptr;
	m_pTextTitle0 = nullptr;
	m_pTextLevel = nullptr;
	m_pButtonUp = nullptr;
	m_pButtonDown = nullptr;
	m_pButtonCancel = nullptr;

	m_iSearchLevel = 0;
	m_iCurrentPage = 1;

	for (int i = 0; i < 10; i++)
		m_saQuestTitle[i].clear();

	for (int i = 0; i < 10; i++)
		m_saQuestText[i].clear();

}

bool CUILevelGuide::Load(HANDLE hFile)
{
	if (!CN3UIBase::Load(hFile)) 
		return false;

	N3_VERIFY_UI_COMPONENT(m_pEditLevel, (CN3UIEdit*) GetChildByID("edit_level"));
	N3_VERIFY_UI_COMPONENT(m_pScrollGuide2, (CN3UIScrollBar*) GetChildByID("scroll_guide2"));
	N3_VERIFY_UI_COMPONENT(m_pScrollGuide1, (CN3UIScrollBar*) GetChildByID("scroll_guide1"));
	N3_VERIFY_UI_COMPONENT(m_pScrollGuide0, (CN3UIScrollBar*) GetChildByID("scroll_guide0"));
	N3_VERIFY_UI_COMPONENT(m_pTextPage, (CN3UIString*) GetChildByID("text_page"));
	N3_VERIFY_UI_COMPONENT(m_pButtonCheck, (CN3UIButton*) GetChildByID("btn_check"));
	N3_VERIFY_UI_COMPONENT(m_pTextGuide2, (CN3UIString*) GetChildByID("text_guide2"));
	N3_VERIFY_UI_COMPONENT(m_pTextGuide1, (CN3UIString*) GetChildByID("text_guide1"));
	N3_VERIFY_UI_COMPONENT(m_pTextGuide0, (CN3UIString*) GetChildByID("text_guide0"));
	N3_VERIFY_UI_COMPONENT(m_pTextTitle2, (CN3UIString*) GetChildByID("text_title2"));
	N3_VERIFY_UI_COMPONENT(m_pTextTitle1, (CN3UIString*) GetChildByID("text_title1"));
	N3_VERIFY_UI_COMPONENT(m_pTextTitle0, (CN3UIString*) GetChildByID("text_title0"));
	N3_VERIFY_UI_COMPONENT(m_pTextLevel, (CN3UIString*) GetChildByID("text_level"));
	N3_VERIFY_UI_COMPONENT(m_pButtonUp, (CN3UIButton*) GetChildByID("btn_up"));
	N3_VERIFY_UI_COMPONENT(m_pButtonDown, (CN3UIButton*) GetChildByID("btn_down"));
	N3_VERIFY_UI_COMPONENT(m_pButtonCancel, (CN3UIButton*) GetChildByID("btn_cancel"));

	return true;
}

void CUILevelGuide::LoadContent()
{

	int iSearchLevel, iUserLevel;

	//if user entered search level use it, else use user's current level
	if (m_iSearchLevel <= 0)
		iSearchLevel = CGameBase::s_pPlayer->m_InfoBase.iLevel;
	else
		iSearchLevel = m_iSearchLevel;
	
	//officially shows searched level
	if(m_pTextLevel != nullptr)
	m_pTextLevel->SetStringAsInt(iSearchLevel);

	//entered level cannot be bigger than max, cannot be smaller than min level
	if (iSearchLevel < 1) iSearchLevel = 1;
	if (iSearchLevel > MAX_LEVEL) iSearchLevel = MAX_LEVEL;

	//set focus to edit on open
	if (m_pEditLevel != nullptr)
		m_pEditLevel->SetFocus();

	//search by starting 5 levels lower and end 5 levels higher than user level.
	int iLowerLevelLimit = iSearchLevel - SEARCH_RANGE;
	int iUpperLevelLimit = iSearchLevel + SEARCH_RANGE;

	if (iLowerLevelLimit <= 1) iLowerLevelLimit = 1;
	if (iUpperLevelLimit <= 1) iUpperLevelLimit = 1;

	if (iLowerLevelLimit >= MAX_LEVEL) iLowerLevelLimit = MAX_LEVEL;
	if (iUpperLevelLimit >= MAX_LEVEL) iUpperLevelLimit = MAX_LEVEL;

	int iCounter = 0;

	size_t TableSize = CGameBase::s_pTbl_QuestContent.GetSize();

	std::fill(std::begin(m_saQuestTitle), std::end(m_saQuestTitle), "");
	std::fill(std::begin(m_saQuestText), std::end(m_saQuestText), "");

	for (size_t i = 0; i < TableSize && iCounter < CONTENT_COUNT; i++)
	{
		__TABLE_QUEST_CONTENT* pQuestContent = CGameBase::s_pTbl_QuestContent.GetIndexedData(i);

		if (pQuestContent == nullptr)
			continue;

		int questLevel = pQuestContent->iLevel;

		if (questLevel < iLowerLevelLimit || questLevel > iUpperLevelLimit)
			continue;

		m_saQuestTitle[iCounter] = pQuestContent->szTitle;
		m_saQuestText[iCounter] = pQuestContent->szQuestText + "\n" + pQuestContent->szHintText;

		iCounter++;
	}

	CN3UIString* pTitles[3] = { m_pTextTitle0, m_pTextTitle1, m_pTextTitle2 };
	CN3UIString* pGuides[3] = { m_pTextGuide0, m_pTextGuide1, m_pTextGuide2 };

	//find max number of pages, ceil without cmath
	int iMaxPage = CONTENT_COUNT / 3 + (CONTENT_COUNT % 3 != 0 ? 1 : 0);

	if (m_iCurrentPage < 1) m_iCurrentPage = 1;

	if (m_iCurrentPage > iMaxPage) m_iCurrentPage = iMaxPage;

	int iStartIndex = (m_iCurrentPage - 1) * 3;

	for (int i = 0; i < 3; i++)
	{

		if (iStartIndex >= iCounter) //last page 2 empty items
		{
			pTitles[i]->SetString("");
			pGuides[i]->SetString("");
			continue;
		}

		if (pTitles[i])
			pTitles[i]->SetString(m_saQuestTitle[iStartIndex]);

		if (pGuides[i])
			pGuides[i]->SetString(m_saQuestText[iStartIndex]);



		iStartIndex++;
	}

	if (m_pTextPage != nullptr)
		m_pTextPage->SetStringAsInt(m_iCurrentPage);



}


void CUILevelGuide::SetTopLine(CN3UIScrollBar* pScroll, CN3UIString* pTextGuide)
{
	if (pTextGuide == nullptr || pScroll == nullptr) return;

	//total number of lines of text
	const int iTotalLineCount = pTextGuide->GetLineCount();

	//max number of lines visible in text area
	const int iVisibleLineCount = 4;

	//return if text is shorter than or equal to 4 lines
	if (iTotalLineCount <= iVisibleLineCount)
		return;

	//topline,size_t
	int iTopLine = static_cast<int>(pScroll->GetCurrentPos());

	// limit check for the line which displayed first, topline
	if (iTopLine < 0)
		iTopLine = 0;
	else if (iTopLine > iTotalLineCount - iVisibleLineCount)
		iTopLine = iTotalLineCount - iVisibleLineCount;


	pTextGuide->SetStartLine(iTopLine);

}

void CUILevelGuide::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);
	if (bVisible)
	{
		//display initial user info with respect to user level
		m_iCurrentPage = 1;
		LoadContent();
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	}
	else
	{
		CGameProcedure::s_pUIMgr->ReFocusUI();//this_ui
		if (m_pEditLevel && m_pEditLevel->HaveFocus())
			m_pEditLevel->KillFocus();
	}
}

bool CUILevelGuide::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pButtonCancel)
		{
			SetVisible(false);
			return true;
		}
		else if (pSender == m_pButtonUp)
		{
			m_iCurrentPage++;
			LoadContent();
			return true;
		}
		else if (pSender == m_pButtonDown)
		{
			m_iCurrentPage--;
			LoadContent();
			return true;
		}
		else if (pSender == m_pButtonCheck) //search button
		{

			std::string szSearchLevel;

			if (m_pEditLevel != nullptr)
			{
				szSearchLevel = m_pEditLevel->GetString();
				//kill focus edit, after user press search button
				m_pEditLevel->KillFocus();
			}

			
			m_iSearchLevel = std::atoi(szSearchLevel.c_str());
			m_iCurrentPage = 1;
			LoadContent();
			return true;
		}
	}
	else if (dwMsg == UIMSG_SCROLLBAR_POS)
	{
		int iCurLinePos = 1;
		CN3UIScrollBar* pScroll = nullptr;
		CN3UIString* pText = nullptr;

		if (pSender == m_pScrollGuide0)
		{
			pScroll = m_pScrollGuide0;
			pText = m_pTextGuide0;
		}
		else if (pSender == m_pScrollGuide1)
		{
			pScroll = m_pScrollGuide1;
			pText = m_pTextGuide1;
		}
		else if (pSender == m_pScrollGuide2)
		{
			pScroll = m_pScrollGuide2;
			pText = m_pTextGuide2;
		}

		if (pScroll != nullptr && pText != nullptr)
			SetTopLine(pScroll, pText);

		return true;
	}

	return false;
}

bool CUILevelGuide::OnKeyPress(int iKey)
{
	switch (iKey)
	{
		case DIK_ESCAPE:
			SetVisible(false);
			return true;
			/*// Not official behavior, focusing edit with tab key			
		case DIK_TAB:
			if (m_pEditLevel != nullptr)
			{
				m_pEditLevel->SetFocus();

				const std::string& strText = m_pEditLevel->GetString();

				size_t stlastPos = strText.length();

				//BUG: it sets position to end, but when user keypress it turns back to start
				m_pEditLevel->SetCaretPos(stlastPos);

			}
			return true;
			*/
			/* Not official behavior, searching with enter key
		case DIK_RETURN:
			if (m_pEditLevel != nullptr)
				m_iSearchLevel = std::atoi(m_pEditLevel->GetString().c_str());
			LoadContent();
			return true;
			*/
	}
	return CN3UIBase::OnKeyPress(iKey);
}

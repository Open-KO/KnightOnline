#include "stdafx.h"
#include "GameDef.h"
#include "GameBase.h"
#include "GameProcedure.h"
#include "UIQuestMenu.h"
#include "UIManager.h"
#include "PlayerOtherMgr.h"
#include "APISocket.h"

#include <N3Base/N3UIScrollBar.h>
#include <N3Base/N3UIString.h>
#include <N3Base/N3UIImage.h>
#include <N3Base/N3UIButton.h>
#include <N3Base/N3UITooltip.h>

#include <algorithm>

//-----------------------------------------------------------------------------
CUIQuestMenu::CUIQuestMenu(void) {
	m_iMenuCnt = 0;

	m_pTextTitle   = NULL;
	m_pTextSample  = NULL;

	m_pBtnClose    = NULL;
	m_pStrNpcName  = NULL;
	m_pScrollBar   = NULL;
	m_pBtnMenu     = NULL;
	m_pImageBtn    = NULL;
	m_pImageBottom = NULL;
	m_pImageMenu   = NULL;

	for(int i=0; i<MAX_STRING_MENU; ++i) {
		m_pTextMenu[i]      = NULL;
		m_pTextMenuImg[i]   = NULL;
		m_pTextMenuImgBk[i] = NULL;
		m_pTextMenuBtn[i]   = NULL;
	}
	
}

//-----------------------------------------------------------------------------
CUIQuestMenu::~CUIQuestMenu(void) {
	InitBase();

	if (m_pTextSample) delete m_pTextSample;
	if (m_pBtnMenu)    delete m_pBtnMenu;
	if (m_pImageBtn)   delete m_pImageBtn;
	if (m_pImageMenu)  delete m_pImageMenu;
}

//-----------------------------------------------------------------------------
void CUIQuestMenu::InitBase(void) {
	SetPos(0, 0);

	m_iMenuCnt = 0;

	for(int i=0; i<MAX_STRING_MENU; ++i) {
		if(m_pTextMenu[i]) delete m_pTextMenu[i];
		m_pTextMenu[i] = NULL;

		if (m_pTextMenuImg[i]) delete m_pTextMenuImg[i]; 
		m_pTextMenuImg[i] = NULL;

		if (m_pTextMenuImgBk[i]) delete m_pTextMenuImgBk[i];
		m_pTextMenuImgBk[i] = NULL;

		if (m_pTextMenuBtn[i]) delete m_pTextMenuBtn[i];
		m_pTextMenuBtn[i] = NULL;
	}
}

//-----------------------------------------------------------------------------
bool CUIQuestMenu::Load(HANDLE hFile)
{
	if(CN3UIBase::Load(hFile)==false) return false;

	m_pTextSample	= (CN3UIString*)(this->GetChildByID("Text_Menu"));
	__ASSERT(m_pTextSample, "NULL UI Component!!!");
	m_pTextTitle	= (CN3UIString*)(this->GetChildByID("Text_Title"));
	__ASSERT(m_pTextTitle, "NULL UI Component!!!");

	// NOTE: grabing the new stuff
	m_pBtnClose   = (CN3UIButton*)GetChildByID("btn_close");
	m_pStrNpcName = (CN3UIString*)GetChildByID("Text_Npcname");
	m_pScrollBar  = (CN3UIScrollBar*)GetChildByID("scroll");
	m_pBtnMenu    = (CN3UIButton*)GetChildByID("btn_menu");

	// the background image for the button
	m_pImageBtn    = (CN3UIImage*)GetChildByID("img_button_menu");
	// this is the bottom of the quest menu GUI
	m_pImageBottom = (CN3UIImage*)GetChildByID("img_Bottom");
	// this is the background image for the background image for the button
	m_pImageMenu   = (CN3UIImage*)GetChildByID("img_menu");

	// NOTE: some of these components are meant only to be copied
	RemoveChild(m_pTextSample);
	RemoveChild(m_pBtnMenu);
	RemoveChild(m_pImageBtn);
	RemoveChild(m_pImageMenu);

	return true;
}

//-----------------------------------------------------------------------------
bool CUIQuestMenu::ReceiveMessage(CN3UIBase *pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_STRING_LCLICK)
	{
		for (int i = 0;i < MAX_STRING_MENU; i++)
		{
			if (pSender == m_pTextMenu[i])
			{
				MsgSend_SelectMenu(i);
				SetVisible(false);
				return true;
			}
		}
	}
	else if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtnClose)
		{
			SetVisible(false);
			return true;
		}
	}
	else if (dwMsg == UIMSG_SCROLLBAR_POS)
	{
		if (pSender == m_pScrollBar)
		{
			UpdateTextForScroll();
			return true;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
void CUIQuestMenu::MsgSend_SelectMenu(uint8_t index)
{
	// TODO(srmeier): will have to look into whether bySelectedReward needs adding
	uint8_t byBuff[10];
	int iOffset=0;

	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_SELECT_MSG);
	CAPISocket::MP_AddByte(byBuff, iOffset, index);
	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
}

//-----------------------------------------------------------------------------
void CUIQuestMenu::Open(Packet& pkt)
{
	InitBase();

	if(m_pTextSample == NULL || m_pImageBtn == NULL || m_pImageMenu == NULL || m_pBtnMenu == NULL)	return;

	std::string szTitle;
	std::string szMenu[MAX_STRING_MENU];

	// NOTE: set the NPC name
	int iNpcID = pkt.read<int16_t>();

	CPlayerNPC* pNPC = CGameProcedure::s_pOPMgr->NPCGetByID(iNpcID, false);
	if (pNPC == NULL) return;

	m_pStrNpcName->SetString(pNPC->IDString());

	// NOTE: get the quest's main text
	int index = pkt.read<uint32_t>();

	__TABLE_QUEST_TALK* pTbl_Quest_Talk = CGameBase::s_pTbl_QuestTalk.Find(index);
	if(pTbl_Quest_Talk == NULL) return;

	szTitle = pTbl_Quest_Talk->szTalk;
	CGameBase::ConvertPipesToNewlines(szTitle);
	m_pTextTitle->SetString(szTitle);

	m_iMenuCnt = 0;

	for(int j=0;j<MAX_STRING_MENU;j++)
	{
		int iMenu = pkt.read<uint32_t>();
		if( iMenu >= 0 )
		{
			__TABLE_QUEST_MENU* pTbl_Quest_Menu = CGameBase::s_pTbl_QuestMenu.Find(iMenu);
			if(pTbl_Quest_Menu)
			{
				szMenu[j] = pTbl_Quest_Menu->szMenu;
			}

			m_iMenuCnt++;
		}
	}

	if(m_iMenuCnt==0) return;

	//set initial position of scroll bar as start
	if (m_pScrollBar != nullptr)
		m_pScrollBar->SetCurrentPos(0);

	SetVisible(true);

	int iIH;
	RECT rcImage, rcText;

	rcImage = m_pImageMenu->GetRegion();
	iIH = rcImage.bottom - rcImage.top;

	int i = 0;
	for(; i<m_iMenuCnt; i++)
	{
		m_pTextMenu[i] = new CN3UIString;
		__ASSERT(m_pTextMenu[i], "NULL UI Component!!!");

		if(m_pTextMenu[i]) {
			*(m_pTextMenu[i]) = *m_pTextSample;

			m_pTextMenu[i]->SetString(szMenu[i]);
			m_pTextMenu[i]->MoveOffset(0, i*iIH);
		}

		// NOTE: adding new stuff
		m_pTextMenuImg[i] = new CN3UIImage;
		m_pTextMenuImgBk[i] = new CN3UIImage;
		m_pTextMenuBtn[i] = new CN3UIButton;

		*(m_pTextMenuImg[i]) = *m_pImageBtn;
		*(m_pTextMenuImgBk[i]) = *m_pImageMenu;
		*(m_pTextMenuBtn[i]) = *m_pBtnMenu;

		m_pTextMenuImg[i]->MoveOffset(0, i*iIH);
		m_pTextMenuImgBk[i]->MoveOffset(0, i*iIH);
		m_pTextMenuBtn[i]->MoveOffset(0, i*iIH);

		m_pTextMenuImgBk[i]->SetParent(this);
		m_pTextMenuImg[i]->SetParent(this);
		m_pTextMenuBtn[i]->SetParent(this);
		m_pTextMenu[i]->SetParent(this);
	}

	rcText = m_pTextMenuImgBk[m_iMenuCnt-1]->GetRegion();
	m_pImageBottom->SetPos(0, rcText.bottom);

	const int iWidth = CN3Base::s_CameraData.vp.Width;
	const int iHeight = CN3Base::s_CameraData.vp.Height;
	int iX = 0, iY = 0;

	rcText = GetRegion();
	rcText.bottom = m_pImageBottom->GetRegion().bottom;
	SetRegion(rcText);

	iX = (iWidth - (rcText.right - rcText.left))/2;
	iY = (iHeight - (rcText.bottom - rcText.top))/2;
	SetPos(iX, iY);
}

//-----------------------------------------------------------------------------
bool CUIQuestMenu::OnKeyPress(int iKey)
{
	switch(iKey)
	{
	case DIK_ESCAPE:
		SetVisible(false);
		return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

//-----------------------------------------------------------------------------
void CUIQuestMenu::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);
	if(bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
		CGameProcedure::s_pUIMgr->ReFocusUI();//this_ui
}

void CUIQuestMenu::UpdateTextForScroll()
{
	if (m_pTextTitle == nullptr
		|| m_pScrollBar == nullptr)
		return;

	// scrollbar's current position
	const int iScrollPosition = m_pScrollBar->GetCurrentPos();

	// total number of lines of text
	const int iTotalLineCount = m_pTextTitle->GetLineCount();

	// max number of lines visible in text area
	const int iVisibleLineCount = 8;

	const int iMaxScrollableLines = iTotalLineCount - iVisibleLineCount;
	m_pScrollBar->SetRangeMax(iMaxScrollableLines);

	// return if text is shorter than or equal to the visible line count
	if (iTotalLineCount <= iVisibleLineCount)
		return;

	// limit check for the line which displayed first, topline
	int iTopLine = std::clamp(
		iScrollPosition,
		0,
		iTotalLineCount - iVisibleLineCount);

	m_pTextTitle->SetStartLine(iTopLine);
}

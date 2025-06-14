#include "stdafx.h"

#if !defined(LOGIN_SCENE_VERSION) || LOGIN_SCENE_VERSION == 1298
#include "resource.h"
#include "UILogIn_1298.h"
#include "GameProcLogIn_1298.h"
#include "N3UIEdit.h"
#include "N3UIButton.h"
#include "N3UIString.h"
#include "UIMessageBoxManager.h"
#include <algorithm>
#include <shellapi.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUILogIn_1298::CUILogIn_1298()
{
	m_pEdit_id = nullptr;
	m_pEdit_pw = nullptr;

	m_pBtn_LogIn = nullptr;
	m_pBtn_Connect = nullptr;
	m_pBtn_Cancel = nullptr;
	m_pBtn_Option = nullptr;
	m_pBtn_Join = nullptr;
	
	m_pGroup_Notice_1 = nullptr;
	m_pGroup_Notice_2 = nullptr;
	m_pGroup_Notice_3 = nullptr;

	m_pText_Notice1_Name_1 = nullptr;
	m_pText_Notice1_Text_1 = nullptr;

	m_pText_Notice2_Name_1 = nullptr;
	m_pText_Notice2_Text_1 = nullptr;
	m_pText_Notice2_Name_2 = nullptr;
	m_pText_Notice2_Text_2 = nullptr;

	m_pText_Notice3_Name_1 = nullptr;
	m_pText_Notice3_Text_1 = nullptr;
	m_pText_Notice3_Name_2 = nullptr;
	m_pText_Notice3_Text_2 = nullptr;
	m_pText_Notice3_Name_3 = nullptr;
	m_pText_Notice3_Text_3 = nullptr;

	m_pBtn_NoticeOK_1 = nullptr;
	m_pBtn_NoticeOK_2 = nullptr;
	m_pBtn_NoticeOK_3 = nullptr;

	m_pGroup_ServerList = nullptr;
	m_pGroup_LogIn = nullptr;

	m_pStr_Premium = nullptr;

	m_iSelectedServerIndex = -1;

	for (size_t i = 0; i < MAX_SERVERS; i++)
	{
		m_pServer_Group[i] = nullptr;
		m_pArrow_Group[i] = nullptr;
		m_pList_Group[i] = nullptr;
	} 

	m_bOpenningNow = false; // 위에서 아래로 스르륵...열려야 한다면..
	m_bIsNewsVisible = false; 
	m_fMoveDelta = 0;

	m_bLogIn = false;
}

CUILogIn_1298::~CUILogIn_1298()
{
}

bool CUILogIn_1298::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (pSender == nullptr)
		return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtn_LogIn
			&& m_pEdit_id != nullptr
			&& m_pEdit_pw != nullptr)
		{
			CGameProcedure::s_pProcLogIn->MsgSend_AccountLogIn(LIC_KNIGHTONLINE);
			return true;
		}
		else if (pSender == m_pBtn_Connect)
		{
			CGameProcedure::s_pProcLogIn->ConnectToGameServer(); // 고른 게임 서버에 접속
			return true;
		}
		else if (pSender == m_pBtn_Cancel)
		{
			PostQuitMessage(0);	// 종료...
			return true;
		}
		else if (pSender == m_pBtn_Option) // 옵션..
		{
			std::string szMsg;
			CGameBase::GetText(IDS_CONFIRM_EXECUTE_OPTION, &szMsg);
			CGameProcedure::MessageBoxPost(szMsg, "", MB_YESNO, BEHAVIOR_EXECUTE_OPTION);
			return true;
		}
		else if (pSender == m_pBtn_Join)
		{
			if (!CGameProcedure::s_pProcLogIn->m_szRegistrationSite.empty())
			{
				ShellExecute(nullptr, "open", CGameProcedure::s_pProcLogIn->m_szRegistrationSite.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
			}

			return true;
		}
		else if (pSender == m_pBtn_NoticeOK_1
			|| pSender == m_pBtn_NoticeOK_2
			|| pSender == m_pBtn_NoticeOK_3)
		{
			OpenServerList();
			return true;
		}
	}
	// double click on string
	else if (UIMSG_STRING_LDCLICK == dwMsg)
	{
		for (int i = 0; i < MAX_SERVERS; i++)
		{
			if (m_pServer_Group[i] == nullptr)
				continue;

			if (pSender == m_pList_Group[i])
			{
				m_iSelectedServerIndex = i;
				CGameProcedure::s_pProcLogIn->ConnectToGameServer();
				return true;
			}
		}
	}
	// change color on left click
	else if (UIMSG_STRING_LCLICK == dwMsg)
	{
		for (int i = 0; i < MAX_SERVERS; i++)
		{
			if (m_pServer_Group[i] != nullptr)
				m_pList_Group[i]->SetColor(D3DCOLOR_XRGB(255, 255, 255)); // white
		}

		for (int i = 0; i < MAX_SERVERS; i++)
		{
			if (m_pServer_Group[i] != nullptr
				&& pSender == m_pList_Group[i])
			{
				m_pList_Group[i]->SetColor(D3DCOLOR_XRGB(255, 255, 0)); // yellow
				m_iSelectedServerIndex = i;
				return true;
			}
		}
	}
	else if (dwMsg == UIMSG_EDIT_RETURN)
	{
		// TEMP(srmeier): there is a weird issue where the key inputs aren't going
		// through CGameProcedure::ProcessUIKeyInput() so CUILogIn_1298::OnKeyPress() isn't
		// being called...
		if (!m_bLogIn && m_pEdit_id && m_pEdit_pw)
		{
			CN3UIBase* pMsgBox = CGameProcedure::s_pMsgBoxMgr->GetFocusMsgBox();
			if (!(pMsgBox && pMsgBox->IsVisible()))
				CGameProcedure::s_pProcLogIn->MsgSend_AccountLogIn(LIC_KNIGHTONLINE);
		}
		else
		{
			return ReceiveMessage(m_pBtn_Connect, UIMSG_BUTTON_CLICK);
		}
	}

	return false;
}

bool CUILogIn_1298::Load(HANDLE hFile)
{
	if (!CN3UIBase::Load(hFile))
		return false;

	// fit to screen
	const int xOffset = static_cast<int>((s_CameraData.vp.Width - (m_rcRegion.right - m_rcRegion.left))/2);
	const int yOffset = static_cast<int>((s_CameraData.vp.Height - (m_rcRegion.bottom - m_rcRegion.top))/2);
	SetPos(xOffset, yOffset);
	m_rcRegion.left = 0;
	m_rcRegion.top = 0;
	m_rcRegion.right = static_cast<int>(s_CameraData.vp.Width);
	m_rcRegion.bottom = static_cast<int>(s_CameraData.vp.Height);
	
	N3_VERIFY_UI_COMPONENT(m_pGroup_LogIn, GetChildByID("Group_LogIn"));

	if (m_pGroup_LogIn != nullptr)
	{
		N3_VERIFY_UI_COMPONENT(m_pBtn_LogIn,		(CN3UIButton*) m_pGroup_LogIn->GetChildByID("btn_ok"));
		N3_VERIFY_UI_COMPONENT(m_pBtn_Cancel,		(CN3UIButton*) m_pGroup_LogIn->GetChildByID("btn_cancel"));
		N3_VERIFY_UI_COMPONENT(m_pBtn_Option,		(CN3UIButton*) m_pGroup_LogIn->GetChildByID("btn_option"));
		N3_VERIFY_UI_COMPONENT(m_pBtn_Join,			(CN3UIButton*) m_pGroup_LogIn->GetChildByID("btn_homepage"));

		N3_VERIFY_UI_COMPONENT(m_pEdit_id,			(CN3UIEdit*) m_pGroup_LogIn->GetChildByID("Edit_ID"));
		N3_VERIFY_UI_COMPONENT(m_pEdit_pw,			(CN3UIEdit*) m_pGroup_LogIn->GetChildByID("Edit_PW"));

		m_pGroup_LogIn->SetPosCenter();
		m_pGroup_LogIn->SetVisible(true);
	}

	// get notice boxes
	N3_VERIFY_UI_COMPONENT(m_pGroup_Notice_1,	GetChildByID("Group_Notice_1"));
	N3_VERIFY_UI_COMPONENT(m_pGroup_Notice_2,	GetChildByID("Group_Notice_2"));
	N3_VERIFY_UI_COMPONENT(m_pGroup_Notice_3,	GetChildByID("Group_Notice_3"));
	
	if (m_pGroup_Notice_1 != nullptr)
	{
		N3_VERIFY_UI_COMPONENT(m_pBtn_NoticeOK_1,		(CN3UIButton*) m_pGroup_Notice_1->GetChildByID("btn_ok"));
		N3_VERIFY_UI_COMPONENT(m_pText_Notice1_Name_1,	(CN3UIString*) m_pGroup_Notice_1->GetChildByID("text_notice_name_01"));
		N3_VERIFY_UI_COMPONENT(m_pText_Notice1_Text_1,	(CN3UIString*) m_pGroup_Notice_1->GetChildByID("text_notice_01"));

		m_pGroup_Notice_1->SetVisible(false);
	}

	if (m_pGroup_Notice_2 != nullptr)
	{
		N3_VERIFY_UI_COMPONENT(m_pBtn_NoticeOK_2,		(CN3UIButton*) m_pGroup_Notice_2->GetChildByID("btn_ok"));
		N3_VERIFY_UI_COMPONENT(m_pText_Notice2_Name_1,	(CN3UIString*) m_pGroup_Notice_2->GetChildByID("text_notice_name_01"));
		N3_VERIFY_UI_COMPONENT(m_pText_Notice2_Text_1,	(CN3UIString*) m_pGroup_Notice_2->GetChildByID("text_notice_01"));
		N3_VERIFY_UI_COMPONENT(m_pText_Notice2_Name_2,	(CN3UIString*) m_pGroup_Notice_2->GetChildByID("text_notice_name_02"));
		N3_VERIFY_UI_COMPONENT(m_pText_Notice2_Text_2,	(CN3UIString*) m_pGroup_Notice_2->GetChildByID("text_notice_02"));

		m_pGroup_Notice_2->SetVisible(false);
	}

	if (m_pGroup_Notice_3 != nullptr)
	{
		N3_VERIFY_UI_COMPONENT(m_pBtn_NoticeOK_3,		(CN3UIButton*) m_pGroup_Notice_3->GetChildByID("btn_ok"));
		N3_VERIFY_UI_COMPONENT(m_pText_Notice3_Name_1,	(CN3UIString*) m_pGroup_Notice_3->GetChildByID("text_notice_name_01"));
		N3_VERIFY_UI_COMPONENT(m_pText_Notice3_Text_1,	(CN3UIString*) m_pGroup_Notice_3->GetChildByID("text_notice_01"));
		N3_VERIFY_UI_COMPONENT(m_pText_Notice3_Name_2,	(CN3UIString*) m_pGroup_Notice_3->GetChildByID("text_notice_name_02"));
		N3_VERIFY_UI_COMPONENT(m_pText_Notice3_Text_2,	(CN3UIString*) m_pGroup_Notice_3->GetChildByID("text_notice_02"));
		N3_VERIFY_UI_COMPONENT(m_pText_Notice3_Name_3,	(CN3UIString*) m_pGroup_Notice_3->GetChildByID("text_notice_name_03"));
		N3_VERIFY_UI_COMPONENT(m_pText_Notice3_Text_3,	(CN3UIString*) m_pGroup_Notice_3->GetChildByID("text_notice_03"));

		m_pGroup_Notice_3->SetVisible(false);
	}

	N3_VERIFY_UI_COMPONENT(m_pStr_Premium,	(CN3UIString*) GetChildByID("premium"));

	if (m_pStr_Premium != nullptr)
		m_pStr_Premium->SetVisible(false);

	N3_VERIFY_UI_COMPONENT(m_pGroup_ServerList,	GetChildByID("Group_ServerList_01"));

	if (m_pGroup_ServerList != nullptr)
		m_pGroup_ServerList->SetVisible(false);

	// get List_Server (structure: Group_ServerList_01 -> server_20 -> List_Server )
	for (size_t i = 0; i < MAX_SERVERS; i++)
	{
		std::string szID = "server_" + std::to_string(i + 1);
		N3_VERIFY_UI_COMPONENT(m_pServer_Group[i], m_pGroup_ServerList->GetChildByID(szID));

		szID = "img_arrow" + std::to_string(i + 1);
		N3_VERIFY_UI_COMPONENT(m_pArrow_Group[i], m_pGroup_ServerList->GetChildByID(szID));
		
		N3_VERIFY_UI_COMPONENT(m_pList_Group[i], (CN3UIString*) m_pServer_Group[i]->GetChildByID("List_Server"));
	}

	N3_VERIFY_UI_COMPONENT(m_pBtn_Connect , (CN3UIButton* )m_pGroup_ServerList->GetChildByID("Btn_Connect"));
		
	return true;
}

void CUILogIn_1298::AccountIDGet(std::string& szID)
{
	if (m_pEdit_id != nullptr)
		szID = m_pEdit_id->GetString();
	else
		szID.clear();
}

void CUILogIn_1298::AccountPWGet(std::string& szPW)
{
	if (m_pEdit_pw != nullptr)
		szPW = m_pEdit_pw->GetString();
	else
		szPW.clear();
}

void CUILogIn_1298::ConnectButtonSetEnable(bool bEnable)
{
	eUI_STATE eState1 = (bEnable ? UI_STATE_BUTTON_NORMAL : UI_STATE_BUTTON_DISABLE);

	if (m_pBtn_Connect != nullptr)
		m_pBtn_Connect->SetState(eState1);
}

void CUILogIn_1298::FocusToID()
{
	if (m_pEdit_id != nullptr)
		m_pEdit_id->SetFocus();
}

void CUILogIn_1298::FocusCircular()
{
	if (m_pEdit_id == nullptr
		|| m_pEdit_pw == nullptr)
		return;

	if (m_pEdit_id->HaveFocus())
		m_pEdit_pw->SetFocus();
	else
		m_pEdit_id->SetFocus();
}

void CUILogIn_1298::InitEditControls()
{
	if (m_pEdit_id != nullptr)
	{
		m_pEdit_id->SetString("");
		m_pEdit_id->SetFocus();
	}

	if (m_pEdit_pw != nullptr)
		m_pEdit_pw->SetString("");
}

bool CUILogIn_1298::ServerInfoAdd(const __GameServerInfo& GSI)
{
	m_ListServerInfos.push_back(GSI);
	return true;
}

bool CUILogIn_1298::ServerInfoGet(int iIndex, __GameServerInfo& GSI)
{
	if (iIndex < 0
		|| iIndex >= (int) m_ListServerInfos.size())
		return false;

	GSI = m_ListServerInfos[iIndex];
	return true;
}

bool CUILogIn_1298::ServerInfoGetCur(__GameServerInfo& GSI)
{
	GSI.Init();

	return ServerInfoGet(m_iSelectedServerIndex, GSI);
}

void CUILogIn_1298::ServerInfoUpdate()
{
	
	if (!m_ListServerInfos.empty())
	{
		// sort(m_ListServerInfos.begin(), m_ListServerInfos.end(), not2(__GameServerInfo()));
		int iSize = m_ListServerInfos.size();

		// show ui of existing servers
		int iMaxPlayerCount = 3000;
		int iNumUserForLine = 3000 / 12;
		int iNumLines = 1;

		for (int i = 0; i < iSize; i++)
		{
			if (m_pServer_Group[i] == nullptr)
				continue;

			if (m_pList_Group[i] != nullptr)
				m_pList_Group[i]->SetString(m_ListServerInfos[i].szName);

			m_pServer_Group[i]->SetVisible(true); 

			if (m_pArrow_Group[i] != nullptr)
				m_pArrow_Group[i]->SetVisible(true);

			// hide number of lines with respect to user number
			iNumLines = m_ListServerInfos[i].iConcurrentUserCount / iNumUserForLine;

			if (iNumLines < 1) // minimum 1 lines
				iNumLines = 1;

			if (iNumLines > 12) // uif file has max 12 lines 
				iNumLines = 12;

			// ids of lines set as 1,2,3 ... 12 in .uif file
			for (int j = iNumLines + 1; j <= 12; j++)
			{
				// TODO: Pre-load this.
				std::string szID = std::to_string(j);

				CN3UIBase* pChild = m_pServer_Group[i]->GetChildByID(szID);
				if (pChild != nullptr)
					pChild->SetVisible(false);
			}
		}

		// hide ui of extra servers
		for (int i = iSize; i < MAX_SERVERS; i++)
		{
			if (m_pServer_Group[i] != nullptr)
				m_pServer_Group[i]->SetVisible(false);

			if (m_pArrow_Group[i] != nullptr)
				m_pArrow_Group[i]->SetVisible(false);
		}
	 
		// TODO: Show Premium info if user have premium
	}
}

void CUILogIn_1298::Tick()
{
	CN3UIBase::Tick();

	if (m_pGroup_ServerList != nullptr)
	{
		// slide effect opening, from top to bottom
		if (m_bOpenningNow)
		{
			POINT ptCur = m_pGroup_ServerList->GetPos();
			RECT rc = m_pGroup_ServerList->GetRegion();
			float fHeight = (float) (rc.bottom - rc.top);

			float fDelta = 5000.0f * CN3Base::s_fSecPerFrm;
			fDelta *= (fHeight - m_fMoveDelta) / fHeight;
			if (fDelta < 2.0f)
				fDelta = 2.0f;
			m_fMoveDelta += fDelta;

			int iYLimit = 0;
			ptCur.y = (int) (m_fMoveDelta - fHeight);
			if (ptCur.y >= iYLimit) // 다열렸다!!
			{
				ptCur.y = iYLimit;
				m_bOpenningNow = false;
			}

			m_pGroup_ServerList->SetPos(ptCur.x, ptCur.y);
		}
	}
}

void CUILogIn_1298::AddNews(const std::string& strNews)
{
	// TODO: needs improvement	

	std::string pieces[6];
	int count = 0;

	size_t BOX_START_LEN = 3; // '#', 0x00, '\n'
	size_t BOX_END_LEN = 5;   // '#', 0x00, '\n', 0x00, '\n'

	size_t searchPos = 0;
	while (count < 6)
	{
		size_t boxStart = strNews.find("#\0\n", searchPos);
		if (boxStart == std::string::npos)
			break;

		// title is inbetween searchPos and boxStart
		std::string title = strNews.substr(searchPos, boxStart - searchPos);

		size_t messageStart = boxStart + BOX_START_LEN;
		size_t boxEnd = strNews.find("#\0\n\0\n", messageStart);
		if (boxEnd == std::string::npos)
			break;

		std::string message = strNews.substr(messageStart, boxEnd - messageStart);

		pieces[count++] = title;
		pieces[count++] = message;

		// jump to next block
		searchPos = boxEnd + BOX_END_LEN;
	}

	int iReqNoticeBox = (count + 1) / 2;
	if (iReqNoticeBox <= 1)
	{
		if (m_pText_Notice1_Name_1 != nullptr)
			m_pText_Notice1_Name_1->SetString(pieces[0]);

		if (m_pText_Notice1_Text_1 != nullptr)
			m_pText_Notice1_Text_1->SetString(pieces[1]);

		if (m_pGroup_Notice_1 != nullptr)
			m_pGroup_Notice_1->SetVisible(true);
	}
	else if (iReqNoticeBox == 2)
	{
		if (m_pText_Notice2_Name_1 != nullptr)
			m_pText_Notice2_Name_1->SetString(pieces[0]);

		if (m_pText_Notice2_Text_1 != nullptr)
			m_pText_Notice2_Text_1->SetString(pieces[1]);

		if (m_pText_Notice2_Name_2 != nullptr)
			m_pText_Notice2_Name_2->SetString(pieces[2]);

		if (m_pText_Notice2_Text_2 != nullptr)
			m_pText_Notice2_Text_2->SetString(pieces[3]);

		if (m_pGroup_Notice_2 != nullptr)
			m_pGroup_Notice_2->SetVisible(true);
	}
	else if (iReqNoticeBox == 3)
	{
		if (m_pText_Notice3_Name_1 != nullptr)
			m_pText_Notice3_Name_1->SetString(pieces[0]);

		if (m_pText_Notice3_Text_1 != nullptr)
			m_pText_Notice3_Text_1->SetString(pieces[1]);

		if (m_pText_Notice3_Name_2 != nullptr)
			m_pText_Notice3_Name_2->SetString(pieces[2]);

		if (m_pText_Notice3_Text_2 != nullptr)
			m_pText_Notice3_Text_2->SetString(pieces[3]);

		if (m_pText_Notice3_Name_3 != nullptr)
			m_pText_Notice3_Name_3->SetString(pieces[4]);

		if (m_pText_Notice3_Text_3 != nullptr)
			m_pText_Notice3_Text_3->SetString(pieces[5]);

		if (m_pGroup_Notice_3 != nullptr)
			m_pGroup_Notice_3->SetVisible(true);
	}
	// No news, skip to server list
	else
	{
		m_bIsNewsVisible = false;
		OpenServerList();
	}
}

void CUILogIn_1298::OpenNews()
{
	if (m_bIsNewsVisible)
		return;

	if (m_pGroup_Notice_1 == nullptr
		|| m_pGroup_Notice_2 == nullptr
		|| m_pGroup_Notice_3 == nullptr)
		return;

	// set position of notice boxes
	// if it is done in Load function, they are not centered.
	RECT rc = m_pGroup_Notice_1->GetRegion();
	int iX = ((int) s_CameraData.vp.Width - (rc.right - rc.left)) / 2;
	int iY = ((int) s_CameraData.vp.Height - (rc.bottom - rc.top)) / 2;
	m_pGroup_Notice_1->SetPos(iX, iY);

	rc = m_pGroup_Notice_2->GetRegion();
	iX = ((int) s_CameraData.vp.Width - (rc.right - rc.left)) / 2;
	iY = ((int) s_CameraData.vp.Height - (rc.bottom - rc.top)) / 2;
	m_pGroup_Notice_2->SetPos(iX, iY);

	rc = m_pGroup_Notice_3->GetRegion();
	iX = ((int) s_CameraData.vp.Width - (rc.right - rc.left)) / 2;
	iY = ((int) s_CameraData.vp.Height - (rc.bottom - rc.top)) / 2;
	m_pGroup_Notice_3->SetPos(iX, iY);

	m_bIsNewsVisible = true;
}

void CUILogIn_1298::OpenServerList()
{
	if (m_bOpenningNow
		|| m_pGroup_ServerList == nullptr)
		return;

	// close all notice boxes
	if (m_pGroup_Notice_1 != nullptr)
		m_pGroup_Notice_1->SetVisible(false);

	if (m_pGroup_Notice_2 != nullptr)
		m_pGroup_Notice_2->SetVisible(false);

	if (m_pGroup_Notice_3 != nullptr)
		m_pGroup_Notice_3->SetVisible(false);

	// 스르륵 열린다!! = open without sound
	if (m_pGroup_ServerList != nullptr)
	{
		m_pGroup_ServerList->SetVisible(true);
		RECT rc = m_pGroup_ServerList->GetRegion();
		int iX = ((int) s_CameraData.vp.Width - (rc.right - rc.left)) / 2;
		int iY = ((int) s_CameraData.vp.Height - (rc.bottom - rc.top)) / 2;
		m_pGroup_ServerList->SetPos(iX, iY);
	}
	
	if (m_pStr_Premium != nullptr)
		m_pStr_Premium->SetVisible(true);

	m_fMoveDelta = 0;
	m_bOpenningNow = true;
	m_bIsNewsVisible = false;
}

void CUILogIn_1298::SetVisibleLogInUIs(bool bEnable)
{
	if (m_pGroup_LogIn != nullptr)
		m_pGroup_LogIn->SetVisible(bEnable); // 로그인을 숨긴다..
}

bool CUILogIn_1298::OnKeyPress(int iKey)
{
	if (!m_bLogIn)
	{
		switch (iKey)
		{
		case DIK_TAB:
			FocusCircular();
			return true;
		// case DIK_NUMPADENTER:
		// case DIK_RETURN:
		//	CGameProcedure::s_pProcLogIn->MsgSend_AccountLogIn(LIC_KNIGHTONLINE);
		//	return true;
		}
	}
	else if (m_pGroup_ServerList != nullptr
		&& m_pGroup_ServerList->IsVisible())
	{
		switch (iKey)
		{
			case DIK_UP:
			{
				int iServerCount = static_cast<int>(m_ListServerInfos.size());
				if (iServerCount == 0)
					return false;

				--m_iSelectedServerIndex;
				if (m_iSelectedServerIndex < 0)
					m_iSelectedServerIndex = iServerCount - 1;

				ReceiveMessage(m_pList_Group[m_iSelectedServerIndex], UIMSG_STRING_LCLICK);
			}
			return true;

			case DIK_DOWN:
			{
				int iServerCount = static_cast<int>(m_ListServerInfos.size());
				if (iServerCount == 0)
					return false;

				++m_iSelectedServerIndex;
				if (m_iSelectedServerIndex >= iServerCount)
					m_iSelectedServerIndex = 0;

				ReceiveMessage(m_pList_Group[m_iSelectedServerIndex], UIMSG_STRING_LCLICK);
			}
			return true;

			case DIK_NUMPADENTER:
			case DIK_RETURN:
			{
				// select the first server if user presses enter at server select screen
				if (m_iSelectedServerIndex == -1)
				{
					m_iSelectedServerIndex = 0;
					ReceiveMessage(m_pList_Group[m_iSelectedServerIndex], UIMSG_STRING_LCLICK);
				}

			}
			return true;
		}
	}
	else if (m_bIsNewsVisible)
	{
		switch (iKey)
		{
			case DIK_RETURN:
				ReceiveMessage(m_pBtn_NoticeOK_1, UIMSG_BUTTON_CLICK);
				return true;
		}
	}

	return CN3UIBase::OnKeyPress(iKey);
}
#endif

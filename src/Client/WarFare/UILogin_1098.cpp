#include "StdAfx.h"

#if defined(LOGIN_SCENE_VERSION) && LOGIN_SCENE_VERSION == 1098
#include "UILogin_1098.h"
#include "GameProcLogIn_1098.h"
#include "UIMessageBoxManager.h"
#include "text_resources.h"

#include <N3Base/N3UIEdit.h>
#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIList.h>

#include <algorithm>
#include <shellapi.h>

CUILogIn_1098::CUILogIn_1098()
{
	m_pEdit_id          = nullptr;
	m_pEdit_pw          = nullptr;

	m_pBtn_LogIn        = nullptr;
	m_pBtn_Connect      = nullptr;
	m_pBtn_Cancel       = nullptr;
	m_pBtn_Option       = nullptr;
	m_pBtn_Join         = nullptr;

	m_pGroup_ServerList = nullptr;
	m_pGroup_LogIn      = nullptr;

	m_pText_Rights      = nullptr;
	m_pImg_MGameLogo    = nullptr;
	m_pImg_DaumLogo     = nullptr;
	m_pImg_GradeLogo    = nullptr;

	m_pList_Server      = nullptr;

	m_bOpenningNow      = false; // 위에서 아래로 스르륵...열려야 한다면..
	m_fMoveDelta        = 0;

	m_bLogIn            = false;
}

CUILogIn_1098::~CUILogIn_1098()
{
}

bool CUILogIn_1098::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (pSender == nullptr)
		return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtn_LogIn && m_pEdit_id != nullptr && m_pEdit_pw != nullptr)
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
			PostQuitMessage(0);            // 종료...
			return true;
		}
		else if (pSender == m_pBtn_Option) // 옵션..
		{
			std::string szMsg = fmt::format_text_resource(IDS_CONFIRM_EXECUTE_OPTION);
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
	}
	else if (UIMSG_LIST_DBLCLK == dwMsg)
	{
		CGameProcedure::s_pProcLogIn->ConnectToGameServer(); // 고른 게임 서버에 접속
		return true;
	}
	else if (dwMsg == UIMSG_EDIT_RETURN)
	{
		// TEMP(srmeier): there is a weird issue where the key inputs aren't going
		// through CGameProcedure::ProcessUIKeyInput() so CUILogIn_1098::OnKeyPress() isn't
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

bool CUILogIn_1098::Load(File& file)
{
	if (!CN3UIBase::Load(file))
		return false;

	N3_VERIFY_UI_COMPONENT(m_pGroup_LogIn, GetChildByID("Group_LogIn"));

	if (m_pGroup_LogIn != nullptr)
	{
		N3_VERIFY_UI_COMPONENT(m_pBtn_LogIn, m_pGroup_LogIn->GetChildByID<CN3UIButton>("Btn_Login"));
		N3_VERIFY_UI_COMPONENT(m_pBtn_Cancel, m_pGroup_LogIn->GetChildByID<CN3UIButton>("Btn_Cancel"));
		N3_VERIFY_UI_COMPONENT(m_pBtn_Option, m_pGroup_LogIn->GetChildByID<CN3UIButton>("Btn_Option"));
		N3_VERIFY_UI_COMPONENT(m_pBtn_Join, m_pGroup_LogIn->GetChildByID<CN3UIButton>("Btn_Join"));

		N3_VERIFY_UI_COMPONENT(m_pEdit_id, m_pGroup_LogIn->GetChildByID<CN3UIEdit>("Edit_ID"));
		N3_VERIFY_UI_COMPONENT(m_pEdit_pw, m_pGroup_LogIn->GetChildByID<CN3UIEdit>("Edit_PW"));

		// N3_VERIFY_UI_COMPONENT(m_pImg_GradeLogo,	m_pGroup_LogIn->GetChildByID("Img_Grade"));
	}

	N3_VERIFY_UI_COMPONENT(m_pText_Rights, GetChildByID("Text_Rights"));
	N3_VERIFY_UI_COMPONENT(m_pImg_MGameLogo, GetChildByID("Img_MGame"));
	N3_VERIFY_UI_COMPONENT(m_pImg_DaumLogo, GetChildByID("Img_Daum"));

	if (m_pText_Rights != nullptr)
		m_pText_Rights->SetVisible(false);

	if (m_pImg_MGameLogo != nullptr)
		m_pImg_MGameLogo->SetVisible(false);

	if (m_pImg_DaumLogo != nullptr)
		m_pImg_DaumLogo->SetVisible(false);

	const std::string szIDsToHide[] = { "Group_Notice_1", "Group_Notice_2", "Group_Notice_3", "premium" };

	for (const std::string& szID : szIDsToHide)
	{
		CN3UIBase* pChild = GetChildByID(szID);
		if (pChild != nullptr)
			pChild->SetVisible(false);
	}

	N3_VERIFY_UI_COMPONENT(m_pGroup_ServerList, GetChildByID("Group_ServerList"));
	if (m_pGroup_ServerList != nullptr)
	{
		N3_VERIFY_UI_COMPONENT(m_pList_Server, m_pGroup_ServerList->GetChildByID<CN3UIList>("List_Server"));
		N3_VERIFY_UI_COMPONENT(m_pBtn_Connect, m_pGroup_ServerList->GetChildByID<CN3UIButton>("Btn_Connect"));

		m_pGroup_ServerList->SetVisible(false);
	}

	return true;
}

void CUILogIn_1098::AccountIDGet(std::string& szID)
{
	if (m_pEdit_id != nullptr)
		szID = m_pEdit_id->GetString();
	else
		szID.clear();
}

void CUILogIn_1098::AccountPWGet(std::string& szPW)
{
	if (m_pEdit_pw != nullptr)
		szPW = m_pEdit_pw->GetString();
	else
		szPW.clear();
}

void CUILogIn_1098::ConnectButtonSetEnable(bool bEnable)
{
	eUI_STATE eState1 = (bEnable ? UI_STATE_BUTTON_NORMAL : UI_STATE_BUTTON_DISABLE);
	eUI_STATE eState2 = (bEnable ? UI_STATE_LIST_ENABLE : UI_STATE_LIST_DISABLE);

	if (m_pBtn_Connect != nullptr)
		m_pBtn_Connect->SetState(eState1);

	if (m_pList_Server != nullptr)
		m_pList_Server->SetState(eState2);
}

void CUILogIn_1098::FocusToID()
{
	if (m_pEdit_id != nullptr)
		m_pEdit_id->SetFocus();
}

void CUILogIn_1098::FocusCircular()
{
	if (m_pEdit_id == nullptr || m_pEdit_pw == nullptr)
		return;

	if (m_pEdit_id->HaveFocus())
		m_pEdit_pw->SetFocus();
	else
		m_pEdit_id->SetFocus();
}

void CUILogIn_1098::InitEditControls()
{
	if (m_pEdit_id != nullptr)
	{
		m_pEdit_id->SetString("");
		m_pEdit_id->SetFocus();
	}

	if (m_pEdit_pw != nullptr)
		m_pEdit_pw->SetString("");
}

bool CUILogIn_1098::ServerInfoAdd(const __GameServerInfo& GSI)
{
	m_ListServerInfos.push_back(GSI);
	return true;
}

bool CUILogIn_1098::ServerInfoGet(int iIndex, __GameServerInfo& GSI)
{
	if (m_pList_Server == nullptr || iIndex < 0 || iIndex >= static_cast<int>(m_ListServerInfos.size()))
		return false;

	GSI = m_ListServerInfos[iIndex];
	return true;
}

bool CUILogIn_1098::ServerInfoGetCur(__GameServerInfo& GSI)
{
	GSI.Init();

	if (m_pList_Server == nullptr)
		return false;

	int iIndex = m_pList_Server->GetCurSel();
	return ServerInfoGet(iIndex, GSI);
}

void CUILogIn_1098::ServerInfoUpdate()
{
	if (m_pList_Server == nullptr)
		return;

	m_pList_Server->ResetContent();
	if (!m_ListServerInfos.empty())
	{
		//sort(m_ListServerInfos.begin(), m_ListServerInfos.end(), not2(__GameServerInfo()));
		for (const __GameServerInfo& GSI : m_ListServerInfos)
			m_pList_Server->AddString(GSI.szName);
	}
}

void CUILogIn_1098::Tick()
{
	CN3UIBase::Tick();

	if (m_pGroup_ServerList != nullptr)
	{
		// 위에서 아래로 스르륵...열려야 한다면..
		if (m_bOpenningNow)
		{
			POINT ptCur    = m_pGroup_ServerList->GetPos();
			RECT rc        = m_pGroup_ServerList->GetRegion();
			float fHeight  = (float) (rc.bottom - rc.top);

			float fDelta   = 5000.0f * CN3Base::s_fSecPerFrm;
			fDelta        *= (fHeight - m_fMoveDelta) / fHeight;
			if (fDelta < 2.0f)
				fDelta = 2.0f;
			m_fMoveDelta += fDelta;

			int iYLimit   = 0;
			ptCur.y       = (int) (m_fMoveDelta - fHeight);
			if (ptCur.y >= iYLimit) // 다열렸다!!
			{
				ptCur.y        = iYLimit;
				m_bOpenningNow = false;
			}

			m_pGroup_ServerList->SetPos(ptCur.x, ptCur.y);
		}
	}
}

void CUILogIn_1098::OpenServerList()
{
	if (m_bOpenningNow || m_pGroup_ServerList == nullptr)
		return;

	// 스르륵 열린다!!
	m_pGroup_ServerList->SetVisible(true);
	RECT rc = m_pGroup_ServerList->GetRegion();
	m_pGroup_ServerList->SetPos(0, -(rc.bottom - rc.top));

	m_fMoveDelta   = 0;
	m_bOpenningNow = true;
}

void CUILogIn_1098::SetVisibleLogInUIs(bool bEnable)
{
	// 로그인을 숨긴다..
	if (m_pGroup_LogIn != nullptr)
		m_pGroup_LogIn->SetVisible(bEnable);

	// 로그인한 계정의 구분에 따라 UI 만지기...
	if (m_pText_Rights != nullptr)
		m_pText_Rights->SetVisible(false);

	if (m_pImg_MGameLogo != nullptr)
		m_pImg_MGameLogo->SetVisible(false);

	if (m_pImg_DaumLogo != nullptr)
		m_pImg_DaumLogo->SetVisible(false);

	if (!bEnable)
	{
		if (LIC_MGAME == CGameProcedure::s_eLogInClassification)
		{
			if (m_pText_Rights != nullptr && m_pImg_MGameLogo != nullptr)
			{
				// 아래쪽 중단으로 맞춘다..
				RECT rcView = { 0, 0, (int) s_CameraData.vp.Width, (int) s_CameraData.vp.Height };
				int iX      = (rcView.right - (m_pText_Rights->GetWidth() + m_pImg_MGameLogo->GetWidth())) / 2;
				int iY      = rcView.bottom - m_pText_Rights->GetHeight() - 20;
				m_pText_Rights->SetPos(iX, iY);

				RECT rc = m_pText_Rights->GetRegion();
				m_pImg_MGameLogo->SetPos(rc.right, rcView.bottom - m_pImg_MGameLogo->GetHeight() - 20);

				m_pText_Rights->SetVisible(true);
				m_pImg_MGameLogo->SetVisible(true);
			}
		}
		else if (LIC_DAUM == CGameProcedure::s_eLogInClassification)
		{
			if (m_pText_Rights != nullptr && m_pImg_DaumLogo != nullptr)
			{
				// 아래쪽 중단으로 맞춘다..
				RECT rcView = { 0, 0, (int) s_CameraData.vp.Width, (int) s_CameraData.vp.Height };
				int iX      = (rcView.right - (m_pText_Rights->GetWidth() + m_pImg_DaumLogo->GetWidth())) / 2;
				int iY      = rcView.bottom - m_pText_Rights->GetHeight() - 20;
				m_pText_Rights->SetPos(iX, iY);

				RECT rc = m_pText_Rights->GetRegion();
				m_pImg_DaumLogo->SetPos(rc.right, rcView.bottom - m_pImg_DaumLogo->GetHeight() - 20);

				m_pText_Rights->SetVisible(true);
				m_pImg_DaumLogo->SetVisible(true);
			}
		}
		else
		{
			if (m_pText_Rights != nullptr)
			{
				RECT rcView = { 0, 0, (int) s_CameraData.vp.Width, (int) s_CameraData.vp.Height };
				int iX      = (rcView.right - m_pText_Rights->GetWidth()) / 2;
				int iY      = rcView.bottom - m_pText_Rights->GetHeight() - 20;
				m_pText_Rights->SetPos(iX, iY);

				m_pText_Rights->SetVisible(true);
			}
		}
	}
}

void CUILogIn_1098::RecalcGradePos()
{
	// 이용등급 표시
	if (m_pImg_GradeLogo != nullptr)
	{
		RECT rc = m_pImg_GradeLogo->GetRegion();
		int iX  = s_CameraData.vp.Width - (rc.right - rc.left + 10);
		int iY  = 10;
		m_pImg_GradeLogo->SetPos(iX, iY);
	}
}

bool CUILogIn_1098::OnKeyPress(int iKey)
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

			default:
				break;
		}
	}
	else if (!m_bOpenningNow && m_pGroup_ServerList != nullptr && m_pGroup_ServerList->IsVisible())
	{
		int iIndex = 0, iCnt = 0;

		switch (iKey)
		{
			case DIK_UP:
				if (m_pList_Server == nullptr)
					return false;

				iIndex = m_pList_Server->GetCurSel();
				if (iIndex > 0)
					m_pList_Server->SetCurSel(iIndex - 1);

				return true;

			case DIK_DOWN:
				if (m_pList_Server == nullptr)
					return false;

				iIndex = m_pList_Server->GetCurSel();
				iCnt   = m_pList_Server->GetCount();
				if ((iCnt - iIndex) > 1)
					m_pList_Server->SetCurSel(iIndex + 1);

				return true;

			case DIK_NUMPADENTER:
			case DIK_RETURN:
				ReceiveMessage(m_pBtn_Connect, UIMSG_BUTTON_CLICK);
				return true;

			default:
				break;
		}
	}

	return CN3UIBase::OnKeyPress(iKey);
}

#endif

// UICmdList.cpp: implementation of the CUICmdList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "text_resources.h"
#include "GameDef.h"
#include "UICmdList.h"
#include "GameProcedure.h"
#include "LocalInput.h"

#include "GameProcMain.h"
#include "APISocket.h"
#include "PacketDef.h"
#include "PlayerMySelf.h"
#include "UIManager.h"

#include "N3UIDBCLButton.h"

#include <N3Base/N3Texture.h>
#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIList.h>
#include <N3Base/N3UIImage.h>
#include <N3Base/N3UIProgress.h>
#include <N3Base/N3UIString.h>

#include <algorithm>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern std::string g_szCmdMsg[CMD_COUNT];

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUICmdList::CUICmdList()
{
	m_bOpenningNow = false; // 열리고 있다..
	m_bClosingNow = false;	// 닫히고 있다..
	m_fMoveDelta = 0.0f; // 부드럽게 열리고 닫히게 만들기 위해서 현재위치 계산에 부동소수점을 쓴다..
	m_pBtn_Cancel = nullptr;
	m_pList_CmdCat = nullptr;
	m_pList_Cmds = nullptr;
	m_pUICmdEdit = nullptr;
	m_iSelectedCategory = 0;
	m_eSelectedList = CMD_LIST_SEL_CATEGORY;
}

CUICmdList::~CUICmdList()
{
}

bool CUICmdList::Load(HANDLE hFile)
{
	if (!CN3UIBase::Load(hFile))
		return false;

	N3_VERIFY_UI_COMPONENT(m_pBtn_Cancel,	GetChildByID<CN3UIButton>("btn_cancel"));
	N3_VERIFY_UI_COMPONENT(m_pList_CmdCat,	GetChildByID<CN3UIList>("list_curtailment"));
	N3_VERIFY_UI_COMPONENT(m_pList_Cmds,	GetChildByID<CN3UIList>("list_content"));

	return true;
}

void CUICmdList::Release()
{
	m_bOpenningNow = false; 
	m_bClosingNow = false;	
	m_fMoveDelta = 0.0f;
	m_pBtn_Cancel = nullptr;
	m_pList_CmdCat = nullptr;
	m_pList_Cmds = nullptr;
	m_pUICmdEdit = nullptr;
	m_iSelectedCategory = 0;
	m_eSelectedList = CMD_LIST_SEL_CATEGORY;

	CN3UIBase::Release();
}

void CUICmdList::Render()
{
	if (!m_bVisible)
		return;

	CN3UIBase::Render();

	if (m_eSelectedList == CMD_LIST_SEL_CATEGORY)
		RenderSelectionBorder(m_pList_CmdCat);
	else if (m_eSelectedList == CMD_LIST_SEL_COMMAND)
		RenderSelectionBorder(m_pList_Cmds);
}

void CUICmdList::RenderSelectionBorder(CN3UIList* pListToRender)
{
	if (pListToRender == nullptr)
		return;

	RECT rcList = pListToRender->GetRegion();

	rcList.left -= 2;
	rcList.top -= 2;
	rcList.right += 2;
	rcList.bottom += 2;

	RenderLines(rcList, D3DCOLOR_XRGB(255, 255, 0)); // yellow
}

void CUICmdList::Tick()
{
	if (m_bOpenningNow) // 오른쪽에서 왼쪽으로 스르륵...열려야 한다면..
	{
		POINT ptCur = this->GetPos();
		RECT rc = this->GetRegion();
		float fWidth = (float)(rc.right - rc.left);

		float fDelta = 5000.0f * CN3Base::s_fSecPerFrm;
		fDelta *= (fWidth - m_fMoveDelta) / fWidth;
		if (fDelta < 2.0f) fDelta = 2.0f;
		m_fMoveDelta += fDelta;

		int iXLimit = CN3Base::s_CameraData.vp.Width - (int)fWidth;
		ptCur.x = CN3Base::s_CameraData.vp.Width - (int)m_fMoveDelta;
		if (ptCur.x <= iXLimit) // 다열렸다!!
		{
			ptCur.x = iXLimit;
			m_bOpenningNow = false;
		}

		this->SetPos(ptCur.x, ptCur.y);
	}
	else if (m_bClosingNow) // 오른쪽에서 왼쪽으로 스르륵...열려야 한다면..
	{
		POINT ptCur = this->GetPos();
		RECT rc = this->GetRegion();
		float fWidth = (float)(rc.right - rc.left);

		float fDelta = 5000.0f * CN3Base::s_fSecPerFrm;
		fDelta *= (fWidth - m_fMoveDelta) / fWidth;
		if (fDelta < 2.0f) fDelta = 2.0f;
		m_fMoveDelta += fDelta;

		int iXLimit = CN3Base::s_CameraData.vp.Width;
		ptCur.x = CN3Base::s_CameraData.vp.Width - (int)(fWidth - m_fMoveDelta);
		if (ptCur.x >= iXLimit) // 다 닫혔다..!!
		{
			ptCur.x = iXLimit;
			m_bClosingNow = false;

			this->SetVisibleWithNoSound(false, false, true); // 다 닫혔으니 눈에서 안보이게 한다.
		}

		this->SetPos(ptCur.x, ptCur.y);
	}

	CN3UIBase::Tick();
}

bool CUICmdList::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (pSender == nullptr)
		return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtn_Cancel)
		{
			Close();
			return true;
		}
	}
	else if (dwMsg == UIMSG_LIST_SELCHANGE)
	{
		if (pSender == m_pList_CmdCat)
		{
			m_iSelectedCategory = m_pList_CmdCat->GetCurSel();
			m_eSelectedList = CMD_LIST_SEL_CATEGORY;
			UpdateCommandList(m_iSelectedCategory);
			return true;
		}
		else if (pSender == m_pList_Cmds)
		{
			m_eSelectedList = CMD_LIST_SEL_COMMAND;
			return true;
		}
	}
	else if (dwMsg == UIMSG_LIST_DBLCLK)
	{
		if (pSender == m_pList_Cmds)
		{
			int iSel = m_pList_Cmds->GetCurSel();
			ExecuteCommand(iSel);
			return true;
		}
	}

	return false;
}

bool CUICmdList::OnMouseWheelEvent(short delta)
{
	if (delta > 0)
		OnKeyPress(DIK_UP);
	else
		OnKeyPress(DIK_DOWN);

	return true;
}

bool CUICmdList::OnKeyPress(int iKey)
{
	switch (iKey)
	{
		case DIK_ESCAPE:
			Close(); // close with animation
			return true;

		case DIK_RETURN:
			if (m_pList_Cmds != nullptr)
				ExecuteCommand(m_pList_Cmds->GetCurSel());
			return true;

		case DIK_DOWN:
			if (m_eSelectedList == CMD_LIST_SEL_CATEGORY)
			{
				int iSelectedIndex = m_pList_CmdCat->GetCurSel();
				int iMaxIndex = m_pList_CmdCat->GetCount() - 1;

				iSelectedIndex = std::clamp(iSelectedIndex + 1, 0, iMaxIndex);

				m_pList_CmdCat->SetCurSel(iSelectedIndex);
				UpdateCommandList(iSelectedIndex);
			}
			else if (m_eSelectedList == CMD_LIST_SEL_COMMAND)
			{
				int iSelectedIndex = m_pList_Cmds->GetCurSel();
				int iMaxIndex = m_pList_Cmds->GetCount() - 1;

				iSelectedIndex = std::clamp(iSelectedIndex + 1, 0, iMaxIndex);

				m_pList_Cmds->SetCurSel(iSelectedIndex);
			}
			return true;

		case DIK_UP:
			if (m_eSelectedList == CMD_LIST_SEL_CATEGORY)
			{
				int iSelectedIndex = m_pList_CmdCat->GetCurSel();
				int iMaxIndex = m_pList_CmdCat->GetCount() - 1;

				iSelectedIndex = std::clamp(iSelectedIndex - 1, 0, iMaxIndex);

				m_pList_CmdCat->SetCurSel(iSelectedIndex);
				UpdateCommandList(iSelectedIndex);
			}
			else if (m_eSelectedList == CMD_LIST_SEL_COMMAND)
			{
				int iSelectedIndex = m_pList_Cmds->GetCurSel();
				int iMaxIndex = m_pList_Cmds->GetCount() - 1;

				iSelectedIndex = std::clamp(iSelectedIndex - 1, 0, iMaxIndex);

				m_pList_Cmds->SetCurSel(iSelectedIndex);
			}
			return true;

		case DIK_TAB:
			if (m_eSelectedList == CMD_LIST_SEL_CATEGORY)
				m_eSelectedList = CMD_LIST_SEL_COMMAND;
			else
				m_eSelectedList = CMD_LIST_SEL_CATEGORY;
			return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

void CUICmdList::Open()
{
	// 스르륵 열린다!!
	SetVisible(true);
	SetPos(CN3Base::s_CameraData.vp.Width, 10);
	m_fMoveDelta = 0;
	m_bOpenningNow = true;
	m_bClosingNow = false;

	// Reset selected command to first in list
	if (m_pList_Cmds != nullptr)
		m_pList_Cmds->SetCurSel(0);
}

void CUICmdList::Close()
{
	RECT rc = GetRegion();
	SetPos(CN3Base::s_CameraData.vp.Width - (rc.right - rc.left), 10);
	m_fMoveDelta = 0;
	m_bOpenningNow = false;
	m_bClosingNow = true;
}

void CUICmdList::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);
	if (bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
		CGameProcedure::s_pUIMgr->ReFocusUI();//this_ui
}

bool CUICmdList::CreateCategoryList()
{
	if (m_pList_CmdCat == nullptr
		|| m_pList_Cmds == nullptr)
		return false;

	std::string szCategory, szTooltip;	

	for (int i = 0; i < CMD_LIST_CAT_COUNT; i++)
	{
		// category names start with 7800
		int iCategoryResourceID = IDS_PRIVATE_CMD_CAT + i;
		szCategory = fmt::format_text_resource(iCategoryResourceID);
		m_pList_CmdCat->AddString(szCategory);

		// category tips start with 7900
		szTooltip = fmt::format_text_resource(iCategoryResourceID + 100);

		CN3UIString* pChild = m_pList_CmdCat->GetChildStrFromList(szCategory);
		if (pChild != nullptr)
		{
			pChild->SetTooltipColor(D3DCOLOR_XRGB(144, 238, 144)); // green
			pChild->SetTooltipText(szTooltip);
		}
	}

	m_pList_CmdCat->SetFontColor(D3DCOLOR_XRGB(255, 255, 0)); // yellow

	struct CommandCategory
	{
		e_CmdListCategory eCategory;
		e_ChatCmd eBaseCmd;
		int iFirstResourceID;
		int iLastResourceID;
	};

	// Temporarily just map everything together so they can be used as-is.
	// The command index needs to be reworked to behave more like official, where it's handled within CUICmdList and categorised,
	// so that entire translation can be thrown away.
	constexpr CommandCategory commandCategories[] =
	{
		// Category					Base command index	First resource ID		Last resource ID
		{ CMD_LIST_CAT_PRIVATE,		CMD_WHISPER,		IDS_CMD_WHISPER,		IDS_CMD_INDIVIDUAL_BATTLE },
		{ CMD_LIST_CAT_TRADE,		CMD_TRADE,			IDS_CMD_TRADE,			IDS_CMD_MERCHANT },
		{ CMD_LIST_CAT_PARTY,		CMD_PARTY,			IDS_CMD_PARTY,			IDS_CMD_PERMITPARTY },
		{ CMD_LIST_CAT_CLAN,		CMD_JOINCLAN,		IDS_CMD_JOINCLAN,		IDS_CMD_CLAN_BATTLE },
		{ CMD_LIST_CAT_KNIGHTS,		CMD_CONFEDERACY,	IDS_CMD_CONFEDERACY,	IDS_CMD_DECLARATION },
		{ CMD_LIST_CAT_GUARDIAN,	CMD_HIDE,			IDS_CMD_HIDE,			IDS_CMD_DESTROY },
		{ CMD_LIST_CAT_KING,		CMD_ROYALORDER,		IDS_CMD_ROYALORDER,		IDS_CMD_REWARD },
	};

	m_categoryToCommandInfoMap.clear();

	for (const CommandCategory& commandCategory : commandCategories)
		AppendToCommandMap(commandCategory.eCategory, commandCategory.eBaseCmd, commandCategory.iFirstResourceID, commandCategory.iLastResourceID);

	// Unofficial. This isn't displayed officially.
	if (CGameBase::s_pPlayer->m_InfoBase.iAuthority == AUTHORITY_MANAGER)
		AppendToCommandMap(CMD_LIST_CAT_GM, CMD_VISIBLE, IDS_CMD_VISIBLE, IDS_CMD_PLC);

	UpdateCommandList(m_iSelectedCategory); // initialize a cmd list for viewing when opening cmd window

	return true;
}

bool CUICmdList::UpdateCommandList(int iCatIndex)
{
	if (iCatIndex < 0
		|| iCatIndex >= CMD_LIST_CAT_COUNT)
		return false;

	if (m_pList_Cmds == nullptr)
		return false;
	
	m_pList_Cmds->ResetContent();

	e_CmdListCategory eCategory = static_cast<e_CmdListCategory>(iCatIndex);

	const auto range = m_categoryToCommandInfoMap.equal_range(eCategory);
	for (const auto& [_, commandInfo] : std::ranges::subrange(range.first, range.second))
	{
		const std::string& commandName = g_szCmdMsg[commandInfo.Command];
		m_pList_Cmds->AddString(commandName);

		// fill with command name exp: /type %s, to /type ban_user
		std::string cmdTip = fmt::format_text_resource(commandInfo.ResourceID  + 100, commandName);

		CN3UIString* pChild = m_pList_Cmds->GetChildStrFromList(commandName);
		if (pChild != nullptr)
		{
			pChild->SetTooltipColor(D3DCOLOR_XRGB(144, 238, 144)); // green
			pChild->SetTooltipText(cmdTip);
		}
	}

	return true;
}

void CUICmdList::AppendToCommandMap(e_CmdListCategory eCategory, e_ChatCmd eBaseCmd, int iFirstResourceID, int iLastResourceID)
{
	for (int iResourceID = iFirstResourceID, iRealCmdIndex = eBaseCmd;
		iResourceID <= iLastResourceID;
		++iResourceID, ++iRealCmdIndex)
	{
		CommandInfo commandInfo;
		commandInfo.ResourceID	= iResourceID;
		commandInfo.Command		= static_cast<e_ChatCmd>(iRealCmdIndex);
		m_categoryToCommandInfoMap.insert(std::make_pair(eCategory, std::move(commandInfo)));
	}
}

bool CUICmdList::ExecuteCommand(int iCmdIndex)
{
	// Fetch base command index for this category.
	// TODO: This should all ultimately be thrown away; it's based on outdated logic where all
	// commands were thrown into the same array and assigned a relative ID.
	// Officially this got moved into CUICmdList and categorised, so this should be refactored
	// and thrown away.
	e_CmdListCategory eCategory = static_cast<e_CmdListCategory>(m_iSelectedCategory);

	// NOTE: The first command in the category is the base. It is guaranteed to be in insert order.
	auto itr = m_categoryToCommandInfoMap.find(eCategory);
	if (itr == m_categoryToCommandInfoMap.end())
		return false;

	std::string command;
	m_pList_Cmds->GetString(iCmdIndex, command);

	int iRealCmdIndex = itr->second.Command + iCmdIndex;
	if (iRealCmdIndex == CMD_WHISPER)
	{
		CGameProcedure::s_pProcMain->OpenCmdEdit(command);
		return true;
	}

	command = '/' + command;
	CGameProcedure::s_pProcMain->ParseChattingCommand(command);

	return true;
}

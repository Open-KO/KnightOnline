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

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUICmdList::CUICmdList()
{
	m_bOpenningNow = false; // 열리고 있다..
	m_bClosingNow = false;	// 닫히고 있다..
	m_fMoveDelta = 0.0f; // 부드럽게 열리고 닫히게 만들기 위해서 현재위치 계산에 부동소수점을 쓴다..
	m_pBtn_cancel = nullptr;
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

	N3_VERIFY_UI_COMPONENT(m_pBtn_cancel,	(CN3UIButton*) GetChildByID("btn_cancel"));
	N3_VERIFY_UI_COMPONENT(m_pList_CmdCat,	(CN3UIList*) GetChildByID("list_curtailment"));
	N3_VERIFY_UI_COMPONENT(m_pList_Cmds,	(CN3UIList*) GetChildByID("list_content"));

	CreateCategoryList();
	return true;
}

void CUICmdList::Release()
{
	m_bOpenningNow = false; 
	m_bClosingNow = false;	
	m_fMoveDelta = 0.0f;
	m_pBtn_cancel = nullptr;
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
		if (pSender == m_pBtn_cancel)
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
		szCategory = fmt::format_text_resource(i + 7800); // load command categories
		m_pList_CmdCat->AddString(szCategory);

		// category tips start with 7900
		szTooltip = fmt::format_text_resource(i + IDS_PRIVATE_CMD_CAT + 100);

		CN3UIString* pChild = m_pList_CmdCat->GetChildStrFromList(szCategory);
		if (pChild != nullptr)
		{
			pChild->SetTooltipColor(D3DCOLOR_XRGB(144, 238, 144)); // green
			pChild->SetTooltipText(szTooltip);
		}
	}

	m_pList_CmdCat->SetFontColor(D3DCOLOR_XRGB(255, 255, 0)); // yellow

	int idCur = 8000;   // Command list strings start at this index
	int idEnd = 9600;   // Command list strings end at this index

	std::string szCommand;
	// create map of commands
	for (int i = idCur; idCur < idEnd; idCur++, i++)
	{
		if (idCur == 9000)
		{
			i += 400; // offset and put gm cmds at end of map
		}
		else if (idCur == 9100)
		{
			i -= 500;
			idCur = 9200;
		}

		szCommand = fmt::format_text_resource(idCur);
		if (!szCommand.empty() && (i / 100) % 2 == 0)
			m_mapCmds[i] = szCommand;
	}

	UpdateCommandList(m_iSelectedCategory); // initialize a cmd list for viewing when opening cmd window

	return true;
}

bool CUICmdList::UpdateCommandList(int iCatIndex)
{
	if (m_iSelectedCategory < 0
		|| m_iSelectedCategory >= CMD_LIST_CAT_COUNT)
		return false;

	if (m_pList_Cmds == nullptr)
		return false;
	
	m_pList_Cmds->ResetContent();

	int indexStart = iCatIndex * 200 + 8000;	// start index for correct loc in map
	int indexEnd = indexStart + 100;			// where to stop iterating

	for (const auto& [resourceId, commandName] : m_mapCmds)
	{
		if (resourceId < indexStart
			|| resourceId >= indexEnd)
			continue;

		m_pList_Cmds->AddString(commandName);

		// fill with command name exp: /type %s, to /type ban_user
		std::string cmdTip = fmt::format_text_resource(resourceId + 100, commandName);

		CN3UIString* pChild = m_pList_Cmds->GetChildStrFromList(commandName);
		if (pChild != nullptr)
		{
			pChild->SetTooltipColor(D3DCOLOR_XRGB(144, 238, 144)); // green
			pChild->SetTooltipText(cmdTip);
		}
	}

	return true;
}

bool CUICmdList::ExecuteCommand(int iCmdIndex)
{
	std::string command;
	m_pList_Cmds->GetString(iCmdIndex, command);

	if (command == "PM")
		CGameProcedure::s_pProcMain->OpenCmdEdit(command);

	command = '/' + command;
	CGameProcedure::s_pProcMain->ParseChattingCommand(command);

	return true;

}

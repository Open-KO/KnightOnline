// UICmdList.cpp: implementation of the CUICmdList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "GameDef.h"
#include "UICmdList.h"
#include "GameProcedure.h"
#include "LocalInput.h"

#include "GameProcMain.h"
#include "APISocket.h"
#include "PacketDef.h"
#include "PlayerMySelf.h"
#include "UIManager.h"



#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUICmdList::CUICmdList()
{
	m_bOpenningNow = false; // It's opening...
	m_bClosingNow = false;	// It's closing...
	m_fMoveDelta = 0.0f;	// To make it open and close smoothly, floating-point numbers are used for the current position calculation.
	m_pBtn_cancel = nullptr;
	m_pList_CmdCat = nullptr;
	m_pList_Cmds = nullptr;
	m_pUICmdEdit = nullptr;
}

CUICmdList::~CUICmdList()
{
}

bool CUICmdList::Load(HANDLE hFile)
{
	if (CN3UIBase::Load(hFile) == false) return false;

	N3_VERIFY_UI_COMPONENT(m_pBtn_cancel, (CN3UIButton*) GetChildByID("btn_cancel"));
	N3_VERIFY_UI_COMPONENT(m_pList_CmdCat, (CN3UIList*) GetChildByID("list_curtailment"));
	N3_VERIFY_UI_COMPONENT(m_pList_Cmds, (CN3UIList*) GetChildByID("list_content"));

	CreateCategoryList();
	return true;
}

void CUICmdList::Release()
{
	if (m_bOpenningNow) // If it should smoothly slide open from right to left...
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
		if (ptCur.x <= iXLimit) // Fully opened!!
		{
			ptCur.x = iXLimit;
			m_bOpenningNow = false;
		}

		this->SetPos(ptCur.x, ptCur.y);
	}
	else if (m_bClosingNow) // If it needs to smoothly open from right to left...
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
		if (ptCur.x >= iXLimit) // Fully closed..!!
		{
			ptCur.x = iXLimit;
			m_bClosingNow = false;

			this->SetVisibleWithNoSound(false, false, true); // Since it's fully closed, make it invisible to the eye.
		}

		this->SetPos(ptCur.x, ptCur.y);
	}

	CN3UIBase::Tick();
	CN3UIBase::Release();

}

void CUICmdList::Render()
{
	if (!m_bVisible) return;

	CN3UIBase::Render();
}

void CUICmdList::Tick()
{
	if (m_bOpenningNow) // If it should smoothly slide open from right to left...
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
		if (ptCur.x <= iXLimit) // Fully opened!!
		{
			ptCur.x = iXLimit;
			m_bOpenningNow = false;
		}

		this->SetPos(ptCur.x, ptCur.y);
	}
	else if (m_bClosingNow) // If it needs to smoothly open from right to left...
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
		if (ptCur.x >= iXLimit) // Fully closed..!!
		{
			ptCur.x = iXLimit;
			m_bClosingNow = false;

			this->SetVisibleWithNoSound(false, false, true); // Since it's fully closed, make it invisible to the eye.
		}

		this->SetPos(ptCur.x, ptCur.y);
	}

	CN3UIBase::Tick();
}

bool CUICmdList::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (pSender == nullptr) return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtn_cancel) {
			SetVisible(false);
			return true;
		}
	}
	else if (dwMsg == UIMSG_LIST_SELCHANGE) {
		if (pSender == m_pList_CmdCat) {
			uint8_t iSel = m_pList_CmdCat->GetCurSel();
			UpdateCommandList(iSel);
			return true;
		}
	}
	else if (dwMsg == UIMSG_LIST_DBLCLK) {
		if (pSender == m_pList_Cmds) {
			uint8_t iSel = m_pList_Cmds->GetCurSel();
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
	{	// When the hotkey has focus, other UIs cannot be closed, so when DIK_ESCAPE is pressed, regain the focus.
		// Closes other open UIs.
		CGameProcedure::s_pUIMgr->ReFocusUI();//this_ui
		CN3UIBase* pFocus = CGameProcedure::s_pUIMgr->GetFocusedUI();
		if (pFocus && pFocus != this) pFocus->OnKeyPress(iKey);
	}
	return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

void CUICmdList::Open()
{
	// It opens smoothly!!
	SetVisible(true);
	this->SetPos(CN3Base::s_CameraData.vp.Width, 10);
	m_fMoveDelta = 0;
	m_bOpenningNow = true;
	m_bClosingNow = false;

	//m_iRBtnDownOffs = -1;
}


void CUICmdList::Close()
{
	//SetVisible(false); 
	RECT rc = this->GetRegion();
	this->SetPos(CN3Base::s_CameraData.vp.Width - (rc.right - rc.left), 10);
	m_fMoveDelta = 0;
	m_bOpenningNow = false;
	m_bClosingNow = true;

	//m_iRBtnDownOffs = -1;
}

void CUICmdList::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);
	if (bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
		CGameProcedure::s_pUIMgr->ReFocusUI();//this_ui
}

bool CUICmdList::CreateCategoryList() {

	if (m_pList_CmdCat == nullptr || m_pList_Cmds == nullptr) return false;

	std::string szCategory;
	int idStart = IDS_PRIVATE_CMD_CAT;

	for (int i = 0; i < 8; i++)
	{
		CGameBase::GetText(i + 7800, &szCategory); //load command categories
		m_pList_CmdCat->AddString(szCategory);
		idStart ++;
	}

	m_pList_CmdCat->SetFontColor(0xffffff00); //green

	int idCur = 8000;   //Command list strings start at this index
	int idEnd = 9600;   //Command list strings end at this index

	std::string szCommand;
	//create map of commands
	for (int i = idCur; idCur < idEnd; idCur++, i++) {
		if (idCur == 9000) 
			i += 400; // offset and put gm cmds at end of map
		else if (idCur == 9100) {
			i -= 500;
			idCur = 9200;
		}
		szCommand.clear();
		CGameBase::GetText(idCur, &szCommand);
		if (!szCommand.empty() && (i/100) % 2 == 0 ) m_mapCmds[i] = szCommand;
	}

	UpdateCommandList(CMD_LIST_PRIVATE); //initialize a cmd list for viewing when opening cmd window

	return true;
}

bool CUICmdList::UpdateCommandList(uint8_t cmdCat ) {

	if (m_pList_Cmds == nullptr) return false;
	
	m_pList_Cmds->ResetContent();
	
	int indexStart = cmdCat * 200 + 8000;  //start index for correct loc in map
	int indexEnd = indexStart + 100;	  //where to stop iterating
	int i = 0;

	for (auto itr = m_mapCmds.begin(); itr != m_mapCmds.end(); ++itr) {
		if (itr->first >= indexStart && itr->first < indexEnd) {
				 m_pList_Cmds->AddString(itr->second);
				 
				 CN3UIString* pChild = m_pList_Cmds->GetChildStrFromList(itr->second);
				 std::string cmdTip, cmdName;
				 
				 //get command name
				 CGameBase::GetText(itr->first, &cmdName);

				 //fill with command name exp: /type %s, to /type ban_user
				 CGameBase::GetTextF(itr->first + 100, &cmdTip, cmdName.c_str());
				 
				 if(pChild != nullptr) pChild->SetTooltipText(cmdTip.c_str());
				 
		}
	}

	return true;
}

bool CUICmdList::ExecuteCommand(uint8_t cmdSel) {

	std::string command;
	m_pList_Cmds->GetString(cmdSel, command);

	if (command == "PM") {
		CGameProcedure::s_pProcMain->OpenCmdEdit(command);
	}

	command = '/' + command;
	CGameProcedure::s_pProcMain->ParseChattingCommand(command);

	return true;

}

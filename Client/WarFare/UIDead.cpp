// UIDead.cpp: implementation of the CUIDead class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UIDead.h"
#include "PacketDef.h"
#include "APISocket.h"
#include "GameProcedure.h"
#include "GameProcMain.h"
#include "UIManager.h"
#include "PlayerMySelf.h"
#include "UIInventory.h"
#include "UIStateBar.h"
#include "MagicSkillMng.h"
#include "N3FXMgr.h"
#include "text_resources.h"

#include <N3Base/N3UITooltip.h>
#include <N3Base/N3UIString.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#define CHILD_UI_REVIVE_MSG				1
#define CHILD_UI_LACK_LIVE_STONE_MSG	2
#define CHILD_UI_LOW_LEVEL				3

#define TIMES_LIFE_STONE				3
#define LIFE_STONE_INDEX				379006000

#define REVIVAL_TYPE_RETURN_TOWN		1
#define REVIVAL_TYPE_LIFE_STONE			2
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUIDead::CUIDead()
{
	m_pTextAlive	= NULL;
	m_pTextTown		= NULL;
	m_bProcessing	= false;
}

CUIDead::~CUIDead()
{

}

bool CUIDead::Load(HANDLE hFile)
{
	if(false == CN3UIBase::Load(hFile)) return false;

	m_pTextAlive	= (CN3UIString*)(this->GetChildByID("Text_Alive"));	__ASSERT(m_pTextAlive, "NULL UI Component!!!");
	m_pTextTown		= (CN3UIString*)(this->GetChildByID("Text_Town"));	__ASSERT(m_pTextTown, "NULL UI Component!!!");


	std::string szMsg = fmt::format_text_resource(IDS_DEAD_REVIVAL);
	if(m_pTextAlive) m_pTextAlive->SetString(szMsg);

	szMsg = fmt::format_text_resource(IDS_DEAD_RETURN_TOWN);
	if(m_pTextTown) m_pTextTown->SetString(szMsg);

	__TABLE_UI_RESRC*	pTblUI	= NULL;
	pTblUI = CGameBase::s_pTbl_UI.Find(NATION_ELMORAD);

	m_MsgBox.LoadFromFile(pTblUI->szMessageBox);

	RECT rt = m_MsgBox.GetRegion();
	POINT pt;
	pt.x = (CN3Base::s_CameraData.vp.Width - (rt.right - rt.left)) / 2;
	pt.y = (CN3Base::s_CameraData.vp.Height - (rt.bottom - rt.top)) / 2;
	m_MsgBox.SetPos(pt.x, pt.y);

	return true;
}

bool CUIDead::ReceiveMessage(CN3UIBase *pSender, uint32_t dwMsg)
{
	if( dwMsg == UIMSG_STRING_LCLICK )
	{
		if(pSender == m_pTextAlive)
		{
			int iItemCnt = 0, iLevel = 0, iNeedItemCnt = 0;
			if(CGameProcedure::s_pProcMain->m_pUIInventory)
				iItemCnt = CGameProcedure::s_pProcMain->m_pUIInventory->GetIndexItemCount(LIFE_STONE_INDEX);

			iLevel = CGameProcedure::s_pPlayer->m_InfoBase.iLevel;
			iNeedItemCnt = iLevel * TIMES_LIFE_STONE;
			std::string szMsg;

			if (iLevel < 6)
			{
				szMsg = fmt::format_text_resource(IDS_DEAD_LOW_LEVEL, iNeedItemCnt);

				m_MsgBox.SetBoxStyle(MB_OK);
				m_MsgBox.m_eBehavior = BEHAVIOR_NOTHING;
				m_MsgBox.SetTitle("");
				m_MsgBox.SetText(szMsg);
				m_MsgBox.ShowWindow(CHILD_UI_LOW_LEVEL, this);
			}
			else if (iItemCnt >= iNeedItemCnt)
			{
				szMsg = fmt::format_text_resource(IDS_DEAD_REVIVAL_MESSAGE, iNeedItemCnt);

				m_MsgBox.SetBoxStyle(MB_YESNO);
				m_MsgBox.m_eBehavior = BEHAVIOR_NOTHING;
				m_MsgBox.SetTitle("");
				m_MsgBox.SetText(szMsg);
				m_MsgBox.ShowWindow(CHILD_UI_REVIVE_MSG, this);
			}
			else
			{
				szMsg = fmt::format_text_resource(IDS_DEAD_LACK_LIFE_STONE);

				m_MsgBox.SetBoxStyle(MB_OK);
				m_MsgBox.m_eBehavior = BEHAVIOR_NOTHING;
				m_MsgBox.SetTitle("");
				m_MsgBox.SetText(szMsg);
				m_MsgBox.ShowWindow(CHILD_UI_LACK_LIVE_STONE_MSG, this);
			}
		}
		else if (pSender == m_pTextTown)
		{
			MsgSend_Revival(REVIVAL_TYPE_RETURN_TOWN);
		}
	}

	return true;
}

uint32_t CUIDead::MouseProc(uint32_t dwFlags, const POINT &ptCur, const POINT &ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (!m_bVisible) return dwRet;

	// UI 움직이는 코드
	if (UI_STATE_COMMON_MOVE == m_eState)
	{
		if (dwFlags&UI_MOUSE_LBCLICKED)
		{
			SetState(UI_STATE_COMMON_NONE);
		}
		else
		{
			MoveOffset(ptCur.x - ptOld.x, ptCur.y - ptOld.y);
		}
		dwRet |= UI_MOUSEPROC_DONESOMETHING;
		return dwRet;
	}

	if(false == IsIn(ptCur.x, ptCur.y))	// 영역 밖이면
	{
		if(false == IsIn(ptOld.x, ptOld.y))
		{
			return dwRet;// 이전 좌표도 영역 밖이면 
		}
		dwRet |= UI_MOUSEPROC_PREVINREGION;	// 이전 좌표는 영역 안이었다.
	}
	else
	{
		// tool tip 관련
		if (s_pTooltipCtrl != nullptr)
			s_pTooltipCtrl->SetText(m_szToolTip, m_crToolTip);
	}

	dwRet |= UI_MOUSEPROC_INREGION;	// 이번 좌표는 영역 안이다.

	if(m_pChildUI && m_pChildUI->IsVisible())
		return dwRet;

	// child에게 메세지 전달
	for(UIListItor itor = m_Children.begin(); m_Children.end() != itor; ++itor)
	{
		CN3UIBase* pChild = (*itor);
		uint32_t dwChildRet = 0;

		dwChildRet = pChild->MouseProc(dwFlags, ptCur, ptOld);
		if( pChild->IsVisible() && UI_TYPE_STRING == pChild->UIType() )
		{
			if(pChild->IsIn(ptCur.x, ptCur.y) && (dwFlags & UI_MOUSE_LBCLICKED) )	
			{
				dwChildRet |= UI_MOUSEPROC_DONESOMETHING;
			}

			if(pChild->IsIn(ptCur.x, ptCur.y) && (dwFlags & UI_MOUSE_LBDBLCLK) )	
			{
				dwChildRet |= UI_MOUSEPROC_DONESOMETHING;
			}
		}

		if (UI_MOUSEPROC_DONESOMETHING & dwChildRet)
		{	// 이경우에는 먼가 포커스를 받은 경우이다.
			// (아래 코드는 dialog를 관리하는 곳에서 해야 한다. 따라서 막아놓음)
//			m_Children.erase(itor);			// 우선 리스트에서 지우고
//			m_Children.push_front(pChild);	// 맨앞에 넣는다. 그리는 순서를 맨 나중에 그리도록 하려고

			dwRet |= (UI_MOUSEPROC_CHILDDONESOMETHING|UI_MOUSEPROC_DONESOMETHING);
			return dwRet;
		}
	}

	// UI 움직이는 코드
	if (UI_STATE_COMMON_MOVE != m_eState && 
			PtInRect(&m_rcMovable, ptCur) && (dwFlags&UI_MOUSE_LBCLICK) )
	{
		SetState(UI_STATE_COMMON_MOVE);
		dwRet |= UI_MOUSEPROC_DONESOMETHING;
		return dwRet;
	}

	return dwRet;
}

void CUIDead::CallBackProc(int iID, uint32_t dwFlag)
{
	//TRACE("OnButton ID:%d Btn %d\n",iID, dwFlag);

	if(iID == CHILD_UI_REVIVE_MSG)
	{
		if(dwFlag == 1)//OK
		{
			MsgSend_Revival(REVIVAL_TYPE_LIFE_STONE);
		}
	}
	else if(iID == CHILD_UI_LACK_LIVE_STONE_MSG)
	{
	}
	else if(iID == CHILD_UI_LOW_LEVEL)
	{
	}
}

void CUIDead::MsgSend_Revival(uint8_t byType)
{
	if(m_bProcessing) return;

	if(CGameProcedure::s_pPlayer->m_iSendRegeneration >= 2) return; // 한번 보내면 다시 죽을때까지 안보내는 플래그

	uint8_t byBuff[4];
	int iOffset=0;

	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_REGENE);
	CAPISocket::MP_AddByte(byBuff, iOffset, byType);
	CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보낸다..

	CLogWriter::Write("Send Regeneration");
	CGameProcedure::s_pPlayer->m_iSendRegeneration = 2; // 한번 보내면 다시 죽을때까지 안보내는 플래그
	//TRACE("보냄 - 다시 살아나기\n");

	m_bProcessing = true;
}

void CUIDead::MsgRecv_Revival(Packet& pkt)
{
	m_bProcessing = false;

	__Vector3 vPosPlayer;
	vPosPlayer.x = (pkt.read<uint16_t>())/10.0f;
	vPosPlayer.z = (pkt.read<uint16_t>())/10.0f;
	vPosPlayer.y = (pkt.read<int16_t>())/10.0f;
	
	CGameProcedure::s_pProcMain->InitPlayerPosition(vPosPlayer); // 플레이어 위치 초기화.. 일으켜 세우고, 기본동작을 취하게 한다.
	CGameProcedure::s_pPlayer->RegenerateCollisionMesh(); // 충돌 메시를 다시 만든다..

	CGameProcedure::s_pPlayer->m_iSendRegeneration = 0; // 한번 보내면 다시 죽을때까지 안보내는 플래그
	CGameProcedure::s_pPlayer->m_fTimeAfterDeath = 0; // 한번 보내면 다시 죽을때까지 안보내는 플래그
	//TRACE("받음 - 다시 살아나기(%.1f, %.1f)\n", vPosPlayer.x, vPosPlayer.z);

	//
	//마법 & 효과 초기화..
	if(CGameProcedure::s_pProcMain->m_pUIStateBarAndMiniMap) 
		CGameProcedure::s_pProcMain->m_pUIStateBarAndMiniMap->ClearMagic();
	if(CGameProcedure::s_pProcMain->m_pMagicSkillMng) 
		CGameProcedure::s_pProcMain->m_pMagicSkillMng->ClearDurationalMagic();
	if(CGameProcedure::s_pProcMain->CGameProcedure::s_pFX) 
		CGameProcedure::s_pFX->StopMine();

	CLogWriter::Write("Receive Regeneration");

	int iID = CGameProcedure::s_pPlayer->IDNumber();
	if(CGameProcedure::s_pPlayer->Nation()==NATION_KARUS) 
		CGameProcedure::s_pFX->TriggerBundle(iID, -1, FXID_REGEN_KARUS, iID, -1);
	else if(CGameProcedure::s_pPlayer->Nation()==NATION_ELMORAD) 
		CGameProcedure::s_pFX->TriggerBundle(iID, -1, FXID_REGEN_ELMORAD, iID, -1);

	SetVisible(false);
}

void CUIDead::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);
	if(bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
		CGameProcedure::s_pUIMgr->ReFocusUI();//this_ui
}

void CUIDead::SetVisibleWithNoSound(bool bVisible, bool bWork, bool bReFocus)
{
	if(!bWork)
	{
		CN3UIBase::SetVisibleWithNoSound(bVisible, bWork, bReFocus);
	}
}

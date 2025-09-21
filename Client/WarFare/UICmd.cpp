// UICmd.cpp: implementation of the CUICmd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UICmd.h"
#include "GameProcMain.h"
#include "PlayerOtherMgr.h"
#include "PlayerMyself.h"
#include "UITransactionDlg.h"
#include "UIManager.h"
#include "text_resources.h"

#include <N3Base/N3UIButton.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUICmd::CUICmd()
{
	m_pBtn_Exit = nullptr;			// 나가기

	m_pBtn_Act = nullptr;			// 행동
	m_pBtn_Act_Walk = nullptr;		// 걷기
	m_pBtn_Act_Run = nullptr;		// 달리기
	m_pBtn_Act_Attack = nullptr;	// 공격
	
	m_pBtn_Act_StandUp = nullptr;	// 일어서기.
	m_pBtn_Act_SitDown = nullptr;	// 앉기

	m_pBtn_Camera = nullptr;		// 카메라
	m_pBtn_Inventory = nullptr;		// 아이템 창 
	m_pBtn_Party_Invite = nullptr;	// 파티 초대
	m_pBtn_Party_Disband = nullptr;	// 파티 탈퇴
	m_pBtn_CmdList = nullptr;		// 옵션
	m_pBtn_Character = nullptr;		// 자기 정보창   
	m_pBtn_Skill = nullptr;			// 스킬트리 또는 마법창 
	m_pBtn_Map = nullptr;			// 미니맵
}

CUICmd::~CUICmd()
{
}

bool CUICmd::Load(HANDLE hFile)
{
	if (!CN3UIBase::Load(hFile))
		return false;
	
	N3_VERIFY_UI_COMPONENT(m_pBtn_Act,				GetChildByID<CN3UIButton>("btn_control"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Act_Walk,			GetChildByID<CN3UIButton>("btn_walk"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Act_Run,			GetChildByID<CN3UIButton>("btn_run"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Act_Attack,		GetChildByID<CN3UIButton>("btn_attack"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Act_StandUp,		GetChildByID<CN3UIButton>("btn_stand"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Act_SitDown,		GetChildByID<CN3UIButton>("btn_sit"));

	// 일어서기 버튼은 미리 죽여놓는다..
	if (m_pBtn_Act_StandUp != nullptr)
		m_pBtn_Act_StandUp->SetVisible(false); 
	
	N3_VERIFY_UI_COMPONENT(m_pBtn_Character,		GetChildByID<CN3UIButton>("btn_character"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Inventory,		GetChildByID<CN3UIButton>("btn_inventory"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_CmdList,			GetChildByID<CN3UIButton>("btn_option"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Camera,			GetChildByID<CN3UIButton>("btn_camera"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Party_Invite,		GetChildByID<CN3UIButton>("btn_invite"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Party_Disband,	GetChildByID<CN3UIButton>("btn_disband"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Skill,			GetChildByID<CN3UIButton>("btn_skill"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Exit,				GetChildByID<CN3UIButton>("btn_exit"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Map,				GetChildByID<CN3UIButton>("btn_map"));

//	this->SetVisibleActButtons(true);
//	this->SetVisibleOptButtons(false);

	return true;
}

bool CUICmd::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if ( CGameProcedure::s_pProcMain->m_pUITransactionDlg->IsVisible() )	
			return true;

	if (dwMsg == UIMSG_BUTTON_CLICK)					
	{
		if(pSender == m_pBtn_CmdList)
		{
			CGameProcedure::s_pProcMain->CommandToggleCmdList();
		}

		if(pSender == m_pBtn_Act)
		{
//			this->SetVisibleActButtons(true);
//			this->SetVisibleOptButtons(false);
		}

		else if(pSender == m_pBtn_Act_Walk)
		{
			CGameProcedure::s_pProcMain->CommandToggleWalkRun();
		}
			
		else if(pSender == m_pBtn_Act_Run)
		{
			CGameProcedure::s_pProcMain->CommandToggleWalkRun();
		}

		else if(pSender == m_pBtn_Act_Attack)
		{
			CGameProcedure::s_pProcMain->CommandToggleAttackContinous();
		}

		else if(pSender == m_pBtn_Inventory)
		{
			CGameProcedure::s_pProcMain->CommandToggleUIInventory();
		}
		
		else if(pSender == m_pBtn_Character)
		{
			CGameProcedure::s_pProcMain->CommandToggleUIState();
		}

		else if(pSender == m_pBtn_Exit) 
		{
			//m_bSuppressNextMouseFocus = true;
			CGameProcedure::s_pProcMain->RequestExit();
		}

		else if(pSender == m_pBtn_Camera)
		{
			CGameProcedure::s_pProcMain->CommandCameraChange(); 
		}

		else if(pSender == m_pBtn_Party_Invite)
		{
			CPlayerOther* pUPC = CGameProcedure::s_pOPMgr->UPCGetByID(CGameBase::s_pPlayer->m_iIDTarget, true);

			// 국가 체크
			if (pUPC != nullptr
				&& !CGameBase::s_pPlayer->IsHostileTarget(pUPC))
				CGameProcedure::s_pProcMain->MsgSend_PartyOrForceCreate(0, pUPC->IDString()); // 파티 초대하기..
		}

		else if(pSender == m_pBtn_Party_Disband)
		{
			CGameProcMain* pMain = CGameProcedure::s_pProcMain;
			CPlayerMySelf* pPlayer = CGameBase::s_pPlayer;

			bool bIAmLeader = false, bIAmMemberOfParty = false;
			int iMemberIndex = -1;
			CPlayerBase* pTarget = nullptr;
			pMain->PartyOrForceConditionGet(bIAmLeader, bIAmMemberOfParty, iMemberIndex, pTarget); // 파티의 상황을 보고..
			
			std::string szMsg;
			if(bIAmLeader) // 내가 리더면..
			{
				if(iMemberIndex > 0)
				{
					szMsg = fmt::format_text_resource(IDS_PARTY_CONFIRM_DISCHARGE);
					szMsg = pTarget->IDString() + szMsg;
				}
				else szMsg = fmt::format_text_resource(IDS_PARTY_CONFIRM_DESTROY);
			}
			else if(bIAmMemberOfParty)
			{
				szMsg = fmt::format_text_resource(IDS_PARTY_CONFIRM_LEAVE);
			}

			if(!szMsg.empty()) CGameProcedure::MessageBoxPost(szMsg, "", MB_YESNO, BEHAVIOR_PARTY_DISBAND); // 파티 해체,축출,탈퇴하기..확인
		}

		else if(pSender == m_pBtn_Act_SitDown)
		{
			CGameProcedure::s_pProcMain->CommandSitDown(true, true);
		}
		
		else if(pSender == m_pBtn_Act_StandUp)
		{
			CGameProcedure::s_pProcMain->CommandSitDown(true, false);
		}

		else if(pSender == m_pBtn_Skill)
		{
			CGameProcedure::s_pProcMain->CommandToggleUISkillTree();
		}

		else if(pSender == m_pBtn_Map)
		{
			CGameProcedure::s_pProcMain->CommandToggleUIMiniMap();
		}
	}

	return true;
}

/*
void CUICmd::SetVisibleActButtons(bool bVisible)
{
	//행동
	if(m_pBtn_Act_Walk) m_pBtn_Act_Walk->SetVisible(bVisible); 
	if(m_pBtn_Act_Run) m_pBtn_Act_Run->SetVisible(bVisible);
	if(m_pBtn_Act_Stop) m_pBtn_Act_Stop->SetVisible(bVisible);
	if(m_pBtn_Act_StandUp) m_pBtn_Act_StandUp->SetVisible(bVisible);
	if(m_pBtn_Act_SitDown) m_pBtn_Act_SitDown->SetVisible(bVisible);
	if(m_pBtn_Act_Attack) m_pBtn_Act_Attack->SetVisible(bVisible);
}
*/

/*
void CUICmd::SetVisibleOptButtons(bool bVisible)
{
	//옵션
	if(m_pBtn_Opt_Quest) m_pBtn_Opt_Quest->SetVisible(bVisible);
	if(m_pBtn_Character) m_pBtn_Character->SetVisible(bVisible);	
	if(m_pBtn_Skill) m_pBtn_Skill->SetVisible(bVisible);
	if(m_pBtn_Opt_Knight) m_pBtn_Opt_Knight->SetVisible(bVisible);
	if(m_pBtn_Inventory) m_pBtn_Inventory->SetVisible(bVisible);	
}
*/

void CUICmd::UpdatePartyButtons(bool bIAmLeader, bool bIAmMemberOfParty, int iMemberIndex, const CPlayerBase* pTarget)
{
	bool bInvite = true;
	if(bIAmLeader) // 내가 리더이면.. 
	{
		if(pTarget) // 타겟이 있고..
		{
			if(iMemberIndex > 0) bInvite = false; // 타겟이 파티원이면..축출 가능하게.
			else bInvite = true;
		}
		else
		{
			bInvite = false; // 리더도 나갈수 있다..
		}
	}
	else
	{
		if(bIAmMemberOfParty) bInvite = false; // 리더는 아니지만 파티에 들어있는 상태이면.. 탈퇴가능..
		else bInvite = true; // 파티에 안들어 있다면 초대 가능...
	}

	if(m_pBtn_Party_Invite) m_pBtn_Party_Invite->SetVisible(bInvite);
	if(m_pBtn_Party_Disband) m_pBtn_Party_Disband->SetVisible(!bInvite);
}

bool CUICmd::OnKeyPress(int iKey)
{
	switch(iKey)
	{
	case DIK_ESCAPE:
		{	//hotkey가 포커스 잡혀있을때는 다른 ui를 닫을수 없으므로 DIK_ESCAPE가 들어오면 포커스를 다시잡고
			//열려있는 다른 유아이를 닫아준다.
			CGameProcedure::s_pUIMgr->ReFocusUI();//this_ui
			CN3UIBase* pFocus = CGameProcedure::s_pUIMgr->GetFocusedUI();
			if (pFocus != nullptr && pFocus != this)
				pFocus->OnKeyPress(iKey);
		}
		return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

// UICmd.h: interface for the CUICmd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UICmd_H__CA4F5382_D9A9_447C_B717_7A0A38724715__INCLUDED_)
#define AFX_UICmd_H__CA4F5382_D9A9_447C_B717_7A0A38724715__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3UIBase.h>

class CUICmd : public CN3UIBase  
{
public:
	CN3UIBase*	m_pBtn_Exit;			//나가기

	CN3UIBase*	m_pBtn_Act;			//행동
	CN3UIBase*	m_pBtn_Act_Walk;	//걷기
	CN3UIBase*	m_pBtn_Act_Run;		//달리기
	CN3UIBase*	m_pBtn_Act_Stop;	//서기
	CN3UIBase*	m_pBtn_Act_Attack;	//공격
	
	CN3UIBase*	m_pBtn_Act_StandUp; // 일어서기.
	CN3UIBase*	m_pBtn_Act_SitDown;	// 앉기

	CN3UIBase*	m_pBtn_Camera;			//카메라
	CN3UIBase*	m_pBtn_Inventory;		//아이템 창 
	CN3UIBase*	m_pBtn_Party_Invite;	//파티 초대
	CN3UIBase*	m_pBtn_Party_Disband;	//파티 탈퇴
	CN3UIBase*  m_pBtn_CmdList;			//옵션
	CN3UIBase*	m_pBtn_Quest;			//퀘스트
	CN3UIBase*	m_pBtn_Character;		//자기 정보창   
	CN3UIBase*	m_pBtn_Skill;			//스킬트리 또는 마법창 
	CN3UIBase*	m_pBtn_Belong;			//소속 기사단 
	CN3UIBase*	m_pBtn_Map;				// 미니맵

public:
	bool OnKeyPress(int iKey);
//	void	SetVisibleOptButtons(bool bVisible);
//	void	SetVisibleActButtons(bool bVisible);

	bool	Load(HANDLE hFile);
	bool	ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);
	void	UpdatePartyButtons(bool bIAmLeader, bool bIAmMemberOfParty, int iMemberIndex, const class CPlayerBase* pTarget);

	CUICmd();
	virtual ~CUICmd();
};

#endif // !defined(AFX_UICmd_H__CA4F5382_D9A9_447C_B717_7A0A38724715__INCLUDED_)

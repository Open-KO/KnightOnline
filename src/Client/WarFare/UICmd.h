// UICmd.h: interface for the CUICmd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UICmd_H__CA4F5382_D9A9_447C_B717_7A0A38724715__INCLUDED_)
#define AFX_UICmd_H__CA4F5382_D9A9_447C_B717_7A0A38724715__INCLUDED_

#pragma once

#include <N3Base/N3UIBase.h>

class CUICmd : public CN3UIBase
{
public:
	CN3UIButton* m_pBtn_Exit;          // 나가기

	CN3UIButton* m_pBtn_Act;           // 행동
	CN3UIButton* m_pBtn_Act_Walk;      // 걷기
	CN3UIButton* m_pBtn_Act_Run;       // 달리기
	CN3UIButton* m_pBtn_Act_Attack;    // 공격

	CN3UIButton* m_pBtn_Act_StandUp;   // 일어서기.
	CN3UIButton* m_pBtn_Act_SitDown;   // 앉기

	CN3UIButton* m_pBtn_Camera;        // 카메라
	CN3UIButton* m_pBtn_Inventory;     // 아이템 창
	CN3UIButton* m_pBtn_Party_Invite;  // 파티 초대
	CN3UIButton* m_pBtn_Party_Disband; // 파티 탈퇴
	CN3UIButton* m_pBtn_CmdList;       // 옵션
	CN3UIButton* m_pBtn_Character;     // 자기 정보창
	CN3UIButton* m_pBtn_Skill;         // 스킬트리 또는 마법창
	CN3UIButton* m_pBtn_Map;           // 미니맵

public:
	CUICmd();
	~CUICmd() override;
	//	void SetVisibleOptButtons(bool bVisible);
	//	void SetVisibleActButtons(bool bVisible);
	bool OnKeyPress(int iKey) override;
	bool Load(File& file) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	void UpdatePartyButtons(bool bIAmLeader, bool bIAmMemberOfParty, int iMemberIndex, const class CPlayerBase* pTarget);
};

#endif // !defined(AFX_UICmd_H__CA4F5382_D9A9_447C_B717_7A0A38724715__INCLUDED_)

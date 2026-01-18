// UISkillTreeDlg.cpp: implementation of the CUISkillTreeDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UISkillTreeDlg.h"
#include "PacketDef.h"
#include "LocalInput.h"
#include "APISocket.h"
#include "PlayerMySelf.h"
#include "GameProcMain.h"
#include "UIHotKeyDlg.h"
#include "MagicSkillMng.h"
#include "UIManager.h"
#include "N3UIIcon.h"
#include "text_resources.h"

#include <N3Base/N3UIArea.h>
#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIString.h>
#include <N3Base/N3SndObj.h>

CUISkillTreeDlg::CUISkillTreeDlg()
{
	m_bOpenningNow     = false; // 열리고 있다..
	m_bClosingNow      = false; // 닫히고 있다..
	m_fMoveDelta       = 0.0f;  // 부드럽게 열리고 닫히게 만들기 위해서 현재위치 계산에 부동소수점을 쓴다..

	m_iRBtnDownOffs    = -1;

	m_iCurKindOf       = 0;
	m_iCurSkillPage    = 0;

	m_pStr_skill_item0 = nullptr;
	m_pStr_skill_item1 = nullptr;
	m_pStr_skill_item2 = nullptr;
	m_pStr_skill_point = nullptr;
	m_pStr_skill_mp    = nullptr;
	m_pStr_info        = nullptr;

	for (int i = 0; i < MAX_SKILL_FROM_SERVER; i++)
		m_iSkillInfo[i] = 0;

	for (int i = 0; i < MAX_SKILL_KIND_OF; i++)
	{
		for (int j = 0; j < MAX_SKILL_PAGE_NUM; j++)
		{
			for (int k = 0; k < MAX_SKILL_IN_PAGE; k++)
				m_pMySkillTree[i][j][k] = nullptr;
		}
	}

	s_sSkillSelectInfo.UIWnd          = UIWND_HOTKEY;
	s_sSkillSelectInfo.pSkillDoneInfo = nullptr;
}

CUISkillTreeDlg::~CUISkillTreeDlg()
{
	CUISkillTreeDlg::Release();
}

void CUISkillTreeDlg::Release()
{
	CN3UIWndBase::Release();

	m_bOpenningNow = false; // 열리고 있다..
	m_bClosingNow  = false; // 닫히고 있다..
	m_fMoveDelta   = 0.0f;  // 부드럽게 열리고 닫히게 만들기 위해서 현재위치 계산에 부동소수점을 쓴다..

	for (int i = 0; i < MAX_SKILL_KIND_OF; i++)
	{
		for (int j = 0; j < MAX_SKILL_PAGE_NUM; j++)
		{
			for (int k = 0; k < MAX_SKILL_IN_PAGE; k++)
			{
				delete m_pMySkillTree[i][j][k];
				m_pMySkillTree[i][j][k] = nullptr;
			}
		}
	}

	m_pStr_skill_item0 = nullptr;
	m_pStr_skill_item1 = nullptr;
	m_pStr_skill_item2 = nullptr;
	m_pStr_skill_point = nullptr;
	m_pStr_skill_mp    = nullptr;
	m_pStr_info        = nullptr;

	if (s_sSkillSelectInfo.pSkillDoneInfo != nullptr && s_sSkillSelectInfo.UIWnd == UIWND_SKILL_TREE)
	{
		delete s_sSkillSelectInfo.pSkillDoneInfo;
		s_sSkillSelectInfo.pSkillDoneInfo = nullptr;
	}
}

bool CUISkillTreeDlg::HasIDSkill(int iID) const
{
	for (int i = 0; i < MAX_SKILL_KIND_OF; i++)
	{
		for (int j = 0; j < MAX_SKILL_PAGE_NUM; j++)
		{
			for (int k = 0; k < MAX_SKILL_IN_PAGE; k++)
			{
				if (m_pMySkillTree[i][j][k] != nullptr && m_pMySkillTree[i][j][k]->pSkill->dwID == iID)
					return true;
			}
		}
	}

	return false;
}

void CUISkillTreeDlg::UpdateDisableCheck()
{
	for (int i = 0; i < MAX_SKILL_KIND_OF; i++)
	{
		for (int j = 0; j < MAX_SKILL_PAGE_NUM; j++)
		{
			for (int k = 0; k < MAX_SKILL_IN_PAGE; k++)
			{
				if (m_pMySkillTree[i][j][k] == nullptr)
					continue;

				uint32_t bitMask = UISTYLE_ICON_SKILL;
				if (!CGameProcedure::s_pProcMain->m_pMagicSkillMng->CheckValidSkillMagic(m_pMySkillTree[i][j][k]->pSkill))
					bitMask |= UISTYLE_DISABLE_SKILL;
				m_pMySkillTree[i][j][k]->pUIIcon->SetStyle(bitMask);
			}
		}
	}
}

void CUISkillTreeDlg::Tick()
{
	if (m_bOpenningNow) // 오른쪽에서 왼쪽으로 스르륵...열려야 한다면..
	{
		POINT ptCur   = this->GetPos();
		RECT rc       = this->GetRegion();
		float fWidth  = (float) (rc.right - rc.left);

		float fDelta  = 5000.0f * CN3Base::s_fSecPerFrm;
		fDelta       *= (fWidth - m_fMoveDelta) / fWidth;
		if (fDelta < 2.0f)
			fDelta = 2.0f;
		m_fMoveDelta += fDelta;

		int iXLimit   = CN3Base::s_CameraData.vp.Width - (int) fWidth;
		ptCur.x       = CN3Base::s_CameraData.vp.Width - (int) m_fMoveDelta;
		if (ptCur.x <= iXLimit) // 다열렸다!!
		{
			ptCur.x        = iXLimit;
			m_bOpenningNow = false;
		}

		this->SetPos(ptCur.x, ptCur.y);
	}
	else if (m_bClosingNow) // 오른쪽에서 왼쪽으로 스르륵...열려야 한다면..
	{
		POINT ptCur   = this->GetPos();
		RECT rc       = this->GetRegion();
		float fWidth  = (float) (rc.right - rc.left);

		float fDelta  = 5000.0f * CN3Base::s_fSecPerFrm;
		fDelta       *= (fWidth - m_fMoveDelta) / fWidth;
		if (fDelta < 2.0f)
			fDelta = 2.0f;
		m_fMoveDelta += fDelta;

		int iXLimit   = CN3Base::s_CameraData.vp.Width;
		ptCur.x       = CN3Base::s_CameraData.vp.Width - (int) (fWidth - m_fMoveDelta);
		if (ptCur.x >= iXLimit) // 다 닫혔다..!!
		{
			ptCur.x       = iXLimit;
			m_bClosingNow = false;

			this->SetVisibleWithNoSound(false, false, true); // 다 닫혔으니 눈에서 안보이게 한다.
		}

		this->SetPos(ptCur.x, ptCur.y);
	}

	CN3UIBase::Tick();
}

uint32_t CUISkillTreeDlg::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (!IsVisible()
		// 실제로 쓰진 않는다..
		|| s_bWaitFromServer)
	{
		// NOLINTNEXTLINE(bugprone-parent-virtual-call)
		dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);
		return dwRet;
	}

	// 드래그 되는 아이콘 갱신..
	if (GetState() == UI_STATE_ICON_MOVING)
	{
		if (CN3UIWndBase::s_sSkillSelectInfo.pSkillDoneInfo)
		{
			CN3UIWndBase::s_sSkillSelectInfo.pSkillDoneInfo->pUIIcon->SetRegion(GetSampleRect());
			CN3UIWndBase::s_sSkillSelectInfo.pSkillDoneInfo->pUIIcon->SetMoveRect(GetSampleRect());
		}
	}

	return CN3UIWndBase::MouseProc(dwFlags, ptCur, ptOld);
}

int CUISkillTreeDlg::GetIndexInArea(POINT pt)
{
	CN3UIArea* pArea = nullptr;
	RECT rect {};

	for (int i = 0; i < MAX_SKILL_IN_PAGE; i++)
	{
		pArea = GetChildAreaByiOrder(UI_AREA_TYPE_SKILL_TREE, i);
		if (pArea == nullptr)
			continue;

		rect = pArea->GetRegion();
		if ((pt.x >= rect.left) && (pt.x <= rect.right) && (pt.y >= rect.top) && (pt.y <= rect.bottom))
			return i;
	}

	return -1;
}

bool CUISkillTreeDlg::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (nullptr == pSender)
		return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender->m_szID == "btn_close")
			Close();
		//..
		if (pSender->m_szID == "btn_left")
			PageLeft();
		if (pSender->m_szID == "btn_right")
			PageRight();
		//..
		if (pSender->m_szID == "btn_0")
			PointPushUpButton(1);
		if (pSender->m_szID == "btn_1")
			PointPushUpButton(2);
		if (pSender->m_szID == "btn_2")
			PointPushUpButton(3);
		if (pSender->m_szID == "btn_3")
			PointPushUpButton(4);
		if (pSender->m_szID == "btn_4")
			PointPushUpButton(5);
		if (pSender->m_szID == "btn_5")
			PointPushUpButton(6);
		if (pSender->m_szID == "btn_6")
			PointPushUpButton(7);
		if (pSender->m_szID == "btn_7")
			PointPushUpButton(8);
		//..
		if (pSender->m_szID == "btn_public")
			SetPageInIconRegion(0, 0);
		if ((pSender->m_szID == "btn_ranger0") || (pSender->m_szID == "btn_blade0") || (pSender->m_szID == "btn_mage0")
			|| (pSender->m_szID == "btn_cleric0") || (pSender->m_szID == "btn_hunter0") || (pSender->m_szID == "btn_berserker0")
			|| (pSender->m_szID == "btn_sorcerer0") || (pSender->m_szID == "btn_shaman0"))
			SetPageInIconRegion(1, 0);
		if ((pSender->m_szID == "btn_ranger1") || (pSender->m_szID == "btn_blade1") || (pSender->m_szID == "btn_mage1")
			|| (pSender->m_szID == "btn_cleric1") || (pSender->m_szID == "btn_hunter1") || (pSender->m_szID == "btn_berserker1")
			|| (pSender->m_szID == "btn_sorcerer1") || (pSender->m_szID == "btn_shaman1"))
			SetPageInIconRegion(2, 0);
		if ((pSender->m_szID == "btn_ranger2") || (pSender->m_szID == "btn_blade2") || (pSender->m_szID == "btn_mage2")
			|| (pSender->m_szID == "btn_cleric2") || (pSender->m_szID == "btn_hunter2") || (pSender->m_szID == "btn_berserker2")
			|| (pSender->m_szID == "btn_sorcerer2") || (pSender->m_szID == "btn_shaman2"))
			SetPageInIconRegion(3, 0);
		if ((pSender->m_szID == "btn_master"))
			SetPageInIconRegion(4, 0);
	}

	__IconItemSkill *spSkill = nullptr, *spSkillCopy = nullptr;

	uint32_t dwLBitMask = 0x000f0000;
	uint32_t dwRBitMask = 0x0f000000;
	uint32_t dwBitMask  = dwLBitMask | dwRBitMask;

	POINT ptCur         = CGameProcedure::s_pLocalInput->MouseGetPos();
	uint32_t bitMask    = 0;

	switch (dwMsg & dwBitMask)
	{
		case UIMSG_ICON_RDOWN_FIRST:
			m_iRBtnDownOffs = GetIndexInArea(ptCur);
			break;

		case UIMSG_ICON_RUP:
		{
			int iRBtn = GetIndexInArea(ptCur);
			if (iRBtn != -1 && m_iRBtnDownOffs != -1 && m_iRBtnDownOffs == iRBtn)
			{
				// 들어갈 자리가 있는지 검사..
				CUIHotKeyDlg* pDlg = CGameProcedure::s_pProcMain->m_pUIHotKeyDlg;
				int iIndex         = -1;
				if (pDlg->GetEmptySlotIndex(iIndex))
				{
					// Get Item..
					spSkill = GetHighlightIconItem((CN3UIIcon*) pSender);
					if (IsSkillUsable(spSkill->pSkill))
					{
						spSkillCopy           = new __IconItemSkill();
						spSkillCopy->pSkill   = spSkill->pSkill;
						spSkillCopy->szIconFN = spSkill->szIconFN;

						// 아이콘 로드하기.. ^^
						spSkillCopy->pUIIcon  = new CN3UIIcon;
						spSkillCopy->pUIIcon->Init(this);
						spSkillCopy->pUIIcon->SetTex(spSkill->szIconFN);
						spSkillCopy->pUIIcon->SetUVRect(0, 0, 1, 1);
						spSkillCopy->pUIIcon->SetUIType(UI_TYPE_ICON);

						bitMask = UISTYLE_ICON_SKILL;
						if (!CGameProcedure::s_pProcMain->m_pMagicSkillMng->CheckValidSkillMagic(spSkillCopy->pSkill))
							bitMask |= UISTYLE_DISABLE_SKILL;
						spSkillCopy->pUIIcon->SetStyle(bitMask);

						// Save Select Info..
						CN3UIWndBase::s_sSkillSelectInfo.UIWnd          = UIWND_SKILL_TREE;
						CN3UIWndBase::s_sSkillSelectInfo.pSkillDoneInfo = spSkillCopy;

						pDlg->SetReceiveSelectedSkill(iIndex);

						CN3UIWndBase::s_sSkillSelectInfo.pSkillDoneInfo = nullptr;
						pDlg->CloseIconRegistry();
					}
				}
			}
		}
		break;

		case UIMSG_ICON_DOWN_FIRST:
			// Get Item..
			spSkill               = GetHighlightIconItem((CN3UIIcon*) pSender);

			// 복사본을 만든다..
			spSkillCopy           = new __IconItemSkill();
			spSkillCopy->pSkill   = spSkill->pSkill;
			spSkillCopy->szIconFN = spSkill->szIconFN;

			// 아이콘 로드하기.. ^^
			spSkillCopy->pUIIcon  = new CN3UIIcon;
			spSkillCopy->pUIIcon->Init(this);
			spSkillCopy->pUIIcon->SetTex(spSkill->szIconFN);
			spSkillCopy->pUIIcon->SetUVRect(0, 0, 1, 1);
			spSkillCopy->pUIIcon->SetUIType(UI_TYPE_ICON);

			bitMask = UISTYLE_ICON_SKILL;
			if (!CGameProcedure::s_pProcMain->m_pMagicSkillMng->CheckValidSkillMagic(spSkillCopy->pSkill))
				bitMask |= UISTYLE_DISABLE_SKILL;
			spSkillCopy->pUIIcon->SetStyle(bitMask);

			spSkillCopy->pUIIcon->SetRegion(GetSampleRect());
			spSkillCopy->pUIIcon->SetMoveRect(GetSampleRect());

			// Save Select Info..
			CN3UIWndBase::s_sSkillSelectInfo.UIWnd          = UIWND_SKILL_TREE;
			CN3UIWndBase::s_sSkillSelectInfo.pSkillDoneInfo = spSkillCopy;
			break;

		case UIMSG_ICON_DOWN:
			if (GetState() == UI_STATE_ICON_MOVING)
			{
				if (CN3UIWndBase::s_sSkillSelectInfo.pSkillDoneInfo)
				{
					CN3UIWndBase::s_sSkillSelectInfo.pSkillDoneInfo->pUIIcon->SetRegion(GetSampleRect());
					CN3UIWndBase::s_sSkillSelectInfo.pSkillDoneInfo->pUIIcon->SetMoveRect(GetSampleRect());
				}
			}
			break;

		case UIMSG_ICON_UP:
			// Hot Key 윈도우를 돌아 다니면서 검사..
			{
				CUIHotKeyDlg* pDlg = CGameProcedure::s_pProcMain->m_pUIHotKeyDlg;
				if (!IsIn(ptCur.x, ptCur.y) && pDlg->IsIn(ptCur.x, ptCur.y))
				{
					if (!pDlg->IsSelectedSkillInRealIconArea())
					{
						// 리소스 Free..
						spSkill = CN3UIWndBase::s_sSkillSelectInfo.pSkillDoneInfo;
						if (spSkill)
						{
							// 매니저에서 제거..
							RemoveChild(spSkill->pUIIcon);

							// 리소스 제거..
							spSkill->pUIIcon->Release();
							delete spSkill->pUIIcon;
							spSkill->pUIIcon = nullptr;
							delete spSkill;
							spSkill = nullptr;
						}
					}
				}
				else
				{
					// 리소스 Free..
					spSkill = CN3UIWndBase::s_sSkillSelectInfo.pSkillDoneInfo;
					if (spSkill)
					{
						// 매니저에서 제거..
						RemoveChild(spSkill->pUIIcon);

						// 리소스 제거..
						spSkill->pUIIcon->Release();
						delete spSkill->pUIIcon;
						spSkill->pUIIcon = nullptr;
						delete spSkill;
						spSkill = nullptr;
					}
				}

				CN3UIWndBase::s_sSkillSelectInfo.pSkillDoneInfo = nullptr;
				SetState(UI_STATE_COMMON_NONE);
				pDlg->CloseIconRegistry();
			}
			break;

		case UIMSG_ICON_DBLCLK:
			SetState(UI_STATE_COMMON_NONE);
			break;

		default:
			break;
	}

	return false;
}

void CUISkillTreeDlg::PageLeft()
{
	if (m_iCurSkillPage == 0)
		return;

	SetPageInIconRegion(m_iCurKindOf, m_iCurSkillPage - 1);
}

void CUISkillTreeDlg::PageRight()
{
	if (m_iCurSkillPage == MAX_SKILL_PAGE_NUM - 1)
		return;

	SetPageInIconRegion(m_iCurKindOf, m_iCurSkillPage + 1);
}

void CUISkillTreeDlg::PointPushUpButton(int iValue)
{
	int iCurKindOfBackup = 0, iCurSkillPageBackup = 0;
	iCurKindOfBackup    = m_iCurKindOf;
	iCurSkillPageBackup = m_iCurSkillPage;

	// 스킬창의 값..
	int iSkillExtra = 0, iSkillPoint = 0;
	std::string str;
	CN3UIString *pStrName = nullptr, *pStrName2 = nullptr;
	N3_VERIFY_UI_COMPONENT(pStrName, GetChildByID<CN3UIString>("string_skillpoint"));
	str         = pStrName->GetString();
	iSkillExtra = atoi(str.c_str());

	if (iSkillExtra == 0)
	{
		std::string szMsg = fmt::format_text_resource(IDS_SKILL_POINT_EXTRA_NOT_EXIST);
		CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);
		return;
	}

	if ((iValue == 1) || (iValue == 2) || (iValue == 3) || (iValue == 4)) //..
	{
		std::string szMsg = fmt::format_text_resource(IDS_SKILL_POINT_NOT_YET);
		CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);
		return;
	}

	// 전직하지 않은 상태면.. 전문스킬은 올릴수 없다..
	switch (iValue)
	{
		case 5:
		case 6:
		case 7:
		case 8:
		{
			switch (CGameBase::s_pPlayer->m_InfoBase.eNation)
			{
				case NATION_KARUS: // 카루스..
					switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
					{
						case CLASS_KA_WARRIOR:
						case CLASS_KA_ROGUE:
						case CLASS_KA_WIZARD:
						case CLASS_KA_PRIEST:
						{
							std::string szMsg = fmt::format_text_resource(IDS_SKILL_POINT_BEFORE_CLASS_CHANGE);
							CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);
							return;
						}
						break;

						default:
							break;
					}
					break;

				case NATION_ELMORAD: // 엘모라도..
					switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
					{
						case CLASS_EL_WARRIOR:
						case CLASS_EL_ROGUE:
						case CLASS_EL_WIZARD:
						case CLASS_EL_PRIEST:
						{
							std::string szMsg = fmt::format_text_resource(IDS_SKILL_POINT_BEFORE_CLASS_CHANGE);
							CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);
							return;
						}
						break;

						default:
							break;
					}
					break;

				default:
					break;
			}
		}
		break;

		default:
			break;
	}

	if (iValue == 8)
	{
		switch (CGameBase::s_pPlayer->m_InfoBase.eNation)
		{
			case NATION_KARUS: // Karus
				switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
				{
					case CLASS_KA_SORCERER:
					case CLASS_KA_HUNTER:
					case CLASS_KA_SHAMAN:
					case CLASS_KA_BERSERKER:
					{
						std::string szMsg = fmt::format_text_resource(IDS_SKILL_POINT_NOT_YET);
						CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);
						return;
					}
					break;

					default:
						break;
				}
				break;

			case NATION_ELMORAD: // Elmorad
				switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
				{
					case CLASS_EL_MAGE:
					case CLASS_EL_RANGER:
					case CLASS_EL_CLERIC:
					case CLASS_EL_BLADE:
					{
						std::string szMsg = fmt::format_text_resource(IDS_SKILL_POINT_NOT_YET);
						CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);
						return;
					}
					break;

					default:
						break;
				}
				break;

			default:
				break;
		}
	}
	// WHY ROGUES CANT POINT TO ASSASIN - WHY WARRIORS CANT POINT TO PASSION ?
	/*
	if (iValue == 7)
	{
		switch ( CGameBase::s_pPlayer->m_InfoBase.eNation )
		{
			case NATION_KARUS:			// 카루스..
				switch ( CGameBase::s_pPlayer->m_InfoBase.eClass )
				{
					case CLASS_KA_BERSERKER:
						{
							std::string szMsg = fmt::format_text_resource(IDS_SKILL_POINT_NOT_YET);
							CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);	
							return;
						}
						break;
				}
				break;

			case NATION_ELMORAD:		// 엘모라도..
				switch ( CGameBase::s_pPlayer->m_InfoBase.eClass )
				{
					case CLASS_EL_BLADE:
						{
							std::string szMsg = fmt::format_text_resource(IDS_SKILL_POINT_NOT_YET);
							CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);	
							return;
						}
						break;
				}
				break;
		}
	}

	if (iValue == 6)
	{
		switch ( CGameBase::s_pPlayer->m_InfoBase.eNation )
		{
			case NATION_KARUS:			// 카루스..
				switch ( CGameBase::s_pPlayer->m_InfoBase.eClass )
				{
					case CLASS_KA_HUNTER:
						{
							std::string szMsg = fmt::format_text_resource(IDS_SKILL_POINT_NOT_YET);
							CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);	
							return;
						}
						break;
				}
				break;

			case NATION_ELMORAD:		// 엘모라도..
				switch ( CGameBase::s_pPlayer->m_InfoBase.eClass )
				{
					case CLASS_EL_RANGER:
						{
							std::string szMsg = fmt::format_text_resource(IDS_SKILL_POINT_NOT_YET);
							CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);	
							return;
						}
						break;
				}
				break;
		}
	}
	*/

	switch (iValue)
	{
		case 1:
			N3_VERIFY_UI_COMPONENT(pStrName2, GetChildByID<CN3UIString>("string_0"));
			break;

		case 2:
			N3_VERIFY_UI_COMPONENT(pStrName2, GetChildByID<CN3UIString>("string_1"));
			break;

		case 3:
			N3_VERIFY_UI_COMPONENT(pStrName2, GetChildByID<CN3UIString>("string_2"));
			break;

		case 4:
			N3_VERIFY_UI_COMPONENT(pStrName2, GetChildByID<CN3UIString>("string_3"));
			break;

		case 5:
			N3_VERIFY_UI_COMPONENT(pStrName2, GetChildByID<CN3UIString>("string_4"));
			break;

		case 6:
			N3_VERIFY_UI_COMPONENT(pStrName2, GetChildByID<CN3UIString>("string_5"));
			break;

		case 7:
			N3_VERIFY_UI_COMPONENT(pStrName2, GetChildByID<CN3UIString>("string_6"));
			break;

		case 8:
			N3_VERIFY_UI_COMPONENT(pStrName2, GetChildByID<CN3UIString>("string_7"));
			break;

		default:
			break;
	}

	if (pStrName2 == nullptr)
		return;

	str         = pStrName2->GetString();
	iSkillPoint = atoi(str.c_str());

	// 자기 자신 레벨보다 높일수 없다..
	if (iSkillPoint >= CGameBase::s_pPlayer->m_InfoBase.iLevel)
	{
		std::string szMsg = fmt::format_text_resource(IDS_SKILL_UP_INVALID);
		CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);
		return;
	}

	// 써버에게 보내고.. 숫자 업데이트..
	uint8_t byBuff[4];
	int iOffset = 0;
	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_SKILLPT_CHANGE);
	CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t) iValue);
	CGameProcedure::s_pSocket->Send(byBuff, iOffset);

	// 스킬 포인트 업데이트..
	iSkillExtra--;
	pStrName->SetStringAsInt(iSkillExtra);
	m_iSkillInfo[0] = iSkillExtra;

	iSkillPoint++;
	pStrName2->SetStringAsInt(iSkillPoint);

	switch (iValue)
	{
		case 1:
			m_iSkillInfo[1] = iSkillPoint;
			break;

		case 2:
			m_iSkillInfo[2] = iSkillPoint;
			break;

		case 3:
			m_iSkillInfo[3] = iSkillPoint;
			break;

		case 4:
			m_iSkillInfo[4] = iSkillPoint;
			break;

		case 5:
			m_iSkillInfo[5] = iSkillPoint;
			InitIconUpdate(); // 스킬 아이콘 업데이트..
			break;

		case 6:
			m_iSkillInfo[6] = iSkillPoint;
			InitIconUpdate(); // 스킬 아이콘 업데이트..
			break;

		case 7:
			m_iSkillInfo[7] = iSkillPoint;
			InitIconUpdate(); // 스킬 아이콘 업데이트..
			break;

		case 8:
			m_iSkillInfo[8] = iSkillPoint;
			InitIconUpdate(); // 스킬 아이콘 업데이트..
			break;

		default:
			break;
	}

	SetPageInIconRegion(iCurKindOfBackup, iCurSkillPageBackup);
}

bool CUISkillTreeDlg::OnMouseWheelEvent(short delta)
{
	if (delta > 0)
		PageLeft();
	else
		PageRight();

	return true;
}

void CUISkillTreeDlg::Render()
{
	if (!m_bVisible)
		return; // 보이지 않으면 자식들을 render하지 않는다.
	__IconItemSkill* spSkill = nullptr;

	TooltipRenderDisable();

	for (UIListReverseItor itor = m_Children.rbegin(); m_Children.rend() != itor; ++itor)
	{
		CN3UIBase* pChild = (*itor);
		if ((GetState() == UI_STATE_ICON_MOVING) && (pChild->UIType() == UI_TYPE_ICON) && (CN3UIWndBase::s_sSkillSelectInfo.pSkillDoneInfo)
			&& ((CN3UIIcon*) pChild == CN3UIWndBase::s_sSkillSelectInfo.pSkillDoneInfo->pUIIcon))
			continue;
		pChild->Render();
		if ((pChild->UIType() == UI_TYPE_ICON) && (pChild->GetStyle() & UISTYLE_ICON_HIGHLIGHT) && (GetState() == UI_STATE_COMMON_NONE))
		{
			spSkill = GetHighlightIconItem((CN3UIIcon*) pChild);
			TooltipRenderEnable(spSkill);
		}
	}

	CheckButtonTooltipRenderEnable();

	if ((GetState() == UI_STATE_ICON_MOVING) && (CN3UIWndBase::s_sSkillSelectInfo.pSkillDoneInfo))
		CN3UIWndBase::s_sSkillSelectInfo.pSkillDoneInfo->pUIIcon->Render();
}

void CUISkillTreeDlg::CheckButtonTooltipRenderEnable()
{
	RECT rect[MAX_SKILL_KIND_OF] = {};

	switch (CGameBase::s_pPlayer->m_InfoBase.eNation)
	{
		case NATION_ELMORAD:
			rect[SKILL_DEF_SPECIAL0] = GetChildByID<CN3UIButton>("btn_blade0")->GetClickRect();
			rect[SKILL_DEF_SPECIAL1] = GetChildByID<CN3UIButton>("btn_blade1")->GetClickRect();
			rect[SKILL_DEF_SPECIAL2] = GetChildByID<CN3UIButton>("btn_blade2")->GetClickRect();
			rect[SKILL_DEF_SPECIAL3] = GetChildByID<CN3UIButton>("btn_master")->GetClickRect();
			break;

		case NATION_KARUS:
			rect[SKILL_DEF_SPECIAL0] = GetChildByID<CN3UIButton>("btn_berserker0")->GetClickRect();
			rect[SKILL_DEF_SPECIAL1] = GetChildByID<CN3UIButton>("btn_berserker1")->GetClickRect();
			rect[SKILL_DEF_SPECIAL2] = GetChildByID<CN3UIButton>("btn_berserker2")->GetClickRect();
			rect[SKILL_DEF_SPECIAL3] = GetChildByID<CN3UIButton>("btn_master")->GetClickRect();
			break;

		default:
			break;
	}

	POINT ptCur = CGameProcedure::s_pLocalInput->MouseGetPos();
	for (int i = 0; i < MAX_SKILL_KIND_OF; i++)
	{
		if (ptCur.x > rect[i].left && ptCur.x < rect[i].right && ptCur.y > rect[i].top && ptCur.y < rect[i].bottom)
			ButtonTooltipRender(i);
	}
}

// Tool tip on hoverover of skill tabs
void CUISkillTreeDlg::ButtonTooltipRender(int iIndex)
{
	std::string szStr;
	if (!m_pStr_info->IsVisible())
		m_pStr_info->SetVisible(true);

	switch (iIndex)
	{
		// basic skill tab
		case SKILL_DEF_BASIC:
			szStr = fmt::format_text_resource(IDS_SKILL_INFO_BASE);
			break;

		// first skill tab
		case SKILL_DEF_SPECIAL0:
			switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
			{
				case CLASS_EL_BLADE:
				case CLASS_EL_PROTECTOR:
				case CLASS_KA_BERSERKER:
				case CLASS_KA_GUARDIAN:
					szStr = fmt::format_text_resource(IDS_SKILL_INFO_BLADE0);
					break;

				case CLASS_EL_RANGER:
				case CLASS_EL_ASSASIN:
				case CLASS_KA_HUNTER:
				case CLASS_KA_PENETRATOR:
					szStr = fmt::format_text_resource(IDS_SKILL_INFO_RANGER0);
					break;

				case CLASS_EL_CLERIC:
				case CLASS_EL_DRUID:
				case CLASS_KA_SHAMAN:
				case CLASS_KA_DARKPRIEST:
					szStr = fmt::format_text_resource(IDS_SKILL_INFO_CLERIC0);
					break;

				case CLASS_EL_MAGE:
				case CLASS_EL_ENCHANTER:
				case CLASS_KA_SORCERER:
				case CLASS_KA_NECROMANCER:
					szStr = fmt::format_text_resource(IDS_SKILL_INFO_MAGE0);
					break;

				default:
					break;
			}
			break;

		// second skill tab
		case SKILL_DEF_SPECIAL1:
			switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
			{
				case CLASS_EL_BLADE:
				case CLASS_EL_PROTECTOR:
				case CLASS_KA_BERSERKER:
				case CLASS_KA_GUARDIAN:
					szStr = fmt::format_text_resource(IDS_SKILL_INFO_BLADE1);
					break;

				case CLASS_EL_RANGER:
				case CLASS_EL_ASSASIN:
				case CLASS_KA_HUNTER:
				case CLASS_KA_PENETRATOR:
					szStr = fmt::format_text_resource(IDS_SKILL_INFO_RANGER1);
					break;

				case CLASS_EL_CLERIC:
				case CLASS_EL_DRUID:
				case CLASS_KA_SHAMAN:
				case CLASS_KA_DARKPRIEST:
					szStr = fmt::format_text_resource(IDS_SKILL_INFO_CLERIC1);
					break;

				case CLASS_EL_MAGE:
				case CLASS_EL_ENCHANTER:
				case CLASS_KA_SORCERER:
				case CLASS_KA_NECROMANCER:
					szStr = fmt::format_text_resource(IDS_SKILL_INFO_MAGE1);
					break;

				default:
					break;
			}
			break;

		// third skill tab
		case SKILL_DEF_SPECIAL2:
			switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
			{
				case CLASS_EL_BLADE:
				case CLASS_EL_PROTECTOR:
				case CLASS_KA_BERSERKER:
				case CLASS_KA_GUARDIAN:
					szStr = fmt::format_text_resource(IDS_SKILL_INFO_BLADE2);
					break;

				case CLASS_EL_RANGER:
				case CLASS_EL_ASSASIN:
				case CLASS_KA_HUNTER:
				case CLASS_KA_PENETRATOR:
					szStr = fmt::format_text_resource(IDS_SKILL_INFO_RANGER2);
					break;

				case CLASS_EL_CLERIC:
				case CLASS_EL_DRUID:
				case CLASS_KA_SHAMAN:
				case CLASS_KA_DARKPRIEST:
					szStr = fmt::format_text_resource(IDS_SKILL_INFO_CLERIC2);
					break;

				case CLASS_EL_MAGE:
				case CLASS_EL_ENCHANTER:
				case CLASS_KA_SORCERER:
				case CLASS_KA_NECROMANCER:
					szStr = fmt::format_text_resource(IDS_SKILL_INFO_MAGE2);
					break;

				default:
					break;
			}
			break;

		// master skill tab
		case SKILL_DEF_SPECIAL3:
			switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
			{
				case CLASS_EL_PROTECTOR:
				case CLASS_KA_GUARDIAN:
					szStr = fmt::format_text_resource(IDS_SKILL_INFO_BLADE3);
					break;

				case CLASS_EL_ASSASIN:
				case CLASS_KA_PENETRATOR:
					szStr = fmt::format_text_resource(IDS_SKILL_INFO_RANGER3);
					break;

				case CLASS_EL_DRUID:
				case CLASS_KA_DARKPRIEST:
					szStr = fmt::format_text_resource(IDS_SKILL_INFO_CLERIC3);
					break;

				case CLASS_EL_ENCHANTER:
				case CLASS_KA_NECROMANCER:
					szStr = fmt::format_text_resource(IDS_SKILL_INFO_MAGE3);
					break;

				default:
					break;
			}
			break;

		default:
			break;
	}

	m_pStr_info->SetString(szStr);
	m_pStr_info->Render();
}

// Render skill tooltip on skill hover
void CUISkillTreeDlg::TooltipRenderEnable(__IconItemSkill* spSkill)
{
	if (spSkill == nullptr || spSkill->pSkill == nullptr)
		return;

	std::string szStr;

	// Tooltip - skill description
	if (!m_pStr_info->IsVisible())
		m_pStr_info->SetVisible(true);

	m_pStr_info->SetString(spSkill->pSkill->szDesc);

	// Tooltip - MP consumed
	if (!m_pStr_skill_mp->IsVisible())
		m_pStr_skill_mp->SetVisible(true);

	if (spSkill->pSkill->iExhaustMSP == 0)
		szStr = fmt::format_text_resource(IDS_SKILL_TOOLTIP_NO_MANA);
	else
		szStr = fmt::format_text_resource(IDS_SKILL_TOOLTIP_USE_MANA, spSkill->pSkill->iExhaustMSP);

	m_pStr_skill_mp->SetString(szStr);
	szStr.clear();

	// Tooltip - Required level (basic job) or skill points (2nd job and master)
	if (!m_pStr_skill_point->IsVisible())
		m_pStr_skill_point->SetVisible(true);

	// Basic skills
	if ((spSkill->pSkill->iNeedSkill % 10) == 0)
		szStr = fmt::format_text_resource(IDS_SKILL_TOOLTIP_NEED_LEVEL, spSkill->pSkill->iNeedLevel);
	// 2nd job and master skills
	else
		szStr = fmt::format_text_resource(IDS_SKILL_TOOLTIP_NEED_SKILL_PT, spSkill->pSkill->iNeedLevel);

	m_pStr_skill_point->SetString(szStr);
	szStr.clear();

	// Tooltip - required item (e.g. weapon)
	if (!m_pStr_skill_item0->IsVisible())
		m_pStr_skill_item0->SetVisible(true);

	switch (spSkill->pSkill->dwNeedItem)
	{
		case 0:
			szStr = fmt::format_text_resource(IDS_SKILL_TOOLTIP_NEED_ITEM_ID1);
			break;
		case 1:
			szStr = fmt::format_text_resource(IDS_SKILL_TOOLTIP_NEED_ITEM_ID2);
			break;
		case 2:
			szStr = fmt::format_text_resource(IDS_SKILL_TOOLTIP_NEED_ITEM_ID3);
			break;
		case 3:
			szStr = fmt::format_text_resource(IDS_SKILL_TOOLTIP_NEED_ITEM_ID4);
			break;
		case 4:
			szStr = fmt::format_text_resource(IDS_SKILL_TOOLTIP_NEED_ITEM_ID5);
			break;
		case 5:
			szStr = fmt::format_text_resource(IDS_SKILL_TOOLTIP_NEED_ITEM_ID6);
			break;
		case 6:
			szStr = fmt::format_text_resource(IDS_SKILL_TOOLTIP_NEED_ITEM_ID7);
			break;
		case 7:
			szStr = fmt::format_text_resource(IDS_SKILL_TOOLTIP_NEED_ITEM_ID8);
			break;
		case 8:
			szStr = fmt::format_text_resource(IDS_SKILL_TOOLTIP_NEED_ITEM_ID9);
			break;
		case 10:
			szStr = fmt::format_text_resource(IDS_SKILL_TOOLTIP_NEED_ITEM_ID10);
			break;
		case 11:
			szStr = fmt::format_text_resource(IDS_SKILL_TOOLTIP_NEED_ITEM_ID11);
			break;
		case 12:
			szStr = fmt::format_text_resource(IDS_SKILL_TOOLTIP_NEED_ITEM_ID12);
			break;
		case 13:
			szStr = fmt::format_text_resource(IDS_SKILL_TOOLTIP_NEED_ITEM_ID13);
			break;
		case 21:
			szStr = fmt::format_text_resource(IDS_SKILL_TOOLTIP_NEED_ITEM_ID14);
			break;
		case 22:
			szStr = fmt::format_text_resource(IDS_SKILL_TOOLTIP_NEED_ITEM_ID15);
			break;
		case 23:
			szStr = fmt::format_text_resource(IDS_SKILL_TOOLTIP_NEED_ITEM_ID16);
			break;
		case 24:
			szStr = fmt::format_text_resource(IDS_SKILL_TOOLTIP_NEED_ITEM_ID17);
			break;
		default:
			szStr = fmt::format_text_resource(IDS_SKILL_TOOLTIP_NEED_ITEM_NO);
			break;
	}

	m_pStr_skill_item0->SetString(szStr);
	szStr.clear();

	// Tooltip - item required (e.g. scroll or arrows)
	if (!m_pStr_skill_item1->IsVisible())
		m_pStr_skill_item1->SetVisible(true);

	if (spSkill->pSkill->dwExhaustItem != 0)
	{
		__TABLE_ITEM_BASIC* pItem = CGameBase::s_pTbl_Items_Basic.Find(spSkill->pSkill->dwExhaustItem);
		__ASSERT(pItem != nullptr, "NULL Item!!!");

		if (pItem != nullptr)
			szStr = fmt::format_text_resource(IDS_SKILL_TOOLTIP_ITEM_NEED, pItem->szName);
	}
	else
	{
		szStr = fmt::format_text_resource(IDS_SKILL_TOOLTIP_ITEM_NO);
	}

	m_pStr_skill_item1->SetString(szStr);
	szStr.clear();

	// Tooltip - item consumed
	if (!m_pStr_skill_item2->IsVisible())
		m_pStr_skill_item2->SetVisible(true);
	uint32_t requiredItemID = spSkill->pSkill->dwExhaustItem;
	uint32_t consumedItemID = 0;

	switch (requiredItemID)
	{
		case ITEM_ID_MASTER_SCROLL_WARRIOR:
			consumedItemID = ITEM_ID_STONE_OF_WARRIOR;
			break;

		case ITEM_ID_MASTER_SCROLL_ROGUE:
			consumedItemID = ITEM_ID_STONE_OF_ROGUE;
			break;

		case ITEM_ID_MASTER_SCROLL_MAGE:
			consumedItemID = ITEM_ID_STONE_OF_MAGE;
			break;

		case ITEM_ID_MASTER_SCROLL_PRIEST:
			consumedItemID = ITEM_ID_STONE_OF_PRIEST;
			break;

		default:
			break;
	}

	if (consumedItemID != 0)
	{
		__TABLE_ITEM_BASIC* pItem = CGameBase::s_pTbl_Items_Basic.Find(consumedItemID);
		__ASSERT(pItem != nullptr, "NULL Item!!!");

		if (pItem != nullptr)
			szStr = fmt::format_text_resource(IDS_SKILL_TOOLTIP_USE_ITEM_EXIST, pItem->szName);
	}
	else
	{
		szStr = fmt::format_text_resource(IDS_SKILL_TOOLTIP_USE_ITEM_NO);
	}

	m_pStr_skill_item2->SetString(szStr);
	szStr.clear();

	m_pStr_info->Render();
	m_pStr_skill_mp->Render();
	m_pStr_skill_point->Render();
	m_pStr_skill_item0->Render();
	m_pStr_skill_item1->Render();
	m_pStr_skill_item2->Render();
}

void CUISkillTreeDlg::TooltipRenderDisable()
{
	m_pStr_info->SetVisible(false);
	m_pStr_skill_mp->SetVisible(false);
	m_pStr_skill_point->SetVisible(false);
	m_pStr_skill_item0->SetVisible(false);
	m_pStr_skill_item1->SetVisible(false);
	m_pStr_skill_item2->SetVisible(false);
}

void CUISkillTreeDlg::InitIconWnd(e_UIWND eWnd)
{
	CN3UIWndBase::InitIconWnd(eWnd);

	N3_VERIFY_UI_COMPONENT(m_pStr_info, GetChildByID<CN3UIString>("string_info"));
	N3_VERIFY_UI_COMPONENT(m_pStr_skill_mp, GetChildByID<CN3UIString>("string_skill_mp"));
	N3_VERIFY_UI_COMPONENT(m_pStr_skill_point, GetChildByID<CN3UIString>("string_skill_point"));
	N3_VERIFY_UI_COMPONENT(m_pStr_skill_item0, GetChildByID<CN3UIString>("string_skill_item0"));
	N3_VERIFY_UI_COMPONENT(m_pStr_skill_item1, GetChildByID<CN3UIString>("string_skill_item1"));
	N3_VERIFY_UI_COMPONENT(m_pStr_skill_item2, GetChildByID<CN3UIString>("string_skill_item2"));
}

void CUISkillTreeDlg::InitIconUpdate()
{
	int iDivide                = 0;
	__TABLE_UPC_SKILL* pUSkill = nullptr;

	// 기존 아이콘 모두 클리어..
	for (int i = 0; i < MAX_SKILL_KIND_OF; i++)
	{
		for (int j = 0; j < MAX_SKILL_PAGE_NUM; j++)
		{
			for (int k = 0; k < MAX_SKILL_IN_PAGE; k++)
			{
				if (m_pMySkillTree[i][j][k] == nullptr)
					continue;

				__IconItemSkill* spSkill = m_pMySkillTree[i][j][k];

				// 매니저에서 제거..
				RemoveChild(spSkill->pUIIcon);

				// 리소스 제거..
				spSkill->pUIIcon->Release();
				delete spSkill->pUIIcon;
				spSkill->pUIIcon = nullptr;
				delete spSkill;
				spSkill                 = nullptr;
				m_pMySkillTree[i][j][k] = nullptr;
			}
		}
	}

	// 아이디 = 직업 코드*1000 + 001부터.. (직업 코드+1)*100 + 001까지..
	int iSkillIDFirst = 0, iSkillIndexFirst = 0, iSkillIndexLast = 0, iModulo = 0;
	iSkillIDFirst   = CGameBase::s_pPlayer->m_InfoBase.eClass * 1000 + 1;
	iSkillIndexLast = CGameBase::s_pTbl_Skill.GetSize();

	if (!CGameBase::s_pTbl_Skill.IDToIndex(iSkillIDFirst, &iSkillIndexFirst))
	{
		PageButtonInitialize();
		return; // 첫번째 스킬이 없으면.. 안된다..
	}

	if (CGameBase::s_pPlayer->m_InfoBase.eClass <= CLASS_EL_DRUID)
	{
		for (int i = iSkillIndexFirst; i < CGameBase::s_pTbl_Skill.GetSize(); i++)
		{
			pUSkill = CGameBase::s_pTbl_Skill.GetIndexedData(i);
			iDivide = pUSkill->dwID / 1000;
			if (iDivide != (iSkillIDFirst / 1000))
			{
				iSkillIndexLast = i;
				break;
			}
		}
	}

	for (int i = iSkillIndexFirst; i < iSkillIndexLast; i++)
	{
		__TABLE_UPC_SKILL* pUSkill = CGameBase::s_pTbl_Skill.GetIndexedData(i);
		if (pUSkill == nullptr)
			continue;
		if (pUSkill->dwID >= UIITEM_TYPE_USABLE_ID_MIN)
			continue;

		// 조건이 충족 되는지 확인한다..
		iModulo = pUSkill->iNeedSkill % 10;
		switch (iModulo)
		{
			case 0:                                                                 // Basic Skills..
				if (pUSkill->iNeedLevel <= CGameBase::s_pPlayer->m_InfoBase.iLevel) // 내 레벨보다 같거나 작으면..
					AddSkillToPage(pUSkill);
				else
					AddSkillToPage(pUSkill, 0, false);
				break;

			case 5: // First Skill Tab..
				if (pUSkill->iNeedLevel <= m_iSkillInfo[5])
					AddSkillToPage(pUSkill, 1);
				else
					AddSkillToPage(pUSkill, 1, false);
				break;

			case 6: // Second Skill Tab..
				if (pUSkill->iNeedLevel <= m_iSkillInfo[6])
					AddSkillToPage(pUSkill, 2);
				else
					AddSkillToPage(pUSkill, 2, false);
				break;

			case 7: // Third Skill Tab..
				if (pUSkill->iNeedLevel <= m_iSkillInfo[7])
					AddSkillToPage(pUSkill, 3);
				else
					AddSkillToPage(pUSkill, 3, false);
				break;

			case 8: // Master Skill Tab..
				if (pUSkill->iNeedLevel <= m_iSkillInfo[8])
					AddSkillToPage(pUSkill, 4);
				else
					AddSkillToPage(pUSkill, 4, false);
				break;

			default:
				break;
		}
	}

	PageButtonInitialize();
}

void CUISkillTreeDlg::PageButtonInitialize()
{
	SetPageInIconRegion(0, 0);
	SetPageInCharRegion();

	// 서버에게 받은 값으로 세팅.. m_iSkillInfo[MAX_SKILL_FROM_SERVER];										// 서버로 받는 슬롯 정보..
	CN3UIString* pStrName = nullptr;
	N3_VERIFY_UI_COMPONENT(pStrName, GetChildByID<CN3UIString>("string_skillpoint"));
	if (pStrName != nullptr)
		pStrName->SetStringAsInt(m_iSkillInfo[0]);

	N3_VERIFY_UI_COMPONENT(pStrName, GetChildByID<CN3UIString>("string_0"));
	if (pStrName != nullptr)
		pStrName->SetStringAsInt(m_iSkillInfo[1]);

	N3_VERIFY_UI_COMPONENT(pStrName, GetChildByID<CN3UIString>("string_1"));
	if (pStrName != nullptr)
		pStrName->SetStringAsInt(m_iSkillInfo[2]);

	N3_VERIFY_UI_COMPONENT(pStrName, GetChildByID<CN3UIString>("string_2"));
	if (pStrName != nullptr)
		pStrName->SetStringAsInt(m_iSkillInfo[3]);

	N3_VERIFY_UI_COMPONENT(pStrName, GetChildByID<CN3UIString>("string_3"));
	if (pStrName != nullptr)
		pStrName->SetStringAsInt(m_iSkillInfo[4]);

	N3_VERIFY_UI_COMPONENT(pStrName, GetChildByID<CN3UIString>("string_4"));
	if (pStrName != nullptr)
		pStrName->SetStringAsInt(m_iSkillInfo[5]);

	N3_VERIFY_UI_COMPONENT(pStrName, GetChildByID<CN3UIString>("string_5"));
	if (pStrName != nullptr)
		pStrName->SetStringAsInt(m_iSkillInfo[6]);

	N3_VERIFY_UI_COMPONENT(pStrName, GetChildByID<CN3UIString>("string_6"));
	if (pStrName != nullptr)
		pStrName->SetStringAsInt(m_iSkillInfo[7]);

	// N3_VERIFY_UI_COMPONENT(pStrName, GetChildByID<CN3UIString>("string_7"));
	// if (pStrName != nullptr)
	//	pStrName->SetStringAsInt(m_iSkillInfo[8]);

	ButtonVisibleStateSet();
}

void CUISkillTreeDlg::ButtonVisibleStateSet()
{
// temp macro..
#define ASSET_0                                    \
	{                                              \
		if (!pButton)                              \
			return;                                \
		pButton->SetVisible(false);                \
		pButton->SetState(UI_STATE_BUTTON_NORMAL); \
	}
#define ASSET_1                                      \
	{                                                \
		if (!pButton)                                \
			return;                                  \
		pButton->SetVisible(true);                   \
		if (m_iCurKindOf == 1)                       \
			pButton->SetState(UI_STATE_BUTTON_DOWN); \
	}
#define ASSET_2                                      \
	{                                                \
		if (!pButton)                                \
			return;                                  \
		pButton->SetVisible(true);                   \
		if (m_iCurKindOf == 2)                       \
			pButton->SetState(UI_STATE_BUTTON_DOWN); \
	}
#define ASSET_3                                      \
	{                                                \
		if (!pButton)                                \
			return;                                  \
		pButton->SetVisible(true);                   \
		if (m_iCurKindOf == 3)                       \
			pButton->SetState(UI_STATE_BUTTON_DOWN); \
	}
#define ASSET_4                                      \
	{                                                \
		if (!pButton)                                \
			return;                                  \
		pButton->SetVisible(true);                   \
		if (m_iCurKindOf == 4)                       \
			pButton->SetState(UI_STATE_BUTTON_DOWN); \
	}

	CN3UIButton* pButton = nullptr;

	N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_public"));
	if (pButton != nullptr)
		pButton->SetState(UI_STATE_BUTTON_NORMAL);

	// Hide all existing buttons by default.
	N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_master"));
	if (pButton != nullptr)
		pButton->SetVisible(false);

	switch (CGameBase::s_pPlayer->m_InfoBase.eNation)
	{
		case NATION_ELMORAD:
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_ranger0"));
			ASSET_0;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_ranger1"));
			ASSET_0;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_ranger2"));
			ASSET_0;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_blade0"));
			ASSET_0;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_blade1"));
			ASSET_0;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_blade2"));
			ASSET_0;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_mage0"));
			ASSET_0;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_mage1"));
			ASSET_0;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_mage2"));
			ASSET_0;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_cleric0"));
			ASSET_0;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_cleric1"));
			ASSET_0;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_cleric2"));
			ASSET_0;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_master"));
			ASSET_0;
			break;

		case NATION_KARUS:
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_hunter0"));
			ASSET_0;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_hunter1"));
			ASSET_0;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_hunter2"));
			ASSET_0;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_hunter3"));
			ASSET_0;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_berserker0"));
			ASSET_0;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_berserker1"));
			ASSET_0;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_berserker2"));
			ASSET_0;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_berserker3"));
			ASSET_0;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_sorcerer0"));
			ASSET_0;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_sorcerer1"));
			ASSET_0;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_sorcerer2"));
			ASSET_0;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_sorcerer3"));
			ASSET_0;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_shaman0"));
			ASSET_0;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_shaman1"));
			ASSET_0;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_shaman2"));
			ASSET_0;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_shaman3"));
			ASSET_0;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_master"));
			ASSET_0;
			break;

		default:
			break;
	}

	if (m_iCurKindOf == 0)
	{
		N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_public"));
		pButton->SetState(UI_STATE_BUTTON_DOWN);
	}

	switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
	{
		case CLASS_KA_BERSERKER:
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_berserker0"));
			ASSET_1;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_berserker1"));
			ASSET_2;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_berserker2"));
			ASSET_3;
			break;

		case CLASS_KA_GUARDIAN:
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_berserker0"));
			ASSET_1;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_berserker1"));
			ASSET_2;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_berserker2"));
			ASSET_3;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_master"));
			ASSET_4;
			break;

		case CLASS_KA_HUNTER:
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_hunter0"));
			ASSET_1;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_hunter1"));
			ASSET_2;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_hunter2"));
			ASSET_3;
			break;

		case CLASS_KA_PENETRATOR:
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_hunter0"));
			ASSET_1;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_hunter1"));
			ASSET_2;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_hunter2"));
			ASSET_3;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_master"));
			ASSET_4;
			break;

		case CLASS_KA_SHAMAN:
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_shaman0"));
			ASSET_1;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_shaman1"));
			ASSET_2;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_shaman2"));
			ASSET_3;
			break;

		case CLASS_KA_DARKPRIEST:
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_shaman0"));
			ASSET_1;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_shaman1"));
			ASSET_2;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_shaman2"));
			ASSET_3;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_master"));
			ASSET_4;
			break;

		case CLASS_KA_SORCERER:
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_sorcerer0"));
			ASSET_1;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_sorcerer1"));
			ASSET_2;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_sorcerer2"));
			ASSET_3;
			break;

		case CLASS_KA_NECROMANCER:
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_sorcerer0"));
			ASSET_1;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_sorcerer1"));
			ASSET_2;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_sorcerer2"));
			ASSET_3;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_master"));
			ASSET_4;
			break;

		case CLASS_EL_BLADE:
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_blade0"));
			ASSET_1;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_blade1"));
			ASSET_2;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_blade2"));
			ASSET_3;
			break;

		case CLASS_EL_PROTECTOR:
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_blade0"));
			ASSET_1;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_blade1"));
			ASSET_2;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_blade2"));
			ASSET_3;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_master"));
			ASSET_4;
			break;

		case CLASS_EL_RANGER:
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_ranger0"));
			ASSET_1;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_ranger1"));
			ASSET_2;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_ranger2"));
			ASSET_3;
			break;

		case CLASS_EL_ASSASIN:
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_ranger0"));
			ASSET_1;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_ranger1"));
			ASSET_2;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_ranger2"));
			ASSET_3;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_master"));
			ASSET_4;
			break;

		case CLASS_EL_MAGE:
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_mage0"));
			ASSET_1;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_mage1"));
			ASSET_2;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_mage2"));
			ASSET_3;
			break;

		case CLASS_EL_ENCHANTER:
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_mage0"));
			ASSET_1;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_mage1"));
			ASSET_2;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_mage2"));
			ASSET_3;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_master"));
			ASSET_4;
			break;

		case CLASS_EL_CLERIC:
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_cleric0"));
			ASSET_1;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_cleric1"));
			ASSET_2;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_cleric2"));
			ASSET_3;
			break;

		case CLASS_EL_DRUID:
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_cleric0"));
			ASSET_1;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_cleric1"));
			ASSET_2;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_cleric2"));
			ASSET_3;
			N3_VERIFY_UI_COMPONENT(pButton, GetChildByID<CN3UIButton>("btn_master"));
			ASSET_4;
			break;

		default:
			break;
	}
}

std::optional<std::pair<int, int>> CUISkillTreeDlg::FindSlotForSkill(const __TABLE_UPC_SKILL* pUSkill, int iCategoryOffset /*= 0*/) const
{
	// Find an existing skill icon to replace
	for (int i = 0; i < MAX_SKILL_PAGE_NUM; i++)
	{
		for (int j = 0; j < MAX_SKILL_IN_PAGE; j++)
		{
			if (m_pMySkillTree[iCategoryOffset][i][j] != nullptr && m_pMySkillTree[iCategoryOffset][i][j]->pSkill->dwID == pUSkill->dwID)
				return std::make_pair(i, j);
		}
	}

	// No existing skill icon exists, find an empty slot.
	for (int i = 0; i < MAX_SKILL_PAGE_NUM; i++)
	{
		for (int j = 0; j < MAX_SKILL_IN_PAGE; j++)
		{
			if (m_pMySkillTree[iCategoryOffset][i][j] == nullptr)
				return std::make_pair(i, j);
		}
	}

	return std::nullopt;
}

void CUISkillTreeDlg::AddSkillToPage(__TABLE_UPC_SKILL* pUSkill, int iCategoryOffset, bool bHasLevelToUse)
{
	auto slot = FindSlotForSkill(pUSkill, iCategoryOffset);
	if (!slot.has_value())
		return;

	auto [i, j]              = slot.value();

	__IconItemSkill* spSkill = new __IconItemSkill();
	spSkill->pSkill          = pUSkill;

	// 아이콘 이름 만들기.. ^^
	if (bHasLevelToUse)
		spSkill->szIconFN = fmt::format("UI\\skillicon_{:02}_{}.dxt", pUSkill->dwID % 100, pUSkill->dwID / 100);
	else
		spSkill->szIconFN = "UI\\skillicon_enigma.dxt";

	// 아이콘 로드하기.. ^^
	spSkill->pUIIcon = new CN3UIIcon;
	spSkill->pUIIcon->Init(this);
	spSkill->pUIIcon->SetTex(spSkill->szIconFN);
	spSkill->pUIIcon->SetUVRect(0, 0, 1, 1);
	spSkill->pUIIcon->SetUIType(UI_TYPE_ICON);
	spSkill->pUIIcon->SetStyle(UISTYLE_ICON_SKILL);

	CN3UIArea* pArea = GetChildAreaByiOrder(UI_AREA_TYPE_SKILL_TREE, j);
	if (pArea != nullptr)
	{
		spSkill->pUIIcon->SetRegion(pArea->GetRegion());
		spSkill->pUIIcon->SetMoveRect(pArea->GetRegion());
	}

	// 아이콘 정보 저장..
	m_pMySkillTree[iCategoryOffset][i][j] = spSkill;
}

bool CUISkillTreeDlg::IsSkillUsable(const __TABLE_UPC_SKILL* pUSkill) const
{
	int iModulo = pUSkill->iNeedSkill % 10;
	switch (iModulo)
	{
		case 0: // Basic Skills..
			return pUSkill->iNeedLevel <= CGameBase::s_pPlayer->m_InfoBase.iLevel;

		case 5: // First Skill Tab..
			return pUSkill->iNeedLevel <= m_iSkillInfo[5];

		case 6: // Second Skill Tab..
			return pUSkill->iNeedLevel <= m_iSkillInfo[6];

		case 7: // Third Skill Tab..
			return pUSkill->iNeedLevel <= m_iSkillInfo[7];

		case 8: // Master Skill Tab..
			return pUSkill->iNeedLevel <= m_iSkillInfo[8];

		default:
			break;
	}

	return false;
}

void CUISkillTreeDlg::Open()
{
	// 스르륵 열린다!!
	SetVisible(true);
	this->SetPos(CN3Base::s_CameraData.vp.Width, 10);
	m_fMoveDelta    = 0;
	m_bOpenningNow  = true;
	m_bClosingNow   = false;

	m_iRBtnDownOffs = -1;
}

void CUISkillTreeDlg::Close()
{
	// 리소스 Free..
	__IconItemSkill* spSkill = nullptr;
	spSkill                  = CN3UIWndBase::s_sSkillSelectInfo.pSkillDoneInfo;
	if (spSkill)
	{
		// 매니저에서 제거..
		RemoveChild(spSkill->pUIIcon);

		// 리소스 제거..
		spSkill->pUIIcon->Release();
		delete spSkill->pUIIcon;
		spSkill->pUIIcon = nullptr;
		delete spSkill;
		spSkill = nullptr;
	}
	CN3UIWndBase::s_sSkillSelectInfo.pSkillDoneInfo = nullptr;
	SetState(UI_STATE_COMMON_NONE);
	CN3UIWndBase::AllHighLightIconFree();

	// 스르륵 닫힌다..!!
	//	SetVisible(false); // 다 닫히고 나서 해준다..
	RECT rc = this->GetRegion();
	this->SetPos(CN3Base::s_CameraData.vp.Width - (rc.right - rc.left), 10);
	m_fMoveDelta   = 0;
	m_bOpenningNow = false;
	m_bClosingNow  = true;

	if (m_pSnd_CloseUI)
		m_pSnd_CloseUI->Play(); // 닫는 소리..

	m_iRBtnDownOffs = -1;
}

__IconItemSkill* CUISkillTreeDlg::GetHighlightIconItem(CN3UIIcon* pUIIcon)
{
	for (int k = 0; k < MAX_SKILL_IN_PAGE; k++)
	{
		if ((m_pMySkillTree[m_iCurKindOf][m_iCurSkillPage][k] != nullptr)
			&& (m_pMySkillTree[m_iCurKindOf][m_iCurSkillPage][k]->pUIIcon == pUIIcon))
			return m_pMySkillTree[m_iCurKindOf][m_iCurSkillPage][k];
	}

	return nullptr;
}

int CUISkillTreeDlg::GetSkilliOrder(__IconItemSkill* spSkill)
{
	for (int k = 0; k < MAX_SKILL_IN_PAGE; k++)
	{
		if (m_pMySkillTree[m_iCurKindOf][m_iCurSkillPage][k] == spSkill)
			return k;
	}

	return -1;
}

RECT CUISkillTreeDlg::GetSampleRect()
{
	CN3UIArea* pArea = GetChildAreaByiOrder(UI_AREA_TYPE_SKILL_TREE, 0);
	if (pArea == nullptr)
		return {};

	POINT ptCur    = CGameProcedure::s_pLocalInput->MouseGetPos();
	RECT rect      = pArea->GetRegion();
	float fWidth   = (float) (rect.right - rect.left);
	float fHeight  = (float) (rect.bottom - rect.top);
	fWidth        *= 0.5f;
	fHeight       *= 0.5f;
	rect.left      = ptCur.x - (int) fWidth;
	rect.right     = ptCur.x + (int) fWidth;
	rect.top       = ptCur.y - (int) fHeight;
	rect.bottom    = ptCur.y + (int) fHeight;
	return rect;
}

void CUISkillTreeDlg::SetPageInIconRegion(int iKindOf, int iPageNum) // 아이콘 역역에서 현재 페이지 설정..
{
	if ((iKindOf >= MAX_SKILL_KIND_OF) || (iPageNum >= MAX_SKILL_PAGE_NUM))
		return;

	m_iCurKindOf    = iKindOf;
	m_iCurSkillPage = iPageNum;

	for (int i = 0; i < MAX_SKILL_KIND_OF; i++)
	{
		if (i != iKindOf)
		{
			for (int j = 0; j < MAX_SKILL_PAGE_NUM; j++)
			{
				for (int k = 0; k < MAX_SKILL_IN_PAGE; k++)
				{
					if (m_pMySkillTree[i][j][k] != nullptr)
						m_pMySkillTree[i][j][k]->pUIIcon->SetVisible(false);
				}
			}
		}
		else
		{
			for (int j = 0; j < MAX_SKILL_PAGE_NUM; j++)
			{
				if (j != iPageNum)
				{
					for (int k = 0; k < MAX_SKILL_IN_PAGE; k++)
					{
						if (m_pMySkillTree[i][j][k] != nullptr)
							m_pMySkillTree[i][j][k]->pUIIcon->SetVisible(false);
					}
				}
				else
				{
					for (int k = 0; k < MAX_SKILL_IN_PAGE; k++)
					{
						if (m_pMySkillTree[i][j][k] != nullptr)
							m_pMySkillTree[i][j][k]->pUIIcon->SetVisible(true);
					}
				}
			}
		}
	}

	// 아이콘 설명 문자열 업데이트.. 현재 스킬 종류와 현재 스킬 페이지중 아이콘이 보이면 String보이게.. 아니면 안보이게..
	CN3UIString* pStrName = nullptr;
	std::string str;

	for (int k = 0; k < MAX_SKILL_IN_PAGE; k++)
	{
		if (m_pMySkillTree[m_iCurKindOf][m_iCurSkillPage][k] != nullptr)
		{
			str = "string_list_" + std::to_string(k);
			N3_VERIFY_UI_COMPONENT(pStrName, GetChildByID<CN3UIString>(str));
			pStrName->SetString(m_pMySkillTree[m_iCurKindOf][m_iCurSkillPage][k]->pSkill->szName);
			pStrName->SetVisible(true);
		}
		else
		{
			str = "string_list_" + std::to_string(k);
			N3_VERIFY_UI_COMPONENT(pStrName, GetChildByID<CN3UIString>(str));
			pStrName->SetVisible(false);
		}
	}

	ButtonVisibleStateSet();

	CN3UIString* pStr = nullptr;
	N3_VERIFY_UI_COMPONENT(pStr, GetChildByID<CN3UIString>("string_page"));
	if (pStr != nullptr)
		pStr->SetStringAsInt(iPageNum + 1);
}

void CUISkillTreeDlg::AllClearImageByName(std::string_view svHeaderID, bool bVisible, std::string_view svCategoryID)
{
	CN3UIBase* pBase  = nullptr;
	std::string str   = "img_";
	str              += svHeaderID;
	pBase             = GetChildByID(str);
	if (pBase != nullptr)
		pBase->SetVisible(bVisible);

	// If a category ID is not set, assume the same as the header ID.
	if (svCategoryID.empty())
		svCategoryID = svHeaderID;

	for (int i = 0; i < 3; i++)
	{
		str    = "img_";
		str   += svCategoryID;
		str   += "_" + std::to_string(i);
		pBase  = GetChildByID(str);
		if (pBase != nullptr)
			pBase->SetVisible(bVisible);
	}
}

// 문자 역역에서 현재 페이지 설정..
void CUISkillTreeDlg::SetPageInCharRegion()
{
	AllClearImageByName("public", false);

	switch (CGameBase::s_pPlayer->m_InfoBase.eNation)
	{
		case NATION_KARUS:
			AllClearImageByName("berserker", false);
			AllClearImageByName("Berserker Hero", false);
			AllClearImageByName("hunter", false);
			AllClearImageByName("Shadow Bane", false);
			AllClearImageByName("sorcerer", false);
			AllClearImageByName("Elemental Lord", false);
			AllClearImageByName("shaman", false);
			AllClearImageByName("Shadow Knight", false);

			switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
			{
				case CLASS_KA_WARRIOR:
				case CLASS_KA_ROGUE:
				case CLASS_KA_WIZARD:
				case CLASS_KA_PRIEST:
					AllClearImageByName("public", true);
					break;

				case CLASS_KA_BERSERKER:
					AllClearImageByName("berserker", true);
					break;

				case CLASS_KA_HUNTER:
					AllClearImageByName("hunter", true);
					break;

				case CLASS_KA_SORCERER:
					AllClearImageByName("sorcerer", true);
					break;

				case CLASS_KA_SHAMAN:
					AllClearImageByName("shaman", true);
					break;

				case CLASS_KA_GUARDIAN:
					AllClearImageByName("Berserker Hero", true, "berserker");
					break;

				case CLASS_KA_PENETRATOR:
					AllClearImageByName("Shadow Bane", true, "hunter");
					break;

				case CLASS_KA_NECROMANCER:
					AllClearImageByName("Elemental Lord", true, "sorcerer");
					break;

				case CLASS_KA_DARKPRIEST:
					AllClearImageByName("Shadow Knight", true, "shaman");
					break;

				default:
					break;
			}
			break;

		case NATION_ELMORAD:
			AllClearImageByName("blade", false);
			AllClearImageByName("Blade Master", false);
			AllClearImageByName("ranger", false);
			AllClearImageByName("Kasar Hood", false);
			AllClearImageByName("mage", false);
			AllClearImageByName("Arc Mage", false);
			AllClearImageByName("cleric", false);
			AllClearImageByName("Paladin", false);

			switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
			{
				case CLASS_EL_WARRIOR:
				case CLASS_EL_ROGUE:
				case CLASS_EL_WIZARD:
				case CLASS_EL_PRIEST:
					AllClearImageByName("public", true);
					break;

				case CLASS_EL_BLADE:
					AllClearImageByName("blade", true);
					break;

				case CLASS_EL_RANGER:
					AllClearImageByName("ranger", true);
					break;

				case CLASS_EL_MAGE:
					AllClearImageByName("mage", true);
					break;

				case CLASS_EL_CLERIC:
					AllClearImageByName("cleric", true);
					break;

				case CLASS_EL_PROTECTOR:
					AllClearImageByName("Blade Master", true, "blade");
					break;

				case CLASS_EL_ASSASIN:
					AllClearImageByName("Kasar Hood", true, "ranger");
					break;

				case CLASS_EL_ENCHANTER:
					AllClearImageByName("Arc Mage", true, "mage");
					break;

				case CLASS_EL_DRUID:
					AllClearImageByName("Paladin", true, "cleric");
					break;

				default:
					break;
			}
			break;

		default:
			break;
	}

	CN3UIBase* pImgMaster = GetChildByID("img_master");
	switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
	{
		case CLASS_KA_GUARDIAN:
		case CLASS_KA_PENETRATOR:
		case CLASS_KA_NECROMANCER:
		case CLASS_KA_DARKPRIEST:
		case CLASS_EL_PROTECTOR:
		case CLASS_EL_ASSASIN:
		case CLASS_EL_ENCHANTER:
		case CLASS_EL_DRUID:
			if (pImgMaster != nullptr)
				pImgMaster->SetVisible(true);
			break;

		default:
			if (pImgMaster != nullptr)
				pImgMaster->SetVisible(false);
			break;
	}
}

CN3UIImage* CUISkillTreeDlg::GetChildImageByName(const std::string& szID)
{
	for (CN3UIBase* pChild : m_Children)
	{
		if (pChild->UIType() == UI_TYPE_IMAGE && lstrcmpiA(szID.c_str(), pChild->m_szID.c_str()) == 0)
			return static_cast<CN3UIImage*>(pChild);
	}

	return nullptr;
}

CN3UIButton* CUISkillTreeDlg::GetChildButtonByName(const std::string& szID)
{
	for (CN3UIBase* pChild : m_Children)
	{
		if (pChild->UIType() == UI_TYPE_BUTTON && lstrcmpiA(szID.c_str(), pChild->m_szID.c_str()) == 0)
			return static_cast<CN3UIButton*>(pChild);
	}

	return nullptr;
}

//this_ui_add_start
bool CUISkillTreeDlg::OnKeyPress(int iKey)
{
	switch (iKey)
	{
		case DIK_PRIOR:
			PageLeft();
			return true;

		case DIK_NEXT:
			PageRight();
			return true;

		case DIK_ESCAPE:
			if (!m_bClosingNow)
				Close();
			return true;

		default:
			break;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

void CUISkillTreeDlg::SetVisibleWithNoSound(bool bVisible, bool bWork, bool bReFocus)
{
	CN3UIBase::SetVisibleWithNoSound(bVisible, bWork, bReFocus);

	if (bReFocus)
	{
		if (bVisible)
			CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
		else
			CGameProcedure::s_pUIMgr->ReFocusUI(); //this_ui
	}
}

void CUISkillTreeDlg::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);
	if (bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
		CGameProcedure::s_pUIMgr->ReFocusUI(); //this_ui
}
//this_ui_add_end

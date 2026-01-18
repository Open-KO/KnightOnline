// UIHotKeyDlg.cpp: implementation of the CUIHotKeyDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UIHotKeyDlg.h"
#include "LocalInput.h"
#include "GameProcMain.h"
#include "PlayerMySelf.h"
#include "UISkillTreeDlg.h"
#include "MagicSkillMng.h"
#include "UIManager.h"
#include "UIInventory.h"
#include "text_resources.h"

#include <N3Base/N3UIString.h>

#include <cmath>
#include <algorithm>
#include <format>

CUIHotKeyDlg::CUIHotKeyDlg()
{
	m_iCurPage     = 0;
	m_iSelectIndex = -1;
	m_iSelectPage  = -1;

	for (int i = 0; i < MAX_SKILL_HOTKEY_PAGE; i++)
	{
		for (int j = 0; j < MAX_SKILL_IN_HOTKEY; j++)
			m_pMyHotkey[i][j] = nullptr;
	}

	for (int j = 0; j < MAX_SKILL_IN_HOTKEY; j++)
	{
		m_pCountStr[j]   = nullptr;
		m_pTooltipStr[j] = nullptr;
	}

	m_ptOffset = {};
}

CUIHotKeyDlg::~CUIHotKeyDlg()
{
	CUIHotKeyDlg::Release();
}

void CUIHotKeyDlg::Release()
{
	for (int i = 0; i < MAX_SKILL_HOTKEY_PAGE; i++)
	{
		for (int j = 0; j < MAX_SKILL_IN_HOTKEY; j++)
		{
			if (m_pMyHotkey[i][j] != nullptr)
			{
				delete m_pMyHotkey[i][j];
				m_pMyHotkey[i][j] = nullptr;
			}
		}
	}

	for (int j = 0; j < MAX_SKILL_IN_HOTKEY; j++)
	{
		m_pCountStr[j]   = nullptr;
		m_pTooltipStr[j] = nullptr;
	}

	m_iCurPage     = 0;
	m_iSelectIndex = -1;
	m_iSelectPage  = -1;

	CN3UIBase::Release();
}

void CUIHotKeyDlg::ReleaseItem()
{
	for (int i = 0; i < MAX_SKILL_HOTKEY_PAGE; i++)
	{
		for (int j = 0; j < MAX_SKILL_IN_HOTKEY; j++)
		{
			if (m_pMyHotkey[i][j] != nullptr)
			{
				if (m_pMyHotkey[i][j]->pUIIcon)
				{
					RemoveChild(m_pMyHotkey[i][j]->pUIIcon);
					m_pMyHotkey[i][j]->pUIIcon->Release();
					delete m_pMyHotkey[i][j]->pUIIcon;
					m_pMyHotkey[i][j]->pUIIcon = nullptr;
				}

				delete m_pMyHotkey[i][j];
				m_pMyHotkey[i][j] = nullptr;
			}
		}
	}

	m_iCurPage = 0;
}

uint32_t CUIHotKeyDlg::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
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
		if (s_sSkillSelectInfo.pSkillDoneInfo != nullptr)
		{
			s_sSkillSelectInfo.pSkillDoneInfo->pUIIcon->SetRegion(GetSampleRect());
			s_sSkillSelectInfo.pSkillDoneInfo->pUIIcon->SetMoveRect(GetSampleRect());
		}
	}

	return CN3UIWndBase::MouseProc(dwFlags, ptCur, ptOld);
}

bool CUIHotKeyDlg::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	//..
	if (pSender->m_szID == "btn_up")
		PageUp();
	if (pSender->m_szID == "btn_down")
		PageDown();

	__IconItemSkill* spSkill = nullptr;

	uint32_t dwBitMask       = 0x0f0f0000;
	POINT ptCur              = CGameProcedure::s_pLocalInput->MouseGetPos();

	switch (dwMsg & dwBitMask)
	{
		case UIMSG_ICON_DOWN_FIRST:
			// Get Item..
			spSkill                                         = GetHighlightIconItem((CN3UIIcon*) pSender);

			// Save Select Info..
			CN3UIWndBase::s_sSkillSelectInfo.UIWnd          = UIWND_HOTKEY;
			CN3UIWndBase::s_sSkillSelectInfo.iOrder         = GetAreaiOrder();
			CN3UIWndBase::s_sSkillSelectInfo.pSkillDoneInfo = spSkill;

			// Calc Move Rect Offset..
			if (!CalcMoveOffset())
				SetState(UI_STATE_COMMON_NONE);
			break;

		case UIMSG_ICON_DOWN:
			if (GetState() == UI_STATE_ICON_MOVING)
			{
				s_sSkillSelectInfo.pSkillDoneInfo->pUIIcon->SetRegion(GetSampleRect());
				s_sSkillSelectInfo.pSkillDoneInfo->pUIIcon->SetMoveRect(GetSampleRect());
			}
			break;

		case UIMSG_ICON_RUP:
			// Hot Key 윈도우를 돌아 다니면서 검사..
			if (IsIn(ptCur.x, ptCur.y))
			{
				int iOrder = GetAreaiOrder();
				if (m_pMyHotkey[m_iCurPage][iOrder] != nullptr)
				{
					m_iSelectIndex = iOrder;
					m_iSelectPage  = m_iCurPage;
				}
			}
			SetState(UI_STATE_COMMON_NONE);
			break;

		case UIMSG_ICON_UP:
			// Hot Key 윈도우를 돌아 다니면서 검사..
			if (IsIn(ptCur.x, ptCur.y))
			{
				int iOrder = GetAreaiOrder();
				if (s_sSkillSelectInfo.iOrder == iOrder) // 실행..
				{
					CN3UIArea* pArea = GetChildAreaByiOrder(UI_AREA_TYPE_SKILL_HOTKEY, iOrder);
					if (pArea != nullptr)
					{
						m_pMyHotkey[m_iCurPage][iOrder]->pUIIcon->SetRegion(pArea->GetRegion());
						m_pMyHotkey[m_iCurPage][iOrder]->pUIIcon->SetMoveRect(pArea->GetRegion());
					}

					DoOperate(m_pMyHotkey[m_iCurPage][iOrder]);
				}
				else
				{
					if (iOrder == -1)
					{
						// 리소스 Free..
						spSkill = CN3UIWndBase::s_sSkillSelectInfo.pSkillDoneInfo;

						// 매니저에서 제거..
						RemoveChild(spSkill->pUIIcon);

						// 리소스 제거..
						spSkill->pUIIcon->Release();
						delete spSkill->pUIIcon;
						spSkill->pUIIcon = nullptr;
						delete spSkill;
						spSkill                                            = nullptr;
						m_pMyHotkey[m_iCurPage][s_sSkillSelectInfo.iOrder] = nullptr;
						if (m_iCurPage == m_iSelectPage && s_sSkillSelectInfo.iOrder == m_iSelectIndex)
						{
							m_iSelectPage  = -1;
							m_iSelectIndex = -1;
						}

						CloseIconRegistry();
					}
					else // 옮기기..
					{
						// 기존 아이콘이 있다면..
						if (m_pMyHotkey[m_iCurPage][iOrder])
						{
							// 기존 아이콘을 삭제한다..
							spSkill = m_pMyHotkey[m_iCurPage][iOrder];

							// 매니저에서 제거..
							RemoveChild(spSkill->pUIIcon);

							// 리소스 제거..
							spSkill->pUIIcon->Release();
							delete spSkill->pUIIcon;
							spSkill->pUIIcon = nullptr;
							delete spSkill;
							spSkill                         = nullptr;
							m_pMyHotkey[m_iCurPage][iOrder] = nullptr;
						}

						spSkill                                            = m_pMyHotkey[m_iCurPage][s_sSkillSelectInfo.iOrder];
						m_pMyHotkey[m_iCurPage][iOrder]                    = spSkill;
						m_pMyHotkey[m_iCurPage][s_sSkillSelectInfo.iOrder] = nullptr;

						if (m_iCurPage == m_iSelectPage && s_sSkillSelectInfo.iOrder == m_iSelectIndex)
						{
							m_iSelectPage  = -1;
							m_iSelectIndex = -1;
						}

						CN3UIArea* pArea = GetChildAreaByiOrder(UI_AREA_TYPE_SKILL_HOTKEY, iOrder);
						if (pArea != nullptr)
						{
							m_pMyHotkey[m_iCurPage][iOrder]->pUIIcon->SetRegion(pArea->GetRegion());
							m_pMyHotkey[m_iCurPage][iOrder]->pUIIcon->SetMoveRect(pArea->GetRegion());
						}

						CloseIconRegistry();
					}
				}
			}
			else // 삭제..
			{
				// 리소스 Free..
				spSkill = s_sSkillSelectInfo.pSkillDoneInfo;

				// 매니저에서 제거..
				RemoveChild(spSkill->pUIIcon);

				// 리소스 제거..
				spSkill->pUIIcon->Release();
				delete spSkill->pUIIcon;
				spSkill->pUIIcon = nullptr;
				delete spSkill;
				spSkill                                            = nullptr;
				m_pMyHotkey[m_iCurPage][s_sSkillSelectInfo.iOrder] = nullptr;

				if (m_iCurPage == m_iSelectPage && s_sSkillSelectInfo.iOrder == m_iSelectIndex)
				{
					m_iSelectPage  = -1;
					m_iSelectIndex = -1;
				}

				CloseIconRegistry();
			}

			s_sSkillSelectInfo.pSkillDoneInfo = nullptr;
			SetState(UI_STATE_COMMON_NONE);
			break;

		default:
			break;
	}

	return false;
}

void CUIHotKeyDlg::Render()
{
	bool bTooltipRender     = false;
	__IconItemSkill* pSkill = nullptr;

	if (!m_bVisible)
		return; // 보이지 않으면 자식들을 render하지 않는다.
	DisableTooltipDisplay();
	DisableCountStrDisplay();

	for (UIListReverseItor itor = m_Children.rbegin(); m_Children.rend() != itor; ++itor)
	{
		CN3UIBase* pChild = (*itor);
		if ((GetState() == UI_STATE_ICON_MOVING) && (pChild->UIType() == UI_TYPE_ICON) && (CN3UIWndBase::s_sSkillSelectInfo.pSkillDoneInfo)
			&& ((CN3UIIcon*) pChild == CN3UIWndBase::s_sSkillSelectInfo.pSkillDoneInfo->pUIIcon))
			continue;
		pChild->Render();
	}

	if ((GetState() == UI_STATE_ICON_MOVING) && (CN3UIWndBase::s_sSkillSelectInfo.pSkillDoneInfo))
		CN3UIWndBase::s_sSkillSelectInfo.pSkillDoneInfo->pUIIcon->Render();

	if (m_iCurPage == m_iSelectPage && m_pMyHotkey[m_iSelectPage][m_iSelectIndex])
	{
		CN3UIIcon* pUIIcon = m_pMyHotkey[m_iSelectPage][m_iSelectIndex]->pUIIcon;
		if (pUIIcon)
			RenderSelectIcon(pUIIcon);
	}

	// 현재 페이지에서
	CN3UIArea* pArea = nullptr;
	POINT ptCur      = CGameProcedure::s_pLocalInput->MouseGetPos();

	int k            = 0;
	for (; k < MAX_SKILL_IN_HOTKEY; k++)
	{
		if (m_pMyHotkey[m_iCurPage][k] != nullptr)
		{
			float fCooldown = CGameProcedure::s_pProcMain->m_pMagicSkillMng->GetCooldown(m_pMyHotkey[m_iCurPage][k]->pSkill);

			// not on cooldown
			if (fCooldown < 0)
			{
				CN3UIIcon* pUIIcon = m_pMyHotkey[m_iCurPage][k]->pUIIcon;
				if (pUIIcon != nullptr)
					pUIIcon->Render();
			}
			else
			{
				RenderCooldown(m_pMyHotkey[m_iCurPage][k], fCooldown);
			}

			DisplayCountStr(m_pMyHotkey[m_iCurPage][k]);
		}
	}

	for (k = 0; k < MAX_SKILL_IN_HOTKEY; k++)
	{
		if (m_pMyHotkey[m_iCurPage][k] != nullptr)
		{
			pArea = nullptr;
			pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_SKILL_HOTKEY, k);
			if (pArea && pArea->IsIn(ptCur.x, ptCur.y))
			{
				bTooltipRender = true;
				break;
			}
		}
	}

	pSkill = m_pMyHotkey[m_iCurPage][k];

	if (bTooltipRender && pSkill)
		DisplayTooltipStr(pSkill);
}

void CUIHotKeyDlg::Open()
{
	SetVisible(true);
}

void CUIHotKeyDlg::Close()
{
	SetVisible(false);
}

void CUIHotKeyDlg::InitIconWnd(e_UIWND eWnd)
{
	CN3UIWndBase::InitIconWnd(eWnd);
}

void CUIHotKeyDlg::InitIconUpdate()
{
	// Get Hotkey Data From Registry..
	// First, Getting Hotkey Data Count..

	for (int j = 0; j < MAX_SKILL_IN_HOTKEY; j++)
	{
		m_pTooltipStr[j] = GetTooltipStrControl(j);
		m_pCountStr[j]   = GetCountStrControl(j);
	}

	int iHCount = 0;
	if (!CGameProcedure::RegGetSetting("Count", &iHCount, sizeof(int)))
		return;

	if ((iHCount < 0) || (iHCount > 65))
		return;

	int iSkillCount = 0;
	CHotkeyData HD;
	//	uint32_t bitMask;

	while (iHCount--)
	{
		std::string str = "Data" + std::to_string(iSkillCount);
		if (CGameProcedure::RegGetSetting(str.c_str(), &HD, sizeof(CHotkeyData)))
		{
			// Skill Tree Window가 아이디를 갖고 있지 않으면 continue..
			if ((HD.iID < UIITEM_TYPE_USABLE_ID_MIN) && (!CGameProcedure::s_pProcMain->m_pUISkillTreeDlg->HasIDSkill(HD.iID)))
				continue;

			__TABLE_UPC_SKILL* pUSkill = CGameBase::s_pTbl_Skill.Find(HD.iID);
			if (!pUSkill)
				continue;

			__IconItemSkill* spSkill = new __IconItemSkill();
			spSkill->pSkill          = pUSkill;

			// 아이콘 이름 만들기.. ^^
			spSkill->szIconFN        = fmt::format("UI\\skillicon_{:02}_{}.dxt", HD.iID % 100, HD.iID / 100);

			// 아이콘 로드하기.. ^^
			spSkill->pUIIcon         = new CN3UIIcon;
			spSkill->pUIIcon->Init(this);
			spSkill->pUIIcon->SetTex(spSkill->szIconFN);
			spSkill->pUIIcon->SetUVRect(0, 0, 1, 1);
			spSkill->pUIIcon->SetUIType(UI_TYPE_ICON);
			spSkill->pUIIcon->SetStyle(UISTYLE_ICON_SKILL);

			SetHotKeyTooltip(spSkill);

			CN3UIArea* pArea = nullptr;
			pArea            = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_SKILL_HOTKEY, HD.column);
			if (pArea)
			{
				spSkill->pUIIcon->SetRegion(pArea->GetRegion());
				spSkill->pUIIcon->SetMoveRect(pArea->GetRegion());
			}

			// 아이콘 정보 저장..
			m_pMyHotkey[HD.row][HD.column] = spSkill;
		}
		iSkillCount++;
	}

	SetHotKeyPage(0);
}

void CUIHotKeyDlg::UpdateDisableCheck()
{
	for (int i = 0; i < MAX_SKILL_HOTKEY_PAGE; i++)
	{
		for (int j = 0; j < MAX_SKILL_IN_HOTKEY; j++)
		{
			if (m_pMyHotkey[i][j] == nullptr)
				continue;

			uint32_t bitMask = UISTYLE_ICON_SKILL;
			if (!CGameProcedure::s_pProcMain->m_pMagicSkillMng->CheckValidSkillMagic(m_pMyHotkey[i][j]->pSkill))
				bitMask |= UISTYLE_DISABLE_SKILL;
			m_pMyHotkey[i][j]->pUIIcon->SetStyle(bitMask);
		}
	}
}

void CUIHotKeyDlg::CloseIconRegistry()
{
	// Save Hotkey Data to Registry..
	// First, Saving Hotkey Data Count..
	int iHCount = 0;
	for (int i = 0; i < MAX_SKILL_HOTKEY_PAGE; i++)
	{
		for (int j = 0; j < MAX_SKILL_IN_HOTKEY; j++)
		{
			if (m_pMyHotkey[i][j] != nullptr)
				iHCount++;
		}
	}

	CGameProcedure::RegPutSetting("Count", &iHCount, sizeof(int));

	int iSkillCount = 0;

	for (int i = 0; i < MAX_SKILL_HOTKEY_PAGE; i++)
	{
		for (int j = 0; j < MAX_SKILL_IN_HOTKEY; j++)
		{
			if (m_pMyHotkey[i][j] == nullptr)
				continue;

			std::string str = "Data" + std::to_string(iSkillCount);

			CHotkeyData HD(i, j, m_pMyHotkey[i][j]->pSkill->dwID);
			CGameProcedure::RegPutSetting(str.c_str(), &HD, sizeof(CHotkeyData));
			iSkillCount++;
		}
	}
}

__IconItemSkill* CUIHotKeyDlg::GetHighlightIconItem(CN3UIIcon* pUIIcon)
{
	for (int k = 0; k < MAX_SKILL_IN_HOTKEY; k++)
	{
		if ((m_pMyHotkey[m_iCurPage][k] != nullptr) && (m_pMyHotkey[m_iCurPage][k]->pUIIcon == pUIIcon))
			return m_pMyHotkey[m_iCurPage][k];
	}

	return nullptr;
}

void CUIHotKeyDlg::AllFactorClear()
{
	__IconItemSkill* spSkill = nullptr;

	for (int i = 0; i < MAX_SKILL_HOTKEY_PAGE; i++)
	{
		for (int j = 0; j < MAX_SKILL_IN_HOTKEY; j++)
		{
			if (m_pMyHotkey[i][j] != nullptr)
			{
				// 리소스 Free..
				spSkill = m_pMyHotkey[i][j];

				// 매니저에서 제거..
				RemoveChild(spSkill->pUIIcon);

				// 리소스 제거..
				spSkill->pUIIcon->Release();
				delete spSkill->pUIIcon;
				spSkill->pUIIcon = nullptr;
				delete spSkill;
				spSkill           = nullptr;
				m_pMyHotkey[i][j] = nullptr;
			}
		}
	}

	m_iCurPage = 0;
}

int CUIHotKeyDlg::GetAreaiOrder()
{
	// 먼저 Area를 검색한다..
	CN3UIArea* pArea = nullptr;
	POINT ptCur      = CGameProcedure::s_pLocalInput->MouseGetPos();

	for (int i = 0; i < MAX_SKILL_IN_HOTKEY; i++)
	{
		pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_SKILL_HOTKEY, i);
		if (pArea && pArea->IsIn(ptCur.x, ptCur.y))
			return i;
	}

	return -1;
}

bool CUIHotKeyDlg::IsSelectedSkillInRealIconArea()
{
	POINT ptCur = CGameProcedure::s_pLocalInput->MouseGetPos();
	for (int i = 0; i < MAX_SKILL_IN_HOTKEY; i++)
	{
		CN3UIArea* pArea = GetChildAreaByiOrder(UI_AREA_TYPE_SKILL_HOTKEY, i);
		if (pArea == nullptr || !pArea->IsIn(ptCur.x, ptCur.y))
			continue;

		if (s_sSkillSelectInfo.pSkillDoneInfo == nullptr)
			return false;

		SetReceiveSelectedSkill(i);
		return true;
	}

	return false;
}

bool CUIHotKeyDlg::GetEmptySlotIndex(int& iIndex)
{
	for (int i = 0; i < MAX_SKILL_IN_HOTKEY; i++)
	{
		if (m_pMyHotkey[m_iCurPage][i] != nullptr)
			continue;

		iIndex = i;
		return true;
	}

	return false;
}

void CUIHotKeyDlg::SetReceiveSelectedSkill(int iIndex)
{
	__IconItemSkill* spSkill = nullptr;

	if (m_pMyHotkey[m_iCurPage][iIndex] != nullptr)
	{
		// 리소스 Free..
		spSkill = m_pMyHotkey[m_iCurPage][iIndex];

		// 매니저에서 제거..
		RemoveChild(spSkill->pUIIcon);

		// 리소스 제거..
		spSkill->pUIIcon->Release();
		delete spSkill->pUIIcon;
		spSkill->pUIIcon = nullptr;
		delete spSkill;
		spSkill                         = nullptr;
		m_pMyHotkey[m_iCurPage][iIndex] = nullptr;
	}

	CN3UIArea* pArea                = GetChildAreaByiOrder(UI_AREA_TYPE_SKILL_HOTKEY, iIndex);

	// 그 다음에.. 그 자리에
	m_pMyHotkey[m_iCurPage][iIndex] = s_sSkillSelectInfo.pSkillDoneInfo;
	m_pMyHotkey[m_iCurPage][iIndex]->pUIIcon->SetRegion(pArea->GetRegion());
	m_pMyHotkey[m_iCurPage][iIndex]->pUIIcon->SetMoveRect(pArea->GetRegion());
	m_pMyHotkey[m_iCurPage][iIndex]->pUIIcon->SetParent(this);

	SetHotKeyTooltip(m_pMyHotkey[m_iCurPage][iIndex]);
}

RECT CUIHotKeyDlg::GetSampleRect()
{
	CN3UIArea* pArea = GetChildAreaByiOrder(UI_AREA_TYPE_SKILL_HOTKEY, 0);
	if (pArea == nullptr)
		return {};

	POINT ptCur   = CGameProcedure::s_pLocalInput->MouseGetPos();
	RECT rect     = pArea->GetRegion();
	float fWidth  = (float) (rect.right - rect.left);
	float fHeight = (float) (rect.bottom - rect.top);
	rect.left     = ptCur.x - m_ptOffset.x;
	rect.right    = rect.left + (int) fWidth;
	rect.top      = ptCur.y - m_ptOffset.y;
	rect.bottom   = rect.top + (int) fHeight;
	return rect;
}

void CUIHotKeyDlg::PageUp()
{
	if (m_iCurPage <= 0)
		return;

	SetHotKeyPage(--m_iCurPage);
}

void CUIHotKeyDlg::PageDown()
{
	if (m_iCurPage >= (MAX_SKILL_HOTKEY_PAGE - 1))
		return;

	SetHotKeyPage(++m_iCurPage);
}

void CUIHotKeyDlg::SetHotKeyPage(int iPageNum)
{
	for (int i = 0; i < MAX_SKILL_HOTKEY_PAGE; i++)
	{
		if (i != iPageNum)
		{
			for (int j = 0; j < MAX_SKILL_IN_HOTKEY; j++)
			{
				if (m_pMyHotkey[i][j] != nullptr)
					m_pMyHotkey[i][j]->pUIIcon->SetVisible(false);
			}
		}
		else
		{
			for (int j = 0; j < MAX_SKILL_IN_HOTKEY; j++)
			{
				if (m_pMyHotkey[i][j] != nullptr)
					m_pMyHotkey[i][j]->pUIIcon->SetVisible(true);
			}
		}
	}

	m_iCurPage = iPageNum;
}

bool CUIHotKeyDlg::CalcMoveOffset()
{
	POINT ptCur = CGameProcedure::s_pLocalInput->MouseGetPos();
	int iOrder  = GetAreaiOrder();
	if (iOrder == -1)
		return false;

	CN3UIArea* pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_SKILL_HOTKEY, iOrder);
	if (pArea == nullptr)
		return false;

	RECT rect    = pArea->GetRegion();
	m_ptOffset.x = ptCur.x - rect.left;
	m_ptOffset.y = ptCur.y - rect.top;
	return true;
}

void CUIHotKeyDlg::EffectTriggerByHotKey(int iIndex)
{
	if (iIndex < 0 || iIndex >= 8)
		return;

	if (m_pMyHotkey[m_iCurPage][iIndex] && m_pMyHotkey[m_iCurPage][iIndex]->pUIIcon->IsVisible())
	{
		DoOperate(m_pMyHotkey[m_iCurPage][iIndex]);
	}
}

void CUIHotKeyDlg::DoOperate(__IconItemSkill* pSkill)
{
	if (pSkill == nullptr)
		return;

	// 메시지 박스 출력..
	// std::string buff = fmt::format("{} 스킬이 사용되었습니다.", pSkill->pSkill->szName);
	// CGameProcedure::s_pProcMain->MsgOutput(buff, 0xffffff00);

	int iIDTarget = CGameBase::s_pPlayer->m_iIDTarget;
	CGameProcedure::s_pProcMain->m_pMagicSkillMng->MsgSend_MagicProcess(iIDTarget, pSkill->pSkill);
}

void CUIHotKeyDlg::ClassChangeHotkeyFlush()
{
	__IconItemSkill* spSkill = nullptr;

	for (int i = 0; i < MAX_SKILL_HOTKEY_PAGE; i++)
	{
		for (int j = 0; j < MAX_SKILL_IN_HOTKEY; j++)
		{
			if (m_pMyHotkey[i][j] != nullptr)
			{
				// 리소스 Free..
				spSkill = m_pMyHotkey[i][j];

				// 매니저에서 제거..
				RemoveChild(spSkill->pUIIcon);

				// 리소스 제거..
				spSkill->pUIIcon->Release();
				delete spSkill->pUIIcon;
				spSkill->pUIIcon = nullptr;
				delete spSkill;
				spSkill           = nullptr;
				m_pMyHotkey[i][j] = nullptr;
			}
		}
	}
}

CN3UIString* CUIHotKeyDlg::GetTooltipStrControl(int iIndex)
{
	std::string str   = std::to_string(iIndex + 10);
	CN3UIString* pStr = nullptr;
	N3_VERIFY_UI_COMPONENT(pStr, GetChildByID<CN3UIString>(str));
	return pStr;
}

CN3UIString* CUIHotKeyDlg::GetCountStrControl(int iIndex)
{
	std::string str   = std::to_string(iIndex);
	CN3UIString* pStr = nullptr;
	N3_VERIFY_UI_COMPONENT(pStr, GetChildByID<CN3UIString>(str));
	return pStr;
}

void CUIHotKeyDlg::DisplayTooltipStr(__IconItemSkill* spSkill)
{
	int iIndex = GetTooltipCurPageIndex(spSkill);
	if (iIndex != -1)
	{
		if (!m_pTooltipStr[iIndex]->IsVisible())
			m_pTooltipStr[iIndex]->SetVisible(true);

		m_pTooltipStr[iIndex]->SetString(spSkill->pSkill->szName);
		m_pTooltipStr[iIndex]->Render();
	}
}

void CUIHotKeyDlg::DisableTooltipDisplay()
{
	for (int j = 0; j < MAX_SKILL_IN_HOTKEY; j++)
	{
		if (m_pTooltipStr[j])
			m_pTooltipStr[j]->SetVisible(false);
	}
}

void CUIHotKeyDlg::DisplayCountStr(__IconItemSkill* spSkill)
{
	int iIndex = GetCountCurPageIndex(spSkill);
	if (iIndex != -1)
	{
		if (!m_pCountStr[iIndex]->IsVisible())
			m_pCountStr[iIndex]->SetVisible(true);

		m_pCountStr[iIndex]->SetStringAsInt(CGameProcedure::s_pProcMain->m_pUIInventory->GetCountInInvByID(spSkill->pSkill->dwExhaustItem));
		m_pCountStr[iIndex]->Render();
	}
}

void CUIHotKeyDlg::DisableCountStrDisplay()
{
	for (int j = 0; j < MAX_SKILL_IN_HOTKEY; j++)
	{
		if (m_pCountStr[j])
			m_pCountStr[j]->SetVisible(false);
	}
}

int CUIHotKeyDlg::GetTooltipCurPageIndex(__IconItemSkill* pSkill)
{
	for (int k = 0; k < MAX_SKILL_IN_HOTKEY; k++)
	{
		if ((m_pMyHotkey[m_iCurPage][k] != nullptr) && (m_pMyHotkey[m_iCurPage][k] == pSkill))
			return k;
	}

	return -1;
}

int CUIHotKeyDlg::GetCountCurPageIndex(__IconItemSkill* spSkill)
{
	if (spSkill->pSkill->dwExhaustItem <= 0)
		return -1;

	return GetTooltipCurPageIndex(spSkill);
}

bool CUIHotKeyDlg::ReceiveIconDrop(__IconItemSkill* /*spItem*/, POINT ptCur)
{
	bool bFound = false;
	// 내가 가졌던 아이콘이 아니면..
	if (CN3UIWndBase::s_sSelectedIconInfo.UIWndSelect.UIWnd != UIWND_INVENTORY)
		return false;

	CN3UIArea* pArea = nullptr;

	int iOrder       = -1;
	for (int i = 0; i < ITEM_SLOT_COUNT; i++)
	{
		pArea = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_SKILL_HOTKEY, i);
		if (pArea && pArea->IsIn(ptCur.x, ptCur.y))
		{
			bFound = true;
			iOrder = i;
			break;
		}
	}

	if (!bFound)
		return false;

	__IconItemSkill *spSkill = nullptr, *spItem = nullptr;

	// 기존 아이콘이 있다면..
	if (m_pMyHotkey[m_iCurPage][iOrder] != nullptr)
	{
		// 기존 아이콘을 삭제한다..
		spSkill = m_pMyHotkey[m_iCurPage][iOrder];

		// 매니저에서 제거..
		RemoveChild(spSkill->pUIIcon);

		// 리소스 제거..
		spSkill->pUIIcon->Release();
		delete spSkill->pUIIcon;
		spSkill->pUIIcon = nullptr;
		delete spSkill;
		spSkill                         = nullptr;
		m_pMyHotkey[m_iCurPage][iOrder] = nullptr;
	}

	spItem                     = CN3UIWndBase::s_sSelectedIconInfo.pItemSelect;

	__TABLE_UPC_SKILL* pUSkill = CGameBase::s_pTbl_Skill.Find(spItem->pItemBasic->dwEffectID1);
	if (pUSkill == nullptr)
		return false;
	if (pUSkill->dwID < UIITEM_TYPE_USABLE_ID_MIN)
		return false;

	spSkill           = new __IconItemSkill();
	spSkill->pSkill   = pUSkill;

	// 아이콘 이름 만들기.. ^^
	spSkill->szIconFN = fmt::format(
		"UI\\skillicon_{:02}_{}.dxt", spItem->pItemBasic->dwEffectID1 % 100, spItem->pItemBasic->dwEffectID1 / 100);

	// 아이콘 로드하기.. ^^
	spSkill->pUIIcon = new CN3UIIcon;
	spSkill->pUIIcon->Init(this);
	spSkill->pUIIcon->SetTex(spSkill->szIconFN);
	spSkill->pUIIcon->SetUVRect(0, 0, 1.0f, 1.0f);
	spSkill->pUIIcon->SetUIType(UI_TYPE_ICON);
	spSkill->pUIIcon->SetStyle(UISTYLE_ICON_SKILL);

	SetHotKeyTooltip(spSkill);

	uint32_t bitMask = UISTYLE_ICON_SKILL;
	if (!CGameProcedure::s_pProcMain->m_pMagicSkillMng->CheckValidSkillMagic(spSkill->pSkill))
		bitMask |= UISTYLE_DISABLE_SKILL;
	spSkill->pUIIcon->SetStyle(bitMask);

	if (pArea != nullptr)
	{
		spSkill->pUIIcon->SetRegion(pArea->GetRegion());
		spSkill->pUIIcon->SetMoveRect(pArea->GetRegion());
	}

	m_pMyHotkey[m_iCurPage][iOrder] = spSkill;

	CloseIconRegistry();

	return false;
}

bool CUIHotKeyDlg::SetReceiveSelectedItem(int iIndex)
{
	if (CN3UIWndBase::s_sSelectedIconInfo.UIWndSelect.UIWnd != UIWND_INVENTORY)
		return false;

	__IconItemSkill* spItem    = CN3UIWndBase::s_sSelectedIconInfo.pItemSelect;

	__TABLE_UPC_SKILL* pUSkill = CGameBase::s_pTbl_Skill.Find(spItem->pItemBasic->dwEffectID1);
	if (pUSkill == nullptr)
		return false;

	if (pUSkill->dwID < UIITEM_TYPE_USABLE_ID_MIN)
		return false;

	if (m_pMyHotkey[m_iCurPage][iIndex] != nullptr)
		return false;

	__IconItemSkill* spSkill = new __IconItemSkill();
	spSkill->pSkill          = pUSkill;

	// Create the icon name
	spSkill->szIconFN        = fmt::format(
        "UI\\skillicon_{:02}_{}.dxt", spItem->pItemBasic->dwEffectID1 % 100, spItem->pItemBasic->dwEffectID1 / 100);

	// load icon
	spSkill->pUIIcon = new CN3UIIcon();
	spSkill->pUIIcon->Init(this);
	spSkill->pUIIcon->SetTex(spSkill->szIconFN);
	spSkill->pUIIcon->SetUVRect(0, 0, 1.0f, 1.0f);
	spSkill->pUIIcon->SetUIType(UI_TYPE_ICON);

	SetHotKeyTooltip(spSkill);

	uint32_t bitMask = UISTYLE_ICON_SKILL;
	if (!CGameProcedure::s_pProcMain->m_pMagicSkillMng->CheckValidSkillMagic(spSkill->pSkill))
		bitMask |= UISTYLE_DISABLE_SKILL;
	spSkill->pUIIcon->SetStyle(bitMask);

	CN3UIArea* pArea = nullptr;
	pArea            = CN3UIWndBase::GetChildAreaByiOrder(UI_AREA_TYPE_SKILL_HOTKEY, iIndex);

	if (pArea != nullptr)
	{
		spSkill->pUIIcon->SetRegion(pArea->GetRegion());
		spSkill->pUIIcon->SetMoveRect(pArea->GetRegion());
	}

	m_pMyHotkey[m_iCurPage][iIndex] = spSkill;

	CloseIconRegistry();
	return true;
}

bool CUIHotKeyDlg::EffectTriggerByMouse()
{
	if (m_iSelectIndex < 0 || m_iSelectIndex >= 8)
		return false;
	if (m_iSelectPage < 0 || m_iSelectPage >= 8)
		return false;

	if (m_pMyHotkey[m_iSelectPage][m_iSelectIndex])
	{
		int iIDTarget = CGameBase::s_pPlayer->m_iIDTarget;
		return CGameProcedure::s_pProcMain->m_pMagicSkillMng->MsgSend_MagicProcess(
			iIDTarget, m_pMyHotkey[m_iSelectPage][m_iSelectIndex]->pSkill);
	}

	return false;
}

void CUIHotKeyDlg::RenderSelectIcon(CN3UIIcon* pUIIcon)
{
	if (!pUIIcon)
		return;

	RECT rc = pUIIcon->GetRegion(); // 선택 표시

	__VertexTransformedColor vLines[5] {};
	vLines[0].Set((float) rc.left, (float) rc.top, UI_DEFAULT_Z, UI_DEFAULT_RHW, 0xff00ff00);
	vLines[1].Set((float) rc.right, (float) rc.top, UI_DEFAULT_Z, UI_DEFAULT_RHW, 0xff00ff00);
	vLines[2].Set((float) rc.right, (float) rc.bottom, UI_DEFAULT_Z, UI_DEFAULT_RHW, 0xff00ff00);
	vLines[3].Set((float) rc.left, (float) rc.bottom, UI_DEFAULT_Z, UI_DEFAULT_RHW, 0xff00ff00);
	vLines[4] = vLines[0];

	DWORD dwZ = 0, dwFog = 0, dwAlpha = 0, dwCOP = 0, dwCA1 = 0, dwSrcBlend = 0, dwDestBlend = 0, dwVertexShader = 0, dwAOP = 0, dwAA1 = 0;
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_ZENABLE, &dwZ);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_FOGENABLE, &dwFog);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlpha);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_SRCBLEND, &dwSrcBlend);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_DESTBLEND, &dwDestBlend);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLOROP, &dwCOP);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLORARG1, &dwCA1);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAOP, &dwAOP);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAARG1, &dwAA1);
	CN3Base::s_lpD3DDev->GetFVF(&dwVertexShader);

	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, FALSE);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, FALSE);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);

	CN3Base::s_lpD3DDev->SetFVF(FVF_TRANSFORMEDCOLOR);
	CN3Base::s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, vLines, sizeof(__VertexTransformedColor));

	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, dwZ);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, dwFog);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlpha);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, dwSrcBlend);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, dwDestBlend);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, dwCOP);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, dwCA1);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, dwAOP);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, dwAA1);
	CN3Base::s_lpD3DDev->SetFVF(dwVertexShader);
}

void CUIHotKeyDlg::RenderCooldown(const __IconItemSkill* pSkill, float fCooldown)
{
	if (pSkill == nullptr)
		return;

	constexpr D3DCOLOR Color = D3DCOLOR_ARGB(0x80, 0xFF, 0x00, 0x00);

	const RECT rc            = pSkill->pUIIcon->GetRegion();

	const float centerX      = static_cast<float>(rc.left + rc.right) / 2;
	const float centerY      = static_cast<float>(rc.top + rc.bottom) / 2;

	const float halfWidth    = static_cast<float>(centerX - rc.left);
	const float halfHeight   = static_cast<float>(centerY - rc.top);

	const float radius       = std::sqrtf(std::pow(halfWidth, 2.0f) + std::pow(halfHeight, 2.0f));

	float progress           = 0.0f;

	if (pSkill->pSkill->iReCastTime > 0)
	{
		progress = (fCooldown / (static_cast<float>(pSkill->pSkill->iReCastTime) / 10.0f));
		progress = std::clamp(progress, 0.0f, 1.0f);
	}

	// arbitrary number of segments. this might be too many for such a small icon.
	const int segments           = 64;
	const int segmentCountToDraw = static_cast<int>((segments * progress));

	std::vector<__VertexTransformedColor> vertices;
	vertices.reserve(segments);

	// not 100% sure on the color. Choosing arbitrary 50% opacity.
	vertices.emplace_back(centerX, centerY, UI_DEFAULT_Z, UI_DEFAULT_RHW, Color);

	const float fullCircle = __PI * 2.0f;
	const float maxAngle   = fullCircle * progress;
	const float startAngle = -__PI / 2.0f; // 12 o'clock

	std::vector<__VertexTransformedColor> arcVertices;
	arcVertices.reserve(segmentCountToDraw);

	for (int i = 0; i <= segmentCountToDraw; i++)
	{
		float angle = startAngle - maxAngle * (static_cast<float>(i) / segmentCountToDraw);
		float x     = centerX + cosf(angle) * radius;
		float y     = centerY + sinf(angle) * radius;
		arcVertices.emplace_back(x, y, UI_DEFAULT_Z, UI_DEFAULT_RHW, Color);
	}

	// very crude way but i'd rather keep culling enabled.
	vertices.insert(vertices.end(), arcVertices.rbegin(), arcVertices.rend());

	// disable culling and reverse vectors.
	//for (int i = 0; i <= segmentCountToDraw; i++)
	//{
	//	// Go clockwise by subtracting angle increments
	//	float angle = startAngle - maxAngle * (static_cast<float>(i) / segmentCountToDraw);
	//	float x = centerX + cosf(angle) * radius;
	//	float y = centerY + sinf(angle) * radius;
	//	vertices.emplace_back(x, y, UI_DEFAULT_Z, UI_DEFAULT_RHW, Color);
	//}

	DWORD dwZ = 0, dwFog = 0, dwAlpha = 0, dwCOP = 0, dwCA1 = 0, dwSrcBlend = 0, dwDestBlend = 0, dwVertexShader = 0, dwAOP = 0, dwAA1 = 0;
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_ZENABLE, &dwZ);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_FOGENABLE, &dwFog);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlpha);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_SRCBLEND, &dwSrcBlend);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_DESTBLEND, &dwDestBlend);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLOROP, &dwCOP);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLORARG1, &dwCA1);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAOP, &dwAOP);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAARG1, &dwAA1);
	CN3Base::s_lpD3DDev->GetFVF(&dwVertexShader);

	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, FALSE);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, FALSE);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
	CN3Base::s_lpD3DDev->SetScissorRect(&rc);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);

	CN3Base::s_lpD3DDev->SetFVF(FVF_TRANSFORMED);
	// CN3Base::s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	CN3Base::s_lpD3DDev->DrawPrimitiveUP(
		D3DPT_TRIANGLEFAN, static_cast<UINT>(vertices.size()) - 2, &vertices[0], sizeof(__VertexTransformedColor));

	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, dwZ);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, dwFog);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlpha);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, dwSrcBlend);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, dwDestBlend);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, dwCOP);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, dwCA1);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, dwAOP);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, dwAA1);
	CN3Base::s_lpD3DDev->SetFVF(dwVertexShader);
}

//this_ui_add_start
bool CUIHotKeyDlg::OnKeyPress(int iKey)
{
	// hotkey가 포커스 잡혀있을때는 다른 ui를 닫을수 없으므로 DIK_ESCAPE가 들어오면 포커스를 다시잡고
	if (iKey == DIK_ESCAPE)
	{
		// 열려있는 다른 유아이를 닫아준다.
		CGameProcedure::s_pUIMgr->ReFocusUI(); //this_ui
		CN3UIBase* pFocus = CGameProcedure::s_pUIMgr->GetFocusedUI();
		if (pFocus && pFocus != this)
			pFocus->OnKeyPress(iKey);
		return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}
//this_ui_add_end

void CUIHotKeyDlg::SetHotKeyTooltip(__IconItemSkill* spSkill)
{
	if (spSkill == nullptr || spSkill->pSkill == nullptr || spSkill->pUIIcon == nullptr)
		return;

	std::string szTooltip = fmt::format("[{}] {}", spSkill->pSkill->szName, spSkill->pSkill->szDesc);
	spSkill->pUIIcon->SetTooltipText(szTooltip);
	spSkill->pUIIcon->SetTooltipColor(D3DCOLOR_XRGB(0x80, 0x80, 0xFF));
}

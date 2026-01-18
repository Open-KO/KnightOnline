#include "StdAfx.h"
#include "UIUpgradeSelect.h"
#include "GameProcMain.h"
// #include "UIItemUpgrade.h"
// #include "UIRingUpgrade.h"
#include "UIManager.h"

#include <N3Base/N3UIButton.h>

CUIUpgradeSelect::CUIUpgradeSelect()
{
	m_pBtn_Upgrade_1 = nullptr;
	m_pBtn_Upgrade_2 = nullptr;
	m_pBtn_Close     = nullptr;
	m_iNpcID         = 0;
}

CUIUpgradeSelect::~CUIUpgradeSelect()
{
}

bool CUIUpgradeSelect::Load(File& file)
{
	if (!CN3UIBase::Load(file))
		return false;

	N3_VERIFY_UI_COMPONENT(m_pBtn_Upgrade_1, GetChildByID<CN3UIButton>("upgrade_1"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Upgrade_2, GetChildByID<CN3UIButton>("upgrade_2"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Close, GetChildByID<CN3UIButton>("btn_close"));

	return true;
}

bool CUIUpgradeSelect::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (pSender == nullptr)
		return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtn_Upgrade_1)
		{
#if 1
			CGameProcedure::MessageBoxPost("CUIItemUpgrade needs to be implemented.", "Not implemented", MB_OK);
#else
			CUIItemUpgrade* pUIItemUpgrade = CGameProcedure::s_pProcMain->m_pUIItemUpgrade;
			if (pUIItemUpgrade != nullptr)
			{
				pUIItemUpgrade->SetVisible(true);
				pUIItemUpgrade->SetNpcID(m_iNpcID);
			}
#endif

			SetVisible(false);
		}
		else if (pSender == m_pBtn_Upgrade_2)
		{
#if 1
			CGameProcedure::MessageBoxPost("CUIRingUpgrade needs to be implemented.", "Not implemented", MB_OK);
#else
			CUIRingUpgrade* pUIRingUpgrade = CGameProcedure::s_pProcMain->m_pUIRingUpgrade;
			if (pUIRingUpgrade != nullptr)
			{
				pUIRingUpgrade->SetVisible(true);
				pUIRingUpgrade->SetNpcID(m_iNpcID);
			}
#endif

			SetVisible(false);
		}
		else if (pSender == m_pBtn_Close)
		{
			SetVisible(false);
		}
	}

	return true;
}

void CUIUpgradeSelect::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);

	if (bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
		CGameProcedure::s_pUIMgr->ReFocusUI();
}

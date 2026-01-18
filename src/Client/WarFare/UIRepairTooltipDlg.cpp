// UIRepairTooltipDlg.cpp: implementation of the UIRepairTooltipDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UIRepairTooltipDlg.h"
#include "text_resources.h"

#include <N3Base/N3UIString.h>

CUIRepairTooltipDlg::CUIRepairTooltipDlg()
{
	m_iPosXBack            = 0;
	m_iPosYBack            = 0;
	m_spItemBack           = nullptr;

	m_bBRender             = false;
	m_iBxpos               = -1;
	m_iBypos               = -1;
	m_pBspItem             = nullptr;
	m_iBRequiredGold       = 0;
	m_bBHaveEnough         = true;

	m_iBRequiredGoldBackup = 0;

	for (int i = 0; i < MAX_REPAIR_TOOLTIP_COUNT; i++)
	{
		m_pStr[i]      = nullptr;
		m_pStrColor[i] = 0;
	}
}

CUIRepairTooltipDlg::~CUIRepairTooltipDlg()
{
	CUIRepairTooltipDlg::Release();
}

void CUIRepairTooltipDlg::Release()
{
	CN3UIBase::Release();
}

void CUIRepairTooltipDlg::InitPos()
{
	N3_VERIFY_UI_COMPONENT(m_pStr[0], GetChildByID<CN3UIString>("string_repairgold"));  //이름
	m_pStr[0]->SetString("0");
	N3_VERIFY_UI_COMPONENT(m_pStr[1], GetChildByID<CN3UIString>("string_dur_max"));     //이름
	m_pStr[1]->SetString("0");
	N3_VERIFY_UI_COMPONENT(m_pStr[2], GetChildByID<CN3UIString>("string_dur_current")); //이름
	m_pStr[2]->SetString("0");
	N3_VERIFY_UI_COMPONENT(m_pStr[3], GetChildByID<CN3UIString>("string_title"));       //이름
	m_pStr[3]->SetString("0");
	BackupStrColor();
}

void CUIRepairTooltipDlg::BackupStrColor()
{
	m_pStrColor[0] = m_pStr[0]->GetColor();
	m_pStrColor[1] = m_pStr[1]->GetColor();
	m_pStrColor[2] = m_pStr[2]->GetColor();
	m_pStrColor[3] = m_pStr[3]->GetColor();
}

void CUIRepairTooltipDlg::DisplayTooltipsEnable(int xpos, int ypos, __IconItemSkill* spItem, int iRequiredGold, bool bHaveEnough)
{
	int iIndex = 0;
	if (spItem == nullptr)
		return;

	if (!IsVisible())
		SetVisible(true);

	std::string szMsg;

	if (m_iPosXBack != xpos || m_iPosYBack != ypos)
	{
		int iWidth  = m_rcRegion.right - m_rcRegion.left;
		int iHeight = m_rcRegion.bottom - m_rcRegion.top;
		int iRight = 1024, iTop = 0;

		if ((xpos + iWidth < iRight) && ((ypos - iHeight) > iTop))
			SetPos(xpos + 26, ypos - iHeight);
		else if ((xpos + iWidth > iRight) && ((ypos - iHeight) < iTop))
			SetPos(xpos - iWidth, ypos);
		else if (xpos + iWidth > iRight)
			SetPos(xpos - iWidth, ypos - iHeight);
		else
			SetPos(xpos + 26, ypos);

		m_iPosXBack = xpos;
		m_iPosYBack = ypos;
	}

	if ((!m_spItemBack) || (m_spItemBack->pItemBasic->dwID != spItem->pItemBasic->dwID) || (m_iBRequiredGoldBackup != m_iBRequiredGold))
	{
		m_spItemBack = spItem;

		if ((spItem->pItemBasic->byContable != UIITEM_TYPE_COUNTABLE) && (spItem->pItemBasic->byContable != UIITEM_TYPE_COUNTABLE_SMALL)
			&& (spItem->pItemBasic->siMaxDurability + spItem->pItemExt->siMaxDurability != 1))
		{
			szMsg = fmt::format_text_resource(IDS_TOOLTIP_NOAH, iRequiredGold);

			SetTooltipTextColor(bHaveEnough);
			if (!m_pStr[iIndex]->IsVisible())
				m_pStr[iIndex]->SetVisible(true);
			m_pStr[iIndex]->SetString(szMsg);
			iIndex++;

			szMsg = fmt::format_text_resource(
				IDS_TOOLTIP_MAX_DURABILITY, spItem->pItemBasic->siMaxDurability + spItem->pItemExt->siMaxDurability);

			if (!m_pStr[iIndex]->IsVisible())
				m_pStr[iIndex]->SetVisible(true);
			m_pStr[iIndex]->SetString(szMsg);
			iIndex++;

			szMsg = fmt::format_text_resource(IDS_TOOLTIP_CUR_DURABILITY, spItem->iDurability);

			if (!m_pStr[iIndex]->IsVisible())
				m_pStr[iIndex]->SetVisible(true);
			m_pStr[iIndex]->SetString(szMsg);
			iIndex++;

			szMsg = fmt::format_text_resource(IDS_TOOLTIP_REPAIR_PRICE);

			if (!m_pStr[iIndex]->IsVisible())
				m_pStr[iIndex]->SetVisible(true);
			m_pStr[iIndex]->SetString(szMsg);
			iIndex++;

			for (int i = iIndex; i < MAX_REPAIR_TOOLTIP_COUNT; i++)
				m_pStr[i]->SetString("");
		}
		else
		{
			szMsg = fmt::format_text_resource(IDS_TOOLTIP_CANNOT);

			m_pStr[0]->SetVisible(false);
			m_pStr[1]->SetVisible(false);
			m_pStr[2]->SetVisible(false);
			m_pStr[3]->SetString(szMsg);
			m_pStr[3]->SetVisible(true);
		}
	}

	m_iBRequiredGoldBackup = m_iBRequiredGold;
	CN3UIBase::Render();
}

void CUIRepairTooltipDlg::DisplayTooltipsDisable()
{
	m_spItemBack = nullptr;
	if (IsVisible())
		SetVisible(FALSE);
}

void CUIRepairTooltipDlg::SetTooltipTextColor(bool bHaveEnough)
{
	if (bHaveEnough)
		m_pStr[0]->SetColor(m_pStrColor[0]);
	else
		m_pStr[0]->SetColor(0xffdd0000);
}

void CUIRepairTooltipDlg::Render()
{
	if (!m_bBRender)
		DisplayTooltipsDisable();
	else
		DisplayTooltipsEnable(m_iBxpos, m_iBypos, m_pBspItem, m_iBRequiredGold, m_bBHaveEnough);
}

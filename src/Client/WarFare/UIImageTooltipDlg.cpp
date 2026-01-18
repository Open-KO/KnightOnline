// UIImageTooltipDlg.cpp: implementation of the CUIImageTooltipDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UIImageTooltipDlg.h"
#include "PlayerMySelf.h"
#include "text_resources.h"

#include <N3Base/N3UIString.h>

class ItemTooltipTooLargeException : public std::runtime_error
{
public:
	ItemTooltipTooLargeException(const char* msg) : runtime_error(msg)
	{
	}
};

class ItemTooltipBuilderContext
{
public:
	static constexpr int MAX_TOOLTIP_COUNT = CUIImageTooltipDlg::MAX_TOOLTIP_COUNT;

	ItemTooltipBuilderContext(bool price, int& index) : _price(price), _index(index)
	{
	}

	void IncIndex(int increment = 1)
	{
		_index += increment;

		if (_price)
		{
			if (_index > MAX_TOOLTIP_COUNT - 2)
			{
				__ASSERT(0, "Too many entries for this tooltip");
				throw ItemTooltipTooLargeException("Too many entries for this tooltip");
			}
		}
		else
		{
			if (_index > MAX_TOOLTIP_COUNT - 1)
			{
				__ASSERT(0, "Too many entries for this tooltip");
				throw ItemTooltipTooLargeException("Too many entries for this tooltip");
			}
		}
	}

protected:
	bool _price;
	int& _index;
};

CUIImageTooltipDlg::CUIImageTooltipDlg() :
	m_CYellow(D3DCOLOR_XRGB(255, 255, 0)), m_CBlue(D3DCOLOR_XRGB(128, 128, 255)), m_CGold(D3DCOLOR_XRGB(220, 199, 124)),
	m_CIvory(D3DCOLOR_XRGB(200, 124, 199)), m_CGreen(D3DCOLOR_XRGB(128, 255, 0)), m_CWhite(D3DCOLOR_XRGB(255, 255, 255)),
	m_CRed(D3DCOLOR_XRGB(255, 60, 60))
{
	m_iPosXBack   = 0;
	m_iPosYBack   = 0;
	m_pImg        = nullptr;
	m_iTooltipNum = 0;

	ResetItem();
}

CUIImageTooltipDlg::~CUIImageTooltipDlg()
{
}

void CUIImageTooltipDlg::Release()
{
	CN3UIBase::Release();

	m_iPosXBack   = 0;
	m_iPosYBack   = 0;
	m_pImg        = nullptr;
	m_iTooltipNum = 0;

	ResetItem();
}

void CUIImageTooltipDlg::InitPos()
{
	std::string str;

	for (int i = 0; i < MAX_TOOLTIP_COUNT; i++)
	{
		str = "string_" + std::to_string(i);
		N3_VERIFY_UI_COMPONENT(m_pStr[i], GetChildByID<CN3UIString>(str));
	}

	N3_VERIFY_UI_COMPONENT(m_pImg, GetChildByID<CN3UIImage>("mins"));
}

void CUIImageTooltipDlg::DisplayTooltipsDisable()
{
	if (IsVisible())
		SetVisible(false);
}

bool CUIImageTooltipDlg::SetTooltipTextColor(int iMyValue, int iTooltipValue)
{
	return iMyValue >= iTooltipValue;
}

bool CUIImageTooltipDlg::SetTooltipTextColor(e_Race eMyValue, e_Race eTooltipValue)
{
	return eMyValue == eTooltipValue;
}

bool CUIImageTooltipDlg::SetTooltipTextColor(e_Class eMyValue, e_Class eTooltipValue)
{
	return eMyValue == eTooltipValue;
}

void CUIImageTooltipDlg::SetPosSomething(int xpos, int ypos)
{
	if (m_iTooltipNum <= 0)
		return;

	int iWidth   = 0;
	int iPadding = 8;

	for (int i = 0; i < m_iTooltipNum; i++)
	{
		if (m_pstdstr[i].empty())
			continue;

		int currentWidth = m_pStr[0]->GetStringRealWidth(m_pstdstr[i]);
		if (currentWidth > iWidth)
			iWidth = currentWidth;
	}

	int iHeight  = m_pStr[m_iTooltipNum - 1]->GetRegion().bottom - m_pStr[0]->GetRegion().top;

	iWidth      += iPadding * 2;
	iHeight     += static_cast<int>(iPadding * 1.5);

	RECT rect {}, rect2 {};
	int iRight = 0, iTop = 0, iBottom = 0, iX = 0, iY = 0;

	iRight  = s_CameraData.vp.Width;
	iTop    = 0;
	iBottom = s_CameraData.vp.Height;

	if ((xpos + 26 + iWidth) < iRight)
	{
		rect.left  = xpos + 26;
		rect.right = rect.left + iWidth;
		iX         = xpos + 26;
	}
	else
	{
		rect.left  = xpos - iWidth;
		rect.right = xpos;
		iX         = xpos - iWidth;
	}

	if ((ypos - iHeight) > iTop)
	{
		rect.top    = ypos - iHeight;
		rect.bottom = ypos;
		iY          = ypos - iHeight;
	}
	else
	{
		if ((ypos + iHeight) < iBottom)
		{
			rect.top    = ypos;
			rect.bottom = ypos + iHeight;
			iY          = ypos;
		}
		else
		{
			rect.top    = iBottom - iHeight;
			rect.bottom = iBottom;
			iY          = rect.top;
		}
	}

	SetPos(iX, iY);
	SetSize(iWidth, iHeight);

	for (int i = 0; i < m_iTooltipNum; i++)
	{
		if (m_pStr[i] == nullptr)
			continue;

		// add padding to rects
		rect2       = m_pStr[i]->GetRegion();
		rect2.left  = rect.left + iPadding;
		rect2.right = rect.right - iPadding;
		m_pStr[i]->SetRegion(rect2);

		if (m_pStr[i]->GetStyle() & UISTYLE_STRING_ALIGNCENTER)
			m_pStr[i]->SetString(m_pstdstr[i]);
		else
			m_pStr[i]->SetString("  " + m_pstdstr[i]);
	}

	for (int i = m_iTooltipNum; i < MAX_TOOLTIP_COUNT; i++)
		m_pStr[i]->SetString("");

	m_pImg->SetRegion(rect);

	m_iPosXBack = xpos;
	m_iPosYBack = ypos;
}

int CUIImageTooltipDlg::CalcTooltipStringNumAndWrite(__IconItemSkill* spItem, bool bPrice, bool bBuy)
{
	if (spItem == nullptr || spItem->pItemBasic == nullptr || spItem->pItemExt == nullptr)
		return 0;

	int iIndex                   = 0;
	__InfoPlayerMySelf* pInfoExt = &CGameBase::s_pPlayer->m_InfoExt;

	ItemTooltipBuilderContext ctx(bPrice, iIndex);

	try
	{
		CalcTooltipStringNumAndWriteImpl(spItem, bPrice, iIndex);
	}
	catch (const ItemTooltipTooLargeException&)
	{
		/* ignore - just stop processing additional entries */
		CLogWriter::Write("Item tooltip too large for item {}", spItem->pItemBasic->dwID + spItem->pItemExt->dwID);
	}

	if (bPrice)
	{
		if (bBuy)
		{
			m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_BUY_PRICE, CGameBase::FormatNumber(spItem->GetBuyPrice()));

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);

			if (SetTooltipTextColor(pInfoExt->iGold, spItem->pItemBasic->iPrice * spItem->pItemExt->siPriceMultiply))
				m_pStr[iIndex]->SetColor(m_CWhite);
			else
				m_pStr[iIndex]->SetColor(m_CRed);
		}
		else
		{
			m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_SELL_PRICE, CGameBase::FormatNumber(spItem->GetSellPrice()));

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CWhite);
		}

		iIndex++;
	}

	for (int i = iIndex; i < MAX_TOOLTIP_COUNT; i++)
		m_pstdstr[iIndex].clear();

	return iIndex; // 임시..	반드시 1보다 크다..
}

void CUIImageTooltipDlg::CalcTooltipStringNumAndWriteImpl(__IconItemSkill* spItem, bool bPrice, int& iIndex)
{
	int iNeedValue               = 0;
	__InfoPlayerMySelf* pInfoExt = &CGameBase::s_pPlayer->m_InfoExt;

	ItemTooltipBuilderContext ctx(bPrice, iIndex);

	if (m_pStr[iIndex] != nullptr)
	{
		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNCENTER);

		std::string szStr = fmt::format_text_resource(IDS_TOOLTIP_GOLD);
		if (spItem->pItemBasic->szName == szStr)
		{
			// 돈이면 흰색..
			m_pStr[iIndex]->SetColor(m_CWhite);
			m_pstdstr[iIndex] = fmt::format("{}  {}", spItem->iCount, spItem->pItemBasic->szName);
			iIndex++;

			for (int i = iIndex; i < MAX_TOOLTIP_COUNT; i++)
				m_pstdstr[iIndex].clear();

			return;
		}

		e_ItemAttrib eTA = (e_ItemAttrib) (spItem->pItemExt->byMagicOrRare);
		switch (eTA)
		{
			case ITEM_ATTRIB_GENERAL:
				m_pStr[iIndex]->SetColor(m_CWhite);
				break;
			case ITEM_ATTRIB_MAGIC:
				m_pStr[iIndex]->SetColor(m_CBlue);
				break;
			case ITEM_ATTRIB_LAIR:
				m_pStr[iIndex]->SetColor(m_CYellow);
				break;
			case ITEM_ATTRIB_CRAFT:
				m_pStr[iIndex]->SetColor(m_CGreen);
				break;
			case ITEM_ATTRIB_UNIQUE:
				m_pStr[iIndex]->SetColor(m_CGold);
				break;
			case ITEM_ATTRIB_UPGRADE:
				m_pStr[iIndex]->SetColor(m_CIvory);
				break;
			default:
				m_pStr[iIndex]->SetColor(m_CWhite);
				break;
		}

		if ((e_ItemAttrib) (spItem->pItemExt->byMagicOrRare) != ITEM_ATTRIB_UNIQUE)
		{
			std::string strtemp;
			if (spItem->pItemExt->dwID % 10 != 0)
				strtemp = fmt::format("(+{})", spItem->pItemExt->dwID % 10);

			m_pstdstr[iIndex] = spItem->pItemBasic->szName + strtemp;
		}
		else
		{
			m_pstdstr[iIndex] = spItem->pItemExt->szHeader;
		}
	}
	ctx.IncIndex();

	if ((spItem->pItemBasic->byContable != UIITEM_TYPE_COUNTABLE) && (spItem->pItemBasic->byContable != UIITEM_TYPE_COUNTABLE_SMALL))
	{
		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNCENTER);
		e_ItemClass eIC = (e_ItemClass) (spItem->pItemBasic->byClass);
		CGameBase::GetTextByItemClass(eIC, m_pstdstr[iIndex]); // 아이템 종류에 따라 문자열 만들기..
		m_pStr[iIndex]->SetColor(m_CWhite);
		ctx.IncIndex();
	}

	e_Race eRace = (e_Race) spItem->pItemBasic->byNeedRace;
	if (eRace != RACE_ALL)
	{
		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNCENTER);
		CGameBase::GetTextByRace(eRace, m_pstdstr[iIndex]); // 아이템을 찰수 있는 종족에 따른 문자열 만들기.
		if (SetTooltipTextColor(CGameBase::s_pPlayer->m_InfoBase.eRace, eRace))
			m_pStr[iIndex]->SetColor(m_CWhite);
		else
			m_pStr[iIndex]->SetColor(m_CRed);
		ctx.IncIndex();
	}

	if ((int) spItem->pItemBasic->byNeedClass != 0)
	{
		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNCENTER);
		e_Class eClass = (e_Class) spItem->pItemBasic->byNeedClass;
		CGameBase::GetTextByClass(eClass, m_pstdstr[iIndex]); // 아이템을 찰수 있는 종족에 따른 문자열 만들기.

		switch (eClass)
		{
			case CLASS_KINDOF_WARRIOR:
				switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
				{
					case CLASS_KA_WARRIOR:
					case CLASS_KA_BERSERKER:
					case CLASS_KA_GUARDIAN:
					case CLASS_EL_WARRIOR:
					case CLASS_EL_BLADE:
					case CLASS_EL_PROTECTOR:
						m_pStr[iIndex]->SetColor(m_CWhite);
						break;
					default:
						m_pStr[iIndex]->SetColor(m_CRed);
						break;
				}
				break;

			case CLASS_KINDOF_ROGUE:
				switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
				{
					case CLASS_KA_ROGUE:
					case CLASS_KA_HUNTER:
					case CLASS_KA_PENETRATOR:
					case CLASS_EL_ROGUE:
					case CLASS_EL_RANGER:
					case CLASS_EL_ASSASIN:
						m_pStr[iIndex]->SetColor(m_CWhite);
						break;
					default:
						m_pStr[iIndex]->SetColor(m_CRed);
						break;
				}
				break;

			case CLASS_KINDOF_WIZARD:
				switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
				{
					case CLASS_KA_WIZARD:
					case CLASS_KA_SORCERER:
					case CLASS_KA_NECROMANCER:
					case CLASS_EL_WIZARD:
					case CLASS_EL_MAGE:
					case CLASS_EL_ENCHANTER:
						m_pStr[iIndex]->SetColor(m_CWhite);
						break;
					default:
						m_pStr[iIndex]->SetColor(m_CRed);
						break;
				}
				break;

			case CLASS_KINDOF_PRIEST:
				switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
				{
					case CLASS_KA_PRIEST:
					case CLASS_KA_SHAMAN:
					case CLASS_KA_DARKPRIEST:
					case CLASS_EL_PRIEST:
					case CLASS_EL_CLERIC:
					case CLASS_EL_DRUID:
						m_pStr[iIndex]->SetColor(m_CWhite);
						break;
					default:
						m_pStr[iIndex]->SetColor(m_CRed);
						break;
				}
				break;

			case CLASS_KINDOF_ATTACK_WARRIOR:
				switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
				{
					case CLASS_KA_BERSERKER:
					case CLASS_EL_BLADE:
						m_pStr[iIndex]->SetColor(m_CWhite);
						break;
					default:
						m_pStr[iIndex]->SetColor(m_CRed);
						break;
				}
				break;

			case CLASS_KINDOF_DEFEND_WARRIOR:
				switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
				{
					case CLASS_KA_GUARDIAN:
					case CLASS_EL_PROTECTOR:
						m_pStr[iIndex]->SetColor(m_CWhite);
						break;
					default:
						m_pStr[iIndex]->SetColor(m_CRed);
						break;
				}
				break;

			case CLASS_KINDOF_ARCHER:
				switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
				{
					case CLASS_KA_HUNTER:
					case CLASS_EL_RANGER:
						m_pStr[iIndex]->SetColor(m_CWhite);
						break;
					default:
						m_pStr[iIndex]->SetColor(m_CRed);
						break;
				}
				break;

			case CLASS_KINDOF_ASSASSIN:
				switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
				{
					case CLASS_KA_PENETRATOR:
					case CLASS_EL_ASSASIN:
						m_pStr[iIndex]->SetColor(m_CWhite);
						break;
					default:
						m_pStr[iIndex]->SetColor(m_CRed);
						break;
				}
				break;

			case CLASS_KINDOF_ATTACK_WIZARD:
				switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
				{
					case CLASS_KA_SORCERER:
					case CLASS_EL_MAGE:
						m_pStr[iIndex]->SetColor(m_CWhite);
						break;
					default:
						m_pStr[iIndex]->SetColor(m_CRed);
						break;
				}
				break;

			case CLASS_KINDOF_PET_WIZARD:
				switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
				{
					case CLASS_KA_NECROMANCER:
					case CLASS_EL_ENCHANTER:
						m_pStr[iIndex]->SetColor(m_CWhite);
						break;
					default:
						m_pStr[iIndex]->SetColor(m_CRed);
						break;
				}
				break;

			case CLASS_KINDOF_HEAL_PRIEST:
				switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
				{
					case CLASS_KA_SHAMAN:
					case CLASS_EL_CLERIC:
						m_pStr[iIndex]->SetColor(m_CWhite);
						break;
					default:
						m_pStr[iIndex]->SetColor(m_CRed);
						break;
				}
				break;

			case CLASS_KINDOF_CURSE_PRIEST:
				switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
				{
					case CLASS_KA_DARKPRIEST:
					case CLASS_EL_DRUID:
						m_pStr[iIndex]->SetColor(m_CWhite);
						break;
					default:
						m_pStr[iIndex]->SetColor(m_CRed);
						break;
				}
				break;

			default:
				if (SetTooltipTextColor(CGameBase::s_pPlayer->m_InfoBase.eClass, eClass))
					m_pStr[iIndex]->SetColor(m_CWhite);
				else
					m_pStr[iIndex]->SetColor(m_CRed);
				break;
		}

		ctx.IncIndex();
	}

	if (spItem->pItemBasic->siDamage + spItem->pItemExt->siDamage != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_DAMAGE, spItem->pItemBasic->siDamage + spItem->pItemExt->siDamage);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CWhite);
		ctx.IncIndex();
	}

	if (spItem->pItemBasic->siAttackInterval * (float) ((float) spItem->pItemExt->siAttackIntervalPercentage / 100.0f) != 0)
	{
		float fValue = spItem->pItemBasic->siAttackInterval * (float) ((float) spItem->pItemExt->siAttackIntervalPercentage / 100.0f);

		if ((0 <= fValue) && (fValue <= 89))
			m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_ATTACKINT_VERYFAST);
		else if ((90 <= fValue) && (fValue <= 110))
			m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_ATTACKINT_FAST);
		else if ((111 <= fValue) && (fValue <= 130))
			m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_ATTACKINT_NORMAL);
		else if ((131 <= fValue) && (fValue <= 150))
			m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_ATTACKINT_SLOW);
		else
			m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_ATTACKINT_VERYSLOW);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CWhite);
		ctx.IncIndex();
	}

	// 공격시간 감소 없어짐..

	if (spItem->pItemBasic->siAttackRange != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_ATTACKRANGE, (float) spItem->pItemBasic->siAttackRange / 10.0f);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CWhite);
		ctx.IncIndex();
	}

	if (spItem->pItemExt->siHitRate != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_HITRATE_OVER, spItem->pItemExt->siHitRate);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CWhite);
		ctx.IncIndex();
	}

	if (spItem->pItemExt->siEvationRate != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_AVOIDRATE_OVER, spItem->pItemExt->siEvationRate);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CWhite);
		ctx.IncIndex();
	}

	if (spItem->pItemBasic->siWeight != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_WEIGHT, spItem->pItemBasic->siWeight * 0.1f);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CWhite);
		ctx.IncIndex();
	}

	if (spItem->pItemBasic->siMaxDurability + spItem->pItemExt->siMaxDurability != 1)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(
			IDS_TOOLTIP_MAX_DURABILITY, spItem->pItemBasic->siMaxDurability + spItem->pItemExt->siMaxDurability);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CWhite);
		ctx.IncIndex();

		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_CUR_DURABILITY, spItem->iDurability);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CWhite);
		ctx.IncIndex();
	}

	if (spItem->pItemBasic->siDefense + spItem->pItemExt->siDefense != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_DEFENSE, spItem->pItemBasic->siDefense + spItem->pItemExt->siDefense);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CWhite);
		ctx.IncIndex();
	}

	if (spItem->pItemExt->siDefenseRateDagger != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_DEFENSE_RATE_DAGGER, spItem->pItemExt->siDefenseRateDagger);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CGreen);
		ctx.IncIndex();
	}

	if (spItem->pItemExt->siDefenseRateSword != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_DEFENSE_RATE_SWORD, spItem->pItemExt->siDefenseRateSword);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CGreen);
		ctx.IncIndex();
	}

	if (spItem->pItemExt->siDefenseRateBlow != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_DEFENSE_RATE_BLOW, spItem->pItemExt->siDefenseRateBlow);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CGreen);
		ctx.IncIndex();
	}

	if (spItem->pItemExt->siDefenseRateAxe != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_DEFENSE_RATE_AXE, spItem->pItemExt->siDefenseRateAxe);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CGreen);
		ctx.IncIndex();
	}

	if (spItem->pItemExt->siDefenseRateSpear != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_DEFENSE_RATE_SPEAR, spItem->pItemExt->siDefenseRateSpear);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CGreen);
		ctx.IncIndex();
	}

	if (spItem->pItemExt->siDefenseRateArrow != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_DEFENSE_RATE_ARROW, spItem->pItemExt->siDefenseRateArrow);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CGreen);
		ctx.IncIndex();
	}

	if (spItem->pItemExt->byDamageFire != 0) // 화염속성
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_ATTRMAGIC1, spItem->pItemExt->byDamageFire);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CGreen);
		ctx.IncIndex();
	}

	if (spItem->pItemExt->byDamageIce != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_ATTRMAGIC2, spItem->pItemExt->byDamageIce);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CGreen);
		ctx.IncIndex();
	}

	if (spItem->pItemExt->byDamageThuner != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_ATTRMAGIC3, spItem->pItemExt->byDamageThuner);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CGreen);
		ctx.IncIndex();
	}

	if (spItem->pItemExt->byDamagePoison != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_ATTRMAGIC4, spItem->pItemExt->byDamagePoison);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CGreen);
		ctx.IncIndex();
	}

	if (spItem->pItemExt->byStillHP != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_ATTRMAGIC5, spItem->pItemExt->byStillHP);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CGreen);
		ctx.IncIndex();
	}

	if (spItem->pItemExt->byDamageMP != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_ATTRMAGIC6, spItem->pItemExt->byDamageMP);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CGreen);
		ctx.IncIndex();
	}

	if (spItem->pItemExt->byStillMP != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_ATTRMAGIC7, spItem->pItemExt->byStillMP);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CGreen);
		ctx.IncIndex();
	}

	if (spItem->pItemExt->siBonusStr != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_BONUSSTR, spItem->pItemExt->siBonusStr);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CGreen);
		ctx.IncIndex();
	}

	if (spItem->pItemExt->siBonusSta != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_BONUSSTA, spItem->pItemExt->siBonusSta);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CGreen);
		ctx.IncIndex();
	}

	if (spItem->pItemExt->siBonusHP != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_BONUSHP, spItem->pItemExt->siBonusHP);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CGreen);
		ctx.IncIndex();
	}

	if (spItem->pItemExt->siBonusDex != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_BONUSDEX, spItem->pItemExt->siBonusDex);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CGreen);
		ctx.IncIndex();
	}

	if (spItem->pItemExt->siBonusMSP != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_BONUSWIZ, spItem->pItemExt->siBonusMSP);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CGreen);
		ctx.IncIndex();
	}

	if (spItem->pItemExt->siBonusInt != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_BONUSINT, spItem->pItemExt->siBonusInt);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CGreen);
		ctx.IncIndex();
	}

	if (spItem->pItemExt->siBonusMagicAttak != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_BONUSMAGICATTACK, spItem->pItemExt->siBonusMagicAttak);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CGreen);
		ctx.IncIndex();
	}

	if (spItem->pItemExt->siRegistFire != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_REGISTFIRE, spItem->pItemExt->siRegistFire);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CGreen);
		ctx.IncIndex();
	}

	if (spItem->pItemExt->siRegistIce != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_REGISTICE, spItem->pItemExt->siRegistIce);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CGreen);
	}

	if (spItem->pItemExt->siRegistElec != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_REGISTELEC, spItem->pItemExt->siRegistElec);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CGreen);
		ctx.IncIndex();
	}

	if (spItem->pItemExt->siRegistMagic != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_REGISTMAGIC, spItem->pItemExt->siRegistMagic);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CGreen);
		ctx.IncIndex();
	}

	if (spItem->pItemExt->siRegistPoison != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_REGISTPOISON, spItem->pItemExt->siRegistPoison);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CGreen);
		ctx.IncIndex();
	}

	if (spItem->pItemExt->siRegistCurse != 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_REGISTCURSE, spItem->pItemExt->siRegistCurse);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		m_pStr[iIndex]->SetColor(m_CGreen);
		ctx.IncIndex();
	}

	if (/*(spItem->pItemBasic->byAttachPoint == ITEM_LIMITED_EXHAUST) &&*/ spItem->pItemBasic->cNeedLevel + spItem->pItemExt->siNeedLevel
		> 1)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(
			IDS_TOOLTIP_NEEDLEVEL, spItem->pItemBasic->cNeedLevel + spItem->pItemExt->siNeedLevel);

		if (SetTooltipTextColor(CGameBase::s_pPlayer->m_InfoBase.iLevel, spItem->pItemBasic->cNeedLevel + spItem->pItemExt->siNeedLevel))
			m_pStr[iIndex]->SetColor(m_CWhite);
		else
			m_pStr[iIndex]->SetColor(m_CRed);
		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
		ctx.IncIndex();
	}

	if ((spItem->pItemBasic->byNeedRank + spItem->pItemExt->siNeedRank) > 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_NEEDRANK, spItem->pItemBasic->byNeedRank + spItem->pItemExt->siNeedRank);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);

		if (SetTooltipTextColor(pInfoExt->iRank, spItem->pItemBasic->byNeedRank + spItem->pItemExt->siNeedRank))
			m_pStr[iIndex]->SetColor(m_CWhite);
		else
			m_pStr[iIndex]->SetColor(m_CRed);
		ctx.IncIndex();
	}

	if ((spItem->pItemBasic->byNeedTitle + spItem->pItemExt->siNeedTitle) > 0)
	{
		m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_NEEDTITLE,
			// TODO: Use title name here.
			std::to_string(spItem->pItemBasic->byNeedTitle + spItem->pItemExt->siNeedTitle));

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);

		if (SetTooltipTextColor(pInfoExt->iTitle, spItem->pItemBasic->byNeedTitle + spItem->pItemExt->siNeedTitle))
			m_pStr[iIndex]->SetColor(m_CWhite);
		else
			m_pStr[iIndex]->SetColor(m_CRed);

		ctx.IncIndex();
	}

	iNeedValue = spItem->pItemBasic->byNeedStrength;
	if (iNeedValue != 0)
		iNeedValue += spItem->pItemExt->siNeedStrength;
	if (iNeedValue > 0)
	{
		std::string szReduce;
		if (spItem->pItemExt->siNeedStrength < 0)
		{
			if (spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UNIQUE || spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UPGRADE
				|| spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UNIQUE_REVERSE
				|| spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UPGRADE_REVERSE)
				szReduce = fmt::format_text_resource(IDS_TOOLTIP_REDUCE);
		}

		m_pstdstr[iIndex] = fmt::format_text_resource(
			IDS_TOOLTIP_NEEDSTRENGTH, spItem->pItemBasic->byNeedStrength + spItem->pItemExt->siNeedStrength, szReduce);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);

		if (SetTooltipTextColor(pInfoExt->iStrength, spItem->pItemBasic->byNeedStrength + spItem->pItemExt->siNeedStrength))
			m_pStr[iIndex]->SetColor(m_CWhite);
		else
			m_pStr[iIndex]->SetColor(m_CRed);

		ctx.IncIndex();
	}

	iNeedValue = spItem->pItemBasic->byNeedStamina;
	if (iNeedValue != 0)
		iNeedValue += spItem->pItemExt->siNeedStamina;
	if (iNeedValue > 0)
	{
		std::string szReduce;
		if (spItem->pItemExt->siNeedStamina < 0)
		{
			if (spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UNIQUE || spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UPGRADE
				|| spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UNIQUE_REVERSE
				|| spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UPGRADE_REVERSE)
				szReduce = fmt::format_text_resource(IDS_TOOLTIP_REDUCE);
		}

		m_pstdstr[iIndex] = fmt::format_text_resource(
			IDS_TOOLTIP_NEEDSTAMINA, spItem->pItemBasic->byNeedStamina + spItem->pItemExt->siNeedStamina, szReduce);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);

		if (SetTooltipTextColor(pInfoExt->iStamina, spItem->pItemBasic->byNeedStamina + spItem->pItemExt->siNeedStamina))
			m_pStr[iIndex]->SetColor(m_CWhite);
		else
			m_pStr[iIndex]->SetColor(m_CRed);

		ctx.IncIndex();
	}

	iNeedValue = spItem->pItemBasic->byNeedDexterity;
	if (iNeedValue != 0)
		iNeedValue += spItem->pItemExt->siNeedDexterity;
	if (iNeedValue > 0)
	{
		std::string szReduce;
		if (spItem->pItemExt->siNeedDexterity < 0)
		{
			if (spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UNIQUE || spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UPGRADE
				|| spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UNIQUE_REVERSE
				|| spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UPGRADE_REVERSE)
				szReduce = fmt::format_text_resource(IDS_TOOLTIP_REDUCE);
		}

		m_pstdstr[iIndex] = fmt::format_text_resource(
			IDS_TOOLTIP_NEEDDEXTERITY, spItem->pItemBasic->byNeedDexterity + spItem->pItemExt->siNeedDexterity, szReduce);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);

		if (SetTooltipTextColor(pInfoExt->iDexterity, spItem->pItemBasic->byNeedDexterity + spItem->pItemExt->siNeedDexterity))
			m_pStr[iIndex]->SetColor(m_CWhite);
		else
			m_pStr[iIndex]->SetColor(m_CRed);

		ctx.IncIndex();
	}

	iNeedValue = spItem->pItemBasic->byNeedInteli;
	if (iNeedValue != 0)
		iNeedValue += spItem->pItemExt->siNeedInteli;
	if (iNeedValue > 0)
	{
		std::string szReduce;
		if (spItem->pItemExt->siNeedInteli < 0)
		{
			if (spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UNIQUE || spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UPGRADE
				|| spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UNIQUE_REVERSE
				|| spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UPGRADE_REVERSE)
				szReduce = fmt::format_text_resource(IDS_TOOLTIP_REDUCE);
		}

		m_pstdstr[iIndex] = fmt::format_text_resource(
			IDS_TOOLTIP_NEEDINTELLI, spItem->pItemBasic->byNeedInteli + spItem->pItemExt->siNeedInteli, szReduce);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);

		if (SetTooltipTextColor(pInfoExt->iIntelligence, spItem->pItemBasic->byNeedInteli + spItem->pItemExt->siNeedInteli))
			m_pStr[iIndex]->SetColor(m_CWhite);
		else
			m_pStr[iIndex]->SetColor(m_CRed);

		ctx.IncIndex();
	}

	iNeedValue = spItem->pItemBasic->byNeedMagicAttack;
	if (iNeedValue != 0)
		iNeedValue += spItem->pItemExt->siNeedMagicAttack;
	if (iNeedValue > 0)
	{
		std::string szReduce;
		if (spItem->pItemExt->siNeedMagicAttack < 0)
		{
			if (spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UNIQUE || spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UPGRADE
				|| spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UNIQUE_REVERSE
				|| spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UPGRADE_REVERSE)
				szReduce = fmt::format_text_resource(IDS_TOOLTIP_REDUCE);
		}

		m_pstdstr[iIndex] = fmt::format_text_resource(
			IDS_TOOLTIP_NEEDMAGICATTACK, spItem->pItemBasic->byNeedMagicAttack + spItem->pItemExt->siNeedMagicAttack, szReduce);

		m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);

		if (SetTooltipTextColor(pInfoExt->iMagicAttak, spItem->pItemBasic->byNeedMagicAttack + spItem->pItemExt->siNeedMagicAttack))
			m_pStr[iIndex]->SetColor(m_CWhite);
		else
			m_pStr[iIndex]->SetColor(m_CRed);

		ctx.IncIndex();
	}

	if (spItem->pItemBasic != nullptr)
	{
		const std::string& szRemark = spItem->pItemBasic->szRemark;

		if (!szRemark.empty())
		{
			size_t totalWords = 1;
			for (char c : szRemark)
			{
				if (c == ' ')
					++totalWords;
			}

			if (totalWords >= MIN_WORDS_TO_SPLIT_DESC)
			{
				size_t wordsInFirstHalf = (totalWords + 1) / 2;
				size_t wordsSeen = 1, splitPos = std::string::npos;
				for (size_t i = 0; i < szRemark.size(); i++)
				{
					if (szRemark[i] != ' ')
						continue;

					if (++wordsSeen > wordsInFirstHalf)
					{
						splitPos = i;
						break;
					}
				}

				_ASSERT(splitPos != std::string::npos);

				m_pStr[iIndex]->SetColor(m_CWhite);
				m_pstdstr[iIndex] = fmt::format("*{}", std::string_view(szRemark.data(), splitPos));
				m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNCENTER);

				ctx.IncIndex();

				// NOTE: This doesn't perfectly match official's behaviour; they rebuild the lines, always appending a space.
				// This means that there's a guaranteed space after the word here, before the '*'
				// To replicate this behaviour, we can manually add a space in the format string, but it's not really necessary.
				m_pStr[iIndex]->SetColor(m_CWhite);
				m_pstdstr[iIndex] = fmt::format("{}*", std::string_view(szRemark.data() + splitPos, szRemark.size() - splitPos));
				m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNCENTER);

				ctx.IncIndex();
			}
			else
			{
				m_pStr[iIndex]->SetColor(m_CWhite);
				m_pstdstr[iIndex] = fmt::format("*{}*", szRemark);
				m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNCENTER);

				ctx.IncIndex();
			}
		}
	}

	if (spItem->pItemExt != nullptr)
	{
		e_ItemAttrib eTA = (e_ItemAttrib) (spItem->pItemExt->byMagicOrRare);
		switch (eTA)
		{
			case ITEM_ATTRIB_UNIQUE:
				m_pStr[iIndex]->SetColor(m_CGreen);
				m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_UNIQUE);
				m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNCENTER);
				ctx.IncIndex();
				break;

			case ITEM_ATTRIB_UPGRADE:
				if (spItem->pItemBasic != nullptr && spItem->pItemExt != nullptr)
				{
					const int iItemGrade = std::min(3, spItem->pItemBasic->byGrade + spItem->pItemExt->bySoulBind);
					if (iItemGrade == ITEM_GRADE_LOW_CLASS)
						m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_LOW_CLASS);
					else if (iItemGrade == ITEM_GRADE_MIDDLE_CLASS)
						m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_MIDDLE_CLASS);
					else if (iItemGrade == ITEM_GRADE_HIGH_CLASS)
						m_pstdstr[iIndex] = fmt::format_text_resource(IDS_TOOLTIP_HIGH_CLASS);
					else
						break;

					m_pStr[iIndex]->SetColor(m_CGreen);
					m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
					ctx.IncIndex();
				}
				break;

			default:
				break;
		}
	}
}

void CUIImageTooltipDlg::DisplayTooltipsEnable(int xpos, int ypos, __IconItemSkill* spItem, bool bPrice, bool bBuy)
{
	if (spItem == nullptr)
	{
		ResetItem();
		return;
	}

	if (!IsVisible())
		SetVisible(true);

	if (IsItemChanged(spItem))
	{
		UpdateItem(spItem);

		m_iTooltipNum = CalcTooltipStringNumAndWrite(spItem, bPrice, bBuy);
		SetPosSomething(xpos, ypos);
	}
	else if (m_iPosXBack != xpos || m_iPosYBack != ypos)
	{
		SetPosSomething(xpos, ypos);
	}

	Render();
}

bool CUIImageTooltipDlg::IsItemChanged(const __IconItemSkill* spItem) const
{
	if (m_dwID_Basic == spItem->pItemBasic->dwID && m_dwID_Ext == spItem->pItemExt->dwID && m_iDurability == spItem->iDurability
		&& m_iCount == spItem->iCount)
		return false;

	return true;
}

void CUIImageTooltipDlg::UpdateItem(const __IconItemSkill* spItem)
{
	if (spItem == nullptr || spItem->pItemBasic == nullptr)
	{
		ResetItem();
		return;
	}

	m_dwID_Basic = spItem->pItemBasic->dwID;

	if (spItem->pItemExt != nullptr)
		m_dwID_Ext = spItem->pItemExt->dwID;
	else
		m_dwID_Ext = 0;

	m_iCount      = spItem->iCount;
	m_iDurability = spItem->iDurability;
}

void CUIImageTooltipDlg::ResetItem()
{
	m_dwID_Basic  = 0;
	m_dwID_Ext    = 0;
	m_iCount      = 0;
	m_iDurability = 0;
}

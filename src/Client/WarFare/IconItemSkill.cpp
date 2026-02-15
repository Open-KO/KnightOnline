#include "StdAfx.h"
#include "IconItemSkill.h"
#include "GameDef.h"
#include "N3UIIcon.h"
#include "N3UIWndBase.h"

__IconItemSkill::__IconItemSkill()
{
	pUIIcon     = nullptr;
	pItemBasic  = nullptr;
	pItemExt    = nullptr;
	iCount      = 0;
	iDurability = 0;
	pSkill      = nullptr;
}

int __IconItemSkill::GetItemID() const
{
	if (pItemBasic == nullptr)
		return 0;

	int iItemID = static_cast<int>(pItemBasic->dwID / 1000 * 1000);
	if (pItemExt != nullptr)
		iItemID += static_cast<int>(pItemExt->dwID % 1000);

	return iItemID;
}

int __IconItemSkill::GetBuyPrice() const
{
	if (pItemBasic == nullptr || pItemExt == nullptr)
		return 0;

	return pItemBasic->iPrice * pItemExt->siPriceMultiply;
}

int __IconItemSkill::GetSellPrice(bool bHasPremium /*= false*/) const
{
	if (pItemBasic == nullptr || pItemExt == nullptr)
		return 0;

	constexpr int PREMIUM_RATIO = 4;
	constexpr int NORMAL_RATIO  = 6;

	int iSellPrice              = pItemBasic->iPrice * pItemExt->siPriceMultiply;

	if (pItemBasic->iSaleType != SALE_TYPE_FULL)
	{
		if (bHasPremium)
			iSellPrice /= PREMIUM_RATIO;
		else
			iSellPrice /= NORMAL_RATIO;
	}

	if (iSellPrice < 1)
		iSellPrice = 1;

	return iSellPrice;
}

bool __IconItemSkill::IsStackable() const
{
	if (pItemBasic == nullptr)
		return false;

	return pItemBasic->byContable == UIITEM_TYPE_COUNTABLE || pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL;
}

void __IconItemSkill::CreateIcon(const std::string& szFN, CN3UIBase* pParent, uint32_t dwStyle, float fUVAspect)
{
	szIconFN = szFN;

	delete pUIIcon;
	pUIIcon = new CN3UIIcon();
	pUIIcon->Init(pParent);

	pUIIcon->SetTex(szFN);
	pUIIcon->SetUVRect(0, 0, fUVAspect, fUVAspect);
	pUIIcon->SetStyle(dwStyle);
	pUIIcon->SetVisible(true);
}

__IconItemSkill* __IconItemSkill::Clone(CN3UIBase* pParent)
{
	__IconItemSkill* spItemNew = new __IconItemSkill();

	spItemNew->szIconFN        = szIconFN;
	spItemNew->pItemBasic      = pItemBasic;
	spItemNew->pItemExt        = pItemExt;
	spItemNew->pSkill          = pSkill;
	spItemNew->iCount          = iCount;
	spItemNew->iDurability     = iDurability;

	if (pUIIcon != nullptr)
	{
		spItemNew->pUIIcon = new CN3UIIcon();
		spItemNew->pUIIcon->Init(pParent);
		spItemNew->pUIIcon->SetTex(szIconFN);
		spItemNew->pUIIcon->SetUVRect(*pUIIcon->GetUVRect());
		spItemNew->pUIIcon->SetStyle(pUIIcon->GetStyle());
		spItemNew->pUIIcon->SetVisible(pUIIcon->IsVisible());
	}

	return spItemNew;
}

#include "StdAfx.h"
#include "IconItemSkill.h"
#include "GameDef.h"

__IconItemSkill::__IconItemSkill()
{
	pItemBasic  = nullptr;
	pItemExt    = nullptr;
	iCount      = 0;
	iDurability = 0;
	pSkill      = nullptr;
	pUIIcon     = nullptr;
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

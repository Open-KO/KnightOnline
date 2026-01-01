#pragma once

class CN3UIIcon;
struct __TABLE_ITEM_BASIC;
struct __TABLE_ITEM_EXT;
struct __TABLE_UPC_SKILL;
struct	__IconItemSkill
{
	CN3UIIcon*	pUIIcon;
	std::string	szIconFN;

	union
	{
		struct
		{
			__TABLE_ITEM_BASIC*	pItemBasic;
			__TABLE_ITEM_EXT*	pItemExt;
			int					iCount;
			int					iDurability;	// 내구력
		};

		__TABLE_UPC_SKILL* pSkill;			// Skill.. ^^
	};

	int GetBuyPrice() const;
	int GetSellPrice(bool bHasPremium = false) const;
};

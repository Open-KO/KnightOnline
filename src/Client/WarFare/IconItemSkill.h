#ifndef CLIENT_WARFARE_ICONITEMSKILL_H
#define CLIENT_WARFARE_ICONITEMSKILL_H

#pragma once

#include <N3Base/N3UIDef.h>

#include <cstdint>
#include <string>

class CN3UIBase;
class CN3UIIcon;
struct __TABLE_ITEM_BASIC;
struct __TABLE_ITEM_EXT;
struct __TABLE_UPC_SKILL;
struct __IconItemSkill
{
	CN3UIIcon* pUIIcon;
	std::string szIconFN;

	union
	{
		struct
		{
			__TABLE_ITEM_BASIC* pItemBasic;
			__TABLE_ITEM_EXT* pItemExt;
			int iCount;
			int iDurability;       // 내구력
		};

		__TABLE_UPC_SKILL* pSkill; // Skill.. ^^
	};

	__IconItemSkill();
	__IconItemSkill(const __IconItemSkill& src) = delete;
	int GetItemID() const;
	int GetBuyPrice() const;
	int GetSellPrice(bool bHasPremium = false) const;
	bool IsStackable() const;
	void CreateIcon(const std::string& szFN, CN3UIBase* pParent, uint32_t dwStyle = UISTYLE_ICON_ITEM | UISTYLE_ICON_CERTIFICATION_NEED,
		float fUVAspect = 45.0f / 64.0f);
	__IconItemSkill* Clone(CN3UIBase* pParent);
};

#endif // CLIENT_WARFARE_ICONITEMSKILL_H

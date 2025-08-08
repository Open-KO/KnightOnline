#include "StdAfx.h"
#include "ClientResourceFormatter.h"
#include "GameBase.h"

bool fmt::resource_helper::get_from_texts(uint32_t resourceId, std::string& fmtStr)
{
	__TABLE_TEXTS* text = CGameBase::s_pTbl_Texts.Find(resourceId);
	if (text == nullptr)
		return false;

	fmtStr = text->szText;
	return true;
}

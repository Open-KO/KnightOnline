#include "StdAfx.h"
#include "ClientResourceFormatter.h"
#include "GameBase.h"

bool fmt::resource_helper::get_from_texts(uint32_t resourceId, std::string& fmtStr)
{
	__TABLE_TEXTS* text = CGameBase::s_pTbl_Texts.Find(resourceId);
	if (text == nullptr)
	{
#if defined(_N3GAME)
		CLogWriter::Write("get_from_texts({}) failed - resource missing in Texts TBL.",
			resourceId);
#endif

		return false;
	}

	fmtStr = text->szText;
	return true;
}

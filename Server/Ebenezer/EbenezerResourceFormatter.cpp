#include "StdAfx.h"
#include "EbenezerDlg.h"
#include "EbenezerResourceFormatter.h"

bool fmt::resource_helper::get_from_db(uint32_t resourceId, std::string& fmtStr)
{
	CEbenezerDlg* instance = CEbenezerDlg::GetInstance();
	if (instance == nullptr)
	{
		spdlog::error("get_from_db({}) failed - server instance unavailable.",
			resourceId);
		return false;
	}

	model::ServerResource* serverResource = instance->m_ServerResourceTableMap.GetData(resourceId);
	if (serverResource == nullptr)
	{
		spdlog::error("get_from_db({}) failed - resource not found.",
			resourceId);
		return false;
	}

	fmtStr = serverResource->Resource;
	return true;
}

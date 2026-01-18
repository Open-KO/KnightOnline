#include "pch.h"
#include "ItemManagerLogger.h"

namespace ItemManager
{

ItemManagerLogger::ItemManagerLogger() : Logger(logger::ItemManager)
{
}

void ItemManagerLogger::SetupExtraLoggers(CIni& ini,
	std::shared_ptr<spdlog::details::thread_pool> threadPool, const std::filesystem::path& baseDir)
{
	SetupExtraLogger(ini, threadPool, baseDir, logger::ItemManagerItem, ini::ITEM_LOG_FILE);
	SetupExtraLogger(ini, threadPool, baseDir, logger::ItemManagerExp, ini::EXP_LOG_FILE);
}

} // namespace ItemManager

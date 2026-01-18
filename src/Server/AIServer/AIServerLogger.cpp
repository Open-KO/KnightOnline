#include "pch.h"
#include "AIServerLogger.h"

namespace AIServer
{

AIServerLogger::AIServerLogger() : Logger(logger::AIServer)
{
}

void AIServerLogger::SetupExtraLoggers(CIni& ini,
	std::shared_ptr<spdlog::details::thread_pool> threadPool, const std::filesystem::path& baseDir)
{
	SetupExtraLogger(ini, threadPool, baseDir, logger::AIServerItem, ini::ITEM_LOG_FILE);
	SetupExtraLogger(ini, threadPool, baseDir, logger::AIServerUser, ini::USER_LOG_FILE);
}

} // namespace AIServer

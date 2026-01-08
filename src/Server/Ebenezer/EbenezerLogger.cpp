#include "pch.h"
#include "EbenezerLogger.h"

namespace Ebenezer
{

EbenezerLogger::EbenezerLogger() : Logger(logger::Ebenezer)
{
}

void EbenezerLogger::SetupExtraLoggers(CIni& ini,
	std::shared_ptr<spdlog::details::thread_pool> threadPool, const std::filesystem::path& baseDir)
{
	SetupExtraLogger(ini, threadPool, baseDir, logger::EbenezerEvent, ini::EVENT_LOG_FILE);
	SetupExtraLogger(ini, threadPool, baseDir, logger::EbenezerRegion, ini::REGION_LOG_FILE);
}

} // namespace Ebenezer

#pragma once

#include <shared-server/logger.h>

class CIni;
class ItemManagerLogger : public logger::Logger
{
public:
	ItemManagerLogger();
	void SetupExtraLoggers(
		CIni& ini,
		std::shared_ptr<spdlog::details::thread_pool> threadPool,
		const std::filesystem::path& baseDir) override;
};

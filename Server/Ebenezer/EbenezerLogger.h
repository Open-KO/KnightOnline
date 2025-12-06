#pragma once

#include <shared-server/logger.h>

class CIni;
class EbenezerLogger : public logger::Logger
{
public:
	EbenezerLogger();
	void SetupExtraLoggers(
		CIni& ini,
		std::shared_ptr<spdlog::details::thread_pool> threadPool,
		const std::filesystem::path& baseDir) override;
};

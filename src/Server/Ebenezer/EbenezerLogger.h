#ifndef SERVER_EBENEZER_EBENEZERLOGGER_H
#define SERVER_EBENEZER_EBENEZERLOGGER_H

#pragma once

#include <shared-server/logger.h>

class CIni;

namespace Ebenezer
{

class EbenezerLogger : public logger::Logger
{
public:
	EbenezerLogger();
	void SetupExtraLoggers(CIni& ini, std::shared_ptr<spdlog::details::thread_pool> threadPool,
		const std::filesystem::path& baseDir) override;
};

} // namespace Ebenezer

#endif // SERVER_EBENEZER_EBENEZERLOGGER_H

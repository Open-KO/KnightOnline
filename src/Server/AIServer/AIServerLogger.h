#ifndef SERVER_AISERVER_AISERVERLOGGER_H
#define SERVER_AISERVER_AISERVERLOGGER_H

#pragma once

#include <shared-server/logger.h>

class CIni;

namespace AIServer
{

class AIServerLogger : public logger::Logger
{
public:
	AIServerLogger();

	void SetupExtraLoggers(CIni& ini, std::shared_ptr<spdlog::details::thread_pool> threadPool,
		const std::filesystem::path& baseDir) override;
};

} // namespace AIServer

#endif // SERVER_AISERVER_AISERVERLOGGER_H

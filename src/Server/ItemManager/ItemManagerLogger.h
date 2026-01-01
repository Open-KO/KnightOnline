#ifndef SERVER_ITEMMANAGER_ITEMMANAGERLOGGER_H
#define SERVER_ITEMMANAGER_ITEMMANAGERLOGGER_H

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

#endif // SERVER_ITEMMANAGER_ITEMMANAGERLOGGER_H

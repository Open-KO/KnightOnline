#include "pch.h"
#include "VersionManagerApp.h"
#include <shared-server/logger.h>
#include <spdlog/spdlog.h>
#include <iostream>

int main(int argc, char* argv[])
{
	logger::Logger logger(logger::VersionManager);
	return AppThread::main<VersionManagerApp>(argc, argv, logger);
}

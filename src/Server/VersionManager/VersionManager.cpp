#include "pch.h"
#include "VersionManagerApp.h"
#include <shared-server/logger.h>

int main(int argc, char* argv[])
{
	using namespace VersionManager;

	logger::Logger logger(logger::VersionManager);
	return AppThread::main<VersionManagerApp>(argc, argv, logger);
}

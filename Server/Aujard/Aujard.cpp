#include "pch.h"
#include "AujardApp.h"

#include <shared-server/logger.h>

int main()
{
	logger::Logger logger(logger::Aujard);
	return AppThread::main<AujardApp>(logger);
}

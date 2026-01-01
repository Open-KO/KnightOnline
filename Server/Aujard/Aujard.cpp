#include "AujardApp.h"
#include "pch.h"

#include <shared-server/logger.h>

int main(int argc, char *argv[])
{
    logger::Logger logger(logger::Aujard);
    return AppThread::main<AujardApp>(argc, argv, logger);
}

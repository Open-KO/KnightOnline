#include "EbenezerApp.h"
#include "EbenezerLogger.h"
#include "pch.h"

int main(int argc, char *argv[])
{
    EbenezerLogger logger;
    return AppThread::main<EbenezerApp>(argc, argv, logger);
}

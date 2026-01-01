#include "AIServerApp.h"
#include "AIServerLogger.h"
#include "pch.h"

int main(int argc, char *argv[])
{
    AIServerLogger logger;
    return AppThread::main<AIServerApp>(argc, argv, logger);
}

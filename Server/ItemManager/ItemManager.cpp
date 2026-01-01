#include "ItemManagerApp.h"
#include "ItemManagerLogger.h"
#include "pch.h"

int main(int argc, char *argv[])
{
    ItemManagerLogger logger;
    return AppThread::main<ItemManagerApp>(argc, argv, logger);
}

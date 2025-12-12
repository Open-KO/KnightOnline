#include "pch.h"
#include "ItemManagerApp.h"
#include "ItemManagerLogger.h"

int main(int argc, char* argv[])
{
	ItemManagerLogger logger;
	return AppThread::main<ItemManagerApp>(argc, argv, logger);
}

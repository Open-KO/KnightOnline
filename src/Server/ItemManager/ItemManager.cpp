#include "pch.h"
#include "ItemManagerApp.h"
#include "ItemManagerLogger.h"

int main(int argc, char* argv[])
{
	using namespace ItemManager;

	ItemManagerLogger logger;
	return AppThread::main<ItemManagerApp>(argc, argv, logger);
}

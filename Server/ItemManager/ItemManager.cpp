#include "pch.h"
#include "ItemManagerApp.h"
#include "ItemManagerLogger.h"

int main()
{
	ItemManagerLogger logger;
	return AppThread::main<ItemManagerApp>(logger);
}

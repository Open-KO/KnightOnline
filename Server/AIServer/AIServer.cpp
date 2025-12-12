#include "pch.h"
#include "AIServerApp.h"
#include "AIServerLogger.h"

int main(int argc, char* argv[])
{
	AIServerLogger logger;
	return AppThread::main<AIServerApp>(argc, argv, logger);
}

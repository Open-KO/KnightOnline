#include "pch.h"
#include "AIServerApp.h"
#include "AIServerLogger.h"

int main(int argc, char* argv[])
{
	using namespace AIServer;

	AIServerLogger logger;
	return AppThread::main<AIServerApp>(argc, argv, logger);
}

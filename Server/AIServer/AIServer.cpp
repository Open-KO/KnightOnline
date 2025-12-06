#include "pch.h"
#include "AIServerApp.h"
#include "AIServerLogger.h"

int main()
{
	AIServerLogger logger;
	return AppThread::main<AIServerApp>(logger);
}

#include "pch.h"
#include "EbenezerApp.h"
#include "EbenezerLogger.h"

int main(int argc, char* argv[])
{
	EbenezerLogger logger;
	return AppThread::main<EbenezerApp>(argc, argv, logger);
}

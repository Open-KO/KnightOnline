#include "pch.h"
#include "EbenezerApp.h"
#include "EbenezerLogger.h"

int main()
{
	EbenezerLogger logger;
	return AppThread::main<EbenezerApp>(logger);
}

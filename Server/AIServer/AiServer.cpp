// AiServer.cpp : contains the main() function to start the server
//

#include "pch.h"
#include "AiServerInstance.h"

#include <signal.h>
#include <spdlog/spdlog.h>

AIServerLogger _logger;
AiServerInstance* appThread;

void signalHandler(int signalNumber)
{
	spdlog::info("AiServer::signalHandler: Caught {}", signalNumber);
	switch (signalNumber)
	{
	case SIGINT:
	case SIGABRT:
	case SIGTERM:
		// Shutdown the application thread
		if (appThread != nullptr && appThread->IsRunning())
		{
			appThread->shutdown();
		}
		break;
	}
	
	signal(signalNumber, signalHandler);
}

int main()
{
	int retCode = EXIT_SUCCESS;
	// catch interrupt signals for graceful shutdowns.
	signal(SIGINT, signalHandler);
	signal(SIGABRT, signalHandler);
	signal(SIGTERM, signalHandler);
	
	// Logger config/setup is handled by the server instance.
	// We just instantiate it early for signal handling.
	appThread = new AiServerInstance(_logger);
	appThread->start();

	try
	{
		// We keep the main() thread alive to catch interrupt signals and call shutdown
		while (appThread != nullptr && !appThread->IsStopped())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	} catch (const std::exception& ex)
	{
		spdlog::error("AiServer::main: Exception caught: {}", ex.what());
		retCode = EXIT_FAILURE;
	} catch (...)
	{
		spdlog::error("AiServer::main: Unknown exception caught");
		retCode = EXIT_FAILURE;
	}

	delete appThread;
	
	exit(retCode);
}

#include "StdAfxBase.h"
#include <spdlog/spdlog.h>

void DebugStringToOutput(const std::string_view logMessage)
{
#ifdef _WIN32
	constexpr std::string_view NewLine = "\r\n";

	std::string outputMessage;
	outputMessage.reserve(logMessage.size() + NewLine.size());
	outputMessage  = logMessage;
	outputMessage += NewLine;
	OutputDebugStringA(outputMessage.c_str());
#endif

	spdlog::trace(logMessage);
}

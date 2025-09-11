#include "StdAfx.h"
#include "EbenezerDlg.h"
#include "User.h"

#include <djb2/djb2_hasher.h>
#include <shared/StringUtils.h>
#include <spdlog/spdlog.h>

#include <sstream>
#include <vector>
#include <string>

// Split string into parts. Delimit by whitespace. Empty spaces are ignored.
// This:
// +cmd arg1    arg2     arg3
// Will become:
// [0] = +cmd, [1] = arg1, [2] = arg3
static std::vector<std::string> SplitCommandIntoParts(const char* command)
{
	std::vector<std::string> parts;

	std::istringstream ss(command);
	std::string part;
	while (ss >> part)
		parts.push_back(std::move(part));

	return parts;
}

void CEbenezerDlg::OperationMessage(const char* command, CUser* pSrcUser)
{
	std::vector<std::string> parts = SplitCommandIntoParts(command);
	if (parts.empty())
		return;

	strtolower(parts[0]);

	auto key = hashing::djb2::hash(parts[0]);
	try
	{
		switch (key)
		{
			// +zonechange: {int: zoneId} [float: x] [float: z]
			// NOTE: Coordinates are unofficial.
			case "+zonechange"_djb2:
				OperationMessage_ZoneChange(command, pSrcUser, parts);
				break;
		}
	}
	catch (const std::invalid_argument& ex)
	{
		if (pSrcUser != nullptr)
		{
			spdlog::warn(
				"EbenezerDlg::OperationMessage: argument could not be parsed from GM [charId={} command={}, exception={}]",
				pSrcUser->m_pUserData->m_id, command, ex.what());
		}
		else
		{
			spdlog::warn(
				"EbenezerDlg::OperationMessage: argument could not be parsed from server [command={}, exception={}]",
				command, ex.what());
		}
	}
	catch (const std::out_of_range& ex)
	{
		if (pSrcUser != nullptr)
		{
			spdlog::warn(
				"EbenezerDlg::OperationMessage: parsed argument out of range from GM [charId={} command={}, exception={}]",
				pSrcUser->m_pUserData->m_id, command, ex.what());
		}
		else
		{
			spdlog::warn(
				"EbenezerDlg::OperationMessage: parsed argument out of range from server [command={}, exception={}]",
				command, ex.what());
		}
	}
}

// +zonechange: {int: zoneId} [float: x] [float: z]
// NOTE: Coordinates are unofficial.
void CEbenezerDlg::OperationMessage_ZoneChange(const char* command, CUser* pSrcUser, const std::vector<std::string>& parts)
{
	// Requires a user.
	if (pSrcUser == nullptr
		|| parts.size() < 2)
		return;

	int zoneId = std::stoi(parts[1]);
	float x = pSrcUser->m_pUserData->m_curx;
	float z = pSrcUser->m_pUserData->m_curz;

	if (parts.size() >= 4)
	{
		x = static_cast<float>(std::stof(parts[2]));
		z = static_cast<float>(std::stof(parts[3]));
	}

	pSrcUser->ZoneChange(zoneId, x, z);
}

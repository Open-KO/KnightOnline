#include "StdAfx.h"
#include "OperationMessage.h"
#include "EbenezerDlg.h"
#include "User.h"

#include <djb2/djb2_hasher.h>
#include <shared/StringUtils.h>
#include <spdlog/spdlog.h>

#include <sstream>

// Split string into parts. Delimit by whitespace. Empty spaces are ignored.
// This:
// +cmd arg1    arg2     arg3
// Will become:
// [0] = +cmd, [1] = arg1, [2] = arg3
static std::vector<std::string> SplitCommandIntoParts(const std::string& command)
{
	std::vector<std::string> parts;

	std::istringstream ss(command);
	std::string part;
	while (ss >> part)
		parts.push_back(part);

	return parts;
}

OperationMessage::OperationMessage(CEbenezerDlg* main, CUser* srcUser)
	: _main(main), _srcUser(srcUser)
{
}

void OperationMessage::ParseGM(const std::string_view command)
{
	_command.assign(command.data(), command.length());
	_parts = SplitCommandIntoParts(_command);

	if (_parts.empty())
		return;

	strtolower(_parts[0]);

	auto key = hashing::djb2::hash(_parts[0]);
	try
	{
		switch (key)
		{
			// +zonechange: {int: zoneId} [float: x] [float: z]
			// NOTE: Coordinates are unofficial.
			case "+zonechange"_djb2:
				ZoneChange();
				break;
		}
	}
	catch (const std::invalid_argument& ex)
	{
		LogInvalidArgumentException("OperationMessage::ParseGM", ex);
	}
	catch (const std::out_of_range& ex)
	{
		LogOutOfRangeException("OperationMessage::ParseGM", ex);
	}
}

// +zonechange: {int: zoneId} [float: x] [float: z]
// NOTE: Coordinates are unofficial.
void OperationMessage::ZoneChange()
{
	// Requires a user.
	if (_srcUser == nullptr
		|| GetArgCount() < 1)
		return;

	int zoneId = ParseInt(1);
	float x = _srcUser->m_pUserData->m_curx;
	float z = _srcUser->m_pUserData->m_curz;

	if (GetArgCount() >= 3)
	{
		x = ParseFloat(2);
		z = ParseFloat(3);
	}

	_srcUser->ZoneChange(zoneId, x, z);
}

// Returns the number of arguments, excluding the command name.
size_t OperationMessage::GetArgCount() const
{
	return _parts.size() - 1;
}

int OperationMessage::ParseInt(size_t partIndex) const
{
	if (partIndex >= _parts.size())
		throw std::invalid_argument(fmt::format("argument {} not supplied", partIndex));

	return std::stoi(_parts[partIndex]);
}

float OperationMessage::ParseFloat(size_t partIndex) const
{
	if (partIndex >= _parts.size())
		throw std::invalid_argument(fmt::format("argument {} not supplied", partIndex));

	return std::stof(_parts[partIndex]);
}

void OperationMessage::LogInvalidArgumentException(const std::string_view source, const std::invalid_argument& ex) const
{
	if (_srcUser != nullptr)
	{
		spdlog::warn(
			"{}: argument could not be parsed from GM [charId={} command='{}' exception='{}']",
			source, _srcUser->m_pUserData->m_id, _command, ex.what());
	}
	else
	{
		spdlog::warn(
			"{}: argument could not be parsed from server [command='{}' exception='{}']",
			source, _command, ex.what());
	}
}

void OperationMessage::LogOutOfRangeException(const std::string_view source, const std::out_of_range& ex) const
{
	if (_srcUser != nullptr)
	{
		spdlog::warn(
			"{}: parsed argument out of range from GM [charId={} command='{}' exception='{}']",
			source, _srcUser->m_pUserData->m_id, _command, ex.what());
	}
	else
	{
		spdlog::warn(
			"{}: parsed argument out of range from server [command='{}' exception='{}']",
			source, _command, ex.what());
	}
}

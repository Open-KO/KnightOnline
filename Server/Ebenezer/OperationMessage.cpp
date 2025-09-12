#include "StdAfx.h"
#include "OperationMessage.h"
#include "EbenezerDlg.h"
#include "User.h"

#include <djb2/djb2_hasher.h>
#include <shared/StringUtils.h>
#include <spdlog/spdlog.h>

#include <sstream>

OperationMessage::OperationMessage(CEbenezerDlg* main, CUser* srcUser)
	: _main(main), _srcUser(srcUser)
{
}

void OperationMessage::ParseGM(const std::string_view command)
{
	size_t key = 0;
	if (!ParseCommand(command, key))
		return;

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

	int zoneId = ParseInt(0);
	float x = _srcUser->m_pUserData->m_curx;
	float z = _srcUser->m_pUserData->m_curz;

	if (GetArgCount() >= 3)
	{
		x = ParseFloat(1);
		z = ParseFloat(2);
	}

	_srcUser->ZoneChange(zoneId, x, z);
}

bool OperationMessage::ParseCommand(const std::string_view command, size_t& key)
{
	_command.assign(command.data(), command.length());
	_args.clear();

	// Split string into parts.
	// Delimit by whitespace.
	// Empty spaces are ignored.
	// This:
	// +cmd arg1    arg2     arg3
	// Will become:
	// [0] = +cmd, [1] = arg1, [2] = arg3
	std::istringstream ss(_command);
	std::string part;
	while (ss >> part)
		_args.push_back(part);

	// Expect at least one "argument" (the command name).
	if (_args.empty())
		return false;

	// Extract and transform the command name to lowercase.
	std::string& commandNameLowercase = _args.front();
	strtolower(commandNameLowercase);

	// Hash the lowercase key name for returning.
	key = hashing::djb2::hash(commandNameLowercase);

	// Strip it from the args list for consistency; we don't need it anymore.
	_args.erase(_args.begin());

	return true;
}

// Returns the number of arguments, excluding the command name.
size_t OperationMessage::GetArgCount() const
{
	return _args.size();
}

int OperationMessage::ParseInt(size_t argIndex) const
{
	if (argIndex >= _args.size())
		throw std::invalid_argument(fmt::format("argument {} not supplied", argIndex));

	return std::stoi(_args[argIndex]);
}

float OperationMessage::ParseFloat(size_t argIndex) const
{
	if (argIndex >= _args.size())
		throw std::invalid_argument(fmt::format("argument {} not supplied", argIndex));

	return std::stof(_args[argIndex]);
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

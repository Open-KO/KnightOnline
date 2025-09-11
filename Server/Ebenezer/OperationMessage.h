#pragma once

#include <vector>
#include <stdexcept>
#include <string>
#include <string_view>

class CEbenezerDlg;
class CUser;
class OperationMessage
{
public:
	OperationMessage(CEbenezerDlg* main, CUser* srcUser);
	void ParseGM(const std::string_view command);

protected:
	void ZoneChange();

	// Returns the number of arguments, excluding the command name.
	size_t GetArgCount() const;

	int ParseInt(size_t partIndex) const;
	float ParseFloat(size_t partIndex) const;

	void LogInvalidArgumentException(const std::string_view source, const std::invalid_argument& ex) const;
	void LogOutOfRangeException(const std::string_view source, const std::out_of_range& ex) const;

protected:
	CEbenezerDlg* _main;
	CUser* _srcUser;
	std::string _command;
	std::vector<std::string> _parts;
};

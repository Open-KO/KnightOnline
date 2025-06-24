#include "stdafx.h"
#include "StringUtils.h"
#include <stdarg.h>
#include <cctype>
#include <functional>
#include <algorithm>

static constexpr char WhitespaceCharsA[]		= " \t\n\r\f\v";
static constexpr wchar_t WhitespaceCharsW[]		= L" \t\n\r\f\v";

void _string_format(const std::string_view fmt, std::string* result, va_list args)
{
	char buffer[1024] = {};
	vsnprintf(buffer, sizeof(buffer), fmt.data(), args);
	*result = buffer;
}

std::string string_format(const std::string_view fmt, ...)
{
	std::string result;
	va_list ap;

	va_start(ap, fmt);
	_string_format(fmt, &result, ap);
	va_end(ap);

	return result;
}

// trim from end
std::string& rtrim(std::string& s)
{
	s.erase(s.find_last_not_of(WhitespaceCharsA) + 1);
	return s;
}

// trim from end
std::wstring& rtrim(std::wstring& s)
{
	s.erase(s.find_last_not_of(WhitespaceCharsW) + 1);
	return s;
}

// trim from start
std::string& ltrim(std::string& s)
{
	s.erase(0, s.find_first_not_of(WhitespaceCharsA));
	return s;
}

// trim from start
std::wstring& ltrim(std::wstring& s)
{
	s.erase(0, s.find_first_not_of(WhitespaceCharsW));
	return s;
}

void strtolower(std::string& str)
{
	for (size_t i = 0; i < str.length(); ++i)
		str[i] = static_cast<char>(tolower(str[i]));
}

void strtolower(std::wstring& str)
{
	for (size_t i = 0; i < str.length(); ++i)
		str[i] = static_cast<wchar_t>(tolower(str[i]));
}

void strtoupper(std::string& str)
{
	for (size_t i = 0; i < str.length(); i++)
		str[i] = static_cast<char>(toupper(str[i]));
}

void strtoupper(std::wstring& str)
{
	for (size_t i = 0; i < str.length(); i++)
		str[i] = static_cast<wchar_t>(toupper(str[i]));
}

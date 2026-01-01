#include "pch.h"
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

// Copies src into dst (up to srcLen), ensuring it's always null-terminated, even if it were to overflow the dst buffer (bufferSize).
// Returns the number of characters truncated (should be 0 if everything is written correctly)
size_t strcpy_safe(char* dst, const char* src, size_t bufferSize, size_t srcLen)
{
	if (bufferSize <= 0)
		return srcLen;

	if (src == nullptr)
	{
		if (bufferSize > 0)
			dst[0] = '\0';

		return srcLen;
	}

	size_t lenToCopy;
	if (srcLen >= bufferSize)
		lenToCopy = bufferSize - 1;
	else
		lenToCopy = srcLen;

	memcpy(dst, src, lenToCopy);
	dst[lenToCopy] = '\0';

	if (lenToCopy >= srcLen)
		return 0;

	return srcLen - lenToCopy;
}

// Copies src into dst (up to src.length()), ensuring it's always null-terminated, even if it were to overflow the dst buffer (bufferSize).
// Returns the number of characters truncated (should be 0 if everything is written correctly)
size_t strcpy_safe(char* dst, const std::string_view src, size_t bufferSize)
{
	return strcpy_safe(dst, src.data(), bufferSize, src.length());
}

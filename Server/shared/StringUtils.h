#pragma once

#include <cstring>
#include <string>
#include <string_view>

void _string_format(const std::string_view fmt, std::string* result, va_list args);
std::string string_format(const std::string_view fmt, ...);
std::string& rtrim(std::string& s);
std::wstring& rtrim(std::wstring& s);
std::string& ltrim(std::string& s);
std::wstring& ltrim(std::wstring& s);
void strtolower(std::string& str);
void strtolower(std::wstring& str);
void strtoupper(std::string& str);
void strtoupper(std::wstring& str);

#pragma once

#include <string>

bool CpToWide(const char* input, size_t input_size, uint32_t input_codepage, wchar_t* output, size_t output_buffer_chars);
std::wstring CpToWide(const char* input, size_t input_size, uint32_t input_codepage);
std::wstring CpToWide(const std::string& input, uint32_t input_codepage);

bool WideToCp(const wchar_t* input, size_t input_size, char* output, size_t output_buffer_chars, uint32_t output_codepage);
std::string WideToCp(const wchar_t* input, size_t input_size, uint32_t output_codepage);
std::string WideToCp(const std::wstring& input, uint32_t output_codepage);

bool Utf8ToWide(const char* input, size_t input_size, wchar_t* output, size_t output_buffer_chars);
std::wstring Utf8ToWide(const char* input, size_t input_size);
std::wstring Utf8ToWide(const std::string& input);

bool WideToUtf8(const wchar_t* input, size_t input_size, char* output, size_t output_buffer_chars);
std::string WideToUtf8(const wchar_t* input, size_t input_size);
std::string WideToUtf8(const std::wstring& input);

bool LocalToWide(const char* input, size_t input_size, wchar_t* output, size_t output_buffer_chars);
std::wstring LocalToWide(const char* input, size_t input_size);
std::wstring LocalToWide(const std::string& input);

bool WideToLocal(const wchar_t* input, size_t input_size, char* output, size_t output_buffer_chars);
std::string WideToLocal(const wchar_t* input, size_t input_size);
std::string WideToLocal(const std::wstring& input);

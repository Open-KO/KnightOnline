#pragma once

#include <string>

bool LocalToWide(const char* input, size_t input_size, wchar_t* output, size_t output_buffer_chars);
std::wstring LocalToWide(const char* input, size_t input_size);
std::wstring LocalToWide(const std::string& input);

bool WideToLocal(const wchar_t* input, size_t input_size, char* output, size_t output_buffer_chars);
std::string WideToLocal(const wchar_t* input, size_t input_size);
std::string WideToLocal(const std::wstring& input);

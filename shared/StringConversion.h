#pragma once

#include <string>
#include <string_view>

bool CpToWide(const char* input, size_t inputSize, uint32_t inputCodePage, wchar_t* output, size_t outputBufferCharCount);
std::wstring CpToWide(const char* input, size_t inputSize, uint32_t inputCodePage);
std::wstring CpToWide(const std::string_view input, uint32_t inputCodePage);

bool WideToCp(const wchar_t* input, size_t inputSize, char* output, size_t outputBufferCharCount, uint32_t outputCodePage);
std::string WideToCp(const wchar_t* input, size_t inputSize, uint32_t outputCodePage);
std::string WideToCp(const std::wstring_view input, uint32_t outputCodePage);

bool Utf8ToWide(const char* input, size_t inputSize, wchar_t* output, size_t outputBufferCharCount);
std::wstring Utf8ToWide(const char* input, size_t inputSize);
std::wstring Utf8ToWide(const std::string_view input);

bool WideToUtf8(const wchar_t* input, size_t inputSize, char* output, size_t outputBufferCharCount);
std::string WideToUtf8(const wchar_t* input, size_t inputSize);
std::string WideToUtf8(const std::wstring_view input);

bool LocalToWide(const char* input, size_t inputSize, wchar_t* output, size_t outputBufferCharCount);
std::wstring LocalToWide(const char* input, size_t inputSize);
std::wstring LocalToWide(const std::string_view input);

bool WideToLocal(const wchar_t* input, size_t inputSize, char* output, size_t outputBufferCharCount);
std::string WideToLocal(const wchar_t* input, size_t inputSize);
std::string WideToLocal(const std::wstring_view input);

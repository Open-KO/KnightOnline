#ifndef SHARED_STRINGUTILS_H
#define SHARED_STRINGUTILS_H

#pragma once

#include <cstring>
#include <string>
#include <string_view>

#if defined(_WIN32)
#define stricmp  _stricmp
#define strnicmp _strnicmp
#elif __has_include(<strings.h>)
#include <strings.h>
#define stricmp  strcasecmp
#define strnicmp strncasecmp
#else
#error "No applicable strncasecmp/_strnicmp implementation found"
#endif

std::string& rtrim(std::string& s);
std::string& ltrim(std::string& s);
void strtolower(std::string& str);
void strtoupper(std::string& str);

// Copies src into dst (up to srcLen), ensuring it's always null-terminated, even if it were to overflow the dst buffer (bufferSize).
// Returns the number of characters truncated (should be 0 if everything is written correctly)
size_t strcpy_safe(char* dst, const char* src, size_t dstBufferSize, size_t srcLen);

// Copies src into dst (up to src.length()), ensuring it's always null-terminated, even if it were to overflow the dst buffer (bufferSize).
// Returns the number of characters truncated (should be 0 if everything is written correctly)
size_t strcpy_safe(char* dst, const std::string_view src, size_t dstBufferSize);

// Copies src into dst (up to src.length()), ensuring it's always null-terminated, even if it were to overflow the dst buffer (BufferSize).
// Returns the number of characters truncated (should be 0 if everything is written correctly)
template <std::size_t BufferSize>
size_t strcpy_safe(char (&dst)[BufferSize], const std::string_view src)
{
	return strcpy_safe(dst, src.data(), BufferSize, src.length());
}

#endif // SHARED_STRINGUTILS_H

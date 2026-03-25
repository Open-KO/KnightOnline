#ifndef SERVER_SHAREDSERVER_UTILITIES_H
#define SERVER_SHAREDSERVER_UTILITIES_H

#pragma once

#include <filesystem>
#include <functional>
#include <string_view>

bool CheckGetVarString(int nLength, char* tBuf, const char* sBuf, int nSize, int& index);
int GetVarString(char* tBuf, const char* sBuf, int nSize, int& index);
void GetString(char* tBuf, const char* sBuf, int len, int& index);
uint8_t GetByte(const char* sBuf, int& index);
int GetShort(const char* sBuf, int& index);
int GetInt(const char* sBuf, int& index);
uint32_t GetDWORD(const char* sBuf, int& index);
float GetFloat(const char* sBuf, int& index);
int64_t GetInt64(const char* sBuf, int& index);
void SetString(char* tBuf, const char* sBuf, int len, int& index);
void SetVarString(char* tBuf, const char* sBuf, int len, int& index);
void SetByte(char* tBuf, uint8_t sByte, int& index);
void SetShort(char* tBuf, int sShort, int& index);
void SetInt(char* tBuf, int sInt, int& index);
void SetDWORD(char* tBuf, uint32_t sDword, int& index);
void SetFloat(char* tBuf, float sFloat, int& index);
void SetInt64(char* tBuf, int64_t nInt64, int& index);
void SetString1(char* tBuf, const std::string_view str, int& index);
void SetString1(char* tBuf, const char* str, int length, int& index);
void SetString2(char* tBuf, const std::string_view str, int& index);
void SetString2(char* tBuf, const char* str, int length, int& index);
bool ParseSpace(char* tBuf, const char* sBuf, int& bufferIndex);

/// \brief Calculates the value of a currency change (gold, NP, Manner) by upcasting the delta
/// to prevent overflows, then applying a clamp between 0 and 2.1B
void CurrencyChange(int32_t& refAmount, int32_t delta);

/// \brief Represents the source from which an asset directory was resolved.
enum class AssetDirSource : uint8_t
{
	/// No valid directory source was found.
	None,

	/// Directory was provided via the command line.
	/// This is checked first in the priority order.
	CommandLine,

	/// Directory was read from the configuration file (i.e. INI file).
	/// This is checked second in the priority order.
	Config,

	/// Directory falls back to the hardcoded default directory.
	/// This will be checked in the current working directory primarily,
	/// and then fallback to checking the parent directory if not found.
	/// This is checked last in the priority order.
	Default
};

/// \brief Identifies an asset directory location, prioritising command-line > INI > defaults.
/// \param identifierName		Identifier name of a directory to be used for logging. e.g. "MAP"
/// \param commandLineDirectory	The directory passed to the command-line, if applicable. First in priority order.
/// \param configDirectory		The directory as stored in the INI config, if applicable. Second in priority order.
/// \param defaultDirectory		The default directory location, relative to the current working directory (e.g. ./MAP/).
///								As a fallback, will try the directory above (e.g. ../MAP/).
///								Last in the priority order.
/// \param outputPath			Path to output the final identified directory to.
/// \returns true when successful, false otherwise
AssetDirSource IdentifyAssetDir(const std::string_view identifierName,
	const std::filesystem::path& commandLineDirectory, const std::filesystem::path& configDirectory,
	const std::filesystem::path& defaultDirectory, std::filesystem::path* outputPath);

extern std::function<int(int min, int max)> myrand;

#endif // SERVER_SHAREDSERVER_UTILITIES_H

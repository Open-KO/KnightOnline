#pragma once

#include <map>
#include <string_view>
#include <string>
#include <filesystem>

class CIni
{
protected:
	struct ci_less
	{
		inline bool operator() (const std::wstring& str1, const std::wstring& str2) const {
			return _wcsicmp(str1.c_str(), str2.c_str()) < 0;
		}
	};

	std::wstring m_szPath;

	// Defines key/value pairs within sections
	using ConfigEntryMap = std::map<std::wstring, std::wstring, ci_less>;

	// Defines the sections containing the key/value pairs
	using ConfigMap = std::map<std::wstring, ConfigEntryMap, ci_less>;

	ConfigMap m_configMap;

public:
	CIni() = default;
	CIni(const std::filesystem::path& path);

	bool Load();
	bool Load(const std::filesystem::path& path);

	void Save();
	void Save(const std::filesystem::path& path);

	int GetInt(std::string_view szAppName, std::string_view szKeyName, const int iDefault);
	int GetInt(std::wstring_view szAppName, std::wstring_view szKeyName, const int iDefault);

	bool GetBool(std::string_view szAppName, std::string_view szKeyName, const bool bDefault);
	bool GetBool(std::wstring_view szAppName, std::wstring_view szKeyName, const bool bDefault);

	std::string GetString(std::string_view szAppName, std::string_view szKeyName, std::string_view szDefault);
	std::wstring GetString(std::wstring_view szAppName, std::wstring_view szKeyName, std::wstring_view szDefault);

	void GetString(std::string_view szAppName, std::string_view szKeyName, std::string_view szDefault, char* szOutBuffer, size_t nBufferLength);
	void GetString(std::wstring_view szAppName, std::wstring_view szKeyName, std::wstring_view szDefault, wchar_t* szOutBuffer, size_t nBufferLength);

	int SetInt(std::string_view szAppName, std::string_view szKeyName, const int iDefault);
	int SetInt(std::wstring_view szAppName, std::wstring_view szKeyName, const int iDefault);

	int SetString(std::string_view szAppName, std::string_view szKeyName, std::string_view szDefault);
	int SetString(std::wstring_view szAppName, std::wstring_view szKeyName, std::wstring_view szDefault);
};

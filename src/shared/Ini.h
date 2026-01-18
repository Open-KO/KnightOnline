#ifndef SHARED_INI_H
#define SHARED_INI_H

#pragma once

#include <algorithm>
#include <filesystem>
#include <map>
#include <string>
#include <string_view>

class CIni
{
protected:
	struct ci_less
	{
		using is_transparent = void;

		bool operator()(std::string_view lhs, std::string_view rhs) const
		{
			const size_t minLength = std::min(lhs.size(), rhs.size());
			for (size_t i = 0; i < minLength; i++)
			{
				int a = std::tolower(lhs[i]);
				int b = std::tolower(rhs[i]);
				if (a != b)
					return a < b;
			}

			return lhs.length() < rhs.length();
		}
	};

	std::filesystem::path _path;

	// Defines key/value pairs within sections
	using ConfigEntryMap = std::map<std::string, std::string, ci_less>;

	// Defines the sections containing the key/value pairs
	using ConfigMap      = std::map<std::string, ConfigEntryMap, ci_less>;

	ConfigMap _configMap;

public:
	const std::filesystem::path& GetPath() const
	{
		return _path;
	}

	CIni() = default;
	CIni(const std::filesystem::path& path);

	bool Load();
	bool Load(const std::filesystem::path& path);

	void Save();
	void Save(const std::filesystem::path& path);

	int GetInt(std::string_view svAppName, std::string_view svKeyName, const int iDefault);
	bool GetBool(std::string_view svAppName, std::string_view svKeyName, const bool bDefault);
	std::string GetString(
		std::string_view svAppName, std::string_view svKeyName, std::string_view svDefault);

	bool SetInt(std::string_view svAppName, std::string_view svKeyName, const int iDefault);
	bool SetString(
		std::string_view svAppName, std::string_view svKeyName, std::string_view svDefault);
};

#endif // SHARED_INI_H

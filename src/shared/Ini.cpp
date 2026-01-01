#include "pch.h"
#include "Ini.h"
#include <iostream>
#include <fstream>
#include "StringUtils.h"

constexpr char INI_SECTION_START	= '[';
constexpr char INI_SECTION_END		= ']';
constexpr char INI_KEY_SEPARATOR	= '=';

CIni::CIni(const std::filesystem::path& path)
{
	Load(path);
}

bool CIni::Load()
{
	return Load(_path);
}

bool CIni::Load(const std::filesystem::path& path)
{
	_path = path;

	std::ifstream file(path);
	if (!file)
		return false;

	std::string currentSection;

	// If an invalid section is hit
	// Ensure that we don't place key/value pairs
	// from the invalid section into the previously loaded section.
	bool skipNextSection = false;
	while (!file.eof())
	{
		std::string line;
		getline(file, line);

		rtrim(line);
		if (line.empty())
			continue;

		// Check for value strings first
		// It's faster than checking for a section
		// at the expense of of not being able to use '=' in section names.
		// As this is uncommon behaviour, this is a suitable trade-off.
		size_t keySeparatorPos = line.find(INI_KEY_SEPARATOR);
		if (keySeparatorPos != std::string::npos)
		{
			if (skipNextSection)
				continue;

			std::string key = line.substr(0, keySeparatorPos);
			std::string value = line.substr(keySeparatorPos + 1);

			// Clean up key/value to allow for 'key = value'
			rtrim(key);   /* remove trailing whitespace from keys */
			ltrim(value); /* remove preleading whitespace from values */

			auto itr = _configMap.find(currentSection);
			if (itr == _configMap.end())
			{
				_configMap.insert(
					std::make_pair(currentSection, ConfigEntryMap()));
				itr = _configMap.find(currentSection);
			}

			itr->second[key] = value;
			continue;
		}

		// Not a value, so assume it's a section
		size_t sectionStart = line.find_first_of(INI_SECTION_START);
		size_t sectionEnd = line.find_last_of(INI_SECTION_END);

		if (sectionStart == std::string::npos
			|| sectionEnd == std::string::npos
			|| sectionStart > sectionEnd)
		{
			/* invalid section */
			skipNextSection = true;
			continue;
		}

		currentSection = line.substr(sectionStart + 1, sectionEnd - 1);
		skipNextSection = false;
	}

	file.close();
	return true;
}

void CIni::Save()
{
	Save(_path);
}

void CIni::Save(const std::filesystem::path& path)
{
	FILE* fp = nullptr;
	
#ifdef _MSC_VER
	_wfopen_s(&fp, path.c_str(), L"w");
#else
	fp = fopen(path.string().c_str(), "w");
#endif

	if (fp == nullptr)
		return;

	for (const auto& [sectionName, keyValuePairs] : _configMap)
	{
		// Start the section
		fprintf(fp, "[%s]\n", sectionName.c_str());

		// Now list out all the key/value pairs
		for (const auto& [key, value] : keyValuePairs)
			fprintf(fp, "%s=%s\n", key.c_str(), value.c_str());

		// Use a trailing newline to finish the section, to make it easier to read
		fputc('\n', fp);
	}

	fclose(fp);
}

int CIni::GetInt(std::string_view svAppName, std::string_view svKeyName, const int iDefault)
{
	auto sectionItr = _configMap.find(svAppName);
	if (sectionItr != _configMap.end())
	{
		auto keyItr = sectionItr->second.find(svKeyName);
		if (keyItr != sectionItr->second.end())
			return atoi(keyItr->second.c_str());
	}

	SetInt(svAppName, svKeyName, iDefault);
	return iDefault;
}

bool CIni::GetBool(std::string_view svAppName, std::string_view svKeyName, const bool bDefault)
{
	return GetInt(svAppName, svKeyName, bDefault) == 1;
}

std::string CIni::GetString(std::string_view svAppName, std::string_view svKeyName, std::string_view svDefault)
{
	auto sectionItr = _configMap.find(svAppName);
	if (sectionItr != _configMap.end())
	{
		auto keyItr = sectionItr->second.find(svKeyName);
		if (keyItr != sectionItr->second.end())
			return keyItr->second;
	}

	std::string szResult(svDefault.data(), svDefault.length());
	SetString(svAppName, svKeyName, svDefault);
	return szResult;
}

void CIni::GetString(std::string_view svAppName, std::string_view svKeyName, std::string_view svDefault, char* szOutBuffer, size_t nBufferLength)
{
	auto sectionItr = _configMap.find(svAppName);
	if (sectionItr != _configMap.end())
	{
		auto keyItr = sectionItr->second.find(svKeyName);
		if (keyItr != sectionItr->second.end())
		{
			snprintf(szOutBuffer, nBufferLength, "%s", keyItr->second.c_str());
			return;
		}
	}

	SetString(svAppName, svKeyName, svDefault);

	snprintf(
		szOutBuffer,
		nBufferLength,
		"%.*s",
		static_cast<int>(svDefault.length()),
		svDefault.data());
}

int CIni::SetInt(std::string_view svAppName, std::string_view svKeyName, const int iDefault)
{
	std::string szDefault = std::to_string(iDefault);
	return SetString(svAppName, svKeyName, szDefault);
}

int CIni::SetString(std::string_view svAppName, std::string_view svKeyName, std::string_view svDefault)
{
	auto itr = _configMap.find(svAppName);
	if (itr == _configMap.end())
	{
		auto ret = _configMap.insert(
			std::make_pair(svAppName, ConfigEntryMap()));
		if (!ret.second)
			return 0;

		itr = ret.first;
	}

	std::string szKeyName(svKeyName.data(), svKeyName.length());
	itr->second[szKeyName].assign(svDefault.data(), svDefault.length());
	return 1;
}

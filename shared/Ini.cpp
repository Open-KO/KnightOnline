#include "stdafx.h"
#include "Ini.h"
#include <iostream>
#include <fstream>
#include "StringConversion.h"
#include "StringUtils.h"

constexpr wchar_t INI_SECTION_START	= L'[';
constexpr wchar_t INI_SECTION_END	= L']';
constexpr wchar_t INI_KEY_SEPARATOR	= L'=';

CIni::CIni(
	const std::filesystem::path& path)
{
	Load(path);
}

bool CIni::Load()
{
	return Load(
		m_szPath);
}

bool CIni::Load(
	const std::filesystem::path& path)
{
	m_szPath = path;

	std::wifstream file(m_szPath);
	if (!file)
	{
		TRACE("Warning: %ls does not exist, will use configured defaults.\n", m_szPath.c_str());
		return false;
	}

	std::wstring currentSection;

	// If an invalid section is hit
	// Ensure that we don't place key/value pairs
	// from the invalid section into the previously loaded section.
	bool bSkipNextSection = false;
	while (!file.eof())
	{
		std::wstring line;
		getline(file, line);

		rtrim(line);
		if (line.empty())
			continue;

		// Check for value strings first
		// It's faster than checking for a section
		// at the expense of of not being able to use '=' in section names.
		// As this is uncommon behaviour, this is a suitable trade-off.
		size_t keySeparatorPos = line.find(INI_KEY_SEPARATOR);
		if (keySeparatorPos != std::wstring::npos)
		{
			if (bSkipNextSection)
				continue;

			std::wstring key = line.substr(0, keySeparatorPos);
			std::wstring value = line.substr(keySeparatorPos + 1);

			// Clean up key/value to allow for 'key = value'
			rtrim(key);   /* remove trailing whitespace from keys */
			ltrim(value); /* remove preleading whitespace from values */

			auto itr = m_configMap.find(currentSection);
			if (itr == m_configMap.end())
			{
				m_configMap.insert(
					std::make_pair(currentSection, std::move(ConfigEntryMap())));
				itr = m_configMap.find(currentSection);
			}

			itr->second[key] = value;
			continue;
		}

		// Not a value, so assume it's a section
		size_t sectionStart = line.find_first_of(INI_SECTION_START),
			sectionEnd = line.find_last_of(INI_SECTION_END);

		if (sectionStart == std::wstring::npos
			|| sectionEnd == std::wstring::npos
			|| sectionStart > sectionEnd)
		{
			/* invalid section */
			bSkipNextSection = true;
			continue;
		}

		currentSection = line.substr(sectionStart + 1, sectionEnd - 1);
		bSkipNextSection = false;
	}

	file.close();
	return true;
}

void CIni::Save()
{
	Save(m_szPath);
}

void CIni::Save(
	const std::filesystem::path& path)
{
	FILE* fp = _wfopen(path.c_str(), L"w");
	if (fp == nullptr)
		return;

	for (const auto& [sectionName, keyValuePairs] : m_configMap)
	{
		// Start the section
		std::string sectionA = WideToLocal(sectionName);
		fprintf(fp, "[%s]\n", sectionA.c_str());

		// Now list out all the key/value pairs
		for (const auto& [key, value] : keyValuePairs)
		{
			std::string keyA	= WideToLocal(key);
			std::string valueA	= WideToLocal(value);
			fprintf(fp, "%s=%s\n", keyA.c_str(), valueA.c_str());
		}

		// Use a trailing newline to finish the section, to make it easier to read
		fputc('\n', fp);
	}

	fclose(fp);
}

int CIni::GetInt(
	std::string_view szAppNameA,
	std::string_view szKeyNameA,
	const int iDefault)
{
	std::wstring szAppNameW = LocalToWide(szAppNameA);
	std::wstring szKeyNameW = LocalToWide(szKeyNameA);

	return GetInt(szAppNameW, szKeyNameW, iDefault);
}

int CIni::GetInt(
	std::wstring_view szAppName,
	std::wstring_view szKeyName,
	const int iDefault)
{
	std::wstring szAppNameW(szAppName.data(), szAppName.length());

	auto sectionItr = m_configMap.find(szAppNameW);
	if (sectionItr != m_configMap.end())
	{
		std::wstring szKeyNameW(szKeyName.data(), szKeyName.length());

		auto keyItr = sectionItr->second.find(szKeyNameW);
		if (keyItr != sectionItr->second.end())
			return _wtoi(keyItr->second.c_str());
	}

	SetInt(szAppName, szKeyName, iDefault);
	return iDefault;
}

bool CIni::GetBool(
	std::string_view szAppName,
	std::string_view szKeyName,
	const bool bDefault)
{
	return GetInt(szAppName, szKeyName, bDefault) == 1;
}

bool CIni::GetBool(
	std::wstring_view szAppName,
	std::wstring_view szKeyName,
	const bool bDefault)
{
	return GetInt(szAppName, szKeyName, bDefault) == 1;
}

std::string CIni::GetString(
	std::string_view szAppNameA,
	std::string_view szKeyNameA,
	std::string_view szDefaultA)
{
	std::wstring szAppNameW = LocalToWide(szAppNameA);
	std::wstring szKeyNameW = LocalToWide(szKeyNameA);
	std::wstring szDefaultW = LocalToWide(szDefaultA);

	auto sectionItr = m_configMap.find(szAppNameW);
	if (sectionItr != m_configMap.end())
	{
		auto keyItr = sectionItr->second.find(szKeyNameW);
		if (keyItr != sectionItr->second.end())
			return WideToLocal(keyItr->second);
	}

	std::string szResult(szDefaultA.data(), szDefaultA.length());
	SetString(szAppNameW, szKeyNameW, szDefaultW);
	return szResult;
}

std::wstring CIni::GetString(
	std::wstring_view szAppName,
	std::wstring_view szKeyName,
	std::wstring_view szDefault)
{
	std::wstring szAppNameW(szAppName.data(), szAppName.length());

	auto sectionItr = m_configMap.find(szAppNameW);
	if (sectionItr != m_configMap.end())
	{
		std::wstring szKeyNameW(szKeyName.data(), szKeyName.length());

		auto keyItr = sectionItr->second.find(szKeyNameW);
		if (keyItr != sectionItr->second.end())
			return keyItr->second;
	}

	std::wstring szResult(szDefault.data(), szDefault.length());
	SetString(szAppName, szKeyName, szDefault);
	return szResult;
}

void CIni::GetString(
	std::string_view szAppNameA,
	std::string_view szKeyNameA,
	std::string_view szDefaultA,
	char* szOutBuffer,
	size_t nBufferLength)
{
	std::wstring szAppNameW	= LocalToWide(szAppNameA);
	std::wstring szKeyNameW	= LocalToWide(szKeyNameA);
	std::wstring szDefaultW	= LocalToWide(szDefaultA);
	std::wstring szResultW	= GetString(szAppNameW, szKeyNameW, szDefaultW);
	std::string szResultA	= WideToLocal(szResultW);

	snprintf(
		szOutBuffer,
		nBufferLength,
		"%s",
		szResultA.c_str());
}

void CIni::GetString(
	std::wstring_view szAppName,
	std::wstring_view szKeyName,
	std::wstring_view szDefault,
	wchar_t* szOutBuffer,
	size_t nBufferLength)
{
	std::wstring szAppNameW(szAppName.data(), szAppName.length());

	auto sectionItr = m_configMap.find(szAppNameW);
	if (sectionItr != m_configMap.end())
	{
		std::wstring szKeyNameW(szKeyName.data(), szKeyName.length());

		auto keyItr = sectionItr->second.find(szKeyNameW);
		if (keyItr != sectionItr->second.end())
		{
			_snwprintf(
				szOutBuffer,
				nBufferLength - 1,
				L"%s",
				keyItr->second.c_str());
			return;
		}
	}

	SetString(szAppName, szKeyName, szDefault);

	_snwprintf(
		szOutBuffer,
		nBufferLength - 1,
		L"%.*s",
		static_cast<int>(szDefault.length()),
		szDefault.data());
}

int CIni::SetInt(
	std::string_view szAppName,
	std::string_view szKeyName,
	const int iDefault)
{
	std::string szDefault = std::to_string(iDefault);
	return SetString(szAppName, szKeyName, szDefault);
}

int CIni::SetInt(
	std::wstring_view szAppName,
	std::wstring_view szKeyName,
	const int iDefault)
{
	std::wstring szDefault = std::to_wstring(iDefault);
	return SetString(szAppName, szKeyName, szDefault);
}

int CIni::SetString(
	std::string_view szAppNameA,
	std::string_view szKeyNameA,
	std::string_view szDefaultA)
{
	std::wstring szAppNameW	= LocalToWide(szAppNameA);
	std::wstring szKeyNameW	= LocalToWide(szKeyNameA);
	std::wstring szDefaultW	= LocalToWide(szDefaultA);

	return SetString(szAppNameW, szKeyNameW, szDefaultW);
}

int CIni::SetString(
	std::wstring_view szAppName,
	std::wstring_view szKeyName,
	std::wstring_view szDefault)
{
	std::wstring szAppNameW(szAppName.data(), szAppName.length());

	auto itr = m_configMap.find(szAppNameW);
	if (itr == m_configMap.end())
	{
		auto ret = m_configMap.insert(
			std::make_pair(szAppNameW, std::move(ConfigEntryMap())));
		if (!ret.second)
			return 0;

		itr = ret.first;
	}

	std::wstring szKeyNameW(szKeyName.data(), szKeyName.length());
	itr->second[szKeyNameW].assign(szDefault.data(), szDefault.length());
	return 1;
}

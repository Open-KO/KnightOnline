#include "stdafx.h"
#include "StringConversion.h"

bool CpToWide(
	const char* input,
	size_t input_size,
	uint32_t input_codepage,
	wchar_t* output,
	size_t output_buffer_chars)
{
	int sizeNeeded = MultiByteToWideChar(
		input_codepage,
		0,
		input,
		static_cast<int>(input_size),
		0,
		0);

	if (sizeNeeded < 0
		|| static_cast<int>(output_buffer_chars) < (sizeNeeded + 1))
		return false;

	if (sizeNeeded > 0)
	{
		MultiByteToWideChar(
			input_codepage,
			0,
			input,
			static_cast<int>(input_size),
			output,
			sizeNeeded);
	}

	output[sizeNeeded] = L'\0';
	return true;
}

std::wstring CpToWide(
	const char* input,
	size_t input_size,
	uint32_t input_codepage)
{
	int sizeNeeded = MultiByteToWideChar(
		input_codepage,
		0,
		input,
		static_cast<int>(input_size),
		0,
		0);
	if (sizeNeeded <= 0)
		return std::wstring();

	std::wstring wideEncoded(sizeNeeded, 0);
	MultiByteToWideChar(
		input_codepage,
		0,
		input,
		static_cast<int>(input_size),
		&wideEncoded[0],
		sizeNeeded);
	return wideEncoded;
}

std::wstring CpToWide(
	const std::string& input,
	uint32_t input_codepage)
{
	return CpToWide(
		input.c_str(),
		input.size(),
		input_codepage);
}

bool WideToCp(
	const wchar_t* input,
	size_t input_size,
	char* output,
	size_t output_buffer_chars,
	uint32_t output_codepage)
{
	int sizeNeeded = WideCharToMultiByte(
		output_codepage,
		0,
		input,
		static_cast<int>(input_size),
		nullptr,
		0,
		nullptr,
		nullptr);

	if (sizeNeeded < 0
		|| static_cast<int>(output_buffer_chars) < (sizeNeeded + 1))
		return false;

	if (sizeNeeded > 0)
	{
		WideCharToMultiByte(
			output_codepage,
			0,
			input,
			static_cast<int>(input_size),
			output,
			sizeNeeded,
			nullptr,
			nullptr);
	}

	output[sizeNeeded] = '\0';
	return true;
}

std::string WideToCp(
	const wchar_t* input,
	size_t input_size,
	uint32_t output_codepage)
{
	int sizeNeeded = WideCharToMultiByte(
		output_codepage,
		0,
		input,
		static_cast<int>(input_size),
		nullptr,
		0,
		nullptr,
		nullptr);
	if (sizeNeeded == 0)
		return std::string();

	std::string cpEncoded(sizeNeeded, 0);
	WideCharToMultiByte(
		output_codepage,
		0,
		input,
		static_cast<int>(input_size),
		&cpEncoded[0],
		sizeNeeded,
		nullptr,
		nullptr);

	return cpEncoded;
}

std::string WideToCp(
	const std::wstring& input,
	uint32_t output_codepage)
{
	return WideToCp(
		input.c_str(),
		input.size(),
		output_codepage);
}

bool Utf8ToWide(
	const char* input,
	size_t input_size,
	wchar_t* output,
	size_t output_buffer_chars)
{
	return CpToWide(
		input,
		input_size,
		CP_UTF8,
		output,
		output_buffer_chars);
}

std::wstring Utf8ToWide(
	const char* input,
	size_t input_size)
{
	return CpToWide(
		input,
		input_size,
		CP_UTF8);
}

std::wstring Utf8ToWide(
	const std::string& input)
{
	return Utf8ToWide(
		input.c_str(),
		input.size());
}

bool WideToUtf8(
	const wchar_t* input,
	size_t input_size,
	char* output,
	size_t output_buffer_chars)
{
	return WideToCp(
		input,
		input_size,
		output,
		output_buffer_chars,
		CP_UTF8);
}

std::string WideToUtf8(
	const wchar_t* input,
	size_t input_size)
{
	return WideToCp(
		input,
		input_size,
		CP_UTF8);
}

std::string WideToUtf8(
	const std::wstring& input)
{
	return WideToUtf8(
		input.c_str(),
		input.size());
}

bool LocalToWide(
	const char* input,
	size_t input_size,
	wchar_t* output,
	size_t output_buffer_chars)
{
	return CpToWide(
		input,
		input_size,
		CP_ACP,
		output,
		output_buffer_chars);
}

std::wstring LocalToWide(
	const char* input,
	size_t input_size)
{
	return CpToWide(
		input,
		input_size,
		CP_ACP);
}

std::wstring LocalToWide(
	const std::string& input)
{
	return LocalToWide(
		input.c_str(),
		input.size());
}

std::wstring LocalToWide(
	std::string_view input)
{
	return LocalToWide(
		input.data(),
		input.size());
}

bool WideToLocal(
	const wchar_t* input,
	size_t input_size,
	char* output,
	size_t output_buffer_chars)
{
	return WideToCp(
		input,
		input_size,
		output,
		output_buffer_chars,
		CP_ACP);
}

std::string WideToLocal(
	const wchar_t* input,
	size_t input_size)
{
	return WideToCp(
		input,
		input_size,
		CP_ACP);
}

std::string WideToLocal(
	const std::wstring& input)
{
	return WideToLocal(
		input.c_str(),
		input.size());
}

std::string WideToLocal(
	std::wstring_view input)
{
	return WideToLocal(
		input.data(),
		input.size());
}

#include "stdafx.h"
#include "StringConversion.h"

bool LocalToWide(
	const char* input,
	size_t input_size,
	wchar_t* output,
	size_t output_buffer_chars)
{
	int sizeNeeded = MultiByteToWideChar(
		CP_ACP,
		0,
		input,
		static_cast<int>(input_size),
		0,
		0);

	if (sizeNeeded < 0
		|| (int) output_buffer_chars < (sizeNeeded + 1))
		return false;

	if (sizeNeeded > 0)
	{
		MultiByteToWideChar(
			CP_ACP,
			0,
			input,
			static_cast<int>(input_size),
			output,
			sizeNeeded);
	}

	output[sizeNeeded] = L'\0';
	return true;
}

std::wstring LocalToWide(
	const char* input,
	size_t input_size)
{
	int sizeNeeded = MultiByteToWideChar(
		CP_ACP,
		0,
		input,
		static_cast<int>(input_size),
		0,
		0);
	if (sizeNeeded <= 0)
		return std::wstring();

	std::wstring wideEncoded(sizeNeeded, 0);
	MultiByteToWideChar(
		CP_ACP,
		0,
		input,
		static_cast<int>(input_size),
		&wideEncoded[0],
		sizeNeeded);
	return wideEncoded;
}

std::wstring LocalToWide(
	const std::string& input)
{
	return LocalToWide(
		input.c_str(),
		input.size());
}

bool WideToLocal(
	const wchar_t* input,
	size_t input_size,
	char* output,
	size_t output_buffer_chars)
{
	int sizeNeeded = WideCharToMultiByte(
		CP_ACP,
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
			CP_ACP,
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

std::string WideToLocal(
	const wchar_t* input,
	size_t input_size)
{
	int sizeNeeded = WideCharToMultiByte(
		CP_ACP,
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
		CP_ACP,
		0,
		input,
		static_cast<int>(input_size),
		&cpEncoded[0],
		sizeNeeded,
		nullptr,
		nullptr);

	return cpEncoded;
}

std::string WideToLocal(
	const std::wstring& input)
{
	return WideToLocal(
		input.c_str(),
		input.size());
}

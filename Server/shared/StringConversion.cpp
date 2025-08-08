#include "stdafx.h"
#include "StringConversion.h"

bool CpToWide(
	const char* input,
	size_t inputSize,
	uint32_t inputCodePage,
	wchar_t* output,
	size_t outputBufferCharCount)
{
	int sizeNeeded = MultiByteToWideChar(
		inputCodePage,
		0,
		input,
		static_cast<int>(inputSize),
		0,
		0);

	if (sizeNeeded < 0
		|| static_cast<int>(outputBufferCharCount) < (sizeNeeded + 1))
		return false;

	if (sizeNeeded > 0)
	{
		MultiByteToWideChar(
			inputCodePage,
			0,
			input,
			static_cast<int>(inputSize),
			output,
			sizeNeeded);
	}

	output[sizeNeeded] = L'\0';
	return true;
}

std::wstring CpToWide(
	const char* input,
	size_t inputSize,
	uint32_t inputCodePage)
{
	int sizeNeeded = MultiByteToWideChar(
		inputCodePage,
		0,
		input,
		static_cast<int>(inputSize),
		0,
		0);
	if (sizeNeeded <= 0)
		return std::wstring();

	std::wstring wideEncoded(sizeNeeded, 0);
	MultiByteToWideChar(
		inputCodePage,
		0,
		input,
		static_cast<int>(inputSize),
		&wideEncoded[0],
		sizeNeeded);
	return wideEncoded;
}

std::wstring CpToWide(
	const std::string_view input,
	uint32_t inputCodePage)
{
	return CpToWide(
		input.data(),
		input.size(),
		inputCodePage);
}

bool WideToCp(
	const wchar_t* input,
	size_t inputSize,
	char* output,
	size_t outputBufferCharCount,
	uint32_t outputCodePage)
{
	int sizeNeeded = WideCharToMultiByte(
		outputCodePage,
		0,
		input,
		static_cast<int>(inputSize),
		nullptr,
		0,
		nullptr,
		nullptr);

	if (sizeNeeded < 0
		|| static_cast<int>(outputBufferCharCount) < (sizeNeeded + 1))
		return false;

	if (sizeNeeded > 0)
	{
		WideCharToMultiByte(
			outputCodePage,
			0,
			input,
			static_cast<int>(inputSize),
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
	size_t inputSize,
	uint32_t outputCodePage)
{
	int sizeNeeded = WideCharToMultiByte(
		outputCodePage,
		0,
		input,
		static_cast<int>(inputSize),
		nullptr,
		0,
		nullptr,
		nullptr);
	if (sizeNeeded == 0)
		return std::string();

	std::string cpEncoded(sizeNeeded, 0);
	WideCharToMultiByte(
		outputCodePage,
		0,
		input,
		static_cast<int>(inputSize),
		&cpEncoded[0],
		sizeNeeded,
		nullptr,
		nullptr);

	return cpEncoded;
}

std::string WideToCp(
	const std::wstring_view input,
	uint32_t outputCodePage)
{
	return WideToCp(
		input.data(),
		input.size(),
		outputCodePage);
}

bool Utf8ToWide(
	const char* input,
	size_t inputSize,
	wchar_t* output,
	size_t outputBufferCharCount)
{
	return CpToWide(
		input,
		inputSize,
		CP_UTF8,
		output,
		outputBufferCharCount);
}

std::wstring Utf8ToWide(
	const char* input,
	size_t inputSize)
{
	return CpToWide(
		input,
		inputSize,
		CP_UTF8);
}

std::wstring Utf8ToWide(
	const std::string_view input)
{
	return Utf8ToWide(
		input.data(),
		input.size());
}

bool WideToUtf8(
	const wchar_t* input,
	size_t inputSize,
	char* output,
	size_t outputBufferCharCount)
{
	return WideToCp(
		input,
		inputSize,
		output,
		outputBufferCharCount,
		CP_UTF8);
}

std::string WideToUtf8(
	const wchar_t* input,
	size_t inputSize)
{
	return WideToCp(
		input,
		inputSize,
		CP_UTF8);
}

std::string WideToUtf8(
	const std::wstring_view input)
{
	return WideToUtf8(
		input.data(),
		input.size());
}

bool LocalToWide(
	const char* input,
	size_t inputSize,
	wchar_t* output,
	size_t outputBufferCharCount)
{
	return CpToWide(
		input,
		inputSize,
		CP_ACP,
		output,
		outputBufferCharCount);
}

std::wstring LocalToWide(
	const char* input,
	size_t inputSize)
{
	return CpToWide(
		input,
		inputSize,
		CP_ACP);
}

std::wstring LocalToWide(
	const std::string_view input)
{
	return LocalToWide(
		input.data(),
		input.size());
}

bool WideToLocal(
	const wchar_t* input,
	size_t inputSize,
	char* output,
	size_t outputBufferCharCount)
{
	return WideToCp(
		input,
		inputSize,
		output,
		outputBufferCharCount,
		CP_ACP);
}

std::string WideToLocal(
	const wchar_t* input,
	size_t inputSize)
{
	return WideToCp(
		input,
		inputSize,
		CP_ACP);
}

std::string WideToLocal(
	const std::wstring_view input)
{
	return WideToLocal(
		input.data(),
		input.size());
}

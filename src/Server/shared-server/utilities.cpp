#include "pch.h"
#include "utilities.h"
#include <algorithm> // std::clamp()
#include <limits>    // INT_MAX

int myrand_generic_impl(int min, int max);

std::function<int(int min, int max)> myrand = myrand_generic_impl;

bool CheckGetVarString(int nLength, char* tBuf, const char* sBuf, int nSize, int& index)
{
	int nRet = GetVarString(tBuf, sBuf, nSize, index);
	if (nRet <= 0 || nRet > nLength)
		return false;

	return true;
}

int GetVarString(char* tBuf, const char* sBuf, int nSize, int& index)
{
	int nLen = 0;

	if (nSize == sizeof(uint8_t))
		nLen = GetByte(sBuf, index);
	else
		nLen = GetShort(sBuf, index);

	GetString(tBuf, sBuf, nLen, index);
	*(tBuf + nLen) = 0;

	return nLen;
}

void GetString(char* tBuf, const char* sBuf, int len, int& index)
{
	memcpy(tBuf, sBuf + index, len);
	index += len;
}

uint8_t GetByte(const char* sBuf, int& index)
{
	int t_index = index;
	index++;
	return (uint8_t) (*(sBuf + t_index));
}

int GetShort(const char* sBuf, int& index)
{
	index += 2;
	return *(int16_t*) (sBuf + index - 2);
}

int GetInt(const char* sBuf, int& index)
{
	index += 4;
	return *(int*) (sBuf + index - 4);
}

uint32_t GetDWORD(const char* sBuf, int& index)
{
	index += 4;
	return *(uint32_t*) (sBuf + index - 4);
}

float GetFloat(const char* sBuf, int& index)
{
	index += 4;
	return *(float*) (sBuf + index - 4);
}

int64_t GetInt64(const char* sBuf, int& index)
{
	index += 8;
	return *(int64_t*) (sBuf + index - 8);
}

void SetString(char* tBuf, const char* sBuf, int len, int& index)
{
	memcpy(tBuf + index, sBuf, len);
	index += len;
}

void SetVarString(char* tBuf, const char* sBuf, int len, int& index)
{
	*(tBuf + index) = (uint8_t) len;
	index++;

	memcpy(tBuf + index, sBuf, len);
	index += len;
}

void SetByte(char* tBuf, uint8_t sByte, int& index)
{
	*(tBuf + index) = (char) sByte;
	index++;
}

void SetShort(char* tBuf, int sShort, int& index)
{
	int16_t temp = (int16_t) sShort;

	memcpy(tBuf + index, &temp, 2);
	index += 2;
}

void SetInt(char* tBuf, int sInt, int& index)
{
	memcpy(tBuf + index, &sInt, 4);
	index += 4;
}

void SetDWORD(char* tBuf, uint32_t sDword, int& index)
{
	memcpy(tBuf + index, &sDword, 4);
	index += 4;
}

void SetFloat(char* tBuf, float sFloat, int& index)
{
	memcpy(tBuf + index, &sFloat, 4);
	index += 4;
}

void SetInt64(char* tBuf, int64_t nInt64, int& index)
{
	memcpy(tBuf + index, &nInt64, 8);
	index += 8;
}

void SetString1(char* tBuf, const std::string_view str, int& index)
{
	uint8_t len = static_cast<uint8_t>(str.length());
	SetByte(tBuf, len, index);
	// NOLINTNEXTLINE(bugprone-suspicious-stringview-data-usage)
	SetString(tBuf, str.data(), len, index);
}

void SetString1(char* tBuf, const char* str, int length, int& index)
{
	uint8_t len = static_cast<uint8_t>(length);
	SetByte(tBuf, len, index);
	SetString(tBuf, str, len, index);
}

void SetString2(char* tBuf, const std::string_view str, int& index)
{
	int16_t len = static_cast<int16_t>(str.length());
	SetShort(tBuf, len, index);
	// NOLINTNEXTLINE(bugprone-suspicious-stringview-data-usage)
	SetString(tBuf, str.data(), len, index);
}

void SetString2(char* tBuf, const char* str, int length, int& index)
{
	int16_t len = static_cast<int16_t>(length);
	SetShort(tBuf, len, index);
	SetString(tBuf, str, len, index);
}

bool ParseSpace(char* tBuf, const char* sBuf, int& bufferIndex)
{
	int i = 0, index = 0;
	bool flag  = false;

	sBuf      += bufferIndex;

	while (sBuf[index] == ' ' || sBuf[index] == '\t')
		index++;

	while (sBuf[index] != ' ' && sBuf[index] != '\t' && sBuf[index] != (uint8_t) 0)
	{
		tBuf[i++] = sBuf[index++];
		flag      = true;
	}
	tBuf[i] = 0;

	while (sBuf[index] == ' ' || sBuf[index] == '\t')
		index++;

	if (!flag)
		return false;

	bufferIndex += index;
	return true;
}

void CurrencyChange(int32_t& refAmount, int32_t delta)
{
	int64_t upcast = static_cast<int64_t>(refAmount) + delta;

	if (upcast < MIN_CURRENCY)
		refAmount = MIN_CURRENCY;
	else if (upcast > MAX_CURRENCY)
		refAmount = MAX_CURRENCY;
	else
		refAmount = static_cast<int32_t>(upcast);
}

int myrand_generic_impl(int min, int max)
{
	if (min == max)
		return min;

	if (min > max)
		std::swap(min, max);

	double gap      = max - min + 1;
	double rrr      = static_cast<double>(RAND_MAX) / gap;

	int rand_result = static_cast<int>(static_cast<double>(rand()) / rrr);
	if (min > (INT_MAX - rand_result))
		return max;

	return std::clamp(min + rand_result, min, max);
}

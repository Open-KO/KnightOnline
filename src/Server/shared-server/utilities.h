#ifndef SERVER_SHAREDSERVER_UTILITIES_H
#define SERVER_SHAREDSERVER_UTILITIES_H

#pragma once

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

extern std::function<int(int min, int max)> myrand;

#endif // SERVER_SHAREDSERVER_UTILITIES_H

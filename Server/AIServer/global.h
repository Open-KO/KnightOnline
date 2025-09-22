#pragma once

#include <string_view>

bool	CheckGetVarString(int nLength, char* tBuf, const char* sBuf, int nSize, int& index);
int		GetVarString(char* tBuf, const char* sBuf, int nSize, int& index);
void	GetString(char* tBuf, const char* sBuf, int len, int& index);
BYTE	GetByte(const char* sBuf, int& index);
int		GetShort(const char* sBuf, int& index);
int		GetInt(const char* sBuf, int& index);
DWORD	GetDWORD(const char* sBuf, int& index);
float	Getfloat(const char* sBuf, int& index);
void	SetString(char* tBuf, const char* sBuf, int len, int& index);
void	SetVarString(char* tBuf, const char* sBuf, int len, int& index);
void	SetByte(char* tBuf, BYTE sByte, int& index);
void	SetShort(char* tBuf, int sShort, int& index);
void	SetInt(char* tBuf, int sInt, int& index);
void	SetDWORD(char* tBuf, DWORD sDword, int& index);
void	Setfloat(char* tBuf, float sFloat, int& index);
void	SetString1(char* tBuf, const std::string_view str, int& index);
void	SetString2(char* tBuf, const std::string_view str, int& index);
int		ParseSpace(char* tBuf, const char* sBuf);
CString	GetProgPath();
int		myrand(int min, int max, bool bSame = false);
int		XdY(int x, int y);

void	CheckMaxValue(DWORD& dest, DWORD add);
void	CheckMaxValue(int& dest, int add);
void	CheckMaxValue(short& dest, short add);
bool	CheckMaxValueReturn(DWORD& dest, DWORD add);

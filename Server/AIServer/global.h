#pragma once

//#include "stdafx.h"

BOOL	CheckGetVarString(int nLength, char* tBuf, char* sBuf, int nSize, int& index);
int		GetVarString(char* tBuf, char* sBuf, int nSize, int& index);
void	GetString(char* tBuf, char* sBuf, int len, int& index);
BYTE	GetByte(char* sBuf, int& index);
int		GetShort(char* sBuf, int& index);
int		GetInt(char* sBuf, int& index);
DWORD	GetDWORD(char* sBuf, int& index);
float	Getfloat(char* sBuf, int& index);
void	SetString(char* tBuf, const char* sBuf, int len, int& index);
void	SetVarString(char* tBuf, const char* sBuf, int len, int& index);
void	SetByte(char* tBuf, BYTE sByte, int& index);
void	SetShort(char* tBuf, int sShort, int& index);
void	SetInt(char* tBuf, int sInt, int& index);
void	SetDWORD(char* tBuf, DWORD sDword, int& index);
void	Setfloat(char* tBuf, float sFloat, int& index);
int		ParseSpace(char* tBuf, char* sBuf);
CString	GetProgPath();
void LogFileWrite(CString logstr);
int		myrand(int min, int max, BOOL bSame = FALSE);
int		XdY(int x, int y);
void	SpaceCopy(LPSTR source, CString dest, int size);
void	SpaceCopy(CString& source, CString dest, int size);

void	CheckMaxValue(DWORD& dest, DWORD add);
void	CheckMaxValue(int& dest, int add);
void	CheckMaxValue(short& dest, short add);
BOOL	CheckMaxValueReturn(DWORD& dest, DWORD add);
void	TimeTrace(TCHAR* pMsg);

#ifndef _DEFINE_H
#define _DEFINE_H

#include <shared/globals.h>
#include <shared/StringConversion.h>

// DEFINE Shared Memory Costumizing

#define MAX_PKTSIZE		512
#define MAX_COUNT		4096
#define SMQ_ITEMLOGGER	"ITEMLOG_SEND"

// Packet Define...
#define WIZ_ITEM_LOG		0x19	// Send To Agent for Writing Log
#define WIZ_DATASAVE		0x37	// User GameData DB Save Request

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
//	Global Function Define
//

inline void GetString(char* tBuf, char* sBuf, int len, int& index)
{
	memcpy(tBuf, sBuf + index, len);
	index += len;
}

inline uint8_t GetByte(char* sBuf, int& index)
{
	int t_index = index;
	index++;
	return (uint8_t) (*(sBuf + t_index));
}

inline int GetShort(char* sBuf, int& index)
{
	index += 2;
	return *(int16_t*) (sBuf + index - 2);
}

inline uint32_t GetDWORD(char* sBuf, int& index)
{
	index += 4;
	return *(uint32_t*) (sBuf + index - 4);
}

inline float Getfloat(char* sBuf, int& index)
{
	index += 4;
	return *(float*) (sBuf + index - 4);
}

inline void SetString(char* tBuf, char* sBuf, int len, int& index)
{
	memcpy(tBuf + index, sBuf, len);
	index += len;
}

inline void SetByte(char* tBuf, uint8_t sByte, int& index)
{
	*(tBuf + index) = (char) sByte;
	index++;
}

inline void SetShort(char* tBuf, int sShort, int& index)
{
	int16_t temp = (int16_t) sShort;

	memcpy(tBuf + index, &temp, 2);
	index += 2;
}

inline void SetDWORD(char* tBuf, uint32_t sDWORD, int& index)
{
	memcpy(tBuf + index, &sDWORD, 4);
	index += 4;
}

inline void Setfloat(char* tBuf, float sFloat, int& index)
{
	memcpy(tBuf + index, &sFloat, 4);
	index += 4;
}

inline void SetInt64(char* tBuf, int64_t nInt64, int& index)
{
	memcpy(tBuf + index, &nInt64, 8);
	index += 8;
}

inline int64_t GetInt64(char* sBuf, int& index)
{
	index += 8;
	return *(int64_t*) (sBuf + index - 8);
}

inline CString GetProgPath()
{
	TCHAR Buf[256], Path[256];
	TCHAR drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];

	::GetModuleFileName(AfxGetApp()->m_hInstance, Buf, 256);
	_tsplitpath(Buf, drive, dir, fname, ext);
	_tcscpy(Path, drive);
	_tcscat(Path, dir);
	return Path;
}

inline void LogFileWrite(LPCTSTR logstr)
{
	CString LogFileName;
	LogFileName.Format(_T("%s\\ItemManager.log"), GetProgPath().GetString());

	CFile file;
	if (!file.Open(LogFileName, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite))
		return;

	file.SeekToEnd();

#if defined(_UNICODE)
	const std::string utf8 = WideToUtf8(logstr, wcslen(logstr));
	file.Write(utf8.c_str(), static_cast<int>(utf8.size()));
#else
	file.Write(logstr, strlen(logstr));
#endif

	file.Close();
}

#endif

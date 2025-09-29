#ifndef _DEFINE_H
#define _DEFINE_H

#include <shared/globals.h>
#include <shared/StringConversion.h>

#define MAX_ITEM			28

////////////////////////////////////////////////////////////
// Socket Define
////////////////////////////////////////////////////////////
#define SOCKET_BUFF_SIZE	(1024*8)
#define MAX_PACKET_SIZE		(1024*2)

#define PACKET_START1		0XAA
#define PACKET_START2		0X55
#define PACKET_END1			0X55
#define PACKET_END2			0XAA
//#define PROTOCOL_VER		0X01

// status
#define STATE_CONNECTED		0X01
#define STATE_DISCONNECTED	0X02
#define STATE_GAMESTART		0x03

/////////////////////////////////////////////////////
// ITEM_SLOT DEFINE
#define RIGHTEAR			0
#define HEAD				1
#define LEFTEAR				2
#define NECK				3
#define BREAST				4
#define SHOULDER			5
#define RIGHTHAND			6
#define WAIST				7
#define LEFTHAND			8
#define RIGHTRING			9
#define LEG					10
#define LEFTRING			11
#define GLOVE				12
#define FOOT				13
/////////////////////////////////////////////////////

////////////////////////////////////////////////////////////

typedef union {
	int16_t		i;
	uint8_t		b[2];
} MYSHORT;

typedef union {
	int32_t		i;
	uint8_t		b[4];
} MYINT;

typedef union {
	uint32_t	w;
	uint8_t		b[4];
} MYDWORD;


// DEFINE Shared Memory Queue Flag

#define E				0x00
#define R				0x01
#define W				0x02
#define WR				0x03

// DEFINE Shared Memory Queue Return VALUE

#define SMQ_BROKEN		10000
#define SMQ_FULL		10001
#define SMQ_EMPTY		10002
#define SMQ_PKTSIZEOVER	10003
#define SMQ_WRITING		10004
#define SMQ_READING		10005

// DEFINE Shared Memory Costumizing

#define MAX_PKTSIZE		512
#define MAX_COUNT		4096
#define SMQ_ITEMLOGGER	"ITEMLOG_SEND"

// Packet Define...
#define WIZ_ITEM_LOG		0x19	// Send To Agent for Writing Log
#define WIZ_DATASAVE		0x37	// User GameData DB Save Request

#define SLOT_MAX			14		// 착용 아템 MAX
#define HAVE_MAX			28		// 소유 아템 MAX (인벤토리창)	
#define ITEMCOUNT_MAX		9999	// 소모 아이템 소유 한계값
#define WAREHOUSE_MAX		196		// 창고 아이템 MAX
/////////////////////////////////////////////////////////////////////////////////
// Structure Define
/////////////////////////////////////////////////////////////////////////////////


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

inline int DisplayErrorMsg(SQLHANDLE hstmt)
{
	SQLTCHAR      SqlState[6], Msg[1024];
	SQLINTEGER    NativeError;
	SQLSMALLINT   i, MsgLen;
	SQLRETURN     rc2;
	TCHAR		  logstr[512] = {};

	i = 1;
	while ((rc2 = SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, i, SqlState, &NativeError, Msg, _countof(Msg), &MsgLen)) != SQL_NO_DATA)
	{
		_stprintf(logstr, _T("*** %s, %d, %s, %d ***\r\n"), SqlState, NativeError, Msg, MsgLen);
		LogFileWrite(logstr);

		i++;
	}

	if (_tcscmp((TCHAR*) SqlState, _T("08S01")) == 0)
		return -1;
	else
		return 0;
}

#endif

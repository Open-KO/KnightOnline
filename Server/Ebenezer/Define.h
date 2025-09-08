#ifndef _DEFINE_H
#define _DEFINE_H

#include <mmsystem.h>
#include <shared/globals.h>
#include <shared/StringConversion.h>

constexpr int MAX_USER				= 3000;

constexpr int _LISTEN_PORT			= 15000;
constexpr int _UDP_PORT				= 8888;
constexpr int AI_KARUS_SOCKET_PORT	= 10020;
constexpr int AI_ELMO_SOCKET_PORT	= 10030;
constexpr int AI_BATTLE_SOCKET_PORT	= 10040;
constexpr int CLIENT_SOCKSIZE		= 100;
constexpr int MAX_AI_SOCKET			= 10;			// sungyong~ 2002.05.22

constexpr int MAX_TYPE3_REPEAT		= 20;
constexpr int MAX_TYPE4_BUFF		= 9;

constexpr int MAX_ITEM				= 28;

constexpr int NPC_HAVE_ITEM_LIST	= 6;
constexpr int ZONEITEM_MAX			= 2'100'000'000;	// 존에 떨어지는 최대 아이템수...

constexpr int SERVER_INFO_START		= 1;
constexpr int SERVER_INFO_END		= 2;

//////////////  Quest 관련 Define ////////////////////////////
constexpr int MAX_EVENT				= 2000;
constexpr int MAX_EVENT_SIZE		= 400;
constexpr int MAX_EVENT_NUM			= 2000;
constexpr int MAX_EXEC_INT			= 30;
constexpr int MAX_LOGIC_ELSE_INT	= 10;
constexpr int MAX_MESSAGE_EVENT		= 10;
constexpr int MAX_COUPON_ID_LENGTH	= 20;
constexpr int MAX_CURRENT_EVENT		= 20;

// 지금 쓰이는것만 정의 해 놨습니다.
// logic관련 define
enum e_LogicCheck
{
	LOGIC_CHECK_UNDER_WEIGHT		= 0x01,
	LOGIC_CHECK_OVER_WEIGHT			= 0x02,
	LOGIC_CHECK_SKILL_POINT			= 0x03,
	LOGIC_EXIST_ITEM				= 0x04,
	LOGIC_CHECK_CLASS				= 0x05,
	LOGIC_CHECK_WEIGHT				= 0x06,
	LOGIC_CHECK_EDITBOX				= 0x07,
	LOGIC_RAND						= 0x08,
	LOGIC_HOWMUCH_ITEM				= 0x09,
	LOGIC_CHECK_LEVEL				= 0x0A,
	LOGIC_NOEXIST_COM_EVENT			= 0x0B,
	LOGIC_EXIST_COM_EVENT			= 0x0C,
	LOGIC_CHECK_NOAH				= 0x0D,
	LOGIC_NOEXIST_ITEM				= 0x0E,
};

// 실행관련 define
enum e_Exec
{
	EXEC_SAY						= 0x01,
	EXEC_SELECT_MSG					= 0x02,
	EXEC_RUN_EVENT					= 0x03,
	EXEC_GIVE_ITEM					= 0x04,
	EXEC_ROB_ITEM					= 0x05,
	EXEC_RETURN						= 0x06,
	EXEC_OPEN_EDITBOX				= 0x07,
	EXEC_GIVE_NOAH					= 0x08,
	EXEC_LOG_COUPON_ITEM			= 0x09,
	EXEC_SAVE_COM_EVENT				= 0x0A,
	EXEC_ROB_NOAH					= 0x0B
};

// EVENT 시작 번호들 :)
constexpr int EVENT_POTION			= 1;
constexpr int EVENT_FT_1			= 20;
constexpr int EVENT_FT_3			= 36;
constexpr int EVENT_FT_2			= 50;
constexpr int EVENT_LOGOS_ELMORAD	= 1001;
constexpr int EVENT_LOGOS_KARUS		= 2001;
constexpr int EVENT_COUPON			= 4001;

////////////////////////////////////////////////////////////

///////////////// BBS RELATED //////////////////////////////
constexpr int MAX_BBS_PAGE			= 23;
constexpr int MAX_BBS_MESSAGE		= 40;
constexpr int MAX_BBS_TITLE			= 20;
constexpr int MAX_BBS_POST			= 500;

constexpr int BUY_POST_PRICE		= 500;
constexpr int SELL_POST_PRICE		= 1000;

constexpr int REMOTE_PURCHASE_PRICE	= 5000;
constexpr int BBS_CHECK_TIME		= 36000;

///////////////// NATION ///////////////////////////////////
#define UNIFY_NATION		0
#define KARUS               1
#define ELMORAD             2
#define BATTLE				3

#define BATTLE_ZONE			101

////////////////////////////////////////////////////////////

// Attack Type
#define DIRECT_ATTACK		0
#define LONG_ATTACK			1
#define MAGIC_ATTACK		2
#define DURATION_ATTACK		3

////////////////// ETC Define //////////////////////////////
// UserInOut //
#define USER_IN					0X01
#define USER_OUT				0X02
#define USER_REGENE				0X03	// Userin하고 같은것인데 효과를 주기위해서.. 분리(게임시작, 리젠. 소환시)
#define USER_WARP				0X04
#define USER_SUMMON				0X05
#define NPC_IN					0X01
#define NPC_OUT					0X02

////////////////// Resurrection related ////////////////////
#define BLINK_TIME				10
#define CLAN_SUMMON_TIME		180
////////////////////////////////////////////////////////////

// Socket Define
////////////////////////////////////////////////////////////
#define SOCKET_BUFF_SIZE	(1024*16)
#define MAX_PACKET_SIZE		(1024*8)
#define REGION_BUFF_SIZE	(1024*16)

#define PACKET_START1				0XAA
#define PACKET_START2				0X55
#define PACKET_END1					0X55
#define PACKET_END2					0XAA

// status
#define STATE_CONNECTED			0X01
#define STATE_DISCONNECTED		0X02
#define STATE_GAMESTART			0x03

// Socket type
#define TYPE_ACCEPT				0x01
#define TYPE_CONNECT			0x02

// Overlapped flag
#define OVL_RECEIVE				0X01
#define OVL_SEND				0X02
#define OVL_CLOSE				0X03
////////////////////////////////////////////////////////////

// ==================================================================
//	About Map Object
// ==================================================================
#define USER_BAND				0			// Map 위에 유저가 있다.
#define NPC_BAND				10000		// Map 위에 NPC(몹포함)가 있다.
#define INVALID_BAND			30000		// 잘못된 ID BAND

#define EVENT_MONSTER			20			// Event monster 총 수

///////////////// snow event define //////////////////////////////
#define SNOW_EVENT_MONEY		2000
#define SNOW_EVENT_SKILL		490043

//////////////////////////////////////////////////////////////////
// DEFINE Shared Memory Queue
//////////////////////////////////////////////////////////////////

#define E	0x00
#define R	0x01
#define W	0x02
#define WR	0x03

// DEFINE Shared Memory Queue Return VALUE

#define SMQ_BROKEN		10000
#define SMQ_FULL		10001
#define SMQ_EMPTY		10002
#define SMQ_PKTSIZEOVER	10003
#define SMQ_WRITING		10004
#define SMQ_READING		10005
#define SMQ_INVALID		10006

// DEFINE Shared Memory Costumizing

#define MAX_PKTSIZE		512
#define MAX_COUNT		4096
#define SMQ_LOGGERSEND	"KNIGHT_SEND"
#define SMQ_LOGGERRECV	"KNIGHT_RECV"

#define SMQ_ITEMLOGGER	"ITEMLOG_SEND"

// Reply packet define...

#define SEND_ME					0x01
#define SEND_REGION				0x02
#define SEND_ALL				0x03
#define SEND_ZONE				0x04

// Battlezone Announcement
#define BATTLEZONE_OPEN					0x00
#define BATTLEZONE_CLOSE				0x01           
#define DECLARE_WINNER					0x02
#define DECLARE_LOSER					0x03
#define DECLARE_BAN						0x04
#define KARUS_CAPTAIN_NOTIFY			0x05
#define ELMORAD_CAPTAIN_NOTIFY			0x06
#define KARUS_CAPTAIN_DEPRIVE_NOTIFY	0x07
#define ELMORAD_CAPTAIN_DEPRIVE_NOTIFY	0x08
#define SNOW_BATTLEZONE_OPEN			0x09

// Battle define
#define NO_BATTLE				0
#define NATION_BATTLE			1
#define SNOW_BATTLE				2

#define MAX_BATTLE_ZONE_USERS	150

//////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
typedef union
{
	WORD		w;
	BYTE		b[2];
} MYSHORT;

typedef union
{
	int			i;
	BYTE		b[4];
} MYINT;

typedef union
{
	DWORD		w;
	BYTE		b[4];
} MYDWORD;

typedef union
{
	int64_t		i;
	BYTE		b[8];
} MYINT64;

struct _REGION_BUFFER
{
	int		iLength;
	BYTE	bFlag;
	DWORD	dwThreadID;

	char	pDataBuff[REGION_BUFF_SIZE];

	_REGION_BUFFER()
	{
		iLength = 0;
		bFlag = E;
		dwThreadID = 0;
		memset(pDataBuff, 0, sizeof(pDataBuff));
	}
};

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
//	Global Function Define
//

inline void GetString(char* tBuf, const char* sBuf, int len, int& index)
{
	memcpy(tBuf, sBuf + index, len);
	index += len;
}

inline BYTE GetByte(const char* sBuf, int& index)
{
	int t_index = index;
	index++;
	return (BYTE) (*(sBuf + t_index));
}

inline int GetShort(const char* sBuf, int& index)
{
	index += 2;
	return *(short*) (sBuf + index - 2);
}

inline DWORD GetDWORD(const char* sBuf, int& index)
{
	index += 4;
	return *(DWORD*) (sBuf + index - 4);
}

inline float Getfloat(const char* sBuf, int& index)
{
	index += 4;
	return *(float*) (sBuf + index - 4);
}

inline int64_t GetInt64(const char* sBuf, int& index)
{
	index += 8;
	return *(int64_t*) (sBuf + index - 8);
}

inline void SetString(char* tBuf, const char* sBuf, int len, int& index)
{
	memcpy(tBuf + index, sBuf, len);
	index += len;
}

inline void SetByte(char* tBuf, BYTE sByte, int& index)
{
	*(tBuf + index) = (char) sByte;
	index++;
}

inline void SetShort(char* tBuf, int sShort, int& index)
{
	short temp = (short) sShort;

	CopyMemory(tBuf + index, &temp, 2);
	index += 2;
}

inline void SetDWORD(char* tBuf, DWORD sDWORD, int& index)
{
	CopyMemory(tBuf + index, &sDWORD, 4);
	index += 4;
}

inline void Setfloat(char* tBuf, float sFloat, int& index)
{
	CopyMemory(tBuf + index, &sFloat, 4);
	index += 4;
}

inline void SetInt64(char* tBuf, int64_t nInt64, int& index)
{
	CopyMemory(tBuf + index, &nInt64, 8);
	index += 8;
}

inline void SetString1(char* tBuf, const char* sBuf, BYTE len, int& index)
{
	SetByte(tBuf, len, index);
	SetString(tBuf, sBuf, len, index);
}

inline void SetString2(char* tBuf, const char* sBuf, short len, int& index)
{
	SetShort(tBuf, len, index);
	SetString(tBuf, sBuf, len, index);
}

inline void SetString1(char* tBuf, const std::string_view str, int& index)
{
	BYTE length = static_cast<BYTE>(str.length());
	SetByte(tBuf, length, index);
	SetString(tBuf, str.data(), length, index);
}

inline void SetString2(char* tBuf, const std::string_view str, int& index)
{
	short length = static_cast<short>(str.length());
	SetShort(tBuf, length, index);
	SetString(tBuf, str.data(), length, index);
}

// sungyong 2001.11.06
inline int GetVarString(char* tBuf, const char* sBuf, int nSize, int& index)
{
	int nLen = 0;
	if (nSize == sizeof(BYTE))
		nLen = GetByte(sBuf, index);
	else
		nLen = GetShort(sBuf, index);

	GetString(tBuf, sBuf, nLen, index);
	*(tBuf + nLen) = 0;

	return nLen;
}

inline void SetVarString(char* tBuf, const char* sBuf, int len, int& index)
{
	*(tBuf + index) = (BYTE) len;
	index ++;

	CopyMemory(tBuf + index, sBuf, len);
	index += len;
}

// ~sungyong 2001.11.06
inline int ParseSpace(char* tBuf, char* sBuf)
{
	int i = 0, index = 0;
	BOOL flag = FALSE;

	while (sBuf[index] == ' '
		|| sBuf[index] == '\t')
		index++;

	while (sBuf[index] != ' '
		&& sBuf[index] != '\t'
		&& sBuf[index] != (BYTE) 0)
	{
		tBuf[i++] = sBuf[index++];
		flag = TRUE;
	}

	tBuf[i] = 0;

	while (sBuf[index] == ' '
		|| sBuf[index] == '\t')
		index++;

	if (!flag)
		return 0;

	return index;
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

inline int myrand(int min, int max)
{
	if (min == max)
		return min;

	if (min > max)
		std::swap(min, max);

	double gap = max - min + 1;
	double rrr = (double) RAND_MAX / gap;

	double rand_result;

	rand_result = (double) rand() / rrr;

	if ((int) (min + (int) rand_result) < min)
		return min;

	if ((int) (min + (int) rand_result) > max)
		return max;

	return (int) (min + (int) rand_result);
}

inline float TimeGet()
{
	static bool bInit = false;
	static bool bUseHWTimer = FALSE;
	static LARGE_INTEGER nTime, nFrequency;

	if (!bInit)
	{
		if (::QueryPerformanceCounter(&nTime))
		{
			::QueryPerformanceFrequency(&nFrequency);
			bUseHWTimer = TRUE;
		}
		else
		{
			bUseHWTimer = FALSE;
		}

		bInit = true;
	}

	if (bUseHWTimer)
	{
		::QueryPerformanceCounter(&nTime);
		return (float) ((double) (nTime.QuadPart) / (double) nFrequency.QuadPart);
	}

	return (float) timeGetTime();
}

#endif

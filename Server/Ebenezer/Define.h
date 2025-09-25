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
	LOGIC_CHECK_NONE						= 0x00,
	LOGIC_CHECK_UNDER_WEIGHT				= 0x01,
	LOGIC_CHECK_OVER_WEIGHT					= 0x02,
	LOGIC_CHECK_SKILL_POINT					= 0x03,
	LOGIC_CHECK_EXIST_ITEM					= 0x04,
	LOGIC_CHECK_CLASS						= 0x05,
	LOGIC_CHECK_WEIGHT						= 0x06,
	LOGIC_CHECK_EDITBOX						= 0x07,
	LOGIC_RAND								= 0x08,
	LOGIC_HOWMUCH_ITEM						= 0x09,
	LOGIC_CHECK_LV 							= 0x0A,
	LOGIC_NOEXIST_COM_EVENT					= 0x0B,
	LOGIC_EXIST_COM_EVENT					= 0x0C,
	LOGIC_CHECK_NOAH						= 0x0D,
	LOGIC_CHECK_NATION						= 0x0E,
	LOGIC_CHECK_PPCARD_SERIAL				= 0x0F,
	LOGIC_CHECK_PPCARD_TYPE					= 0x10,
	LOGIC_CHECK_EXIST_ITEM_AND				= 0x11,
	LOGIC_CHECK_EXIST_ITEM_OR				= 0x12,
	LOGIC_CHECK_NOEXIST_ITEM_AND			= 0x13,
	LOGIC_CHECK_NOEXIST_ITEM_OR				= 0x14,
	LOGIC_CHECK_EXIST_EVENT					= 0x15,
	LOGIC_CHECK_NOEXIST_EVENT				= 0x16,
	LOGIC_CHECK_PROMOTION_ELIGIBLE			= 0x17,
	LOGIC_CHECK_EXCHANGE					= 0x18,
	LOGIC_CHECK_NOEXIST_ITEM				= 0x19,
	LOGIC_CHECK_ITEMCHANGE_NUM				= 0x1A,
	LOGIC_CHECK_NOCLASS						= 0x1B,
	LOGIC_CHECK_LOYALTY						= 0x1C,
	LOGIC_CHECK_CHIEF						= 0x1D,
	LOGIC_CHECK_NO_CHIEF					= 0x1E,
	LOGIC_CHECK_CLAN_GRADE					= 0x1F,
	LOGIC_CHECK_KNIGHT						= 0x20,
	LOGIC_CHECK_DICE						= 0x21,
	LOGIC_CHECK_CLAN						= 0x22,
	LOGIC_CHECK_NO_CLAN						= 0x23,
	LOGIC_CHECK_MANNER						= 0x24,
	LOGIC_CHECK_MONSTER_CHALLENGE_TIME		= 0x25,
	LOGIC_CHECK_MONSTER_CHALLENGE_USERCOUNT	= 0x26,
	LOGIC_CHECK_CASTLE						= 0x27,
	LOGIC_CHECK_NO_CASTLE					= 0x28,
	LOGIC_CHECK_SKILL_TOTAL					= 0x29,
	LOGIC_CHECK_STAT_TOTAL					= 0x2A,
	LOGIC_CHECK_EMPTY_SLOT					= 0x2B,
	LOGIC_CHECK_LOYALTY_RANK_MONTHLY		= 0x2C,
	LOGIC_CHECK_LOYALTY_RANK				= 0x2D,
	LOGIC_CHECK_CLAN_RANKING				= 0x2E,
	LOGIC_CHECK_MIDDLE_STATUE_CAPTURE		= 0x2F,
	LOGIC_CHECK_MIDDLE_STATUE_NOCAPTURE		= 0x30,
	LOGIC_CHECK_INPUT_COUNT					= 0x31,
	LOGIC_CHECK_EXIST_ITEM_INPUT_COUNT		= 0x32,
	LOGIC_CHECK_WEIGHT_INPUT_COUNT			= 0x33,
	LOGIC_CHECK_EXAM_COUNT					= 0x34,
	LOGIC_CHECK_BEEF_ROAST_KARUS_VICTORY	= 0x35,
	LOGIC_CHECK_BEEF_ROAST_ELMORAD_VICTORY	= 0x36,
	LOGIC_CHECK_BEEF_ROAST_NO_VICTORY		= 0x37
};

// 실행관련 define
enum e_Exec
{
	EXEC_NONE								= 0x00,
	EXEC_SAY								= 0x01,
	EXEC_SELECT_MSG							= 0x02,
	EXEC_RUN_EVENT							= 0x03,
	EXEC_GIVE_ITEM							= 0x04,
	EXEC_ROB_ITEM 							= 0x05,
	EXEC_RETURN								= 0x06,
	EXEC_OPEN_EDITBOX						= 0x07,
	EXEC_GIVE_NOAH							= 0x08,
	EXEC_LOG_COUPON_ITEM					= 0x09,
	EXEC_SAVE_COM_EVENT						= 0x0A,
	EXEC_ROB_NOAH							= 0x0B,
	EXEC_REQUEST_REWARD						= 0x0C,
	EXEC_GIVE_PPCARD_ITEM					= 0x0D,
	EXEC_SAVE_EVENT							= 0x0E,
	EXEC_PROMOTE_USER						= 0x0F,
	EXEC_GIVE_PROMOTION_QUEST				= 0x10,
	EXEC_RUN_EXCHANGE						= 0x11,
	EXEC_KISS_USER							= 0x12,
	EXEC_ZONE_CHANGE						= 0x13,
	EXEC_PROMOTE_USER_NOVICE				= 0x14,
	EXEC_SKILL_POINT_DISTRIBUTE				= 0x15,
	EXEC_STAT_POINT_DISTRIBUTE				= 0x16,
	EXEC_LEVEL_UP							= 0x17,
	EXEC_EXP_CHANGE							= 0x18,
	EXEC_DESTROY_ITEM						= 0x19,
	EXEC_PROMOTE_KNIGHT						= 0x1A,
	EXEC_CHANGE_POSITION					= 0x1B,
	EXEC_ROLL_DICE							= 0x1C,
	EXEC_ZONE_CHANGE_CLAN					= 0x1D,
	EXEC_CHANGE_LOYALTY						= 0x1E,
	EXEC_SKILL_POINT_FREE					= 0x1F,
	EXEC_STAT_POINT_FREE					= 0x20,
	EXEC_CHANGE_NAME						= 0x21,
	EXEC_SEND_WEBPAGE_ADDRESS				= 0x22,
	EXEC_ROB_ALLITEM_PARTY					= 0x23,
	EXEC_ZONE_CHANGE_PARTY					= 0x24,
	EXEC_STATE_CHANGE						= 0x25,
	EXEC_EMIGRATION_ACCEPT					= 0x26,
	EXEC_CHANGE_MANNER						= 0x27,
	EXEC_SHOW_PCBANG_ITEM					= 0x28,
	EXEC_CHECK_PCBANG_ITEM					= 0x29,
	EXEC_GIVE_PCBANG_ITEM					= 0x3A,
	EXEC_CHECK_PCBANG_OWNER					= 0x3B,
	EXEC_REQUEST_PERSONAL_RANK_REWARD		= 0x3C,
	EXEC_CHECK_KJWAR_ACCOUNT				= 0x3D,
	EXEC_GIVE_KJWAR_ITEM					= 0x3E,
	EXEC_CHECK_LOGTIME_ACCOUNT				= 0x3F,
	EXEC_GIVE_LOGTIME_ITEM					= 0x40,
	EXEC_MOVE_MIDDLE_STATUE					= 0x41,
	EXEC_CHECK_OLYMPIC_ACCOUNT				= 0x42,
	EXEC_LOG_OLYMPIC_ACCOUNT				= 0x43,
	EXEC_OPEN_INPUT_COUNT					= 0x44,
	EXEC_ROB_ITEM_INPUT_COUNT				= 0x45,
	EXEC_GIVE_ITEM_INPUT_COUNT				= 0x46,
	EXEC_ROB_EXAM							= 0x47
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

// TODO: Throw these away. They're too vague and contaminate things.
constexpr uint8_t E		= 0x00;
constexpr uint8_t R		= 0x01;
constexpr uint8_t W		= 0x02;
constexpr uint8_t WR	= 0x03;

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
	uint16_t	w;
	uint8_t		b[2];
} MYSHORT;

typedef union
{
	int32_t		i;
	uint8_t		b[4];
} MYINT;

typedef union
{
	uint32_t	w;
	uint8_t		b[4];
} MYDWORD;

typedef union
{
	int64_t		i;
	uint8_t		b[8];
} MYINT64;

struct _REGION_BUFFER
{
	int			iLength;
	uint8_t		bFlag;
	uint32_t	dwThreadID;

	char		pDataBuff[REGION_BUFF_SIZE];

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

inline uint8_t GetByte(const char* sBuf, int& index)
{
	int t_index = index;
	index++;
	return (uint8_t) (*(sBuf + t_index));
}

inline int GetShort(const char* sBuf, int& index)
{
	index += 2;
	return *(int16_t*) (sBuf + index - 2);
}

inline uint32_t GetDWORD(const char* sBuf, int& index)
{
	index += 4;
	return *(uint32_t*) (sBuf + index - 4);
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

inline void SetByte(char* tBuf, uint8_t sByte, int& index)
{
	*(tBuf + index) = (char) sByte;
	index++;
}

inline void SetShort(char* tBuf, int sShort, int& index)
{
	int16_t temp = (int16_t) sShort;

	CopyMemory(tBuf + index, &temp, 2);
	index += 2;
}

inline void SetDWORD(char* tBuf, uint32_t sDWORD, int& index)
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

inline void SetString1(char* tBuf, const char* sBuf, uint8_t len, int& index)
{
	SetByte(tBuf, len, index);
	SetString(tBuf, sBuf, len, index);
}

inline void SetString2(char* tBuf, const char* sBuf, int16_t len, int& index)
{
	SetShort(tBuf, len, index);
	SetString(tBuf, sBuf, len, index);
}

inline void SetString1(char* tBuf, const std::string_view str, int& index)
{
	uint8_t length = static_cast<uint8_t>(str.length());
	SetByte(tBuf, length, index);
	SetString(tBuf, str.data(), length, index);
}

inline void SetString2(char* tBuf, const std::string_view str, int& index)
{
	int16_t length = static_cast<int16_t>(str.length());
	SetShort(tBuf, length, index);
	SetString(tBuf, str.data(), length, index);
}

// sungyong 2001.11.06
inline int GetVarString(char* tBuf, const char* sBuf, int nSize, int& index)
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

inline void SetVarString(char* tBuf, const char* sBuf, int len, int& index)
{
	*(tBuf + index) = (uint8_t) len;
	index ++;

	CopyMemory(tBuf + index, sBuf, len);
	index += len;
}

// ~sungyong 2001.11.06
inline int ParseSpace(char* tBuf, const char* sBuf)
{
	int i = 0, index = 0;
	bool flag = false;

	while (sBuf[index] == ' '
		|| sBuf[index] == '\t')
		index++;

	while (sBuf[index] != ' '
		&& sBuf[index] != '\t'
		&& sBuf[index] != (uint8_t) 0)
	{
		tBuf[i++] = sBuf[index++];
		flag = true;
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
	static bool bUseHWTimer = false;
	static LARGE_INTEGER nTime, nFrequency;

	if (!bInit)
	{
		if (::QueryPerformanceCounter(&nTime))
		{
			::QueryPerformanceFrequency(&nFrequency);
			bUseHWTimer = true;
		}
		else
		{
			bUseHWTimer = false;
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

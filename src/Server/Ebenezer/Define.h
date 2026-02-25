#ifndef SERVER_EBENEZER_DEFINE_H
#define SERVER_EBENEZER_DEFINE_H

#pragma once

#include <shared/globals.h>

namespace Ebenezer
{

inline constexpr int MAX_USER              = 3000;

inline constexpr int _LISTEN_PORT          = 15000;
inline constexpr int _UDP_PORT             = 8888;
inline constexpr int AI_KARUS_SOCKET_PORT  = 10020;
inline constexpr int AI_ELMO_SOCKET_PORT   = 10030;
inline constexpr int AI_BATTLE_SOCKET_PORT = 10040;
inline constexpr int CLIENT_SOCKSIZE       = 100;
inline constexpr int MAX_AI_SOCKET         = 10;

inline constexpr int MAX_TYPE3_REPEAT      = 20;
inline constexpr int MAX_TYPE4_BUFF        = 9;

inline constexpr int MAX_ITEM              = 28;

inline constexpr int NPC_HAVE_ITEM_LIST    = 6;
inline constexpr int ZONEITEM_MAX          = 2'100'000'000; // 존에 떨어지는 최대 아이템수...

inline constexpr int SERVER_INFO_START     = 1;
inline constexpr int SERVER_INFO_END       = 2;

//////////////  Quest 관련 Define ////////////////////////////
inline constexpr int MAX_EVENT             = 2000;
inline constexpr int MAX_EVENT_SIZE        = 400;
inline constexpr int MAX_EVENT_NUM         = 2000;
inline constexpr int MAX_EXEC_INT          = 30;
inline constexpr int MAX_LOGIC_ELSE_INT    = 10;
inline constexpr int MAX_MESSAGE_EVENT     = 10;
inline constexpr int MAX_COUPON_ID_LENGTH  = 20;
inline constexpr int MAX_CURRENT_EVENT     = 20;

// 지금 쓰이는것만 정의 해 놨습니다.
// logic관련 define
enum e_LogicCheck : uint8_t
{
	LOGIC_CHECK_NONE                        = 0x00,
	LOGIC_CHECK_UNDER_WEIGHT                = 0x01,
	LOGIC_CHECK_OVER_WEIGHT                 = 0x02,
	LOGIC_CHECK_SKILL_POINT                 = 0x03,
	LOGIC_CHECK_EXIST_ITEM                  = 0x04,
	LOGIC_CHECK_CLASS                       = 0x05,
	LOGIC_CHECK_WEIGHT                      = 0x06,
	LOGIC_CHECK_EDITBOX                     = 0x07,
	LOGIC_RAND                              = 0x08,
	LOGIC_HOWMUCH_ITEM                      = 0x09,
	LOGIC_CHECK_LV                          = 0x0A,
	LOGIC_NOEXIST_COM_EVENT                 = 0x0B,
	LOGIC_EXIST_COM_EVENT                   = 0x0C,
	LOGIC_CHECK_NOAH                        = 0x0D,
	LOGIC_CHECK_NATION                      = 0x0E,
	LOGIC_CHECK_PPCARD_SERIAL               = 0x0F,
	LOGIC_CHECK_PPCARD_TYPE                 = 0x10,
	LOGIC_CHECK_EXIST_ITEM_AND              = 0x11,
	LOGIC_CHECK_EXIST_ITEM_OR               = 0x12,
	LOGIC_CHECK_NOEXIST_ITEM_AND            = 0x13,
	LOGIC_CHECK_NOEXIST_ITEM_OR             = 0x14,
	LOGIC_CHECK_EXIST_EVENT                 = 0x15,
	LOGIC_CHECK_NOEXIST_EVENT               = 0x16,
	LOGIC_CHECK_PROMOTION_ELIGIBLE          = 0x17,
	LOGIC_CHECK_EXCHANGE                    = 0x18,
	LOGIC_CHECK_NOEXIST_ITEM                = 0x19,
	LOGIC_CHECK_ITEMCHANGE_NUM              = 0x1A,
	LOGIC_CHECK_NOCLASS                     = 0x1B,
	LOGIC_CHECK_LOYALTY                     = 0x1C,
	LOGIC_CHECK_CHIEF                       = 0x1D,
	LOGIC_CHECK_NO_CHIEF                    = 0x1E,
	LOGIC_CHECK_CLAN_GRADE                  = 0x1F,
	LOGIC_CHECK_KNIGHT                      = 0x20,
	LOGIC_CHECK_DICE                        = 0x21,
	LOGIC_CHECK_CLAN                        = 0x22,
	LOGIC_CHECK_NO_CLAN                     = 0x23,
	LOGIC_CHECK_MANNER                      = 0x24,
	LOGIC_CHECK_MONSTER_CHALLENGE_TIME      = 0x25,
	LOGIC_CHECK_MONSTER_CHALLENGE_USERCOUNT = 0x26,
	LOGIC_CHECK_CASTLE                      = 0x27,
	LOGIC_CHECK_NO_CASTLE                   = 0x28,
	LOGIC_CHECK_SKILL_TOTAL                 = 0x29,
	LOGIC_CHECK_STAT_TOTAL                  = 0x2A,
	LOGIC_CHECK_EMPTY_SLOT                  = 0x2B,
	LOGIC_CHECK_LOYALTY_RANK_MONTHLY        = 0x2C,
	LOGIC_CHECK_LOYALTY_RANK                = 0x2D,
	LOGIC_CHECK_CLAN_RANKING                = 0x2E,
	LOGIC_CHECK_MIDDLE_STATUE_CAPTURE       = 0x2F,
	LOGIC_CHECK_MIDDLE_STATUE_NOCAPTURE     = 0x30,
	LOGIC_CHECK_INPUT_COUNT                 = 0x31,
	LOGIC_CHECK_EXIST_ITEM_INPUT_COUNT      = 0x32,
	LOGIC_CHECK_WEIGHT_INPUT_COUNT          = 0x33,
	LOGIC_CHECK_EXAM_COUNT                  = 0x34,
	LOGIC_CHECK_BEEF_ROAST_KARUS_VICTORY    = 0x35,
	LOGIC_CHECK_BEEF_ROAST_ELMORAD_VICTORY  = 0x36,
	LOGIC_CHECK_BEEF_ROAST_NO_VICTORY       = 0x37
};

// 실행관련 define
enum e_Exec : uint8_t
{
	EXEC_NONE                         = 0x00,
	EXEC_SAY                          = 0x01,
	EXEC_SELECT_MSG                   = 0x02,
	EXEC_RUN_EVENT                    = 0x03,
	EXEC_GIVE_ITEM                    = 0x04,
	EXEC_ROB_ITEM                     = 0x05,
	EXEC_RETURN                       = 0x06,
	EXEC_OPEN_EDITBOX                 = 0x07,
	EXEC_GIVE_NOAH                    = 0x08,
	EXEC_LOG_COUPON_ITEM              = 0x09,
	EXEC_SAVE_COM_EVENT               = 0x0A,
	EXEC_ROB_NOAH                     = 0x0B,
	EXEC_REQUEST_REWARD               = 0x0C,
	EXEC_GIVE_PPCARD_ITEM             = 0x0D,
	EXEC_SAVE_EVENT                   = 0x0E,
	EXEC_PROMOTE_USER                 = 0x0F,
	EXEC_GIVE_PROMOTION_QUEST         = 0x10,
	EXEC_RUN_EXCHANGE                 = 0x11,
	EXEC_KISS_USER                    = 0x12,
	EXEC_ZONE_CHANGE                  = 0x13,
	EXEC_PROMOTE_USER_NOVICE          = 0x14,
	EXEC_SKILL_POINT_DISTRIBUTE       = 0x15,
	EXEC_STAT_POINT_DISTRIBUTE        = 0x16,
	EXEC_LEVEL_UP                     = 0x17,
	EXEC_EXP_CHANGE                   = 0x18,
	EXEC_DESTROY_ITEM                 = 0x19,
	EXEC_PROMOTE_KNIGHT               = 0x1A,
	EXEC_CHANGE_POSITION              = 0x1B,
	EXEC_ROLL_DICE                    = 0x1C,
	EXEC_ZONE_CHANGE_CLAN             = 0x1D,
	EXEC_CHANGE_LOYALTY               = 0x1E,
	EXEC_SKILL_POINT_FREE             = 0x1F,
	EXEC_STAT_POINT_FREE              = 0x20,
	EXEC_CHANGE_NAME                  = 0x21,
	EXEC_SEND_WEBPAGE_ADDRESS         = 0x22,
	EXEC_ROB_ALLITEM_PARTY            = 0x23,
	EXEC_ZONE_CHANGE_PARTY            = 0x24,
	EXEC_STATE_CHANGE                 = 0x25,
	EXEC_EMIGRATION_ACCEPT            = 0x26,
	EXEC_CHANGE_MANNER                = 0x27,
	EXEC_SHOW_PCBANG_ITEM             = 0x28,
	EXEC_CHECK_PCBANG_ITEM            = 0x29,
	EXEC_GIVE_PCBANG_ITEM             = 0x3A,
	EXEC_CHECK_PCBANG_OWNER           = 0x3B,
	EXEC_REQUEST_PERSONAL_RANK_REWARD = 0x3C,
	EXEC_CHECK_KJWAR_ACCOUNT          = 0x3D,
	EXEC_GIVE_KJWAR_ITEM              = 0x3E,
	EXEC_CHECK_LOGTIME_ACCOUNT        = 0x3F,
	EXEC_GIVE_LOGTIME_ITEM            = 0x40,
	EXEC_MOVE_MIDDLE_STATUE           = 0x41,
	EXEC_CHECK_OLYMPIC_ACCOUNT        = 0x42,
	EXEC_LOG_OLYMPIC_ACCOUNT          = 0x43,
	EXEC_OPEN_INPUT_COUNT             = 0x44,
	EXEC_ROB_ITEM_INPUT_COUNT         = 0x45,
	EXEC_GIVE_ITEM_INPUT_COUNT        = 0x46,
	EXEC_ROB_EXAM                     = 0x47
};

// EVENT 시작 번호들 :)
inline constexpr int EVENT_POTION          = 1;
inline constexpr int EVENT_FT_1            = 20;
inline constexpr int EVENT_FT_3            = 36;
inline constexpr int EVENT_FT_2            = 50;
inline constexpr int EVENT_LOGOS_ELMORAD   = 1001;
inline constexpr int EVENT_LOGOS_KARUS     = 2001;
inline constexpr int EVENT_COUPON          = 4001;

////////////////////////////////////////////////////////////

///////////////// BBS RELATED //////////////////////////////
inline constexpr int MAX_BBS_PAGE          = 23;
inline constexpr int MAX_BBS_MESSAGE       = 40;
inline constexpr int MAX_BBS_TITLE         = 20;
inline constexpr int MAX_BBS_POST          = 500;

inline constexpr int BUY_POST_PRICE        = 500;
inline constexpr int SELL_POST_PRICE       = 1000;

inline constexpr int REMOTE_PURCHASE_PRICE = 5000;
inline constexpr int BBS_CHECK_TIME        = 36000;

///////////////// NATION ///////////////////////////////////
enum e_Nation : uint8_t
{
	NATION_KARUS   = 1,
	NATION_ELMORAD = 2
};

enum e_ServerZoneType : uint8_t
{
	SERVER_ZONE_UNIFY   = 0,
	SERVER_ZONE_KARUS   = 1,
	SERVER_ZONE_ELMORAD = 2,
	SERVER_ZONE_BATTLE  = 3
};

////////////////////////////////////////////////////////////

// Attack Type
enum e_AttackType : uint8_t
{
	DIRECT_ATTACK   = 0,
	LONG_ATTACK     = 1,
	MAGIC_ATTACK    = 2,
	DURATION_ATTACK = 3
};

////////////////// ETC Define //////////////////////////////
// UserInOut //
enum e_UserInOutType : uint8_t
{
	USER_IN     = 1,
	USER_OUT    = 2,
	// Userin하고 같은것인데 효과를 주기위해서.. 분리(게임시작, 리젠. 소환시)
	USER_REGENE = 3,
	USER_WARP   = 4,
	USER_SUMMON = 5
};

enum e_NpcInOutType : uint8_t
{
	NPC_IN  = 1,
	NPC_OUT = 2
};

////////////////// Resurrection related ////////////////////
inline constexpr int BLINK_TIME                 = 10;
inline constexpr int CLAN_SUMMON_TIME           = 180;
////////////////////////////////////////////////////////////

// Socket Define
////////////////////////////////////////////////////////////

inline constexpr int AI_SOCKET_RECV_BUFF_SIZE   = (1024 * 64);
inline constexpr int AI_SOCKET_SEND_BUFF_SIZE   = (1024 * 64);

inline constexpr int GAME_SOCKET_RECV_BUFF_SIZE = (1024 * 16);
inline constexpr int GAME_SOCKET_SEND_BUFF_SIZE = (1024 * 64);

// Max individual packet size
inline constexpr int MAX_PACKET_SIZE            = (1024 * 8);
inline constexpr int REGION_BUFF_SIZE           = (1024 * 64);

inline constexpr uint8_t PACKET_START1          = 0xAA;
inline constexpr uint8_t PACKET_START2          = 0x55;
inline constexpr uint8_t PACKET_END1            = 0x55;
inline constexpr uint8_t PACKET_END2            = 0xAA;
////////////////////////////////////////////////////////////

// ==================================================================
//	About Map Object
// ==================================================================
inline constexpr int USER_BAND                  = 0;     // Map 위에 유저가 있다.
inline constexpr int NPC_BAND                   = 10000; // Map 위에 NPC(몹포함)가 있다.
inline constexpr int INVALID_BAND               = 30000; // 잘못된 ID BAND

///////////////// snow event define //////////////////////////////
inline constexpr int SNOW_EVENT_MONEY           = 2000;
inline constexpr int SNOW_EVENT_SKILL           = 490043;

//////////////////////////////////////////////////////////////////
// DEFINE Shared Memory Queue
//////////////////////////////////////////////////////////////////

// DEFINE Shared Memory Customisation
inline constexpr char SMQ_LOGGERSEND[]          = "KNIGHT_SEND";
inline constexpr char SMQ_LOGGERRECV[]          = "KNIGHT_RECV";

inline constexpr char SMQ_ITEMLOGGER[]          = "ITEMLOG_SEND";

//
//	To Who ???
//
enum e_SendTarget : uint8_t
{
	SEND_ME     = 0x01,
	SEND_REGION = 0x02,
	SEND_ALL    = 0x03,
	SEND_ZONE   = 0x04
};

// Battlezone Announcement
enum e_BattleZoneNoticeType : uint8_t
{
	BATTLEZONE_OPEN                = 0x00,
	BATTLEZONE_CLOSE               = 0x01,
	DECLARE_WINNER                 = 0x02,
	DECLARE_LOSER                  = 0x03,
	DECLARE_BAN                    = 0x04,
	KARUS_CAPTAIN_NOTIFY           = 0x05,
	ELMORAD_CAPTAIN_NOTIFY         = 0x06,
	KARUS_CAPTAIN_DEPRIVE_NOTIFY   = 0x07,
	ELMORAD_CAPTAIN_DEPRIVE_NOTIFY = 0x08,
	SNOW_BATTLEZONE_OPEN           = 0x09,
};

// Battle define
enum e_BattleType : uint8_t
{
	NO_BATTLE     = 0,
	NATION_BATTLE = 1,
	SNOW_BATTLE   = 2
};

inline constexpr int MAX_BATTLE_ZONE_USERS = 150;

//////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
typedef union
{
	uint16_t w;
	uint8_t b[2];
} MYSHORT;

typedef union
{
	int64_t i;
	uint8_t b[8];
} MYINT64;

struct _REGION_BUFFER
{
	int iLength = 0;
	char pDataBuff[REGION_BUFF_SIZE] {};
};

struct ItemPair
{
	int ItemId    = -1;
	int16_t Count = 1;
};

} // namespace Ebenezer

#endif // SERVER_EBENEZER_DEFINE_H

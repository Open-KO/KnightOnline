#pragma once

enum e_GameOpcode
{
	WIZ_LOGIN						= 0x01,	// Account Login
	WIZ_NEW_CHAR					= 0x02,	// Create Character DB
	WIZ_DEL_CHAR					= 0x03,	// Delete Character DB
	WIZ_SEL_CHAR					= 0x04,	// Select Character
	WIZ_SEL_NATION					= 0x05,	// Select Nation
	WIZ_MOVE						= 0x06,	// Move ( 1 Second )
	WIZ_USER_INOUT					= 0x07,	// User Info Insert, delete
	WIZ_ATTACK						= 0x08,	// General Attack 
	WIZ_ROTATE						= 0x09,	// Rotate
	WIZ_NPC_INOUT					= 0x0A,	// Npc Info Insert, delete
	WIZ_NPC_MOVE					= 0x0B,	// Npc Move ( 1 Second )
	WIZ_ALLCHAR_INFO_REQ			= 0x0C,	// Account All Character Info Request
	WIZ_GAMESTART					= 0x0D,	// Request Other User, Npc Info
	WIZ_MYINFO						= 0x0E,	// User Detail Data Download
	WIZ_LOGOUT						= 0x0F,	// Request Logout
	WIZ_CHAT						= 0x10,	// User Chatting..
	WIZ_DEAD						= 0x11,	// User Dead
	WIZ_REGENE						= 0x12,	// User	Regeneration
	WIZ_TIME						= 0x13,	// Game Timer
	WIZ_WEATHER						= 0x14,	// Game Weather
	WIZ_REGIONCHANGE				= 0x15,	// Region UserInfo Receive
	WIZ_REQ_USERIN					= 0x16,	// Client Request UnRegistered User List
	WIZ_HP_CHANGE					= 0x17,	// Current HP Download
	WIZ_MSP_CHANGE					= 0x18,	// Current MP Download
	WIZ_ITEM_LOG					= 0x19,	// Send To Agent for Writing Log
	WIZ_EXP_CHANGE					= 0x1A,	// Current EXP Download
	WIZ_LEVEL_CHANGE				= 0x1B,	// Max HP, MP, SP, Weight, Exp Download
	WIZ_NPC_REGION					= 0x1C,	// Npc Region Change Receive
	WIZ_REQ_NPCIN					= 0x1D,	// Client Request UnRegistered NPC List
	WIZ_WARP						= 0x1E,	// User Remote Warp
	WIZ_ITEM_MOVE					= 0x1F,	// User Item Move
	WIZ_NPC_EVENT					= 0x20,	// User Click Npc Event
	WIZ_ITEM_TRADE					= 0x21,	// Item Trade 
	WIZ_TARGET_HP					= 0x22,	// Attack Result Target HP 
	WIZ_ITEM_DROP					= 0x23,	// Zone Item Insert
	WIZ_BUNDLE_OPEN_REQ				= 0x24,	// Zone Item list Request
	WIZ_TRADE_NPC					= 0x25,	// ITEM Trade start
	WIZ_ITEM_GET					= 0x26,	// Zone Item Get
	WIZ_ZONE_CHANGE					= 0x27,	// Zone Change
	WIZ_POINT_CHANGE				= 0x28,	// Str, Sta, dex, intel, cha, point up down
	WIZ_STATE_CHANGE				= 0x29,	// User Sitdown or Stand
	WIZ_LOYALTY_CHANGE				= 0x2A,	// Nation Contribution
	WIZ_VERSION_CHECK				= 0x2B,	// Client version check 
	WIZ_CRYPTION					= 0x2C,	// Cryption
	WIZ_USERLOOK_CHANGE				= 0x2D,	// User Slot Item Resource Change
	WIZ_NOTICE						= 0x2E,	// Update Notice Alarm 
	WIZ_PARTY						= 0x2F,	// Party Related Packet
	WIZ_EXCHANGE					= 0x30,	// Exchange Related Packet
	WIZ_MAGIC_PROCESS				= 0x31,	// Magic Related Packet
	WIZ_SKILLPT_CHANGE				= 0x32,	// User changed particular skill point
	WIZ_OBJECT_EVENT				= 0x33,	// Map Object Event Occur ( ex : Bind Point Setting )
	WIZ_CLASS_CHANGE				= 0x34,	// 10 level over can change class 
	WIZ_CHAT_TARGET					= 0x35,	// Select Private Chanting User
	WIZ_CONCURRENTUSER				= 0x36,	// Current Game User Count
	WIZ_DATASAVE					= 0x37,	// User GameData DB Save Request
	WIZ_DURATION					= 0x38,	// Item Durability Change
	WIZ_TIMENOTIFY					= 0x39,	// Time Adaption Magic Time Notify Packet ( 2 Seconds )
	WIZ_REPAIR_NPC					= 0x3A,	// Item Trade, Upgrade and Repair
	WIZ_ITEM_REPAIR					= 0x3B,	// Item Repair Processing
	WIZ_KNIGHTS_PROCESS				= 0x3C,	// Knights Related Packet..
	WIZ_ITEM_COUNT_CHANGE			= 0x3D,    // Item cout change.  
	WIZ_KNIGHTS_LIST				= 0x3E,	// All Knights List Info download
	WIZ_ITEM_REMOVE					= 0x3F,	// Item Remove from inventory
	WIZ_OPERATOR					= 0x40,	// Operator Authority Packet
	WIZ_SPEEDHACK_CHECK				= 0x41,	// Speed Hack Using Check
	WIZ_COMPRESS_PACKET				= 0x42,	// Data Compressing Packet
	WIZ_SERVER_CHECK				= 0x43,	// Server Status Check Packet
	WIZ_CONTINOUS_PACKET			= 0x44,	// Region Data Packet 
	WIZ_WAREHOUSE					= 0x45,	// Warehouse Open, In, Out
	WIZ_SERVER_CHANGE				= 0x46,	// When you change the server
	WIZ_REPORT_BUG					= 0x47,	// Report Bug to the manager
	WIZ_HOME						= 0x48,    // 'Come back home' by Seo Taeji & Boys
	WIZ_FRIEND_PROCESS				= 0x49,	// Get the status of your friend
	WIZ_GOLD_CHANGE					= 0x4A,	// When you get the gold of your enemy.
	WIZ_WARP_LIST					= 0x4B,	// Warp List by NPC or Object
	WIZ_VIRTUAL_SERVER				= 0x4C,	// Battle zone Server Info packet	(IP, Port)
	WIZ_ZONE_CONCURRENT				= 0x4D,	// Battle zone concurrent users request packet
	WIZ_CORPSE						= 0x4e,	// To have your corpse have an ID on top of it.
	WIZ_PARTY_BBS					= 0x4f,	// For the party wanted bulletin board service..
	WIZ_MARKET_BBS					= 0x50,	// For the market bulletin board service...
	WIZ_KICKOUT						= 0x51,	// Account ID forbid duplicate connection
	WIZ_CLIENT_EVENT				= 0x52,	// Client Event (for quest)
	WIZ_MAP_EVENT					= 0x53,	// 클라이언트에서 무슨 에코로 쓰고 있데요.
	WIZ_WEIGHT_CHANGE				= 0x54,	// Notify change of weight
	WIZ_SELECT_MSG					= 0x55,	// Select Event Message...
	WIZ_NPC_SAY						= 0x56,	// Select Event Message...
	WIZ_BATTLE_EVENT				= 0x57,	// Battle Event Result
	WIZ_AUTHORITY_CHANGE			= 0x58,	// Authority change 
	WIZ_EDIT_BOX					= 0x59,	// Activate/Receive info from Input_Box.
	WIZ_SANTA						= 0x5A,	// Activate motherfucking Santa Claus!!! :(
	WIZ_ITEM_UPGRADE				= 0x5B,
	WIZ_PACKET1						= 0x5C,
	WIZ_PACKET2						= 0x5D,
	WIZ_ZONEABILITY					= 0x5E,	
	WIZ_EVENT						= 0x5F,
	WIZ_STEALTH						= 0x60, // stealth related.
	WIZ_ROOM_PACKETPROCESS			= 0x61, // room system
	WIZ_ROOM						= 0x62,
	WIZ_ROOM_MATCH					= 0x63,
	WIZ_QUEST						= 0x64,
	WIZ_PP_CARD						= 0x65,
	WIZ_KISS						= 0x66,
	WIZ_RECOMMEND_USER				= 0x67,
	WIZ_MERCHANT					= 0x68,
	WIZ_MERCHANT_INOUT				= 0x69,
	WIZ_SHOPPING_MALL				= 0x6A,
	WIZ_SERVER_INDEX				= 0x6B,
	WIZ_EFFECT						= 0x6C,
	WIZ_SIEGE						= 0x6D,
	WIZ_NAME_CHANGE					= 0x6E,
	WIZ_WEBPAGE						= 0x6F,
	WIZ_CAPE						= 0x70,
	WIZ_PREMIUM						= 0x71,
	WIZ_HACKTOOL					= 0x72,
	WIZ_RENTAL						= 0x73,
	WIZ_REWARD_ITEMS				= 0x74,
	WIZ_CHALLENGE					= 0x75,
	WIZ_PET							= 0x76,
	WIZ_CHINA						= 0x77, // we shouldn't need to worry about this
	WIZ_KING						= 0x78,
	WIZ_SKILLDATA					= 0x79,
	WIZ_PROGRAMCHECK				= 0x7A,
	WIZ_BIFROST						= 0x7B,
	WIZ_SERVER_KILL					= 0x7F,

	// NOTE(srmeier): testing this debug string functionality
	WIZ_DEBUG_STRING_PACKET			= 0xFE,

	WIZ_TEST_PACKET					= 0xFF	// Test packet
};

// TODO: DB opcodes.
// Currently they're somewhat shared with game opcodes but they should be defined separately.
#define WIZ_LOGIN_INFO			0x50	// define for DBAgent Communication

enum e_LoginOpcode
{
	LS_VERSION_REQ				= 0x01,
	LS_DOWNLOADINFO_REQ			= 0x02,
	LS_CRYPTION					= 0xF2,
	LS_LOGIN_REQ				= 0xF3,
	LS_MGAME_LOGIN				= 0xF4, // NOTE: We don't implement this stored procedure.
	LS_SERVERLIST				= 0xF5,
	LS_NEWS						= 0xF6,

	NUM_LS_OPCODES
};

enum e_AuthResult : uint8_t
{
	AUTH_OK				= 0x01,
	AUTH_NOT_FOUND		= 0x02,
	AUTH_INVALID_PW		= 0x03,
	AUTH_BANNED			= 0x04,
	AUTH_IN_GAME		= 0x05,
	AUTH_ERROR			= 0x06,
	AUTH_FAILED			= 0xFF
};

// NOTE: All of this is largely irrelevant.
// It cares only about finding the first and last #.
constexpr char NEWS_MESSAGE_START[]	= { '#', '\0', '\n' };
constexpr char NEWS_MESSAGE_END[]	= { '\0', '\n', '#', '\0', '\n', '\0', '\n' };
constexpr int MAX_NEWS_COUNT		= 3;

////////////////////////////////////////////////////////////////
// chat define
////////////////////////////////////////////////////////////////
enum e_ChatType
{
	GENERAL_CHAT				= 1,
	PRIVATE_CHAT				= 2,
	PARTY_CHAT					= 3,
	FORCE_CHAT					= 4,
	SHOUT_CHAT					= 5,
	KNIGHTS_CHAT				= 6,
	PUBLIC_CHAT					= 7,
	WAR_SYSTEM_CHAT				= 8,
	PERMANENT_CHAT				= 9,
	END_PERMANENT_CHAT			= 10,
	MONUMENT_NOTICE				= 11,
	GM_CHAT						= 12,
	COMMAND_CHAT				= 13,
	MERCHANT_CHAT				= 14,
	ALLIANCE_CHAT				= 15,
	ANNOUNCEMENT_CHAT			= 17,
	SEEKING_PARTY_CHAT			= 19,
};

enum e_GoldChangeOpcode
{
	GOLD_CHANGE_GAIN	= 1,
	GOLD_CHANGE_LOSE	= 2,
	GOLD_CHANGE_SPEND	= 3
};

enum e_ZoneChangeOpcode
{
	ZONE_CHANGE_LOADING		= 1,
	ZONE_CHANGE_LOADED		= 2,
	ZONE_CHANGE_TELEPORT	= 3
};

enum e_QuestOpcode
{
	QUEST_LIST		= 1,
	QUEST_UPDATE	= 2
};

////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
// weather define
////////////////////////////////////////////////////////////////
enum e_WeatherType
{
	WEATHER_FINE			= 0x01,
	WEATHER_RAIN			= 0x02,
	WEATHER_SNOW			= 0x03
};
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
// Party Related subpacket define
////////////////////////////////////////////////////////////////
enum e_PartyOpcode
{
	PARTY_CREATE			= 0x01,	// Party Group Create
	PARTY_PERMIT			= 0x02,	// Party Insert Permit
	PARTY_INSERT			= 0x03,	// Party Member Insert
	PARTY_REMOVE			= 0x04,	// Party Member Remove
	PARTY_DELETE			= 0x05,	// Party Group Delete
	PARTY_HPCHANGE			= 0x06,	// Party Member HP change
	PARTY_LEVELCHANGE		= 0x07,	// Party Member Level change
	PARTY_CLASSCHANGE		= 0x08,	// Party Member Class Change
	PARTY_STATUSCHANGE		= 0x09,	// Party Member Status ( disaster, poison ) Change
	PARTY_REGISTER			= 0x0A,	// Party Message Board Register
	PARTY_REPORT			= 0x0B,	// Party Request Message Board Messages
	PARTY_PROMOTE			= 0x1C,	// Promotes user to party leader
	PARTY_ALL_STATUSCHANGE	= 0x1D	// Sets the specified status of the selected party members to 1.
};

// NOTE: Used for trading
enum e_ExchangeOpcode
{
	EXCHANGE_REQ			= 1,
	EXCHANGE_AGREE			= 2,
	EXCHANGE_ADD			= 3,
	EXCHANGE_OTHERADD		= 4,
	EXCHANGE_DECIDE			= 5,
	EXCHANGE_OTHERDECIDE	= 6,
	EXCHANGE_DONE			= 7,
	EXCHANGE_CANCEL			= 8
};

enum e_MerchantOpcode
{
	MERCHANT_OPEN				= 1,
	MERCHANT_CLOSE				= 2,
	MERCHANT_ITEM_ADD			= 3,
	MERCHANT_ITEM_CANCEL		= 4,
	MERCHANT_ITEM_LIST			= 5,
	MERCHANT_ITEM_BUY			= 6,
	MERCHANT_INSERT				= 7,
	MERCHANT_TRADE_CANCEL		= 8,
	MERCHANT_ITEM_PURCHASED		= 9,
};

// NOTE: This needs downgrading.
enum e_NameChangeOpcode
{
	NAME_CHANGE_PLAYER_REQUEST	= 0, // contains the request with the player's name
	NAME_CHANGE_SHOW_DIALOG		= 1,
	NAME_CHANGE_INVALID_NAME	= 2,
	NAME_CHANGE_SUCCESS			= 3,
	NAME_CHANGE_IN_CLAN			= 4,
	NAME_CHANGE_KING			= 5
};

enum e_KingSystemOpcode
{
	KING_ELECTION		= 1,
	KING_IMPEACHMENT	= 2,
	KING_TAX			= 3,
	KING_EVENT			= 4,
	KING_NPC			= 5,
	KING_NATION_INTRO	= 6
};

enum e_KingEventOpcode
{
	KING_EVENT_NOAH		= 1,
	KING_EVENT_EXP		= 2,
	KING_EVENT_PRIZE	= 3,
	KING_EVENT_FUGITIVE	= 4, // not sure what this is exactly
	KING_EVENT_WEATHER	= 5,
	KING_EVENT_NOTICE	= 6
};

enum e_KingSystemElectionOpcode
{
	KING_ELECTION_SCHEDULE		= 1,
	KING_ELECTION_NOMINATE		= 2,
	KING_ELECTION_NOTICE_BOARD	= 3,
	KING_ELECTION_POLL			= 4,
	KING_ELECTION_RESIGN		= 5
}; 

enum e_KingSystemElectionDBOpcode
{
	KING_ELECTION_UPDATE_STATUS,
	KING_ELECTION_UPDATE_LIST
};

enum e_KingSystemCandidacyNoticeBoardOpcode
{
	KING_CANDIDACY_BOARD_WRITE	= 1,
	KING_CANDIDACY_BOARD_READ	= 2,
	// 4, 5
};

enum e_KingSystemImpeachmentOpcode
{
	KING_IMPEACHMENT_REQUEST			= 1,
	KING_IMPEACHMENT_REQUEST_ELECT		= 2,
	KING_IMPEACHMENT_LIST				= 3,
	KING_IMPEACHMENT_ELECT				= 4,
	KING_IMPEACHMENT_REQUEST_UI_OPEN	= 8,
	KING_IMPEACHMENT_ELECTION_UI_OPEN	= 9
};

////////////////////////////////////////////////////////////////
// Magic Packet sub define 
////////////////////////////////////////////////////////////////
enum e_MagicOpcode
{
	MAGIC_CASTING				= 1,
	MAGIC_FLYING				= 2,
	MAGIC_EFFECTING				= 3,
	MAGIC_FAIL					= 4,
	MAGIC_DURATION_EXPIRED		= 5,	// For the removal of durational (i.e. type 3/4) skills.
	MAGIC_TYPE3_END				= MAGIC_DURATION_EXPIRED,
	MAGIC_TYPE4_END				= MAGIC_DURATION_EXPIRED,
	MAGIC_CANCEL				= 6,	// When the client requests a buff to be removed.
	MAGIC_CANCEL_TRANSFORMATION	= 7,	// When removing a transformation.
	MAGIC_TYPE4_EXTEND			= 8	,	// Extends the time of your type4 buffs by 2 times (requires "Duration Item" (PUS))
	MAGIC_TRANSFORM_LIST		= 9,	// Shows the transformation list 
	MAGIC_FAIL_TRANSFORMATION	= 10,	// Transformation errors
	MAGIC_UNKNOWN				= 12,
	MAGIC_CANCEL2				= 13	// Not sure but it cancels...
};

enum e_SkillMagicFailMsg 
{
	SKILLMAGIC_FAIL_CASTING		= -100,	// "Casting failed."
	SKILLMAGIC_FAIL_KILLFLYING	= -101,
	SKILLMAGIC_FAIL_ENDCOMBO	= -102,
	SKILLMAGIC_FAIL_NOEFFECT	= -103,	// "<skill name> failed"
	SKILLMAGIC_FAIL_ATTACKZERO	= -104,	// "<skill name> missed"
	SKILLMAGIC_FAIL_UNKNOWN		= 0xffffffff
};

////////////////////////////////////////////////////////////////
// Knights Packet sub define 
////////////////////////////////////////////////////////////////
enum e_KnightsOpcode
{
	KNIGHTS_CREATE			= 0x01, // clan creation
	KNIGHTS_JOIN			= 0x02, // joining a clan
	KNIGHTS_WITHDRAW		= 0x03, // leaving a clan
	KNIGHTS_REMOVE			= 0x04,	// removing a clan member
	KNIGHTS_DESTROY			= 0x05, // disbanding a clan
	KNIGHTS_ADMIT			= 0x06,
	KNIGHTS_REJECT			= 0x07,
	KNIGHTS_PUNISH			= 0x08,
	KNIGHTS_CHIEF			= 0x09,
	KNIGHTS_VICECHIEF		= 0x0A,
	KNIGHTS_OFFICER			= 0x0B,
	KNIGHTS_ALLLIST_REQ		= 0x0C,
	KNIGHTS_MEMBER_REQ		= 0x0D,
	KNIGHTS_CURRENT_REQ		= 0x0E,
	KNIGHTS_STASH			= 0x0F,
	KNIGHTS_MODIFY_FAME		= 0x10,
	KNIGHTS_JOIN_REQ		= 0x11,
	KNIGHTS_LIST_REQ		= 0x12,

	KNIGHTS_WAR_ANSWER		= 0x14,
	KNIGHTS_WAR_SURRENDER	= 0x15,

	KNIGHTS_MARK_VERSION_REQ= 0x19,
	KNIGHTS_MARK_REGISTER	= 0x1A,
	KNIGHTS_CAPE_NPC		= 0x1B,

	KNIGHTS_ALLY_CREATE		= 0x1C,
	KNIGHTS_ALLY_REQ		= 0x1D,
	KNIGHTS_ALLY_INSERT		= 0x1E,
	KNIGHTS_ALLY_REMOVE		= 0x1F,
	KNIGHTS_ALLY_PUNISH		= 0x20,
	KNIGHTS_ALLY_LIST		= 0x22,

	KNIGHTS_MARK_REQ		= 0x23,
	KNIGHTS_UPDATE			= 0x24,
	KNIGHTS_MARK_REGION_REQ	= 0x25,

	KNIGHTS_UPDATE_GRADE	= 0x30,
};

enum e_OperatorOpcode
{
	OPERATOR_ARREST			= 1,
	OPERATOR_FORBID_CONNECT	= 2,
	OPERATOR_CHAT_FORBID	= 3,
	OPERATOR_CHAT_PERMIT	= 4,
	OPERATOR_CUTOFF			= 5,
	OPERATOR_FORBID_USER	= 6,
	OPERATOR_SUMMONUSER		= 7,
	OPERATOR_ATTACKDISABLE	= 8,
	OPERATOR_ATTACKENABLE	= 9
};

enum e_WarpListError : uint8_t
{
	WARP_LIST_ERROR_GENERIC					= 0,
	WARP_LIST_ERROR_SUCCESS					= 1,  // "You've arrived at <destination>."
	WARP_LIST_ERROR_MIN_LEVEL				= 2,  // "You need to be at least level <level>."
	WARP_LIST_ERROR_NOT_DURING_CSW			= 3,  // "You cannot enter during the Castle Siege War."
	WARP_LIST_ERROR_NOT_DURING_WAR			= 4,  // "You cannot enter during the Lunar War."
	WARP_LIST_ERROR_NEED_LOYALTY			= 5,  // "You cannot enter when you have 0 national points."
	WARP_LIST_ERROR_WRONG_LEVEL_DLW			= 6,  // "Only characters with level 30~50 can enter." (dialog) 
	WARP_LIST_ERROR_DO_NOT_QUALIFY			= 7,  // "You can not enter because you do not qualify." (dialog) 

	// Available only in newer client versions:
	WARP_LIST_ERROR_RECENTLY_TRADED			= 8,  // "You can't teleport for 2 minutes after trading." (dialog) 
	WARP_LIST_ERROR_ARENA_FULL				= 9,  // "Arena Server is full to capacity. Please try again later." (dialog) 
	WARP_LIST_ERROR_FINISHED_7_KEYS_QUEST	= 10, // "You can't enter because you completed Guardian of 7 Keys quest." (dialog)
};

////////////////////////////////////////////////////////////////
// WareHouse Packet sub define
////////////////////////////////////////////////////////////////
enum e_WarehouseOpcode
{
	WAREHOUSE_OPEN			= 0x01,
	WAREHOUSE_INPUT			= 0x02,
	WAREHOUSE_OUTPUT		= 0x03,
	WAREHOUSE_MOVE			= 0x04,
	WAREHOUSE_INVENMOVE		= 0x05,

	WAREHOUSE_REQ			= 0x10,
};

////////////////////////////////////////////////////////////////
// Class change define
////////////////////////////////////////////////////////////////
enum e_ClassChangeOpcode
{
	CLASS_CHANGE_REQ		= 0x01,
	CLASS_CHANGE_RESULT		= 0x02,
	ALL_POINT_CHANGE		= 0x03,
	ALL_SKILLPT_CHANGE		= 0x04,
	CHANGE_MONEY_REQ		= 0x05,
	NOVICE_CLASS_CHANGE_REQ = 0x06
};

////////////////////////////////////////////////////////////////
// Friend subpacket define
////////////////////////////////////////////////////////////////
enum e_FriendOpcode
{
	FRIEND_REQUEST	= 1,
	FRIEND_REPORT	= 2,
	FRIEND_ADD		= 3,
	FRIEND_REMOVE	= 4
};

enum e_FriendAddResult
{
	FRIEND_ADD_SUCCESS	= 0,
	FRIEND_ADD_ERROR	= 1,
	FRIEND_ADD_FULL		= 2,

	FRIEND_ADD_MAX
};

enum e_FriendRemoveResult
{
	FRIEND_REMOVE_SUCCESS	= 0,
	FRIEND_REMOVE_ERROR		= 1,
	FRIEND_REMOVE_NOT_FOUND	= 2,

	FRIEND_REMOVE_MAX
};

enum e_ItemUpgradeOpcode
{
	ITEM_UPGRADE_REQ			= 1,
	ITEM_UPGRADE_PROCESS		= 2,
	ITEM_UPGRADE_ACCESSORIES	= 3,
	ITEM_BIFROST_REQ			= 4,
	ITEM_BIFROST_EXCHANGE		= 5,
};

////////////////////////////////////////////////////////////////
// Party BBS subpacket define
////////////////////////////////////////////////////////////////
enum e_PartyBBSOpcode
{
	PARTY_BBS_REGISTER				= 0x01,
	PARTY_BBS_DELETE				= 0x02,
	PARTY_BBS_NEEDED				= 0x03,
	PARTY_BBS_WANTED				= 0x04,
	PARTY_BBS_LIST					= 0x0B
};

////////////////////////////////////////////////////////////////
// Market BBS primary subpacket define
////////////////////////////////////////////////////////////////
enum e_MarketBBSOpcode
{
	MARKET_BBS_REGISTER				= 0x01,
	MARKET_BBS_DELETE				= 0x02,
	MARKET_BBS_REPORT				= 0x03,
	MARKET_BBS_OPEN					= 0x04,
	MARKET_BBS_REMOTE_PURCHASE		= 0x05,
	MARKET_BBS_MESSAGE				= 0x06
};

////////////////////////////////////////////////////////////////
// Market BBS secondary subpacket define
////////////////////////////////////////////////////////////////
enum e_MarketBBSType
{
	MARKET_BBS_BUY			= 0x01,
	MARKET_BBS_SELL			= 0x02
};

////////////////////////////////////////////////////////////////
// Server to Server Communication
////////////////////////////////////////////////////////////////
#define STS_CHAT							0xD0
#define UDP_BATTLE_EVENT_PACKET				0xD1
#define UDP_KNIGHTS_PROCESS					0xD2
#define UDP_BATTLEZONE_CURRENT_USERS		0xD3
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
// Server to DB Agnent Communication
////////////////////////////////////////////////////////////////
#define DB_COUPON_EVENT			0x10	// coupon event
#define CHECK_COUPON_EVENT		0x01
#define UPDATE_COUPON_EVENT		0x02
////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////
// Authority change subpacket define
////////////////////////////////////////////////////////////////
#define COMMAND_AUTHORITY			0x01

enum e_StoreOpcode
{
	STORE_OPEN		= 1,
	STORE_CLOSE		= 2,
	STORE_BUY		= 3,
	STORE_MINI		= 4,
	STORE_PROCESS	= 5
};

enum e_RentalOpcode
{
	RENTAL_PREMIUM	= 1,
	RENTAL_PVP		= 2,
	RENTAL_NPC		= 3
};

enum e_RentalPvPOpcode
{
	RENTAL_OPEN			= 0,
	RENTAL_REGISTER		= 1,
	RENTAL_LEND			= 2,
	RENTAL_ITEM_CHECK	= 3,
	RENTAL_ITEM_CANCEL	= 4,
	RENTAL_REPORT		= 10
};

// Skillbar
enum e_SkillBarOpcode
{
	SKILL_DATA_SAVE = 1,
	SKILL_DATA_LOAD = 2
};

enum e_ZoneAbilityOpcode
{
	ZONE_ABILITY_UPDATE	= 1
};

// ---------------------------------------------------------------------
// AI Server와 게임서버간의 Npc에 관련된 패킷은 1번~49번 
// ---------------------------------------------------------------------
constexpr uint8_t	AI_SERVER_CONNECT		= 1;
constexpr uint8_t	NPC_INFO_ALL			= 2;
constexpr uint8_t	MOVE_REQ				= 3;
constexpr uint8_t	MOVE_RESULT				= 4;
constexpr uint8_t	MOVE_END_REQ			= 5;
constexpr uint8_t	MOVE_END_RESULT			= 6;
constexpr uint8_t	AG_NPC_INFO				= 7;	
constexpr uint8_t	AG_NPC_GIVE_ITEM		= 8;
constexpr uint8_t	AG_NPC_GATE_OPEN		= 9;	
constexpr uint8_t	AG_NPC_GATE_DESTORY		= 10;	
constexpr uint8_t	AG_NPC_INOUT			= 11;	
constexpr uint8_t	AG_NPC_EVENT_ITEM		= 12;	
constexpr uint8_t	AG_NPC_HP_REQ			= 13;

// ---------------------------------------------------------------------
// AI Server와 게임서버간의 User, Npc 공통 관련된 패킷은 50번~100번 
// ---------------------------------------------------------------------
constexpr uint8_t	AG_SERVER_INFO			= 50;	// 
constexpr uint8_t	AG_ATTACK_REQ			= 51;	// Attck Packet
constexpr uint8_t	AG_ATTACK_RESULT		= 52;	// Attck Packet
constexpr uint8_t	AG_DEAD					= 53;	// Dead Packet
constexpr uint8_t	AG_SYSTEM_MSG			= 54;	// System message Packet
constexpr uint8_t	AG_CHECK_ALIVE_REQ		= 55;	// Server alive check
constexpr uint8_t	AG_COMPRESSED_DATA		= 56;	// Packet Data compressed
constexpr uint8_t	AG_ZONE_CHANGE			= 57;	// Zone change
constexpr uint8_t	AG_MAGIC_ATTACK_REQ		= 58;	// Magic Attck Packet
constexpr uint8_t	AG_MAGIC_ATTACK_RESULT	= 59;	// Magic Attck Packet
constexpr uint8_t	AG_USER_INFO_ALL		= 60;	// User의 모든 정보 전송
constexpr uint8_t	AG_LONG_MAGIC_ATTACK	= 61;	// Magic Attck Packet
constexpr uint8_t	AG_PARTY_INFO_ALL		= 62;	// Party의 모든 정보 전송
constexpr uint8_t	AG_HEAL_MAGIC			= 63;	// Healing magic
constexpr uint8_t	AG_TIME_WEATHER			= 64;	// time and whether info
constexpr uint8_t	AG_BATTLE_EVENT			= 65;	// battle event
constexpr uint8_t	AG_COMPRESSED			= 66;

// ---------------------------------------------------------------------
// Battle Event Sub Packet
// ---------------------------------------------------------------------
constexpr uint8_t	BATTLE_EVENT_OPEN		= 1;	// battle event open
constexpr uint8_t	BATTLE_MAP_EVENT_RESULT = 2;	// battle zone map event result
constexpr uint8_t	BATTLE_EVENT_RESULT		= 3;	// battle event result ( victory nation )
constexpr uint8_t	BATTLE_EVENT_MAX_USER	= 4;	// battle event result ( user name )
constexpr uint8_t	BATTLE_EVENT_KILL_USER	= 5;	// battle event result ( user kill count )

// ---------------------------------------------------------------------
// AI Server와 게임서버간의 User에 관련된 패킷은 101번 부터 시작
// ---------------------------------------------------------------------
constexpr uint8_t	AG_USER_INFO		= 101;	// User의 정보
constexpr uint8_t	AG_USER_INOUT		= 102;	// User의 In,Out 정보
constexpr uint8_t	AG_USER_MOVE		= 103;	// User의 move 정보
constexpr uint8_t	AG_USER_MOVEEDGE	= 104;	// User의 move end 정보
constexpr uint8_t	AG_USER_SET_HP		= 105;	// User의 HP
constexpr uint8_t	AG_USER_LOG_OUT		= 106;	// User의 LogOut
constexpr uint8_t	AG_USER_REGENE		= 107;	// User의 Regene
constexpr uint8_t	AG_USER_EXP			= 108;	// User의 경험치
constexpr uint8_t	AG_USER_UPDATE		= 109;	// User의 Update Info
constexpr uint8_t	AG_USER_FAIL		= 110;	// 잘못된 유저 처리...
constexpr uint8_t	AG_USER_PARTY		= 111;	// 파티처리 담당
constexpr uint8_t	AG_USER_VISIBILITY  = 112;	// updates invisibility status
constexpr uint8_t	AG_NPC_HP_CHANGE	= 113;	// updates an NPC's HP

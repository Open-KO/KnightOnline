#ifndef SERVER_AUJARD_DEFINE_H
#define SERVER_AUJARD_DEFINE_H

#if defined(_DEBUG)
#include <iostream>
#endif

#include <shared/globals.h>
#include <shared-server/_USER_DATA.h>

#include <Aujard/model/AujardModel.h>

#include <string_view>

namespace Aujard
{

constexpr int MAX_USER            = 3000;
constexpr long DB_PROCESS_TIMEOUT = 10;

////////////////////////////////////////////////////////////
// DEFINE Shared Memory Costumizing

constexpr char SMQ_LOGGERSEND[]   = "KNIGHT_SEND";
constexpr char SMQ_LOGGERRECV[]   = "KNIGHT_RECV";

////////////////////////////////////////////////////////////////
// Update User Data type define
////////////////////////////////////////////////////////////////
enum e_UserUpdateType : uint8_t
{
	UPDATE_LOGOUT      = 1,
	UPDATE_ALL_SAVE    = 2,
	UPDATE_PACKET_SAVE = 3
};

////////////////////////////////////////////////////////////////
// WIZ_NEW_CHAR Results
////////////////////////////////////////////////////////////////
enum e_NewCharResult : int8_t
{
	NEW_CHAR_ERROR        = -1,
	NEW_CHAR_SUCCESS      = 0,
	NEW_CHAR_NO_FREE_SLOT = 1,
	NEW_CHAR_INVALID_RACE = 2,
	NEW_CHAR_NAME_IN_USE  = 3,
	NEW_CHAR_SYNC_ERROR   = 4
};

namespace model = aujard_model;

namespace ini
{
// ODBC Config Section
static constexpr std::string_view ODBC        = "ODBC";
static constexpr std::string_view GAME_DSN    = "GAME_DSN";
static constexpr std::string_view GAME_UID    = "GAME_UID";
static constexpr std::string_view GAME_PWD    = "GAME_PWD";
static constexpr std::string_view ACCOUNT_DSN = "ACCOUNT_DSN";
static constexpr std::string_view ACCOUNT_UID = "ACCOUNT_UID";
static constexpr std::string_view ACCOUNT_PWD = "ACCOUNT_PWD";

// ZONE_INFO section
static constexpr std::string_view ZONE_INFO   = "ZONE_INFO";
static constexpr std::string_view GROUP_INFO  = "GROUP_INFO";
} // namespace ini

} // namespace Aujard

#endif // SERVER_AUJARD_DEFINE_H

#ifndef _DEFINE_H
#define _DEFINE_H

#if defined(_DEBUG)
#include <iostream>
#endif

#include <shared/globals.h>
#include <shared-server/_USER_DATA.h>

constexpr int MAX_USER			= 3000;

constexpr long DB_PROCESS_TIMEOUT = 10;

#define MAX_ITEM			28

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

// DEFINE Shared Memory Costumizing

#define SMQ_LOGGERSEND	"KNIGHT_SEND"
#define SMQ_LOGGERRECV	"KNIGHT_RECV"

// Packet Define...
#define WIZ_LOGIN				0x01	// Account Login
#define WIZ_NEW_CHAR			0x02	// Create Character DB
#define WIZ_DEL_CHAR			0x03	// Delete Character DB
#define WIZ_SEL_CHAR			0x04	// Select Character
#define WIZ_SEL_NATION			0x05	// Select Nation
#define WIZ_ALLCHAR_INFO_REQ	0x0C	// Account All Character Info Request
#define WIZ_LOGOUT				0x0F	// Request Logout
#define WIZ_DATASAVE			0x37	// User GameData DB Save Request
#define WIZ_KNIGHTS_PROCESS		0x3C	// Knights Related Packet..
#define WIZ_CLAN_PROCESS		0x4E	// Clans Related Packet..
#define WIZ_LOGIN_INFO			0x50	// define for DBAgent Communication
#define WIZ_KICKOUT				0x51	// Account ID forbid duplicate connection
#define WIZ_BATTLE_EVENT		0x57	// Battle Event Result

////////////////////////////////////////////////////////////////
// Knights Packet sub define 
////////////////////////////////////////////////////////////////
#define KNIGHTS_CREATE			0x11		// 생성
#define KNIGHTS_JOIN			0x12		// 가입
#define KNIGHTS_WITHDRAW		0x13		// 탈퇴
#define KNIGHTS_REMOVE			0x14		// 멤버 삭제
#define KNIGHTS_DESTROY			0x15		// 뽀개기
#define KNIGHTS_ADMIT			0x16		// 멤버 가입 허가
#define KNIGHTS_REJECT			0x17		// 멤버 가입 거절
#define KNIGHTS_PUNISH			0x18		// 멤버 징계
#define KNIGHTS_CHIEF			0x19		// 단장 임명
#define KNIGHTS_VICECHIEF		0x1A		// 부단장 임명
#define KNIGHTS_OFFICER			0x1B		// 장교임명
#define KNIGHTS_ALLLIST_REQ		0x1C		// 리스트를 10개 단위로 Page 요청
#define KNIGHTS_MEMBER_REQ		0x1D		// 모든 멤버 요청
#define KNIGHTS_CURRENT_REQ		0x1E		// 현재 접속 리스트
#define KNIGHTS_STASH			0x1F		// 기사단 창고
#define KNIGHTS_MODIFY_FAME		0x20		// 멤버의 직위 변경.. 해당 멤버에게 간다
#define KNIGHTS_JOIN_REQ		0x21		// 해당멤버에게 가입요청을 한다
#define KNIGHTS_LIST_REQ		0x22		// 기사단 리스트를  요청 ( index 검색 )

////////////////////////////////////////////////////////////////
// Clan Packet sub define
////////////////////////////////////////////////////////////////
#define CLAN_CREATE				0x01
#define CLAN_JOIN				0x02

////////////////////////////////////////////////////////////////
// Update User Data type define
////////////////////////////////////////////////////////////////
#define UPDATE_LOGOUT			0x01
#define UPDATE_ALL_SAVE			0x02
#define UPDATE_PACKET_SAVE		0x03

////////////////////////////////////////////////////////////////
// WIZ_NEW_CHAR Results
////////////////////////////////////////////////////////////////
#define NEW_CHAR_ERROR			-1
#define NEW_CHAR_SUCCESS		0
#define NEW_CHAR_NO_FREE_SLOT	1
#define NEW_CHAR_INVALID_RACE	2
#define NEW_CHAR_NAME_IN_USE	3
#define NEW_CHAR_SYNC_ERROR		4


// Reply packet define...

#define SEND_ME					0x01
#define SEND_REGION				0x02
#define SEND_ALL				0x03

#define ITEMCOUNT_MAX		9999	// 소모 아이템 소유 한계값

/////////////////////////////////////////////////////////////////////////////////
// Structure Define
/////////////////////////////////////////////////////////////////////////////////

#include <Aujard/model/AujardModel.h>
namespace model = aujard_model;

namespace ini
{
	// ODBC Config Section
	static constexpr char ODBC[] = "ODBC";
	static constexpr char GAME_DSN[] = "GAME_DSN";
	static constexpr char GAME_UID[] = "GAME_UID";
	static constexpr char GAME_PWD[] = "GAME_PWD";
	static constexpr char ACCOUNT_DSN[] = "ACCOUNT_DSN";
	static constexpr char ACCOUNT_UID[] = "ACCOUNT_UID";
	static constexpr char ACCOUNT_PWD[] = "ACCOUNT_PWD";

	// ZONE_INFO section
	static constexpr char ZONE_INFO[] = "ZONE_INFO";
	static constexpr char GROUP_INFO[] = "GROUP_INFO";
}

#endif

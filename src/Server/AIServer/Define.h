#ifndef SERVER_AISERVER_DEFINE_H
#define SERVER_AISERVER_DEFINE_H

#pragma once

#include <shared/globals.h>
#include <MathUtils/GeometricStructs.h>

namespace AIServer
{

constexpr int USER_DAMAGE_OVERRIDE_GM         = 30'000;
constexpr int USER_DAMAGE_OVERRIDE_LIMITED_GM = 0;
constexpr int USER_DAMAGE_OVERRIDE_TEST_MODE  = 10'000;

//
//	Defines About Communication
//
constexpr int AI_KARUS_SOCKET_PORT            = 10020;
constexpr int AI_ELMO_SOCKET_PORT             = 10030;
constexpr int AI_BATTLE_SOCKET_PORT           = 10040;
constexpr int USER_SOCKET_PORT                = 10000;
constexpr int MAX_USER                        = 3000;
constexpr int MAX_SOCKET                      = 100;
constexpr int MAX_AI_SOCKET                   = 10; // sungyong 2002.05.22
constexpr int CLIENT_SOCKSIZE                 = 10;
constexpr int MAX_PATH_LINE                   = 100;

constexpr int NOW_TEST_MODE                   = 0;

constexpr int MAX_WEAPON_NAME_SIZE            = 40;
constexpr int MAX_ITEM                        = 28;
constexpr int VIEW_DIST                       = 48; // 가시거리

///////////////// NATION ///////////////////////////////////
//
constexpr int UNIFY_ZONE                      = 0;
constexpr int KARUS_ZONE                      = 1;
constexpr int ELMORAD_ZONE                    = 2;
constexpr int BATTLE_ZONE                     = 3;

// Npc InOut
constexpr int NPC_IN                          = 1;
constexpr int NPC_OUT                         = 2;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////
constexpr int TILE_SIZE                       = 4;
constexpr int CELL_SIZE                       = 4;

////////////////////////////////////////////////////////////
// Socket Define
////////////////////////////////////////////////////////////
constexpr int SOCKET_BUFF_SIZE                = (1024 * 32);
constexpr int MAX_PACKET_SIZE                 = (1024 * 8);

constexpr uint8_t PACKET_START1               = 0xAA;
constexpr uint8_t PACKET_START2               = 0x55;
constexpr uint8_t PACKET_END1                 = 0x55;
constexpr uint8_t PACKET_END2                 = 0xAA;
////////////////////////////////////////////////////////////

typedef union
{
	int16_t i;
	uint8_t b[2];
} MYSHORT;

struct _NpcPosition
{
	uint8_t byType;  // type
	uint8_t bySpeed; // speed
	_POINT pPoint;   // position
	float fXPos;
	float fZPos;
};

struct _NpcMovePosition
{
	bool bX; // x (true +, flase -)
	bool bZ; // z (true +, flase -)
	float fMovePos;
	float fAddPos;
	float fAdd_ZPos;
};

struct _OBJECT_EVENT
{
	// 소속
	int sBelong;

	// 100 번대 - 카루스 바인드 포인트 | 200 번대 엘모라드 바인드 포인트 | 1100 번대 - 카루스 성문들 1200 - 엘모라드 성문들
	int16_t sIndex;

	// 0 - 바인드 포인트.. 1 - 좌우로 열리는 성문 2 - 상하로 열리는 성문 3 - 레버
	int16_t sType;

	// 조종할 NPC ID (조종할 Object Index)
	int16_t sControlNpcID;

	// status
	int16_t sStatus;

	// 위치값
	float fPosX;
	float fPosY;
	float fPosZ;
};

//
//	About USER
//
enum e_UserState : uint8_t
{
	USER_DEAD = 0,
	USER_LIVE = 1
};

//
//	About USER Log define
//
enum e_UserLogType : uint8_t
{
	USER_LOGIN    = 1,
	USER_LOGOUT   = 2,
	USER_LEVEL_UP = 3
};

//
//	About NPC
//
constexpr int NPC_NUM                  = 20;
constexpr int MAX_DUNGEON_BOSS_MONSTER = 20;
constexpr int NPC_MAX_MOVE_RANGE       = 100;

//
//	About Map Object
//
constexpr int USER_BAND                = 0;     // Map 위에 유저가 있다.
constexpr int NPC_BAND                 = 10000; // Map 위에 NPC(몹포함)가 있다.
constexpr int INVALID_BAND             = 20000; // 잘못된 ID BAND

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

// 타격비별 성공률 //
enum e_SuccessType : uint8_t
{
	GREAT_SUCCESS = 0x01, // 대성공
	SUCCESS       = 0x02, // 성공
	NORMAL        = 0x03, // 보통
	FAIL          = 0x04  // 실패
};

// 각 보고있는 방향을 정의한다.
enum e_Direction : uint8_t
{
	DIR_DOWN      = 0,
	DIR_DOWNLEFT  = 1,
	DIR_LEFT      = 2,
	DIR_UPLEFT    = 3,
	DIR_UP        = 4,
	DIR_UPRIGHT   = 5,
	DIR_RIGHT     = 6,
	DIR_DOWNRIGHT = 7
};

enum e_Moral : uint8_t
{
	MORAL_SELF            = 1,  // 나 자신..
	MORAL_FRIEND_WITHME   = 2,  // 나를 포함한 우리편(국가) 중 하나 ..
	MORAL_FRIEND_EXCEPTME = 3,  // 나를 뺀 우리편 중 하나
	MORAL_PARTY           = 4,  // 나를 포함한 우리파티 중 하나..
	MORAL_NPC             = 5,  // NPC중 하나.
	MORAL_PARTY_ALL       = 6,  // 나를 호함한 파티 모두..
	MORAL_ENEMY           = 7,  // 울편을 제외한 모든 적중 하나(NPC포함)
	MORAL_ALL             = 8,  // 겜상에 존재하는 모든 것중 하나.
	MORAL_AREA_ENEMY      = 10, // 지역에 포함된 적
	MORAL_AREA_FRIEND     = 11, // 지역에 포함된 우리편
	MORAL_AREA_ALL        = 12, // 지역에 포함된 모두
	MORAL_SELF_AREA       = 13  // 나를 중심으로 한 지역
};

// Attack Type
enum e_AttackType : uint8_t
{
	DIRECT_ATTACK   = 0,
	LONG_ATTACK     = 1,
	MAGIC_ATTACK    = 2,
	DURATION_ATTACK = 3
};

enum e_GateStatus : uint8_t
{
	GATE_CLOSE = 0,
	GATE_OPEN  = 1
};

enum e_SpecialObjectType : uint8_t
{
	NORMAL_OBJECT  = 0,
	SPECIAL_OBJECT = 1
};

// Battlezone Announcement
enum e_BattleZoneNoticeType : uint8_t
{
	BATTLEZONE_OPEN  = 0x00,
	BATTLEZONE_CLOSE = 0x01,
	DECLARE_WINNER   = 0x02,
	DECLARE_BAN      = 0x03
};

////////////////////////////////////////////////////////////////
// magic define
////////////////////////////////////////////////////////////////
enum e_ResistanceType : uint8_t
{
	NONE_R      = 0,
	FIRE_R      = 1,
	COLD_R      = 2,
	LIGHTNING_R = 3,
	MAGIC_R     = 4,
	DISEASE_R   = 5,
	POISON_R    = 6,
	LIGHT_R     = 7,
	DARKNESS_R  = 8
};

////////////////////////////////////////////////////////////////
// Type 3 Attribute define
////////////////////////////////////////////////////////////////
enum e_AttributeType : uint8_t
{
	ATTRIBUTE_FIRE      = 1,
	ATTRIBUTE_ICE       = 2,
	ATTRIBUTE_LIGHTNING = 3
};

} // namespace AIServer

#endif // SERVER_AISERVER_DEFINE_H

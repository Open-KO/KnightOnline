#ifndef _DEFINE_H
#define _DEFINE_H

#include <shared/globals.h>

/*
	 ** Repent AI Server 작업시 참고 사항 **
	1. #define AI_SOCKET_PORT	10020 -> 11020으로 수정됨..
*/

constexpr int USER_DAMAGE_OVERRIDE_GM			= 30'000;
constexpr int USER_DAMAGE_OVERRIDE_LIMITED_GM	= 0;
constexpr int USER_DAMAGE_OVERRIDE_TEST_MODE	= 10'000;

//
//	MAX VALUE DEFINE
//
#define _MAX_DWORD				0xFFFFFFFF
#define _MAX_INT				0x7FFFFFFF
#define _MAX_SHORT				0x7FFF

//
//	Defines About Communication
//
#define AI_KARUS_SOCKET_PORT		10020
#define AI_ELMO_SOCKET_PORT			10030
#define AI_BATTLE_SOCKET_PORT		10040
#define USER_SOCKET_PORT			10000
#define MAX_USER					3000
#define MAX_SOCKET					100
#define MAX_AI_SOCKET				10				// sungyong 2002.05.22
#define CLIENT_SOCKSIZE		10
#define MAX_PATH_LINE		100

#define NOW_TEST_MODE		0

#define MAX_WEAPON_NAME_SIZE	40
#define MAX_ITEM			28
#define VIEW_DIST			48		// 가시거리

///////////////// NATION ///////////////////////////////////
//
#define UNIFY_ZONE			0
#define KARUS_ZONE			1
#define ELMORAD_ZONE		2
#define BATTLE_ZONE			3

//enum MOVE_SPEED {SPEED_SLOW=0, SPEED_NORMAL, SPEED_FAST};
//enum USER_TYPE {TYPE_USER=0, TYPE_MONSTER, TYPE_NPC, TYPE_DOOR, TYPE_GUARD};

#define TIME_SLOW	500
#define TIME_NORMAL	350

//
//	User Authority
//
#define MANAGER_USER	0	// 운영자, 관리자
#define GENERAL_USER	1	// 일반유저

// Npc InOut
#define NPC_IN					0X01
#define NPC_OUT					0X02


////////////////////////////////////////////////////////////
// 
////////////////////////////////////////////////////////////
#define TILE_SIZE		4
#define CELL_SIZE		4

#define SKILL_NUM			5


////////////////////////////////////////////////////////////
// Socket Define
////////////////////////////////////////////////////////////
#define SOCKET_BUFF_SIZE	(1024*32)
#define MAX_PACKET_SIZE		(1024*8)

#define COMPARE(x,min,max) ((x>=min)&&(x<max))

#define PACKET_START1				0XAA
#define PACKET_START2				0X55
#define PACKET_END1					0X55
#define PACKET_END2					0XAA
//#define PROTOCOL_VER				0X01

// Socket type
#define TYPE_ACCEPT				0x01
#define TYPE_CONNECT			0x02

// Overlapped flag
#define OVL_RECEIVE				0X01
#define OVL_SEND				0X02
#define OVL_CLOSE				0X03
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

struct _NpcPosition
{
	uint8_t	byType;			// type
	uint8_t	bySpeed;		// speed
	POINT	pPoint;			// position
	float fXPos;
	float fZPos;
};

struct _NpcMovePosition
{
	bool bX;				// x (true +, flase -)
	bool bZ;				// z (true +, flase -)
	float fMovePos;
	float fAddPos;
	float fAdd_ZPos;
};

struct _OBJECT_EVENT
{
	int sBelong;			// 소속
	int16_t sIndex;			// 100 번대 - 카루스 바인드 포인트 | 200 번대 엘모라드 바인드 포인트 | 1100 번대 - 카루스 성문들 1200 - 엘모라드 성문들
	int16_t sType;			// 0 - 바인드 포인트.. 1 - 좌우로 열리는 성문 2 - 상하로 열리는 성문 3 - 레버
	int16_t sControlNpcID;	// 조종할 NPC ID (조종할 Object Index)
	int16_t sStatus;			// status
	float fPosX;			// 위치값
	float fPosY;
	float fPosZ;
};

// DEFINE MACRO PART...
#define BufInc(x) (x)++;(x) %= SOCKET_BUF_SIZE;

//
//	Define CriticalSection Spin Count
//
#define SPIN_COUNT				4000

//
//	About USER
//
#define USER_DEAD				0X00
#define USER_LIVE				0X01

//
//	About USER Log define 
//
#define USER_LOGIN				0X01
#define USER_LOGOUT				0X02
#define USER_LEVEL_UP			0X03


//
//	About NPC
//
#define NPC_NUM					20
#define MAX_DUNGEON_BOSS_MONSTER	20

#define NPC_DEAD				0X00
#define NPC_LIVE				0X01
#define NPC_ATTACKING			0X02
#define NPC_ATTACKED			0X03
#define NPC_ESCAPE				0X04
#define NPC_STANDING			0X05
#define NPC_MOVING				0X06
#define NPC_TRACING				0X07
#define NPC_FIGHTING			0X08
#define NPC_STRATEGY			0x09
#define NPC_BACK				0x0A
#define NPC_SLEEPING			0x0B
#define NPC_FAINTING			0x0C
#define NPC_HEALING				0x0D

#define NPC_PASSIVE				150
#define NPC_MAX_MOVE_RANGE		100

//
//	About Map Object
//
#define USER_BAND				0			// Map 위에 유저가 있다.
#define NPC_BAND				10000		// Map 위에 NPC(몹포함)가 있다.
#define INVALID_BAND			20000		// 잘못된 ID BAND

//
//	Defines About Max Value
//
#define MAX_BASIC_ITEM			1000
#define MAX_EVENT				1000

//
//	To Who ???
//
#define SEND_ME					0x01
#define SEND_REGION				0x02
#define SEND_ALL				0x03
#define SEND_ZONE				0x04


//
//	State Value
//
#define STATE_ACCEPTED			0X01
#define STATE_CONNECTED			0X02
#define STATE_DISCONNECTED		0X03
#define STATE_GAMESTARTED		0X04
#define STATE_INITED			0X05
#define STATE_LOGOUT			0X06
#define STATE_GAMERESTART		0X07


//
//  Item
//
//#define MAX_ITEM_IN_USER		45		// Body(10) + Inventory(35)
//#define MAX_ITEM_IN_QUICK		4
//#define ITEM_DATA_LENGTH		80
#define TYPE_MONEY				0
#define TYPE_ITEM				1

#define ITEM_NAME_LENGTH		20
#define MAX_THROW_ITEM			30000
#define ITEM_MAX_USE_WEAR		13		// 아이템중 쓸수있는 속성들만 모아넣기위해, 이벤트 아이템과 구분

////////////////////////////////////////////////////////////
// Durability Type
#define ATTACK				0x01
#define DEFENCE				0x02
////////////////////////////////////////////////////////////

//
//	Attack
//
#define		DEFAULT_AT_DELAY	700
#define		DEFAULT_SP_DEC		1

const uint8_t	ATTACK_MIN		=	20;
const uint8_t	ATTACK_MAX		=	80;

#define		DAMAGE_DELAY_C_TIME	2000

// 타격비별 성공률 //
#define GREAT_SUCCESS			0X01		// 대성공
#define SUCCESS					0X02		// 성공
#define NORMAL					0X03		// 보통
#define	FAIL					0X04		// 실패

//
//	User Status Value
//
#define USER_ABILITY_NUM		5		// 기본 상태 능력치 종류

#define USER_STR				0
#define USER_CON				1
#define USER_DEX				2
#define USER_VOL				3
#define USER_WIS				4

#define DIR_DOWN			0			// 각 보고있는 방향을 정의한다.
#define	DIR_DOWNLEFT		1
#define DIR_LEFT			2
#define	DIR_UPLEFT			3
#define DIR_UP				4
#define DIR_UPRIGHT			5
#define DIR_RIGHT			6
#define	DIR_DOWNRIGHT		7

#define NPC_EVENT_MOP		1000		// 이벤트 몹 번호
#define NPC_MAGIC_ITEM		100			// 1~10000번을 기준
#define NPC_RARE_ITEM		120			// 
#define NPC_EVENT_CHANCE	20			// 이벤트 몹일경우 매직확률과 레어 확률울 올려준다. X 20 

////////////////////////////////////////////////////////////
// Npc Type
// Monster는 0부터 시작 10까지의 타입
#define NPCTYPE_MONSTER				0	// monster
#define NPC_BOSS_MONSTER			3	// 대장 몬스터
#define NPC_DUNGEON_MONSTER			4	// 던젼 몬스터
#define NPC_TRAP_MONSTER			5	// 함정 몬스터
// NPC는 11부터 시작
#define NPC_GUARD					11	// 붙박이형 경비병
#define NPC_PATROL_GUARD			12	// 일반 필드에서 정찰을 담당하는 정찰병
#define NPC_STORE_GUARD				13	// 일반 필드에서 상점주변을 보호하는 경비병
#define NPC_MERCHANT				21	// 상점주인 NPC 
#define NPC_TINKER					22	// 대장장이
#define NPC_WAREHOUSE				23	// 창고지기
#define NPC_CAPTAIN_NPC				35	// 전직 시켜주는 NPC
#define NPC_KNIGHTS_NPC				36	// 기사단 관리 NPC
#define NPC_CLERIC					37	// 대사제 NPC
#define NPC_HEALER					40	// Healer
#define NPC_DOOR					50	// 성문 (6->50)
#define NPC_PHOENIX_GATE			51	// 깨지지 않는 문 (8->51)
#define NPC_SPECIAL_GATE			52	// 깨지지 않는 문이면서 2분마다 열렸다 닫혔다 하는 문
#define NPC_GATE_LEVER				55	// 성문 레버...	(9->55)	
#define NPC_ARTIFACT				60	// 결계석 (7->60)
#define NPC_DESTORY_ARTIFACT		61	// 파괴되는 결계석
#define NPC_DOMESTIC_ANIMAL			99	// 가축 NPC
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Magic State
#define NONE				0x01
#define CASTING				0x02
////////////////////////////////////////////////////////////

#define MORAL_SELF				1		// 나 자신..
#define MORAL_FRIEND_WITHME		2		// 나를 포함한 우리편(국가) 중 하나 ..
#define MORAL_FRIEND_EXCEPTME	3		// 나를 뺀 우리편 중 하나 
#define MORAL_PARTY				4		// 나를 포함한 우리파티 중 하나..
#define MORAL_NPC				5		// NPC중 하나.
#define MORAL_PARTY_ALL			6		// 나를 호함한 파티 모두..
#define MORAL_ENEMY				7		// 울편을 제외한 모든 적중 하나(NPC포함)
#define MORAL_ALL				8		// 겜상에 존재하는 모든 것중 하나.
#define MORAL_AREA_ENEMY		10		// 지역에 포함된 적
#define MORAL_AREA_FRIEND		11		// 지역에 포함된 우리편
#define MORAL_AREA_ALL			12		// 지역에 포함된 모두
#define MORAL_SELF_AREA			13		// 나를 중심으로 한 지역

////////////////////////////////////////////////////////////////
// Magic Packet sub define 
////////////////////////////////////////////////////////////////
#define MAGIC_CASTING			0x01
#define MAGIC_FLYING			0x02
#define MAGIC_EFFECTING			0x03
#define MAGIC_FAIL				0x04
#define MAGIC_TYPE4_END			0x05	// For type 4 durational spells.					

// Attack Type
#define DIRECT_ATTACK		0
#define LONG_ATTACK			1
#define MAGIC_ATTACK		2
#define DURATION_ATTACK		3

#define GATE_OPEN		1
#define GATE_CLOSE		0

#define NORMAL_OBJECT		0
#define SPECIAL_OBJECT		1

// Battlezone Announcement
#define BATTLEZONE_OPEN         0x00
#define BATTLEZONE_CLOSE        0x01           
#define DECLARE_WINNER          0x02
#define DECLARE_BAN				0x03

////////////////////////////////////////////////////////////////
// magic define
////////////////////////////////////////////////////////////////
#define MORAL_GOOD		0x01
#define MORAL_BAD		0x02
#define MORAL_NEUTRAL	0x03

#define NONE_R				0
#define	FIRE_R				1
#define	COLD_R				2
#define LIGHTNING_R			3
#define MAGIC_R				4
#define DISEASE_R			5
#define POISON_R			6
#define LIGHT_R				7
#define DARKNESS_R			8

////////////////////////////////////////////////////////////////
// Type 3 Attribute define
////////////////////////////////////////////////////////////////
#define ATTRIBUTE_FIRE			 1
#define ATTRIBUTE_ICE			 2
#define ATTRIBUTE_LIGHTNING		 3

#endif

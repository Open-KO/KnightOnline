#ifndef _PACKET_H_
#define _PACKET_H_

const BYTE INFO_MODIFY			=	1;
const BYTE INFO_DELETE			= 	2;

// 상태이상 정보
#define _ABNORMAL_DB			5

const BYTE ABNORMAL_NONE		=	0;
const BYTE ABNORMAL_POISON		=	1;
const BYTE ABNORMAL_CONFUSION	=	2;
const BYTE ABNORMAL_PARALYSIS	=	3;
const BYTE ABNORMAL_BLIND		=	4;
const BYTE ABNORMAL_LIGHT		=	5;
const BYTE ABNORMAL_FIRE		=	6;
const BYTE ABNORMAL_COLD		=	7;
const BYTE ABNORMAL_HASTE		=	8;
const BYTE ABNORMAL_SHIELD		=	9;
const BYTE ABNORMAL_INFRAVISION =	10;

const int TYPE_MONEY_SID		=	900000000;	// 아이템 과 돈을 구분하기위해 sid를 크게 잡았다.

#define SERVER_INFO_START			0X01
#define SERVER_INFO_END				0X02

// ---------------------------------------------------------------------
// AI Server와 게임서버간의 Npc에 관련된 패킷은 1번~49번 
// ---------------------------------------------------------------------

// A : AIServer , G : GameServer
const BYTE SERVER_CONNECT		= 1;
const BYTE	NPC_INFO_ALL		= 2;
const BYTE	MOVE_REQ			= 3;
const BYTE	MOVE_RESULT			= 4;
const BYTE	MOVE_END_REQ		= 5;
const BYTE	MOVE_END_RESULT		= 6;
const BYTE	AG_NPC_INFO			= 7;	
const BYTE	AG_NPC_GIVE_ITEM	= 8;	
const BYTE	AG_NPC_GATE_OPEN	= 9;	
const BYTE	AG_NPC_GATE_DESTORY	= 10;	
const BYTE	AG_NPC_INOUT		= 11;	
const BYTE	AG_NPC_EVENT_ITEM	= 12;	

// ---------------------------------------------------------------------
// AI Server와 게임서버간의 User, Npc 공통 관련된 패킷은 50번~100번 
// ---------------------------------------------------------------------
const BYTE	AG_SERVER_INFO			= 50;	// 
const BYTE	AG_ATTACK_REQ			= 51;	// Attck Packet
const BYTE	AG_ATTACK_RESULT		= 52;	// Attck Packet
const BYTE	AG_DEAD					= 53;	// Dead Packet
const BYTE	AG_SYSTEM_MSG			= 54;	// System message Packet
const BYTE	AG_CHECK_ALIVE_REQ		= 55;	// Server alive check
const BYTE	AG_COMPRESSED_DATA		= 56;	// Packet Data compressed
const BYTE	AG_ZONE_CHANGE			= 57;	// Zone change
const BYTE	AG_MAGIC_ATTACK_REQ		= 58;	// Magic Attck Packet (User->Monster
const BYTE	AG_MAGIC_ATTACK_RESULT	= 59;	// Magic Attck Packet
const BYTE	AG_USER_INFO_ALL		= 60;	// User의 모든 정보 전송
const BYTE	AG_LONG_MAGIC_ATTACK	= 61;	// Magic Attck Packet
const BYTE	AG_PARTY_INFO_ALL		= 62;	// Party의 모든 정보 전송
const BYTE	AG_HEAL_MAGIC			= 63;	// Healing magic
const BYTE	AG_TIME_WEATHER			= 64;	// time and whether info
const BYTE	AG_BATTLE_EVENT			= 65;	// battle event
		// ---------------------------------------------------------------------
		// Battle Event Sub Packet
		// ---------------------------------------------------------------------
		const BYTE	BATTLE_EVENT_OPEN		= 1;	// battle event open
		const BYTE	BATTLE_MAP_EVENT_RESULT = 2;	// battle zone map event result
		const BYTE	BATTLE_EVENT_RESULT		= 3;	// battle event result ( victory nation )
		const BYTE	BATTLE_EVENT_MAX_USER	= 4;	// battle event result ( user name )
	

// ---------------------------------------------------------------------
// AI Server와 게임서버간의 User에 관련된 패킷은 101번 부터 시작
// ---------------------------------------------------------------------

const BYTE	AG_USER_INFO		= 101;	// User의 정보
const BYTE	AG_USER_INOUT		= 102;	// User의 In,Out 정보
const BYTE	AG_USER_MOVE		= 103;	// User의 move
const BYTE	AG_USER_MOVEEDGE	= 104;	// User의 move end
const BYTE	AG_USER_SET_HP		= 105;	// User의 HP
const BYTE	AG_USER_LOG_OUT		= 106;	// User의 LogOut
const BYTE	AG_USER_REGENE		= 107;	// User의 Regene
const BYTE	AG_USER_EXP			= 108;	// User의 경험치
const BYTE	AG_USER_UPDATE		= 109;	// User의 Update Info
const BYTE	AG_USER_FAIL		= 110;	// 잘못된 유저 처리...
const BYTE	AG_USER_PARTY		= 111;	// 파티처리 담당

////////////////////////////////////////////////////////////////
// Party Related subpacket define
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// Party Related subpacket define
////////////////////////////////////////////////////////////////
#define PARTY_CREATE		0x01	// Party Group Create
#define PARTY_PERMIT		0x02	// Party Insert Permit
#define PARTY_INSERT		0x03	// Party Member Insert
#define PARTY_REMOVE		0x04	// Party Member Remove
#define PARTY_DELETE		0x05	// Party Group Delete
#define PARTY_HPCHANGE		0x06	// Party Member HP change
#define PARTY_LEVELCHANGE	0x07	// Party Member Level change
#define PARTY_CLASSCHANGE	0x08	// Party Member Class Change
#define PARTY_STATUSCHANGE	0x09	// Party Member Status ( disaster, poison ) Change

////////////////////////////////////////////////////////////////
// chat define
////////////////////////////////////////////////////////////////
#define GENERAL_CHAT			0x01
#define PRIVATE_CHAT			0x02
#define PARTY_CHAT				0x03
#define FORCE_CHAT				0x04
#define SHOUT_CHAT				0x05
#define KNIGHTS_CHAT			0x06
#define PUBLIC_CHAT				0x07
#define WAR_SYSTEM_CHAT			0x08

#endif
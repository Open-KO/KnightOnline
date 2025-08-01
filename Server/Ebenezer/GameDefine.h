#ifndef _GAMEDEFINE_H
#define _GAMEDEFINE_H

#include <shared/globals.h>
#include <shared/_USER_DATA.h>

//////////////////// 직업별 Define ////////////////////
constexpr int KARUWARRRIOR		= 101;		// 카루전사
constexpr int KARUROGUE			= 102;		// 카루로그
constexpr int KARUWIZARD		= 103;		// 카루마법
constexpr int KARUPRIEST		= 104;		// 카루사제
constexpr int BERSERKER			= 105;		// 버서커
constexpr int GUARDIAN			= 106;		// 가디언
constexpr int HUNTER			= 107;		// 헌터
constexpr int PENETRATOR		= 108;		// 페너트레이터
constexpr int SORSERER			= 109;		// 소서러
constexpr int NECROMANCER		= 110;		// 네크로맨서
constexpr int SHAMAN			= 111;		// 샤만
constexpr int DARKPRIEST		= 112;		// 다크프리스트
constexpr int ELMORWARRRIOR		= 201;		// 엘모전사
constexpr int ELMOROGUE			= 202;		// 엘모로그
constexpr int ELMOWIZARD		= 203;		// 엘모마법
constexpr int ELMOPRIEST		= 204;		// 엘모사제
constexpr int BLADE				= 205;		// 블레이드
constexpr int PROTECTOR			= 206;		// 프로텍터
constexpr int RANGER			= 207;		// 레인져
constexpr int ASSASSIN			= 208;		// 어쌔신
constexpr int MAGE				= 209;		// 메이지
constexpr int ENCHANTER			= 210;		// 엔첸터
constexpr int CLERIC			= 211;		// 클레릭
constexpr int DRUID				= 212;		// 드루이드
/////////////////////////////////////////////////////

/////////////////////////////////////////////////////
// Race Define
/////////////////////////////////////////////////////
constexpr int KARUS_BIG			= 1;
constexpr int KARUS_MIDDLE		= 2;
constexpr int KARUS_SMALL		= 3;
constexpr int KARUS_WOMAN		= 4;
constexpr int BABARIAN			= 11;
constexpr int ELMORAD_MAN		= 12;
constexpr int ELMORAD_WOMAN		= 13;

// 타격비별 성공률 //
constexpr int GREAT_SUCCESS		= 1;	// 대성공
constexpr int SUCCESS			= 2;	// 성공
constexpr int NORMAL			= 3;	// 보통
constexpr int FAIL				= 4;	// 실패 

// Item Move Direction Define 
enum e_ItemMoveDirection
{
	ITEM_MOVE_INVEN_SLOT	= 1,
	ITEM_MOVE_SLOT_INVEN	= 2,
	ITEM_MOVE_INVEN_INVEN	= 3,
	ITEM_MOVE_SLOT_SLOT		= 4,
	ITEM_MOVE_INVEN_ZONE	= 5,
	ITEM_MOVE_ZONE_INVEN	= 6
};

// Item Weapon Type Define
enum e_WeaponType
{
	WEAPON_DAGGER		= 1,
	WEAPON_SWORD		= 2,
	WEAPON_AXE			= 3,
	WEAPON_MACE			= 4,
	WEAPON_SPEAR		= 5,
	WEAPON_SHIELD		= 6,
	WEAPON_BOW			= 7,
	WEAPON_LONGBOW		= 8,
	WEAPON_LAUNCHER		= 10,
	WEAPON_STAFF		= 11,
	WEAPON_ARROW		= 12,	// 스킬 없음
	WEAPON_JAVELIN		= 13,	// 스킬 없음
	WEAPON_WORRIOR_AC	= 21,	// 스킬 없음
	WEAPON_LOG_AC		= 22,	// 스킬 없음
	WEAPON_WIZARD_AC	= 23,	// 스킬 없음
	WEAPON_PRIEST_AC	= 24,	// 스킬 없음
};

////////////////////////////////////////////////////////////
// User Status //
enum e_UserStatus
{
	USER_STANDING				= 1,	// 서 있다.
	USER_SITDOWN				= 2,	// 앉아 있다.
	USER_DEAD					= 3,	// 듀거떠
	USER_BLINKING				= 4		// 방금 살아났어!!!
};
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Magic State
enum e_MagicState
{
	MAGIC_STATE_NONE			= 1,
	MAGIC_STATE_CASTING			= 2
};
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Durability Type
enum e_DurabilityType
{
	DURABILITY_TYPE_ATTACK		= 1,
	DURABILITY_TYPE_DEFENCE		= 2
};
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Knights Authority Type
/*
#define CHIEF				0x06
#define VICECHIEF			0x05*/
#define OFFICER				0x04
#define KNIGHT				0x03
//#define TRAINEE				0x02
#define PUNISH				0x01	

#define CHIEF				0x01	// 단장
#define VICECHIEF			0x02	// 부단장
#define TRAINEE				0x05	// 멤버
#define COMMAND_CAPTAIN		100		// 지휘권자
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// COMMUNITY TYPE DEFINE
#define CLAN_TYPE			0x01
#define KNIGHTS_TYPE		0x02
////////////////////////////////////////////////////////////

constexpr int MAX_CLAN			= 36;
constexpr int MAX_KNIGHTS_BANK	= 200;

constexpr int ITEM_GOLD			= 900000000;	// 돈 아이템 번호...
constexpr int ITEM_NO_TRADE		= 900000001;	// 거래 불가 아이템들.... 비러머글 크리스마스 이밴트 >.<		

////////////////////////////////////////////////////////////
// EVENT TYPE DEFINE
#define ZONE_CHANGE			0x01
#define ZONE_TRAP_DEAD		0x02
#define ZONE_TRAP_AREA		0x03

////////////////////////////////////////////////////////////
// EVENT MISCELLANOUS DATA DEFINE
#define ZONE_TRAP_INTERVAL	   1		// Interval is one second right now.
#define ZONE_TRAP_DAMAGE	   10		// HP Damage is 10 for now :)

////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// USER POINT DEFINE
enum e_StatType
{ /* explicitly used by CUser::PointChange() */
	STAT_TYPE_STR			= 1,
	STAT_TYPE_STA			= 2,
	STAT_TYPE_DEX			= 3,
	STAT_TYPE_INTEL			= 4,
	STAT_TYPE_CHA			= 5
};

enum e_SkillPtType
{
	SKILLPT_TYPE_ORDER		= 1,
	SKILLPT_TYPE_MANNER		= 2,
	SKILLPT_TYPE_LANGUAGE	= 3,
	SKILLPT_TYPE_BATTLE		= 4,
	SKILLPT_TYPE_PRO_1		= 5,
	SKILLPT_TYPE_PRO_2		= 6,
	SKILLPT_TYPE_PRO_3		= 7,
	SKILLPT_TYPE_PRO_4		= 8
};

/////////////////////////////////////////////////////////////
// ITEM TYPE DEFINE
enum e_ItemType
{
	ITEM_TYPE_FIRE			= 1,
	ITEM_TYPE_COLD			= 2,
	ITEM_TYPE_LIGHTNING		= 3,
	ITEM_TYPE_POISON		= 4,
	ITEM_TYPE_HP_DRAIN		= 5,
	ITEM_TYPE_MP_DAMAGE		= 6,
	ITEM_TYPE_MP_DRAIN		= 7,
	ITEM_TYPE_MIRROR_DAMAGE	= 8
};

/////////////////////////////////////////////////////////////
// ITEM LOG TYPE
enum e_ItemLogType
{
	ITEM_LOG_MERCHANT_BUY	= 1,
	ITEM_LOG_MERCHANT_SELL	= 2,
	ITEM_LOG_MONSTER_GET	= 3,
	ITEM_LOG_EXCHANGE_PUT	= 4,
	ITEM_LOG_EXCHANGE_GET	= 5,
	ITEM_LOG_DESTROY		= 6,
	ITEM_LOG_WAREHOUSE_PUT	= 7,
	ITEM_LOG_WAREHOUSE_GET	= 8,
	ITEM_LOG_UPGRADE		= 9
};

/////////////////////////////////////////////////////////////
// JOB GROUP TYPES
enum e_JobGroupType
{
	JOB_GROUP_WARRIOR				= 1,
	JOB_GROUP_ROGUE					= 2,
	JOB_GROUP_MAGE					= 3,
	JOB_GROUP_CLERIC				= 4,
	JOB_GROUP_ATTACK_WARRIOR		= 5,
	JOB_GROUP_DEFENSE_WARRIOR		= 6,
	JOB_GROUP_ARCHERER				= 7,
	JOB_GROUP_ASSASSIN				= 8,
	JOB_GROUP_ATTACK_MAGE			= 9,
	JOB_GROUP_PET_MAGE				= 10,
	JOB_GROUP_HEAL_CLERIC			= 11,
	JOB_GROUP_CURSE_CLERIC			= 12
};

//////////////////////////////////////////////////////////////
// USER ABNORMAL STATUS TYPES
enum e_AbnormalStatusType
{
	ABNORMAL_NORMAL					= 1,
	ABNORMAL_GIANT					= 2,
	ABNORMAL_DWARF					= 3,
	ABNORMAL_BLINKING				= 4
};

//////////////////////////////////////////////////////////////
// Object Type
#define NORMAL_OBJECT		0
#define SPECIAL_OBJECT		1

//////////////////////////////////////////////////////////////
// REGENE TYPES
#define REGENE_NORMAL		0
#define REGENE_MAGIC		1
#define REGENE_ZONECHANGE	2

//////////////////////////////////////////////////////////////
// TYPE 3 ATTRIBUTE TYPES
#define ATTRIBUTE_FIRE				1
#define ATTRIBUTE_ICE				2
#define ATTRIBUTE_LIGHTNING			3

import EbenezerModel;
namespace model = ebenezer_model;

// Bundle unit
struct _ZONE_ITEM
{
	DWORD bundle_index;
	int itemid[6];
	short count[6];
	float x;
	float z;
	float y;
	float time;
};

struct _EXCHANGE_ITEM
{
	int itemid;
	int count;
	short duration;
	BYTE pos;			//  교환후 들어갈 자리..
	int64_t	nSerialNum;	// item serial code
};

struct	_PARTY_GROUP
{
	WORD wIndex;
	short uid[8];		// 하나의 파티에 8명까지 가입가능
	short sMaxHp[8];
	short sHp[8];
	BYTE bLevel[8];
	short sClass[8];
	BYTE bItemRouting;
	_PARTY_GROUP()
	{
		for (int i = 0; i < 8; i++)
		{
			uid[i] = -1;
			sMaxHp[i] = 0;
			sHp[i] = 0;
			bLevel[i] = 0;
			sClass[i] = 0;
		}

		bItemRouting = 0;
	}
};

struct _OBJECT_EVENT
{
	BYTE byLife;			// 1:살아있다, 0:켁,, 죽음
	int sBelong;			// 소속
	short sIndex;			// 100 번대 - 카루스 바인드 포인트 | 200 번대 엘모라드 바인드 포인트 | 1100 번대 - 카루스 성문들 1200 - 엘모라드 성문들
	short sType;			// 0 - 바인드 포인트, 1 - 좌우로 열리는 성문, 2 - 상하로 열리는 성문, 3 - 레버, 4 - 깃발레버, 6:철창, 7-깨지는 부활비석
	short sControlNpcID;	// 조종할 NPC ID (조종할 Object Index), Type-> 5 : Warp Group ID
	short sStatus;			// status
	float fPosX;			// 위치값
	float fPosY;
	float fPosZ;
};

struct _REGENE_EVENT
{
	int	  sRegenePoint;		// 캐릭터 나타나는 지역 번호
	float fRegenePosX;		// 캐릭터 나타나는 지역의 왼아래쪽 구석 좌표 X
	float fRegenePosY;		// 캐릭터 나타나는 지역의 왼아래쪽 구석 좌표 Y
	float fRegenePosZ;		// 캐릭터 나타나는 지역의 왼아래쪽 구석 좌표 Z
	float fRegeneAreaZ;		// 캐릭터 나타나는 지역의 Z 축 길이 
	float fRegeneAreaX;		// 캐릭터 나타나는 지역의 X 축 길이
};

struct _KNIGHTS_USER
{
	BYTE    byUsed;								// 사용중 : 1, 비사용중 : 0
	char	strUserName[MAX_ID_SIZE + 1];		// 캐릭터의 이름
};

struct _ZONE_SERVERINFO
{
	short		sServerNo;
	short		sPort;
	char		strServerIP[20];

	_ZONE_SERVERINFO()
	{
		memset(strServerIP, 0, sizeof(strServerIP));
	}
};

// NOTE: This is loaded as-is from the SMD, padding and all.
// As such, this struct cannot be touched.
// If it needs to be extended, they should be split.
#pragma pack(push, 4)
struct _WARP_INFO
{
	short	sWarpID;
	char	strWarpName[32];
	char	strAnnounce[256];
	DWORD	dwPay;
	short	sZone;
	float	fX;
	float	fY;
	float	fZ;
	float	fR;
	short	sNation;

	_WARP_INFO()
	{
		sWarpID = 0;
		sZone = 0;
		fX = fZ = fY = fR = 0.0f;
		memset(strWarpName, 0, sizeof(strWarpName));
		memset(strAnnounce, 0, sizeof(strAnnounce));
		sNation = 0;
	}
};
#pragma pack(pop)

#endif

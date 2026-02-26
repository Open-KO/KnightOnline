#ifndef SERVER_EBENEZER_GAMEDEFINE_H
#define SERVER_EBENEZER_GAMEDEFINE_H

#pragma once

#include <shared/globals.h>
#include <shared-server/_USER_DATA.h>

#include <Ebenezer/model/EbenezerModel.h>

namespace Ebenezer
{

//////////////////// 직업별 Define ////////////////////
/////////////////////////////////////////////////////
// Race Define
/////////////////////////////////////////////////////
inline constexpr int KARUS_BIG     = 1;
inline constexpr int KARUS_MIDDLE  = 2;
inline constexpr int KARUS_SMALL   = 3;
inline constexpr int KARUS_WOMAN   = 4;
inline constexpr int BABARIAN      = 11;
inline constexpr int ELMORAD_MAN   = 12;
inline constexpr int ELMORAD_WOMAN = 13;

// 타격비별 성공률 //
inline constexpr int GREAT_SUCCESS = 1; // 대성공
inline constexpr int SUCCESS       = 2; // 성공
inline constexpr int NORMAL        = 3; // 보통
inline constexpr int FAIL          = 4; // 실패

// Item Move Direction Define
enum e_ItemMoveDirection : uint8_t
{
	ITEM_MOVE_INVEN_SLOT  = 1,
	ITEM_MOVE_SLOT_INVEN  = 2,
	ITEM_MOVE_INVEN_INVEN = 3,
	ITEM_MOVE_SLOT_SLOT   = 4,
	ITEM_MOVE_INVEN_ZONE  = 5,
	ITEM_MOVE_ZONE_INVEN  = 6
};

// Item Weapon Type Define
enum e_WeaponType : uint8_t
{
	WEAPON_DAGGER     = 1,
	WEAPON_SWORD      = 2,
	WEAPON_AXE        = 3,
	WEAPON_MACE       = 4,
	WEAPON_SPEAR      = 5,
	WEAPON_SHIELD     = 6,
	WEAPON_BOW        = 7,
	WEAPON_LONGBOW    = 8,
	WEAPON_LAUNCHER   = 10,
	WEAPON_STAFF      = 11,
	WEAPON_ARROW      = 12, // 스킬 없음
	WEAPON_JAVELIN    = 13, // 스킬 없음
	WEAPON_WORRIOR_AC = 21, // 스킬 없음
	WEAPON_LOG_AC     = 22, // 스킬 없음
	WEAPON_WIZARD_AC  = 23, // 스킬 없음
	WEAPON_PRIEST_AC  = 24, // 스킬 없음
};

////////////////////////////////////////////////////////////
// User Status //
enum e_UserStatus : uint8_t
{
	USER_STANDING = 1, // 서 있다.
	USER_SITDOWN  = 2, // 앉아 있다.
	USER_DEAD     = 3, // 듀거떠
	USER_BLINKING = 4  // 방금 살아났어!!!
};
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Magic State
enum e_MagicState : uint8_t
{
	MAGIC_STATE_NONE    = 1,
	MAGIC_STATE_CASTING = 2
};
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Durability Type
enum e_DurabilityType : uint8_t
{
	DURABILITY_TYPE_ATTACK  = 1,
	DURABILITY_TYPE_DEFENCE = 2
};
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// COMMUNITY TYPE DEFINE
enum e_ClanType : uint8_t
{
	CLAN_TYPE    = 0x01,
	KNIGHTS_TYPE = 0x02
};
////////////////////////////////////////////////////////////

inline constexpr int MAX_CLAN         = 36;
inline constexpr int MAX_KNIGHTS_BANK = 200;

// 돈 아이템 번호...
inline constexpr int ITEM_GOLD        = 900000000;

// 거래 불가 아이템들.... 비러머글 크리스마스 이밴트 >.<
inline constexpr int ITEM_NO_TRADE    = 900000001;

////////////////////////////////////////////////////////////
// EVENT TYPE DEFINE
enum e_GameEventType : uint8_t
{
	ZONE_CHANGE    = 0x01,
	ZONE_TRAP_DEAD = 0x02,
	ZONE_TRAP_AREA = 0x03
};

////////////////////////////////////////////////////////////
// EVENT MISCELLANOUS DATA DEFINE
inline constexpr int ZONE_TRAP_INTERVAL = 1;  // Interval is one second right now.
inline constexpr int ZONE_TRAP_DAMAGE   = 10; // HP Damage is 10 for now :)

////////////////////////////////////////////////////////////

enum e_BeefRoastVictory : uint8_t
{
	// Stage for Bifrost to be be captured by a nation - 30 minutes.
	BEEF_ROAST_VICTORY_START = 0,

	// Karus captured the Bifrost monument and can now solely enter Bifrost - 60 minutes.
	BEEF_ROAST_VICTORY_KARUS,

	// El Morad captured the Bifrost monument and can now solely enter Bifrost - 60 minutes.
	BEEF_ROAST_VICTORY_ELMORAD,

	// Both nations are allowed to enter Bifrost - 2 hours.
	BEEF_ROAST_VICTORY_ALL,

	// Bifrost is not active - 4 hours
	BEEF_ROAST_VICTORY_PENDING_RESTART_AFTER_VICTORY,

	// Bifrost is not active - 2 hours
	BEEF_ROAST_VICTORY_PENDING_RESTART_AFTER_DRAW,
};

enum e_InvasionMonumentType : uint8_t
{
	INVASION_MONUMENT_BASE  = 0, // Luferson/El Morad
	INVASION_MONUMENT_ASGA  = 1, // Asga/Bellua
	INVASION_MONUMENT_RAIBA = 2, // Raiba/Linate
	INVASION_MONUMENT_DODA  = 3, // Doda/Laon
	INVASION_MONUMENT_COUNT
};

////////////////////////////////////////////////////////////
// USER POINT DEFINE
enum e_StatType : uint8_t
{ /* explicitly used by CUser::PointChange() */
	STAT_TYPE_STR   = 1,
	STAT_TYPE_STA   = 2,
	STAT_TYPE_DEX   = 3,
	STAT_TYPE_INTEL = 4,
	STAT_TYPE_CHA   = 5
};

enum e_SkillPtType : uint8_t
{
	SKILLPT_TYPE_FREE     = 0,
	SKILLPT_TYPE_ORDER    = 1,
	SKILLPT_TYPE_MANNER   = 2,
	SKILLPT_TYPE_LANGUAGE = 3,
	SKILLPT_TYPE_BATTLE   = 4,
	SKILLPT_TYPE_PRO_1    = 5,
	SKILLPT_TYPE_PRO_2    = 6,
	SKILLPT_TYPE_PRO_3    = 7,
	SKILLPT_TYPE_PRO_4    = 8
};

/////////////////////////////////////////////////////////////
// ITEM TYPE DEFINE
enum e_ItemType : uint8_t
{
	ITEM_TYPE_NONE          = 0,
	ITEM_TYPE_FIRE          = 1,
	ITEM_TYPE_COLD          = 2,
	ITEM_TYPE_LIGHTNING     = 3,
	ITEM_TYPE_POISON        = 4,
	ITEM_TYPE_HP_DRAIN      = 5,
	ITEM_TYPE_MP_DAMAGE     = 6,
	ITEM_TYPE_MP_DRAIN      = 7,
	ITEM_TYPE_MIRROR_DAMAGE = 8
};

/////////////////////////////////////////////////////////////
// ITEM LOG TYPE
enum e_ItemLogType : uint8_t
{
	ITEM_LOG_MERCHANT_BUY   = 1,
	ITEM_LOG_MERCHANT_SELL  = 2,
	ITEM_LOG_MONSTER_GET    = 3,
	ITEM_LOG_EXCHANGE_PUT   = 4,
	ITEM_LOG_EXCHANGE_GET   = 5,
	ITEM_LOG_DESTROY        = 6,
	ITEM_LOG_WAREHOUSE_PUT  = 7,
	ITEM_LOG_WAREHOUSE_GET  = 8,
	ITEM_LOG_UPGRADE        = 9,
	ITEM_LOG_UPGRADE_FAILED = 10
};

/////////////////////////////////////////////////////////////
// JOB GROUP TYPES
enum e_JobGroupType : uint8_t
{
	JOB_GROUP_WARRIOR         = 1,
	JOB_GROUP_ROGUE           = 2,
	JOB_GROUP_MAGE            = 3,
	JOB_GROUP_CLERIC          = 4,
	JOB_GROUP_ATTACK_WARRIOR  = 5,
	JOB_GROUP_DEFENSE_WARRIOR = 6,
	JOB_GROUP_ARCHERER        = 7,
	JOB_GROUP_ASSASSIN        = 8,
	JOB_GROUP_ATTACK_MAGE     = 9,
	JOB_GROUP_PET_MAGE        = 10,
	JOB_GROUP_HEAL_CLERIC     = 11,
	JOB_GROUP_CURSE_CLERIC    = 12
};

//////////////////////////////////////////////////////////////
// USER ABNORMAL STATUS TYPES
enum e_AbnormalStatusType : uint8_t
{
	ABNORMAL_NORMAL   = 1,
	ABNORMAL_GIANT    = 2,
	ABNORMAL_DWARF    = 3,
	ABNORMAL_BLINKING = 4
};

//////////////////////////////////////////////////////////////
// Object Type
enum e_SpecialObjectType : uint8_t
{
	NORMAL_OBJECT  = 0,
	SPECIAL_OBJECT = 1
};

//////////////////////////////////////////////////////////////
// REGENE TYPES
enum e_RegeneType : uint8_t
{
	REGENE_NORMAL     = 0,
	REGENE_MAGIC      = 1,
	REGENE_ZONECHANGE = 2
};

//////////////////////////////////////////////////////////////
// TYPE 3 ATTRIBUTE TYPES
enum e_AttributeType : uint8_t
{
	ATTRIBUTE_FIRE      = 1,
	ATTRIBUTE_ICE       = 2,
	ATTRIBUTE_LIGHTNING = 3
};

// Bundle unit
struct _ZONE_ITEM
{
	uint32_t bundle_index = 0;
	int itemid[6]         = {};
	int16_t count[6]      = {};
	float x               = 0.0f;
	float z               = 0.0f;
	float y               = 0.0f;
	double time           = 0.0;
};

struct _EXCHANGE_ITEM
{
	int itemid         = 0;
	int count          = 0;
	int16_t duration   = 0;
	uint8_t pos        = 0; //  교환후 들어갈 자리..
	int64_t nSerialNum = 0; // item serial code
};

struct _PARTY_GROUP
{
	uint16_t wIndex;
	int16_t userSocketIds[MAX_PARTY_SIZE];
	int16_t sMaxHp[MAX_PARTY_SIZE];
	int16_t sHp[MAX_PARTY_SIZE];
	uint8_t bLevel[MAX_PARTY_SIZE];
	int16_t sClass[MAX_PARTY_SIZE];
	uint8_t bItemRouting;

	_PARTY_GROUP()
	{
		for (int i = 0; i < MAX_PARTY_SIZE; i++)
		{
			userSocketIds[i] = -1;
			sMaxHp[i]        = 0;
			sHp[i]           = 0;
			bLevel[i]        = 0;
			sClass[i]        = 0;
		}

		wIndex       = 0;
		bItemRouting = 0;
	}
};

struct _OBJECT_EVENT
{
	// 1:살아있다, 0:켁,, 죽음
	uint8_t byLife        = 0;

	// 소속
	int sBelong           = 0;

	// 100 번대 - 카루스 바인드 포인트 | 200 번대 엘모라드 바인드 포인트 | 1100 번대 - 카루스 성문들 1200 - 엘모라드 성문들
	int16_t sIndex        = 0;

	// 0 - 바인드 포인트, 1 - 좌우로 열리는 성문, 2 - 상하로 열리는 성문, 3 - 레버, 4 - 깃발레버, 6:철창, 7-깨지는 부활비석
	int16_t sType         = 0;

	// 조종할 NPC ID (조종할 Object Index), Type-> 5 : Warp Group ID
	int16_t sControlNpcID = -1;

	// status
	int16_t sStatus       = 0;

	// 위치값
	float fPosX           = 0.0f;
	float fPosY           = 0.0f;
	float fPosZ           = 0.0f;
};

struct _REGENE_EVENT
{
	int sRegenePoint   = 0;    // 캐릭터 나타나는 지역 번호
	float fRegenePosX  = 0.0f; // 캐릭터 나타나는 지역의 왼아래쪽 구석 좌표 X
	float fRegenePosY  = 0.0f; // 캐릭터 나타나는 지역의 왼아래쪽 구석 좌표 Y
	float fRegenePosZ  = 0.0f; // 캐릭터 나타나는 지역의 왼아래쪽 구석 좌표 Z
	float fRegeneAreaZ = 0.0f; // 캐릭터 나타나는 지역의 Z 축 길이
	float fRegeneAreaX = 0.0f; // 캐릭터 나타나는 지역의 X 축 길이
};

struct _KNIGHTS_USER
{
	// 사용중 : 1, 비사용중 : 0
	uint8_t byUsed                    = 0;

	// 캐릭터의 이름
	char strUserName[MAX_ID_SIZE + 1] = {};
};

struct _ZONE_SERVERINFO
{
	int16_t sServerNo       = 0;
	int16_t sPort           = 0;
	std::string strServerIP = {};
};

// NOTE: This is loaded as-is from the SMD, padding and all.
// As such, this struct cannot be touched.
// If it needs to be extended, they should be split.
#pragma pack(push, 4)
struct _WARP_INFO
{
	int16_t sWarpID       = 0;
	char strWarpName[32]  = {};
	char strAnnounce[256] = {};
	uint32_t dwPay        = 0;
	int16_t sZone         = 0;
	float fX              = 0.0f;
	float fY              = 0.0f;
	float fZ              = 0.0f;
	float fR              = 0.0f;
	int16_t sNation       = 0;
};
#pragma pack(pop)

namespace model = ebenezer_model;

} // namespace Ebenezer

#endif // SERVER_EBENEZER_GAMEDEFINE_H

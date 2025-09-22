#pragma once

#include "version.h"
#include "packets.h"
#include "Packet.h"

#define MAP_DIR				"../MAP/"
#define QUESTS_DIR			"../QUESTS/"

constexpr int MIN_ID_SIZE		= 6;
constexpr int MAX_ID_SIZE		= 20;
constexpr int MAX_NPC_NAME_SIZE	= 30;
constexpr int MAX_PW_SIZE		= 12;
constexpr int MAX_IP_SIZE		= 15;	// IPv4 addresses are max ###.###.###.### (3*4 + 3), or 15 bytes

constexpr int MAX_ITEM_COUNT	= 9999;	// 한 슬롯에 가지는 최대 화살/송편 개수
constexpr int MAX_QUEST			= 100;
constexpr int MAX_LEVEL			= 80;	// 최고렙...
constexpr int MAX_GOLD			= 2'100'000'000;
constexpr int VIEW_DISTANCE		= 48;

enum e_ItemSaleType
{
	SALE_TYPE_LOW			= 0, // sells lower than purchase price
	SALE_TYPE_FULL			= 1, // sells equal to purchase price
	SALE_TYPE_LOW_NO_REPAIR	= 2, // irreparable items sell for lower price than purchase
};

enum e_NpcState
{
	NPC_DEAD = 0,
	NPC_LIVE,
	NPC_ATTACKING,
	NPC_ATTACKED,
	NPC_ESCAPE,
	NPC_STANDING,
	NPC_MOVING,
	NPC_TRACING,
	NPC_FIGHTING,
	NPC_STRATEGY,
	NPC_BACK,
	NPC_SLEEPING,
	NPC_FAINTING,
	NPC_HEALING,
	NPC_CASTING
};

enum e_NpcType : uint8_t
{
	NPC_MONSTER				= 0,
	NPCTYPE_MONSTER			= NPC_MONSTER,

	NPC_GENERAL				= 1,
	NPC_EVENT				= 2, // can only use regular attacks on them, damage output is always 20.
	NPC_BOSS				= 3, // 대장 몬스터
	NPC_BOSS_MONSTER		= NPC_BOSS,

	NPC_DUNGEON_MONSTER		= 4, // 던젼 몬스터
	NPC_TRAP_MONSTER		= 5, // 함정 몬스터
	NPC_UNK_6				= 6,
	NPC_REFUGEE				= 10,
	NPC_GUARD				= 11, // 붙박이형 경비병
	NPC_PATROL_GUARD		= 12, // 일반 필드에서 정찰을 담당하는 정찰병
	NPC_STORE_GUARD			= 13, // 일반 필드에서 상점주변을 보호하는 경비병
	NPC_WAR_GUARD			= 14,
	NPC_PET					= 15,
	NPC_GUARD_SUMMON		= 16,

	NPC_MERCHANT			= 21, // 상점주인 NPC
	NPC_TINKER				= 22, // 대장장이
	NPC_SELITH				= 23, // Selith[special store]
	NPC_ANVIL				= 24,
	NPC_MARK				= 25,
	NPC_CLAN_MATCH_ADVISOR	= 26,
	NPC_SIEGE_MAINTAIN		= 27, // Delos castle manager, opens tax rates etc. interface
	NPC_TELEPORT_GATE		= 28,
	NPC_OPERATOR			= 29, // not sure what Operator Moira was ever supposed to do...
	NPC_SUMMON				= 30,
	NPC_WAREHOUSE			= 31,
	NPC_KISS				= 32, // pretty useless.
	NPC_ISAAC				= 33, // need to check which quests he handles
	NPC_KAISHAN				= 34, // need to see what he actually does to name this properly
	NPC_CAPTAIN				= 35, // 전직 시켜주는 NPC
	NPC_CLAN				= 36, // 기사단 관리 NPC
	NPC_OFFICER				= NPC_CLAN,
	NPC_CLERIC				= 37, // 대사제 NPC
	NPC_LADY				= 38, // Calamus lady event -- need to see what they're used for
	NPC_ATHIAN				= 39, // Priest athian -- need to see what they're used for
	NPC_HEALER				= 40, // Healer
	NPC_WARP				= 41,
	NPC_ROOM				= 42,
	NPC_ARENA				= 43, // also recon guards
	NPC_SIEGE				= 44,
	NPC_TRAINER_KATE		= 45, // scripted NPC, but can sell items from a regular shop as well.
	NPC_GENERIC				= 46, // general bytype used for NPCs.
	NPC_SENTINEL_PATRICK	= 47, // need to check which quests he handles (was it the beginner quests, or was that isaac?)
	NPC_TRADER_KIM			= 48, // Offering Trader Kim
	NPC_PRIEST_IRIS			= 49, // [Priest] Iris
	NPC_GATE				= 50, // 성문 (6->50)
	NPC_DOOR				= NPC_GATE,

	NPC_PHOENIX_GATE		= 51, // 깨지지 않는 문 (8->51)
	NPC_SPECIAL_GATE		= 52, // 깨지지 않는 문이면서 2분마다 열렸다 닫혔다 하는 문
	NPC_VICTORY_GATE		= 53,
	NPC_GATE_LEVER			= 55, // 성문 레버...	(9->55)	
	NPC_ARTIFACT			= 60, // 결계석 (7->60)
	NPC_DESTROYED_ARTIFACT	= 61, // 파괴되는 결계석
	NPC_DESTORY_ARTIFACT	= NPC_DESTROYED_ARTIFACT,

	NPC_GUARD_TOWER_NEW		= 62,
	NPC_GUARD_TOWER			= 63,
	NPC_BOARD				= 64, // also encampment
	NPC_ARTIFACT1			= 65, // Protective artifact
	NPC_ARTIFACT2			= 66, // Guard Tower artifact
	NPC_ARTIFACT3			= 67, // Guard artifact
	NPC_ARTIFACT4			= 68,
	NPC_MONK_ELMORAD		= 71,
	NPC_MONK_KARUS			= 72,
	NPC_MASTER_WARRIOR		= 73, // Warrior Master Skaky
	NPC_MASTER_ROGUE		= 74, // Secret Agent Clarence
	NPC_MASTER_MAGE			= 75, // Arch Mage Drake
	NPC_MASTER_PRIEST		= 76, // Priest Minerva
	NPC_BLACKSMITH			= 77,
	NPC_RENTAL				= 78,
	NPC_ELECTION			= 79, // king elections
	NPC_TREASURY			= 80,
	NPC_CLAN_BANK			= 85,
	NPC_DOMESTIC_ANIMAL		= 99, // 가축 NPC
	NPC_COUPON				= 100,
	NPC_NPC_1				= 101, // "NPC1"
	NPC_NPC_2				= 102, // "NPC2"
	NPC_NPC_3				= 103, // "NPC3"
	NPC_NPC_4				= 104, // "NPC4"
	NPC_NPC_5				= 105, // "NPC5"
	NPC_HERO_STATUE_1		= 106, // 1st place
	NPC_HERO_STATUE_2		= 107, // 2nd place
	NPC_HERO_STATUE_3		= 108, // 3rd place
	NPC_KARUS_HERO_STATUE	= 109,
	NPC_ELMORAD_HERO_STATUE	= 110,
	NPC_KEY_QUEST_1			= 111, // Sentinel of the Key
	NPC_KEY_QUEST_2			= 112, // Watcher of the Key
	NPC_KEY_QUEST_3			= 113, // Protector of the Key
	NPC_KEY_QUEST_4			= 114, // Ranger of the Key
	NPC_KEY_QUEST_5			= 115, // Patroller of the Key
	NPC_KEY_QUEST_6			= 116, // Recon of the Key
	NPC_KEY_QUEST_7			= 117, // Keeper of the Key
	NPC_ROBOS				= 118, // need to see what he actually does to name this properly
	NPC_KARUS_MONUMENT		= 121, // Karus invasion zone monuments (Asga village/Raiba village/Doda camp monuments)
	NPC_ELMORAD_MONUMENT	= 122, // El Morad invasion zone monuments (Asga village/Raiba village/Doda camp monuments)
	NPC_SERVER_TRANSFER		= 123,
	NPC_RANKING				= 124,
	NPC_LYONI				= 125, // need to see what this NPC actually does to name this properly
	NPC_BEGINNER_HELPER_1	= 126, // Adien[Beginner Helper]
	NPC_BEGINNER_HELPER_2	= 127, // Adine[Beginner helper]
	NPC_BEGINNER_HELPER_3	= 128, // Adirian[Beginner Helper]
	NPC_FT_1				= 129,
	NPC_FT_2				= 130,
	NPC_FT_3				= 131, // also Priest Minerva
	NPC_PREMIUM_PC			= 132, // captain[Premium PC]
	NPC_KJWAR				= 133,
	NPC_SIEGE_2				= 134,
	NPC_CRAFTSMAN			= 135, // Craftsman boy, not sure what he's actually used for
	NPC_COLISEUM_ARTES		= 136, // [Coliseum] Artes
	NPC_MANAGER_BARREL		= 137, // Manager Barrel
	NPC_UNK_138				= 138,
	NPC_DB_CHINA			= 139, // Opens DB_CHINA.
	NPC_LOVE_AGENT			= 140,
	NPC_SPY					= 141,
	NPC_ROYAL_GUARD			= 142,
	NPC_ROYAL_CHEF			= 143,
	NPC_ESLANT_WOMAN		= 144,
	NPC_FARMER				= 145,
	NPC_NAMELESS_WARRIOR	= 146,
	NPC_UNK_147				= 147,
	NPC_GATE_GUARD			= 148,
	NPC_ROYAL_ADVISOR		= 149,
	NPC_BIFROST_GATE		= 150,
	NPC_SANGDUF				= 151, // Sangduf[Teleport NPC]
	NPC_UNK_152				= 152,
	NPC_ADELIA				= 153, // Goddess Adelia[event]
	NPC_BIFROST_MONUMENT	= 154,
	NPC_UNK_155				= 155,
	NPC_CHAOTIC_GENERATOR	= 162, // newer type used by the Chaotic Generator
	NPC_SCARECROW			= 171, // official scarecrow byType
	NPC_KARUS_WARDER1		= 190,
	NPC_KARUS_WARDER2		= 191,
	NPC_ELMORAD_WARDER1		= 192,
	NPC_ELMORAD_WARDER2		= 193,
	NPC_KARUS_GATEKEEPER	= 198,
	NPC_ELMORAD_GATEKEEPER	= 199,
	NPC_CHAOS_STONE		 	= 200,
	NPC_PVP_MONUMENT		= 210,
	NPC_BATTLE_MONUMENT		= 211
};

enum e_ObjectType
{
	OBJECT_TYPE_BIND				= 0,
	OBJECT_TYPE_BINDPOINT			= OBJECT_TYPE_BIND,
	OBJECT_TYPE_GATE				= 1,
	OBJECT_TYPE_DOOR_LEFTRIGHT		= OBJECT_TYPE_GATE,
	OBJECT_TYPE_DOOR_TOPDOWN		= 2,
	OBJECT_TYPE_GATE_LEVER			= 3,
	OBJECT_TYPE_LEVER_TOPDOWN		= OBJECT_TYPE_GATE_LEVER,
	OBJECT_TYPE_FLAG				= 4,
	OBJECT_TYPE_WARP_GATE			= 5,
	OBJECT_TYPE_WARP_POINT			= OBJECT_TYPE_WARP_GATE,
	OBJECT_TYPE_BARRICADE			= 6,
	OBJECT_TYPE_REMOVE_BIND			= 7, // this seems to behave identically to OBJECT_TYPE_BIND
	OBJECT_TYPE_ANVIL				= 8,
	OBJECT_TYPE_ARTIFACT			= 9,
	OBJECT_TYPE_UNKNOWN				= 0xffffffff
};

// These control neutrality-related settings client-side, 
// including whether collision is enabled for other players.
enum e_ZoneAbilityType
{
	ZONE_ABILITY_NEUTRAL				= 0, // Players cannot attack each other, or NPCs. Can walk through players.
	ZONE_ABILITY_PVP					= 1, // Players can attack each other, and only NPCs from the opposite nation. Cannot walk through players.
	ZONE_ABILITY_SPECTATOR				= 2, // Player is spectating a 1v1 match (ZoneAbilityPVP is sent for the attacker)
	ZONE_ABILITY_SIEGE_TYPE_1			= 3, // siege type 1 (unknown)
	ZONE_ABILITY_SIEGE_TYPE_2			= 4, // siege type 2/4: if they have 0 NP & this is set, it will not show the message telling them to buy mo	 re.
	ZONE_ABILITY_SIEGE_TYPE_3			= 5, // Siege type 3 (unknown)
	ZONE_ABILITY_SIEGE_DISABLED			= 6, // CSW not running
	ZONE_ABILITY_CAITHAROS_ARENA		= 7, // Players can attack each other (don't seem to be able to anymore?), but not NPCs. Can walk through players.
	ZONE_ABILITY_PVP_NEUTRAL_NPCS		= 8	 // Players can attack each other, but not NPCs. Cannot walk through players.
};

enum e_ZoneID
{
	ZONE_KARUS				= 1,
	ZONE_ELMORAD			= 2,
	ZONE_ESLANT_KARUS		= 11,
	ZONE_ESLANT_ELMORAD		= 12,
	ZONE_MORADON			= 21,
	ZONE_DELOS				= 30,
	ZONE_BIFROST			= 31,
	ZONE_DESPERATION_ABYSS	= 32,
	ZONE_HELL_ABYSS			= 33,
	ZONE_ARENA				= 48,
	ZONE_CAITHAROS_ARENA	= 55,
	ZONE_BATTLE				= 101,
	ZONE_BATTLE2			= 102,
	ZONE_BATTLE3			= 103,
	ZONE_SNOW_BATTLE		= 111,
	ZONE_FRONTIER			= 201
};

// ITEM_SLOT DEFINE
constexpr uint8_t RIGHTEAR			= 0;
constexpr uint8_t HEAD				= 1;
constexpr uint8_t LEFTEAR			= 2;
constexpr uint8_t NECK				= 3;
constexpr uint8_t BREAST			= 4;
constexpr uint8_t SHOULDER			= 5;
constexpr uint8_t RIGHTHAND			= 6;
constexpr uint8_t WAIST				= 7;
constexpr uint8_t LEFTHAND			= 8;
constexpr uint8_t RIGHTRING			= 9;
constexpr uint8_t LEG				= 10;
constexpr uint8_t LEFTRING			= 11;
constexpr uint8_t GLOVE				= 12;
constexpr uint8_t FOOT				= 13;
constexpr uint8_t RESERVED			= 14;

constexpr uint8_t SLOT_MAX			= 14;	// 14 equipped item slots
constexpr uint8_t HAVE_MAX			= 28;	// 28 inventory slots
constexpr uint8_t WAREHOUSE_MAX		= 192;	// 창고 아이템 MAX

// Start of inventory area
constexpr int INVENTORY_INVENT		= SLOT_MAX;

// Total slots in the general-purpose inventory storage
constexpr int INVENTORY_TOTAL		= SLOT_MAX + HAVE_MAX;

constexpr uint8_t MAX_MERCH_ITEMS	= 12;

constexpr int MAX_MERCH_MESSAGE		= 40;

constexpr int ITEMCOUNT_MAX			= 9999;

constexpr int MAX_KNIGHTS_MARK		= 2400;
constexpr int CLAN_SYMBOL_COST		= 5000000;

#define NEWCHAR_SUCCESS						uint8_t(0)
#define NEWCHAR_NO_MORE						uint8_t(1)
#define NEWCHAR_INVALID_DETAILS				uint8_t(2)
#define NEWCHAR_EXISTS						uint8_t(3)
#define NEWCHAR_DB_ERROR					uint8_t(4)
#define NEWCHAR_INVALID_NAME				uint8_t(5)
#define NEWCHAR_BAD_NAME					uint8_t(6)
#define NEWCHAR_INVALID_RACE				uint8_t(7)
#define NEWCHAR_NOT_SUPPORTED_RACE			uint8_t(8)
#define NEWCHAR_INVALID_CLASS				uint8_t(9)
#define NEWCHAR_POINTS_REMAINING			uint8_t(10)
#define NEWCHAR_STAT_TOO_LOW				uint8_t(11)

enum e_ItemFlag
{
	ITEM_FLAG_NONE		= 0,
	ITEM_FLAG_RENTED	= 1,
	ITEM_FLAG_DUPLICATE = 3
};

enum e_Authority
{
	AUTHORITY_MANAGER			= 0,
	AUTHORITY_USER				= 1,
	// AUTHORITY_NOCHAT			= 2,
	AUTHORITY_NPC				= 3,
	AUTHORITY_NOCHAT			= 11,
	AUTHORITY_ATTACK_DISABLED	= 12,
	AUTHORITY_LIMITED_MANAGER	= 250,
	AUTHORITY_BLOCK_USER		= 255
};

enum e_AttackResult
{
	ATTACK_FAIL					= 0,
	ATTACK_SUCCESS				= 1,
	ATTACK_TARGET_DEAD			= 2,
	ATTACK_TARGET_DEAD_OK		= 3,
	MAGIC_ATTACK_TARGET_DEAD	= 4
};

enum e_QuestState : uint8_t
{
	QUEST_STATE_NOT_STARTED		= 0,
	QUEST_STATE_IN_PROGRESS		= 1,
	QUEST_STATE_COMPLETE		= 2
};

uint64_t RandUInt64();
time_t getMSTime();

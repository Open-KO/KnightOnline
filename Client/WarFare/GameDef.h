#ifndef __GAME_DEF_H_
#define __GAME_DEF_H_

#include <string>
#include <dinput.h>

#include <shared/types.h>
#include <shared/version.h>

// TODO: Shift this logic into a separate header and generally clean this shared logic up
#ifndef ASSERT
#if defined(_DEBUG)
#define ASSERT assert
#include <assert.h>
#else
#define ASSERT
#endif
#endif

#include <shared/Packet.h>

#include <shared/globals.h>

constexpr int CURRENT_VERSION = 1298;

// This is the maximum time we must wait after sending the WIZ_VERSION_CHECK packet on the login scene, before we're allowed
// to attempt to re-establish a connection to the game server.
// We'll use 5 seconds here as it's a more than reasonable enough time for it to receive a packet, even with lag, while not being
// too excessive.
// If the client disconnects in this time, this timer will be reset, so there's no need to account for this.
// Officially it Sleep()s for 1 second prior to even sending WIZ_VERSION_CHECK packet we need to wait for, so there's not really
// a comparable official limit. All this accomplishes is reducing the number of connection attempts, not preventing overlaps.
constexpr float TIME_UNTIL_NEXT_GAME_CONNECTION_ATTEMPT = 5.0f;

constexpr float PACKET_INTERVAL_MOVE = 1.5f;				// Interval between regularly sent player/NPC movement packets.
constexpr float PACKET_INTERVAL_ROTATE = 4.0f;				// Interval between regularly sent player rotation packets.
constexpr float PACKET_INTERVAL_REQUEST_TARGET_HP = 2.0f;

#define N3_FORMAT_VER_1068 0x00000001
#define N3_FORMAT_VER_1298 0x00000002

enum e_ExitType
{
	EXIT_TYPE_NONE			= 0,
	EXIT_TYPE_CHR_SELECT	= 1,
	EXIT_TYPE_QUIT			= 2,
};

constexpr int EXIT_TIME_AFTER_BATTLE	= 10;

// 단축키 지정해 놓은 부분..
enum eKeyMap {	KM_HOTKEY1 = DIK_1, 
				KM_HOTKEY2 = DIK_2, 
				KM_HOTKEY3 = DIK_3, 
				KM_HOTKEY4 = DIK_4, 
				KM_HOTKEY5 = DIK_5, 
				KM_HOTKEY6 = DIK_6, 
				KM_HOTKEY7 = DIK_7, 
				KM_HOTKEY8 = DIK_8, 
				KM_TOGGLE_RUN = DIK_T, 
				KM_TOGGLE_MOVE_CONTINOUS = DIK_E, 
				KM_TOGGLE_ATTACK = DIK_R, 
				KM_TOGGLE_SITDOWN = DIK_C, 
				KM_TOGGLE_INVENTORY = DIK_I, 
				KM_TOGGLE_SKILL = DIK_K, 
				KM_TOGGLE_STATE = DIK_U, 
				KM_TOGGLE_MINIMAP = DIK_M, 
				KM_TOGGLE_HELP = DIK_F10,
				KM_TOGGLE_CMDLIST = DIK_H,
				KM_CAMERA_CHANGE = DIK_F9, 
				KM_DROPPED_ITEM_OPEN = DIK_F, 
				KM_MOVE_FOWARD = DIK_W, 
				KM_MOVE_BACKWARD = DIK_S, 
				KM_ROTATE_LEFT = DIK_A, 
				KM_ROTATE_RIGHT = DIK_D, 
				KM_TARGET_NEAREST_ENEMY = DIK_Z, 
				KM_TARGET_NEAREST_PARTY = DIK_X, 
				KM_TARGET_NEAREST_FRIEND = DIK_V, 
				KM_TARGET_NEAREST_NPC = DIK_B,
				KM_SKILL_PAGE_1 = DIK_F1, 
				KM_SKILL_PAGE_2 = DIK_F2,
				KM_SKILL_PAGE_3 = DIK_F3,
				KM_SKILL_PAGE_4 = DIK_F4,
				KM_SKILL_PAGE_5 = DIK_F5,
				KM_SKILL_PAGE_6 = DIK_F6,
				KM_SKILL_PAGE_7 = DIK_F7,
				KM_SKILL_PAGE_8 = DIK_F8 };

enum e_PlayerType { PLAYER_BASE = 0, PLAYER_NPC = 1, PLAYER_OTHER = 2, PLAYER_MYSELF = 3 };

enum e_Race {	RACE_ALL = 0,
				RACE_KA_ARKTUAREK = 1, RACE_KA_TUAREK = 2, RACE_KA_WRINKLETUAREK = 3, RACE_KA_PURITUAREK = 4, 
				RACE_EL_BABARIAN = 11, RACE_EL_MAN = 12, RACE_EL_WOMEN = 13,
				//RACE_KA_NORMAL = 11, RACE_KA_WARRIOR = 12, RACE_KA_ROGUE = 13, RACE_KA_MAGE = 14,
				RACE_NPC = 100,
				RACE_UNKNOWN = 0xffffffff };

enum e_Class {	CLASS_KINDOF_WARRIOR = 1, CLASS_KINDOF_ROGUE, CLASS_KINDOF_WIZARD, CLASS_KINDOF_PRIEST,
				CLASS_KINDOF_ATTACK_WARRIOR, CLASS_KINDOF_DEFEND_WARRIOR, CLASS_KINDOF_ARCHER, CLASS_KINDOF_ASSASSIN, 
				CLASS_KINDOF_ATTACK_WIZARD, CLASS_KINDOF_PET_WIZARD, CLASS_KINDOF_HEAL_PRIEST, CLASS_KINDOF_CURSE_PRIEST,

				CLASS_KA_WARRIOR = 101, CLASS_KA_ROGUE, CLASS_KA_WIZARD, CLASS_KA_PRIEST, // Basic classes up to this point
				CLASS_KA_BERSERKER = 105, CLASS_KA_GUARDIAN, CLASS_KA_HUNTER = 107, CLASS_KA_PENETRATOR, 
				CLASS_KA_SORCERER = 109, CLASS_KA_NECROMANCER, CLASS_KA_SHAMAN = 111, CLASS_KA_DARKPRIEST, 
				
				CLASS_EL_WARRIOR = 201, CLASS_EL_ROGUE, CLASS_EL_WIZARD, CLASS_EL_PRIEST, // Basic classes up to this point
				CLASS_EL_BLADE = 205, CLASS_EL_PROTECTOR, CLASS_EL_RANGER = 207, CLASS_EL_ASSASIN, 
				CLASS_EL_MAGE = 209, CLASS_EL_ENCHANTER, CLASS_EL_CLERIC = 211, CLASS_EL_DRUID,
				
				CLASS_UNKNOWN = 0xffffffff };

enum e_Class_Represent { CLASS_REPRESENT_WARRIOR = 0, CLASS_REPRESENT_ROGUE, CLASS_REPRESENT_WIZARD, CLASS_REPRESENT_PRIEST, CLASS_REPRESENT_UNKNOWN = 100 };

constexpr float WEAPON_WEIGHT_STAND_SWORD = 5.0f;	// Standard weight of swords
constexpr float WEAPON_WEIGHT_STAND_AXE = 5.0f;		// Standard weight of axes
constexpr float WEAPON_WEIGHT_STAND_BLUNT = 8.0f;	// Standard weight of blunt type weapons

enum e_Ani {	ANI_BREATH = 0, ANI_WALK, ANI_RUN, ANI_WALK_BACKWARD, ANI_STRUCK0, ANI_STRUCK1, ANI_STRUCK2, ANI_GUARD,
				ANI_DEAD_NEATLY = 8, ANI_DEAD_KNOCKDOWN, ANI_DEAD_ROLL, ANI_SITDOWN, ANI_SITDOWN_BREATH, ANI_STANDUP,
				ANI_ATTACK_WITH_WEAPON_WHEN_MOVE = 14, ANI_ATTACK_WITH_NAKED_WHEN_MOVE, 

				ANI_SPELLMAGIC0_A = 16, ANI_SPELLMAGIC0_B, 
				ANI_SPELLMAGIC1_A = 18, ANI_SPELLMAGIC1_B, 
				ANI_SPELLMAGIC2_A = 20, ANI_SPELLMAGIC2_B, 
				ANI_SPELLMAGIC3_A = 22, ANI_SPELLMAGIC3_B, 
				ANI_SPELLMAGIC4_A = 24, ANI_SPELLMAGIC4_B, 
				
				ANI_SHOOT_ARROW_A = 26, ANI_SHOOT_ARROW_B, 
				ANI_SHOOT_QUARREL_A = 28, ANI_SHOOT_QUARREL_B, 
				ANI_SHOOT_JAVELIN_A = 30, ANI_SHOOT_JAVELIN_B, 
				
				ANI_SWORD_BREATH_A = 32,	ANI_SWORD_ATTACK_A0, ANI_SWORD_ATTACK_A1,
				ANI_SWORD_BREATH_B,			ANI_SWORD_ATTACK_B0, ANI_SWORD_ATTACK_B1,		// One-handed swords
				
				ANI_DAGGER_BREATH_A = 38,	ANI_DAGGER_ATTACK_A0, ANI_DAGGER_ATTACK_A1,
				ANI_DAGGER_BREATH_B,		ANI_DAGGER_ATTACK_B0, ANI_DAGGER_ATTACK_B1,		// Daggers
				
				ANI_DUAL_BREATH_A = 44,		ANI_DUAL_ATTACK_A0, ANI_DUAL_ATTACK_A1, 
				ANI_DUAL_BREATH_B,			ANI_DUAL_ATTACK_B0, ANI_DUAL_ATTACK_B1,			// Dual wielded items
				
				ANI_SWORD2H_BREATH_A = 50,	ANI_SWORD2H_ATTACK_A0, ANI_SWORD2H_ATTACK_A1, 
				ANI_SWORD2H_BREATH_B,		ANI_SWORD2H_ATTACK_B0, ANI_SWORD2H_ATTACK_B1,	// Two-handed swords
				
				ANI_BLUNT_BREATH_A = 56,	ANI_BLUNT_ATTACK_A0, ANI_BLUNT_ATTACK_A1, 
				ANI_BLUNT_BREATH_B,			ANI_BLUNT_ATTACK_B0, ANI_BLUNT_ATTACK_B1,		// Blunt weapons – maces
				
				ANI_BLUNT2H_BREATH_A = 62,	ANI_BLUNT2H_ATTACK_A0, ANI_BLUNT2H_ATTACK_A1, 
				ANI_BLUNT2H_BREATH_B,		ANI_BLUNT2H_ATTACK_B0, ANI_BLUNT2H_ATTACK_B1,	// Two-handed blunt weapons (maces), and axes.
				
				ANI_AXE_BREATH_A = 68,		ANI_AXE_ATTACK_A0, ANI_AXE_ATTACK_A1, 
				ANI_AXE_BREATH_B,			ANI_AXE_ATTACK_B0, ANI_AXE_ATTACK_B1,			// One-handed axes
				
				ANI_SPEAR_BREATH_A = 74,	ANI_SPEAR_ATTACK_A0, ANI_SPEAR_ATTACK_A1, 
				ANI_SPEAR_BREATH_B,			ANI_SPEAR_ATTACK_B0, ANI_SPEAR_ATTACK_B1,		// Spears – just a simple spear with no cutting edge.
				
				ANI_POLEARM_BREATH_A = 80,	ANI_POLEARM_ATTACK_A0, ANI_POLEARM_ATTACK_A1, 
				ANI_POLEARM_BREATH_B,		ANI_POLEARM_ATTACK_B0, ANI_POLEARM_ATTACK_B1,	// Two-handed bladed spears – something like a "Cheongryongdo" (Blue Dragon Sword)?
				
				ANI_NAKED_BREATH_A = 86,	ANI_NAKED_ATTACK_A0, ANI_NAKED_ATTACK_A1, 
				ANI_NAKED_BREATH_B,			ANI_NAKED_ATTACK_B0, ANI_NAKED_ATTACK_B1,		// Bare-handed??
				
				ANI_BOW_BREATH = 92,		ANI_CROSS_BOW_BREATH, ANI_LAUNCHER_BREATH, 
				ANI_BOW_BREATH_B,			ANI_BOW_ATTACK_B0, ANI_BOW_ATTACK_B1,			// Bow attacks
				
				ANI_SHIELD_BREATH_A = 98,	ANI_SHIELD_ATTACK_A0, ANI_SHIELD_ATTACK_A1, 
				ANI_SHIELD_BREATH_B,		ANI_SHIELD_ATTACK_B0, ANI_SHIELD_ATTACK_B1,		// Shield attacks

				ANI_GREETING0 = 104, ANI_GREETING1, ANI_GREETING2, 
				ANI_WAR_CRY0 = 107, ANI_WAR_CRY1, ANI_WAR_CRY2, ANI_WAR_CRY3, ANI_WAR_CRY4, 

				ANI_SKILL_AXE0 = 112, ANI_SKILL_AXE1, ANI_SKILL_AXE2, ANI_SKILL_AXE3, 
				ANI_SKILL_DAGGER0 = 116, ANI_SKILL_DAGGER1,
				ANI_SKILL_DUAL0 = 118, ANI_SKILL_DUAL1,
				ANI_SKILL_BLUNT0 = 120, ANI_SKILL_BLUNT1, ANI_SKILL_BLUNT2, ANI_SKILL_BLUNT3, 
				ANI_SKILL_POLEARM0 = 124, ANI_SKILL_POLEARM1,
				ANI_SKILL_SPEAR0 = 126, ANI_SKILL_SPEAR1,
				ANI_SKILL_SWORD0 = 128, ANI_SKILL_SWORD1, ANI_SKILL_SWORD2, ANI_SKILL_SWORD3, 
				ANI_SKILL_AXE2H0 = 132, ANI_SKILL_AXE2H1,
				ANI_SKILL_SWORD2H0 = 134, ANI_SKILL_SWORD2H1,

				// From here on: NPC Animation
				ANI_NPC_BREATH = 0, ANI_NPC_WALK, ANI_NPC_RUN, ANI_NPC_WALK_BACKWARD,
				ANI_NPC_ATTACK0 = 4, ANI_NPC_ATTACK1, ANI_NPC_STRUCK0, ANI_NPC_STRUCK1, ANI_NPC_STRUCK2, ANI_NPC_GUARD, 
				ANI_NPC_DEAD0 = 10, ANI_NPC_DEAD1, ANI_NPC_TALK0, ANI_NPC_TALK1, ANI_NPC_TALK2, ANI_NPC_TALK3, 
				ANI_NPC_SPELLMAGIC0 = 16, ANI_NPC_SPELLMAGIC1, 

				ANI_UNKNOWN = 0xffffffff };


// MAX_INCLINE_CLIMB = sqrt(1 - sin(90 - Maximum slope angle)^2)
constexpr float MAX_INCLINE_CLIMB = 0.6430f; // Maximum climbable slope value = 40 degrees

enum e_MoveDirection { MD_STOP, MD_FORWARD, MD_BACKWARD, MD_UNKNOWN = 0xffffffff };

constexpr float MOVE_DELTA_WHEN_RUNNING = 3.0f;	// Movement multiplier for running.
constexpr float MOVE_SPEED_WHEN_WALK = 1.5f;	// Standard player walking speed.

// 현재 상태...
enum e_StateMove {	PSM_STOP = 0,
					PSM_WALK,
					PSM_RUN,
					PSM_WALK_BACKWARD,
					PSM_COUNT };

enum e_StateAction {	PSA_BASIC = 0,		// Idle
						PSA_ATTACK,			// Attacking.
						PSA_GUARD,			// Successfully defended - attack blocked.
						PSA_STRUCK,			// Taking heavy damage.
						PSA_DYING,			// In the process of dying (collapsing)
						PSA_DEATH,			// Dead and lying down/knocked out.
						PSA_SPELLMAGIC,		// Casting a spell.
						PSA_SITDOWN, 		// Sitting down.
						PSA_COUNT }; 

enum e_StateDying {		PSD_DISJOINT = 0,	// Dies with a twisting/rolling death animation. NOTE: The original comment indicated the body physically breaking apart, but this is misleading -- the actual animations for players and NPCs simply twist and roll.
						PSD_KNOCK_DOWN,		// Dies while being knocked back.
						PSD_KEEP_POSITION,	// Dies posing in place.
						PSD_COUNT,

						PSD_UNKNOWN = 0xffffffff };

enum e_StateParty {	PSP_NORMAL = 0,
					PSP_POISONING = 1,
					PSP_CURSED = 2,
					PSP_MAGIC_TAKEN = 4,
					PSP_BLESSED = 8,
					PSP_UNKNOWN = 0xffffffff };

enum e_PartPosition	{	PART_POS_UPPER = 0,
						PART_POS_LOWER,
						PART_POS_FACE,
						PART_POS_HANDS,
						PART_POS_FEET, 
						PART_POS_HAIR_HELMET,
						PART_POS_COUNT,
						PART_POS_UNKNOWN = 0xffffffff };

enum e_PlugPosition {	PLUG_POS_RIGHTHAND = 0,
						PLUG_POS_LEFTHAND, 
						PLUG_POS_BACK, 
						PLUG_POS_KNIGHTS_GRADE, 
						PLUG_POS_COUNT,
						PLUG_POS_UNKNOWN = 0xffffffff };

enum e_ItemAttrib	{
						ITEM_ATTRIB_GENERAL = 0,
						ITEM_ATTRIB_MAGIC	= 1,
						ITEM_ATTRIB_LAIR	= 2,
						ITEM_ATTRIB_CRAFT	= 3,
						ITEM_ATTRIB_UNIQUE	= 4,
						ITEM_ATTRIB_UPGRADE	= 5,
						ITEM_ATTRIB_UNIQUE_REVERSE = 11,
						ITEM_ATTRIB_UPGRADE_REVERSE = 12,
						ITEM_ATTRIB_UNKNOWN = 0xffffffff };	

enum e_ItemClass	{	ITEM_CLASS_DAGGER = 11, // dagger
						ITEM_CLASS_SWORD = 21, // onehandsword
						ITEM_CLASS_SWORD_2H = 22, // 3 : twohandsword
						ITEM_CLASS_AXE = 31, // onehandaxe
						ITEM_CLASS_AXE_2H = 32, // twohandaxe
						ITEM_CLASS_MACE = 41, // mace
						ITEM_CLASS_MACE_2H = 42, // twohandmace
						ITEM_CLASS_SPEAR = 51, // spear
						ITEM_CLASS_POLEARM = 52, // polearm
						
						ITEM_CLASS_SHIELD = 60, // shield

						ITEM_CLASS_BOW = 70, //  Shortbow
						ITEM_CLASS_BOW_CROSS = 71, // crossbow
						ITEM_CLASS_BOW_LONG = 80, // longbow

						ITEM_CLASS_EARRING = 91, // Earring
						ITEM_CLASS_AMULET = 92, // Necklace
						ITEM_CLASS_RING = 93, // Ring
						ITEM_CLASS_BELT = 94, // Belt
						ITEM_CLASS_CHARM = 95, // Items carried in inventory
						ITEM_CLASS_JEWEL = 96, // Jewels/gems
						ITEM_CLASS_POTION = 97, // Potion / consumable
						ITEM_CLASS_SCROLL = 98, // Scroll

						ITEM_CLASS_LAUNCHER = 100, // Item used when throwing a spear.
						
						ITEM_CLASS_STAFF = 110, // Staff
						ITEM_CLASS_ARROW = 120, // Arrow
						ITEM_CLASS_JAVELIN = 130, // Javelin
						
						ITEM_CLASS_ARMOR_WARRIOR = 210, // Warrior armor
						ITEM_CLASS_ARMOR_ROGUE = 220, // Rogue armor
						ITEM_CLASS_ARMOR_MAGE = 230, // Mage armor
						ITEM_CLASS_ARMOR_PRIEST = 240, // Priest armor

						ITEM_CLASS_ETC = 251, // Miscellaneous
						ITEM_CLASS_CONSUMABLE = 255, // Consumable items with 'charges' that use the durability/duration instead of stacks

						ITEM_CLASS_UNKNOWN = 0xffffffff }; // 

enum e_Nation { NATION_NOTSELECTED = 0, NATION_KARUS, NATION_ELMORAD, NATION_UNKNOWN = 0xffffffff };

struct __TABLE_ITEM_BASIC;
struct __TABLE_ITEM_EXT;
struct __TABLE_PLAYER;

struct __InfoPlayerOther
{
	int			iFace;			// Face type
	int			iHair;			// Hair type

	int			iCity;			// Affiliated city
	std::string szKnights;		// Clan name
	int			iKnightsGrade;	// Clan grade
	int			iKnightsRank;	// Clan ranking

	int			iRank;			// Noble rank - used to identify high-ranking titles like King [1], Senator [2].
	int			iTitle;			// Bitmask representing various titles/roles including:
								// Clan Leader, Clan Assistant, Castle Lord, Feudal Lord, King, Emperor, Party leader, Solo player

	void Init()
	{
		iFace = 0;
		iHair = 0;
		iCity;
		szKnights.clear();
		iKnightsGrade = 0;
		iKnightsRank = 0;
		iTitle = 0;
	}
};

// Clan member position/role/duty
enum e_KnightsDuty {	KNIGHTS_DUTY_UNKNOWN = 0,		// Unknown - probably kicked out.
						KNIGHTS_DUTY_CHIEF = 1,			// Clan Leader
						KNIGHTS_DUTY_VICECHIEF = 2,		// Assistant
						KNIGHTS_DUTY_PUNISH = 3,		// Under punishment.
						KNIGHTS_DUTY_TRAINEE = 4,		// Trainee/apprentice
						KNIGHTS_DUTY_KNIGHT = 5,		// Regular member
						KNIGHTS_DUTY_OFFICER = 6		// Officer
					};

constexpr int VICTORY_ABSENCE	= 0;
constexpr int VICTORY_KARUS		= 1;
constexpr int VICTORY_ELMORAD	= 2;

struct __InfoPlayerMySelf : public __InfoPlayerOther
{
	int					iBonusPointRemain;		// Bonus points remaining to assign
	int					iLevelPrev;				// Previous level

	int					iMSPMax; 
	int					iMSP; 
			
	int					iTargetHPPercent;
	int					iGold;
	int64_t				iExpNext;
	int64_t				iExp;
	int					iRealmPoint;			// National Points
	int					iRealmPointMonthly;		// Monthly National Points
	e_KnightsDuty		eKnightsDuty;			// Clan member position/role/duty
	int					iWeightMax;				// Max weight
	int					iWeight;				// Current weight
	int					iStrength;				// Strength
	int					iStrength_Delta;		// Bonus strength
	int					iStamina;				// Stamina
	int					iStamina_Delta;			// Bonus stamina
	int					iDexterity;				// Dexterity
	int					iDexterity_Delta;		// Bonus dexterity
	int					iIntelligence;			// Intelligence
	int					iIntelligence_Delta;	// Bonus intelligence
	int 				iMagicAttak;			// Charisma/Magic Power
	int 				iMagicAttak_Delta;		// Bonus Charisma/Magic Power
	
	int 				iAttack;				// Attack Power
	int 				iAttack_Delta;			// Bonus Attack Power
	int 				iGuard;					// Defense
	int 				iGuard_Delta;			// Bonus Defense

	int 				iRegistFire;			// Fire resistance
	int 				iRegistFire_Delta;		// Bonus fire resistance
	int 				iRegistCold;			// Cold resistance
	int 				iRegistCold_Delta;		// Bonus cold resistance
	int 				iRegistLight;			// Lightning resistance
	int 				iRegistLight_Delta;		// Bonus lightning resistance
	int 				iRegistMagic;			// Magic resistance
	int 				iRegistMagic_Delta;		// Bonus magic resistance
	int 				iRegistCurse;			// Curse resistance
	int 				iRegistCurse_Delta;		// Bonus curse resistance
	int 				iRegistPoison;			// Poison resistance
	int 				iRegistPoison_Delta;	// Bonus poison resistance

	int					iZoneInit;				// Initial Zone ID received from the server
	int					iZoneCur;				// Current zone ID
	int					iVictoryNation;			// Last war outcome - 0: Draw, 1: El Morad victory, 2: Karus victory

	e_ZoneAbilityType	eZoneAbilityType;
	bool				bCanTradeWithOtherNation;
	bool				bCanTalkToOtherNation;
	int16_t				sZoneTariff;

	void Init()
	{
		__InfoPlayerOther::Init();

		iBonusPointRemain = 0;
		iLevelPrev = 0;

		iMSPMax = 0;
		iMSP = 0;

		iTargetHPPercent = 0;
		iGold = 0;
		iExpNext = 0;
		iExp = 0;
		iRealmPoint = 0;
		iRealmPointMonthly = 0;
		eKnightsDuty = KNIGHTS_DUTY_UNKNOWN;
		iWeightMax = 0;
		iWeight = 0;
		iStrength = 0;
		iStrength_Delta = 0;
		iStamina = 0;
		iStamina_Delta = 0;
		iDexterity = 0;
		iDexterity_Delta = 0;
		iIntelligence = 0;
		iIntelligence_Delta = 0;
		iMagicAttak = 0;
		iMagicAttak_Delta = 0;

		iAttack = 0;
		iAttack_Delta = 0;
		iGuard = 0;
		iGuard_Delta = 0;

		iRegistFire = 0;
		iRegistFire_Delta = 0;
		iRegistCold = 0;
		iRegistCold_Delta = 0;
		iRegistLight = 0;
		iRegistLight_Delta = 0;
		iRegistMagic = 0;
		iRegistMagic_Delta = 0;
		iRegistCurse = 0;
		iRegistCurse_Delta = 0;
		iRegistPoison = 0;
		iRegistPoison_Delta = 0;

		iZoneInit = 1;
		iZoneCur = 0;
		iVictoryNation = -1;

		eZoneAbilityType = ZONE_ABILITY_PVP;
		bCanTradeWithOtherNation = false;
		bCanTalkToOtherNation = false;
	}
};

constexpr int MAX_PARTY_OR_FORCE = 8;

struct __InfoPartyOrForce
{
	int			iID;				// Player's ID
	int			iLevel;				// Level
	e_Class		eClass;				// Class
	int			iHP;				// Hit Points
	int			iHPMax;				// Max Hit Points
	int			iMP;				// Mana Points
	int			iMPMax;				// Max Mana Points
	bool		bSufferDown_HP;		// Status - HP debuffed.
	bool		bSufferDown_Etc;	// Status - Cursed.
	std::string szID;				// Player's name

	void Init()
	{
		iID = -1;
		iLevel = 0;
		eClass = CLASS_UNKNOWN;
		iHP = 0;
		iHPMax = 0;
		iMP = 0;
		iMPMax = 0;
		szID.clear();

		bSufferDown_HP = false;			
		bSufferDown_Etc = false;		
	}

	__InfoPartyOrForce()
	{
		Init();
	}
};

enum e_PartyStatus { PARTY_STATUS_DOWN_HP = 1, PARTY_STATUS_DOWN_ETC = 2 };

// Seeking party board entry
struct __InfoPartyBBS
{
	std::string szID;			// Player's name
	int			iID;			// Player's ID
	int			iLevel;			// Level
	e_Class		eClass;			// Class
	int			iMemberCount;

	void Init()
	{
		szID.clear();
		iID = -1;
		iLevel = 0;
		eClass = CLASS_UNKNOWN;
		iMemberCount = 0;
	}

	__InfoPartyBBS()
	{
		Init();
	}
};

struct __TABLE_TEXTS
{
	uint32_t	dwID;
	std::string	szText;
};

struct __TABLE_ZONE
{
	uint32_t	dwID;					// 01 Zone ID
	std::string	szTerrainFN;			// 02 GTD
	std::string	szName;					// 03	
	std::string	szColorMapFN;			// 04 TCT
	std::string	szLightMapFN;			// 05 TLT
	std::string	szObjectPostDataFN;		// 06 OPD
	std::string	szOpdExtFN;				// 07 OPDEXT
	std::string	szMiniMapFN;			// 08 DXT
	std::string	szSkySetting;			// 09 N3Sky
	int			bIndicateEnemyPlayer;	// 10 Int32 (BOOL)
	int			iFixedSundDirection;	// 11 Int32
	std::string	szLightObjFN;			// 12 GLO
	std::string	szGevFN;				// 13 GEV
	int			iIdk0;					// 14 idk
	std::string	szEnsFN;				// 15 ENS
	float		fIdk1;					// 16 idk
	std::string	szFlagFN;				// 17 FLAG
	uint32_t	iIdk2;					// 18	
	uint32_t	iIdk3;					// 19	
	uint32_t	iIdk4;					// 20	
	uint32_t	iIdk5;					// 21
	std::string	szOpdSubFN;				// 22 OPDSUB
	int			iIdk6;					// 23
	std::string	szEvtSub;				// 24 EVTSUB
};

struct __TABLE_UI_RESRC
{
	uint32_t dwID;						// 01 (Karus/Human)
	std::string szLogIn;				// 02
	std::string szCmd;					// 03
	std::string szChat;					// 04
	std::string szMsgOutput;			// 05
	std::string szStateBar;				// 06
	std::string szVarious;				// 07
	std::string szState;				// 08
	std::string szKnights;				// 09
	std::string szQuest;				// 10
	std::string szFriends;				// 11 
	std::string szInventory;			// 12
	std::string szTransaction;			// 13
	std::string szDroppedItem;			// 14
	std::string szTargetBar;			// 15
	std::string szTargetSymbolShape;	// 16
	std::string szSkillTree;			// 17
	std::string szHotKey;				// 18
	std::string szMiniMap;				// 19
	std::string szPartyOrForce;			// 20
	std::string szPartyBBS;				// 21
	std::string szHelp;					// 22
	std::string szNotice;				// 23
	std::string szCharacterCreate;		// 24
	std::string szCharacterSelect;		// 25
	std::string szToolTip;				// 26
	std::string szMessageBox;			// 27
	std::string szLoading;				// 28
	std::string szItemInfo;				// 29
	std::string szPersonalTrade;		// 30
	std::string szPersonalTradeEdit;	// 31
	std::string szNpcEvent;				// 32
	std::string szZoneChangeOrWarp;		// 33
	std::string szExchangeRepair;		// 34
	std::string szRepairTooltip;		// 35
	std::string szNpcTalk;				// 36
	std::string szNpcExchangeList;		// 37
	std::string szKnightsOperation;		// 38
	std::string szClassChange;			// 39
	std::string szEndingDisplay;		// 40
	std::string szWareHouse;			// 41
	std::string szChangeClassInit;		// 42
	std::string szChangeInitBill;		// 43
	std::string szInn;					// 44
	std::string szInputClanName;		// 45
	std::string szTradeBBS;				// 46
	std::string szTradeBBSSelector;		// 47
	std::string szTradeExplanation;		// 48
	std::string szTradeMemolist;		// 49
	std::string szQuestMenu;			// 50
	std::string szQuestTalk;			// 51
	std::string szQuestEdit;			// 52
	std::string szDead;					// 53
	std::string szElLoading;			// 54
	std::string szKaLoading;			// 55
	std::string szNationSelect;			// 56
	std::string szChat2;				// 57
	std::string szMsgOutput2;			// 58
	std::string szItemUpgrade;			// 59
	std::string szDuelCreate;			// 60
	std::string szDuelList;				// 61
	std::string szDuelMsg;				// 62
	std::string szDuelMsgEdit;			// 63
	std::string szDuelLobby;			// 64
	std::string szQuestContent;			// 65
	std::string szDuelItemCnt;			// 66
	std::string szTradeInv;				// 67
	std::string szTradeBuyInv;			// 68
	std::string szTradeItemDisplay;		// 69
	std::string szTradePrice;			// 70
	std::string szTradeCnt;				// 71
	std::string szTradeMsgBox;			// 72
	std::string szClanPage;				// 73
	std::string szAllyPage;				// 74
	std::string szAlly2Page;			// 75
	std::string szCmdList;				// 76
	std::string szCmdEdit;				// 77
	std::string szClanLogo;				// 78
	std::string szShopMall;				// 79
	std::string szLvlGuide;				// 80
	std::string szCSWNpc;				// 81
	std::string szKCSWPetition;			// 82
	std::string szCSWAlly;				// 83
	std::string szCSWSchedule;			// 84
	std::string szExitMenu;				// 85
	std::string szResurrect;			// 86
	std::string szNameChange;			// 87
	std::string szNameEditBox;			// 88
	std::string szNameCheck;			// 89
	std::string szCSWAdmin;				// 90
	std::string szCSWTax;				// 91
	std::string szCSWCapeList;			// 92
	std::string szKnightCapeShop;		// 93
	std::string szCSWTaxCollection;		// 94
	std::string szCSWTaxRate;			// 95
	std::string szCSWTaxRateMsg;		// 96
	std::string szCatapult;				// 97
	std::string szDisguiseRing;			// 98
	std::string szMsgBoxOk;				// 99
	std::string szMsgBoxOkCancel;		// 100
	std::string szOpenChat;				// 101
	std::string szCloseChat;			// 102
	std::string szChrClanLogo;			// 103
	std::string szWarning;				// 104
	std::string szConvo;				// 105
	std::string szBlog;					// 106
	std::string szInnPass;				// 107
	std::string szNoviceTips;			// 108
	std::string szWebpage;				// 109
	std::string szPartyMsgBox;			// 110
	std::string szClanLogo2;			// 111
	std::string szRentalNpc;			// 112
	std::string szRentalTransaction;	// 113
	std::string szRentalEntry;			// 114
	std::string szRentalItem;			// 115
	std::string szRentalMsg;			// 116
	std::string szRentalCnt;			// 117
	std::string szNetDIO;				// 118
	std::string szLoginIntro;			// 119
	std::string szSubLoginIntro;		// 120
	std::string szCharSelect;			// 121
	std::string szCharCreate;			// 122
	std::string szOtherState;			// 123
	std::string szPPCardBegin;			// 124
	std::string szPPCardList;			// 125
	std::string szPPCardReg;			// 126
	std::string szPPCardMsg;			// 127
	std::string szPPCardBuyList;		// 128
	std::string szPPCardMyInfo;			// 129
	std::string szNationSelectNew;		// 130
	std::string szUSALogo;				// 131
	std::string szMonster;				// 132
	std::string szNationTaxNPC;			// 133
	std::string szNationTaxRate;		// 134
	std::string szKingMsgBoxOk;			// 135
	std::string szKingMsgBoxOkCancel;	// 136
	std::string szKingElectionBoard;	// 137
	std::string szKingElectionList;		// 138
	std::string szKingElectionMain;		// 139
	std::string szKingNominate;			// 140
	std::string szKingRegister;			// 141
	std::string szUpgradeRing;			// 142
	std::string szUpgradeSelect;		// 143
	std::string szTradeMsg;				// 144
	std::string szShowIcon;				// 145
};

struct __TABLE_ITEM_BASIC
{
	uint32_t	dwID;					// 01 Encoded item number: first 2 digits = item type, next 2 digits = equip position (used to determine Plugs or Parts), last 4 digits = item index

	uint8_t		byExtIndex;				// 02 Extension index (i.e. Item_Ext_<extension index>.tbl)
	std::string	szName;					// 03 Name
	std::string	szRemark;				// 04 Item Description

	uint32_t	dwIDK0;					// 05
	uint8_t		byIDK1;					// 06

	uint32_t	dwIDResrc;				// 07 Encoded resource ID
	uint32_t	dwIDIcon;				// 08 Encoded icon resource ID
	uint32_t	dwSoundID0;				// 09 Sound ID - set to 0 for no sound
	uint32_t	dwSoundID1;				// 10 Sound ID - set to 0 for no sound

	uint8_t		byClass;				// 11 Item type — see e_ItemClass enum for reference.
	uint8_t		byIsRobeType;			// 12 Robe-type item that replaces both upper and lower equipment slots, showing only this.
	uint8_t		byAttachPoint;			// 13 Equip position — identifies the specific slot on the character's body where this item is equipped
	uint8_t		byNeedRace;				// 14 Required race
	uint8_t		byNeedClass;			// 15 Required class

	int16_t		siDamage;				// 16 Weapon damage
	int16_t		siAttackInterval;		// 17 Attack speed (100 units = 1 second)
	int16_t		siAttackRange;			// 18 Effective attack range (in 0.1 meter units)
	int16_t		siWeight;				// 19 Weight (in 0.1 units)
	int16_t		siMaxDurability;		// 20 Max durability
	int			iPrice;					// 21 Purchase price
	int			iSaleType;				// 22 Sale type (see e_ItemSaleType)
	int16_t		siDefense;				// 23 Defense
	uint8_t		byContable;				// 24 Is the item countable/stackable?

	uint32_t	dwEffectID1;			// 25 Magic effect ID 1
	uint32_t	dwEffectID2;			// 26 Magic effect ID 2

	char		cNeedLevel;				// 27 Required level — player's iLevel (can be negative)

	char		cIDK2;					// 28

	uint8_t		byNeedRank;				// 29 Required rank — player's iRank
	uint8_t		byNeedTitle;			// 30 Required title — player's iTitle
	uint8_t		byNeedStrength;			// 31 Required strength — player's iStrength
	uint8_t		byNeedStamina;			// 32 Required stamina — player's iStamina
	uint8_t		byNeedDexterity;		// 33 Required dexterity — player's iDexterity
	uint8_t		byNeedInteli;			// 34 Required intelligence — player's iIntelligence
	uint8_t		byNeedMagicAttack;		// 35 Required charisma/magic power — player's iMagicAttack

	uint8_t		bySellGroup;			// 36 Selling group associated with vendor NPC

	uint8_t		byIDK3;					// 37
};

constexpr int MAX_ITEM_EXTENSION	= 24; // Number of item extension tables. (Item_Ext_0..23.tbl is a total of 24)
constexpr int LIMIT_FX_DAMAGE		= 64;
constexpr int ITEM_LIMITED_EXHAUST	= 17;

struct __TABLE_ITEM_EXT
{
	uint32_t	dwID;						// 01 Encoded item number: first 2 digits = item type, next 2 digits = equip position (used to determine Plugs or Parts), last 4 digits = item index
	std::string	szHeader;					// 02 Name prefix

	uint32_t	dwBaseID;					// 03

	std::string	szRemark;					// 04 Item description

	uint32_t	dwIDK0;						// 05 TODO: will need to implement this one
	uint32_t	dwIDResrc;					// 06
	uint32_t	dwIDIcon;					// 07

	uint8_t		byMagicOrRare;				// 08 Item attribute (see e_ItemAttrib enum). Is it a magic, rare item, etc.

	int16_t		siDamage;					// 09 Weapon damage
	int16_t		siAttackIntervalPercentage;	// 10 Attack speed (percentage: 100% = normal speed)
	int16_t		siHitRate;					// 11 Hit rate/accuracy (percentage modifier: 20% = 120% chance to hit)
	int16_t		siEvationRate;				// 12 Evasion rate/dodge (percentage modifier: 20% = 120% chance to dodge)

	int16_t		siMaxDurability;			// 13 Maximum durability
	int16_t		siPriceMultiply;			// 14 Purchase price multiplier
	int16_t		siDefense;					// 15 Defense

	int16_t		siDefenseRateDagger;		// 16 Defense against daggers (percentage modifier: 20% = 120% defense)
	int16_t		siDefenseRateSword;			// 17 Defense against swords (percentage modifier: 20% = 120% defense)
	int16_t		siDefenseRateBlow;			// 18 Defense against blunt weapons [maces/clubs] (percentage modifier: 20% = 120% defense)
	int16_t		siDefenseRateAxe;			// 19 Defense against axes (percentage modifier: 20% = 120% defense)
	int16_t		siDefenseRateSpear;			// 20 Defense against spears (percentage modifier: 20% = 120% defense)
	int16_t		siDefenseRateArrow;			// 21 Defense against arrows (percentage modifier: 20% = 120% defense)

	uint8_t		byDamageFire;				// 22 Bonus fire damage
	uint8_t		byDamageIce;				// 23 Bonus ice damage
	uint8_t		byDamageThuner;				// 24 Bonus thunder damage
	uint8_t		byDamagePoison;				// 25 Bonus poison damage

	uint8_t		byStillHP;					// 26 HP drain ("still HP = steal HP")
	uint8_t		byDamageMP;					// 27 MP damage
	uint8_t		byStillMP;					// 28 MP drain
	uint8_t		byReturnPhysicalDamage;		// 29 Physical damage reflection

	uint8_t		bySoulBind;					// 30 Soul bind — percentage chance of dropping this item upon death in one-on-one combat; not currently in use.

	int16_t		siBonusStr;					// 31 Bonus strength
	int16_t		siBonusSta;					// 32 Bonus stamina
	int16_t		siBonusDex;					// 33 Bonus dexterity
	int16_t		siBonusInt;					// 34 Bonus intelligence
	int16_t		siBonusMagicAttak;			// 35 Bonus charisma/magic power
	int16_t		siBonusHP;					// 36 Bonus HP
	int16_t		siBonusMSP;					// 37 Bonus MSP

	int16_t		siRegistFire;				// 38 Fire damage resistance
	int16_t		siRegistIce;				// 39 Ice damage resistance
	int16_t		siRegistElec;				// 40 Electric damage resistance
	int16_t		siRegistMagic;				// 41 Magic damage resistance
	int16_t		siRegistPoison;				// 42 Poison damage resistance
	int16_t		siRegistCurse;				// 43 Curse damage resistance

	uint32_t	dwEffectID1;				// 44 Magic effect ID 1
	uint32_t	dwEffectID2;				// 45 Magic effect ID 2

	int16_t		siNeedLevel;				// 46 Required level (player's iLevel)
	int16_t		siNeedRank;					// 47 Required rank (player's iRank)
	int16_t		siNeedTitle;				// 48 Required title (player's iTitle)
	int16_t		siNeedStrength;				// 49 Required strength
	int16_t		siNeedStamina;				// 50 Required Stamina
	int16_t		siNeedDexterity;			// 51 Required Dexterity
	int16_t		siNeedInteli;				// 52 Required Intelligence
	int16_t		siNeedMagicAttack;			// 53 Required Charisma/Magic power
};

constexpr int MAX_NPC_SHOP_ITEM = 30;
struct __TABLE_NPC_SHOP
{
	uint32_t	dwNPCID;
	std::string	szName;
	uint32_t	dwItems[MAX_NPC_SHOP_ITEM];
};

enum e_ItemType { ITEM_TYPE_PLUG = 1, ITEM_TYPE_PART, ITEM_TYPE_ICONONLY, ITEM_TYPE_GOLD = 9, ITEM_TYPE_SONGPYUN = 10, ITEM_TYPE_UNKNOWN = 0xffffffff };

enum e_ItemPosition {	ITEM_POS_DUAL = 0,	ITEM_POS_RIGHTHAND, ITEM_POS_LEFTHAND,	ITEM_POS_TWOHANDRIGHT,	ITEM_POS_TWOHANDLEFT,
						ITEM_POS_UPPER = 5, ITEM_POS_LOWER,		ITEM_POS_HEAD,		ITEM_POS_GLOVES,		ITEM_POS_SHOES,
						ITEM_POS_EAR = 10,	ITEM_POS_NECK,		ITEM_POS_FINGER,	ITEM_POS_SHOULDER,		ITEM_POS_BELT,
						ITEM_POS_INVENTORY = 15, ITEM_POS_GOLD = 16, ITEM_POS_SONGPYUN = 17,
						ITEM_POS_UNKNOWN = 0xffffffff };
					
enum e_ItemSlot {	ITEM_SLOT_EAR_RIGHT = 0,	ITEM_SLOT_HEAD	= 1,	ITEM_SLOT_EAR_LEFT	= 2,
					ITEM_SLOT_NECK = 3,			ITEM_SLOT_UPPER	= 4,	ITEM_SLOT_SHOULDER	= 5,
					ITEM_SLOT_HAND_RIGHT = 6,	ITEM_SLOT_BELT	= 7,	ITEM_SLOT_HAND_LEFT = 8,
					ITEM_SLOT_RING_RIGHT = 9,	ITEM_SLOT_LOWER = 10,	ITEM_SLOT_RING_LEFT = 11,
					ITEM_SLOT_GLOVES = 12,		ITEM_SLOT_SHOES = 13, 
					ITEM_SLOT_COUNT = 14, ITEM_SLOT_UNKNOWN = 0xffffffff };

// Manages NPC/mob/player appearance
struct __TABLE_PLAYER_LOOKS
{
	uint32_t	dwID;			// NPC resource ID
	std::string	szName;			// Model name
	std::string	szJointFN;		// Joint filename
	std::string	szAniFN;		// Animation filename
	std::string	szPartFNs[10];	// Each character part — upper body, lower body, head, arms, legs, hair, cape
	std::string	szSkinFN;
	std::string	szChrFN;
	std::string	szFXPlugFN;

	int			iIdk1;

	int			iJointRH;		// Joint index for tip of right hand
	int			iJointLH;		// Joint index for tip of left hand
	int			iJointLH2;		// Joint index for left forearm
	int			iJointCloak;	// Joint index for cape attachment

	int			iSndID_Move;
	int			iSndID_Attack0;
	int			iSndID_Attack1;
	int			iSndID_Struck0;
	int			iSndID_Struck1;
	int			iSndID_Dead0;
	int			iSndID_Dead1;
	int			iSndID_Breathe0;
	int			iSndID_Breathe1;
	int			iSndID_Reserved0;
	int			iSndID_Reserved1;

	int			iIdk2;
	int			iIdk3;
	uint8_t		byIdk4;
	uint8_t		byIdk5;
	uint8_t		byIdk6;
};

struct __TABLE_EXCHANGE_QUEST
{
	uint32_t	dwID;				// 01 Quest ID
	uint32_t	dwNpcNum;			// 02 NPC ID
	std::string	szDesc;				// 03 Description
	int			iCondition0;		// 04 Condition 1
	int			iCondition1;		// 05 Condition 2
	int			iCondition2;		// 06 Condition 3
	int			iCondition3;		// 07 Condition 4
	int			iNeedGold;			// 08 Required Gold
	uint8_t		bNeedLevel;			// 09 Required Level
	uint8_t		bNeedClass;			// 10 Required Class
	uint8_t		bNeedRank;			// 11 Required Rank
	uint8_t		bNeedExtra1;		// 12 Required Extra 1
	uint8_t		bNeedExtra2;		// 13 Required Extra 2
	uint8_t		bCreatePercentage;	// 14 Spawn chance (%)
	int			iArkTuarek;			// 15 Arch Tuarek
	int			iTuarek;			// 16 Tuarek
	int			iRinkleTuarek;		// 17 Wrinkle Tuarek
	int			iBabarian;			// 18 Barbarian
	int			iMan;				// 19 Man
	int			iWoman;				// 20 Woman
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Magic Table

struct __TABLE_UPC_SKILL
{
	uint32_t	dwID;				// 01 Skill ID
	std::string	szEngName;			// 02 English name
	std::string	szName;				// 03 Korean name
	std::string	szDesc;				// 04 Description
	int			iSelfAnimID1;		// 05 Start animation (caster)
	int			iSelfAnimID2;		// 06 End animation (caster)

	int			idwTargetAnimID;	// 07 Target animation
	int			iSelfFX1;			// 08 Effect on caster (1)
	int			iSelfPart1;			// 09 Effect position for iSelfFX1
	int			iSelfFX2;			// 10 Effect on caster (2)
	int			iSelfPart2;			// 11 Effect position for iSelfFX2
	int			iFlyingFX;			// 12 Flying effect
	int			iTargetFX;			// 13 Target effect
	int			iTargetPart;		// 14 Effect position for iTargetFX

	int			iTarget;			// 15 Target type/"moral"
	int			iNeedLevel;			// 16 Required player level
	int			iNeedSkill;			// 17 Required skill

	int			iExhaustMSP;		// 18 MSP consumed
	int			iExhaustHP;			// 19 HP consumed

	uint32_t	dwNeedItem;			// 20 Required item (refer to e_ItemClass enum - divide value by 10)
	uint32_t	dwExhaustItem;		// 21 Item consumed
	int			iCastTime;			// 22 Cast time
	int			iReCastTime;		// 23 Cooldown time

	float		fIDK0;				// 24 TODO: will need to implement this...?
	float		fIDK1;				// 25 1298 (unknown purpose)

	int			iPercentSuccess;	// 26 Success rate
	uint32_t	dw1stTableType;		// 27 Primary skill type
	uint32_t	dw2ndTableType;		// 28 Secondary skill type
	int			iValidDist;			// 29 Effective skill range

	int			iIDK2;				// 30 1298 (unknown purpose)
};

struct __TABLE_UPC_SKILL_TYPE_1
{
	uint32_t	dwID;				// 01 Skill ID
	int			iSuccessType;		// 02 Success type
	int			iSuccessRatio;		// 03 Success ratio (%)
	int			iPower;				// 04 Attack power
	int			iDelay;				// 05 Skill delay (time before next action)
	int			iComboType;			// 06 Combo type
	int			iNumCombo;			// 07 Number of hits in combo
	int			iComboDamage;		// 08 Damage per combo hit
	int			iValidAngle;		// 09 Attack radius
	int			iAct[3];			// 10
};

struct __TABLE_UPC_SKILL_TYPE_2
{
	uint32_t	dwID;				// 01 Skill ID
	int			iSuccessType;		// 02 Success type
	int			iPower;				// 03 Attack power
	int			iAddDamage;			// 04 Bonus damage
	int			iAddDist;			// 05 Distance increase
	int			iNumArrow;			// 06 Number of arrows used
};

struct __TABLE_UPC_SKILL_TYPE_3
{
	uint32_t	dwID;				// 01 Skill ID
	int			iRadius;			// 02 Skill radius
	int			iDDType;			// 03 Is this a DoT or a HoT
	int			iStartDamage;		// 04 Initial damage
	int			iDuraDamage;		// 05 Duration damage (e.g. DoT or HoT tick damage)
	int			iDurationTime;		// 06 Effect duration (in seconds)
	int			iAttribute;			// 07 Elemental type
};

struct __TABLE_UPC_SKILL_TYPE_4
{
	uint32_t	dwID;				// 01 Skill ID

	int			iBuffType;			// 02 Buff type
	int			iRadius;			// 03 Buff radius
	int			iDuration;			// 04 Buff duration
	int			iAttackSpeed;		// 05 Attack speed percentage (100% = base attack speed)
	int			iMoveSpeed;			// 06 Movement speed percentage (100% = base movement speed)
	int			iAC;				// 07 Flat defense modifier; mutually exclusive with iACPct.
	int			iACPct;				// 08 Defense percentage (100% = base defense); mutually exclusive with iAC.
	int			iAttack;			// 09 Attack power percentage (100% = base attack power)
	int			iMagicAttack;		// 10 Magic attack power percentage (100% = base magic attack power)
	int			iMaxHP;				// 11 Flat maximum HP modifier; mutually exclusive with iMaxHPPct.
	int			iMaxHPPct;			// 12 Maximum HP percentage (100% = base maximum HP); mutually exclusive with iMaxHP.
	int			iMaxMP;				// 13 Flat maximum MP modifier; mutually exclusive with iMaxMPPct.
	int			iMaxMPPct;			// 14 Maximum MP percentage (100% = base maximum MP); mutually exclusive with iMaxMP.
	int			iStr;				// 15 Flat strength modifier
	int			iSta;				// 16 Flat stamina modifier
	int			iDex;				// 17 Flat dexterity modifier
	int			iInt;				// 18 Flat intelligence modifier
	int			iMAP;				// 19 Flat charisma/magic power modifier
	int			iFireResist;		// 20 Flat fire resistance modifier
	int			iColdResist;		// 21 Flat cold resistance modifier
	int			iLightningResist;	// 22 Flat lightning resistance modifier
	int			iMagicResist;		// 23 Flat magic resistance modifier
	int			iDeseaseResist;		// 24 Flat disease/curse resistance modifier
	int			iPoisonResist;		// 25 Flat poison resistance modifier

	int			iExpPct;			// 26 Experience gain percentage (100% = base experience gain)
};

struct __TABLE_UPC_SKILL_TYPE_6
{
	uint32_t		dwID;				// 01 Skill ID
	std::string		szEngName;			// 02 Transformation name (English)
	std::string		szName;				// 03 Transformation name (Korean)
	int32_t			iSize;				// 04 Size (%)
	int32_t			iTransformID;		// 05 Model ID
	int32_t			iDuration;			// 06 Duration (in seconds)
	int32_t			iMaxHP;				// 07 Flat max HP - 0 if unused
	int32_t			iMaxMP;				// 08 Flat max MP - 0 if unused
	int32_t			iSpeed;				// 09 Movement speed - 0 if unused
	int32_t			iAttackSpeed;		// 10 Attack speed - 0 if unused
	int32_t			iAttack;			// 11 Attack damage - 0 if unused
	int32_t			iAC;				// 12 Defense - 0 if unused
	int32_t			iHitRate;			// 13 Hit rate (accuracy) - 0 if unused
	int32_t			iEvasionRate;		// 14 Evasion rate (dodge) - 0 if unused
	int32_t			iFireResist;		// 15 Flat fire resistance modifier
	int32_t			iColdResist;		// 16 Flat cold resistance modifier
	int32_t			iLightningResist;	// 17 Flat lightning resistance modifier
	int32_t			iMagicResist;		// 18 Flat magic resistance modifier
	int32_t			iCurseResist;		// 19 Flat disease/curse resistance modifier
	int32_t			iPoisonResist;		// 20 Flat poison resistance modifier
	uint8_t			byNeedItem;			// 21 Item type required for transformation
	uint32_t		dwClass;			// 22 Classes allowed for transformation
	uint32_t		dwUserSkillUse;		// 23
	uint32_t		dwSkillSuccessRate;	// 24 NOTE: These columns may be shuffled slightly, the naming is based on the server data
	uint32_t		dwMonsterFriendly;	// 25
	uint8_t			byNation;			// 26
	uint32_t		dwRightHand;		// 27 
	uint32_t		dwLeftHand;			// 28
};

struct __TABLE_UPC_SKILL_TYPE_7
{
	uint32_t	dwID;					// 01 Skill ID
	int32_t		iRadius;				// 02 Radius
};

struct __TABLE_UPC_SKILL_TYPE_9
{
	uint32_t		dwID;				// 01 ID
	// TODO: Fill out this struct
};

// Magic Table
///////////////////////////////////////////////////////////////////////////////////////////////////////////

struct __TABLE_QUEST_MENU
{
	uint32_t	dwID;		// 01 ID
	std::string szMenu;		// 02 Menu text
};

struct __TABLE_QUEST_TALK
{
	uint32_t	dwID;		// 01 ID
	std::string szTalk;		// 02 Dialogue text
};

struct __TABLE_QUEST_CONTENT
{
	uint32_t		dwID;
	int				iReqLevel;
	int				iReqClass;
	std::string		szName;
	std::string		szDesc;
	std::string		szReward;
};

struct __TABLE_HELP
{
	DWORD		dwID;
	int			iMinLevel;
	int			iMaxLevel;
	int			iReqClass;
	std::string	szQuestName;
	std::string	szQuestDesc;
};

constexpr int MAX_ITEM_SLOT_OPC				= 8;	// Max equipment slots for other players (including NPCs): 0-4 = upper body, lower body, helmet, arms, legs; 5 = cloak; 6 = right hand; 7 = left hand

constexpr int MAX_ITEM_INVENTORY			= 28;	// Max items a player can hold in their inventory
constexpr int MAX_ITEM_TRADE				= 24;	// Max items per page in NPC trades
constexpr int MAX_ITEM_TRADE_PAGE			= 12;
constexpr int MAX_ITEM_WARE_PAGE			= 8;
constexpr int MAX_ITEM_PER_TRADE			= 12;	// Max items in a player trading window
constexpr int MAX_ITEM_BUNDLE_DROP_PIECE	= 6;
constexpr int MAX_ITEM_EX_RE_NPC			= 4;	// Max items in the (outdated, unused) NPC exchange/repair UI.

constexpr int MAX_SKILL_FROM_SERVER			= 9;	// Max number of skill point slots received from the server.

constexpr int MAX_SKILL_KIND_OF				= 5;	// Total skill types: 1 - base skills, 4 - specialized skills
constexpr int MAX_SKILL_IN_PAGE				= 6;	// Max number of of skill icons per page
constexpr int MAX_SKILL_PAGE_NUM			= 7;	// Max number of pages per skill category

constexpr int MAX_SKILL_HOTKEY_PAGE			= 8;	// Max pages for a skill bar (CUIHotKeyDlg).
constexpr int MAX_SKILL_IN_HOTKEY			= 8;	// Max number of skill icons per page for a skill bar (CUIHotKeyDlg).

constexpr int MAX_AVAILABLE_CHARACTER		= 3;	// Max character slots available per server

// Sound IDs
constexpr int ID_SOUND_ITEM_ETC_IN_INVENTORY	= 2000;
constexpr int ID_SOUND_ITEM_IN_REPAIR			= 2001;
constexpr int ID_SOUND_ITEM_WEAPON_IN_INVENTORY = 2002;
constexpr int ID_SOUND_ITEM_ARMOR_IN_INVENTORY	= 2003;
constexpr int ID_SOUND_GOLD_IN_INVENTORY		= 3000;
constexpr int ID_SOUND_SKILL_THROW_ARROW		= 5500;
constexpr int ID_SOUND_BGM_TOWN					= 20000;
constexpr int ID_SOUND_BGM_KA_BATTLE			= 20002;
constexpr int ID_SOUND_BGM_EL_BATTLE			= 20003;
constexpr int ID_SOUND_CHR_SELECT_ROTATE		= 2501;

constexpr float SOUND_RANGE_TO_SET				= 10.0f;
constexpr float SOUND_RANGE_TO_RELEASE			= 20.0f;

constexpr float STUN_TIME						= 3.0f;

enum e_Behavior {	BEHAVIOR_NOTHING = 0,
					BEHAVIOR_EXIT,						// Exit the game
					BEHAVIOR_RESTART_GAME,				// Return to character selection
					BEHAVIOR_REGENERATION,				// Respawn/revive character
					BEHAVIOR_PERSONAL_TRADE_CANCEL,		// Private trade: Cancel a request (outdated & unused)

					BEHAVIOR_PARTY_PERMIT,				// Accept a party invite from another player.
					BEHAVIOR_PARTY_DISBAND,				// Leave/disband party
					BEHAVIOR_FORCE_PERMIT,				// Accept a force/squad invite from another player
					BEHAVIOR_FORCE_DISBAND,				// Leave/disband force/squad

					BEHAVIOR_REQUEST_BINDPOINT,			// Return to binding point

					BEHAVIOR_DELETE_CHR,

					BEHAVIOR_KNIGHTS_CREATE,
					BEHAVIOR_KNIGHTS_DESTROY,			// Disband clan
					BEHAVIOR_KNIGHTS_WITHDRAW,			// Leave clan

					BEHAVIOR_PERSONAL_TRADE_FMT_WAIT,	// Private trade: Wait for other player to accept our trade request [does nothing].
					BEHAVIOR_PERSONAL_TRADE_PERMIT,		// Private trade: Accept a trade request from another player.

					BEHAVIOR_MGAME_LOGIN,
					
					BEHAVIOR_CLAN_JOIN,
					BEHAVIOR_PARTY_BBS_REGISTER,		// Register on party bulletin board (i.e. seeking party board)
					BEHAVIOR_PARTY_BBS_REGISTER_CANCEL, // Unregister from party bulletin board (i.e. seeking party board)

					BEHAVIOR_EXECUTE_OPTION,			// Exit game and open options.
				
					BEHAVIOR_UNKNOWN = 0xffffffff
				};

enum e_SkillMagicTaget	{	SKILLMAGIC_TARGET_SELF = 1,					// Targets myself
							SKILLMAGIC_TARGET_FRIEND_WITHME = 2,		// Targets an ally (includes myself)
							SKILLMAGIC_TARGET_FRIEND_ONLY = 3,			// Targets an ally (excludes myself)
							SKILLMAGIC_TARGET_PARTY = 4,				// Targets a party member (includes myself)
							SKILLMAGIC_TARGET_NPC_ONLY = 5,				// Targets an NPC only
							SKILLMAGIC_TARGET_PARTY_ALL = 6,			// Targets the entire party (includes myself)
							SKILLMAGIC_TARGET_ENEMY_ONLY = 7,			// Targets only enemies (anything hostile, including NPCs)
							SKILLMAGIC_TARGET_ALL = 8,					// Targets anyone (includes myself)
							
							SKILLMAGIC_TARGET_AREA_ENEMY = 10,			// Targets enemies in an area
							SKILLMAGIC_TARGET_AREA_FRIEND = 11,			// Targets allies in an area
							SKILLMAGIC_TARGET_AREA_ALL = 12,			// Targets anyone in an area
							SKILLMAGIC_TARGET_AREA = 13,				// Targets anyone in an area centered around myself
							SKILLMAGIC_TARGET_DEAD_FRIEND_ONLY = 25,	// Targets dead allies (excluding myself)
							
							SKILLMAGIC_TARGET_UNKNOWN = 0xffffffff
						};

// define fx...
struct __TABLE_FX
{
	uint32_t		dwID;		// 01 ID
	std::string		szName;		// 02 Effect name
	std::string		szFN;		// 03 Effect filename
	uint32_t		dwSoundID;	// 04 Sound ID
	uint8_t			byAOE;		// 05 AOE ??
};

constexpr int	MAX_COMBO = 3;

constexpr int   FXID_CLASS_CHANGE				= 603;
constexpr int	FXID_BLOOD						= 10002;
constexpr int	FXID_LEVELUP_KARUS				= 10012;
constexpr int	FXID_LEVELUP_ELMORAD			= 10018;
constexpr int	FXID_REGEN_ELMORAD				= 10019;
constexpr int	FXID_REGEN_KARUS				= 10020;
constexpr int	FXID_SWORD_FIRE_MAIN			= 10021;
constexpr int	FXID_SWORD_FIRE_TAIL			= 10022;
constexpr int	FXID_SWORD_FIRE_TARGET			= 10031;
constexpr int	FXID_SWORD_ICE_MAIN				= 10023;
constexpr int	FXID_SWORD_ICE_TAIL				= 10024;
constexpr int	FXID_SWORD_ICE_TARGET			= 10032;
constexpr int	FXID_SWORD_LIGHTNING_MAIN		= 10025;
constexpr int	FXID_SWORD_LIGHTNING_TAIL		= 10026;
constexpr int	FXID_SWORD_LIGHTNING_TARGET		= 10033;
constexpr int	FXID_SWORD_POISON_MAIN			= 10027;
constexpr int	FXID_SWORD_POISON_TAIL			= 10028;
constexpr int	FXID_SWORD_POISON_TARGET		= 10034;
//constexpr int	FXID_GROUND_TARGET = 10035;
constexpr int	FXID_REGION_TARGET_EL_ROGUE		= 10035;
constexpr int	FXID_REGION_TARGET_EL_WIZARD	= 10036;
constexpr int	FXID_REGION_TARGET_EL_PRIEST	= 10037;
constexpr int	FXID_REGION_TARGET_KA_ROGUE		= 10038;
constexpr int	FXID_REGION_TARGET_KA_WIZARD	= 10039;
constexpr int	FXID_REGION_TARGET_KA_PRIEST	= 10040;
constexpr int	FXID_CLAN_RANK_1				= 10041;
constexpr int	FXID_WARP_KARUS					= 10046;
constexpr int	FXID_WARP_ELMORAD				= 10047;
constexpr int	FXID_REGION_POISON				= 10100;
constexpr int	FXID_TARGET_POINTER				= 30001;
constexpr int	FXID_ZONE_POINTER				= 30002;

enum e_SkillMagicType4	{	BUFFTYPE_MAXHP = 1,				// Max HP
							BUFFTYPE_AC = 2,				// Defense
							BUFFTYPE_RESIZE = 3,			// Character size
							BUFFTYPE_ATTACK = 4,			// Attack power
							BUFFTYPE_ATTACKSPEED = 5,		// Attack speed
							BUFFTYPE_SPEED = 6,				// Movement speed
							BUFFTYPE_ABILITY = 7,			// Base stats (str, sta, dex, int, cha)
							BUFFTYPE_RESIST = 8,			// Resistances (fire, ice, lightning, etc.)
							BUFFTYPE_HITRATE_AVOIDRATE = 9,	// Hit rate / evasion rate
							BUFFTYPE_TRANS = 10,			// Transformation/invisibility
							BUFFTYPE_SLEEP = 11,			// Puts to sleep
							BUFFTYPE_EYE = 12				// Vision-related							
};

enum e_SkillMagicType3	{	DDTYPE_TYPE3_DUR_OUR = 100,
							DDTYPE_TYPE3_DUR_ENEMY = 200
};

// Special items associated with skill usage
constexpr uint32_t ITEM_ID_MASTER_SCROLL_WARRIOR	= 379063000;
constexpr uint32_t ITEM_ID_MASTER_SCROLL_ROGUE		= 379064000;
constexpr uint32_t ITEM_ID_MASTER_SCROLL_MAGE		= 379065000;
constexpr uint32_t ITEM_ID_MASTER_SCROLL_PRIEST		= 379066000;

constexpr uint32_t ITEM_ID_STONE_OF_WARRIOR			= 379059000;
constexpr uint32_t ITEM_ID_STONE_OF_ROGUE			= 379060000;
constexpr uint32_t ITEM_ID_STONE_OF_MAGE			= 379061000;
constexpr uint32_t ITEM_ID_STONE_OF_PRIEST			= 379062000;

//definitions related clan....
constexpr int	CLAN_LEVEL_LIMIT		= 20;
constexpr int	CLAN_COST				= 500000;
constexpr uint32_t KNIGHTS_FONT_COLOR	= 0xffff0000; // Clan name font color

enum e_Cursor		{	CURSOR_ATTACK,
						CURSOR_EL_NORMAL,
						CURSOR_EL_CLICK,
						CURSOR_KA_NORMAL,
						CURSOR_KA_CLICK,
						CURSOR_PRE_REPAIR,
						CURSOR_NOW_REPAIR,
						CURSOR_COUNT,
						CURSOR_UNKNOWN = 0xffffffff
					};

#endif // end of #define __GAME_DEF_H_

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

constexpr int CURRENT_VERSION = 1298;//1068; // Current version

// Server.ini doesn't exist by default with our assets.
// For simplicity, have the login server default to a local server in debug builds
// if it's not otherwise supplied.
#if defined(_DEBUG)
static constexpr int DEFAULT_LOGIN_SERVER_COUNT = 1;
static constexpr char DEFAULT_LOGIN_SERVER_IP[] = "127.0.0.1";
#else
static constexpr int DEFAULT_LOGIN_SERVER_COUNT = 0;
static constexpr char DEFAULT_LOGIN_SERVER_IP[] = "";
#endif

constexpr float PACKET_INTERVAL_MOVE = 1.5f;				// Time interval for regularly sent packets..
constexpr float PACKET_INTERVAL_ROTATE = 4.0f;
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
				KM_TARGET_NEARST_ENEMY = DIK_Z, 
				KM_TARGET_NEARST_PARTY = DIK_X, 
				KM_TARGET_NEARST_FRIEND = DIK_V, 
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

const float WEAPON_WEIGHT_STAND_SWORD = 5.0f; // Weapon weight standard... sword
const float WEAPON_WEIGHT_STAND_AXE = 5.0f; // Weapon weight standard... axe
const float WEAPON_WEIGHT_STAND_BLUNT = 8.0f; // Weapon weight standard... blunt type

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
				ANI_SWORD_BREATH_B,			ANI_SWORD_ATTACK_B0, ANI_SWORD_ATTACK_B1,		// One-handed sword
				
				ANI_DAGGER_BREATH_A = 38,	ANI_DAGGER_ATTACK_A0, ANI_DAGGER_ATTACK_A1,
				ANI_DAGGER_BREATH_B,		ANI_DAGGER_ATTACK_B0, ANI_DAGGER_ATTACK_B1,		// Dagger
				
				ANI_DUAL_BREATH_A = 44,		ANI_DUAL_ATTACK_A0, ANI_DUAL_ATTACK_A1, 
				ANI_DUAL_BREATH_B,			ANI_DUAL_ATTACK_B0, ANI_DUAL_ATTACK_B1,			// Dual wield
				
				ANI_SWORD2H_BREATH_A = 50,	ANI_SWORD2H_ATTACK_A0, ANI_SWORD2H_ATTACK_A1, 
				ANI_SWORD2H_BREATH_B,		ANI_SWORD2H_ATTACK_B0, ANI_SWORD2H_ATTACK_B1,	// Two-handed sword
				
				ANI_BLUNT_BREATH_A = 56,	ANI_BLUNT_ATTACK_A0, ANI_BLUNT_ATTACK_A1, 
				ANI_BLUNT_BREATH_B,			ANI_BLUNT_ATTACK_B0, ANI_BLUNT_ATTACK_B1,		// Blunt weapon – club?
				
				ANI_BLUNT2H_BREATH_A = 62,	ANI_BLUNT2H_ATTACK_A0, ANI_BLUNT2H_ATTACK_A1, 
				ANI_BLUNT2H_BREATH_B,		ANI_BLUNT2H_ATTACK_B0, ANI_BLUNT2H_ATTACK_B1,	// Two-handed blunt weapon. - Same as a two-handed axe.
				
				ANI_AXE_BREATH_A = 68,		ANI_AXE_ATTACK_A0, ANI_AXE_ATTACK_A1, 
				ANI_AXE_BREATH_B,			ANI_AXE_ATTACK_B0, ANI_AXE_ATTACK_B1,			// One-handed axe
				
				ANI_SPEAR_BREATH_A = 74,	ANI_SPEAR_ATTACK_A0, ANI_SPEAR_ATTACK_A1, 
				ANI_SPEAR_BREATH_B,			ANI_SPEAR_ATTACK_B0, ANI_SPEAR_ATTACK_B1,		// Spear – just a spear without a cutting edge.
				
				ANI_POLEARM_BREATH_A = 80,	ANI_POLEARM_ATTACK_A0, ANI_POLEARM_ATTACK_A1, 
				ANI_POLEARM_BREATH_B,		ANI_POLEARM_ATTACK_B0, ANI_POLEARM_ATTACK_B1,	// Two-handed bladed spear – "Cheongryongdo" (Blue Dragon Sword)?
				
				ANI_NAKED_BREATH_A = 86,	ANI_NAKED_ATTACK_A0, ANI_NAKED_ATTACK_A1, 
				ANI_NAKED_BREATH_B,			ANI_NAKED_ATTACK_B0, ANI_NAKED_ATTACK_B1,		// With bare hands??
				
				ANI_BOW_BREATH = 92,		ANI_CROSS_BOW_BREATH, ANI_LAUNCHER_BREATH, 
				ANI_BOW_BREATH_B,			ANI_BOW_ATTACK_B0, ANI_BOW_ATTACK_B1,			// Bow attack
				
				ANI_SHIELD_BREATH_A = 98,	ANI_SHIELD_ATTACK_A0, ANI_SHIELD_ATTACK_A1, 
				ANI_SHIELD_BREATH_B,		ANI_SHIELD_ATTACK_B0, ANI_SHIELD_ATTACK_B1,		// Shield attack

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


//MAX_INCLINE_CLIMB = sqrt( 1 - sin(90 - Maximum slope angle)^2 )
//const	float MAX_INCLINE_CLIMB = 0.5f;	   // Maximum climbable slope value = 30 degree
const	float MAX_INCLINE_CLIMB = 0.6430f; // Maximum climbable slope value = 40 degree
//const	float MAX_INCLINE_CLIMB = 0.7071f; // Maximum climbable slope value = 45 degree
//const	float MAX_INCLINE_CLIMB = 0.7660f; // Maximum climbable slope value = 50 degree
//const	float MAX_INCLINE_CLIMB = 0.8660f; // Maximum climbable slope value = 60 degree


enum e_MoveDirection { MD_STOP, MD_FOWARD, MD_BACKWARD, MD_UNKNOWN = 0xffffffff };

const float MOVE_DELTA_WHEN_RUNNING = 3.0f; // Variable multiplied when jumping..
const float MOVE_SPEED_WHEN_WALK = 1.5f; // Standard walking speed for players

// 현재 상태...
enum e_StateMove {	PSM_STOP = 0,
					PSM_WALK,
					PSM_RUN,
					PSM_WALK_BACKWARD,
					PSM_COUNT };

enum e_StateAction {	PSA_BASIC = 0,		// Doing nothing...
						PSA_ATTACK,			// Attack state.. 
						PSA_GUARD,			// Defense successful – blocked..
						PSA_STRUCK,			// Taking heavy damage.
						PSA_DYING,			// Dying (falling down)
						PSA_DEATH,			// Dead and collapsed..
						PSA_SPELLMAGIC,		// Casting a magic spell..
						PSA_SITDOWN, 		// Sitting...
						PSA_COUNT }; 

enum e_StateDying {		PSD_DISJOINT = 0,	// Dies by disintegration.. 
						PSD_KNOCK_DOWN,		// Dies while flying or being knocked back.
						PSD_KEEP_POSITION,	// Dies posing in place..
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

/*
enum e_ItemClass	{	ITEM_CLASS_DAGGER = 1, // 단검(dagger)
						ITEM_CLASS_SWORD, // 2 : 한손검(onehandsword)
						ITEM_CLASS_SWORD_2H, // 3 : 양손검(twohandsword)
						ITEM_CLASS_AXE, // 4 : 한손도끼(onehandaxe)
						ITEM_CLASS_AXE_2H, // 5 : 두손도끼(twohandaxe)
						ITEM_CLASS_MACE, // 6 : 한손타격무기(mace)
						ITEM_CLASS_MACE_2H, // 7 : 두손타격무기(twohandmace)
						ITEM_CLASS_SPEAR, // 8 : 창(spear)
						ITEM_CLASS_POLEARM, // 9 : 폴암(polearm)
						
						ITEM_CLASS_SHIELD_SMALL = 11, // 11 : 스몰쉴드(smallshield)
						ITEM_CLASS_SHIELD_LARGE, // 12 : 라아지쉴드(largeshield)
						ITEM_CLASS_SHIELD_KITE, // 13 : 카이트쉴드(kiteshield)
						ITEM_CLASS_SHIELD_LARGETKITE, // 14 : 라아지카이트(largekite)
						ITEM_CLASS_SHIELD_PLATE, // 15 : 플레이트쉴드(plateshield)
						
						ITEM_CLASS_BOW_SHORT = 21, // 21 : 쇼트보우(Shortbow)
						ITEM_CLASS_BOW_WRAPT, // 22 : 랩트보우(wraptbow)
						ITEM_CLASS_BOW_COMPOSITE, // 23 : 콤포지트보우(compositebow)
						ITEM_CLASS_BOW_IRON, // 24 : 아이언보우(ironbow)
						ITEM_CLASS_BOW_LONG, // 25 : 롱보우(longbow)
						ITEM_CLASS_BOW_CROSS, // 28 : 크로스보우(crossbow)
						
						ITEM_CLASS_STAFF = 31, // 31 : 지팡이(staff)
						ITEM_CLASS_ETC, // 32 : 기타 마법 물품
						
						ITEM_CLASS_ARMOR_COTTON = 41, // 41 : 천방어구(cotton)
						ITEM_CLASS_ARMOR_FUR, // 42 : 털가죽(Fur)
						ITEM_CLASS_ARMOR_LEATHER, // 43 : 가죽방어구(leather)
						ITEM_CLASS_ARMOR_HADLEATHER, // 44 : 하드레더방어구(hardleather)
						ITEM_CLASS_ARMOR_RINGMAIL, // 45 : 링방어구(ringmail)
						ITEM_CLASS_ARMOR_SCALEMAIL, // 46 : 비늘방어구(scaledmail)
						ITEM_CLASS_ARMOR_HALFPLATE, // 47 : 하프 플레이트 방어구
						ITEM_CLASS_ARMOR_FULLPLATE, // 48 : 철판방어구(platemail)
						ITEM_CLASS_ROBE, // 49 : 마법사로브(robe)
						
						ITEM_CLASS_ARROW = 101,
						
						ITEM_CLASS_UNKNOWN = 0xffffffff }; // 101: 화살(arrow) 
*/
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
						ITEM_CLASS_CHARM = 95, //Items carried in inventory
						ITEM_CLASS_JEWEL = 96, //Types of gems
						ITEM_CLASS_POTION = 97, // Potion ( + transformation scrolls , usable items )
						ITEM_CLASS_SCROLL = 98, // Scroll

						ITEM_CLASS_LAUNCHER = 100, //  Item used for throwing spears..
						
						ITEM_CLASS_STAFF = 110, // Staff
						ITEM_CLASS_ARROW = 120, // Arrow
						ITEM_CLASS_JAVELIN = 130, // Javelin
						
						ITEM_CLASS_ARMOR_WARRIOR = 210, // Warrior armor
						ITEM_CLASS_ARMOR_ROGUE = 220, // Rogue armor
						ITEM_CLASS_ARMOR_MAGE = 230, // Mage armor
						ITEM_CLASS_ARMOR_PRIEST = 240, // Priest armor

						ITEM_CLASS_ETC = 251, // Miscellaneous, other items

						ITEM_CLASS_UNKNOWN = 0xffffffff }; // 

enum e_Nation { NATION_NOTSELECTED = 0, NATION_KARUS, NATION_ELMORAD, NATION_UNKNOWN = 0xffffffff };

struct __TABLE_ITEM_BASIC;
struct __TABLE_ITEM_EXT;
struct __TABLE_PLAYER;

struct __InfoPlayerOther
{
	int			iFace;			// Face shape
	int			iHair;			// Hairstyle

	int			iCity;			// Affiliated city
	int			iKnightsID;		// Affiliated knight order (clan) ID
	std::string szKnights;		// Affiliated knight order (clan) name
	int			iKnightsGrade;	// Affiliated knight order (clan) rank
	int			iKnightsRank;	// Affiliated knight order (clan) ranking

	int			iRank;			// Titles – count, duke – classification based on authority [king]
	int			iTitle;			// Position – lord, castle lord – simple status -> clan leader

	void Init()
	{
		iFace = 0;			
		iHair = 0;			
		iCity;				
		iKnightsID = 0;		
		szKnights = "";		
		iKnightsGrade = 0;		
		iKnightsRank = 0;			
		iTitle = 0;			
	}
};

// Knight order position..
enum e_KnightsDuty {	KNIGHTS_DUTY_UNKNOWN = 0,		// ????? kicked out??
						KNIGHTS_DUTY_CHIEF = 1,			// Leader
						KNIGHTS_DUTY_VICECHIEF = 2,		// Vice leader
						KNIGHTS_DUTY_PUNISH = 3,		// Under disciplinary action.
						KNIGHTS_DUTY_TRAINEE = 4,		// Apprentice knight
						KNIGHTS_DUTY_KNIGHT = 5,		// Regular knight
						KNIGHTS_DUTY_OFFICER = 6		// Officer
					};

#define VICTORY_ABSENCE		0
#define VICTORY_KARUS		1
#define VICTORY_ELMORAD		2

struct __InfoPlayerMySelf : public __InfoPlayerOther
{
	int					iBonusPointRemain;	// remaining bonus points
	int					iLevelPrev;			// previous level

	int					iMSPMax; 
	int					iMSP; 
			
	int					iTargetHPPercent;
	int					iGold;
	uint64_t			iExpNext;
	uint64_t			iExp;
	int					iRealmPoint;		// National contribution [NP]
	int					iRealmPointMonthly;	// Monthly NP
	e_KnightsDuty		eKnightsDuty;		// templar authority
	int					iWeightMax;			// max weight
	int					iWeight;			// current weight
	int					iStrength;			// strength
	int					iStrength_Delta;	// excess strength
	int					iStamina;			// stamina
	int					iStamina_Delta;		// excess stamina
	int					iDexterity;			// dexterity
	int					iDexterity_Delta;	// excess dexterity
	int					iIntelligence;		// intelligence
	int					iIntelligence_Delta; // excess intelligence
	int 				iMagicAttak;		// magic power, MP
	int 				iMagicAttak_Delta;	// excess MP
	
	int 				iAttack;		// Attack power
	int 				iAttack_Delta;	// Value modified by magic..
	int 				iGuard;			// Defense
	int 				iGuard_Delta;	// Value modified by magic..

	int 				iRegistFire;			// Resistance
	int 				iRegistFire_Delta;		// Resistance change value due to magic..
	int 				iRegistCold;			// Resistance
	int 				iRegistCold_Delta;		// Resistance change value due to magic..
	int 				iRegistLight;			// Resistance
	int 				iRegistLight_Delta;		// Resistance change value due to magic..
	int 				iRegistMagic;			// Resistance
	int 				iRegistMagic_Delta;		// Resistance change value due to magic..
	int 				iRegistCurse;			// Resistance
	int 				iRegistCurse_Delta;		// Magic resistance modifier..
	int 				iRegistPoison;			// Resistance
	int 				iRegistPoison_Delta;	// Magic resistance modifier..

	int					iZoneInit;				// Initial zone number received from server
	int					iZoneCur;				// Current zone..
	int					iVictoryNation;			// 0: Draw 1: El Morad victory 2: Karus victory

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

		iZoneInit = 0x01;			
		iZoneCur = 0;				
		iVictoryNation = -1;		
	}
};

const int MAX_PARTY_OR_FORCE = 8;

struct __InfoPartyOrForce
{
	int			iID;			// Party member ID
	int			iLevel;			// Level
	e_Class		eClass;			// Class
	int			iHP;			// Hit Point
	int			iHPMax;			// Hit Point Max
	bool		bSufferDown_HP;			// Status - HP decreased...
	bool		bSufferDown_Etc;		// Status - When affected by curse-type effects
	std::string szID;		// Party name string

	void Init()
	{
		iID = -1;
		iLevel = 0;
		eClass = CLASS_UNKNOWN;
		iHP = 0;
		iHPMax = 0;
		szID = "";

		bSufferDown_HP = false;			
		bSufferDown_Etc = false;		
	};

	__InfoPartyOrForce()
	{
		this->Init();
	}
};

enum e_PartyStatus { PARTY_STATUS_DOWN_HP = 1, PARTY_STATUS_DOWN_ETC = 2 };

struct __InfoPartyBBS // Party support board structure... (seeking party)
{
	std::string szID;			// Party name string
	int			iID;			// Party member ID
	int			iLevel;			// Level
	e_Class		eClass;			// Class
	int			iMemberCount;

	void Init()
	{
		szID = "";
		iID = -1;
		iLevel = 0;
		eClass = CLASS_UNKNOWN;
		iMemberCount = 0;
	};

	__InfoPartyBBS()
	{
		this->Init();
	}
};

typedef struct __TABLE_TEXTS
{
	uint32_t		dwID;
	std::string	szText;
} TABLE_TEXTS;

typedef struct __TABLE_ZONE
{
	uint32_t    dwID;                 //01 zone ID
	std::string	szTerrainFN;          //02 GTD
	std::string szName;				  //03	
	std::string	szColorMapFN;         //04 TCT
	std::string	szLightMapFN;         //05 TLT
	std::string	szObjectPostDataFN;   //06 OPD

#if __VERSION > 1264
	std::string szOpdExtFN;           //07 OPDEXT
#endif

	std::string	szMiniMapFN;          //08 DXT
	std::string szSkySetting;         //09 N3Sky
	int         bIndicateEnemyPlayer; //10 Int32 (BOOL)
	int         iFixedSundDirection;  //11 Int32
	std::string szLightObjFN;         //12 GLO

	std::string szGevFN;              //13 GEV
	int         iIdk0;                //14 idk
	std::string szEnsFN;              //15 ENS
	float       fIdk1;                //16 idk
	std::string szFlagFN;             //17 FLAG
	uint32_t    iIdk2;				  //18	
	uint32_t    iIdk3;				  //19	
	uint32_t    iIdk4;				  //20	
	uint32_t    iIdk5;				  //21
	std::string szOpdSubFN;           //22 OPDSUB
	int         iIdk6;				  //23
	std::string szEvtSub;             //24 EVTSUB
} TABLE_ZONE;

typedef struct __TABLE_UI_RESRC
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
#if __VERSION > 1264  
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
#endif
} TABLE_UI_RESRC;

typedef struct __TABLE_ITEM_BASIC		// Resource record related to equipped items...
{
	uint32_t		dwID;				// 01 Encoded item number d -  // 00 - Item type, 00 - Item equip position (can determine whether it's Plug or Part based on equip position).) - 0000 - ItemIndex
	uint8_t 		byExtIndex;			// 02 Extended index
	std::string	szName;					// 03 Name	
	std::string	szRemark;				// 04 Item Description

	uint32_t   dwIDK0;					// 05
	uint8_t    byIDK1;					// 06

	uint32_t   dwIDResrc;				// 07 resource id
	uint32_t   dwIDIcon;				// 08 icon id
	uint32_t   dwSoundID0;				// 09 Sound ID - 0 There is no sound for this~
	uint32_t   dwSoundID1;				// 10 Sound ID - 0 There is no sound for this~

	uint8_t	byClass;					// 11 Item type — see enum e_ItemClass for reference...
	uint8_t	byIsRobeType;				// 12 A robe-type item with the top and bottom connected as a whole.....
	uint8_t	byAttachPoint;				// 13 Equip position
	uint8_t	byNeedRace;					// 14 Race
	uint8_t	byNeedClass;				// 15 Class

	int16_t	siDamage;					// 16 Weapon attack
	int16_t	siAttackInterval;			// 17 Attack time 100 equals 1 second
	int16_t	siAttackRange;				// 18 Effective range (in 0.1 meter units)
	int16_t	siWeight;					// 19 Weight (in 0.1 weight units)
	int16_t	siMaxDurability;			// 20 Durability
	int		iPrice;						// 21 price
	int		iPriceSale;					// 22 selling price
	int16_t	siDefense;					// 23 Defense
	uint8_t	byContable;					// 24 countable ?

	uint32_t	dwEffectID1;			// 25 Magic effect ID1
	uint32_t	dwEffectID2;			// 26 Magic effect ID2

	char	cNeedLevel;					// 27 Required level — player’s iLevel; can also be a negative value.

	char    cIDK2;						// 28

	uint8_t	byNeedRank;					// 29 Required rank — player’s iRank.
	uint8_t	byNeedTitle;				// 30 Required title — player’s iTitle.
	uint8_t	byNeedStrength;				// 31 Required strength — player’s iStrength
	uint8_t	byNeedStamina;				// 32 Required stamina — player’s iStamina
	uint8_t	byNeedDexterity;			// 33 Required dexterity — player’s iDexterity
	uint8_t	byNeedInteli;				// 34 Required intelligence — player’s iIntelligence.
	uint8_t	byNeedMagicAttack;			// 35 Required magic power — player’s iMagicAttack.

	uint8_t	bySellGroup;				// 36 Group related to where the merchant sells it.
		
	uint8_t    byIDK3;					// 37
} TABLE_ITEM_BASIC;

const int MAX_ITEM_EXTENSION = 24; // Number of item extension tables. (Item_Ext_0..23.tbl is a total of 24)
const int LIMIT_FX_DAMAGE = 64;
const int ITEM_LIMITED_EXHAUST = 17;

typedef struct __TABLE_ITEM_EXT		// Resource record related to the equipped item
{
	uint32_t		dwID;			// 01 Coded item number - // 00 - Item type, 00 - Item equip position (used to determine whether it is a plug or part). - 0000 - ItemIndex
	std::string	szHeader;			// 02 Prefix

	uint32_t dwBaseID;				// 03

	std::string	szRemark;			// 04 Item description

	uint32_t dwIDK0;				// 05 TODO: will need to implement this one
	uint32_t dwIDResrc;				// 06
	uint32_t dwIDIcon;				// 07

	uint8_t		byMagicOrRare;		// Whether it is a magic or rare item...

	int16_t	siDamage;				// 09 Weapon damage
	int16_t	siAttackIntervalPercentage;		// 10 Attack speed ratio
	int16_t	siHitRate;				// 11 Hit rate — percentage
	int16_t	siEvationRate;			// 12 Evasion rate — percentage

	int16_t	siMaxDurability;		// 13 Durability
	int16_t	siPriceMultiply;		// 14 Quantity multiplier
	int16_t	siDefense;				// 15 Defense
	
	int16_t	siDefenseRateDagger;	// 16 Dagger defense — percentage
	int16_t	siDefenseRateSword;		// 17 sword defense	- percentage
	int16_t	siDefenseRateBlow;		// 18 strike(club?) def	- percentage
	int16_t	siDefenseRateAxe;		// 19 axe defense	- percentage
	int16_t	siDefenseRateSpear;		// 20 spear defense	- percentage
	int16_t	siDefenseRateArrow;		// 21 arrow defense	- percentage
	
	uint8_t	byDamageFire;			// 22 Additional damage — fire
	uint8_t	byDamageIce;			// 23 Additional damage - ice
	uint8_t	byDamageThuner;			// 24 Additional damage - thunder
	uint8_t	byDamagePoison;			// 25 Additional damage - poison

	uint8_t	byStillHP;				// 26 HP recovery
	uint8_t	byDamageMP;				// 27 MP Damage
	uint8_t	byStillMP;				// 28 MP recovery
	uint8_t	byReturnPhysicalDamage;	// 29 Physical damage reflection

	uint8_t	bySoulBind;				// 30 Soul bind — the percentage chance of dropping the item upon death in one-on-one combat; currently not in use.
	
	int16_t	siBonusStr;				// 31 bonus str
	int16_t	siBonusSta;				// 32 bonus stamina
	int16_t	siBonusDex;				// 33 bonux dex
	int16_t	siBonusInt;				// 34 bonus int
	int16_t	siBonusMagicAttak;		// 35 bonus mp
	int16_t	siBonusHP;				// 36 HP bonus
	int16_t	siBonusMSP;				// 37 MSP bonus

	int16_t	siRegistFire;			// 38 fire resistence
	int16_t	siRegistIce;			// 39 ice resistance
	int16_t	siRegistElec;			// 40 Electric resistance
	int16_t	siRegistMagic;			// 41 magic resistance
	int16_t	siRegistPoison;			// 42 poison resistance
	int16_t	siRegistCurse;			// 43 curse resistance
	
	uint32_t	dwEffectID1;		// 44 magic effect ID1
	uint32_t	dwEffectID2;		// 45 magic effect ID2

	int16_t	siNeedLevel;			// 46 ilevel needed
	int16_t	siNeedRank;				// 47 iRank needed
	int16_t	siNeedTitle;			// 48 title needed
	int16_t	siNeedStrength;			// 49 strength needed
	int16_t	siNeedStamina;			// 50 stamina needed
	int16_t	siNeedDexterity;		// 51 dexterity needed
	int16_t	siNeedInteli;			// 52 intelligence needed
	int16_t	siNeedMagicAttack;		// 53 mp needed
} TABLE_ITEM_EXT;

const int MAX_NPC_SHOP_ITEM = 30;
typedef struct __TABLE_NPC_SHOP
{
	uint32_t		dwNPCID;
	std::string	szName;
	uint32_t		dwItems[MAX_NPC_SHOP_ITEM];
} TABLE_NPC_SHOP;

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


typedef struct __TABLE_PLAYER_LOOKS // Resource record related to NPC and Mob appearance...
{
	uint32_t		dwID;		// NPC unique ID
	std::string	szName;			// npc name
	std::string	szJointFN;		// Joint file name
	std::string	szAniFN;		// Animation file name
	//std::string	szPartFNs[7]; // Each character part — upper body, lower body, head, arms, legs, hair, cape
	std::string	szPartFNs[13];	// temp for 1264 TBLs

	int  iIdk1;

	int			iJointRH;			// Right hand end joint number
	int			iJointLH;			// Left hand end joint number
	int			iJointLH2;			// Left forearm joint number
	int			iJointCloak;		// Cape attachment joint number
	
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

	int  iIdk2;
	int  iIdk3;
	uint8_t bIdk4;
	uint8_t bIdk5;
	uint8_t bIdk6;
} TABLE_PLAYER;

typedef struct __TABLE_EXCHANGE_QUEST
{
	uint32_t		dwID;				// 01 Quest Number
	uint32_t		dwNpcNum;			// 02 NPC Number
	std::string szDesc;					// 03 Description
	int			iCondition0;			// 04 Condition 1..
	int			iCondition1;			// 05 Condition 2..
	int			iCondition2;			// 06 Condition 3..
	int			iCondition3;			// 07 Condition 4..
	int			iNeedGold;				// 08 Required Noah	
	uint8_t		bNeedLevel;				// 09 Required Level
	uint8_t		bNeedClass;				// 10 Required Class
	uint8_t		bNeedRank;				// 11 Required Rank	
	uint8_t		bNeedExtra1;			// 12 Required Extra 1	
	uint8_t		bNeedExtra2;			// 13 Required Extra 2
	uint8_t		bCreatePercentage;		// 14 Generation Probability
	int			iArkTuarek;				// 15 Ark Tuarek
	int			iTuarek;				// 16 Tuarek
	int			iRinkleTuarek;			// 17 Wrinkle Tuarek
	int			iBabarian;				// 18 Barbarian
	int			iMan;					// 19 Man
	int			iWoman;					// 20 Woman
} TABLE_EXCHANGE_QUEST;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//Magic Table

typedef struct __TABLE_UPC_SKILL
{
	uint32_t	dwID;				// Unique SKILL ID
	std::string	szEngName;			// Skill English name
	std::string	szName;				// Skill Korean name
	std::string	szDesc;				// Skill description
	int			iSelfAnimID1;		// Caster animation start
	int			iSelfAnimID2;		// Caster animation end

	int			idwTargetAnimID;	// Target animation
	int			iSelfFX1;			// Caster effect 1
	int			iSelfPart1;			// Caster effect position 1
	int			iSelfFX2;			// Caster effect 2
	int			iSelfPart2;			// Caster effect position 2	
	int			iFlyingFX;			// Flying effect
	int			iTargetFX;			// Target effect

	int			iTargetPart;		// Effect position
	int			iTarget;			// Target	
	int			iNeedLevel;			// Required level
	int			iNeedSkill;			// Required skill
	int			iExhaustMSP;		// MSP consumption

	int			iExhaustHP;			// HP consumption
	uint32_t	dwNeedItem;			// Required item (refer to e_ItemClass.. value divided by 10)
	uint32_t	dwExhaustItem;		// Consumed item
	int			iCastTime;			// Casting time
	int			iReCastTime;		// Time until recasting

	float fIDK0; // TODO: will need to implement this...?
	float fIDK1; // 1298 (unknown purpose)

	int			iPercentSuccess;	// Success rate
	uint32_t	dw1stTableType;		// First type
	uint32_t	dw2ndTableType;		// Second type
	int			iValidDist;			// Valid distance

	int			iIDK2;				// 1298 (unknown purpose)

} TABLE_UPC_ATTACK_B;


typedef struct __TABLE_UPC_SKILL_TYPE_1
{
	uint32_t	dwID;			// SKILL unique ID
	int			iSuccessType;	// Success type
	int			iSuccessRatio;	// Success ratio
	int			iPower;			// Attack power
	int			iDelay;			// Delay
	int			iComboType;		// Combo type
	int			iNumCombo;		// Number of combos
	int			iComboDamage;	// Combo damage			
	int			iValidAngle;	// Attack radius
	int			iAct[3];
} TABLE_UPC_SKILL_TYPE_1;

typedef struct __TABLE_UPC_SKILL_TYPE_2
{
	uint32_t	dwID;			// SKILL unique ID
	int			iSuccessType;	// Success type
	int			iPower;			// Attack power
	int			iAddDamage;
	int			iAddDist;		// Distance increase
	int			iNumArrow;		// Number of arrows required
} TABLE_UPC_SKILL_TYPE_2;

typedef struct __TABLE_UPC_SKILL_TYPE_3
{
	uint32_t	dwID;			//  SKILL Unique ID
	int			iRadius;
	int			iDDType;
	int			iStartDamage;
	int			iDuraDamage;
	int			iDurationTime;	// 지속시간
	int			iAttribute;
} TABLE_UPC_SKILL_TYPE_3;

typedef struct __TABLE_UPC_SKILL_TYPE_4
{
	uint32_t	dwID;			// Serial number

	int			iBuffType;		// Buff type
	int			iRadius;
	int			iDuration;
	int			iAttackSpeed;	// Attack speed
	int			iMoveSpeed;		// Movement speed
	int			iAC;			// Defense
	int			iACPct;
	int			iAttack;		// Attack power
	int			iMagicAttack;
	int			iMaxHP;			// Max HP
	int			iMaxHPPct;
	int			iMaxMP;
	int			iMaxMPPct;
	int			iStr;			// Strength
	int			iSta;			// Stamina
	int			iDex;			// Dexterity
	int			iInt;			// Intelligence
	int			iMAP;			// Magic power
	int			iFireResist;	// Fire resistance
	int			iColdResist;	// Cold resistance
	int			iLightningResist;// Lightning resistance
	int			iMagicResist;	// Magic resistance
	int			iDeseaseResist;	// Curse resistance
	int			iPoisonResist;	// Poison resistance

	int			iExpPct;
} TABLE_UPC_SKILL_TYPE_4;

typedef struct __TABLE_UPC_SKILL_TYPE_5
{
	uint32_t		dwID;			// Serial number
	uint32_t		dwTarget;		// Target
	int			iSuccessRatio;		// Success ratio
	int			iValidDist;			// Valid distance
	int			iRadius;			// Radius
	float		fCastTime;			// Casting time
	float		fRecastTime;		// Recasting time
	int			iDurationTime;		// Duration time
	uint32_t		dwExhaustItem;	// Consumed item
	uint32_t		dwFX;			// Magic effect
} TABLE_UPC_SKILL_TYPE_5;

typedef struct __TABLE_UPC_SKILL_TYPE_6
{
	uint32_t		dwID;			// Serial number
	uint32_t		dwTarget;		// Target
	int			iSuccessRatio;		// Success ratio
	int			iValidDist;			// Valid distance
	int			iRadius;			// Radius
	float		fCastTime;			// Casting time
	float		fRecastTime;		// Recasting time
	int			iDurationTime;		// Duration time
	uint32_t		dwExhaustItem;	// Consumed item
	uint32_t		dwFX;			// Magic effect
	uint32_t		dwTranform;		// Transformation
} TABLE_UPC_SKILL_TYPE_6;

typedef struct __TABLE_UPC_SKILL_TYPE_7
{
	uint32_t		dwID;			// Serial number
	uint32_t		dwTarget;		// Target
	uint32_t		dwValidGroup;	// Valid group
	int			iSuccessRatio;		// Success ratio
	int			iValidDist;			// Valid distance
	int			iRadius;			// Radius
	float		fCastTime;			// Casting time
	float		fRecastTime;		// Recasting time
	int			iDurationTime;		// Duration time
	uint32_t		dwExhaustItem;	// Consumed item
	uint32_t		dwFX;			// Magic effect	
} TABLE_UPC_SKILL_TYPE_7;

typedef struct __TABLE_UPC_SKILL_TYPE_8
{
	uint32_t		dwID;			// Serial number
	uint32_t		dwTarget;		// Target
	int			iRadius;			// Radius
	uint32_t		dwWarpType;		// Teleport type
	float		fRefillEXP;			// Experience recovery
	uint32_t		dwZone1;		// Zone number 1
	uint32_t		dwZone2;		// Zone number 2
	uint32_t		dwZone3;		// Zone number 3
	uint32_t		dwZone4;		// Zone number 4
	uint32_t		dwZone5;		// Zone number 5
} TABLE_UPC_SKILL_TYPE_8;

typedef struct __TABLE_UPC_SKILL_TYPE_9
{
	uint32_t		dwID;			// Serial number
	uint32_t		dwTarget;		// Target
	int			iSuccessRatio;		// Success ratio
	int			iValidDist;			// Valid distance
	int			iRadius;			// Radius
	float		fCastTime;			// Casting time
	float		fRecastTime;		// Recasting time
	int			iDurationTime;		// Duration time
	uint32_t		dwExhaustItem;	// Consumed item
	uint32_t		dwAttr;			// Attribute
	int			iDamage;			// Damage	
} TABLE_UPC_SKILL_TYPE_9;

typedef struct __TABLE_UPC_SKILL_TYPE_10
{
	uint32_t		dwID;			// Serial number
	uint32_t		dwTarget;		// Target
	int			iSuccessRatio;		// Success ratio
	int			iValidDist;			// Valid distance
	int			iRadius;			// Radius
	float		fCastTime;			// Casting time
	float		fRecastTime;		// Recasting time
	uint32_t		dwExhaustItem;	// Consumed item
	uint32_t		dwRemoveAttr;	// Removed attribute	
} TABLE_UPC_SKILL_TYPE_10;

//Magic Table
///////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct __TABLE_QUEST_MENU
{
	uint32_t		dwID;	// id
	std::string szMenu;		// Selection menu
} TABLE_QUEST_MENU;

typedef struct __TABLE_QUEST_TALK
{
	uint32_t		dwID;		// id
	std::string szTalk;			// Quest talk
} TABLE_QUEST_TALK;

typedef struct __TABLE_QUEST_CONTENT
{
	uint32_t		dwID;
	int				iReqLevel;
	int				iReqClass;
	std::string		szName;
	std::string		szDesc;
	std::string		szReward;
} TABLE_QUEST_CONTENT;

typedef struct __TABLE_HELP
{
	DWORD		dwID;
	int			iMinLevel;
	int			iMaxLevel;
	int			iReqClass;
	std::string	szQuestName;
	std::string	szQuestDesc;
} TABLE_HELP;

const int MAX_ITEM_SLOT_OPC = 8; // 착용 아이템 - 다른 플레이어(NPC 포함) 0 ~ 4 상체,하체,헬멧,팔,발 5 망토 6 오른손 7 왼손
const int MAX_ITEM_INVENTORY = 28;		// 소유 아템 MAX (인벤토리창)
const int MAX_ITEM_TRADE = 24;	// 상인과 거래..
const int MAX_ITEM_TRADE_PAGE = 12;
const int MAX_ITEM_WARE_PAGE = 8;
const int MAX_ITEM_PER_TRADE = 12;	// 개인과 거래..	
const int MAX_ITEM_BUNDLE_DROP_PIECE = 6;
const int MAX_ITEM_EX_RE_NPC = 4; // 교환, 수리창 NPC 영역..

const int MAX_SKILL_FROM_SERVER = 9;		// 서버에게서 받는 정보 슬롯 갯수..			

const int MAX_SKILL_KIND_OF = 5;			// Base Skill 1개, 전문 스킬 4개..			
const int MAX_SKILL_IN_PAGE = 6;//8;			// 한 페이지 내의 아이콘 갯수..				
const int MAX_SKILL_PAGE_NUM = 7;			// Maximum number of pages per skill tab		


const int MAX_SKILL_HOTKEY_PAGE = 8;		// Hot Key의 총 페이지 수.. 				
const int MAX_SKILL_IN_HOTKEY = 8;			// Hot Key의 현 페이지내의 갯수..			
		
const int MAX_AVAILABLE_CHARACTER = 3;		// 한 서버당 선택할수 있는 최대 캐릭터 수..	

// 싸운도.... By ecli666
const int ID_SOUND_ITEM_ETC_IN_INVENTORY	= 2000;
const int ID_SOUND_ITEM_IN_REPAIR			= 2001;
const int ID_SOUND_ITEM_WEAPON_IN_INVENTORY = 2002;
const int ID_SOUND_ITEM_ARMOR_IN_INVENTORY	= 2003;
const int ID_SOUND_GOLD_IN_INVENTORY		= 3000;
const int ID_SOUND_SKILL_THROW_ARROW		= 5500;
const int ID_SOUND_BGM_TOWN					= 20000;
const int ID_SOUND_BGM_KA_BATTLE			= 20002;
const int ID_SOUND_BGM_EL_BATTLE			= 20003;
const int ID_SOUND_CHR_SELECT_ROTATE		= 2501;

const float SOUND_RANGE_TO_SET = 10.0f;
const float SOUND_RANGE_TO_RELEASE = 20.0f;

const float STUN_TIME = 3.0f;

enum e_Behavior {	BEHAVIOR_NOTHING = 0,
					BEHAVIOR_EXIT,			// Exit program..
					BEHAVIOR_RESTART_GAME,	// Restart game (character selection)
					BEHAVIOR_REGENERATION,	// Resurrection
					BEHAVIOR_CANCEL,		// Cancel... Currently when the user presses cancel during an item trade request..

					BEHAVIOR_PARTY_PERMIT,	// Allow joining when the other party sends a party invitation request.
					BEHAVIOR_PARTY_DISBAND, // Leave party
					BEHAVIOR_FORCE_PERMIT,	// Allow joining when the other party sends a squad join request.
					BEHAVIOR_FORCE_DISBAND, // Leave squad

					BEHAVIOR_REQUEST_BINDPOINT, // To binding point 

					BEHAVIOR_DELETE_CHR,

					BEHAVIOR_KNIGHTS_CREATE,
					BEHAVIOR_KNIGHTS_DESTROY, // Disband the clan
					BEHAVIOR_KNIGHTS_WITHDRAW, // Disband the order

					BEHAVIOR_PERSONAL_TRADE_FMT_WAIT,	// Private trade... when I initiated the request..
					BEHAVIOR_PERSONAL_TRADE_PERMIT,		//Private trade... when I received the request..

					BEHAVIOR_MGAME_LOGIN,
					
					BEHAVIOR_CLAN_JOIN,
					BEHAVIOR_PARTY_BBS_REGISTER,		// Register to the party board 
					BEHAVIOR_PARTY_BBS_REGISTER_CANCEL, // Unregister from the party board

					BEHAVIOR_EXECUTE_OPTION,			// Exit game and open options..
				
					BEHAVIOR_UNKNOWN = 0xffffffff
				};

enum e_SkillMagicTaget	{	SKILLMAGIC_TARGET_SELF = 1,				// Myself
							SKILLMAGIC_TARGET_FRIEND_WITHME = 2,	// One of our allies (including myself/nation)
							SKILLMAGIC_TARGET_FRIEND_ONLY = 3,		// One of our allies excluding myself 
							SKILLMAGIC_TARGET_PARTY = 4,			// One of our party members, including myself
							SKILLMAGIC_TARGET_NPC_ONLY = 5,			// One of the NPCs
							SKILLMAGIC_TARGET_PARTY_ALL = 6,		// Our entire party, including myself
							SKILLMAGIC_TARGET_ENEMY_ONLY = 7,		// One of all enemies excluding our side (including NPCs)
							SKILLMAGIC_TARGET_ALL = 8,				// One of everything that exists in the game
							
							SKILLMAGIC_TARGET_AREA_ENEMY = 10,		// Enemies in that area
							SKILLMAGIC_TARGET_AREA_FRIEND = 11,		// Our allies in that area
							SKILLMAGIC_TARGET_AREA_ALL = 12,		// Everyone in that area
							SKILLMAGIC_TARGET_AREA = 13,			// Area centered around me
							SKILLMAGIC_TARGET_DEAD_FRIEND_ONLY = 25,//One of our deceased allies excluding myself.
							
							SKILLMAGIC_TARGET_UNKNOWN = 0xffffffff
						};


//define fx...
typedef struct __TABLE_FX	// FX Table
{
	uint32_t		dwID;		// ID
	std::string		szName;		// NOTE: adding the name of the FX
	std::string		szFN;		// file name
	uint32_t		dwSoundID;	// sound id
#if __VERSION > 1264
	uint8_t			byAOE;		// AOE ??
#endif
} TABLE_FX;

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

//define skillmagic_type4_bufftype
enum e_SkillMagicType4	{	BUFFTYPE_MAXHP = 1,				//Max HP change
							BUFFTYPE_AC = 2,				//AC change
							BUFFTYPE_RESIZE = 3,			//Character size adjustment
							BUFFTYPE_ATTACK = 4,			//Attack power
							BUFFTYPE_ATTACKSPEED = 5,		//Attack speed
							BUFFTYPE_SPEED = 6,				//Movement speed
							BUFFTYPE_ABILITY = 7,			//Five abilities(str, sta, cha, dex int)
							BUFFTYPE_RESIST = 8,			// Five types of resistance
							BUFFTYPE_HITRATE_AVOIDRATE = 9,	//hitrate n avoidrate
							BUFFTYPE_TRANS = 10,			// Transformation, Invisibility
							BUFFTYPE_SLEEP = 11,			// Putting to sleep
							BUFFTYPE_EYE = 12				// Vision-related							
};

enum e_SkillMagicType3	{	DDTYPE_TYPE3_DUR_OUR = 100,
							DDTYPE_TYPE3_DUR_ENEMY = 200
};



enum e_ObjectType	{	OBJECT_TYPE_BINDPOINT,
						OBJECT_TYPE_DOOR_LEFTRIGHT,
						OBJECT_TYPE_DOOR_TOPDOWN,
						OBJECT_TYPE_LEVER_TOPDOWN,
						OBJECT_TYPE_FLAG,
						OBJECT_TYPE_WARP_POINT,
						OBJECT_TYPE_UNKNOWN = 0xffffffff
					};

//definitions related clan....
const int	CLAN_LEVEL_LIMIT	= 20;
const int	CLAN_COST			= 500000;
const uint32_t KNIGHTS_FONT_COLOR	= 0xffff0000; // Font color of the knight (clan) name...

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


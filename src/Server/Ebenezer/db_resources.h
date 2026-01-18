#ifndef SERVER_EBENEZER_DB_RESOURCES_H
#define SERVER_EBENEZER_DB_RESOURCES_H

#pragma once

#include "EbenezerResourceFormatter.h"

// clang-format off

enum e_DBResourceID : uint8_t
{
	// NOTE: This is in the table, but it's unintentional - it's an MFC error,
	// and typically triggered by MFC's code.
	// It's technically *also* triggered in our code, but it's way before we even establish a connection to the DB server.
	// Officially it uses the MFC resource.
	// IDP_SOCKETS_INIT_FAILED							= 103, // Failed initializing Windows socket
	IDP_BATTLEZONE_OPEN									= 105, // Lunar Gate is now open.  The War has begun.
	IDP_KARUS_VICTORY									= 106, // Karus is victorious. They killed %d warriors and lost %d warriors
	IDP_ELMORAD_VICTORY									= 107, // El Morad is victorious. They killed %d warriors and lost %d warriors
	IDP_KARUS_BANISH									= 108, // El Morad warriors in the battle zone have been banned!
	IDP_ELMORAD_BANISH									= 109, // Karus warriors in the battle zone have been banned!
	IDP_KNIGHT_NAME_REPEAT								= 110, // This Knights name already exist.
	IDP_KNIGHT_ALREADY_REGISTERED						= 111, // You are already in a Knights
	IDP_KNIGHT_INSUFFICIENT_LEVEL						= 112, // You need to be at least at Level 20 in order to create a Knights
	IDP_KNIGHT_INSUFFICIENT_LOYALTY						= 113, // You need to have at least 800 National Points in order to create a Knights
	IDP_KNIGHT_INSUFFICIENT_SKILL						= 114, // You need to have at least 10 Leadership skill points and 5 Manner skill points in order to create a
	IDP_KNIGHT_INSUFFICIENT_CHARISMA					= 115, // You need to have at least 120 Attractactiveness skill points in order to create a Knights
	IDP_KNIGHT_INSUFFICIENT_GOLD						= 116, // You need to have at least 1,000,000 Coins in order to create a Knights
	IDP_KNIGHT_CREATE_FAIL								= 117, // You have failed creating a Knights
	IDP_KNIGHT_NOT_AVAILABLE							= 118, // This Knights does not exist
	IDP_KNIGHT_LEADER_ONLY								= 119, // This is authorized for Head of Knights only.
	IDP_KNIGHT_MINIMUM_SUBLEADER						= 120, // This is authorized for ranks higher than Vice Head of Knights only.
	IDP_KNIGHT_NOT_REGISTERED							= 121, // Not Register Knights
	IDP_KNIGHT_DB_FAIL									= 122, // Failed Update DB with Knights
	IDP_WRONG_ID										= 123, // This ID does not exist
	IDP_SOURCE_UNABLE									= 124, // Source Unable
	IDP_MINIMUM_OFFICER									= 125, // This is authorized for ranks higher than officer
	IDP_ANNOUNCEMENT									= 126, // #### NOTICE : %s ####
	IDS_ELMORAD_LOSER									= 130, // El Morad have lost the war. They killed [%d] warriors and lost [%d] warriors
	IDS_KARUS_LOSER										= 131, // Karus have lost the war. They killed [%d] warriors and lost [%d] warriors
	IDS_BANISH_USER										= 132, // Users in the Lunar Valley and the enemy’s territory will be teleported back to their own Nations.
	IDS_BATTLE_CLOSE									= 133, // Lunar Gate is closing.
	IDS_KILL_GATEKEEPER									= 134, // The user that killed the enemy’s gatekeeper is [%s][%s]. Congratulations!!
	IDS_KILL_CAPTAIN									= 135, // The user that killed the enemy’s captain is [%s][%s]. Congratulations!!
	IDS_KILL_KARUS_GUARD1								= 136, // The sentry at the 1st Karus fortress has been killed. The user that killed the sentry is [%s][%s]
	IDS_KILL_KARUS_GUARD2								= 137, // The sentry at the 2nd Karus fortress has been killed. The user that killed the sentry is [%s][%s]
	IDS_KILL_ELMO_GUARD1								= 138, // The sentry at the 1st El Morad fortress has been killed. The user that killed the sentry is [%s][%s]
	IDS_KILL_ELMO_GUARD2								= 139, // The sentry at the 2nd El Morad fortress has been killed. The user that killed the sentry is [%s][%s]
	IDS_KARUS_CAPTAIN									= 140, // The captain(s) for Karus is(are) %s, %s, %s, %s, %s
	IDS_ELMO_CAPTAIN									= 141, // The captain(s) for El Morad is(are) %s, %s, %s, %s, %s
	IDS_KARUS_CAPTAIN_DEPRIVE							= 142, // [%s][%s], the captain for Karus has been killed
	IDS_ELMO_CAPTAIN_DEPRIVE							= 143, // [%s][%s], the captain for El Morad has been killed
	IDS_COUPON_NOTEPAD_ID								= 144, // 3572-8471-3720-2495
	IDS_COUPON_POSTIT_ID								= 145, // 7352-6539-3720-2495
	IDS_KNIGHTS_JOIN									= 146, // #### %s has joined. ####
	IDS_KNIGHTS_WITHDRAW								= 147, // #### %s has left. ####
	IDS_KNIGHTS_REMOVE									= 148, // #### %s has been banned. ####
	IDS_KNIGHTS_CHIEF									= 149, // #### %s has been appointed as a leader. ####
	IDS_KNIGHTS_VICECHIEF								= 150, // #### %s has been appointed as a co-leader. ####
	IDS_KNIGHTS_DESTROY									= 151, // #### the knights %s has disbanded ####
	IDS_CLAN_DESTORY									= 152, // #### the clan %s has disbanded ####
	IDS_CLANGAME_VICTORY								= 153, // The Clan %s has won the clan duel.
	IDS_CLANGAME_JOIN									= 154, // *** %s(Lv %d)has entered the Arena Lobby ***
	IDS_CLANGAME_EXIT									= 155, // *** %s(Lv %d)has left ***
	IDS_SNOWBATTLE_OPEN									= 156, // The Snow Fight Zone is now open
	IDS_SNOWBATTLE_CLOSE								= 157, // The Snow Fight Zone is now closed
	IDS_SNOWBATTLE_BANISH_USER							= 158, // The users in the Snow Fight Zone will be teleported back to their native country.
	IDS_BATTLEZONE_STATUS								= 159, // Number of killed Karus users : %d, Number of killed El Morad users : %d
	IDS_BANISH_LOSER									= 160, // The users of the defeated nation in Lunar Valley will be teleported back to their native country.
	IDS_INFILTRATION									= 161, // The victorious nation can invade the opposite nation.
	IDS_UNDER_ATTACK_KARUS								= 162, // Karus is being invaded.
	IDS_UNDER_ATTACK_ELMORAD							= 163, // El Morad is being invaded.
	IDS_INSERT_KNIGHTS_ALLIANCE							= 164, // The Knights %s has entered the confederacy.
	IDS_REMOVE_KNIGHTS_ALLIANCE							= 165, // The Knights %s has left the confederacy.
	IDS_PUNISH_KNIGHTS_ALLIANCE							= 166, // The alliance with the Knights %s has been canceled
	IDS_PERSONAL_GAME_VICTORY							= 167, // %s has won the fight
	IDS_DEATH_ANNOUNCEMENT								= 168, // *** %s has defeated %s ***
	IDS_SIEGE_WAR_READY_TIME							= 169, // Castle Siege War is about to begin. The designated Knights should enter the Castle Siege War zone.
	IDS_SIEGE_WAR_BASE_TIME								= 170, // This is a time for building your base. Please build your base.
	IDS_SIEGE_WAR_START									= 171, // The Castle Siege War has begun!
	IDS_SIEGE_WAR_END									= 172, // The Castle Siege War has ended!
	IDS_SIEGE_WAR_READY_TIME_NOTICE						= 173, // There are %d minutes remaining until the Castle Siege War. Knights should enter the CSW now.
	IDS_CHALLENGER_REQUEST_START_NOTICE					= 174, // We are currently accepting challengers for Castle Siege War. Location Moradon : 351,400
	IDS_CHALLENGER_REQUEST_END_NOTICE					= 175, // The registration period for Castle Siege War has now ended.
	IDS_SIEGE_WAR_GUIDON_TIME_NOTICE					= 176, // There are %d minutes remaining for building your base. If you don't build a base by then, you'll aut
	IDS_SIEGE_WAR_TIME_NOTICE							= 177, // There are %d minutes remaining in the war.
	IDS_NPC_GUIDON_DESTORY								= 178, // The base of the Knights %s has been destroyed.
	IDS_NPC_DESTORY_ARTIFACT							= 179, // Center Artifact has been destroyed
	IDS_NPC_DESTORY_ARTIFACT_S1							= 180, // Sentry Artifact has been destroyed
	IDS_NPC_DESTORY_ARTIFACT_S2							= 181, // Guard Tower Artifact has been destroyed
	IDS_NPC_DESTORY_ARTIFACT_S3							= 182, // Defense Artifact has been destroyed
	IDS_NPC_DESTORY_ARTIFACT_S4							= 184, // Shield Artifact has been destroyed
	IDS_GUARD_TOWER_DEATH_ANNOUNCEMENT					= 185, // *** %s was killed by %s ***
	IDS_SIEGE_WAR_VICTORY								= 186, // The clan %s is victorious!
	IDS_MONEY_REPAY_EVENT								= 187, // Coin event has started. You will earn an additional %d percent of coins for killing mobs.
	IDS_PRE_BATTLE_ANNOUNCEMENT							= 188, // There are 5 minutes remaining until the war commences.
	IDS_INFILTRATION_CONQUER							= 189, // %s has been overrun
	IDS_INFILTRATION_RECAPTURE							= 190, // %s has been captured
	IDS_INFILTRATION_REWARD_KARUS						= 191, // %s has blessed the Karus warriors
	IDS_INFILTRATION_REWARD_ELMORAD						= 192, // %s has blessed the El Morad warriors
	IDS_SERVER_AUTO_DOWN								= 193, // The server will shutdown after %d minutes.
	IDS_NPC_DESTORY_ARTIFACT_CLAN						= 194, // %s clan has destroyed the Center Artifact.
	IDS_ASSAULT_EVENT_START								= 195, // We're under attack by monsters. Please save the World from the monsters!!
	IDS_ASSAULT_EVENT_END								= 196, // The monsters have stopped. The World is safe now.
	IDS_EXP_REPAY_EVENT									= 197, // Experience point event Started. You will Earn %d Percent of Experience for killing Mobs.
	IDS_MONSTER_CHALLENGE_ANNOUNCEMENT					= 198, // The Forgotten Temple Event will start at %d for levels %d~%d
	IDS_MONSTER_CHALLENGE_OPEN							= 199, // The gate to the Forgotten Temple is now open.
	IDS_MONSTER_CHALLENGE_START							= 200, // The Monsters are appearing, Get ready!!!
	IDS_MONSTER_CHALLENGE_VICTORY						= 201, // You have eliminated all the monsters in the temple. You've received the treasure chest.
	IDS_MONSTER_CHALLENGE_CLOSE							= 202, // Forgotten Temple is now over. Users remaining in the Forgotten Temple zone will be Teleported out.
	IDS_MID_LEVEL_BATTLEZONE_OPEN						= 203, // Dark Lunar Gate is now open. The war has begun.
	IDS_KING_RECOMMEND_TIME								= 204, // Please nominate a King.
	IDS_KING_RECOMMEND_FINISH_TIME						= 205, // The period for nomination has ended.
	IDS_KING_ELECTION_TIME								= 206, // Election is in progress for selecting a King.
	IDS_KING_IMPEACHMENT_REQUEST_TIME					= 207, // A request for an impeachment of King has been made.
	IDS_KING_IMPEACHMENT_ELECTION_TIME					= 208, // election in progress for impeaching a King.
	IDS_KING_REIGN_TIME									= 209, // A new King has been appointed.
	IDS_KING_KARUS_PRIZE_EVENT_MESSAGE					= 210, // The Karus King has awarded %s %d coins as a reward.
	IDS_KING_KARUS_FUGITIVE_EVENT_MESSAGE_1				= 211, // The Karus King has awarded %s %d coins as a reward.
	IDS_KING_FUGITIVE_EVENT_MESSAGE_2					= 212, // %s killed %s and received %d coins as a reward.
	IDS_KING_ELMO_PRIZE_EVENT_MESSAGE					= 213, // The ElMorad King has awarded %s %d coins as a reward.
	IDS_KING_ELMO_FUGITIVE_EVENT_MESSAGE_1				= 214, // The El Morad King has awarded %s %d coins as a reward.
	IDS_KING_KARUS_WEATHER_FINE_EVENT					= 215, // The Karus King has cleared the weather.
	IDS_KING_KARUS_WEATHER_RAIN_EVENT					= 216, // The Karus King has summoned the rain clouds.
	IDS_KING_KARUS_WEATHER_SNOW_EVENT					= 217, // The Karus King has summoned the snow storm.
	IDS_KING_ELMO_WEATHER_FINE_EVENT					= 218, // The El Morad King has cleared the weather.
	IDS_KING_ELMO_WEATHER_RAIN_EVENT					= 219, // The El Morad King has summoned the rain clouds.
	IDS_KING_ELMO_WEATHER_SNOW_EVENT					= 220, // The El Morad King has summoned the snow storm.
	IDS_KING_KARUS_NOAH_BONUS_EVENT						= 221, // The Karus King has initiated the %d (percent) increased item drop rate event.
	IDS_KING_KARUS_EXP_BONUS_EVENT						= 222, // The Karus King has initiated the %d (percent) increased experience point event.
	IDS_KING_ELMO_NOAH_BONUS_EVENT						= 223, // The El Morad King has initiated the %d (percent) increased item drop rate event.
	IDS_KING_ELMO_EXP_BONUS_EVENT						= 224, // The El Morad King has initiated the %d (percent) increased experience point event.
	IDS_KING_RECOMMEND_REQUEST_MESSAGE					= 225, // User %s can nominate a King. Please nominate a King.
	IDS_KING_CANDIDACY_RECOMMEND_MESSAGE				= 226, // %s has nominated %s as a King.
	IDS_KING_PERIOD_OF_RECOMMEND_MESSAGE				= 227, // This is a period for nominating a King. Users with nomination rights should nominate a King.
	IDS_KING_PERIOD_OF_ELECTION_MESSAGE					= 228, // King election period is on. Users that have not voted yet should vote for a King.
	IDS_KING_ELECTION_RESULT_MESSAGE					= 229, // %d(percent) of the vote was won by %s to become a King.
	IDS_KING_EXP_BONUS_EVENT_STOP						= 230, // The increased experience point event has ended.
	IDS_KING_ITEMDROP_BONUS_EVENT_STOP					= 231, // The increased item drop rate event has ended.
	IDS_KING_IMPEACHMENT_REQUEST_MESSAGE				= 232, // Inpeachment Request In Progress
	IDS_KING_IMPEACHMENT_PASS_MESSAGE					= 233, // Inpeachment successfull
	IDS_KING_IMPEACHMENT_REJECT_MESSAGE					= 234, // Inpeachment has been Rejected
	IDS_KING_IMPEACHMENT_ELECTION_MESSAGE				= 235, // Please elect for your Inpeachment
	IDS_KING_IMPEACHMENT_ELECTION_YES_RESULT_MESSAGE	= 236, // Inpeachment Election has been accepted
	IDS_KING_IMPEACHMENT_ELECTION_NO_RESULT_MESSAGE		= 237, // Inpeachment Election has been Declined
	IDS_BEEF_ROAST_START								= 238, // Bifrost event has started!
	IDS_BEEF_ROAST_VICTORY_KARUS						= 239, // The Karus warriors have destroyed the monument and can now enter Bifrost!
	IDS_BEEF_ROAST_VICTORY_ELMORAD						= 240, // The El Morad warriors have destroyed the monument and can now enter Bifrost!
	IDS_BEEF_ROAST_FINISH								= 241, // Bifrost event is now over!
	IDS_BEEF_ROAST_DRAW									= 242, // Nobody has won the Bifrost event, it's a draw!
};

// clang-format on

#endif // SERVER_EBENEZER_DB_RESOURCES_H

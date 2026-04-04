#ifndef TESTS_SERVER_EBENEZER_PACKET_STRUCTS_H
#define TESTS_SERVER_EBENEZER_PACKET_STRUCTS_H

#pragma once

#include <shared/globals.h>
#include <shared/packets.h>

#pragma pack(push, 1)
struct ItemPosPair
{
	int32_t ID = 0;
	int8_t Pos = -1;
};

struct ItemUpgradeProcessPacket
{
	uint8_t SubOpcode    = ITEM_UPGRADE_PROCESS;
	int16_t NpcID        = 0;
	ItemPosPair Item[10] = {};
};

struct ItemUpgradeProcessErrorResponsePacket
{
	uint8_t Opcode    = WIZ_ITEM_UPGRADE;
	uint8_t SubOpcode = ITEM_UPGRADE_PROCESS;
	uint8_t Result    = 0;
};

struct ItemUpgradeProcessResponseSuccessPacket
{
	uint8_t Opcode       = WIZ_ITEM_UPGRADE;
	uint8_t SubOpcode    = ITEM_UPGRADE_PROCESS;
	uint8_t Result       = 1;
	ItemPosPair Item[10] = {};
};

struct GoldChangePacket
{
	uint8_t Opcode        = WIZ_GOLD_CHANGE;
	uint8_t SubOpcode     = GOLD_CHANGE_LOSE;
	uint32_t ChangeAmount = 0;
	uint32_t NewGold      = 0;
};

struct ObjectEventAnvilResponsePacket
{
	uint8_t Opcode     = WIZ_OBJECT_EVENT;
	uint8_t ObjectType = OBJECT_TYPE_ANVIL;
	bool Successful    = true;
	int16_t NpcID      = 0;
};

struct WeightChangePacket
{
	uint8_t Opcode        = WIZ_WEIGHT_CHANGE;
	int16_t CurrentWeight = 0;
};

struct ItemCountChangePacket
{
	uint8_t Opcode     = WIZ_ITEM_COUNT_CHANGE;
	int16_t Items      = 1;
	uint8_t District   = 1;
	uint8_t SlotNumber = 0;
	int32_t ItemId     = 0;
	int32_t ItemCount  = 0;
	uint8_t ChangeType = ITEM_COUNT_CHANGE_NEW;
	int16_t Durability = 0;
};

/// \brief packet definition for WIZ_NPC_SAY
struct SendSayPacket
{
	uint8_t opCode    = WIZ_NPC_SAY;
	int32_t eventIdUp = Ebenezer::EVENTID_NULL;
	int32_t eventIdOk = Ebenezer::EVENTID_NULL;
	int32_t message1  = Ebenezer::MSG_NULL;
	int32_t message2  = Ebenezer::MSG_NULL;
	int32_t message3  = Ebenezer::MSG_NULL;
	int32_t message4  = Ebenezer::MSG_NULL;
	int32_t message5  = Ebenezer::MSG_NULL;
	int32_t message6  = Ebenezer::MSG_NULL;
	int32_t message7  = Ebenezer::MSG_NULL;
	int32_t message8  = Ebenezer::MSG_NULL;
};

/// \brief Subpacket definition for DB_CUSTOM_STIPEND_REQUEST
struct StipendRequestSubPacket
{
	uint8_t stipendType  = STIPEND_TYPE_USER_KNIGHTS;
	uint8_t rank         = Ebenezer::RANK_INVALID;
	uint8_t nation       = Ebenezer::NATION_KARUS;
	uint8_t charIdLength = 0;
	char charId[MAX_ID_SIZE + 1] {};
};

/// \brief Subpacket definition for DB_CUSTOM_STIPEND_RESPONSE
struct StipendResponseSubPacket
{
	uint8_t stipendType  = STIPEND_TYPE_USER_KNIGHTS;
	uint8_t responseCode = STIPEND_RESPONSE_ERROR;
	uint8_t rank         = Ebenezer::RANK_INVALID;
	uint8_t nation       = Ebenezer::NATION_KARUS;
	uint8_t charIdLength = 0;
	char charId[MAX_ID_SIZE + 1] {};
};
#pragma pack(pop)

#endif // TESTS_SERVER_EBENEZER_PACKET_STRUCTS_H

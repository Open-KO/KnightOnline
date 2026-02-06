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
#pragma pack(pop)

#endif // TESTS_SERVER_EBENEZER_PACKET_STRUCTS_H

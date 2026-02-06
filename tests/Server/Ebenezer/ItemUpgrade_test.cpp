#include <gtest/gtest.h>
#include "TestApp.h"
#include "TestUser.h"
#include "packet_structs.h"

#include "data/Item_test_data.h"
#include "data/ItemUpgrade_test_data.h"

#include <shared-server/utilities.h>

#include <cstdlib>
#include <memory> // std::memcpy()
#include <utility>

using namespace Ebenezer;

class ItemUpgradeTest : public ::testing::TestWithParam<std::tuple<int, int, int>>
{
protected:
	static constexpr uint16_t ANVIL_NPC_ID     = 10001;
	static constexpr uint16_t NOT_ANVIL_NPC_ID = 10002;

	static constexpr uint8_t ZONE_ID           = 0;
	static constexpr uint16_t REGION_X         = 0;
	static constexpr uint16_t REGION_Z         = 0;

	std::unique_ptr<TestApp> _app;
	TestMap* _map                   = nullptr;
	std::shared_ptr<TestUser> _user = nullptr;
	Ebenezer::CNpc* _anvilNpc       = nullptr;

	void SetUp() override
	{
		_app = std::make_unique<TestApp>();
		ASSERT_TRUE(_app != nullptr);

		// Load required tables
		for (const auto& itemModel : s_itemData)
			ASSERT_TRUE(_app->AddItemEntry(itemModel));

		// Pre-reserve data so we only need to allocate once.
		_app->m_ItemUpgradeTableArray.reserve(
			sizeof(s_itemUpgradeData) / sizeof(s_itemUpgradeData[0]));

		for (const auto& itemUpgradeModel : s_itemUpgradeData)
			ASSERT_TRUE(_app->AddItemUpgradeEntry(itemUpgradeModel));

		// Setup map
		_map = _app->CreateMap(ZONE_ID);
		ASSERT_TRUE(_map != nullptr);

		// Setup user
		_user = _app->AddUser();
		ASSERT_TRUE(_user != nullptr);

		// Mark player as ingame
		_user->SetState(CONNECTION_STATE_GAMESTART);

		// Add user to map
		ASSERT_TRUE(_map->Add(_user.get(), REGION_X, REGION_Z));

		// Setup anvil NPC
		_anvilNpc = _app->CreateNPC(ANVIL_NPC_ID);
		ASSERT_TRUE(_anvilNpc != nullptr);

		// Set as anvil type
		_anvilNpc->m_tNpcType = NPC_ANVIL;

		// Add NPC to map
		ASSERT_TRUE(_map->Add(_anvilNpc, REGION_X, REGION_Z));
	}

	void TearDown() override
	{
		_user.reset();
		_anvilNpc = nullptr;
		_map      = nullptr;
		_app.reset();
	}
};

INSTANTIATE_TEST_SUITE_P(UpgradeSuccessTestCases, ItemUpgradeTest,
	testing::Values( //
		//               Old item | New item | Cost
		std::make_tuple(110110001, 110110002, 0),     // Dagger (+1) -> Dagger (+2)
		std::make_tuple(111210001, 111210002, 0),     // Shard (+1) -> Shard (+2)
		std::make_tuple(156210001, 156210002, 0),     // Raptor (+1) -> Raptor (+2)
		std::make_tuple(126410001, 126410002, 0),     // Mirage (+1) -> Mirage (+2)
		std::make_tuple(181110001, 181110002, 0),     // Elixir Staff (+1) Elixir Staff (+2)
		std::make_tuple(190250271, 190250272, 240000) // Lobo hammer (+1) -> Lobo hammer (+2)
		));

TEST_P(ItemUpgradeTest, BasicUpgradeSucceeds)
{
	auto [OLD_ITEM_ID, NEW_ITEM_ID, EXPECTED_COST] = GetParam();
	constexpr int REQ_ITEM1_ID                     = 379152000; // Blessed Item Upgrade Scroll
	constexpr int START_GOLD                       = 100'000'000;
	int EXPECTED_NEW_GOLD                          = START_GOLD - EXPECTED_COST;

	char sendBuffer[128] {};
	ItemUpgradeProcessPacket packet {};

	_ITEM_DATA& originItem    = _user->m_pUserData->m_sItemArray[SLOT_MAX + 0];
	_ITEM_DATA& reqItem1      = _user->m_pUserData->m_sItemArray[SLOT_MAX + 1];

	model::Item* oldItemModel = _app->m_ItemTableMap.GetData(OLD_ITEM_ID);
	model::Item* newItemModel = _app->m_ItemTableMap.GetData(NEW_ITEM_ID);

	ASSERT_TRUE(oldItemModel != nullptr);
	ASSERT_TRUE(newItemModel != nullptr);

	// The upgrade rate works in reverse from how you'd expect.
	// For 100%, we'd have to return min (0), not max (10000).
	myrand                      = [](int min, int) { return min; };

	// Prepare inventory
	originItem                  = { .nNum = OLD_ITEM_ID, .sDuration = 1, .sCount = 1 };
	reqItem1                    = { .nNum = REQ_ITEM1_ID, .sCount = 1 };

	// Upgrades need gold
	_user->m_pUserData->m_iGold = START_GOLD;

	// Prepare packet data
	packet.NpcID                = ANVIL_NPC_ID;
	packet.Item[0]              = { .ID = OLD_ITEM_ID, .Pos = 0 };
	packet.Item[1]              = { .ID = REQ_ITEM1_ID, .Pos = 1 };

	_user->ResetSend();

	// Expect the gold change packet
	_user->AddSendCallback(
		[=](const char* pBuf, int len)
		{
			ASSERT_EQ(len, sizeof(GoldChangePacket));

			auto packet = reinterpret_cast<const GoldChangePacket*>(pBuf);

			EXPECT_EQ(packet->Opcode, WIZ_GOLD_CHANGE);
			EXPECT_EQ(packet->SubOpcode, GOLD_CHANGE_LOSE);
			EXPECT_EQ(packet->ChangeAmount, EXPECTED_COST);
			EXPECT_EQ(packet->NewGold, EXPECTED_NEW_GOLD);
		});

	// Then the success packet
	_user->AddSendCallback(
		[=](const char* pBuf, int len)
		{
			ASSERT_EQ(len, sizeof(ItemUpgradeProcessResponseSuccessPacket));

			auto packet = reinterpret_cast<const ItemUpgradeProcessResponseSuccessPacket*>(pBuf);

			EXPECT_EQ(packet->Opcode, WIZ_ITEM_UPGRADE);
			EXPECT_EQ(packet->SubOpcode, ITEM_UPGRADE_PROCESS);
			EXPECT_EQ(packet->Result, ITEM_UPGRADE_ERROR_SUCCEEDED);
			EXPECT_EQ(packet->Item[0].ID, NEW_ITEM_ID);
			EXPECT_EQ(packet->Item[0].Pos, 0);
			EXPECT_EQ(packet->Item[1].ID, REQ_ITEM1_ID);
			EXPECT_EQ(packet->Item[1].Pos, 1);

			for (int i = 2; i < 10; i++)
			{
				EXPECT_EQ(packet->Item[i].ID, 0);
				EXPECT_EQ(packet->Item[i].Pos, -1);
			}
		});

	// Then the packet to show the visual effect for the anvil
	_user->AddSendCallback(
		[](const char* pBuf, int len)
		{
			ASSERT_EQ(len, sizeof(ObjectEventAnvilResponsePacket));

			auto packet = reinterpret_cast<const ObjectEventAnvilResponsePacket*>(pBuf);

			EXPECT_EQ(packet->Opcode, WIZ_OBJECT_EVENT);
			EXPECT_EQ(packet->ObjectType, OBJECT_TYPE_ANVIL);
			EXPECT_TRUE(packet->Successful);
			EXPECT_EQ(packet->NpcID, ANVIL_NPC_ID);
		});

	// Copy it into the larger buffer in case it were to ever read beyond the struct's size.
	std::memcpy(sendBuffer, &packet, sizeof(packet));
	_user->ItemUpgradeProcess(sendBuffer);

	EXPECT_EQ(_user->GetPacketsSent(), 3);

	// Verify the item ID was updated in the inventory
	EXPECT_EQ(originItem.nNum, NEW_ITEM_ID);

	// Verify its durability was restored to max
	EXPECT_EQ(originItem.sDuration, newItemModel->Durability);
}

TEST_F(ItemUpgradeTest, BasicUpgradeBurns)
{
	constexpr int OLD_ITEM_ID       = 110110007; // Dagger (+7)
	constexpr int REQ_ITEM1_ID      = 379021000; // Blessed Upgrade Scroll (+0)
	constexpr int START_GOLD        = 100'000'000;
	constexpr int EXPECTED_COST     = 0;
	constexpr int EXPECTED_NEW_GOLD = START_GOLD - EXPECTED_COST;

	char sendBuffer[128] {};
	ItemUpgradeProcessPacket packet {};

	_ITEM_DATA& originItem    = _user->m_pUserData->m_sItemArray[SLOT_MAX + 0];
	_ITEM_DATA& reqItem1      = _user->m_pUserData->m_sItemArray[SLOT_MAX + 1];

	model::Item* oldItemModel = _app->m_ItemTableMap.GetData(OLD_ITEM_ID);
	EXPECT_TRUE(oldItemModel != nullptr);

	// The upgrade rate works in reverse from how you'd expect.
	// For 0%, we'd have to return max (10000), not min (0).
	myrand     = [](int, int max) { return max; };

	// Prepare inventory
	originItem = { .nNum = OLD_ITEM_ID, .sDuration = 1, .sCount = 1, .nSerialNum = 123456789 };
	reqItem1   = { .nNum = REQ_ITEM1_ID, .sCount = 1 };

	// Upgrades need gold
	_user->m_pUserData->m_iGold = START_GOLD;

	// Prepare packet data
	packet.NpcID                = ANVIL_NPC_ID;
	packet.Item[0]              = { .ID = OLD_ITEM_ID, .Pos = 0 };
	packet.Item[1]              = { .ID = REQ_ITEM1_ID, .Pos = 1 };

	_user->ResetSend();

	// Expect the gold change packet
	_user->AddSendCallback(
		[=](const char* pBuf, int len)
		{
			ASSERT_EQ(len, sizeof(GoldChangePacket));

			auto packet = reinterpret_cast<const GoldChangePacket*>(pBuf);

			EXPECT_EQ(packet->Opcode, WIZ_GOLD_CHANGE);
			EXPECT_EQ(packet->SubOpcode, GOLD_CHANGE_LOSE);
			EXPECT_EQ(packet->ChangeAmount, EXPECTED_COST);
			EXPECT_EQ(packet->NewGold, EXPECTED_NEW_GOLD);
		});

	// Then the fail packet
	_user->AddSendCallback(
		[=](const char* pBuf, int len)
		{
			ASSERT_EQ(len, sizeof(ItemUpgradeProcessResponseSuccessPacket));

			auto packet = reinterpret_cast<const ItemUpgradeProcessResponseSuccessPacket*>(pBuf);

			EXPECT_EQ(packet->Opcode, WIZ_ITEM_UPGRADE);
			EXPECT_EQ(packet->SubOpcode, ITEM_UPGRADE_PROCESS);
			EXPECT_EQ(packet->Result, ITEM_UPGRADE_ERROR_FAILED);
			EXPECT_EQ(packet->Item[0].ID, OLD_ITEM_ID);
			EXPECT_EQ(packet->Item[0].Pos, 0);
			EXPECT_EQ(packet->Item[1].ID, REQ_ITEM1_ID);
			EXPECT_EQ(packet->Item[1].Pos, 1);

			for (int i = 2; i < 10; i++)
			{
				EXPECT_EQ(packet->Item[i].ID, 0);
				EXPECT_EQ(packet->Item[i].Pos, -1);
			}
		});

	// Then the packet to show the visual effect for the anvil
	_user->AddSendCallback(
		[](const char* pBuf, int len)
		{
			ASSERT_EQ(len, sizeof(ObjectEventAnvilResponsePacket));

			auto packet = reinterpret_cast<const ObjectEventAnvilResponsePacket*>(pBuf);

			EXPECT_EQ(packet->Opcode, WIZ_OBJECT_EVENT);
			EXPECT_EQ(packet->ObjectType, OBJECT_TYPE_ANVIL);
			EXPECT_FALSE(packet->Successful);
			EXPECT_EQ(packet->NpcID, ANVIL_NPC_ID);
		});

	// Copy it into the larger buffer in case it were to ever read beyond the struct's size.
	std::memcpy(sendBuffer, &packet, sizeof(packet));
	_user->ItemUpgradeProcess(sendBuffer);

	EXPECT_EQ(_user->GetPacketsSent(), 3);

	// Verify the item was removed from the inventory
	EXPECT_EQ(originItem.nNum, 0);
	EXPECT_EQ(originItem.sDuration, 0);
	EXPECT_EQ(originItem.sCount, 0);
	EXPECT_EQ(originItem.nSerialNum, 0);
}

TEST_F(ItemUpgradeTest, OriginItemNotInInventoryDropped)
{
	constexpr int OLD_ITEM_ID  = 110110001; // Dagger (+1)
	constexpr int REQ_ITEM1_ID = 379016000; // Blessed Item Upgrade Scroll
	constexpr int START_GOLD   = 100'000'000;

	char sendBuffer[128] {};
	ItemUpgradeProcessPacket packet {};

	_ITEM_DATA& originItem      = _user->m_pUserData->m_sItemArray[SLOT_MAX + 0];
	_ITEM_DATA& reqItem1        = _user->m_pUserData->m_sItemArray[SLOT_MAX + 1];

	// Origin item purposefully doesn't exist in the inventory
	originItem                  = { .nNum = 0, .sCount = 0 };
	reqItem1                    = { .nNum = REQ_ITEM1_ID, .sCount = 1 };

	_user->m_pUserData->m_iGold = START_GOLD;

	// Prepare packet data
	packet.NpcID                = ANVIL_NPC_ID;
	packet.Item[0]              = { .ID = OLD_ITEM_ID, .Pos = 0 };
	packet.Item[1]              = { .ID = REQ_ITEM1_ID, .Pos = 1 };

	_user->ResetSend();

	// Copy it into the larger buffer in case it were to ever read beyond the struct's size.
	std::memcpy(sendBuffer, &packet, sizeof(packet));
	_user->ItemUpgradeProcess(sendBuffer);

	EXPECT_EQ(_user->GetPacketsSent(), 0);
}

TEST_F(ItemUpgradeTest, RequirementItemNotInInventoryDropped)
{
	constexpr int OLD_ITEM_ID  = 110110001; // Dagger (+1)
	constexpr int REQ_ITEM1_ID = 379016000; // Blessed Item Upgrade Scroll
	constexpr int START_GOLD   = 100'000'000;

	char sendBuffer[128] {};
	ItemUpgradeProcessPacket packet {};

	_ITEM_DATA& originItem      = _user->m_pUserData->m_sItemArray[SLOT_MAX + 0];
	_ITEM_DATA& reqItem1        = _user->m_pUserData->m_sItemArray[SLOT_MAX + 1];

	// Requirement item purposefully doesn't exist in the inventory
	originItem                  = { .nNum = OLD_ITEM_ID, .sCount = 1 };
	reqItem1                    = { .nNum = 0, .sCount = 0 };

	_user->m_pUserData->m_iGold = START_GOLD;

	// Prepare packet data
	packet.NpcID                = ANVIL_NPC_ID;
	packet.Item[0]              = { .ID = OLD_ITEM_ID, .Pos = 0 };
	packet.Item[1]              = { .ID = REQ_ITEM1_ID, .Pos = 1 };

	_user->ResetSend();

	// Copy it into the larger buffer in case it were to ever read beyond the struct's size.
	std::memcpy(sendBuffer, &packet, sizeof(packet));
	_user->ItemUpgradeProcess(sendBuffer);

	EXPECT_EQ(_user->GetPacketsSent(), 0);
}

TEST_F(ItemUpgradeTest, InsufficientGoldRejected)
{
	constexpr int OLD_ITEM_ID  = 110110001; // Dagger (+1)
	constexpr int REQ_ITEM1_ID = 379025000; // Blessed Elemental Scroll
	constexpr int START_GOLD   = -100;      // -100 is not enough for an upgrade

	char sendBuffer[128] {};
	ItemUpgradeProcessPacket packet {};
	_ITEM_DATA& originItem      = _user->m_pUserData->m_sItemArray[SLOT_MAX + 0];
	_ITEM_DATA& reqItem1        = _user->m_pUserData->m_sItemArray[SLOT_MAX + 1];

	// Prepare inventory
	originItem                  = { .nNum = OLD_ITEM_ID, .sCount = 1 };
	reqItem1                    = { .nNum = REQ_ITEM1_ID, .sCount = 1 };

	// Set gold to -100 - not enough for upgrade
	_user->m_pUserData->m_iGold = START_GOLD;

	// Prepare packet data
	packet.NpcID                = ANVIL_NPC_ID;
	packet.Item[0]              = { .ID = OLD_ITEM_ID, .Pos = 0 };
	packet.Item[1]              = { .ID = REQ_ITEM1_ID, .Pos = 1 };

	_user->ResetSend();
	_user->AddSendCallback(
		[](const char* pBuf, int len)
		{
			ASSERT_EQ(len, sizeof(ItemUpgradeProcessErrorResponsePacket));

			auto packet = reinterpret_cast<const ItemUpgradeProcessErrorResponsePacket*>(pBuf);
			EXPECT_EQ(packet->Opcode, WIZ_ITEM_UPGRADE);
			EXPECT_EQ(packet->SubOpcode, ITEM_UPGRADE_PROCESS);
			EXPECT_EQ(packet->Result, ITEM_UPGRADE_ERROR_NEED_COINS);
		});

	// Copy it into the larger buffer in case it were to ever read beyond the struct's size.
	std::memcpy(sendBuffer, &packet, sizeof(packet));
	_user->ItemUpgradeProcess(sendBuffer);

	EXPECT_EQ(_user->GetPacketsSent(), 1);
}

TEST_F(ItemUpgradeTest, MissingNpcDropped)
{
	constexpr int OLD_ITEM_ID  = 110110001; // Dagger (+1)
	constexpr int REQ_ITEM1_ID = 379025000; // Blessed Elemental Scroll

	char sendBuffer[128] {};
	ItemUpgradeProcessPacket packet {};

	_ITEM_DATA& originItem = _user->m_pUserData->m_sItemArray[SLOT_MAX + 0];
	_ITEM_DATA& reqItem1   = _user->m_pUserData->m_sItemArray[SLOT_MAX + 1];

	// Prepare inventory (we want to have a fully valid request otherwise)
	originItem             = { .nNum = OLD_ITEM_ID, .sCount = 1 };
	reqItem1               = { .nNum = REQ_ITEM1_ID, .sCount = 1 };

	// Prepare packet data
	packet.NpcID           = -1; // doesn't exist
	packet.Item[0]         = { .ID = OLD_ITEM_ID, .Pos = 0 };
	packet.Item[1]         = { .ID = REQ_ITEM1_ID, .Pos = 1 };

	_user->ResetSend();

	// Copy it into the larger buffer in case it were to ever read beyond the struct's size.
	std::memcpy(sendBuffer, &packet, sizeof(packet));
	_user->ItemUpgradeProcess(sendBuffer);

	// Packet is ignored.
	EXPECT_EQ(_user->GetPacketsSent(), 0);
}

TEST_F(ItemUpgradeTest, WrongNpcTypeDropped)
{
	constexpr int OLD_ITEM_ID  = 110110001; // Dagger (+1)
	constexpr int REQ_ITEM1_ID = 379025000; // Blessed Elemental Scroll

	char sendBuffer[128] {};
	ItemUpgradeProcessPacket packet {};

	_ITEM_DATA& originItem = _user->m_pUserData->m_sItemArray[SLOT_MAX + 0];
	_ITEM_DATA& reqItem1   = _user->m_pUserData->m_sItemArray[SLOT_MAX + 1];

	// Prepare inventory (we want to have a fully valid request otherwise)
	originItem             = { .nNum = OLD_ITEM_ID, .sCount = 1 };
	reqItem1               = { .nNum = REQ_ITEM1_ID, .sCount = 1 };

	// Prepare packet data
	packet.NpcID           = NOT_ANVIL_NPC_ID;
	packet.Item[0]         = { .ID = OLD_ITEM_ID, .Pos = 0 };
	packet.Item[1]         = { .ID = REQ_ITEM1_ID, .Pos = 1 };

	// Setup NPC that isn't the anvil
	CNpc* notAnvilNpc      = _app->CreateNPC(NOT_ANVIL_NPC_ID);
	ASSERT_TRUE(notAnvilNpc != nullptr);

	// Set NPC type to indicate it's not the anvil
	notAnvilNpc->m_tNpcType = NPC_ARENA;

	// Add NPC to map
	ASSERT_TRUE(_map->Add(notAnvilNpc, REGION_X, REGION_Z));

	_user->ResetSend();

	// Copy it into the larger buffer in case it were to ever read beyond the struct's size.
	std::memcpy(sendBuffer, &packet, sizeof(packet));
	_user->ItemUpgradeProcess(sendBuffer);

	// Packet is ignored.
	EXPECT_EQ(_user->GetPacketsSent(), 0);
}

TEST_F(ItemUpgradeTest, NotInZoneDropped)
{
	constexpr int OLD_ITEM_ID  = 110110001; // Dagger (+1)
	constexpr int REQ_ITEM1_ID = 379025000; // Blessed Elemental Scroll

	char sendBuffer[128] {};
	ItemUpgradeProcessPacket packet {};

	_ITEM_DATA& originItem      = _user->m_pUserData->m_sItemArray[SLOT_MAX + 0];
	_ITEM_DATA& reqItem1        = _user->m_pUserData->m_sItemArray[SLOT_MAX + 1];

	// Prepare inventory (we want to have a fully valid request otherwise)
	originItem                  = { .nNum = OLD_ITEM_ID, .sCount = 1 };
	reqItem1                    = { .nNum = REQ_ITEM1_ID, .sCount = 1 };

	// Prepare packet data
	packet.NpcID                = ANVIL_NPC_ID;
	packet.Item[0]              = { .ID = OLD_ITEM_ID, .Pos = 0 };
	packet.Item[1]              = { .ID = REQ_ITEM1_ID, .Pos = 1 };

	_user->m_pUserData->m_bZone = ZONE_ID;

	// Explicitly set the NPC to another zone.
	_anvilNpc->m_sCurZone       = ZONE_ID + 1;

	_user->ResetSend();

	// Copy it into the larger buffer in case it were to ever read beyond the struct's size.
	std::memcpy(sendBuffer, &packet, sizeof(packet));
	_user->ItemUpgradeProcess(sendBuffer);

	// Packet is ignored.
	EXPECT_EQ(_user->GetPacketsSent(), 0);
}

TEST_F(ItemUpgradeTest, OutOfRangeDropped)
{
	constexpr int OLD_ITEM_ID  = 110110001; // Dagger (+1)
	constexpr int REQ_ITEM1_ID = 379025000; // Blessed Elemental Scroll

	char sendBuffer[128] {};
	ItemUpgradeProcessPacket packet {};

	_ITEM_DATA& originItem     = _user->m_pUserData->m_sItemArray[SLOT_MAX + 0];
	_ITEM_DATA& reqItem1       = _user->m_pUserData->m_sItemArray[SLOT_MAX + 1];

	// Prepare inventory (we want to have a fully valid request otherwise)
	originItem                 = { .nNum = OLD_ITEM_ID, .sCount = 1 };
	reqItem1                   = { .nNum = REQ_ITEM1_ID, .sCount = 1 };

	// Prepare packet data
	packet.NpcID               = ANVIL_NPC_ID;
	packet.Item[0]             = { .ID = OLD_ITEM_ID, .Pos = 0 };
	packet.Item[1]             = { .ID = REQ_ITEM1_ID, .Pos = 1 };

	// Explicitly set us to 20,20.
	_user->m_pUserData->m_curx = 20.0f;
	_user->m_pUserData->m_curz = 20.0f;

	// Explicitly set the anvil NPC to 100,100.
	// This is more than far enough away.
	_anvilNpc->m_fCurX         = 100.0f;
	_anvilNpc->m_fCurZ         = 100.0f;

	_user->ResetSend();

	// Copy it into the larger buffer in case it were to ever read beyond the struct's size.
	std::memcpy(sendBuffer, &packet, sizeof(packet));
	_user->ItemUpgradeProcess(sendBuffer);

	// Packet is ignored.
	EXPECT_EQ(_user->GetPacketsSent(), 0);
}

TEST_F(ItemUpgradeTest, UserTradingRejected)
{
	constexpr int OLD_ITEM_ID  = 110110001; // Dagger (+1)
	constexpr int REQ_ITEM1_ID = 379016000; // Blessed Item Upgrade Scroll
	constexpr int START_GOLD   = 100'000'000;

	char sendBuffer[128] {};
	ItemUpgradeProcessPacket packet {};

	_ITEM_DATA& originItem      = _user->m_pUserData->m_sItemArray[SLOT_MAX + 0];
	_ITEM_DATA& reqItem1        = _user->m_pUserData->m_sItemArray[SLOT_MAX + 1];

	// Prepare inventory
	originItem                  = { .nNum = OLD_ITEM_ID, .sCount = 1 };
	reqItem1                    = { .nNum = REQ_ITEM1_ID, .sCount = 1 };

	_user->m_sExchangeUser      = 1; // assign a user for trading
	_user->m_pUserData->m_iGold = START_GOLD;

	// Prepare packet data
	packet.NpcID                = ANVIL_NPC_ID;
	packet.Item[0]              = { .ID = OLD_ITEM_ID, .Pos = 0 };
	packet.Item[1]              = { .ID = REQ_ITEM1_ID, .Pos = 1 };

	_user->ResetSend();
	_user->AddSendCallback(
		[](const char* pBuf, int len)
		{
			ASSERT_EQ(len, sizeof(ItemUpgradeProcessErrorResponsePacket));

			auto packet = reinterpret_cast<const ItemUpgradeProcessErrorResponsePacket*>(pBuf);
			EXPECT_EQ(packet->Opcode, WIZ_ITEM_UPGRADE);
			EXPECT_EQ(packet->SubOpcode, ITEM_UPGRADE_PROCESS);
			EXPECT_EQ(packet->Result, ITEM_UPGRADE_ERROR_TRADING);
		});

	// Copy it into the larger buffer in case it were to ever read beyond the struct's size.
	std::memcpy(sendBuffer, &packet, sizeof(packet));
	_user->ItemUpgradeProcess(sendBuffer);

	EXPECT_EQ(_user->GetPacketsSent(), 1);
}

TEST_F(ItemUpgradeTest, RentalItemRejected)
{
	constexpr int OLD_ITEM_ID  = 110110001; // Dagger (+1)
	constexpr int REQ_ITEM1_ID = 379016000; // Blessed Item Upgrade Scroll
	constexpr int START_GOLD   = 100'000'000;

	char sendBuffer[128] {};
	ItemUpgradeProcessPacket packet {};

	_ITEM_DATA& originItem      = _user->m_pUserData->m_sItemArray[SLOT_MAX + 0];
	_ITEM_DATA& reqItem1        = _user->m_pUserData->m_sItemArray[SLOT_MAX + 1];

	// Prepare inventory
	originItem                  = { .nNum = OLD_ITEM_ID, .sCount = 1, .byFlag = ITEM_FLAG_RENTED };
	reqItem1                    = { .nNum = REQ_ITEM1_ID, .sCount = 1 };

	_user->m_pUserData->m_iGold = START_GOLD;

	// Prepare packet data
	packet.NpcID                = ANVIL_NPC_ID;
	packet.Item[0]              = { .ID = OLD_ITEM_ID, .Pos = 0 };
	packet.Item[1]              = { .ID = REQ_ITEM1_ID, .Pos = 1 };

	_user->ResetSend();
	_user->AddSendCallback(
		[](const char* pBuf, int len)
		{
			ASSERT_EQ(len, sizeof(ItemUpgradeProcessErrorResponsePacket));

			auto packet = reinterpret_cast<const ItemUpgradeProcessErrorResponsePacket*>(pBuf);
			EXPECT_EQ(packet->Opcode, WIZ_ITEM_UPGRADE);
			EXPECT_EQ(packet->SubOpcode, ITEM_UPGRADE_PROCESS);
			EXPECT_EQ(packet->Result, ITEM_UPGRADE_ERROR_ITEM_RENTED);
		});

	// Copy it into the larger buffer in case it were to ever read beyond the struct's size.
	std::memcpy(sendBuffer, &packet, sizeof(packet));
	_user->ItemUpgradeProcess(sendBuffer);

	EXPECT_EQ(_user->GetPacketsSent(), 1);
}

TEST_F(ItemUpgradeTest, MalformedPositionsDropped)
{
	constexpr int REQ_ITEM1_ID = 379016000; // Blessed Item Upgrade Scroll
	constexpr int START_GOLD   = 100'000'000;

	char sendBuffer[128] {};
	ItemUpgradeProcessPacket packet {};

	// Prepare packet data
	packet.NpcID = ANVIL_NPC_ID;

	// Prepare initially with valid positions
	for (int8_t i = 0; i < 10; i++)
	{
		// Prepare inventory
		_ITEM_DATA& reqItem = _user->m_pUserData->m_sItemArray[SLOT_MAX + i];
		reqItem             = { .nNum = REQ_ITEM1_ID, .sCount = 1 };
		packet.Item[i]      = { .ID = REQ_ITEM1_ID, .Pos = i };
	}

	// Now run 10 times with only the one position broken. It should fail on each individually.
	for (int8_t i = 0; i < 10; i++)
	{
		// Restore previous in loop.
		if (i > 0)
			packet.Item[i - 1].Pos = i - 1;

		packet.Item[i].Pos          = -20;

		// Reset gold
		_user->m_pUserData->m_iGold = START_GOLD;

		_user->ResetSend();

		// Copy it into the larger buffer in case it were to ever read beyond the struct's size.
		std::memcpy(sendBuffer, &packet, sizeof(packet));
		_user->ItemUpgradeProcess(sendBuffer);

		EXPECT_EQ(_user->GetPacketsSent(), 0);
	}
}

TEST_F(ItemUpgradeTest, ReusedPositionsDropped)
{
	constexpr int REQ_ITEM1_ID = 379016000; // Blessed Item Upgrade Scroll
	constexpr int START_GOLD   = 100'000'000;

	char sendBuffer[128] {};
	ItemUpgradeProcessPacket packet {};

	// Prepare packet data
	packet.NpcID = ANVIL_NPC_ID;

	// Prepare initially with valid positions
	for (int8_t i = 0; i < 10; i++)
	{
		// Prepare inventory
		_ITEM_DATA& reqItem = _user->m_pUserData->m_sItemArray[SLOT_MAX + i];
		reqItem             = { .nNum = REQ_ITEM1_ID, .sCount = 1 };
		packet.Item[i]      = { .ID = REQ_ITEM1_ID, .Pos = i };
	}

	// Now run 10 times with only the one position reused. It should fail on each individually.
	for (int8_t i = 0; i < 10; i++)
	{
		if (i > 0)
		{
			// Restore previous in loop.
			packet.Item[i - 1].Pos = i - 1;
			packet.Item[i].Pos     = i - 1; // reuse previous position
		}
		else
		{
			packet.Item[i].Pos = i + 1; // reuse next position
		}

		// Reset gold
		_user->m_pUserData->m_iGold = START_GOLD;

		_user->ResetSend();

		// Copy it into the larger buffer in case it were to ever read beyond the struct's size.
		std::memcpy(sendBuffer, &packet, sizeof(packet));
		_user->ItemUpgradeProcess(sendBuffer);

		EXPECT_EQ(_user->GetPacketsSent(), 0);
	}
}

TEST_F(ItemUpgradeTest, MalformedItemIDsDropped)
{
	constexpr int REQ_ITEM1_ID = 379016000; // Blessed Item Upgrade Scroll
	constexpr int START_GOLD   = 100'000'000;

	char sendBuffer[128] {};
	ItemUpgradeProcessPacket packet {};

	// Prepare packet data
	packet.NpcID = ANVIL_NPC_ID;

	// Prepare initially with valid positions
	for (int8_t i = 0; i < 10; i++)
	{
		// Prepare inventory
		_ITEM_DATA& reqItem = _user->m_pUserData->m_sItemArray[SLOT_MAX + i];
		reqItem             = { .nNum = REQ_ITEM1_ID, .sCount = 1 };
		packet.Item[i]      = { .ID = REQ_ITEM1_ID, .Pos = i };
	}

	// Now run 10 times with only the one item ID broken. It should fail on each individually.
	for (int8_t i = 0; i < 10; i++)
	{
		// Restore previous in loop.
		if (i > 0)
			packet.Item[i - 1].ID = REQ_ITEM1_ID;

		packet.Item[i].ID           = 123; // not a valid item ID

		// Reset gold
		_user->m_pUserData->m_iGold = START_GOLD;

		_user->ResetSend();

		// Copy it into the larger buffer in case it were to ever read beyond the struct's size.
		std::memcpy(sendBuffer, &packet, sizeof(packet));
		_user->ItemUpgradeProcess(sendBuffer);

		EXPECT_EQ(_user->GetPacketsSent(), 0);
	}
}

#include <gtest/gtest.h>
#include "TestApp.h"
#include "TestUser.h"
#include "packet_structs.h"

#include "data/Item_test_data.h"
#include "data/ItemExchange_test_data.h"

#include <cstdlib>
#include <memory> // std::memcpy()

using namespace Ebenezer;

class ItemExchangeTest : public testing::Test
{
protected:
	std::unique_ptr<TestApp> _app                  = nullptr;
	std::shared_ptr<TestUser> _user                = nullptr;

	static constexpr int ITEM_GOLD_BAR             = 379068000;
	static constexpr int ITEM_HP_POTION            = 389011000;
	static constexpr int ITEM_MP_POTION            = 389017000;

	// used as an exchangeId that isn't in the data set
	static constexpr int EXCHANGE_MISSING          = 0;
	// used as our example of EXCHANGE_TYPE_ALL_ITEMS
	static constexpr int EXCHANGE_WARRIOR_BEGINNER = 1;
	// used as our example of EXCHANGE_TYPE_ONE_OF_EQUAL
	static constexpr int EXCHANGE_ISAAC_KISS       = 5;
	// used as our example of EXCHANGE_TYPE_ONE_OF_WEIGHTED
	static constexpr int EXCHANGE_ABYSS_GEM        = 35271;

	void SetUp() override
	{
		_app = std::make_unique<TestApp>();
		ASSERT_TRUE(_app != nullptr);

		// Load required tables
		for (const auto& itemModel : s_itemData)
			ASSERT_TRUE(_app->AddItemEntry(itemModel));

		for (const auto& itemExchangeModel : itemExchangeData)
		{
			ASSERT_TRUE(_app->AddItemExchangeEntry(itemExchangeModel));
		}

		// Setup user
		_user = _app->AddUser();
		ASSERT_TRUE(_user != nullptr);

		// Mark player as ingame
		_user->SetState(CONNECTION_STATE_GAMESTART);
	}

	void TearDown() override
	{
		_user.reset();
		_app.reset();
	}
};

// This test covers CUser::CheckExchange.  CheckExchange checks to make sure
// that an ITEM_EXCHANGE record exists, and that the user has enough inventory
// space to be granted any rewards.
TEST_F(ItemExchangeTest, CheckExchange)
{
	_user->ClearInventory();
	_user->ResetSend();

	// Should return false on a db miss
	EXPECT_FALSE(_user->CheckExchange(EXCHANGE_MISSING));

	// For EXCHANGE_TYPE_ONE_OF_WEIGHTED and EXCHANGE_TYPE_ONE_OF_EQUAL
	// There must be at least one open inventory slot to accept the result
	// of the exchange.  User inventory should currently be empty, so
	// the checks should pass
	EXPECT_TRUE(_user->CheckExchange(EXCHANGE_WARRIOR_BEGINNER));
	EXPECT_TRUE(_user->CheckExchange(EXCHANGE_ISAAC_KISS));
	EXPECT_TRUE(_user->CheckExchange(EXCHANGE_ABYSS_GEM));

	// We'll fill the user's inventory with non-stacking items.
	int emptySlots = _user->GetNumberOfEmptySlots();
	for (int i = 0; i < emptySlots; i++)
	{
		_user->TestGiveItem(ITEM_GOLD_BAR, 1);
	}
	ASSERT_EQ(_user->GetNumberOfEmptySlots(), 0);

	// Should return false with full inventory
	EXPECT_FALSE(_user->CheckExchange(EXCHANGE_WARRIOR_BEGINNER));
	EXPECT_FALSE(_user->CheckExchange(EXCHANGE_ISAAC_KISS));
	EXPECT_FALSE(_user->CheckExchange(EXCHANGE_ABYSS_GEM));

	// EXCHANGE_WARRIOR_BEGINNER grants the following items:
	// 120110992 Beginner's Weapon
	// 389011000 Water of life (x10)
	// 389017000 Potion of intelligence (x10)
	// If we free up 3 empty slots, then add the potions such that there
	// is only one slot, the check should pass.
	_user->TestRobItem(ITEM_GOLD_BAR, 1);
	_user->TestRobItem(ITEM_GOLD_BAR, 1);
	_user->TestRobItem(ITEM_GOLD_BAR, 1);
	_user->TestGiveItem(ITEM_HP_POTION, 5);
	_user->TestGiveItem(ITEM_MP_POTION, 5);
	ASSERT_EQ(_user->GetNumberOfEmptySlots(), 1);
	EXPECT_TRUE(_user->CheckExchange(EXCHANGE_WARRIOR_BEGINNER));
}

// This test covers CUser::RunExchange for a EXCHANGE_TYPE_ALL_ITEMS type.
// RunExchange checks for inputs, robs them, then gives the appropriate
// exchange reward.  This function does not return a status, so testing
// is done via inventory inspection.
TEST_F(ItemExchangeTest, RunExchange_EXCHANGE_TYPE_ALL_ITEMS)
{
	_user->ClearInventory();
	_user->ResetSend();

	// load exchange data
	auto* exchange = _app->m_ItemExchangeMap.GetData(EXCHANGE_WARRIOR_BEGINNER);
	ASSERT_TRUE(exchange != nullptr);

	ItemPair originItems[MAX_EXCHANGE_ITEMS] {};
	ItemPair exchangeItems[MAX_EXCHANGE_ITEMS] {};
	for (int i = 0; i < MAX_EXCHANGE_ITEMS; i++)
	{
		originItems[i].ItemId   = exchange->OriginItemNumber[i];
		originItems[i].Count    = exchange->OriginItemCount[i];
		exchangeItems[i].ItemId = exchange->ExchangeItemNumber[i];
		exchangeItems[i].Count  = exchange->ExchangeItemCount[i];
	}

	// Give the player required origin items for the beginner quest
	for (ItemPair item : originItems)
	{
		_user->TestGiveItem(item.ItemId, item.Count);
	}

	// RUN_EXCHANGE - EXCHANGE_TYPE_ALL_ITEMS
	// will check and rob origin items then give all the exchange items
	// stub RobItem callbacks
	for (ItemPair item : originItems)
	{
		if (item.ItemId <= 0 || item.Count <= 0)
			continue;

		_user->AddSendCallback(
			[=](const char* pBuf, int len)
			{
				ASSERT_EQ(len, sizeof(WeightChangePacket));
				auto packet = reinterpret_cast<const WeightChangePacket*>(pBuf);
				EXPECT_EQ(packet->Opcode, WIZ_WEIGHT_CHANGE);
			});
		_user->AddSendCallback(
			[=](const char* pBuf, int len)
			{
				ASSERT_EQ(len, sizeof(ItemCountChangePacket));
				auto packet = reinterpret_cast<const ItemCountChangePacket*>(pBuf);
				EXPECT_EQ(packet->Opcode, WIZ_ITEM_COUNT_CHANGE);
				EXPECT_EQ(packet->ItemId, item.ItemId);
			});
	}

	// stub GiveItem callbacks
	for (ItemPair item : exchangeItems)
	{
		if (item.ItemId <= 0 || item.Count <= 0)
			continue;

		_user->AddSendCallback(
			[=](const char* pBuf, int len)
			{
				ASSERT_EQ(len, sizeof(WeightChangePacket));
				auto packet = reinterpret_cast<const WeightChangePacket*>(pBuf);
				EXPECT_EQ(packet->Opcode, WIZ_WEIGHT_CHANGE);
			});
		_user->AddSendCallback(
			[=](const char* pBuf, int len)
			{
				ASSERT_EQ(len, sizeof(ItemCountChangePacket));
				auto packet = reinterpret_cast<const ItemCountChangePacket*>(pBuf);
				EXPECT_EQ(packet->Opcode, WIZ_ITEM_COUNT_CHANGE);
				EXPECT_EQ(packet->ItemId, item.ItemId);
				EXPECT_EQ(packet->ItemCount, item.Count);
			});
	}

	_user->RunExchange(EXCHANGE_WARRIOR_BEGINNER);

	EXPECT_TRUE(_user->CheckExistItemAnd(exchangeItems));
}

// This test covers CUser::RunExchange for a EXCHANGE_TYPE_ONE_OF_EQUAL type.
// RunExchange checks for inputs, robs them, then gives the appropriate
// exchange reward.  This function does not return a status, so testing
// is done via inventory inspection.
TEST_F(ItemExchangeTest, RunExchange_EXCHANGE_TYPE_ONE_OF_EQUAL)
{
	_user->ClearInventory();
	_user->ResetSend();

	// load exchange data
	auto* exchange = _app->m_ItemExchangeMap.GetData(EXCHANGE_ISAAC_KISS);
	ASSERT_TRUE(exchange != nullptr);

	ItemPair originItems[MAX_EXCHANGE_ITEMS] {};
	ItemPair exchangeItems[MAX_EXCHANGE_ITEMS] {};
	for (int i = 0; i < MAX_EXCHANGE_ITEMS; i++)
	{
		originItems[i].ItemId   = exchange->OriginItemNumber[i];
		originItems[i].Count    = exchange->OriginItemCount[i];
		exchangeItems[i].ItemId = exchange->ExchangeItemNumber[i];
		exchangeItems[i].Count  = exchange->ExchangeItemCount[i];
	}

	// Give the player required origin items
	for (ItemPair item : originItems)
	{
		_user->TestGiveItem(item.ItemId, item.Count);
	}

	// RUN_EXCHANGE - EXCHANGE_TYPE_ONE_OF_EQUAL
	// will check and rob origin items then give one of the exchange items
	// stub RobItem callbacks
	for (ItemPair item : originItems)
	{
		if (item.ItemId <= 0 || item.Count <= 0)
			continue;

		_user->AddSendCallback(
			[=](const char* pBuf, int len)
			{
				ASSERT_EQ(len, sizeof(WeightChangePacket));
				auto packet = reinterpret_cast<const WeightChangePacket*>(pBuf);
				EXPECT_EQ(packet->Opcode, WIZ_WEIGHT_CHANGE);
			});
		_user->AddSendCallback(
			[=](const char* pBuf, int len)
			{
				ASSERT_EQ(len, sizeof(ItemCountChangePacket));
				auto packet = reinterpret_cast<const ItemCountChangePacket*>(pBuf);
				EXPECT_EQ(packet->Opcode, WIZ_ITEM_COUNT_CHANGE);
				EXPECT_EQ(packet->ItemId, item.ItemId);
			});
	}

	// stub GiveItem callback
	_user->AddSendCallback(
		[=](const char* pBuf, int len)
		{
			ASSERT_EQ(len, sizeof(WeightChangePacket));
			auto packet = reinterpret_cast<const WeightChangePacket*>(pBuf);
			EXPECT_EQ(packet->Opcode, WIZ_WEIGHT_CHANGE);
		});
	_user->AddSendCallback(
		[=](const char* pBuf, int len)
		{
			ASSERT_EQ(len, sizeof(ItemCountChangePacket));
			auto packet = reinterpret_cast<const ItemCountChangePacket*>(pBuf);
			EXPECT_EQ(packet->Opcode, WIZ_ITEM_COUNT_CHANGE);
			// The ItemId/Count will be of the randomly selected item, so we
			// won't check it here.
		});

	_user->RunExchange(EXCHANGE_ISAAC_KISS);

	EXPECT_TRUE(_user->CheckExistItemOr(exchangeItems));
}

// This test covers CUser::RunExchange for a EXCHANGE_TYPE_ONE_OF_WEIGHTED type.
// RunExchange checks for inputs, robs them, then gives the appropriate
// exchange reward.  This function does not return a status, so testing
// is done via inventory inspection.
TEST_F(ItemExchangeTest, RunExchange_EXCHANGE_TYPE_ONE_OF_WEIGHTED)
{
	_user->ClearInventory();
	_user->ResetSend();

	// load exchange data
	auto* exchange = _app->m_ItemExchangeMap.GetData(EXCHANGE_ABYSS_GEM);
	ASSERT_TRUE(exchange != nullptr);

	ItemPair originItems[MAX_EXCHANGE_ITEMS] {};
	ItemPair exchangeItems[MAX_EXCHANGE_ITEMS] {};
	for (int i = 0; i < MAX_EXCHANGE_ITEMS; i++)
	{
		originItems[i].ItemId   = exchange->OriginItemNumber[i];
		originItems[i].Count    = exchange->OriginItemCount[i];
		exchangeItems[i].ItemId = exchange->ExchangeItemNumber[i];
		exchangeItems[i].Count  = exchange->ExchangeItemCount[i];
	}

	// Give the player required origin items
	for (ItemPair item : originItems)
	{
		_user->TestGiveItem(item.ItemId, item.Count);
	}

	// RUN_EXCHANGE - EXCHANGE_TYPE_ONE_OF_WEIGHTED
	// will check and rob origin items then give one of the exchange items
	// stub RobItem callbacks
	for (ItemPair item : originItems)
	{
		if (item.ItemId <= 0 || item.Count <= 0)
			continue;

		_user->AddSendCallback(
			[=](const char* pBuf, int len)
			{
				ASSERT_EQ(len, sizeof(WeightChangePacket));
				auto packet = reinterpret_cast<const WeightChangePacket*>(pBuf);
				EXPECT_EQ(packet->Opcode, WIZ_WEIGHT_CHANGE);
			});
		_user->AddSendCallback(
			[=](const char* pBuf, int len)
			{
				ASSERT_EQ(len, sizeof(ItemCountChangePacket));
				auto packet = reinterpret_cast<const ItemCountChangePacket*>(pBuf);
				EXPECT_EQ(packet->Opcode, WIZ_ITEM_COUNT_CHANGE);
				EXPECT_EQ(packet->ItemId, item.ItemId);
			});
	}

	// stub GiveItem callback
	_user->AddSendCallback(
		[=](const char* pBuf, int len)
		{
			ASSERT_EQ(len, sizeof(WeightChangePacket));
			auto packet = reinterpret_cast<const WeightChangePacket*>(pBuf);
			EXPECT_EQ(packet->Opcode, WIZ_WEIGHT_CHANGE);
		});
	_user->AddSendCallback(
		[=](const char* pBuf, int len)
		{
			ASSERT_EQ(len, sizeof(ItemCountChangePacket));
			auto packet = reinterpret_cast<const ItemCountChangePacket*>(pBuf);
			EXPECT_EQ(packet->Opcode, WIZ_ITEM_COUNT_CHANGE);
			// The ItemId/Count will be of the randomly selected item, so we
			// won't check it here.
		});

	_user->RunExchange(EXCHANGE_ABYSS_GEM);

	// check for the reward. We have to stub a count of 1
	// in rather than using the value in the array (it's a percentage,
	// not a stack size).
	bool rewardFound = false;
	for (ItemPair item : exchangeItems)
	{
		if (_user->CheckExistItem(item.ItemId, 1))
		{
			rewardFound = true;
			break;
		}
	}
	EXPECT_TRUE(rewardFound);
}
#include <gtest/gtest.h>

#include <Ebenezer/BotRegistry.h>
#include <Ebenezer/User.h>

#include <memory>
#include <vector>

namespace
{

class TestBotUser final : public Ebenezer::CUser
{
public:
	TestBotUser() : CUser(test_tag {})
	{
	}
};

}

TEST(BotRegistryTest, AllocatesTheEntireBotBandAndRejectsOverflow)
{
	Ebenezer::BotRegistry registry;
	std::vector<std::shared_ptr<TestBotUser>> bots;
	bots.reserve(Ebenezer::MAX_BOT_USER + 1);

	for (int index = 0; index < Ebenezer::MAX_BOT_USER; ++index)
	{
		auto bot = std::make_shared<TestBotUser>();
		const int userId = registry.Register(bot);

		if (index == 0)
		{
			EXPECT_EQ(userId, Ebenezer::BOT_USER_ID_MIN);
			EXPECT_EQ(bot->GetSocketID(), Ebenezer::BOT_USER_ID_MIN);
		}
		if (index == Ebenezer::MAX_BOT_USER - 1)
			EXPECT_EQ(userId, Ebenezer::BOT_USER_ID_MAX);

		bots.push_back(std::move(bot));
	}

	auto overflowBot = std::make_shared<TestBotUser>();
	EXPECT_EQ(registry.Register(overflowBot), -1);
}

TEST(BotRegistryTest, GetsAndRemovesTheRegisteredBot)
{
	Ebenezer::BotRegistry registry;
	auto bot = std::make_shared<TestBotUser>();
	const int userId = registry.Register(bot);

	EXPECT_EQ(registry.Get(userId), bot);
	EXPECT_EQ(registry.Remove(userId), bot);
	EXPECT_EQ(registry.Get(userId), nullptr);
}

TEST(BotRegistryTest, RejectsNullBots)
{
	Ebenezer::BotRegistry registry;

	EXPECT_EQ(registry.Register(nullptr), -1);
	EXPECT_EQ(registry.Size(), 0u);
}

TEST(BotRegistryTest, ReusesTheLowestRemovedId)
{
	Ebenezer::BotRegistry registry;
	auto firstBot = std::make_shared<TestBotUser>();
	auto secondBot = std::make_shared<TestBotUser>();
	const int firstId = registry.Register(firstBot);
	registry.Register(secondBot);

	registry.Remove(firstId);
	auto replacement = std::make_shared<TestBotUser>();

	EXPECT_EQ(registry.Register(replacement), Ebenezer::BOT_USER_ID_MIN);
}

TEST(BotRegistryTest, SnapshotKeepsBotsAliveAfterRegistryMutation)
{
	Ebenezer::BotRegistry registry;
	auto bot = std::make_shared<TestBotUser>();
	registry.Register(bot);
	auto snapshot = registry.Snapshot();

	registry.Clear();
	bot.reset();

	ASSERT_EQ(snapshot.size(), 1u);
	EXPECT_NE(snapshot.front(), nullptr);
}

TEST(BotRegistryTest, ClearMakesTheRegistryEmpty)
{
	Ebenezer::BotRegistry registry;
	registry.Register(std::make_shared<TestBotUser>());

	registry.Clear();

	EXPECT_EQ(registry.Size(), 0u);
}

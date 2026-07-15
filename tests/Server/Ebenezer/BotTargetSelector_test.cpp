#include <gtest/gtest.h>

#include <Ebenezer/BotTargetSelector.h>
#include <Ebenezer/BotUser.h>
#include <Ebenezer/EbenezerApp.h>

#include <shared/StringUtils.h>

#include "TestApp.h"

#include <memory>
#include <vector>

using namespace Ebenezer;

namespace
{

class BotTargetSelectorTest : public testing::Test
{
protected:
	void SetUp() override
	{
		_app = std::make_unique<TestApp>();
		_map = _app->CreateMap(ZONE_FRONTIER);
		ASSERT_NE(_map, nullptr);

		BotSpawnRequest request;
		request.name           = "Bot_K_000";
		request.nation         = NATION_KARUS;
		request.characterClass = CLASS_KA_WARRIOR;
		request.level          = 60;
		request.spawn          = { ZONE_FRONTIER, 120.0f, 0.0f, 120.0f };

		_source                = std::make_shared<CBotUser>();
		ASSERT_TRUE(_source->InitializeBot(request));
		ASSERT_GE(_app->GetBotRegistry().Register(_source), 0);
		ASSERT_TRUE(_map->Add(_source.get(), _source->m_RegionX, _source->m_RegionZ));
	}

	void TearDown() override
	{
		// Prevent CUser::CloseProcess from sending region packets to callback-free test users.
		if (_source != nullptr)
			_source->SetState(CONNECTION_STATE_DISCONNECTED);
		for (const auto& user : _users)
			user->SetState(CONNECTION_STATE_DISCONNECTED);
	}

	std::shared_ptr<TestUser> AddUser(uint8_t nation, float x, float z, int regionX, int regionZ)
	{
		auto user = _app->AddUser();
		if (user == nullptr)
			return nullptr;

		strcpy_safe(user->m_pUserData->m_id, nation == NATION_KARUS ? "Target_K" : "Target_E");
		user->m_pUserData->m_bNation = nation;
		user->m_pUserData->m_bZone   = ZONE_FRONTIER;
		user->m_pUserData->m_curx    = x;
		user->m_pUserData->m_cury    = 0.0f;
		user->m_pUserData->m_curz    = z;
		user->m_pUserData->m_sHp     = 100;
		user->m_bResHpType           = USER_STANDING;
		user->SetState(CONNECTION_STATE_GAMESTART);
		if (!_map->Add(user.get(), static_cast<uint16_t>(regionX), static_cast<uint16_t>(regionZ)))
		{
			user->SetState(CONNECTION_STATE_DISCONNECTED);
			return nullptr;
		}
		_users.push_back(user);
		return user;
	}

	BotTargetSelector Selector()
	{
		return BotTargetSelector(*_app);
	}

	std::unique_ptr<TestApp> _app;
	TestMap* _map = nullptr;
	std::shared_ptr<CBotUser> _source;
	std::vector<std::shared_ptr<TestUser>> _users;
};

TEST_F(BotTargetSelectorTest, SelectsNearestEnemyAcrossSameAndNeighboringRegions)
{
	auto farther = AddUser(NATION_ELMORAD, 145.0f, 120.0f, 3, 2);
	auto nearest = AddUser(NATION_ELMORAD, 125.0f, 120.0f, 2, 2);
	ASSERT_NE(farther, nullptr);
	ASSERT_NE(nearest, nullptr);

	EXPECT_EQ(Selector().SelectNearestEnemy(*_source), nearest->GetSocketID());
}

TEST_F(BotTargetSelectorTest, IgnoresSelf)
{
	EXPECT_EQ(Selector().SelectNearestEnemy(*_source), -1);
}

TEST_F(BotTargetSelectorTest, IgnoresSameNation)
{
	ASSERT_NE(AddUser(NATION_KARUS, 121.0f, 120.0f, 2, 2), nullptr);

	EXPECT_EQ(Selector().SelectNearestEnemy(*_source), -1);
}

TEST_F(BotTargetSelectorTest, IgnoresZeroHpEnemy)
{
	auto enemy = AddUser(NATION_ELMORAD, 121.0f, 120.0f, 2, 2);
	ASSERT_NE(enemy, nullptr);
	enemy->m_pUserData->m_sHp = 0;

	EXPECT_EQ(Selector().SelectNearestEnemy(*_source), -1);
}

TEST_F(BotTargetSelectorTest, IgnoresDeadEnemy)
{
	auto enemy = AddUser(NATION_ELMORAD, 121.0f, 120.0f, 2, 2);
	ASSERT_NE(enemy, nullptr);
	enemy->m_bResHpType = USER_DEAD;

	EXPECT_EQ(Selector().SelectNearestEnemy(*_source), -1);
}

TEST_F(BotTargetSelectorTest, IgnoresEnemyOutsideGameStartState)
{
	auto enemy = AddUser(NATION_ELMORAD, 121.0f, 120.0f, 2, 2);
	ASSERT_NE(enemy, nullptr);
	enemy->SetState(CONNECTION_STATE_CONNECTED);

	EXPECT_EQ(Selector().SelectNearestEnemy(*_source), -1);
}

TEST_F(BotTargetSelectorTest, IgnoresStaleRegionEntryForEnemyInAnotherZone)
{
	auto enemy = AddUser(NATION_ELMORAD, 121.0f, 120.0f, 2, 2);
	ASSERT_NE(enemy, nullptr);
	enemy->m_pUserData->m_bZone = ZONE_BATTLE;

	EXPECT_EQ(Selector().SelectNearestEnemy(*_source), -1);
}

TEST_F(BotTargetSelectorTest, IgnoresEnemyOutsideThreeByThreeRegionNeighborhood)
{
	auto enemy = AddUser(NATION_ELMORAD, 220.0f, 120.0f, 4, 2);
	ASSERT_NE(enemy, nullptr);

	EXPECT_EQ(Selector().SelectNearestEnemy(*_source), -1);
}

TEST_F(BotTargetSelectorTest, EqualDistanceSelectsLowerIdEvenWhenScannedLater)
{
	auto lowerIdLaterRegion  = AddUser(NATION_ELMORAD, 145.0f, 120.0f, 3, 2);
	auto higherIdFirstRegion = AddUser(NATION_ELMORAD, 95.0f, 120.0f, 1, 2);
	ASSERT_NE(lowerIdLaterRegion, nullptr);
	ASSERT_NE(higherIdFirstRegion, nullptr);
	ASSERT_LT(lowerIdLaterRegion->GetSocketID(), higherIdFirstRegion->GetSocketID());

	EXPECT_EQ(Selector().SelectNearestEnemy(*_source), lowerIdLaterRegion->GetSocketID());
}

TEST_F(BotTargetSelectorTest, DeduplicatesCandidateIdsCopiedFromMultipleRegions)
{
	auto enemy = AddUser(NATION_ELMORAD, 125.0f, 120.0f, 2, 2);
	ASSERT_NE(enemy, nullptr);
	ASSERT_TRUE(_map->RegionUserAdd(3, 2, enemy->GetSocketID()));

	EXPECT_EQ(Selector().SelectNearestEnemy(*_source), enemy->GetSocketID());
}

} // namespace

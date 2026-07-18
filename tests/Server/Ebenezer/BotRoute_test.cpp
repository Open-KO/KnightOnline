#include <gtest/gtest.h>

#include <Ebenezer/BotRoute.h>
#include "TestApp.h"
#include "TestMap.h"

#include <array>
#include <memory>

using namespace Ebenezer;

TEST(BotRouteTest, NationsUseExactOutwardRoutesAndConvergeAtBowl)
{
	constexpr std::array<BotRoutePoint, 4> expectedKarus {
		BotRoutePoint { 890.0f, 350.0f }, BotRoutePoint { 930.0f, 600.0f },
		BotRoutePoint { 970.0f, 820.0f }, BotRoutePoint { 1000.0f, 1000.0f }
	};
	constexpr std::array<BotRoutePoint, 4> expectedElmorad {
		BotRoutePoint { 400.0f, 930.0f }, BotRoutePoint { 620.0f, 960.0f },
		BotRoutePoint { 820.0f, 980.0f }, BotRoutePoint { 1000.0f, 1000.0f }
	};
	const auto karus = BotRoute::Resolve(NATION_KARUS, ZONE_FRONTIER);
	const auto elmorad = BotRoute::Resolve(NATION_ELMORAD, ZONE_FRONTIER);
	ASSERT_TRUE(karus.has_value());
	ASSERT_TRUE(elmorad.has_value());
	ASSERT_EQ(karus->travel.size(), expectedKarus.size());
	ASSERT_EQ(elmorad->travel.size(), expectedElmorad.size());
	for (size_t index = 0; index < expectedKarus.size(); ++index)
	{
		EXPECT_FLOAT_EQ(karus->travel[index].x, expectedKarus[index].x);
		EXPECT_FLOAT_EQ(karus->travel[index].z, expectedKarus[index].z);
		EXPECT_FLOAT_EQ(elmorad->travel[index].x, expectedElmorad[index].x);
		EXPECT_FLOAT_EQ(elmorad->travel[index].z, expectedElmorad[index].z);
	}
}

TEST(BotRouteTest, FinalTravelPointSwitchesToLoopAndLoopWraps)
{
	BotRuntime runtime;
	runtime.routeIndex = 3;
	BotRoute::Advance(runtime, NATION_KARUS, ZONE_FRONTIER);
	EXPECT_TRUE(runtime.reachedBowl);
	EXPECT_EQ(runtime.routeIndex, 4u);
	EXPECT_EQ(runtime.bowlPatrolIndex, 0u);

	const float expected[][2] = {
		{ 990.0f, 990.0f }, { 1010.0f, 990.0f },
		{ 1010.0f, 1010.0f }, { 990.0f, 1010.0f }, { 990.0f, 990.0f }
	};
	for (size_t index = 0; index < 5; ++index)
	{
		auto destination = BotRoute::CurrentDestination(
			NATION_KARUS, ZONE_FRONTIER, runtime);
		ASSERT_TRUE(destination.has_value());
		EXPECT_FLOAT_EQ(destination->x, expected[index][0]);
		EXPECT_FLOAT_EQ(destination->z, expected[index][1]);
		BotRoute::Advance(runtime, NATION_KARUS, ZONE_FRONTIER);
	}
}

TEST(BotRouteTest, ResetReturnsEveryNavigationCursorToFirstOutwardWaypoint)
{
	BotRuntime runtime;
	runtime.routeIndex = 3;
	runtime.bowlPatrolIndex = 2;
	runtime.reachedBowl = true;
	runtime.patrolIndex = 3;

	BotRoute::Reset(runtime);

	EXPECT_EQ(runtime.routeIndex, 0u);
	EXPECT_EQ(runtime.bowlPatrolIndex, 0u);
	EXPECT_FALSE(runtime.reachedBowl);
	EXPECT_EQ(runtime.patrolIndex, 0u);
}

TEST(BotRouteTest, ValidationRequiresZone201AndEveryPointInsideTheMap)
{
	TestApp app;
	auto validMap = std::make_unique<TestMap>(ZONE_FRONTIER, 513, 4.0f);
	auto smallMap = std::make_unique<TestMap>(ZONE_FRONTIER, 256, 1.0f);
	EXPECT_TRUE(BotRoute::Validate(NATION_KARUS, ZONE_FRONTIER, *validMap));
	EXPECT_TRUE(BotRoute::Validate(NATION_ELMORAD, ZONE_FRONTIER, *validMap));
	EXPECT_FALSE(BotRoute::Validate(NATION_KARUS, ZONE_BATTLE, *validMap));
	EXPECT_FALSE(BotRoute::Validate(NATION_KARUS, ZONE_FRONTIER, *smallMap));
	EXPECT_FALSE(BotRoute::Validate(0, ZONE_FRONTIER, *validMap));
}

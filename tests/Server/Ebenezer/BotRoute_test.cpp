#include <gtest/gtest.h>

#include <Ebenezer/BotRoute.h>
#include "TestApp.h"
#include "TestMap.h"

#include <memory>

using namespace Ebenezer;

TEST(BotRouteTest, NationsUseExactOutwardRoutesAndConvergeAtBowl)
{
	BotRuntime runtime;
	auto karus = BotRoute::CurrentDestination(NATION_KARUS, ZONE_FRONTIER, runtime);
	auto elmorad = BotRoute::CurrentDestination(NATION_ELMORAD, ZONE_FRONTIER, runtime);
	ASSERT_TRUE(karus.has_value());
	ASSERT_TRUE(elmorad.has_value());
	EXPECT_FLOAT_EQ(karus->x, 890.0f);
	EXPECT_FLOAT_EQ(karus->z, 350.0f);
	EXPECT_FLOAT_EQ(elmorad->x, 400.0f);
	EXPECT_FLOAT_EQ(elmorad->z, 930.0f);

	runtime.routeIndex = 3;
	karus = BotRoute::CurrentDestination(NATION_KARUS, ZONE_FRONTIER, runtime);
	elmorad = BotRoute::CurrentDestination(NATION_ELMORAD, ZONE_FRONTIER, runtime);
	ASSERT_TRUE(karus.has_value());
	ASSERT_TRUE(elmorad.has_value());
	EXPECT_FLOAT_EQ(karus->x, 1000.0f);
	EXPECT_FLOAT_EQ(karus->z, 1000.0f);
	EXPECT_FLOAT_EQ(elmorad->x, 1000.0f);
	EXPECT_FLOAT_EQ(elmorad->z, 1000.0f);
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

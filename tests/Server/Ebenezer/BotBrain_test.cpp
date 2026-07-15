#include <gtest/gtest.h>

#include <Ebenezer/BotBrain.h>

#include <chrono>

using namespace Ebenezer;

namespace
{

class BotBrainTest : public testing::Test
{
protected:
	static constexpr float AttackRange = 2.5f;
	const std::chrono::steady_clock::time_point now { std::chrono::seconds(100) };
	BotBrain brain;
};

TEST_F(BotBrainTest, SelectsTargetWhenLiveBotHasNotSearchedYet)
{
	BotRuntime runtime;
	const BotPerception noTarget { true, false, 0.0f };

	const BotIntent intent = brain.Decide(runtime, noTarget, now, AttackRange);

	EXPECT_EQ(intent.type, BotIntentType::SelectTarget);
	EXPECT_EQ(intent.targetId, -1);
}

TEST_F(BotBrainTest, PatrolsAfterSelectorReportsNoEnemy)
{
	BotRuntime runtime;
	runtime.state = BotState::SelectTarget;
	const BotPerception noEnemy { true, false, 0.0f };

	const BotIntent intent = brain.Decide(runtime, noEnemy, now, AttackRange);

	EXPECT_EQ(intent.type, BotIntentType::Patrol);
	EXPECT_EQ(intent.targetId, -1);
}

TEST_F(BotBrainTest, ApproachesValidTargetOutsideAttackRange)
{
	BotRuntime runtime;
	runtime.targetId = 42;
	const BotPerception farTarget { true, true, AttackRange + 0.01f };

	const BotIntent intent = brain.Decide(runtime, farTarget, now, AttackRange);

	EXPECT_EQ(intent.type, BotIntentType::Approach);
	EXPECT_EQ(intent.targetId, 42);
}

TEST_F(BotBrainTest, AttacksValidTargetAtAttackRangeBoundary)
{
	BotRuntime runtime;
	runtime.targetId = 73;
	const BotPerception nearTarget { true, true, AttackRange };

	const BotIntent intent = brain.Decide(runtime, nearTarget, now, AttackRange);

	EXPECT_EQ(intent.type, BotIntentType::BasicAttack);
	EXPECT_EQ(intent.targetId, 73);
}

TEST_F(BotBrainTest, DeadBotWaitsBeforeRespawnTime)
{
	BotRuntime runtime;
	runtime.state     = BotState::Dead;
	runtime.respawnAt = now + std::chrono::seconds(15);
	const BotPerception dead { false, false, 0.0f };

	const BotIntent intent = brain.Decide(
		runtime, dead, runtime.respawnAt - std::chrono::milliseconds(1), AttackRange);

	EXPECT_EQ(intent.type, BotIntentType::Wait);
	EXPECT_EQ(intent.targetId, -1);
}

TEST_F(BotBrainTest, DeadBotRespawnsExactlyAtRespawnTime)
{
	BotRuntime runtime;
	runtime.state     = BotState::Dead;
	runtime.respawnAt = now + std::chrono::seconds(15);
	const BotPerception dead { false, false, 0.0f };

	const BotIntent intent = brain.Decide(runtime, dead, runtime.respawnAt, AttackRange);

	EXPECT_EQ(intent.type, BotIntentType::Respawn);
	EXPECT_EQ(intent.targetId, -1);
}

TEST_F(BotBrainTest, PerceivedDeathOverridesLiveRuntimeState)
{
	BotRuntime runtime;
	runtime.state     = BotState::Approach;
	runtime.targetId  = 91;
	runtime.respawnAt = now + std::chrono::seconds(15);
	const BotPerception dead { false, true, 1.0f };

	const BotIntent intent = brain.Decide(runtime, dead, now, AttackRange);

	EXPECT_EQ(intent.type, BotIntentType::Wait);
	EXPECT_EQ(intent.targetId, -1);
}

} // namespace

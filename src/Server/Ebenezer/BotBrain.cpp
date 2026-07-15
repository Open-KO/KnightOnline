#include "pch.h"
#include "BotBrain.h"

namespace Ebenezer
{

BotIntent BotBrain::Decide(const BotRuntime& runtime, const BotPerception& perception,
	std::chrono::steady_clock::time_point now, float attackRange) const
{
	if (runtime.state == BotState::Dead || !perception.alive)
	{
		return { now < runtime.respawnAt ? BotIntentType::Wait : BotIntentType::Respawn, -1 };
	}

	if (!perception.targetValid)
	{
		return { runtime.state == BotState::SelectTarget ? BotIntentType::Patrol
														 : BotIntentType::SelectTarget,
			-1 };
	}

	return { perception.targetDistance > attackRange ? BotIntentType::Approach
													 : BotIntentType::BasicAttack,
		runtime.targetId };
}

} // namespace Ebenezer

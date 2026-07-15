#ifndef SERVER_EBENEZER_BOTBRAIN_H
#define SERVER_EBENEZER_BOTBRAIN_H

#pragma once

#include "BotTypes.h"

#include <chrono>
#include <cstdint>

namespace Ebenezer
{

enum class BotIntentType : uint8_t
{
	Wait,
	Patrol,
	SelectTarget,
	Approach,
	BasicAttack,
	Respawn
};

struct BotPerception
{
	bool alive           = true;
	bool targetValid     = false;
	float targetDistance = 0.0f;
};

struct BotIntent
{
	BotIntentType type = BotIntentType::Wait;
	int targetId       = -1;
};

class BotBrain
{
public:
	BotIntent Decide(const BotRuntime& runtime, const BotPerception& perception,
		std::chrono::steady_clock::time_point now, float attackRange) const;
};

} // namespace Ebenezer

#endif // SERVER_EBENEZER_BOTBRAIN_H

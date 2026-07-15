#ifndef SERVER_EBENEZER_BOTTYPES_H
#define SERVER_EBENEZER_BOTTYPES_H

#pragma once

#include "Define.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>

namespace Ebenezer
{

enum class BotState : uint8_t
{
	Spawn,
	Patrol,
	SelectTarget,
	Approach,
	BasicAttack,
	Dead
};

struct BotSpawnPoint
{
	uint8_t zoneId = ZONE_FRONTIER;
	float x        = 0.0f;
	float y        = 0.0f;
	float z        = 0.0f;
};

struct BotSpawnRequest
{
	std::string name;
	uint8_t nation         = NATION_KARUS;
	e_Class characterClass = CLASS_KA_WARRIOR;
	uint8_t level          = 60;
	BotSpawnPoint spawn;
};

struct BotRuntime
{
	BotState state = BotState::Spawn;
	int targetId   = -1;
	BotSpawnPoint home;
	std::chrono::steady_clock::time_point nextAttackAt {};
	std::chrono::steady_clock::time_point respawnAt {};
	size_t patrolIndex = 0;
};

} // namespace Ebenezer

#endif // SERVER_EBENEZER_BOTTYPES_H

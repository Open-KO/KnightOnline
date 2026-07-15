#ifndef SERVER_EBENEZER_BOTTYPES_H
#define SERVER_EBENEZER_BOTTYPES_H

#pragma once

#include "Define.h"

#include <chrono>
#include <algorithm>
#include <cstddef>
#include <cctype>
#include <cstdint>
#include <string>
#include <string_view>

namespace Ebenezer
{

struct BotConfig
{
	bool enabled              = false;
	uint16_t count            = 10;
	uint16_t tickMilliseconds = 200;
	uint16_t respawnSeconds   = 15;
	uint8_t zoneId            = ZONE_FRONTIER;
	float attackRange         = 2.5f;
	float moveStep            = 1.5f;
};

inline std::string NormalizeBotToken(std::string_view token)
{
	std::string normalized(token);
	std::transform(normalized.begin(), normalized.end(), normalized.begin(),
		[](unsigned char value) { return static_cast<char>(std::tolower(value)); });
	return normalized;
}

inline bool IsReservedBotName(std::string_view name)
{
	if (name.size() < 6)
		return false;
	const std::string prefix = NormalizeBotToken(name.substr(0, 6));
	return prefix == "bot_k_" || prefix == "bot_e_";
}

inline uint8_t ResolveBotNation(std::string_view token)
{
	const std::string normalized = NormalizeBotToken(token);
	if (normalized == "karus")
		return NATION_KARUS;
	if (normalized == "elmorad")
		return NATION_ELMORAD;
	return 0;
}

inline e_Class ResolveBotClass(uint8_t nation, std::string_view token)
{
	const std::string normalized = NormalizeBotToken(token);
	if (nation == NATION_KARUS)
	{
		if (normalized == "warrior") return CLASS_KA_WARRIOR;
		if (normalized == "rogue") return CLASS_KA_ROGUE;
		if (normalized == "mage") return CLASS_KA_WIZARD;
		if (normalized == "priest") return CLASS_KA_PRIEST;
	}
	else if (nation == NATION_ELMORAD)
	{
		if (normalized == "warrior") return CLASS_EL_WARRIOR;
		if (normalized == "rogue") return CLASS_EL_ROGUE;
		if (normalized == "mage") return CLASS_EL_WIZARD;
		if (normalized == "priest") return CLASS_EL_PRIEST;
	}
	return CLASS_UNKNOWN;
}

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

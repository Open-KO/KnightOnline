#ifndef SERVER_EBENEZER_BOTROUTE_H
#define SERVER_EBENEZER_BOTROUTE_H

#pragma once

#include "BotTypes.h"

#include <cstdint>
#include <optional>
#include <span>

namespace Ebenezer
{
class C3DMap;

struct BotRoutePoint
{
	float x = 0.0f;
	float z = 0.0f;
};

struct BotRouteView
{
	std::span<const BotRoutePoint> travel;
	std::span<const BotRoutePoint> bowlPatrol;
};

class BotRoute
{
public:
	static std::optional<BotRouteView> Resolve(uint8_t nation, uint8_t zoneId) noexcept;
	static std::optional<BotSpawnPoint> CurrentDestination(
		uint8_t nation, uint8_t zoneId, const BotRuntime& runtime) noexcept;
	static void Advance(BotRuntime& runtime, uint8_t nation, uint8_t zoneId) noexcept;
	static void Reset(BotRuntime& runtime) noexcept;
	static bool Validate(uint8_t nation, uint8_t zoneId, const C3DMap& map) noexcept;
};
} // namespace Ebenezer

#endif // SERVER_EBENEZER_BOTROUTE_H

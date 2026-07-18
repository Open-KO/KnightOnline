#include "pch.h"
#include "BotRoute.h"

#include "Map.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace Ebenezer
{
namespace
{
constexpr std::array<BotRoutePoint, 4> KARUS_TRAVEL {
	BotRoutePoint { 890.0f, 350.0f }, BotRoutePoint { 930.0f, 600.0f },
	BotRoutePoint { 970.0f, 820.0f }, BotRoutePoint { 1000.0f, 1000.0f }
};
constexpr std::array<BotRoutePoint, 4> ELMORAD_TRAVEL {
	BotRoutePoint { 400.0f, 930.0f }, BotRoutePoint { 620.0f, 960.0f },
	BotRoutePoint { 820.0f, 980.0f }, BotRoutePoint { 1000.0f, 1000.0f }
};
constexpr std::array<BotRoutePoint, 4> BOWL_PATROL {
	BotRoutePoint { 990.0f, 990.0f }, BotRoutePoint { 1010.0f, 990.0f },
	BotRoutePoint { 1010.0f, 1010.0f }, BotRoutePoint { 990.0f, 1010.0f }
};
}

std::optional<BotRouteView> BotRoute::Resolve(uint8_t nation, uint8_t zoneId) noexcept
{
	if (zoneId != ZONE_FRONTIER)
		return std::nullopt;
	if (nation == NATION_KARUS)
		return BotRouteView { KARUS_TRAVEL, BOWL_PATROL };
	if (nation == NATION_ELMORAD)
		return BotRouteView { ELMORAD_TRAVEL, BOWL_PATROL };
	return std::nullopt;
}

std::optional<BotSpawnPoint> BotRoute::CurrentDestination(
	uint8_t nation, uint8_t zoneId, const BotRuntime& runtime) noexcept
{
	const auto route = Resolve(nation, zoneId);
	if (!route.has_value())
		return std::nullopt;
	const BotRoutePoint* point = nullptr;
	if (!runtime.reachedBowl)
	{
		if (runtime.routeIndex >= route->travel.size())
			return std::nullopt;
		point = &route->travel[runtime.routeIndex];
	}
	else
	{
		point = &route->bowlPatrol[runtime.bowlPatrolIndex % route->bowlPatrol.size()];
	}
	return BotSpawnPoint { zoneId, point->x, 0.0f, point->z };
}

void BotRoute::Advance(BotRuntime& runtime, uint8_t nation, uint8_t zoneId) noexcept
{
	const auto route = Resolve(nation, zoneId);
	if (!route.has_value())
		return;
	if (!runtime.reachedBowl)
	{
		++runtime.routeIndex;
		if (runtime.routeIndex >= route->travel.size())
		{
			runtime.routeIndex = route->travel.size();
			runtime.reachedBowl = true;
			runtime.bowlPatrolIndex = 0;
		}
		return;
	}
	runtime.bowlPatrolIndex = (runtime.bowlPatrolIndex + 1) % route->bowlPatrol.size();
}

void BotRoute::Reset(BotRuntime& runtime) noexcept
{
	runtime.patrolIndex = 0;
	runtime.routeIndex = 0;
	runtime.bowlPatrolIndex = 0;
	runtime.reachedBowl = false;
}

bool BotRoute::Validate(uint8_t nation, uint8_t zoneId, const C3DMap& map) noexcept
{
	const auto route = Resolve(nation, zoneId);
	if (!route.has_value() || map.m_nZoneNumber != zoneId)
		return false;
	const auto valid = [&map](const BotRoutePoint& point)
	{
		return std::isfinite(point.x) && std::isfinite(point.z)
			&& point.x >= 0.0f && point.z >= 0.0f
			&& map.IsValidPosition(point.x, point.z);
	};
	return std::all_of(route->travel.begin(), route->travel.end(), valid)
		&& std::all_of(route->bowlPatrol.begin(), route->bowlPatrol.end(), valid);
}
} // namespace Ebenezer

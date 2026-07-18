# CZ Town-to-Bowl Bot Routes Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Spawn Karus and El Morad bots in their own zone 201 towns, march them through nation-specific routes into CZ Bowl, preserve combat priority, and restart the march from the correct town after death.

**Architecture:** Add a pure `BotRoute` catalog/state helper that owns the fixed outward waypoints and shared Bowl loop. Keep the existing brain intents: `BotCommandFacade::Patrol()` resolves the route destination and moves through the existing validated 1.5-unit movement pipeline, while `BotManager` continues to give target selection, approach, and attack priority. Validate both complete routes against the loaded zone 201 map before the ten-bot startup transaction creates any bot.

**Tech Stack:** C++20, OpenKO `Ebenezer.Core`, `C3DMap`, GoogleTest, CMake/MSBuild 17, PowerShell localhost operations.

## Global Constraints

- Automatic roster remains exactly ten transient socketless bots: five Karus warriors and five El Morad warriors.
- Tick interval remains exactly 200 ms; respawn delay remains exactly 15 seconds.
- Zone remains `ZONE_FRONTIER` (`201`); no teleporting and no navmesh is introduced.
- Every movement uses `BotMovement::NextStep()` and `BotMovement::Move()` and never exceeds 1.5 world units.
- Enemy targeting, approach, and basic attack remain higher priority than route travel.
- Karus HOME comes from `HOME.FreeZoneX/FreeZoneZ` and leads to `(890,350) -> (930,600) -> (970,820) -> (1000,1000)`.
- El Morad HOME comes from `HOME.FreeZoneX/FreeZoneZ` and leads to `(400,930) -> (620,960) -> (820,980) -> (1000,1000)`.
- Bowl patrol is exactly `(990,990) -> (1010,990) -> (1010,1010) -> (990,1010)`.
- Invalid route data disables only bot startup; the ordinary game server must continue.
- Loopback bindings, ignored credentials, local SQL setup, client packet behavior, and launcher scripts remain unchanged.

## File Structure

- Create `src/Server/Ebenezer/BotRoute.h`: route points, route views, cursor resolution, advancement, reset, and validation interface.
- Create `src/Server/Ebenezer/BotRoute.cpp`: fixed zone 201 route tables and pure route state transitions.
- Modify `src/Server/Ebenezer/BotTypes.h`: add explicit outward-route and Bowl-loop cursor fields without removing the legacy non-CZ `patrolIndex`.
- Modify `src/Server/Ebenezer/BotCommandFacade.cpp`: route-aware `Patrol()` and navigation reset during `Respawn()`.
- Modify `src/Server/Ebenezer/EbenezerApp.h`: declare configured-route startup validation.
- Modify `src/Server/Ebenezer/EbenezerApp.cpp`: validate both routes atomically before building the automatic roster.
- Modify `src/Server/Ebenezer/CMakeLists.txt`: compile the new route source into `Ebenezer.Core`.
- Modify `src/Server/Ebenezer/Ebenezer.Core.vcxproj` and `.filters`: compile and display the new route source in the checked-in MSBuild project.
- Create `tests/Server/Ebenezer/BotRoute_test.cpp`: exact route data, cursor transitions, Bowl looping, reset, and map validation tests.
- Modify `tests/Server/Ebenezer/BotManager_test.cpp`: route movement, waypoint threshold, combat interruption/resume, and respawn reset integration tests.
- Modify `tests/Server/Ebenezer/BotOperationMessage_test.cpp`: exact nation HOME startup and invalid-route atomic rollback tests.
- Modify `tests/Server/Ebenezer/CMakeLists.txt`: compile `BotRoute_test.cpp` into `Ebenezer.Tests`.
- Modify `tests/Server/Ebenezer/Ebenezer.Tests.vcxproj` and `.filters`: compile and display the new route test in the checked-in MSBuild project.

---

### Task 1: Fixed CZ Route Catalog and Runtime Cursor

**Files:**
- Create: `src/Server/Ebenezer/BotRoute.h`
- Create: `src/Server/Ebenezer/BotRoute.cpp`
- Modify: `src/Server/Ebenezer/BotTypes.h`
- Modify: `src/Server/Ebenezer/CMakeLists.txt`
- Modify: `src/Server/Ebenezer/Ebenezer.Core.vcxproj`
- Modify: `src/Server/Ebenezer/Ebenezer.Core.vcxproj.filters`
- Create: `tests/Server/Ebenezer/BotRoute_test.cpp`
- Modify: `tests/Server/Ebenezer/CMakeLists.txt`
- Modify: `tests/Server/Ebenezer/Ebenezer.Tests.vcxproj`
- Modify: `tests/Server/Ebenezer/Ebenezer.Tests.vcxproj.filters`

**Interfaces:**
- Consumes: `BotRuntime`, `BotSpawnPoint`, nation constants, `ZONE_FRONTIER`, and `C3DMap::IsValidPosition(float, float) const`.
- Produces: `BotRoute::Resolve(uint8_t, uint8_t)`, `CurrentDestination(uint8_t, uint8_t, const BotRuntime&)`, `Advance(BotRuntime&, uint8_t, uint8_t)`, `Reset(BotRuntime&)`, and `Validate(uint8_t, uint8_t, const C3DMap&)`.

- [ ] **Step 1: Add the failing route catalog tests and register the test source**

Create `tests/Server/Ebenezer/BotRoute_test.cpp` with the exact behavioral assertions:

```cpp
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
```

Add `BotRoute_test.cpp` to `tests/Server/Ebenezer/CMakeLists.txt`, `Ebenezer.Tests.vcxproj`, and `Ebenezer.Tests.vcxproj.filters`.

- [ ] **Step 2: Build the test target and prove the new interface is missing**

Stop the currently running local stack only when the build first needs to replace binaries:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\local\Stop-Local.ps1
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\local\Build-Local.ps1 -Configuration Debug
```

Expected: build fails because `Ebenezer/BotRoute.h` and the new cursor fields do not exist. Do not weaken or remove the tests.

- [ ] **Step 3: Add route cursor fields and the complete route interface**

Keep `patrolIndex` for the existing fallback patrol and add these fields to `BotRuntime` in `BotTypes.h`:

```cpp
size_t patrolIndex = 0;
size_t routeIndex = 0;
size_t bowlPatrolIndex = 0;
bool reachedBowl = false;
```

Create `BotRoute.h`:

```cpp
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
```

- [ ] **Step 4: Implement the fixed arrays and pure cursor transitions**

Create `BotRoute.cpp` with fixed `constexpr std::array` values and no database or timer access:

```cpp
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
```

Add `BotRoute.cpp` and `BotRoute.h` beside the other bot sources in `src/Server/Ebenezer/CMakeLists.txt`, `Ebenezer.Core.vcxproj`, and `Ebenezer.Core.vcxproj.filters`.

- [ ] **Step 5: Build and run the focused route tests**

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\local\Build-Local.ps1 -Configuration Debug
.\bin\Debug-x64\Ebenezer.Tests\Ebenezer.Tests.exe --gtest_filter=BotRouteTest.*
```

Expected: build succeeds with zero errors and every `BotRouteTest.*` test passes.

- [ ] **Step 6: Commit the pure route model**

```powershell
git add src/Server/Ebenezer/BotRoute.h src/Server/Ebenezer/BotRoute.cpp src/Server/Ebenezer/BotTypes.h src/Server/Ebenezer/CMakeLists.txt src/Server/Ebenezer/Ebenezer.Core.vcxproj src/Server/Ebenezer/Ebenezer.Core.vcxproj.filters tests/Server/Ebenezer/BotRoute_test.cpp tests/Server/Ebenezer/CMakeLists.txt tests/Server/Ebenezer/Ebenezer.Tests.vcxproj tests/Server/Ebenezer/Ebenezer.Tests.vcxproj.filters docs/superpowers/plans/2026-07-18-cz-town-to-bowl-bot-routes-implementation-plan.md
git commit -m "feat: add fixed CZ bot routes"
```

---

### Task 2: Route-Aware Movement, Combat Resume, and Respawn Reset

**Files:**
- Modify: `src/Server/Ebenezer/BotCommandFacade.cpp`
- Modify: `tests/Server/Ebenezer/BotManager_test.cpp`
- Modify: `tests/Server/Ebenezer/BotUserIntegration_test.cpp`

**Interfaces:**
- Consumes: Task 1 `BotRoute` methods, existing `BotMovement::NextStep()` / `Move()`, and the existing `BotIntentType::Patrol` branch.
- Produces: route-aware behavior through the unchanged `bool BotCommandFacade::Patrol(CBotUser&, float)` signature; `Respawn()` resets route state.

- [ ] **Step 1: Add failing movement, combat-resume, and respawn-reset tests**

Change the `BotManagerTest` zone 201 fixture map to `CreateMap(ZONE_FRONTIER, 513, 2.0f)` so Bowl coordinates are valid, then add:

```cpp
TEST_F(BotManagerTest, FrontierPatrolMovesKarusTowardFirstOutwardWaypoint)
{
	auto bot = Spawn("Bot_K_Route", NATION_KARUS, 848.0f, 128.0f);
	ASSERT_NE(bot, nullptr);

	_app->GetBotManager().Tick(_now);
	_app->GetBotManager().Tick(_now + 200ms);

	EXPECT_GT(bot->m_fWill_x, 848.0f);
	EXPECT_GT(bot->m_fWill_z, 128.0f);
	EXPECT_EQ(bot->Runtime().routeIndex, 0u);
	EXPECT_FALSE(bot->Runtime().reachedBowl);
}

TEST_F(BotManagerTest, ReachingWaypointAdvancesOnceWithoutOvershoot)
{
	auto bot = Spawn("Bot_K_Reach", NATION_KARUS, 889.0f, 349.5f);
	ASSERT_NE(bot, nullptr);
	BotCommandFacade commands(*_app);

	ASSERT_TRUE(commands.Patrol(*bot, 1.5f));
	EXPECT_FLOAT_EQ(bot->m_fWill_x, 890.0f);
	EXPECT_FLOAT_EQ(bot->m_fWill_z, 350.0f);
	EXPECT_EQ(bot->Runtime().routeIndex, 1u);
	ASSERT_TRUE(commands.Patrol(*bot, 1.5f));
	EXPECT_EQ(bot->Runtime().routeIndex, 1u);
}

TEST_F(BotManagerTest, CombatDoesNotConsumeRouteCursorAndPatrolResumesStoredWaypoint)
{
	auto source = Spawn("Bot_K_Resume", NATION_KARUS, 925.0f, 595.0f);
	auto enemy = Spawn("Bot_E_Resume", NATION_ELMORAD, 927.0f, 595.0f);
	ASSERT_NE(source, nullptr);
	ASSERT_NE(enemy, nullptr);
	source->Runtime().routeIndex = 1;
	BotCommandFacade commands(*_app);

	ASSERT_TRUE(commands.BasicAttack(*source, enemy->GetSocketID(), _now));
	EXPECT_EQ(source->Runtime().routeIndex, 1u);
	enemy->m_pUserData->m_sHp = 0;
	const float before = source->GetDistance2D(930.0f, 600.0f);
	ASSERT_TRUE(commands.Patrol(*source, 1.5f));
	EXPECT_EQ(source->Runtime().routeIndex, 1u);
	EXPECT_LT(std::hypot(source->m_fWill_x - 930.0f, source->m_fWill_z - 600.0f), before);
}
```

Extend `RespawnsOnlyAtExactBoundaryAtHomeWithCleanRuntimeAndRegions` before death with non-default `routeIndex`, `bowlPatrolIndex`, `reachedBowl`, and `patrolIndex`; after respawn expect all four to be reset exactly as in Task 1. Extend `BotUserIntegrationTest.InitializesAndRegistersSocketlessBot` to assert the three new route fields start at zero/false.

- [ ] **Step 2: Run focused tests and verify route movement is still absent**

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\local\Build-Local.ps1 -Configuration Debug
.\bin\Debug-x64\Ebenezer.Tests\Ebenezer.Tests.exe --gtest_filter=BotManagerTest.FrontierPatrolMovesKarusTowardFirstOutwardWaypoint:BotManagerTest.ReachingWaypointAdvancesOnceWithoutOvershoot:BotManagerTest.CombatDoesNotConsumeRouteCursorAndPatrolResumesStoredWaypoint:BotManagerTest.RespawnsOnlyAtExactBoundaryAtHomeWithCleanRuntimeAndRegions:BotUserIntegrationTest.InitializesAndRegistersSocketlessBot
```

Expected: the new route movement/advancement/reset assertions fail against the old HOME-offset patrol.

- [ ] **Step 3: Make `Patrol()` follow a route when one exists and preserve fallback patrol elsewhere**

Include `BotRoute.h`. At the start of `BotCommandFacade::Patrol()`, resolve the destination. When it exists, validate it against the bot's current loaded map, calculate remaining distance from the finite pending endpoint (`m_fWill_x/m_fWill_z`, falling back to current coordinates), create the next step, move it through the existing facade, and advance only when the distance before that step is at most `min(moveStep, 1.5f)`:

```cpp
const auto destination = BotRoute::CurrentDestination(
	source.m_pUserData->m_bNation, source.m_pUserData->m_bZone, source.Runtime());
if (destination.has_value())
{
	C3DMap* routeMap = _app.GetMapByID(destination->zoneId);
	if (routeMap == nullptr || !routeMap->IsValidPosition(destination->x, destination->z))
		return false;
	const float originX = std::isfinite(source.m_fWill_x)
		? source.m_fWill_x : source.m_pUserData->m_curx;
	const float originZ = std::isfinite(source.m_fWill_z)
		? source.m_fWill_z : source.m_pUserData->m_curz;
	const float remaining = std::hypot(destination->x - originX, destination->z - originZ);
	if (!std::isfinite(remaining))
		return false;
	const float reachThreshold = std::min(moveStep, 1.5f);
	const BotSpawnPoint step = BotMovement::NextStep(
		source, destination->x, destination->z, moveStep);
	if (!BotMovement::Move(source, step, MOVE_SPEED))
		return false;
	if (remaining <= reachThreshold)
		BotRoute::Advance(source.Runtime(), source.m_pUserData->m_bNation,
			source.m_pUserData->m_bZone);
	return true;
}
```

Leave the existing four HOME-offset loop below this block unchanged as the fallback for non-zone-201 bots.

- [ ] **Step 4: Reset navigation only after a successful respawn validation**

In `BotCommandFacade::Respawn()`, after the HOME and map validation succeeds and immediately before `USER_REGENE`, call:

```cpp
BotRoute::Reset(runtime);
```

Do not reset before validation: `RespawnRejectsInvalidHomeWithoutRemovingLiveRegionEntry` must preserve the live bot and its runtime on failure.

- [ ] **Step 5: Run focused and complete Ebenezer tests**

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\local\Build-Local.ps1 -Configuration Debug
.\bin\Debug-x64\Ebenezer.Tests\Ebenezer.Tests.exe --gtest_filter=BotRouteTest.*:BotManagerTest.*:BotBrainTest.*:BotUserIntegrationTest.*
.\bin\Debug-x64\Ebenezer.Tests\Ebenezer.Tests.exe
```

Expected: all focused tests pass, then the complete Ebenezer suite passes with no regressions.

- [ ] **Step 6: Commit route movement and respawn integration**

```powershell
git add src/Server/Ebenezer/BotCommandFacade.cpp tests/Server/Ebenezer/BotManager_test.cpp tests/Server/Ebenezer/BotUserIntegration_test.cpp
git commit -m "feat: march CZ bots toward Bowl"
```

---

### Task 3: Atomic Startup Validation and Nation Town Spawns

**Files:**
- Modify: `src/Server/Ebenezer/EbenezerApp.h`
- Modify: `src/Server/Ebenezer/EbenezerApp.cpp`
- Modify: `tests/Server/Ebenezer/BotOperationMessage_test.cpp`

**Interfaces:**
- Consumes: Task 1 `BotRoute::Validate()` and the existing automatic `BuildBotBatchRequests()` / `StartConfiguredRoster()` transaction.
- Produces: private `bool EbenezerApp::ValidateConfiguredBotRoutes()`; on failure `_botConfig.enabled == false`, registry remains empty, timer remains stopped, and ordinary server startup continues.

- [ ] **Step 1: Add failing exact-HOME and invalid-route transaction tests**

Use a zone 201 fixture large enough for the routes and the real local HOME anchors:

```cpp
#include <Ebenezer/BotUser.h>

ASSERT_NE(_app->CreateMap(ZONE_FRONTIER, 513, 4.0f), nullptr);
ASSERT_TRUE(_app->AddHomeEntry(NATION_KARUS, 848, 128));
ASSERT_TRUE(_app->AddHomeEntry(NATION_ELMORAD, 193, 898));
```

Extend `AutoRosterCreatesExactlyFivePerNationAfterValidation`:

```cpp
size_t karus = 0;
size_t elmorad = 0;
for (const auto& entry : _app->GetBotRegistry().Snapshot())
{
	auto bot = std::dynamic_pointer_cast<CBotUser>(entry);
	ASSERT_NE(bot, nullptr);
	ASSERT_NE(bot->m_pUserData, nullptr);
	if (bot->m_pUserData->m_bNation == NATION_KARUS)
	{
		++karus;
		EXPECT_GE(bot->m_pUserData->m_curx, 848.0f);
		EXPECT_LE(bot->m_pUserData->m_curx, 868.0f);
		EXPECT_GE(bot->m_pUserData->m_curz, 128.0f);
		EXPECT_LE(bot->m_pUserData->m_curz, 148.0f);
	}
	else
	{
		++elmorad;
		EXPECT_EQ(bot->m_pUserData->m_bNation, NATION_ELMORAD);
		EXPECT_GE(bot->m_pUserData->m_curx, 193.0f);
		EXPECT_LE(bot->m_pUserData->m_curx, 213.0f);
		EXPECT_GE(bot->m_pUserData->m_curz, 898.0f);
		EXPECT_LE(bot->m_pUserData->m_curz, 918.0f);
	}
	EXPECT_EQ(bot->Runtime().routeIndex, 0u);
	EXPECT_FALSE(bot->Runtime().reachedBowl);
}
EXPECT_EQ(karus, 5u);
EXPECT_EQ(elmorad, 5u);
```

Add an atomic invalid-route test with both HOME rows present but a map too small for the Bowl:

```cpp
TEST(BotConfiguredStartupTest, InvalidCzRouteDisablesBotsBeforeRosterCommit)
{
	TestApp app;
	ASSERT_NE(app.CreateMap(ZONE_FRONTIER, 256, 1.0f), nullptr);
	ASSERT_TRUE(app.AddHomeEntry(NATION_KARUS, 120, 120));
	ASSERT_TRUE(app.AddHomeEntry(NATION_ELMORAD, 140, 140));
	CIni ini;
	ini.SetInt("BOTS", "Enabled", 1);
	ini.SetInt("BOTS", "Count", 10);
	ASSERT_TRUE(app.LoadBotConfig(ini));
	ASSERT_TRUE(app.ValidateBotConfigZone());

	EXPECT_FALSE(app.StartConfiguredBots());
	EXPECT_FALSE(app.GetBotConfig().enabled);
	EXPECT_EQ(app.GetBotRegistry().Size(), 0u);
	EXPECT_FALSE(app.GetBotManager().Status().running);
}
```

- [ ] **Step 2: Run startup tests and prove route validation is not wired yet**

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\local\Build-Local.ps1 -Configuration Debug
.\bin\Debug-x64\Ebenezer.Tests\Ebenezer.Tests.exe --gtest_filter=BotOperationMessageTest.AutoRosterCreatesExactlyFivePerNationAfterValidation:BotConfiguredStartupTest.InvalidCzRouteDisablesBotsBeforeRosterCommit
```

Expected: the exact-HOME test passes after fixture adjustment, but the invalid-route transaction test fails because the old startup accepts the small loaded map.

- [ ] **Step 3: Add configured-route validation before any bot is created**

Declare `bool ValidateConfiguredBotRoutes();` in the private section of `EbenezerApp`. Include `BotRoute.h` in `EbenezerApp.cpp` and implement:

```cpp
bool EbenezerApp::ValidateConfiguredBotRoutes()
{
	C3DMap* map = GetMapByID(_botConfig.zoneId);
	if (map != nullptr
		&& BotRoute::Validate(NATION_KARUS, _botConfig.zoneId, *map)
		&& BotRoute::Validate(NATION_ELMORAD, _botConfig.zoneId, *map))
	{
		return true;
	}
	_botConfig.enabled = false;
	spdlog::error("EbenezerApp::ValidateConfiguredBotRoutes: invalid zone 201 bot route; bots disabled");
	return false;
}
```

In `StartConfiguredBots()`, preserve the existing non-milestone-count behavior. For the exact ten-bot roster, call `ValidateConfiguredBotRoutes()` after the `count != 10` early return and before constructing the first `BotSpawnRequest`:

```cpp
if (!ValidateConfiguredBotRoutes())
	return false;
```

This ordering is the atomicity guarantee: no registry or region mutation has happened when validation fails.

- [ ] **Step 4: Run startup, operation, and full Ebenezer suites**

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\local\Build-Local.ps1 -Configuration Debug
.\bin\Debug-x64\Ebenezer.Tests\Ebenezer.Tests.exe --gtest_filter=BotOperationMessageTest.*:BotConfiguredStartupTest.*:BotConfiguredCountTest.*
.\bin\Debug-x64\Ebenezer.Tests\Ebenezer.Tests.exe
```

Expected: automatic roster has exactly five bots at each authoritative HOME range; invalid routes leave `total=0`, `running=false`, and bot config disabled; every Ebenezer test passes.

- [ ] **Step 5: Commit atomic route startup**

```powershell
git add src/Server/Ebenezer/EbenezerApp.h src/Server/Ebenezer/EbenezerApp.cpp tests/Server/Ebenezer/BotOperationMessage_test.cpp
git commit -m "feat: validate CZ bot routes at startup"
```

---

### Task 4: Full Regression and Local Gameplay Acceptance

**Files:**
- Verify only: all modified source/test files
- Runtime evidence: ignored `local/logs/*`, generated INIs, and `local/pids.json`

**Interfaces:**
- Consumes: Tasks 1-3 and the existing `Stop-Local.ps1`, `Build-Local.ps1`, `Start-Local.ps1`, client, test account, and `+bot_status` command.
- Produces: a verified Debug build, green complete test suites, live zone 201 town-to-Bowl combat evidence, correct 15-second HOME respawn, and a running playable localhost stack.

- [ ] **Step 1: Inspect the final diff before running completion checks**

```powershell
git status --short
git diff --check
git diff --stat HEAD~3..HEAD
```

Expected: only the planned bot route source, tests, CMake entries, and plan/spec files are present; `git diff --check` prints nothing.

- [ ] **Step 2: Run the complete Debug build and every maintained native test suite**

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\local\Build-Local.ps1 -Configuration Debug
.\bin\Debug-x64\WarFare.Network.Tests\WarFare.Network.Tests.exe
.\bin\Debug-x64\Ebenezer.Tests\Ebenezer.Tests.exe
.\bin\Debug-x64\FileIO.Tests\FileIO.Tests.exe
.\bin\Debug-x64\MathUtils.Tests\MathUtils.Tests.exe
```

Expected: all four solutions build with zero errors and all four test executables report zero failed tests. Record the fresh totals from the output; do not reuse earlier counts.

- [ ] **Step 3: Start the local stack and verify process ownership**

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\local\Start-Local.ps1
Get-Content -LiteralPath .\local\pids.json
```

Expected: Aujard, ItemManager, VersionManager, AIServer, Ebenezer, and KnightOnLine have live owned PIDs; every listener remains bound to `127.0.0.1`. Never print or expose `local/.env.local`.

- [ ] **Step 4: Perform live zone 201 acceptance**

Use the visible client and the local `testing/testing` test account:

1. Enter character `Testing` in zone 201.
2. Observe Karus bots spawning inside the Karus CZ HOME range `(848..868,128..148)` and moving first toward `(890,350)`.
3. Observe El Morad bots spawning inside the Human CZ HOME range `(193..213,898..918)` and moving first toward `(400,930)`.
4. Allow roughly two minutes at the 200 ms tick and 1.5-unit maximum step for both routes to reach the Bowl neighborhood.
5. At Bowl, verify `Bot_K_*` and `Bot_E_*` enter neighboring regions, select opponents, approach, damage, kill, and continue the four-point Bowl loop whenever no enemy is valid.
6. During combat, verify a bot resumes its stored route/loop point after its target dies or disappears.
7. Kill at least one bot; verify it remains dead for 15 seconds, respawns in its own nation HOME town, and starts at its first outward waypoint again.
8. Run `+bot_status`; require `total=10`, `alive+dead=10`, and `running=true`.

Expected: both teams meet through ordinary movement, not teleportation, and all movement/attack/respawn events remain server-authoritative.

- [ ] **Step 5: Request a code review and address only evidence-backed findings**

Review the three implementation commits against the approved spec, focusing on route bounds, cursor advancement, target priority, respawn reset ordering, startup atomicity, and regressions in non-zone-201 patrol. If a finding changes code, rerun the focused suite plus the complete Ebenezer suite and commit the correction separately.

- [ ] **Step 6: Run final post-review verification and leave the game playable**

```powershell
git status --short
git diff --check
.\bin\Debug-x64\WarFare.Network.Tests\WarFare.Network.Tests.exe
.\bin\Debug-x64\Ebenezer.Tests\Ebenezer.Tests.exe
.\bin\Debug-x64\FileIO.Tests\FileIO.Tests.exe
.\bin\Debug-x64\MathUtils.Tests\MathUtils.Tests.exe
```

Expected: working tree is clean, whitespace check has no output, all tests pass fresh, `+bot_status` remains consistent, and the client plus all six localhost services remain running for the user.

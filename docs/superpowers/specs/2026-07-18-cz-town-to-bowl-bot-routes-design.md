# CZ Town-to-Bowl Bot Routes Design

## Goal

Keep Karus bots spawning in Karus CZ town and El Morad bots spawning in Human CZ town, then make both teams travel through deterministic routes to the CZ Bowl where they can find and fight one another. Death returns a bot to its own town; after the existing 15-second respawn delay it starts the route again.

## Scope

This increment changes navigation behavior only. It preserves the existing ten transient socketless bots, 200 ms tick, 15-second respawn, authoritative movement/attack paths, nearest-enemy targeting, bot ID band, and localhost operation. It does not add navmesh pathfinding, teleporting, parties, skills, potions, equipment, persistence, or additional bot counts.

## Architecture

Each nation receives an ordered CZ route whose origin is its own HOME `FreeZone` town spawn and whose final point is the shared Bowl rally area at `(1000,1000)`, matching zone 201's configured initial center. A bot keeps a route cursor in `BotRuntime`; after spawning at HOME, that cursor starts at the first outward travel waypoint rather than sending an offset spawn back to the exact HOME anchor. When no valid enemy is visible, the existing `Patrol` intent means route travel until Bowl and Bowl-loop travel afterward. `BotManager` asks `BotCommandFacade` to advance toward the current waypoint using the existing validated movement path with a maximum 1.5-unit step. A waypoint is reached when the remaining planar distance is at most 1.5 units, at which point the cursor advances without overshooting. Reaching the final Bowl point switches the bot to the deterministic patrol loop `(990,990) -> (1010,990) -> (1010,1010) -> (990,1010)`.

Enemy acquisition always has priority over route travel. If an enemy becomes valid, the bot approaches or attacks through the existing brain and combat facade. When the target disappears or dies, the bot resumes the same stored route waypoint or Bowl patrol point. Respawn restores the town home position, resets the route cursor to the first outward waypoint, and begins the march again.

## Route Data and Validation

The zone 201 routes are fixed for this local milestone:

- Karus: spawn at HOME town `(848,128)`; travel `(890,350)` -> `(930,600)` -> `(970,820)` -> Bowl `(1000,1000)`.
- El Morad/Human: spawn at HOME town `(193,898)`; travel `(400,930)` -> `(620,960)` -> `(820,980)` -> Bowl `(1000,1000)`.

The town coordinates come from the imported HOME table and remain the authoritative respawn points. The Bowl coordinate comes from zone 201's `ZONE_INFO` center. All route coordinates are verified against the loaded `C3DMap` before the roster is committed. Both lists end in the same region around the Bowl anchor so the current 3x3-region target selector can discover opponents.

Startup validation is atomic: every waypoint must be finite, belong to zone 201, and pass `IsValidPosition`. If either route is invalid, the configured ten-bot roster is not started and bot configuration is disabled without failing the ordinary game server.

Movement never teleports and never skips an invalid step. The existing zone 201 server map accepts finite in-bounds positions; every 1.5-unit step is still checked by `IsValidPosition`. If a step is rejected, the bot waits and retries on a later tick rather than leaving map bounds.

## Behavior Flow

1. Spawn five Karus bots at Karus CZ town and five El Morad bots at Human CZ town.
2. With no visible enemy, follow the nation route toward Bowl.
3. On enemy detection, suspend route travel and use existing approach/basic-attack behavior.
4. After combat, resume the same route waypoint stored before target acquisition.
5. At Bowl, patrol a small loop and continue target acquisition.
6. On death, wait exactly 15 seconds, respawn in the bot's own town, reset the route, and march again.

## Testing

Unit tests will prove that both nations spawn at the correct HOME town, select the first outward waypoint, converge on the same Bowl neighborhood, advance only within the 1.5-unit reach threshold, prioritize enemies over travel, resume the stored waypoint after target loss, reset to the first outward waypoint on respawn, loop around Bowl, and reject invalid route coordinates atomically. Existing movement, targeting, combat, configuration, and lifecycle suites must remain green.

Live acceptance will start the localhost stack, enter zone 201 with `Testing`, and verify that Karus and Human bots leave their respective towns, arrive in Bowl, become mutually visible, move, damage, die, and respawn at their own towns. `+bot_status` must continue to report `total=10` with `alive+dead=10` and `running=true`.

## Operations

The existing `Start-Local.ps1`, `Stop-Local.ps1`, ignored credentials, loopback bindings, and visible client launch remain unchanged. The final playable stack stays running after acceptance.

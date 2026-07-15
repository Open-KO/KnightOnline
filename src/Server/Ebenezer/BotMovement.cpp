#include "pch.h"
#include "BotMovement.h"

#include "BotUser.h"
#include "EbenezerApp.h"
#include "Map.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace Ebenezer
{

BotSpawnPoint BotMovement::NextStep(
	const CBotUser& source, float targetX, float targetZ, float maxStep)
{
	BotSpawnPoint destination;
	if (source.m_pUserData == nullptr)
		return destination;

	destination.zoneId = source.m_pUserData->m_bZone;
	destination.x      = source.m_pUserData->m_curx;
	destination.y      = source.m_pUserData->m_cury;
	destination.z      = source.m_pUserData->m_curz;
	if (!std::isfinite(source.m_fWill_x) || !std::isfinite(source.m_fWill_y)
		|| !std::isfinite(source.m_fWill_z))
		return destination;
	destination.x = source.m_fWill_x;
	destination.y = source.m_fWill_y;
	destination.z = source.m_fWill_z;
	if (!std::isfinite(targetX) || !std::isfinite(targetZ) || !std::isfinite(maxStep)
		|| maxStep <= 0.0f)
		return destination;

	const float deltaX   = targetX - destination.x;
	const float deltaZ   = targetZ - destination.z;
	const float distance = std::hypot(deltaX, deltaZ);
	if (!std::isfinite(distance) || distance == 0.0f)
		return destination;

	const float step  = std::min({ distance, maxStep, 1.5f });
	destination.x    += deltaX / distance * step;
	destination.z    += deltaZ / distance * step;
	return destination;
}

bool BotMovement::Move(CBotUser& source, const BotSpawnPoint& destination, int16_t speed)
{
	if (source.m_pUserData == nullptr || destination.zoneId != source.m_pUserData->m_bZone
		|| !std::isfinite(destination.x) || !std::isfinite(destination.y)
		|| !std::isfinite(destination.z) || destination.x < 0.0f || destination.z < 0.0f)
		return false;

	const float originX = std::isfinite(source.m_fWill_x) ? source.m_fWill_x
														: source.m_pUserData->m_curx;
	const float originY = std::isfinite(source.m_fWill_y) ? source.m_fWill_y
														: source.m_pUserData->m_cury;
	const float originZ = std::isfinite(source.m_fWill_z) ? source.m_fWill_z
														: source.m_pUserData->m_curz;
	if (!std::isfinite(originX) || !std::isfinite(originY) || !std::isfinite(originZ))
		return false;

	const float movementDistance = std::hypot(destination.x - originX, destination.z - originZ);
	// NextStep uses float arithmetic; allow only a sub-millimetre accumulation tolerance.
	constexpr float MAX_STEP_EPSILON = 0.001f;
	if (!std::isfinite(movementDistance) || movementDistance > 1.5f + MAX_STEP_EPSILON)
		return false;

	C3DMap* map = source.m_pMain == nullptr ? nullptr
											: source.m_pMain->GetMapByIndex(source.m_iZoneIndex);
	if (map == nullptr || !map->IsValidPosition(destination.x, destination.z))
		return false;

	constexpr float scale = 10.0f;
	if (destination.x * scale > std::numeric_limits<uint16_t>::max()
		|| destination.z * scale > std::numeric_limits<uint16_t>::max()
		|| destination.y * scale < std::numeric_limits<int16_t>::min()
		|| destination.y * scale > std::numeric_limits<int16_t>::max())
		return false;

	// MoveProcess advances current from the previous pending endpoint. Repair only invalid pending
	// coordinates from the finite authoritative fallback before entering that legacy pipeline.
	source.m_fWill_x = originX;
	source.m_fWill_y = originY;
	source.m_fWill_z = originZ;

	char movement[16] {};
	int index = 0;
	SetShort(movement, static_cast<uint16_t>(destination.x * scale), index);
	SetShort(movement, static_cast<uint16_t>(destination.z * scale), index);
	SetShort(movement, static_cast<int16_t>(destination.y * scale), index);
	SetShort(movement, speed, index);
	SetByte(movement, 0, index);
	source.MoveProcess(movement);
	return true;
}

} // namespace Ebenezer

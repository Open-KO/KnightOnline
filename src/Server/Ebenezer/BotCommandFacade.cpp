#include "pch.h"
#include "BotCommandFacade.h"

#include "BotMovement.h"
#include "BotUser.h"
#include "EbenezerApp.h"
#include "Map.h"

#include <array>
#include <cmath>
#include <utility>

namespace Ebenezer
{
namespace
{
constexpr int16_t MOVE_SPEED = 45;

bool IsAliveEnemy(const CBotUser& source, const std::shared_ptr<CUser>& target)
{
	return source.m_pUserData != nullptr && target != nullptr && target->m_pUserData != nullptr
		   && source.GetState() == CONNECTION_STATE_GAMESTART
		   && target->GetState() == CONNECTION_STATE_GAMESTART
		   && source.m_pUserData->m_bZone == target->m_pUserData->m_bZone
		   && source.m_pUserData->m_bNation != target->m_pUserData->m_bNation
		   && source.m_bResHpType != USER_DEAD && source.m_pUserData->m_sHp > 0
		   && target->m_bResHpType != USER_DEAD && target->m_pUserData->m_sHp > 0
		   && source.m_bAbnormalType != ABNORMAL_BLINKING
		   && target->m_bAbnormalType != ABNORMAL_BLINKING;
}
} // namespace

BotCommandFacade::BotCommandFacade(EbenezerApp& app) : _app(app)
{
}

bool BotCommandFacade::Approach(CBotUser& source, int targetId, float moveStep)
{
	auto target = _app.GetUserPtr(targetId);
	if (!IsAliveEnemy(source, target))
		return false;
	return BotMovement::Move(source,
		BotMovement::NextStep(
			 source, target->m_pUserData->m_curx, target->m_pUserData->m_curz, moveStep),
		MOVE_SPEED);
}

bool BotCommandFacade::BasicAttack(
	CBotUser& source, int targetId, std::chrono::steady_clock::time_point now, float attackRange)
{
	auto target = _app.GetUserPtr(targetId);
	if (!IsAliveEnemy(source, target) || now < source.Runtime().nextAttackAt)
		return false;

	const float distance = source.GetDistance2D(
		target->m_pUserData->m_curx, target->m_pUserData->m_curz);
	if (!std::isfinite(distance) || distance > attackRange)
		return false;

	char attack[16] {};
	int index = 0;
	SetByte(attack, DIRECT_ATTACK, index);
	SetByte(attack, 1, index);
	SetShort(attack, targetId, index);
	SetShort(attack, 100, index);
	SetShort(attack, static_cast<int16_t>(distance * 10.0f), index);
	source.Attack(attack);
	source.Runtime().nextAttackAt = now + std::chrono::seconds(1);
	return true;
}

bool BotCommandFacade::Patrol(CBotUser& source, float moveStep)
{
	if (source.m_pUserData == nullptr)
		return false;
	C3DMap* map = _app.GetMapByID(source.Runtime().home.zoneId);
	if (map == nullptr)
		return false;

	constexpr std::array<std::pair<float, float>, 4> offsets { std::pair { 10.0f, 0.0f },
		std::pair { 0.0f, 10.0f }, std::pair { -10.0f, 0.0f }, std::pair { 0.0f, -10.0f } };
	for (size_t attempt = 0; attempt < offsets.size(); ++attempt)
	{
		const size_t index  = source.Runtime().patrolIndex % offsets.size();
		const float targetX = source.Runtime().home.x + offsets[index].first;
		const float targetZ = source.Runtime().home.z + offsets[index].second;
		if (targetX < 0.0f || targetZ < 0.0f || !map->IsValidPosition(targetX, targetZ))
		{
			source.Runtime().patrolIndex = (index + 1) % offsets.size();
			continue;
		}

		const float distance = source.GetDistance2D(targetX, targetZ);
		if (!BotMovement::Move(
				source, BotMovement::NextStep(source, targetX, targetZ, moveStep), MOVE_SPEED))
			return false;
		source.Runtime().patrolIndex = distance <= moveStep ? (index + 1) % offsets.size() : index;
		return true;
	}
	return false;
}

void BotCommandFacade::PurgeRegionEntries(int userId)
{
	for (C3DMap* map : _app.m_ZoneArray)
	{
		if (map == nullptr)
			continue;
		for (int x = 0; x <= map->GetXRegionMax(); ++x)
			for (int z = 0; z <= map->GetZRegionMax(); ++z)
				map->RegionUserRemove(x, z, userId);
	}
}

bool BotCommandFacade::Respawn(CBotUser& source)
{
	const int userId = source.GetSocketID();
	if (source.m_pUserData == nullptr || _app.GetBotRegistry().Get(userId).get() != &source)
		return false;

	const auto& home    = source.Runtime().home;
	const int zoneIndex = _app.GetZoneIndex(home.zoneId);
	C3DMap* map         = _app.GetMapByIndex(zoneIndex);
	if (map == nullptr || !std::isfinite(home.x) || !std::isfinite(home.y)
		|| !std::isfinite(home.z) || home.x < 0.0f || home.z < 0.0f
		|| !map->IsValidPosition(home.x, home.z))
		return false;

	source.UserInOut(USER_OUT);
	PurgeRegionEntries(userId);

	source.m_pUserData->m_bZone = home.zoneId;
	source.m_iZoneIndex         = static_cast<int16_t>(zoneIndex);
	source.m_pUserData->m_curx = source.m_fWill_x = home.x;
	source.m_pUserData->m_cury = source.m_fWill_y = home.y;
	source.m_pUserData->m_curz = source.m_fWill_z = home.z;
	source.m_RegionX                              = static_cast<int16_t>(home.x / VIEW_DISTANCE);
	source.m_RegionZ                              = static_cast<int16_t>(home.z / VIEW_DISTANCE);
	source.m_pUserData->m_sHp                     = source.m_iMaxHp;
	source.m_pUserData->m_sMp                     = source.m_iMaxMp;
	source.m_bResHpType                           = USER_STANDING;
	source.m_bAbnormalType                        = ABNORMAL_NORMAL;
	source.SetState(CONNECTION_STATE_GAMESTART);
	auto& runtime        = source.Runtime();
	runtime.state        = BotState::SelectTarget;
	runtime.targetId     = -1;
	runtime.nextAttackAt = {};
	runtime.respawnAt    = {};
	source.UserInOut(USER_REGENE);
	return true;
}

bool BotCommandFacade::Despawn(CBotUser& source)
{
	const int userId = source.GetSocketID();
	if (_app.GetBotRegistry().Get(userId).get() != &source)
		return false;
	if (source.m_pUserData != nullptr)
	{
		try
		{
			source.UserInOut(USER_OUT);
		}
		catch (const std::exception& ex)
		{
			spdlog::error(
				"BotCommandFacade::Despawn: USER_OUT failed for bot {}: {}", userId, ex.what());
		}
		catch (...)
		{
			spdlog::error(
				"BotCommandFacade::Despawn: USER_OUT failed for bot {} with unknown error", userId);
		}
	}
	PurgeRegionEntries(userId);
	source.SetState(CONNECTION_STATE_DISCONNECTED);
	return _app.GetBotRegistry().Remove(userId) != nullptr;
}
} // namespace Ebenezer

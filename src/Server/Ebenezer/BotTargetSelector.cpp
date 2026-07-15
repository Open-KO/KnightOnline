#include "pch.h"
#include "BotTargetSelector.h"

#include "BotUser.h"
#include "EbenezerApp.h"
#include "Map.h"

#include <limits>
#include <mutex>
#include <unordered_set>

namespace Ebenezer
{

extern std::recursive_mutex g_region_mutex;

BotTargetSelector::BotTargetSelector(EbenezerApp& app) : _app(app)
{
}

int BotTargetSelector::SelectNearestEnemy(const CBotUser& source) const
{
	const auto* sourceData = source.m_pUserData;
	if (sourceData == nullptr)
		return -1;

	C3DMap* map = _app.GetMapByID(sourceData->m_bZone);
	if (map == nullptr || source.m_RegionX < 0 || source.m_RegionZ < 0
		|| source.m_RegionX > map->GetXRegionMax() || source.m_RegionZ > map->GetZRegionMax())
	{
		return -1;
	}

	std::unordered_set<int> candidateIds;
	{
		std::lock_guard<std::recursive_mutex> lock(g_region_mutex);
		for (int regionX = source.m_RegionX - 1; regionX <= source.m_RegionX + 1; ++regionX)
		{
			if (regionX < 0 || regionX > map->GetXRegionMax())
				continue;

			for (int regionZ = source.m_RegionZ - 1; regionZ <= source.m_RegionZ + 1; ++regionZ)
			{
				if (regionZ < 0 || regionZ > map->GetZRegionMax())
					continue;

				for (const auto& [_, candidateId] :
					map->m_ppRegion[regionX][regionZ].m_RegionUserArray)
				{
					if (candidateId != nullptr)
						candidateIds.insert(*candidateId);
				}
			}
		}
	}

	const int sourceId           = source.GetSocketID();
	int nearestId                = -1;
	float nearestDistanceSquared = std::numeric_limits<float>::infinity();

	for (int candidateId : candidateIds)
	{
		if (candidateId == sourceId)
			continue;

		auto candidate = _app.GetUserPtr(candidateId);
		if (candidate == nullptr || candidate->m_pUserData == nullptr
			|| candidate->GetState() != CONNECTION_STATE_GAMESTART
			|| candidate->m_pUserData->m_bZone != sourceData->m_bZone
			|| candidate->m_pUserData->m_bNation == sourceData->m_bNation
			|| candidate->m_pUserData->m_sHp == 0 || candidate->m_bResHpType == USER_DEAD)
		{
			continue;
		}

		const float distanceSquared = source.GetDistanceSquared2D(
			candidate->m_pUserData->m_curx, candidate->m_pUserData->m_curz);
		if (distanceSquared < nearestDistanceSquared
			|| (distanceSquared == nearestDistanceSquared
				&& (nearestId < 0 || candidateId < nearestId)))
		{
			nearestId              = candidateId;
			nearestDistanceSquared = distanceSquared;
		}
	}

	return nearestId;
}

} // namespace Ebenezer

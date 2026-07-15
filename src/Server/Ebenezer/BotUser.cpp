#include "pch.h"
#include "BotUser.h"

#include "EbenezerApp.h"
#include "Map.h"

#include <shared/StringUtils.h>

#include <cmath>

namespace Ebenezer
{

namespace
{

bool IsSupportedClass(uint8_t nation, e_Class characterClass)
{
	if (nation == NATION_KARUS)
	{
		return characterClass == CLASS_KA_WARRIOR || characterClass == CLASS_KA_ROGUE
			|| characterClass == CLASS_KA_WIZARD || characterClass == CLASS_KA_PRIEST;
	}

	if (nation == NATION_ELMORAD)
	{
		return characterClass == CLASS_EL_WARRIOR || characterClass == CLASS_EL_ROGUE
			|| characterClass == CLASS_EL_WIZARD || characterClass == CLASS_EL_PRIEST;
	}

	return false;
}

} // namespace

CBotUser::CBotUser() : CUser(test_tag {})
{
	m_pUserData = &_userData;
}

bool CBotUser::InitializeBot(const BotSpawnRequest& request)
{
	auto app = EbenezerApp::instance();
	if (_initialized || app == nullptr || request.name.empty() || request.name.length() > MAX_ID_SIZE
		|| !IsValidName(request.name.c_str()) || !IsSupportedClass(request.nation, request.characterClass)
		|| request.level == 0 || request.level > MAX_LEVEL || !std::isfinite(request.spawn.x)
		|| !std::isfinite(request.spawn.y) || !std::isfinite(request.spawn.z))
	{
		return false;
	}

	const int zoneIndex = app->GetZoneIndex(request.spawn.zoneId);
	auto map            = app->GetMapByIndex(zoneIndex);
	if (map == nullptr || map->m_nMapSize <= 1 || !std::isfinite(map->m_fUnitDist)
		|| map->m_fUnitDist <= 0.0f)
	{
		return false;
	}

	const float worldWidth = (map->m_nMapSize - 1) * map->m_fUnitDist;
	if (!std::isfinite(worldWidth) || worldWidth <= 0.0f || request.spawn.x < 0.0f
		|| request.spawn.z < 0.0f || request.spawn.x >= worldWidth
		|| request.spawn.z >= worldWidth)
	{
		return false;
	}

	_userData  = {};
	_runtime   = {};
	m_pUserData = &_userData;
	CUser::Initialize();

	strcpy_safe(m_pUserData->m_id, request.name);
	strcpy_safe(m_pUserData->m_Accountid, request.name);
	m_pUserData->m_bAuthority = AUTHORITY_USER;
	m_pUserData->m_bNation    = request.nation;
	m_pUserData->m_sClass     = request.characterClass;
	m_pUserData->m_bLevel     = request.level;
	m_pUserData->m_bZone      = request.spawn.zoneId;
	m_pUserData->m_curx       = request.spawn.x;
	m_pUserData->m_cury       = request.spawn.y;
	m_pUserData->m_curz       = request.spawn.z;
	m_fWill_x                 = request.spawn.x;
	m_fWill_y                 = request.spawn.y;
	m_fWill_z                 = request.spawn.z;
	m_iZoneIndex              = static_cast<int16_t>(zoneIndex);
	m_RegionX                 = static_cast<int16_t>(request.spawn.x / VIEW_DISTANCE);
	m_RegionZ                 = static_cast<int16_t>(request.spawn.z / VIEW_DISTANCE);

	m_iMaxHp                = 1500;
	m_pUserData->m_sHp      = m_iMaxHp;
	m_iMaxMp                = 500;
	m_pUserData->m_sMp      = m_iMaxMp;
	m_sTotalHit             = 180;
	m_sTotalAc              = 120;
	m_fTotalHitRate         = 1.0f;
	m_fTotalEvasionRate     = 1.0f;
	m_sSpeed                = 45;
	m_bAttackAmount         = 100;
	m_pUserData->m_iLoyalty = 100;
	m_pUserData->m_iGold    = 0;

	_runtime.state = BotState::Spawn;
	_runtime.home  = request.spawn;
	SetState(CONNECTION_STATE_GAMESTART);
	_initialized = true;
	return true;
}

int CBotUser::Send(char* pBuf, int length)
{
	(void) pBuf;
	return length;
}

BotRuntime& CBotUser::Runtime()
{
	return _runtime;
}

const BotRuntime& CBotUser::Runtime() const
{
	return _runtime;
}

} // namespace Ebenezer

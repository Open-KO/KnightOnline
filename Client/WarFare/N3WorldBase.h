// N3WorldBase.h: interface for the CN3WorldBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3WORLDBASE_H__8064AFB4_48C5_4CFB_8435_635B9F6F321D__INCLUDED_)
#define AFX_N3WORLDBASE_H__8064AFB4_48C5_4CFB_8435_635B9F6F321D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3BaseFileAccess.h>
#include <N3Base/N3SkyMng.h>

#include "GameBase.h"

class CN3Shape;
class CN3Terrain;
class CN3SkyMng;

class CN3WorldBase : public CN3BaseFileAccess, public CGameBase
{
	friend class CN3WorldManager;

public:
	// NOTE(srmeier): current setup carries no WIZ_ZONEABILITY information
	// I am adding this information here. Anything which inherits from CGameBase
	// should be able to get/set this information using the ACT_WORLD macro.
	// therefore CPlayerMySelf::IsAttackableTarget() should be able to check
	// against this information and GameProcMain should be able to process the WIZ_ZONEABILITY
	// packet and set this information.
	uint8_t m_byTariff;
	uint16_t m_zoneFlags;
	e_ZoneAbilityType m_zoneType;
	uint8_t m_byMinLevel, m_byMaxLevel;

	bool canTradeWithOtherNation() { return (m_zoneFlags & ZF_TRADE_OTHER_NATION) != 0; }
	bool canTalkToOtherNation() { return (m_zoneFlags & ZF_TALK_OTHER_NATION) != 0; }
	bool canAttackOtherNation() { return (m_zoneFlags & ZF_ATTACK_OTHER_NATION) != 0; }
	bool canAttackSameNation() { return (m_zoneFlags & ZF_ATTACK_SAME_NATION) != 0; }
	bool isWarZone() { return (m_zoneFlags & ZF_WAR_ZONE) != 0; }
	bool isNationPVPZone() { return canAttackOtherNation() && !canAttackSameNation(); }
	bool areNPCsFriendly() { return (m_zoneFlags & ZF_FRIENDLY_NPCS) != 0; }
	bool canUpdateClan() { return (m_zoneFlags & ZF_CLAN_UPDATE) != 0; }
	uint16_t GetZoneFlags(void) { return m_zoneFlags; }
	uint8_t GetZoneType() { return m_zoneType; }
	uint8_t GetTariff() { return m_byTariff; }
	void SetTariff(uint8_t tariff) { m_byTariff = tariff; }
	uint8_t GetMinLevelReq() { return m_byMinLevel; }
	uint8_t GetMaxLevelReq() { return m_byMaxLevel; }

	//..
	virtual CN3Terrain* GetTerrainRef() = 0;
	virtual CN3SkyMng* GetSkyRef() = 0;

	//..
	virtual void InitWorld(int iZoneID, const __Vector3& vPosPlayer) = 0;
	virtual void Tick() = 0;

	// Terrain..
	virtual bool CheckCollisionCameraWithTerrain(__Vector3& vEyeResult, const __Vector3& vAt, float fNP) = 0;
	virtual float GetHeightWithTerrain(float x, float z, bool bWarp = false) = 0;
	virtual BOOL PickWideWithTerrain(int x, int y, __Vector3& vPick) = 0;
	virtual bool CheckCollisionWithTerrain(__Vector3& vPos, __Vector3& vDir, float fVelocity, __Vector3* vCol) = 0;
	virtual void GetNormalWithTerrain(float x, float z, __Vector3& vNormal) = 0;
	virtual float GetWidthByMeterWithTerrain() = 0; 
	virtual bool IsInTerrainWithTerrain(float x, float z, __Vector3 vPosBefore = __Vector3()) = 0;
	virtual bool CheckInclineWithTerrain(const __Vector3& vPos, const __Vector3& vDir, float fIncline) = 0;

	// Shapes..
	virtual bool CheckCollisionCameraWithShape(__Vector3& vEyeResult, const __Vector3& vAt, float fNP) = 0;
	virtual float GetHeightNearstPosWithShape(const __Vector3& vPos, float fDist, __Vector3* pvNormal = NULL) = 0; 
	virtual void RenderCollisionWithShape(const __Vector3& vPos) = 0;
	virtual float GetHeightWithShape(float fX, float fZ, __Vector3* pvNormal = NULL) = 0; 
	virtual CN3Shape* ShapeGetByIDWithShape(int iID) = 0;
	virtual CN3Shape* PickWithShape(int iXScreen, int iYScreen, bool bMustHaveEvent, __Vector3* pvPick = NULL) = 0;
	virtual bool CheckCollisionWithShape(	const __Vector3& vPos,			// 충돌 위치
														const __Vector3& vDir,					    // 방향 벡터
														float fSpeedPerSec,							 // 초당 움직이는 속도
														__Vector3* pvCol = NULL,				  // 충돌 지점
														__Vector3* pvNormal = NULL,				// 충돌한면의 법선벡터
														__Vector3* pVec = NULL) = 0;			// 충돌한 면 의 폴리곤 __Vector3[3]

	// Sky..
	virtual D3DCOLOR GetSkyColorWithSky()	{	return 0x00000000;	}
	virtual float GetSunAngleByRadinWithSky()	{ 	return -FLT_MAX;	}
	virtual void RenderWeatherWithSky()	{}
	virtual void SetGameTimeWithSky(int iYear, int iMonth, int iDay, int iHour, int iMin)	{}
	virtual void SetWeatherWithSky(int iWeather, int iPercentage)	{}
	virtual D3DCOLOR GetLightDiffuseColorWithSky(int iIndex)	{	return 0xffffffff;	}
	virtual D3DCOLOR GetLightAmbientColorWithSky(int iIndex)	{	return 0xffffffff;	}
	virtual D3DCOLOR GetFogColorWithSky()	{	return 0xffffffff;	}
	//virtual CN3Sun*	GetSunPointerWithSky()	{	return NULL;	}

	// Bird..

	// Grass..
	virtual void InitWithGrass(__Vector3 CamPo)	{}

	// Rendering..
	virtual void RenderTerrain() = 0;
	virtual void RenderShape() = 0;	
	virtual void RenderSky() {}
	virtual void RenderGrass() {}
	virtual void RenderBirdMgr() {}
	virtual void RenderSkyWeather() {}

public:
	CN3WorldBase();
	virtual ~CN3WorldBase();
};

#endif // !defined(AFX_N3WORLDBASE_H__8064AFB4_48C5_4CFB_8435_635B9F6F321D__INCLUDED_)

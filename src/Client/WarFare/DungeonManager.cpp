// DungeonManager.cpp: implementation of the CDungeonManager class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "DungeonManager.h"

#include "PlayerMySelf.h"

CDungeonManager::CDungeonManager()
{
}

CDungeonManager::~CDungeonManager()
{
}

//////////////////////////////////////////////////////////////////////

void CDungeonManager::InitWorld(int iZoneID)
{
	__TABLE_ZONE* pZone = s_pTbl_Zones.Find(s_pPlayer->m_InfoExt.iZoneCur);
	if (pZone == nullptr)
	{
		CLogWriter::Write("Null Zone data : {}", iZoneID);
		return;
	}

	m_pvsmgr.LoadFromFile(pZone->szTerrainFN);
}

void CDungeonManager::Tick()
{
	m_pvsmgr.Tick();
}

void CDungeonManager::RenderTerrain()
{
	m_pvsmgr.Render();
}

void CDungeonManager::RenderShape()
{
}

CN3Terrain* CDungeonManager::GetTerrainRef()
{
	return nullptr;
}

CN3SkyMng* CDungeonManager::GetSkyRef()
{
	return nullptr;
}

//////////////////////////////////////////////////////////////////////

// Terrain..
bool CDungeonManager::CheckCollisionCameraWithTerrain(__Vector3& vEyeResult, const __Vector3& vAt, float fNP) // Done..
{
	return m_pvsmgr.CheckCollisionCameraWithTerrain(vEyeResult, vAt, fNP);
}

float CDungeonManager::GetHeightWithTerrain(float x, float z) // Done..
{
	return m_pvsmgr.GetHeightWithTerrain(x, z);
}

BOOL CDungeonManager::PickWideWithTerrain(int x, int y, __Vector3& vPick) // Done..
{
	return m_pvsmgr.PickWideWithTerrain(x, y, vPick);
}

bool CDungeonManager::CheckCollisionWithTerrain(__Vector3& /*vPos*/, __Vector3& /*vDir*/, float /*fVelocity*/, __Vector3* /*vCol*/)
{
	return false;
}

void CDungeonManager::GetNormalWithTerrain(float /*x*/, float /*z*/, __Vector3& /*vNormal*/)
{
}

float CDungeonManager::GetWidthByMeterWithTerrain()
{
	return 100.0f;
}

bool CDungeonManager::IsInTerrainWithTerrain(float x, float z) // Done..
{
	return m_pvsmgr.IsInTerrainWithTerrain(x, z);
}

bool CDungeonManager::CheckInclineWithTerrain(const __Vector3& /*vPos*/, const __Vector3& /*vDir*/, float /*fIncline*/)
{
	return false;
}

// Shapes..
bool CDungeonManager::CheckCollisionCameraWithShape(__Vector3& vEyeResult, const __Vector3& vAt, float fNP) // Done..
{
	return m_pvsmgr.CheckCollisionCameraWithShape(vEyeResult, vAt, fNP);
}

float CDungeonManager::GetHeightNearstPosWithShape(const __Vector3& vPos, __Vector3* pvNormal) // Done..
{
	return m_pvsmgr.GetHeightNearstPosWithShape(vPos, pvNormal);
}

void CDungeonManager::RenderCollisionWithShape(const __Vector3& vPos) // No need..
{
}

float CDungeonManager::GetHeightWithShape(float fX, float fZ, __Vector3* pvNormal)
{
	return m_pvsmgr.GetHeightWithShape(fX, fZ, pvNormal);
}

bool CDungeonManager::CheckCollisionWithShape(const __Vector3& vPos, // 충돌 위치
	const __Vector3& vDir,                                           // 방향 벡터
	float fSpeedPerSec,                                              // 초당 움직이는 속도
	__Vector3* pvCol,                                                // 충돌 지점
	__Vector3* pvNormal,                                             // 충돌한면의 법선벡터
	__Vector3* pVec)                                                 // 충돌한 면 의 폴리곤 __Vector3[3]
{
	return m_pvsmgr.CheckCollisionWithShape(vPos, vDir, fSpeedPerSec, pvCol, pvNormal, pVec);
}

CN3Shape* CDungeonManager::ShapeGetByIDWithShape(int iID)
{
	return m_pvsmgr.ShapeGetByIDWithShape(iID);
}

CN3Shape* CDungeonManager::PickWithShape(int iXScreen, int iYScreen, bool bMustHaveEvent, __Vector3* pvPick)
{
	return m_pvsmgr.PickWithShape(iXScreen, iYScreen, bMustHaveEvent, pvPick);
}


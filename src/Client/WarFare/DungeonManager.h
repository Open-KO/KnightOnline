// DungeonManager.h: interface for the CDungeonManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DUNGEONMANAGER_H__492B3E57_9A05_4F4D_B98C_63CB6ACC572E__INCLUDED_)
#define AFX_DUNGEONMANAGER_H__492B3E57_9A05_4F4D_B98C_63CB6ACC572E__INCLUDED_

#pragma once

#include "N3WorldBase.h"
#include "PvsMgr.h"

class CDungeonManager : public CN3WorldBase
{
	friend class CN3WorldManager;

	//..
	CPvsMgr m_pvsmgr;

	// Function..
	void InitWorld(int iZoneID) override;
	void Tick() override;

	//////////////////////////////////////////////////////////////////////

	CN3Terrain* GetTerrainRef() override;
	CN3SkyMng* GetSkyRef() override;

	// Terrain..
	bool CheckCollisionCameraWithTerrain(__Vector3& vEyeResult, const __Vector3& vAt, float fNP) override;
	float GetHeightWithTerrain(float x, float z) override;
	BOOL PickWideWithTerrain(int x, int y, __Vector3& vPick) override;
	bool CheckCollisionWithTerrain(__Vector3& vPos, __Vector3& vDir, float fVelocity, __Vector3* vCol) override;
	void GetNormalWithTerrain(float x, float z, __Vector3& vNormal) override;
	float GetWidthByMeterWithTerrain() override;
	bool IsInTerrainWithTerrain(float x, float z) override;
	bool CheckInclineWithTerrain(const __Vector3& vPos, const __Vector3& vDir, float fIncline) override;

	// Shapes..
	bool CheckCollisionCameraWithShape(__Vector3& vEyeResult, const __Vector3& vAt, float fNP) override;
	float GetHeightNearstPosWithShape(const __Vector3& vPos, __Vector3* pvNormal = nullptr) override;
	void RenderCollisionWithShape(const __Vector3& vPos) override;
	float GetHeightWithShape(float fX, float fZ, __Vector3* pvNormal = nullptr) override;
	CN3Shape* ShapeGetByIDWithShape(int iID) override;
	CN3Shape* PickWithShape(int iXScreen, int iYScreen, bool bMustHaveEvent, __Vector3* pvPick = nullptr) override;
	bool CheckCollisionWithShape(const __Vector3& vPos, // 충돌 위치
		const __Vector3& vDir,                          // 방향 벡터
		float fSpeedPerSec,                             // 초당 움직이는 속도
		__Vector3* pvCol    = nullptr,                  // 충돌 지점
		__Vector3* pvNormal = nullptr,                  // 충돌한면의 법선벡터
		__Vector3* pVec     = nullptr)                      // 충돌한 면 의 폴리곤 __Vector3[3]
		override;

	// Rendering..
	void RenderTerrain() override;
	void RenderShape() override;

public:
	CDungeonManager();
	~CDungeonManager() override;
};

#endif // !defined(AFX_DUNGEONMANAGER_H__492B3E57_9A05_4F4D_B98C_63CB6ACC572E__INCLUDED_)

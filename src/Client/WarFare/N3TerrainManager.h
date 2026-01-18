// N3TerrainManager.h: interface for the CN3TerrainManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3TERRAINMANAGER_H__4FA514AC_F8F8_4E09_88DC_AC31F2C9B21D__INCLUDED_)
#define AFX_N3TERRAINMANAGER_H__4FA514AC_F8F8_4E09_88DC_AC31F2C9B21D__INCLUDED_

#pragma once

#include "N3WorldBase.h"

class CN3TerrainManager : public CN3WorldBase
{
	friend class CN3WorldManager;

	class CN3Terrain* m_pTerrain; // 지형 클래스
	class CN3ShapeMgr* m_pShapes; // 물체 클래스
	class CN3SkyMng* m_pSky;      // 하늘 클래스
	class CBirdMng* m_pBirdMng;   // 하늘에 날라다니는 새들 관리..

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

	// Sky..
	D3DCOLOR GetSkyColorWithSky() override;
	float GetSunAngleByRadinWithSky() override;
	void RenderWeatherWithSky() override;
	void SetGameTimeWithSky(int iYear, int iMonth, int iDay, int iHour, int iMin) override;
	void SetWeatherWithSky(int iWeather, int iPercentage) override;
	D3DCOLOR GetLightDiffuseColorWithSky(int iIndex) override;
	D3DCOLOR GetLightAmbientColorWithSky(int iIndex) override;
	D3DCOLOR GetFogColorWithSky() override;

	// Bird..

	// Rendering..
	void RenderTerrain() override;
	void RenderShape() override;
	void RenderSky() override;
	void RenderBirdMgr() override;
	void RenderSkyWeather() override;

public:
	CN3TerrainManager();
	~CN3TerrainManager() override;
};

#endif // !defined(AFX_N3TERRAINMANAGER_H__4FA514AC_F8F8_4E09_88DC_AC31F2C9B21D__INCLUDED_)

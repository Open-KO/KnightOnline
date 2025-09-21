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
	virtual float GetHeightNearstPosWithShape(const __Vector3& vPos, float fDist, __Vector3* pvNormal = nullptr) = 0; 
	virtual void RenderCollisionWithShape(const __Vector3& vPos) = 0;
	virtual float GetHeightWithShape(float fX, float fZ, __Vector3* pvNormal = nullptr) = 0; 
	virtual CN3Shape* ShapeGetByIDWithShape(int iID) = 0;
	virtual CN3Shape* PickWithShape(int iXScreen, int iYScreen, bool bMustHaveEvent, __Vector3* pvPick = nullptr) = 0;
	virtual bool CheckCollisionWithShape(	const __Vector3& vPos,			// 충돌 위치
														const __Vector3& vDir,					    // 방향 벡터
														float fSpeedPerSec,							 // 초당 움직이는 속도
														__Vector3* pvCol = nullptr,				  // 충돌 지점
														__Vector3* pvNormal = nullptr,				// 충돌한면의 법선벡터
														__Vector3* pVec = nullptr) = 0;			// 충돌한 면 의 폴리곤 __Vector3[3]

	// Sky..
	virtual D3DCOLOR GetSkyColorWithSky()	{	return 0x00000000;	}
	virtual float GetSunAngleByRadinWithSky()	{ 	return -FLT_MAX;	}
	virtual void RenderWeatherWithSky()	{}
	virtual void SetGameTimeWithSky(int iYear, int iMonth, int iDay, int iHour, int iMin)	{}
	virtual void SetWeatherWithSky(int iWeather, int iPercentage)	{}
	virtual D3DCOLOR GetLightDiffuseColorWithSky(int iIndex)	{	return 0xffffffff;	}
	virtual D3DCOLOR GetLightAmbientColorWithSky(int iIndex)	{	return 0xffffffff;	}
	virtual D3DCOLOR GetFogColorWithSky()	{	return 0xffffffff;	}
	//virtual CN3Sun*	GetSunPointerWithSky()	{	return nullptr;	}

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

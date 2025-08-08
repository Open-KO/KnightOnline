// DungeonManager.h: interface for the CDungeonManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DUNGEONMANAGER_H__492B3E57_9A05_4F4D_B98C_63CB6ACC572E__INCLUDED_)
#define AFX_DUNGEONMANAGER_H__492B3E57_9A05_4F4D_B98C_63CB6ACC572E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "N3WorldBase.h"
#include "PvsMgr.h"

class CDungeonManager : public CN3WorldBase
{
	friend class CN3WorldManager;

	//..
	CPvsMgr	m_pvsmgr;

	// Function..
	void InitWorld(int iZoneID, const __Vector3& vPosPlayer);
	void Tick();

//////////////////////////////////////////////////////////////////////

	CN3Terrain* GetTerrainRef();
	CN3SkyMng* GetSkyRef();

	// Terrain..
	bool CheckCollisionCameraWithTerrain(__Vector3& vEyeResult, const __Vector3& vAt, float fNP);
	float GetHeightWithTerrain(float x, float z, bool bWarp = false);
	BOOL PickWideWithTerrain(int x, int y, __Vector3& vPick);
	bool CheckCollisionWithTerrain(__Vector3& vPos, __Vector3& vDir, float fVelocity, __Vector3* vCol);
	void GetNormalWithTerrain(float x, float z, __Vector3& vNormal);
	float GetWidthByMeterWithTerrain(); 
	bool IsInTerrainWithTerrain(float x, float z, __Vector3 vPosBefore = __Vector3());
	bool CheckInclineWithTerrain(const __Vector3& vPos, const __Vector3& vDir, float fIncline);

	// Shapes..
	bool CheckCollisionCameraWithShape(__Vector3& vEyeResult, const __Vector3& vAt, float fNP);
	float GetHeightNearstPosWithShape(const __Vector3& vPos, float fDist, __Vector3* pvNormal = NULL); 
	void RenderCollisionWithShape(const __Vector3& vPos);
	float GetHeightWithShape(float fX, float fZ, __Vector3* pvNormal = NULL); 
	CN3Shape* ShapeGetByIDWithShape(int iID);
	CN3Shape* PickWithShape(int iXScreen, int iYScreen, bool bMustHaveEvent, __Vector3* pvPick = NULL);
	bool CheckCollisionWithShape(	const __Vector3& vPos,				 // 충돌 위치
														const __Vector3& vDir,				   // 방향 벡터
														float fSpeedPerSec,					    // 초당 움직이는 속도
														__Vector3* pvCol = NULL,			 // 충돌 지점
														__Vector3* pvNormal = NULL,		 // 충돌한면의 법선벡터
														__Vector3* pVec = NULL);			// 충돌한 면 의 폴리곤 __Vector3[3]

	// Rendering..
	void RenderTerrain();
	void RenderShape();	

public:
	CDungeonManager();
	virtual ~CDungeonManager();

};

#endif // !defined(AFX_DUNGEONMANAGER_H__492B3E57_9A05_4F4D_B98C_63CB6ACC572E__INCLUDED_)

// PvsMgr.h: interface for the CPvsMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PVSMGR_H__7E562A54_E3B8_4484_A861_7ADD71D4411D__INCLUDED_)
#define AFX_PVSMGR_H__7E562A54_E3B8_4484_A861_7ADD71D4411D__INCLUDED_

#pragma once

#include "PortalVolume.h"
#include <string>

typedef std::list<CPortalVolume*>::iterator iter;

class CPvsMgr : public CN3BaseFileAccess
{
	friend class CDungeonManager;
	friend class CPortalVolume;

	const std::string m_IndoorFolder;
	const float m_fVolumeOffs; // Volume 체크 높이..

	std::list<CPortalVolume*> m_pPvsList;

	//.. Current Portal Volume.. ^^
	CPortalVolume* m_pCurVol;

public:
	static CN3Mng<class CN3Shape> s_MngShape;
	static CN3Mng<class CN3ShapeExtra> s_MngShapeExt;
	static std::list<ShapeInfo*> s_plShapeInfoList;
	static ShapeInfo* GetShapeInfoByManager(int iID);

private:
	void Tick(bool bWarp = false, const __Vector3* vPos = nullptr);
	void Render();

	void DeleteAllPvsObj();

	// String Cryptograph.. ^^
	static std::string ReadDecryptString(File& file);
	std::string GetIndoorFolderPath()
	{
		return m_IndoorFolder;
	}

	CPortalVolume* GetPortalVolPointerByID(int iID);

	////////////////////////////////////////////////////////////////
	bool CheckCollisionCameraWithTerrain(__Vector3& vEyeResult, const __Vector3& vAt, float fNP);
	bool CheckCollisionCameraWithShape(__Vector3& vEyeResult, const __Vector3& vAt, float fNP);
	float GetHeightWithTerrain(float x, float z);
	float GetHeightNearstPosWithShape(const __Vector3& vPos, __Vector3* pvNormal = nullptr);
	bool IsInTerrainWithTerrain(float x, float z);
	float GetHeightWithShape(float fX, float fZ, __Vector3* pvNormal = nullptr);
	BOOL PickWideWithTerrain(int x, int y, __Vector3& vPick);
	CN3Shape* PickWithShape(int iXScreen, int iYScreen, bool bMustHaveEvent, __Vector3* pvPick = nullptr);
	CN3Shape* ShapeGetByIDWithShape(int iID);
	bool CheckCollisionWithShape(const __Vector3& vPos, // 충돌 위치
		const __Vector3& vDir,                          // 방향 벡터
		float fSpeedPerSec,                             // 초당 움직이는 속도
		__Vector3* pvCol,                               // 충돌 지점
		__Vector3* pvNormal,                            // 충돌한면의 법선벡터
		__Vector3* pVec);                               // 충돌한 면 의 폴리곤 __Vector3[3]

public:
	CPvsMgr();
	~CPvsMgr() override;

	//..
	bool Load(File& file) override;
	bool LoadOldVersion(File& file, int iVersionFromData);
};

#endif // !defined(AFX_PVSMGR_H__7E562A54_E3B8_4484_A861_7ADD71D4411D__INCLUDED_)

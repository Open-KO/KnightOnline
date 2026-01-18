// N3TerrainManager.cpp: implementation of the CN3TerrainManager class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "N3TerrainManager.h"
#include "N3Terrain.h"
#include "N3ClientShapeMgr.h"
#include "BirdMng.h"
#include "GameProcedure.h"
#include "PlayerMySelf.h"

#include <N3Base/N3SkyMng.h>
#include <N3Base/LogWriter.h>

CN3TerrainManager::CN3TerrainManager()
{
	// N3Terrain..
	m_pTerrain = new CN3Terrain;

	// Shape..
	m_pShapes  = new CN3ClientShapeMgr();

	// Sky..
	m_pSky     = new CN3SkyMng();

	// Bird..
	m_pBirdMng = new CBirdMng();
}

CN3TerrainManager::~CN3TerrainManager()
{
	// N3Terrain..
	delete m_pTerrain;
	m_pTerrain = nullptr;

	// Shape..
	delete m_pShapes;
	m_pShapes = nullptr;

	// Sky..
	delete m_pSky;
	m_pSky = nullptr;

	// Bird..
	delete m_pBirdMng;
	m_pBirdMng = nullptr;
}

/////////////////////////////////////////////////////////////////////

void CN3TerrainManager::InitWorld(int iZoneID)
{
	__TABLE_ZONE* pZone = s_pTbl_Zones.Find(s_pPlayer->m_InfoExt.iZoneCur);
	if (pZone == nullptr)
	{
		CLogWriter::Write("Null Zone Data : {}", iZoneID);
		return;
	}

	m_pTerrain->LoadFromFile(pZone->szTerrainFN);

	m_pTerrain->LoadColorMap(pZone->szColorMapFN);      // 컬러맵 로드..
	m_pShapes->Release();

	m_pShapes->LoadFromFile(pZone->szObjectPostDataFN); // 오브젝트 데이터 로드..

	char szFName[_MAX_PATH];
	_splitpath(pZone->szTerrainFN.c_str(), nullptr, nullptr, szFName, nullptr);
	std::string szFName2 = fmt::format("{}_Bird", szFName);

	char szFullPathName[_MAX_PATH];
	_makepath(szFullPathName, nullptr, "misc\\bird", szFName2.c_str(), "lst");
	m_pBirdMng->LoadFromFile(szFullPathName);

	m_pSky->LoadFromFile(pZone->szSkySetting);                        // 하늘, 구름, 태양, 날씨 변화등 정보 및 텍스처 로딩..
	m_pSky->SunAndMoonDirectionFixByHour(pZone->iFixedSundDirection); // 해, 달 방향을 고정하든가 혹은 0 이면 고정하지 않는다.
}

void CN3TerrainManager::Tick()
{
	m_pTerrain->Tick();
	m_pShapes->Tick();
	m_pSky->Tick();
	m_pBirdMng->Tick();
}

CN3Terrain* CN3TerrainManager::GetTerrainRef()
{
	return m_pTerrain;
}

CN3SkyMng* CN3TerrainManager::GetSkyRef()
{
	return m_pSky;
}

void CN3TerrainManager::RenderTerrain()
{
	if (m_pTerrain)
		m_pTerrain->Render();
}

void CN3TerrainManager::RenderShape()
{
	if (m_pShapes)
		m_pShapes->Render();
}

void CN3TerrainManager::RenderSky()
{
	if (m_pSky)
		m_pSky->Render();
}

void CN3TerrainManager::RenderBirdMgr()
{
	if (m_pBirdMng)
		m_pBirdMng->Render();
}

void CN3TerrainManager::RenderSkyWeather()
{
	if (m_pSky)
		m_pSky->RenderWeather();
}

//////////////////////////////////////////////////////////////////////////////////

// Terrain..
bool CN3TerrainManager::CheckCollisionCameraWithTerrain(__Vector3& vEyeResult, const __Vector3& vAt, float fNP)
{
	if (m_pTerrain)
		return m_pTerrain->CheckCollisionCamera(vEyeResult, vAt, fNP);
	else
		return false;
}

float CN3TerrainManager::GetHeightWithTerrain(float x, float z)
{
	if (m_pTerrain)
		return m_pTerrain->GetHeight(x, z);
	else
		return -FLT_MAX;
}

BOOL CN3TerrainManager::PickWideWithTerrain(int x, int y, __Vector3& vPick)
{
	if (m_pTerrain)
		return m_pTerrain->PickWide(x, y, vPick);
	else
		return FALSE;
}

bool CN3TerrainManager::CheckCollisionWithTerrain(__Vector3& vPos, __Vector3& vDir, float fVelocity, __Vector3* vCol)
{
	if (m_pTerrain)
		return m_pTerrain->CheckCollision(vPos, vDir, fVelocity, vCol);
	else
		return false;
}

void CN3TerrainManager::GetNormalWithTerrain(float x, float z, __Vector3& vNormal)
{
	if (m_pTerrain)
		m_pTerrain->GetNormal(x, z, vNormal);
}

float CN3TerrainManager::GetWidthByMeterWithTerrain()
{
	if (m_pTerrain)
		return m_pTerrain->GetWidthByMeter();
	else
		return -FLT_MAX;
}

bool CN3TerrainManager::IsInTerrainWithTerrain(float x, float z)
{
	if (m_pTerrain)
		return m_pTerrain->IsInTerrain(x, z);
	else
		return false;
}

bool CN3TerrainManager::CheckInclineWithTerrain(const __Vector3& vPos, const __Vector3& vDir, float fIncline)
{
	if (m_pTerrain)
		return m_pTerrain->CheckIncline(vPos, vDir, fIncline);
	else
		return false;
}

// Shapes..
bool CN3TerrainManager::CheckCollisionCameraWithShape(__Vector3& vEyeResult, const __Vector3& vAt, float fNP)
{
	if (m_pShapes)
		return m_pShapes->CheckCollisionCamera(vEyeResult, vAt, fNP);
	else
		return false;
}

float CN3TerrainManager::GetHeightNearstPosWithShape(const __Vector3& vPos, __Vector3* pvNormal)
{
	if (m_pShapes)
		return m_pShapes->GetHeightNearstPos(vPos, pvNormal);
	else
		return -FLT_MAX;
}

void CN3TerrainManager::RenderCollisionWithShape(const __Vector3& vPos)
{
	if (m_pShapes)
		m_pShapes->RenderCollision(vPos);
}

float CN3TerrainManager::GetHeightWithShape(float fX, float fZ, __Vector3* pvNormal)
{
	if (m_pShapes)
		return m_pShapes->GetHeight(fX, fZ, pvNormal);
	else
		return -FLT_MAX;
}

CN3Shape* CN3TerrainManager::ShapeGetByIDWithShape(int iID)
{
	if (m_pShapes)
		return m_pShapes->ShapeGetByID(iID);
	else
		return nullptr;
}

CN3Shape* CN3TerrainManager::PickWithShape(int iXScreen, int iYScreen, bool bMustHaveEvent, __Vector3* pvPick)
{
	if (m_pShapes)
		return m_pShapes->Pick(iXScreen, iYScreen, bMustHaveEvent, pvPick);
	else
		return nullptr;
}

bool CN3TerrainManager::CheckCollisionWithShape(const __Vector3& vPos, // 충돌 위치
	const __Vector3& vDir,                                             // 방향 벡터
	float fSpeedPerSec,                                                // 초당 움직이는 속도
	__Vector3* pvCol,                                                  // 충돌 지점
	__Vector3* pvNormal,                                               // 충돌한면의 법선벡터
	__Vector3* pVec)                                                   // 충돌한 면 의 폴리곤 __Vector3[3]
{
	if (m_pShapes)
		return m_pShapes->CheckCollision(vPos, vDir, fSpeedPerSec, pvCol, pvNormal, pVec);
	else
		return false;
}

// Sky..
D3DCOLOR CN3TerrainManager::GetSkyColorWithSky()
{
	if (m_pSky)
		return m_pSky->GetSkyColor();
	else
		return 0xffffffff;
}

float CN3TerrainManager::GetSunAngleByRadinWithSky()
{
	if (m_pSky)
		return m_pSky->GetSunAngleByRadin();
	else
		return -FLT_MAX;
}

void CN3TerrainManager::RenderWeatherWithSky()
{
	if (m_pSky)
		m_pSky->RenderWeather();
}

void CN3TerrainManager::SetGameTimeWithSky(int iYear, int iMonth, int iDay, int iHour, int iMin)
{
	if (m_pSky)
		m_pSky->SetGameTime(iYear, iMonth, iDay, iHour, iMin);
}

void CN3TerrainManager::SetWeatherWithSky(int iWeather, int iPercentage)
{
	if (m_pSky)
		m_pSky->SetWeather((CN3SkyMng::eSKY_WEATHER) iWeather, iPercentage);
}

D3DCOLOR CN3TerrainManager::GetLightDiffuseColorWithSky(int iIndex)
{
	if (m_pSky)
		return m_pSky->GetLightDiffuseColor(iIndex);
	else
		return 0xffffffff;
}

D3DCOLOR CN3TerrainManager::GetLightAmbientColorWithSky(int iIndex)
{
	if (m_pSky)
		return m_pSky->GetLightAmbientColor(iIndex);
	else
		return 0xffffffff;
}

D3DCOLOR CN3TerrainManager::GetFogColorWithSky()
{
	if (m_pSky)
		return m_pSky->GetFogColor();
	else
		return 0xffffffff;
}

/*
CN3Sun*	CN3TerrainManager::GetSunPointerWithSky()
{
	if (m_pSky)
		return m_pSky->GetSunPointer();
	else
		return nullptr;
}
*/

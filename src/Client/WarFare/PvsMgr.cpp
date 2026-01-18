// PvsMgr.cpp: implementation of the CPvsMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "PvsMgr.h"
#include "GameBase.h"
#include "PlayerMySelf.h"
#include "GameProcedure.h"
#include "GameEng.h"

#include <N3Base/N3Camera.h>
#include <N3Base/N3ShapeMgr.h>
#include <N3Base/N3ShapeExtra.h>

constexpr int ciVersion = 1;

CN3Mng<CN3Shape> CPvsMgr::s_MngShape;
CN3Mng<CN3ShapeExtra> CPvsMgr::s_MngShapeExt;
std::list<ShapeInfo*> CPvsMgr::s_plShapeInfoList;

CPvsMgr::CPvsMgr() : m_IndoorFolder("N3Indoor\\"), m_fVolumeOffs(0.6f) //..
{
	s_plShapeInfoList.clear();
	m_pCurVol = nullptr;
}

CPvsMgr::~CPvsMgr()
{
	DeleteAllPvsObj();
}

void CPvsMgr::DeleteAllPvsObj()
{
	iter it = m_pPvsList.begin();

	while (it != m_pPvsList.end())
	{
		CPortalVolume* pvs = *it;
		if (pvs)
			delete pvs;
		it++;
	}
	m_pPvsList.clear();

	for (ShapeInfo* pSI : s_plShapeInfoList)
		delete pSI;
	s_plShapeInfoList.clear();
	s_MngShape.Release();
	s_MngShapeExt.Release();
}

ShapeInfo* CPvsMgr::GetShapeInfoByManager(int iID)
{
	for (ShapeInfo* pSI : s_plShapeInfoList)
	{
		if (pSI->m_iID == iID)
			return pSI;
	}

	return nullptr;
}

void CPvsMgr::Tick(bool bWarp, const __Vector3* vPos)
{
	CPortalVolume* pVol = nullptr;
	__Vector3 vec {};
	if (bWarp)
	{
		if (vPos != nullptr)
			vec = *vPos;
		else
			vec = {};
	}
	else
	{
		vec = CGameBase::s_pPlayer->Position();
	}

	vec.y     += m_fVolumeOffs;
	m_pCurVol  = nullptr;

	iter it    = m_pPvsList.begin();
	while (it != m_pPvsList.end())
	{
		pVol = *it++;
		if (pVol->IsInVolumn(vec))
		{
			m_pCurVol = pVol;
			break;
		}
	}

	it = m_pPvsList.begin();
	while (it != m_pPvsList.end())
	{
		pVol                = *it++;
		pVol->m_eRenderType = TYPE_UNKNOWN;
	}

	if (!m_pCurVol)
		return;

	VisPortalPriority vPP;
	vppiter vppit = m_pCurVol->m_pVisiblePvsList.begin();
	while (vppit != m_pCurVol->m_pVisiblePvsList.end())
	{
		vPP                       = *vppit++;
		vPP.m_pVol->m_eRenderType = TYPE_TRUE;
	}
}

void CPvsMgr::Render()
{
	CPortalVolume* pVol = nullptr;
	iter it             = m_pPvsList.begin();

	while (it != m_pPvsList.end())
	{
		pVol = *it++;
		if (pVol->m_eRenderType == TYPE_TRUE)
		{
			pVol->Render();
		}
	}
}

bool CPvsMgr::LoadOldVersion(File& /*file*/, int /*iVersionFromData*/)
{
	//..

	return true;
}

bool CPvsMgr::Load(File& file)
{
	int iT = 0;
	file.Read(&iT, sizeof(int));
	if (iT != ciVersion)
		return LoadOldVersion(file, iT);

	// N3Scene 화일.. 안쓴다.. -.-;
	std::string strSrc = ReadDecryptString(file), strDest;

	// 전체 이동값.. 안슨다.. -.-;
	file.Read(&iT, sizeof(int));
	file.Read(&iT, sizeof(int));
	file.Read(&iT, sizeof(int));

	char szDrive[_MAX_DRIVE] {}, szDir[_MAX_DIR] {}, szFName[_MAX_FNAME] {}, szExt[_MAX_EXT] {};
	int iCount = 0;
	file.Read(&iCount, sizeof(int));

	for (int i = 0; i < iCount; i++)
	{
		ShapeInfo* pSI = new ShapeInfo;
		file.Read(&pSI->m_iID, sizeof(int));

		// 문자열 길이..
		strSrc = ReadDecryptString(file);
		_splitpath(strSrc.c_str(), szDrive, szDir, szFName, szExt);
		strDest              = szFName;
		strDest             += szExt;
		pSI->m_strShapeFile  = m_IndoorFolder + strDest;
		pSI->m_pShape        = s_MngShape.Get(m_IndoorFolder + strDest);
		__ASSERT(pSI->m_pShape, "Shape Not Found");

		file.Read(&pSI->m_iBelong, sizeof(int));
		file.Read(&pSI->m_iEventID, sizeof(int));
		file.Read(&pSI->m_iEventType, sizeof(int));
		file.Read(&pSI->m_iNPC_ID, sizeof(int));
		file.Read(&pSI->m_iNPC_Status, sizeof(int));
		pSI->Load(file);
		s_plShapeInfoList.push_back(pSI);
	}

	// Total Count..
	file.Read(&iCount, sizeof(int));

	CPortalVolume *pVol = nullptr, *pVolTo = nullptr;
	for (int i = 0; i < iCount; i++)
	{
		int iID = 0;
		file.Read(&iID, sizeof(int));

		pVol             = new CPortalVolume;
		pVol->m_pManager = this;
		pVol->m_iID      = iID;
		pVol->Load(file);
		m_pPvsList.push_back(pVol);
	}

	iter it = m_pPvsList.begin();
	idapiter idapit;
	IDAndPriority IDAP;
	VisPortalPriority vPP;

	while (it != m_pPvsList.end())
	{
		pVol   = *it++;

		idapit = pVol->m_piVisibleIDList.begin();
		while (idapit != pVol->m_piVisibleIDList.end())
		{
			IDAP            = *idapit++;
			pVolTo          = GetPortalVolPointerByID(IDAP.m_iID);
			vPP.m_pVol      = pVolTo;
			vPP.m_iPriority = IDAP.m_iPriority;
			pVol->m_pVisiblePvsList.push_back(vPP);
		}

		pVol->m_piVisibleIDList.clear();
	}

	return true;
}

CPortalVolume* CPvsMgr::GetPortalVolPointerByID(int iID)
{
	CPortalVolume* pVol = nullptr;

	iter it             = m_pPvsList.begin();

	while (it != m_pPvsList.end())
	{
		// 자신의 데이터 저장..
		pVol = *it++;
		if (pVol->m_iID == iID)
			return pVol;
	}

	return nullptr;
}

constexpr uint16_t CRY_KEY = 0x0816;

std::string CPvsMgr::ReadDecryptString(File& file)
{
	int iCount = 0;
	file.Read(&iCount, sizeof(int));

	std::string strDest;
	if (iCount > 0)
	{
		strDest.assign(iCount, '\0');
		file.Read(&strDest[0], iCount);

		for (int i = 0; i < iCount; i++)
			strDest[i] = static_cast<char>(static_cast<uint8_t>(strDest[i]) ^ CRY_KEY);
	}

	return strDest;
}

//////////////////////////////////////////////////////////////////////////////////

bool CPvsMgr::CheckCollisionCameraWithTerrain(__Vector3& vEyeResult, const __Vector3& vAt, float fNP)
{
	fNP             = (vAt - vEyeResult).Magnitude();
	bool bCollision = false;
	float fNPMin = fNP, fTemp = 0.0f;

	CPortalVolume* pVol = nullptr;
	iter it             = m_pPvsList.begin();

	while (it != m_pPvsList.end())
	{
		pVol = *it++;
		if ((pVol->m_eRenderType == TYPE_TRUE) && pVol->CheckCollisionCameraWithTerrain(vEyeResult, vAt, fNP))
		{
			bCollision = true;
			fTemp      = (vEyeResult - vAt).Magnitude();
			if (fTemp < fNPMin)
				fNPMin = fTemp;
		}
	}

	if (bCollision && (fNPMin < fNP))
	{
		__Vector3 vT;
		vT.Zero();
		vT = vEyeResult - vAt;
		vT.Normalize();
		vT         *= fNPMin;
		vEyeResult  = vT + vAt;
	}

	return bCollision;
}

bool CPvsMgr::CheckCollisionCameraWithShape(__Vector3& vEyeResult, const __Vector3& vAt, float fNP)
{
	fNP             = (vAt - vEyeResult).Magnitude();
	bool bCollision = false;
	float fNPMin = fNP, fTemp = 0.0f;

	CPortalVolume* pVol = nullptr;
	iter it             = m_pPvsList.begin();

	while (it != m_pPvsList.end())
	{
		pVol = *it++;
		if ((pVol->m_eRenderType == TYPE_TRUE) && pVol->CheckCollisionCameraWithShape(vEyeResult, vAt, fNP))
		{
			bCollision = true;
			fTemp      = (vEyeResult - vAt).Magnitude();
			if (fTemp < fNPMin)
				fNPMin = fTemp;
		}
	}

	if (bCollision && (fNPMin < fNP))
	{
		__Vector3 vT;
		vT.Zero();
		vT = vEyeResult - vAt;
		vT.Normalize();
		vT         *= fNPMin;
		vEyeResult  = vT + vAt;
	}

	return bCollision;
}

float CPvsMgr::GetHeightWithTerrain(float x, float z)
{
	if (!m_pCurVol)
		Tick();

	float fHeight = FLT_MIN;
	if (!m_pCurVol)
		return fHeight;

	m_pCurVol->GetHeightWithTerrain(x, z, fHeight);
	return fHeight;
}

float CPvsMgr::GetHeightNearstPosWithShape(const __Vector3& vPos, __Vector3* pvNormal)
{
	float fHeightMax = FLT_MIN, fHeight = 0.0f;

	CPortalVolume* pVol = nullptr;
	iter it             = m_pPvsList.begin();

	while (it != m_pPvsList.end())
	{
		pVol = *it++;
		if (pVol->m_eRenderType == TYPE_TRUE)
		{
			fHeight = pVol->GetHeightNearstPosWithShape(vPos, pvNormal);
			if (fHeightMax < fHeight)
				fHeightMax = fHeight;
		}
	}

	return fHeightMax;
}

bool CPvsMgr::IsInTerrainWithTerrain(float x, float z)
{
	if (!m_pCurVol)
		Tick();

	float fHeight = FLT_MIN;
	if (!m_pCurVol)
		return false;

	return m_pCurVol->GetHeightWithTerrain(x, z, fHeight);
}

float CPvsMgr::GetHeightWithShape(float fX, float fZ, __Vector3* /*pvNormal*/)
{
	float fHeightMax = FLT_MIN, fHeight = 0.0f;

	CPortalVolume *pVol = nullptr, *pVolNe = nullptr;
	iter it = m_pPvsList.begin();

	while (it != m_pPvsList.end())
	{
		pVol    = *it++;
		fHeight = pVol->GetHeightNearstPosWithShape(__Vector3(fX, FLT_MIN, fZ));
		if (fHeightMax < fHeight)
		{
			fHeightMax = fHeight;
			pVolNe     = pVol;
		}
	}

	__Vector3 vPos;
	vPos.Set(fX, fHeightMax, fZ);
	if (pVolNe && pVolNe->m_eRenderType != TYPE_TRUE)
		Tick(true, &vPos);

	return fHeightMax;
}

BOOL CPvsMgr::PickWideWithTerrain(int x, int y, __Vector3& vPick)
{
	BOOL bColl       = FALSE;
	__Vector3 vCamPo = CGameProcedure::s_pEng->CameraGetActive()->EyePos(), vPT;
	float fDistMax = FLT_MAX, fDT = 0.0f;

	CPortalVolume* pVol = nullptr;
	iter it             = m_pPvsList.begin();

	while (it != m_pPvsList.end())
	{
		pVol = *it++;
		if ((pVol->m_eRenderType == TYPE_TRUE) && (pVol->PickWideWithTerrain(x, y, vPT)))
		{
			fDT = (vPT - vCamPo).Magnitude();
			if (fDT <= fDistMax)
			{
				fDistMax = fDT;
				vPick    = vPT;
				bColl    = TRUE;
			}
		}
	}

	return bColl;
}

CN3Shape* CPvsMgr::PickWithShape(int iXScreen, int iYScreen, bool bMustHaveEvent, __Vector3* pvPick)
{
	if (!m_pCurVol)
		Tick();

	if (!m_pCurVol)
		return nullptr;

	return m_pCurVol->PickWithShape(iXScreen, iYScreen, bMustHaveEvent, pvPick);
}

bool CPvsMgr::CheckCollisionWithShape(const __Vector3& vPos, // 충돌 위치
	const __Vector3& vDir,                                   // 방향 벡터
	float fSpeedPerSec,                                      // 초당 움직이는 속도
	__Vector3* pvCol,                                        // 충돌 지점
	__Vector3* pvNormal,                                     // 충돌한면의 법선벡터
	__Vector3* pVec)                                         // 충돌한 면 의 폴리곤 __Vector3[3]
{
	if (!m_pCurVol)
		Tick();

	if (!m_pCurVol)
		return false;

	return m_pCurVol->CheckCollisionWithShape(vPos, vDir, fSpeedPerSec, pvCol, pvNormal, pVec);
}

CN3Shape* CPvsMgr::ShapeGetByIDWithShape(int iID)
{
	if (!m_pCurVol)
		Tick();

	if (!m_pCurVol)
		return nullptr;

	return m_pCurVol->ShapeGetByIDWithShape(iID);
}

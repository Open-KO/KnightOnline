// PortalVolume.cpp: implementation of the CPortalVolume class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "PortalVolume.h"
#include "GameBase.h"
#include "PlayerMySelf.h"

#include "PvsMgr.h"

#include <N3Base/N3ShapeMgr.h>
#include <N3Base/N3ShapeExtra.h>

CPortalVolume::CPortalVolume() : m_fOffs(0.001f), m_fHeightOffs(0.01f), m_fVolOffs(0.001f), m_fPickIncline(0.6f), m_fCameraOffs(0.4f)
{
	m_pvVertex[0].Set(-fBaseVolumnSize, -fBaseVolumnSize, fBaseVolumnSize);
	m_pvVertex[1].Set(fBaseVolumnSize, -fBaseVolumnSize, fBaseVolumnSize);
	m_pvVertex[2].Set(fBaseVolumnSize, -fBaseVolumnSize, -fBaseVolumnSize);
	m_pvVertex[3].Set(-fBaseVolumnSize, -fBaseVolumnSize, -fBaseVolumnSize);
	m_pvVertex[4].Set(-fBaseVolumnSize, fBaseVolumnSize, fBaseVolumnSize);
	m_pvVertex[5].Set(fBaseVolumnSize, fBaseVolumnSize, fBaseVolumnSize);
	m_pvVertex[6].Set(fBaseVolumnSize, fBaseVolumnSize, -fBaseVolumnSize);
	m_pvVertex[7].Set(-fBaseVolumnSize, fBaseVolumnSize, -fBaseVolumnSize);

	const uint16_t index[] = { //
		// Bottom
		0, 1, 3, 2, 3, 1,

		// Front
		7, 3, 6, 2, 6, 3,

		// Left
		4, 0, 7, 3, 7, 0,

		// Right
		6, 2, 5, 1, 5, 2,

		// Back
		5, 1, 4, 0, 4, 1,

		// Top
		4, 7, 5, 6, 5, 7
	};

	memcpy(m_pIndex, index, sizeof(m_pIndex));

	m_iID         = -1;
	m_pManager    = nullptr;
	m_iPriority   = 100;

	m_eRenderType = TYPE_UNKNOWN;
}

CPortalVolume::~CPortalVolume()
{
	DeleteAllPvsObj();
}

void CPortalVolume::DeleteAllPvsObj()
{
	for (ShapeInfo* pSI : m_plShapeInfoList)
		delete pSI;
	m_plShapeInfoList.clear();

	for (ShapePart* pSP : m_lpShapePartList)
	{
		pSP->Clear();
		delete pSP;
	}
	m_lpShapePartList.clear();

	for (__ColIndex* pCI : m_lpShapeColPartList)
		delete pCI;
	m_lpShapeColPartList.clear();

	m_piVisibleIDList.clear();
	m_pVisiblePvsList.clear();
}

bool CPortalVolume::IsInVolumn(const __Vector3& vec) const
{
	__Vector3 vec2[8] {};
	for (int i = 0; i < 8; i++)
	{
		vec2[i]  = m_pvVertex[i];
		vec2[i] *= m_Matrix;
	}

	if (vec.x >= vec2[0].x && vec.x <= vec2[1].x && vec.y >= vec2[0].y && vec.y <= vec2[4].y && vec.z >= vec2[2].z && vec.z <= vec2[0].z)
		return true;

	return false;
}

void CPortalVolume::Render()
{
	DWORD dwAlpha = 0, dwFog = 0, dwLight = 0, dwPointSize = 0;
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_FOGENABLE, &dwFog);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlpha);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_LIGHTING, &dwLight);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_POINTSIZE, &dwPointSize);

	if (dwFog)
		CN3Base::s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, FALSE);
	if (dwAlpha)
		CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	if (dwLight)
		CN3Base::s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);

	static __Material smtl;
	static bool bInit = false;
	if (false == bInit)
	{
		smtl.Init();
		bInit = true;
	}

	__Matrix44 mtxWorld;
	mtxWorld.Identity();

	CN3Base::s_lpD3DDev->SetTransform(D3DTS_WORLD, mtxWorld.toD3D());
	CN3Base::s_lpD3DDev->SetTexture(0, nullptr);

	// Shape..
	RenderShape();

#ifdef _DEBUG
	RenderCollision();
#endif

	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_POINTSIZE, dwPointSize);
	if (dwFog)
		CN3Base::s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, dwFog);
	if (dwAlpha)
		CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlpha);
	if (dwLight)
		CN3Base::s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, dwLight);
}

void CPortalVolume::RenderShape()
{
	for (ShapeInfo* pSI : m_plShapeInfoList)
	{
		pSI->m_pShape->PosSet(pSI->Pos());
		pSI->m_pShape->RotSet(pSI->Rot());
		pSI->m_pShape->ScaleSet(pSI->Scale());
		pSI->m_pShape->Tick(-1000);
		pSI->m_pShape->m_bDontRender = false;
		pSI->m_pShape->Render();

		// 로딩할때 미리 계산해 놓은 월드 행렬 적용..
		__Matrix44 mtxBackup;
		CN3Base::s_lpD3DDev->GetTransform(D3DTS_WORLD, mtxBackup.toD3D());
		CN3Base::s_lpD3DDev->SetTransform(D3DTS_WORLD, pSI->m_pShape->m_Matrix.toD3D());

#ifdef _DEBUG
		if (pSI->m_pShape->CollisionMesh())
			pSI->m_pShape->CollisionMesh()->Render(0xffffffff);
#endif

		CN3Base::s_lpD3DDev->SetTransform(D3DTS_WORLD, mtxBackup.toD3D());
	}

	for (ShapePart* pSP : m_lpShapePartList)
	{
		for (__VPI& vpi : pSP->m_viIndex)
		{
			size_t vecSize     = vpi.m_ivVector.size();

			uint16_t* pIndices = new uint16_t[vecSize] {};
			for (size_t k = 0; k < vecSize; k++)
				pIndices[k] = vpi.m_ivVector[k];

			ShapeInfo* pSI = CPvsMgr::GetShapeInfoByManager(pSP->m_iID);
			pSI->m_pShape->PosSet(pSI->Pos());
			pSI->m_pShape->RotSet(pSI->Rot());
			pSI->m_pShape->ScaleSet(pSI->Scale());
			pSI->m_pShape->Part(vpi.m_iPartIndex)->m_bOutOfCameraRange = FALSE;
			pSI->m_pShape->PartialRender(vpi.m_iPartIndex, static_cast<int>(vecSize), pIndices);
			delete[] pIndices;
		}
	}
}

void CPortalVolume::RenderCollision()
{
	__ColIndex* pCI = nullptr;
	ShapeInfo* pSI  = nullptr;

	ciiter ciit     = m_lpShapeColPartList.begin();
	while (ciit != m_lpShapeColPartList.end())
	{
		pCI = *ciit++;

		// 행렬 계산..
		__Matrix44 mtxWorld;
		mtxWorld.Identity();

		CN3Base::s_lpD3DDev->SetTransform(D3DTS_WORLD, mtxWorld.toD3D());

		size_t vecSize     = pCI->m_ivVector.size();
		uint32_t* pIndices = new uint32_t[vecSize];
		memset(pIndices, 0, sizeof(uint32_t) * vecSize);

		for (size_t k = 0; k < vecSize; k++)
			pIndices[k] = pCI->m_ivVector[k];

		__Matrix44 mtxBackup;
		CN3Base::s_lpD3DDev->GetTransform(D3DTS_WORLD, mtxBackup.toD3D());
		pSI = CPvsMgr::GetShapeInfoByManager(pCI->m_iID);
		CN3Base::s_lpD3DDev->SetTransform(D3DTS_WORLD, pSI->m_Matrix.toD3D());
		pSI->m_pShape->PartialColRender(static_cast<int>(vecSize), reinterpret_cast<int*>(pIndices));
		delete[] pIndices;

		CN3Base::s_lpD3DDev->SetTransform(D3DTS_WORLD, mtxBackup.toD3D());
	}
}

//////////////////////////////////////////////////////////////////////

bool CPortalVolume::Load(File& file)
{
	CN3Transform::Load(file);

	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];

	// 자신의 데이터 로드..
	std::string strSrc, strDest;

	// 링크된 갯수를 로드..	일단 읽구 버린다..
	int iLinkedCount = 0, iTID = 0, iEWT = 0;
	file.Read(&iLinkedCount, sizeof(int));
	for (int i = 0; i < iLinkedCount; i++)
	{
		file.Read(&iTID, sizeof(int));
		file.Read(&iEWT, sizeof(int));
	}

	// 링크된 Shape 갯수 로드..
	int iCount = 0;
	file.Read(&iCount, sizeof(int));
	for (int i = 0; i < iCount; i++)
	{
		ShapeInfo* pSI = new ShapeInfo;
		file.Read(&pSI->m_iID, sizeof(int));

		// 문자열 길이..
		strSrc = CPvsMgr::ReadDecryptString(file);
		_splitpath(strSrc.c_str(), szDrive, szDir, szFName, szExt);
		strDest              = szFName;
		strDest             += szExt;
		pSI->m_strShapeFile  = m_pManager->GetIndoorFolderPath() + strDest;
		file.Read(&pSI->m_iBelong, sizeof(int));
		file.Read(&pSI->m_iEventID, sizeof(int));
		file.Read(&pSI->m_iEventType, sizeof(int));
		file.Read(&pSI->m_iNPC_ID, sizeof(int));
		file.Read(&pSI->m_iNPC_Status, sizeof(int));
		if (pSI->m_iEventID || pSI->m_iEventType || pSI->m_iNPC_ID || pSI->m_iNPC_Status) // 이벤트가 있으면
			pSI->m_pShape = CPvsMgr::s_MngShapeExt.Get(m_pManager->GetIndoorFolderPath() + strDest);
		else
			pSI->m_pShape = CPvsMgr::s_MngShape.Get(m_pManager->GetIndoorFolderPath() + strDest);

		__ASSERT(pSI->m_pShape, "Shape Not Found");
		pSI->Load(file);
		m_plShapeInfoList.push_back(pSI);
	}

	// Visible..
	IDAndPriority IDAP;
	file.Read(&iCount, sizeof(int));

	for (int i = 0; i < iCount; i++)
	{
		file.Read(&IDAP.m_iID, sizeof(int));
		file.Read(&IDAP.m_iPriority, sizeof(int));
		__ASSERT(IDAP.m_iPriority != -1, "잘못된 파일");
		m_piVisibleIDList.push_back(IDAP);
	}

	file.Read(&iCount, sizeof(int));

	int iSize_2 = 0, iSize_3 = 0;
	for (int i = 0; i < iCount; i++)
	{
		ShapePart* pSP = new ShapePart;
		file.Read(&pSP->m_iID, sizeof(int));

		file.Read(&iSize_2, sizeof(int));
		for (int j = 0; j < iSize_2; j++)
		{
			__VPI vpi;
			file.Read(&vpi.m_iPartIndex, sizeof(int));

			file.Read(&iSize_3, sizeof(int));
			for (int k = 0; k < iSize_3; k++)
			{
				int iV = 0;
				file.Read(&iV, sizeof(int));
				vpi.m_ivVector.push_back(iV);
			}

			pSP->m_viIndex.push_back(vpi);
		}

		m_lpShapePartList.push_back(pSP);
	}

	file.Read(&iCount, sizeof(int));

	for (int i = 0; i < iCount; i++)
	{
		__ColIndex* pCI = new __ColIndex;
		file.Read(&pCI->m_iID, sizeof(int));

		file.Read(&iSize_2, sizeof(int));
		for (int j = 0; j < iSize_2; j++)
		{
			int iV = 0;
			file.Read(&iV, sizeof(int));
			pCI->m_ivVector.push_back(iV);
		}

		m_lpShapeColPartList.push_back(pCI);
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////

bool CPortalVolume::CheckCollisionCameraWithTerrain(__Vector3& vEyeResult, const __Vector3& vAt, float fNP)
{
	bool bCollision = false;
	bool bColl      = false;
	__Vector3 vDir  = vEyeResult - vAt;
	vDir.Normalize();
	__ColIndex* pCI = nullptr;

	ciiter ciit     = m_lpShapeColPartList.begin();
	while (ciit != m_lpShapeColPartList.end())
	{
		pCI            = *ciit++;

		size_t vecSize = pCI->m_ivVector.size();
		__Vector3 vA, vB, vC, vPick;
		float t = 0.0f, u = 0.0f, v = 0.0f;

		ShapeInfo* pSI = CPvsMgr::GetShapeInfoByManager(pCI->m_iID);
		pSI->m_pShape->PosSet(pSI->Pos());
		pSI->m_pShape->RotSet(pSI->Rot());
		pSI->m_pShape->ScaleSet(pSI->Scale());
		pSI->m_pShape->Tick(-1000);
		CN3Shape* pShape = pSI->m_pShape;

		for (size_t k = 0; k < vecSize / 3; k++)
		{
			pShape->PartialGetCollision(pCI->m_ivVector[k * 3], vA);
			vA   *= pShape->m_Matrix;
			vA.y += m_fCameraOffs;
			pShape->PartialGetCollision(pCI->m_ivVector[k * 3 + 1], vB);
			vB   *= pShape->m_Matrix;
			vB.y += m_fCameraOffs;
			pShape->PartialGetCollision(pCI->m_ivVector[k * 3 + 2], vC);
			vC               *= pShape->m_Matrix;
			vC.y             += m_fCameraOffs;

			__Vector3 vEdge1  = vB - vA;
			__Vector3 vEdge2  = vC - vA;
			__Vector3 pVec;
			pVec.Cross(vEdge1, vEdge2);
			pVec.Normalize();
			pVec  *= m_fOffs;
			vA    += pVec;
			vB    += pVec;
			vC    += pVec;

			bColl  = ::_IntersectTriangle(vAt, vDir, vA, vB, vC, t, u, v, &vPick);
			if (bColl)
			{
				if ((vPick - vAt).Magnitude() < fNP)
				{
					vEyeResult = vPick;
					bCollision = true;
				}
			}
		}
	}

	return bCollision;
}

bool CPortalVolume::CheckCollisionCameraWithShape(__Vector3& vEyeResult, const __Vector3& vAt, float fNP)
{
	bool bCollision = false;
	bool bColl      = false;
	__Vector3 vDir  = vEyeResult - vAt;
	vDir.Normalize();
	ShapeInfo* pSI = nullptr;

	siiter siit    = m_plShapeInfoList.begin();
	while (siit != m_plShapeInfoList.end())
	{
		pSI       = *siit++;

		int iSize = pSI->m_pShape->GetColIndexbufferCount();
		__Vector3 vA, vB, vC, vPick;
		float t = 0.0f, u = 0.0f, v = 0.0f;

		for (int k = 0; k < iSize / 3; k++)
		{
			vA                = pSI->m_pShape->GetColVertexByIndex(pSI->m_pShape->GetColIndexByiOrder(k * 3));
			vA               *= pSI->m_Matrix;
			vB                = pSI->m_pShape->GetColVertexByIndex(pSI->m_pShape->GetColIndexByiOrder(k * 3 + 1));
			vB               *= pSI->m_Matrix;
			vC                = pSI->m_pShape->GetColVertexByIndex(pSI->m_pShape->GetColIndexByiOrder(k * 3 + 2));
			vC               *= pSI->m_Matrix;

			__Vector3 vEdge1  = vB - vA;
			__Vector3 vEdge2  = vC - vA;
			__Vector3 pVec;
			pVec.Cross(vEdge1, vEdge2);
			pVec.Normalize();
			pVec  *= m_fOffs;
			vA    += pVec;
			vB    += pVec;
			vC    += pVec;

			bColl  = ::_IntersectTriangle(vAt, vDir, vA, vB, vC, t, u, v, &vPick);
			if (bColl)
			{
				if ((vPick - vAt).Magnitude() < fNP)
				{
					vEyeResult = vPick;
					bCollision = true;
				}
			}
		}
	}

	return bCollision;
}

bool CPortalVolume::GetHeightWithTerrain(float fx, float fz, float& fy)
{
	CPortalVolume* pVol = nullptr;

	VisPortalPriority vPP;
	vppiter vppit = m_pVisiblePvsList.begin();
	while (vppit != m_pVisiblePvsList.end())
	{
		vPP = *vppit++;
		if (vPP.m_iPriority > 1)
			continue;

		pVol            = vPP.m_pVol;

		__ColIndex* pCI = nullptr;
		ciiter ciit     = pVol->m_lpShapeColPartList.begin();
		while (ciit != pVol->m_lpShapeColPartList.end())
		{
			pCI            = *ciit++;

			size_t vecSize = pCI->m_ivVector.size();
			__Vector3 vA, vB, vC;
			ShapeInfo* pSI = CPvsMgr::GetShapeInfoByManager(pCI->m_iID);
			pSI->m_pShape->PosSet(pSI->Pos());
			pSI->m_pShape->RotSet(pSI->Rot());
			pSI->m_pShape->ScaleSet(pSI->Scale());
			pSI->m_pShape->Tick(-1000);
			CN3Shape* pShape = pSI->m_pShape;

			for (size_t k = 0; k < vecSize / 3; k++)
			{
				pShape->PartialGetCollision(pCI->m_ivVector[k * 3], vA);
				vA *= pShape->m_Matrix;
				pShape->PartialGetCollision(pCI->m_ivVector[k * 3 + 1], vB);
				vB *= pShape->m_Matrix;
				pShape->PartialGetCollision(pCI->m_ivVector[k * 3 + 2], vC);
				vC      *= pShape->m_Matrix;

				float a = 0.0f, b = 0.0f, c = 0.0f;
				a = ((vB.x - vA.x) * (fz - vA.z)) - ((vB.z - vA.z) * (fx - vA.x));
				b = ((vC.x - vB.x) * (fz - vB.z)) - ((vC.z - vB.z) * (fx - vB.x));
				c = ((vA.x - vC.x) * (fz - vC.z)) - ((vA.z - vC.z) * (fx - vC.x));

				if ((a <= 0 && b <= 0 && c <= 0) || (a >= 0 && b >= 0 && c >= 0))
				{
					__Vector3 vAB, vAC, vAxis;
					vAB.Set(vB.x - vA.x, vB.y - vA.y, vB.z - vA.z);
					vAC.Set(vC.x - vA.x, vC.y - vA.y, vC.z - vA.z);

					vAxis.Cross(vAB, vAC);
					if (vAxis.y <= 0.0f)
						continue;

					fy  = (vAxis.x * (vA.x - fx) + vAxis.z * (vA.z - fz)) / vAxis.y + vA.y;
					fy += m_fHeightOffs;

					return true;
				}
			}
		}
	}

	return false;
}

float CPortalVolume::GetHeightNearstPosWithShape(const __Vector3& vPos, __Vector3* pvNormal)
{
	float fHeight  = FLT_MIN;
	ShapeInfo* pSI = nullptr;

	siiter siit    = m_plShapeInfoList.begin();
	while (siit != m_plShapeInfoList.end())
	{
		pSI       = *siit++;

		int iSize = pSI->m_pShape->GetColIndexbufferCount();
		__Vector3 vA, vB, vC;
		for (int k = 0; k < iSize / 3; k++)
		{
			vA       = pSI->m_pShape->GetColVertexByIndex(pSI->m_pShape->GetColIndexByiOrder(k * 3));
			vA      *= pSI->m_Matrix;
			vB       = pSI->m_pShape->GetColVertexByIndex(pSI->m_pShape->GetColIndexByiOrder(k * 3 + 1));
			vB      *= pSI->m_Matrix;
			vC       = pSI->m_pShape->GetColVertexByIndex(pSI->m_pShape->GetColIndexByiOrder(k * 3 + 2));
			vC      *= pSI->m_Matrix;

			float a = 0.0f, b = 0.0f, c = 0.0f;
			a = ((vB.x - vA.x) * (vPos.z - vA.z)) - ((vB.z - vA.z) * (vPos.x - vA.x));
			b = ((vC.x - vB.x) * (vPos.z - vB.z)) - ((vC.z - vB.z) * (vPos.x - vB.x));
			c = ((vA.x - vC.x) * (vPos.z - vC.z)) - ((vA.z - vC.z) * (vPos.x - vC.x));

			if ((a <= 0 && b <= 0 && c <= 0) || (a >= 0 && b >= 0 && c >= 0))
			{
				__Vector3 vAB, vAC, vAxis;
				vAB.Set(vB.x - vA.x, vB.y - vA.y, vB.z - vA.z);
				vAC.Set(vC.x - vA.x, vC.y - vA.y, vC.z - vA.z);

				vAxis.Cross(vAB, vAC);
				if (vAxis.y <= 0.0f)
					continue;

				fHeight  = (vAxis.x * (vA.x - vPos.x) + vAxis.z * (vA.z - vPos.z)) / vAxis.y + vA.y;
				fHeight += m_fHeightOffs;

				if (pvNormal)
				{
					__Vector3 vEdge1 = vB - vA;
					__Vector3 vEdge2 = vC - vA;
					pvNormal->Cross(vEdge1, vEdge2);
					pvNormal->Normalize();
				}

				return fHeight;
			}
		}
	}

	return fHeight;
}

bool CPortalVolume::IsInTerrainWithTerrain(__Vector3& vec)
{
	__Vector3 vec2[3] {};
	for (int i = 0; i < 3; i++)
	{
		vec2[i]  = m_pvVertex[i];
		vec2[i] *= m_Matrix;
	}

	if (vec.x >= vec2[0].x && vec.x <= vec2[1].x && vec.z >= vec2[2].z && vec.z <= vec2[0].z)
		return GetHeightWithTerrain(vec.x, vec.z, vec.y);

	return false;
}

BOOL CPortalVolume::PickWideWithTerrain(int x, int y, __Vector3& vPick)
{
	// Compute the vector of the pick ray in screen space
	__Vector3 vTmp;
	vTmp.x             = (((2.0f * x) / (CN3Base::s_CameraData.vp.Width)) - 1) / CN3Base::s_CameraData.mtxProjection.m[0][0];
	vTmp.y             = -(((2.0f * y) / (CN3Base::s_CameraData.vp.Height)) - 1) / CN3Base::s_CameraData.mtxProjection.m[1][1];
	vTmp.z             = 1.0f;

	// Transform the screen space pick ray into 3D space
	__Matrix44* pMtxVI = &CN3Base::s_CameraData.mtxViewInverse;
	__Vector3 vDir;
	vDir.x         = vTmp.x * pMtxVI->m[0][0] + vTmp.y * pMtxVI->m[1][0] + vTmp.z * pMtxVI->m[2][0];
	vDir.y         = vTmp.x * pMtxVI->m[0][1] + vTmp.y * pMtxVI->m[1][1] + vTmp.z * pMtxVI->m[2][1];
	vDir.z         = vTmp.x * pMtxVI->m[0][2] + vTmp.y * pMtxVI->m[1][2] + vTmp.z * pMtxVI->m[2][2];
	__Vector3 vPos = pMtxVI->Pos();

	vDir.Normalize();

	__Vector3 A, B, C;
	float t = 0.0f, u = 0.0f, v = 0.0f;
	__ColIndex* pCI = nullptr;

	ciiter ciit     = m_lpShapeColPartList.begin();
	while (ciit != m_lpShapeColPartList.end())
	{
		pCI            = *ciit++;

		ShapeInfo* pSI = CPvsMgr::GetShapeInfoByManager(pCI->m_iID);
		pSI->m_pShape->PosSet(pSI->Pos());
		pSI->m_pShape->RotSet(pSI->Rot());
		pSI->m_pShape->ScaleSet(pSI->Scale());
		pSI->m_pShape->Tick(-1000);
		CN3Shape* pShape = pSI->m_pShape;

		size_t vecSize   = pCI->m_ivVector.size();
		for (size_t k = 0; k < vecSize / 3; k++)
		{
			pShape->PartialGetCollision(pCI->m_ivVector[k * 3], A);
			A *= pShape->m_Matrix;
			pShape->PartialGetCollision(pCI->m_ivVector[k * 3 + 1], B);
			B *= pShape->m_Matrix;
			pShape->PartialGetCollision(pCI->m_ivVector[k * 3 + 2], C);
			C *= pShape->m_Matrix;

			if (::_IntersectTriangle(vPos, vDir, A, B, C, t, u, v, &vPick))
			{
				__Vector3 vEdge1 = B - A;
				__Vector3 vEdge2 = C - A;
				__Vector3 pVec;
				pVec.Cross(vEdge1, vEdge2);
				pVec.Normalize();
				pVec.y = 0.0f;
				if (pVec.Magnitude() < m_fPickIncline) // 기울기..
					return TRUE;
			}
		}
	}

	return FALSE;
}

CN3Shape* CPortalVolume::PickWithShape(int iXScreen, int iYScreen, bool bMustHaveEvent, __Vector3* pvPick)
{
	__Vector3 vPos, vDir;
	::_Convert2D_To_3DCoordinate(
		iXScreen, iYScreen, s_CameraData.mtxView, s_CameraData.mtxProjection, s_CameraData.vp.Width, s_CameraData.vp.Height, vPos, vDir);

	// 거리순으로 정렬..
	std::vector<ShapeInfo*> Shapes;
	ShapeInfo* pSI      = nullptr;
	CPortalVolume* pVol = nullptr;
	VisPortalPriority vPP;
	vppiter vppit = m_pVisiblePvsList.begin();
	while (vppit != m_pVisiblePvsList.end())
	{
		vPP         = *vppit++;
		pVol        = vPP.m_pVol;

		siiter siit = pVol->m_plShapeInfoList.begin();
		while (siit != pVol->m_plShapeInfoList.end())
		{
			pSI = *siit++;
			Shapes.push_back(pSI);
		}
	}

	// NOTE: This is broken. This is meant to be used with __AlphaPrimitive*, not ShapeInfo*.
	// If called, this would crash.
	// qsort(&Shapes[0], Shapes.size(), sizeof(ShapeInfo*), CN3ShapeMgr::SortByCameraDistance);

	for (ShapeInfo* pShr : Shapes)
	{
		// 이벤트가 있어야 한다면...
		if (bMustHaveEvent && pShr->m_iEventID <= 0)
			continue;

		pShr->m_pShape->PosSet(pShr->Pos());
		pShr->m_pShape->RotSet(pShr->Rot());
		pShr->m_pShape->ScaleSet(pShr->Scale());

		if (pShr->m_pShape->CheckCollisionPrecisely(false, vPos, vDir, pvPick) >= 0)
		{
			pShr->m_pShape->m_iEventID    = pShr->m_iEventID;
			pShr->m_pShape->m_iEventType  = pShr->m_iEventType;
			pShr->m_pShape->m_iNPC_ID     = pShr->m_iNPC_ID;
			pShr->m_pShape->m_iNPC_Status = pShr->m_iNPC_Status;
			return pShr->m_pShape;
		}
	}

	return nullptr;
}

CN3Shape* CPortalVolume::ShapeGetByIDWithShape(int iID)
{
	ShapeInfo* pSI      = nullptr;
	CPortalVolume* pVol = nullptr;
	VisPortalPriority vPP;
	vppiter vppit = m_pVisiblePvsList.begin();
	while (vppit != m_pVisiblePvsList.end())
	{
		vPP         = *vppit++;
		pVol        = vPP.m_pVol;

		siiter siit = pVol->m_plShapeInfoList.begin();
		while (siit != pVol->m_plShapeInfoList.end())
		{
			pSI = *siit++;
			if (pSI->m_iEventID == iID)
			{
				pSI->m_pShape->m_iEventID    = pSI->m_iEventID;
				pSI->m_pShape->m_iEventType  = pSI->m_iEventType;
				pSI->m_pShape->m_iNPC_ID     = pSI->m_iNPC_ID;
				pSI->m_pShape->m_iNPC_Status = pSI->m_iNPC_Status;
				return pSI->m_pShape;
			}
		}
	}

	return nullptr;
}

bool CPortalVolume::CheckCollisionWithShape(const __Vector3& vPos, // 충돌 위치
	const __Vector3& vDir,                                         // 방향 벡터
	float fSpeedPerSec,                                            // 초당 움직이는 속도
	__Vector3* pvCol,                                              // 충돌 지점
	__Vector3* pvNormal,                                           // 충돌한면의 법선벡터
	__Vector3* pVec)                                               // 충돌한 면 의 폴리곤 __Vector3[3]
{
	__Vector3 vPosNext  = vPos + (vDir * fSpeedPerSec);            // 다음 위치
	float fMcs          = (vPosNext - vPos).Magnitude();

	bool bCollision     = false;
	bool bColl          = false;
	CPortalVolume* pVol = nullptr;
	ShapeInfo* pSI      = nullptr;

	VisPortalPriority vPP;
	vppiter vppit = m_pVisiblePvsList.begin();
	while (vppit != m_pVisiblePvsList.end())
	{
		vPP = *vppit++;
		if (vPP.m_iPriority > 1)
			continue;

		pVol        = vPP.m_pVol;

		siiter siit = pVol->m_plShapeInfoList.begin();
		while (siit != pVol->m_plShapeInfoList.end())
		{
			pSI       = *siit++;

			int iSize = pSI->m_pShape->GetColIndexbufferCount();
			__Vector3 vA, vB, vC, vPick, vEdge1, vEdge2;
			float t = 0.0f, u = 0.0f, v = 0.0f;

			for (int k = 0; k < iSize / 3; k++)
			{
				vA      = pSI->m_pShape->GetColVertexByIndex(pSI->m_pShape->GetColIndexByiOrder(k * 3));
				vA     *= pSI->m_Matrix;
				vB      = pSI->m_pShape->GetColVertexByIndex(pSI->m_pShape->GetColIndexByiOrder(k * 3 + 1));
				vB     *= pSI->m_Matrix;
				vC      = pSI->m_pShape->GetColVertexByIndex(pSI->m_pShape->GetColIndexByiOrder(k * 3 + 2));
				vC     *= pSI->m_Matrix;

				vEdge1  = vB - vA;
				vEdge2  = vC - vA;

				__Vector3 pVecto;
				pVecto.Cross(vEdge1, vEdge2);
				pVecto.Normalize();
				pVecto *= m_fOffs;
				vA     += pVecto;
				vB     += pVecto;
				vC     += pVecto;

				bColl   = ::_IntersectTriangle(vPos, vDir, vA, vB, vC, t, u, v, &vPick);
				if (bColl)
				{
					if ((vPick - vPos).Magnitude() < fMcs)
					{
						if (pvCol)
							*pvCol = vPick;
						if (pvNormal)
						{
							(*pvNormal).Cross(vB - vA, vC - vA);
							(*pvNormal).Normalize();
						}
						if (pVec)
						{
							pVec[0] = vA;
							pVec[1] = vB;
							pVec[2] = vC;
						}
						bCollision = true;
					}
				}
			}
		}
	}

	return bCollision;
}

// N3TransformCollision.cpp: implementation of the CN3TransformCollision class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfxBase.h"
#include "N3TransformCollision.h"
#include "N3VMesh.h"

CN3TransformCollision::CN3TransformCollision()
{
	m_dwType  |= OBJ_TRANSFORM_COLLISION;

	m_fRadius  = 0;
	m_vMin.Set(0, 0, 0);
	m_vMax.Set(0, 0, 0);

	m_pMeshCollision = nullptr;
	m_pMeshClimb     = nullptr;
}

CN3TransformCollision::~CN3TransformCollision()
{
	s_MngVMesh.Delete(&m_pMeshCollision);
	s_MngVMesh.Delete(&m_pMeshClimb);
}

void CN3TransformCollision::Release()
{
	m_fRadius = 0;
	m_vMin.Set(0, 0, 0);
	m_vMax.Set(0, 0, 0);

	s_MngVMesh.Delete(&m_pMeshCollision);
	s_MngVMesh.Delete(&m_pMeshClimb);

	CN3Transform::Release();
}

bool CN3TransformCollision::Load(File& file)
{
	CN3Transform::Load(file);

	int nL         = 0;
	char szFN[512] = "";

	file.Read(&nL, 4); // Mesh FileName
	if (nL > 0)
	{
		file.Read(szFN, nL);
		szFN[nL]         = '\0'; // 메시 파일 이름..
		m_pMeshCollision = s_MngVMesh.Get(szFN);
	}

	file.Read(&nL, 4); // Mesh FileName
	if (nL > 0)
	{
		file.Read(szFN, nL);
		szFN[nL]     = '\0'; // 메시 파일 이름..
		m_pMeshClimb = s_MngVMesh.Get(szFN);
	}
	return true;
}

#ifdef _N3TOOL
bool CN3TransformCollision::Save(File& file)
{
	CN3Transform::Save(file);

	int nL = 0;
	if (m_pMeshCollision != nullptr)
		nL = static_cast<int>(m_pMeshCollision->FileName().size());
	file.Write(&nL, 4); // Mesh FileName

	if (nL > 0)
	{
		// 임시로 경로를 바꾸려고 넣었다.. 나중에 필요없음 지운다..
		if (m_pMeshCollision->FileName().find("object\\") == std::string::npos)
		{
			std::string szFNTmp = fmt::format("Object\\{}.N3VMesh", m_pMeshCollision->m_szName);
			m_pMeshCollision->FileNameSet(szFNTmp);

			file.Seek(-4, SEEK_CUR);
			nL = static_cast<int>(m_pMeshCollision->FileName().size());
			file.Write(&nL, 4); // Mesh FileName
		}

		file.Write(m_pMeshCollision->FileName().c_str(), nL);
	}

	nL = 0;
	if (m_pMeshClimb != nullptr)
		nL = static_cast<int>(m_pMeshClimb->FileName().size());
	file.Write(&nL, 4); // Mesh FileName

	if (nL > 0)
	{
		// 임시로 경로를 바꾸려고 넣었다.. 나중에 필요없음 지운다..
		if (m_pMeshClimb->FileName().find("object\\") == std::string::npos)
		{
			std::string szFNTmp = fmt::format("Object\\{}.N3VMesh", m_pMeshClimb->m_szName);
			m_pMeshClimb->FileNameSet(szFNTmp);

			file.Seek(-4, SEEK_CUR);
			nL = static_cast<int>(m_pMeshClimb->FileName().size());
			file.Write(&nL, 4); // Mesh FileName
		}

		file.Write(m_pMeshClimb->FileName().c_str(), nL);
	}
	return true;
}
#endif // end of _N3TOOL

void CN3TransformCollision::CollisionMeshSet(const std::string& szFN)
{
	s_MngVMesh.Delete(&m_pMeshCollision);
	m_pMeshCollision = s_MngVMesh.Get(szFN);
	if (m_pMeshCollision)
		this->FindMinMax();
}

void CN3TransformCollision::ClimbMeshSet(const std::string& szFN)
{
	s_MngVMesh.Delete(&m_pMeshClimb);
	m_pMeshClimb = s_MngVMesh.Get(szFN);
	if (m_pMeshClimb)
		m_pMeshClimb->FindMinMax();
}

int CN3TransformCollision::CheckCollisionPrecisely(
	bool /*bIgnoreBoxCheck*/, int ixScreen, int iyScreen, __Vector3* pVCol, __Vector3* pVNormal)
{
	__Vector3 vPos, vDir; // 2D 좌표를 3D 좌표로 바꾸고..
	::_Convert2D_To_3DCoordinate(ixScreen, iyScreen, s_CameraData.mtxView,
		s_CameraData.mtxProjection, s_CameraData.vp.Width, s_CameraData.vp.Height, vPos, vDir);

	if (!m_pMeshCollision->Pick(m_Matrix, vPos, vDir, pVCol, pVNormal))
		return -1;
	else
		return 0;
}

#if defined(_DEBUG) || defined(_N3TOOL)
void CN3TransformCollision::RenderCollisionMesh()
{
	if (nullptr == m_pMeshCollision)
		return;
	s_lpD3DDev->SetTransform(D3DTS_WORLD, m_Matrix.toD3D());

	m_pMeshCollision->Render(0xffff0000); // 빨간색.
}

void CN3TransformCollision::RenderClimbMesh()
{
	if (nullptr == m_pMeshClimb)
		return;
	s_lpD3DDev->SetTransform(D3DTS_WORLD, m_Matrix.toD3D());

	m_pMeshClimb->Render(0xff0000ff); // 파란색..
}
#endif

/*
#ifdef _N3TOOL
BOOL CN3TransformCollision::CheckClimb(__Vector3 &vPos, __Vector3 &vDir, __Vector3* pVCol, __Vector3* pVNormal, __Vector3* pVPolygon)
{
	if(nullptr == m_pMeshClimb) return FALSE;
	
	int nIC = m_pMeshClimb->IndexCount();
	int nFC = 0;
	if(nIC > 0)
	{
		nFC = nIC / 3; // Face Count
	}
	else
	{
		nFC = m_pMeshClimb->VertexCount() / 3;
	}
	if(nFC <= 0) return FALSE;

	float t,u,v;
	__Vector3 vSrc[3];

	if(nIC > 0)
	{
		uint16_t* pwIndices = m_pMeshClimb->Indices();
		__Vector3* pVSrc = m_pMeshClimb->Vertices();
		for(int i = 0; i < nFC; i++)
		{
			vSrc[0] = pVSrc[pwIndices[i*3+0]] * m_Matrix;
			vSrc[1] = pVSrc[pwIndices[i*3+1]] * m_Matrix;
			vSrc[2] = pVSrc[pwIndices[i*3+2]] * m_Matrix;

			if(TRUE == ::IntersectTriangle(vPos, vDir, vSrc[0], vSrc[1], vSrc[2], t, u, v)) 
			{
				if(pVCol) *pVCol = (vDir * t) + vPos;
				if(pVNormal) { pVNormal->Cross(vSrc[1] - vSrc[0], vSrc[2] - vSrc[0]); pVNormal->Normalize(); }
				if(pVPolygon)
				{
					pVPolygon[0] = vSrc[0];
					pVPolygon[1] = vSrc[1];
					pVPolygon[2] = vSrc[2];
				}
				return TRUE;
			}
		}
	}
	else
	{
		__Vector3* pVSrc = m_pMeshClimb->Vertices();
		for(int i = 0; i < nFC; i++, pVSrc += 3)
		{
			vSrc[0] = pVSrc[0] * m_Matrix;
			vSrc[1] = pVSrc[1] * m_Matrix;
			vSrc[2] = pVSrc[2] * m_Matrix;

			if(TRUE == ::IntersectTriangle(vPos, vDir, vSrc[0], vSrc[1], vSrc[2], t, u, v)) 
			{
				if(pVCol) *pVCol = (vDir * t) + vPos;
				if(pVNormal) { pVNormal->Cross(vSrc[1] - vSrc[0], vSrc[2] - vSrc[0]); pVNormal->Normalize(); }
				if(pVPolygon)
				{
					pVPolygon[0] = vSrc[0];
					pVPolygon[1] = vSrc[1];
					pVPolygon[2] = vSrc[2];
				}
				return TRUE;
			}
		}
	}

	return FALSE;
}
#endif // end of _N3TOOL

#ifdef _N3TOOL
BOOL CN3TransformCollision::CheckClimb(int x, int y, __Vector3* pVCol, __Vector3* pVNormal, __Vector3* pVPolygon)
{
	if(nullptr == m_pMeshClimb || nullptr == m_pMeshClimb->Vertices()) return FALSE;

	// Compute the vector of the pick ray in screen space
	__Vector3 vTmp;
	vTmp.x =  ( ( ( 2.0f * x ) / (CN3Base::s_CameraData.vp.Width) ) - 1 ) / CN3Base::s_CameraData.mtxProjection.m[0][0];
	vTmp.y = -( ( ( 2.0f * y ) / (CN3Base::s_CameraData.vp.Height) ) - 1 ) / CN3Base::s_CameraData.mtxProjection.m[1][1];
	vTmp.z =  1.0f;

	// Transform the screen space pick ray into 3D space
	__Matrix44* pMtxVI = &CN3Base::s_CameraData.mtxViewInverse;
	__Vector3 vPos, vDir;
	vDir.x  = vTmp.x * pMtxVI->m[0][0] + vTmp.y * pMtxVI->m[1][0] + vTmp.z * pMtxVI->m[2][0];
	vDir.y  = vTmp.x * pMtxVI->m[0][1] + vTmp.y * pMtxVI->m[1][1] + vTmp.z * pMtxVI->m[2][1];
	vDir.z  = vTmp.x * pMtxVI->m[0][2] + vTmp.y * pMtxVI->m[1][2] + vTmp.z * pMtxVI->m[2][2];
	vPos = pMtxVI->Pos();

	return CheckClimb(vPos, vDir, pVCol, pVNormal, pVPolygon);
}
#endif // end of _N3TOOL
*/

void CN3TransformCollision::FindMinMax()
{
	m_vMin.Set(0, 0, 0);
	m_vMax.Set(0, 0, 0);
	m_fRadius = 0.0f;

	if (nullptr == m_pMeshCollision || m_pMeshCollision->VertexCount() <= 0)
		return;

	m_vMin.Set(FLT_MAX, FLT_MAX, FLT_MAX);
	m_vMax.Set(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	int nVC        = m_pMeshCollision->VertexCount();
	__Vector3* pVs = m_pMeshCollision->Vertices();
	for (int i = 0; i < nVC; i++)
	{
		if (pVs[i].x < m_vMin.x)
			m_vMin.x = pVs[i].x;
		if (pVs[i].y < m_vMin.y)
			m_vMin.y = pVs[i].y;
		if (pVs[i].z < m_vMin.z)
			m_vMin.z = pVs[i].z;
		if (pVs[i].x > m_vMax.x)
			m_vMax.x = pVs[i].x;
		if (pVs[i].y > m_vMax.y)
			m_vMax.y = pVs[i].y;
		if (pVs[i].z > m_vMax.z)
			m_vMax.z = pVs[i].z;
	}

	// 최대 최소값을 갖고 반지름 계산한다..
	m_fRadius = (m_vMax - m_vMin).Magnitude() * 0.5f;
}

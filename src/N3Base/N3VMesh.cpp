// N3VMesh.cpp: implementation of the C3DObject class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfxBase.h"
#include "N3VMesh.h"
#include "N3IMesh.h"

CN3VMesh::CN3VMesh()
{
	m_dwType    |= OBJ_MESH_VECTOR3;

	m_pVertices  = nullptr; // 점 버퍼
	m_pwIndices  = nullptr; // Index...

	m_nVC        = 0;
	m_nIC        = 0;

	m_vMin.Zero();
	m_vMax.Zero();
	m_fRadius = 0.0f; // 반지름
}

CN3VMesh::~CN3VMesh()
{
	if (m_nVC > 32768)
		::GlobalFree((HGLOBAL) (m_pVertices));
	else
		delete[] m_pVertices;
	delete[] m_pwIndices;
	m_pwIndices = nullptr;
}

void CN3VMesh::Release()
{
	if (m_nVC > 32768)
		::GlobalFree((HGLOBAL) (m_pVertices));
	else
		delete[] m_pVertices;
	m_pVertices = nullptr;
	m_nVC       = 0;

	delete[] m_pwIndices;
	m_pwIndices = nullptr;
	m_nIC       = 0;

	m_vMin.Zero();
	m_vMax.Zero();
	m_fRadius = 0.0f; // 반지름
}

bool CN3VMesh::Load(File& file)
{
	CN3BaseFileAccess::Load(file);

	int nVC = 0, nIC = 0;
	file.Read(&nVC, 4);      // 점갯수 읽기..
	if (nVC > 0)
	{
		CreateVertices(nVC); // Vertex Buffer 생성 및 데이터 채우기
		file.Read(m_pVertices, nVC * sizeof(__Vector3));
	}

	file.Read(&nIC, 4);   // Index Count..
	if (nIC > 0)
	{
		CreateIndex(nIC); // Vertex Buffer 생성 및 데이터 채우기
		file.Read(m_pwIndices, nIC * 2);
	}

	FindMinMax(); // 최대 최소점과 중심점과 반지름을 계산해 준다..

	return true;
}

#ifdef _N3TOOL
bool CN3VMesh::Save(File& file)
{
	CN3BaseFileAccess::Save(file);

	file.Write(&m_nVC, 4); // 점갯수 읽기..
	if (m_nVC > 0)
	{
		file.Write(m_pVertices, m_nVC * sizeof(__Vector3));
	}

	file.Write(&m_nIC, 4);                  // Index Count..
	if (m_nIC > 0)
	{
		file.Write(m_pwIndices, m_nIC * 2); // Index Buffer 데이터 쓰기..
	}

	return true;
}
#endif // end of _N3TOOL

void CN3VMesh::CreateVertices(int nVC)
{
	if (nVC <= 0)
		return;

	if (m_nVC > 32768)
		::GlobalFree((HGLOBAL) (m_pVertices));
	else
		delete[] m_pVertices;

	m_pVertices = nullptr;
	m_nVC       = 0;

	if (nVC > 32768)
	{
		m_pVertices = (__Vector3*) (::GlobalAlloc(
			GMEM_FIXED | GMEM_ZEROINIT, nVC * sizeof(__Vector3)));
	}
	else
	{
		m_pVertices = new __Vector3[nVC] {};
	}

	if (m_pVertices == nullptr)
		return;

	m_nVC = nVC;
}

void CN3VMesh::CreateIndex(int nIC)
{
	__ASSERT(nIC > 0, "Index count is less than 0");

#ifdef _N3GAME
	if (nIC > 32768)
		CLogWriter::Write("N3VMesh creation warning ({}) - Too many Indices.", m_szFileName);
#endif

	delete[] m_pwIndices;
	m_pwIndices = new uint16_t[nIC];
	memset(m_pwIndices, 0, nIC * 2); // Index Buffer 생성
	m_nIC = nIC;
}

void CN3VMesh::CreateCube(const __Vector3& vMin, const __Vector3& vMax)
{
	this->CreateVertices(8);
	this->CreateIndex(36);

	m_pVertices[0].Set(vMin.x, vMax.y, vMin.z);
	m_pVertices[1].Set(vMax.x, vMax.y, vMin.z);
	m_pVertices[2].Set(vMax.x, vMin.y, vMin.z);
	m_pVertices[3].Set(vMin.x, vMin.y, vMin.z);

	m_pVertices[4].Set(vMax.x, vMax.y, vMax.z);
	m_pVertices[5].Set(vMin.x, vMax.y, vMax.z);
	m_pVertices[6].Set(vMin.x, vMin.y, vMax.z);
	m_pVertices[7].Set(vMax.x, vMin.y, vMax.z);

	m_pwIndices[0]  = 0;
	m_pwIndices[1]  = 1;
	m_pwIndices[2]  = 2;
	m_pwIndices[3]  = 0;
	m_pwIndices[4]  = 2;
	m_pwIndices[5]  = 3;
	m_pwIndices[6]  = 1;
	m_pwIndices[7]  = 4;
	m_pwIndices[8]  = 7;
	m_pwIndices[9]  = 1;
	m_pwIndices[10] = 7;
	m_pwIndices[11] = 2;
	m_pwIndices[12] = 4;
	m_pwIndices[13] = 5;
	m_pwIndices[14] = 6;
	m_pwIndices[15] = 4;
	m_pwIndices[16] = 6;
	m_pwIndices[17] = 7;
	m_pwIndices[18] = 5;
	m_pwIndices[19] = 0;
	m_pwIndices[20] = 3;
	m_pwIndices[21] = 5;
	m_pwIndices[22] = 3;
	m_pwIndices[23] = 6;
	m_pwIndices[24] = 5;
	m_pwIndices[25] = 4;
	m_pwIndices[26] = 1;
	m_pwIndices[27] = 5;
	m_pwIndices[28] = 1;
	m_pwIndices[29] = 0;
	m_pwIndices[30] = 3;
	m_pwIndices[31] = 2;
	m_pwIndices[32] = 7;
	m_pwIndices[33] = 3;
	m_pwIndices[34] = 7;
	m_pwIndices[35] = 6;

	this->FindMinMax(); // 중심점과 반지름을 계산해 준다..
}

void CN3VMesh::Render(D3DCOLOR crLine)
{
	if (m_nVC <= 0)
		return;

	DWORD dwLight = 0, dwShade = 0;
	s_lpD3DDev->GetRenderState(D3DRS_LIGHTING, &dwLight);
	s_lpD3DDev->GetRenderState(D3DRS_FILLMODE, &dwShade);
	if (dwLight != 0)
		s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);
	if (D3DFILL_WIREFRAME != dwShade)
		s_lpD3DDev->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	s_lpD3DDev->SetTexture(0, nullptr);
	s_lpD3DDev->SetFVF(FVF_CV);

	__VertexColor vTs[3] {};
	if (m_nIC > 0)
	{
		int nFC = m_nIC / 3;
		for (int i = 0; i < nFC; i++)
		{
			vTs[0].Set(m_pVertices[m_pwIndices[i * 3 + 0]], crLine);
			vTs[1].Set(m_pVertices[m_pwIndices[i * 3 + 1]], crLine);
			vTs[2].Set(m_pVertices[m_pwIndices[i * 3 + 2]], crLine);

			s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, vTs, sizeof(__VertexColor));
		}
	}
	else
	{
		int nFC = m_nVC / 3;
		for (int i = 0; i < nFC; i++)
		{
			vTs[0].Set(m_pVertices[i * 3 + 0], crLine);
			vTs[1].Set(m_pVertices[i * 3 + 1], crLine);
			vTs[2].Set(m_pVertices[i * 3 + 2], crLine);

			s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, vTs, sizeof(__VertexColor));
		}
	}

	if (dwLight != 0)
		s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, dwLight);

	if (D3DFILL_WIREFRAME != dwShade)
		s_lpD3DDev->SetRenderState(D3DRS_FILLMODE, dwShade);
}

void CN3VMesh::FindMinMax()
{
	m_vMin.Zero();
	m_vMax.Zero();
	m_fRadius = 0;

	if (m_nVC <= 0)
		return;

	m_vMin.Set(FLT_MAX, FLT_MAX, FLT_MAX);
	m_vMax.Set(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (int i = 0; i < m_nVC; i++)
	{
		if (m_pVertices[i].x < m_vMin.x)
			m_vMin.x = m_pVertices[i].x;
		if (m_pVertices[i].y < m_vMin.y)
			m_vMin.y = m_pVertices[i].y;
		if (m_pVertices[i].z < m_vMin.z)
			m_vMin.z = m_pVertices[i].z;
		if (m_pVertices[i].x > m_vMax.x)
			m_vMax.x = m_pVertices[i].x;
		if (m_pVertices[i].y > m_vMax.y)
			m_vMax.y = m_pVertices[i].y;
		if (m_pVertices[i].z > m_vMax.z)
			m_vMax.z = m_pVertices[i].z;
	}

	// 최대 최소값을 갖고 반지름 계산한다..
	m_fRadius = (m_vMax - m_vMin).Magnitude() * 0.5f;
}

bool CN3VMesh::CheckCollision(const __Matrix44& MtxWorld, const __Vector3& v0, const __Vector3& v1,
	__Vector3* pVCol, __Vector3* pVNormal)
{
	if (m_nVC <= 0)
		return false;

	static __Vector3 vPos0, vPos1, vDir, vColTmp;
	static __Matrix44 mtxWI, mtxWIRot, mtxRot;
	static float fT, fU, fV, fDistTmp, fDistClosest;
	fDistClosest = FLT_MAX;

	mtxWI        = MtxWorld.Inverse(); // World Matrix Inverse
	mtxWIRot     = mtxWI;
	mtxWIRot.PosSet(0, 0, 0);
	mtxRot = MtxWorld;
	mtxRot.PosSet(0, 0, 0);

	vPos0   = v0 * mtxWI;
	vPos1   = v1 * mtxWI;
	vDir    = vPos1 - vPos0;

	int nFC = 0;
	if (m_nIC > 0)
		nFC = m_nIC / 3;
	else
		nFC = m_nVC / 3;

	int nCI0 = 0, nCI1 = 0, nCI2 = 0; // Collision polygon index
	for (int i = 0; i < nFC; i++)
	{
		if (m_nIC > 0)
		{
			nCI0 = m_pwIndices[i * 3 + 0];
			nCI1 = m_pwIndices[i * 3 + 1];
			nCI2 = m_pwIndices[i * 3 + 2];
		}
		else
		{
			nCI0 = i * 3;
			nCI1 = i * 3 + 1;
			nCI2 = i * 3 + 2;
		}

		if (false
			== ::_IntersectTriangle(vPos0, vDir, m_pVertices[nCI0], m_pVertices[nCI1],
				m_pVertices[nCI2], fT, fU, fV, &vColTmp))
			continue;                                 // 첫째 벡터가 걸치면..
		if (false
			== ::_IntersectTriangle(vPos1, vDir, m_pVertices[nCI0], m_pVertices[nCI1],
				m_pVertices[nCI2]))                   // 둘째는 안 걸치면..
		{
			fDistTmp = (vPos0 - vColTmp).Magnitude(); // 거리를 재보고..
			if (fDistTmp < fDistClosest)
			{
				fDistClosest = fDistTmp;

				if (pVCol)
					*pVCol = vColTmp * MtxWorld;

				// 법선 벡터 구하기..
				if (pVNormal)
				{
					(*pVNormal).Cross(m_pVertices[nCI1] - m_pVertices[nCI0],
						m_pVertices[nCI2] - m_pVertices[nCI1]);
					(*pVNormal) *= mtxRot;
					(*pVNormal).Normalize();
				}
			}
		}
	}
	if (fDistClosest != FLT_MAX)
		return true;

	//두점이 충돌메시 안에 있는 경우..by lynus..
	__Vector3 tmpNormal;
	float d = 0.0f;
	for (int i = 0; i < nFC; i++)
	{
		if (m_nIC > 0)
		{
			nCI0 = m_pwIndices[i * 3 + 0];
			nCI1 = m_pwIndices[i * 3 + 1];
			nCI2 = m_pwIndices[i * 3 + 2];
		}
		else
		{
			nCI0 = i * 3;
			nCI1 = i * 3 + 1;
			nCI2 = i * 3 + 2;
		}

		tmpNormal.Cross(
			m_pVertices[nCI1] - m_pVertices[nCI0], m_pVertices[nCI2] - m_pVertices[nCI1]);
		d = -(tmpNormal.x * m_pVertices[nCI0].x) - (tmpNormal.y * m_pVertices[nCI0].y)
			- (tmpNormal.z * m_pVertices[nCI0].z);
		if (((tmpNormal.x * vPos0.x) + (tmpNormal.y * vPos0.y) + (tmpNormal.z * vPos0.z) + d) > 0)
			return false;
	}

	return true;
}

bool CN3VMesh::Pick(const __Matrix44& MtxWorld, const __Vector3& vPos, const __Vector3& vDir,
	__Vector3* pVCol, __Vector3* pVNormal)
{
	if (m_nVC <= 0)
		return false;

	__Matrix44 mtxWI, mtxWIRot, mtxRot;
	mtxWI    = MtxWorld.Inverse(); // World Matrix Inverse
	mtxWIRot = mtxWI;
	mtxWIRot.PosSet(0, 0, 0);
	mtxRot = MtxWorld;
	mtxRot.PosSet(0, 0, 0);
	__Vector3 vPos2 = vPos * mtxWI;
	__Vector3 vDir2 = vDir * mtxWIRot;

	int nFC         = 0;
	if (m_nIC > 0)
		nFC = m_nIC / 3;
	else
		nFC = m_nVC / 3;

	int nCI0 = 0, nCI1 = 0, nCI2 = 0; // Collision polygon index
	for (int i = 0; i < nFC; i++)
	{
		if (m_nIC > 0)
		{
			nCI0 = m_pwIndices[i * 3 + 0];
			nCI1 = m_pwIndices[i * 3 + 1];
			nCI2 = m_pwIndices[i * 3 + 2];
		}
		else
		{
			nCI0 = i * 3;
			nCI1 = i * 3 + 1;
			nCI2 = i * 3 + 2;
		}

		if (!::_IntersectTriangle(
				vPos2, vDir2, m_pVertices[nCI0], m_pVertices[nCI1], m_pVertices[nCI2]))
			continue;

		// 충돌이다..
		if (pVCol != nullptr)
		{
			float fT = 0.0f, fU = 0.0f, fV = 0.0f;
			::_IntersectTriangle(vPos2, vDir2, m_pVertices[nCI0], m_pVertices[nCI1],
				m_pVertices[nCI2], fT, fU, fV, pVCol);
			*pVCol *= MtxWorld;
		}

		// 법선 벡터 구하기..
		if (pVNormal != nullptr)
		{
			pVNormal->Cross(
				m_pVertices[nCI1] - m_pVertices[nCI0], m_pVertices[nCI2] - m_pVertices[nCI1]);
			(*pVNormal) *= mtxRot;
			pVNormal->Normalize();
		}

		return true;
	}

	return false;
}

#ifdef _N3TOOL
bool CN3VMesh::Import(CN3IMesh* pIMesh)
{
	if (nullptr == pIMesh)
		return false;
	__VertexT1* pvSrc = pIMesh->BuildVertexList();
	if (nullptr == pvSrc)
		return false;

	int nFC = pIMesh->FaceCount();
	this->Release();
	this->CreateVertices(nFC * 3);

	for (int i = 0; i < nFC; i++) // Normal 값 다시 세팅..
	{
		m_pVertices[i * 3 + 0] = pvSrc[i * 3 + 0];
		m_pVertices[i * 3 + 1] = pvSrc[i * 3 + 1];
		m_pVertices[i * 3 + 2] = pvSrc[i * 3 + 2];
	}

	m_szName = pIMesh->m_szName; // 이름..
	return true;
}
#endif                           // end of _N3TOOL

int CN3VMesh::GetColIndexByiOrder(int iOrder)
{
	if (m_nIC)
	{
		return m_pwIndices[iOrder];
	}
	else
	{
		return iOrder;
	}
}

__Vector3 CN3VMesh::GetColVertexByIndex(int iIndex)
{
	return m_pVertices[iIndex];
}

void CN3VMesh::PartialColRender(int iCount, int* piIndices)
{
	if (m_nVC <= 0)
		return;

	DWORD dwLight = 0, dwShade = 0;
	s_lpD3DDev->GetRenderState(D3DRS_LIGHTING, &dwLight);
	s_lpD3DDev->GetRenderState(D3DRS_FILLMODE, &dwShade);
	if (dwLight)
		s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);
	if (D3DFILL_WIREFRAME != dwShade)
		s_lpD3DDev->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	s_lpD3DDev->SetTexture(0, nullptr);
	s_lpD3DDev->SetFVF(FVF_CV);

	__VertexColor vTs[3] {};
	if (iCount > 0)
	{
		int nFC = iCount / 3;
		for (int i = 0; i < nFC; i++)
		{
			vTs[0].Set(m_pVertices[piIndices[i * 3 + 0]], 0xffffffff);
			vTs[1].Set(m_pVertices[piIndices[i * 3 + 1]], 0xffffffff);
			vTs[2].Set(m_pVertices[piIndices[i * 3 + 2]], 0xffffffff);

			s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, vTs, sizeof(__VertexColor));
		}
	}

	if (dwLight)
		s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, dwLight);
	if (D3DFILL_WIREFRAME != dwShade)
		s_lpD3DDev->SetRenderState(D3DRS_FILLMODE, dwShade);
}

void CN3VMesh::PartialGetCollision(int iIndex, __Vector3& vec)
{
	if (m_nVC <= 0)
		return;

	vec = m_pVertices[iIndex];
}

// N3Mesh.cpp: implementation of the C3DObject class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfxBase.h"
#include "N3Mesh.h"
#include "N3IMesh.h"
#include "N3PMesh.h"
#include "N3PMeshInstance.h"

CN3Mesh::CN3Mesh()
{
	m_dwType |= OBJ_MESH;

	m_vMin.Set(0, 0, 0);
	m_vMax.Set(0, 0, 0);

	m_nVC        = 0;
	m_nIC        = 0;

	m_pVertices  = nullptr; // 점 버퍼
	m_psnIndices = nullptr; // 인덱스 버퍼
}

CN3Mesh::~CN3Mesh()
{
	delete[] m_pVertices;
	m_pVertices = nullptr;
	delete[] m_psnIndices;
	m_psnIndices = nullptr;
}

void CN3Mesh::Release()
{
	this->ReleaseVertices();
	this->ReleaseIndices();

	CN3BaseFileAccess::Release();
}

void CN3Mesh::ReleaseVertices()
{
	delete[] m_pVertices;
	m_pVertices = nullptr;

	m_vMin.Set(0, 0, 0);
	m_vMax.Set(0, 0, 0);

	m_nVC = 0;
}

void CN3Mesh::ReleaseIndices()
{
	delete[] m_psnIndices;
	m_psnIndices = nullptr;
	m_nIC        = 0;
}

bool CN3Mesh::Load(File& file)
{
	if (m_pVertices != nullptr || m_psnIndices != nullptr)
		Release();

	file.Read(&m_nVC, 4); // 점갯수 읽기..

	if (m_nVC > 0)
	{
		Create(m_nVC, 0); // Vertex Buffer 생성 및 데이터 채우기
		file.Read(m_pVertices, m_nVC * sizeof(__VertexT1));
		FindMinMax();     // 최대, 최소값을 찾는다.
	}

	file.Read(&m_nIC, 4); // 인덱스 갯수 읽기..
	if (m_nIC > 0)
	{
		Create(0, m_nIC); // 인덱스 버퍼 생성 및 데이터 채우기
		file.Read(m_psnIndices, m_nIC * 2);
	}

	return true;
}

#ifdef _N3TOOL
bool CN3Mesh::Save(File& file)
{
	file.Write(&m_nVC, 4); // 점갯수 읽기..
	if (m_nVC > 0)
		file.Write(m_pVertices, m_nVC * sizeof(__VertexT1));

	file.Write(&m_nIC, 4); // 인덱스 갯수 읽기..
	if (m_nIC > 0)
		file.Write(m_psnIndices, m_nIC * 2);

	return true;
}
#endif // end of _N3TOOL

#ifdef _N3TOOL
void CN3Mesh::MakeIndexed()
{
	if (m_nVC <= 0 || m_nIC > 0)
		return;

	int nIs[100000];
	memset(nIs, -1, sizeof(nIs));

	__VertexT1* pVs = new __VertexT1[m_nVC];
	memcpy(pVs, m_pVertices, sizeof(__VertexT1) * m_nVC);
	int nICount = 0, nVCount = 0;
	int i;
	for (i = 0; i < m_nVC; i++)
	{
		BOOL bAccord = FALSE;
		int j;
		for (j = 0; j < nVCount; j++)
		{
			if (m_pVertices[i].x == pVs[j].x && m_pVertices[i].y == pVs[j].y
				&& m_pVertices[i].z == pVs[j].z &&
				//				m_pVertices[i].nx == pVs[j].nx &&
				//				m_pVertices[i].ny == pVs[j].ny &&
				//				m_pVertices[i].nz == pVs[j].nz &&
				m_pVertices[i].tu == pVs[j].tu && m_pVertices[i].tv == pVs[j].tv)
			{
				bAccord = TRUE;
				break;
			}
		}

		if (TRUE == bAccord)
		{
			nIs[i] = j;
		}
		else
		{
			nIs[i]       = nVCount;
			pVs[nVCount] = m_pVertices[i];
			nVCount++;
		}
	}

	this->Create(nVCount, m_nVC);
	memcpy(m_pVertices, pVs, sizeof(__VertexT1) * nVCount);
	for (i = 0; i < m_nIC; i++)
		m_psnIndices[i] = nIs[i];

	delete[] pVs;
	pVs = nullptr;
}
#endif // end of _N3TOOL

void CN3Mesh::Create(int nVC, int nIC)
{
	if (nVC > 0) // 점이 있으면...
	{
#ifdef _N3GAME
		if (nVC > 32768)
		{
			CLogWriter::Write(
				"CN3Mesh::Create - Too many vertices. (more than 32768) ({})", m_szFileName);
		}
#endif
		if (m_pVertices != nullptr)
			ReleaseVertices();
		m_pVertices = new __VertexT1[nVC] {};
		m_nVC       = nVC;
	}

	if (nIC > 0) // Mesh 로딩에 성공하고, 인덱스가 있으면..
	{
#ifdef _N3GAME
		if (nIC > 32768)
		{
			CLogWriter::Write(
				"CN3Mesh::Create - Too many indices. (more than 32768) ({})", m_szFileName);
		}
#endif
		if (m_psnIndices)
			ReleaseIndices();
		m_psnIndices = new uint16_t[nIC];
		memset(m_psnIndices, 0, nIC * 2); // Index Buffer 생성
		m_nIC = nIC;
	}
}

void CN3Mesh::FindMinMax()
{
	m_vMin.Zero();
	m_vMax.Zero();

	if (m_nVC <= 0)
		return;

	// 최소, 최대 점을 찾는다.
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
}

void CN3Mesh::Create_Cube(const __Vector3& vMin, const __Vector3& vMax)
{
	this->Create(36, 0);

	__Vector3 vPs[6] {};
	__Vector3 vN;
	float fTUVs[6][2] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 0 }, { 1, 1 }, { 0, 1 } };

	// z 축 음의 면
	vN.Set(0, 0, -1);
	vPs[0].Set(vMin.x, vMax.y, vMin.z);
	vPs[1].Set(vMax.x, vMax.y, vMin.z);
	vPs[2].Set(vMax.x, vMin.y, vMin.z);
	vPs[3] = vPs[0];
	vPs[4] = vPs[2];
	vPs[5].Set(vMin.x, vMin.y, vMin.z);
	for (int i = 0; i < 6; i++)
		m_pVertices[0 + i].Set(vPs[i], vN, fTUVs[i][0], fTUVs[i][1]);

	// x 축 양의 면
	vN.Set(1, 0, 0);
	vPs[0].Set(vMax.x, vMax.y, vMin.z);
	vPs[1].Set(vMax.x, vMax.y, vMax.z);
	vPs[2].Set(vMax.x, vMin.y, vMax.z);
	vPs[3] = vPs[0];
	vPs[4] = vPs[2];
	vPs[5].Set(vMax.x, vMin.y, vMin.z);
	for (int i = 0; i < 6; i++)
		m_pVertices[6 + i].Set(vPs[i], vN, fTUVs[i][0], fTUVs[i][1]);

	// z 축 양의 면
	vN.Set(0, 0, 1);
	vPs[0].Set(vMax.x, vMax.y, vMax.z);
	vPs[1].Set(vMin.x, vMax.y, vMax.z);
	vPs[2].Set(vMin.x, vMin.y, vMax.z);
	vPs[3] = vPs[0];
	vPs[4] = vPs[2];
	vPs[5].Set(vMax.x, vMin.y, vMax.z);
	for (int i = 0; i < 6; i++)
		m_pVertices[12 + i].Set(vPs[i], vN, fTUVs[i][0], fTUVs[i][1]);

	// x 축 음의 면
	vN.Set(-1, 0, 0);
	vPs[0].Set(vMin.x, vMax.y, vMax.z);
	vPs[1].Set(vMin.x, vMax.y, vMin.z);
	vPs[2].Set(vMin.x, vMin.y, vMin.z);
	vPs[3] = vPs[0];
	vPs[4] = vPs[2];
	vPs[5].Set(vMin.x, vMin.y, vMax.z);
	for (int i = 0; i < 6; i++)
		m_pVertices[18 + i].Set(vPs[i], vN, fTUVs[i][0], fTUVs[i][1]);

	// y 축 양의 면
	vN.Set(0, 1, 0);
	vPs[0].Set(vMin.x, vMax.y, vMax.z);
	vPs[1].Set(vMax.x, vMax.y, vMax.z);
	vPs[2].Set(vMax.x, vMax.y, vMin.z);
	vPs[3] = vPs[0];
	vPs[4] = vPs[2];
	vPs[5].Set(vMin.x, vMax.y, vMin.z);
	for (int i = 0; i < 6; i++)
		m_pVertices[24 + i].Set(vPs[i], vN, fTUVs[i][0], fTUVs[i][1]);

	// y 축 음의 면
	vN.Set(0, -1, 0);
	vPs[0].Set(vMin.x, vMin.y, vMin.z);
	vPs[1].Set(vMax.x, vMin.y, vMin.z);
	vPs[2].Set(vMax.x, vMin.y, vMax.z);
	vPs[3] = vPs[0];
	vPs[4] = vPs[2];
	vPs[5].Set(vMin.x, vMin.y, vMax.z);
	for (int i = 0; i < 6; i++)
		m_pVertices[30 + i].Set(vPs[i], vN, fTUVs[i][0], fTUVs[i][1]);

	FindMinMax();
}

#ifdef _N3TOOL
void CN3Mesh::Create_Axis(float fLength)
{
	Create(12, 0);

	this->FindMinMax();

	// x 축
	//	m_pVertices[0].Set(-fLength/2.0f, 0, 0, 0, 1, 0, 0, 0);
	//	m_pVertices[1].Set( fLength/2.0f, 0, 0, 0, 1, 0, 0, 0);
	//	m_pVertices[2] = m_pVertices[1];
	//	m_pVertices[3] = m_pVertices[1];
	//	m_pVertices[3].x -= fLength / 20.0f;
	//	m_pVertices[3].y += fLength / 20.0f;

	m_pVertices[0].Set(0, 0, 0, 0, 1, 0, 0, 0);
	m_pVertices[1].Set(fLength, 0, 0, 0, 1, 0, 0, 0);
	m_pVertices[2]    = m_pVertices[1];
	m_pVertices[3]    = m_pVertices[1];
	m_pVertices[3].x -= fLength / 10.0f;
	m_pVertices[3].y += fLength / 10.0f;

	__Matrix44 mtx;

	// y 축
	mtx.RotationZ(__PI / -2.0f);
	int i;
	for (i = 0; i < 4; i++)
	{
		m_pVertices[4 + i].Set(m_pVertices[i] * mtx, m_pVertices[i].n * mtx, 0, 0);
	}

	// z 축
	mtx.RotationY(__PI / -2.0f);
	for (i = 0; i < 4; i++)
	{
		m_pVertices[8 + i].Set(m_pVertices[i] * mtx, m_pVertices[i].n * mtx, 0, 0);
	}
}
#endif // end of _N3TOOL

bool CN3Mesh::Import(CN3PMesh* pPMesh)
{
	if (nullptr == pPMesh)
		return false;
	int iNumIndices = pPMesh->GetMaxNumIndices();
	if (0 >= iNumIndices)
		return false;
	Release(); // 초기화

	// pmesh instance를 만들고 lod를 최고 상태로 조정
	CN3PMeshInstance PMeshInstance;
	PMeshInstance.Create(pPMesh);
	PMeshInstance.SetLODByNumVertices(pPMesh->GetMaxNumVertices());

	// vertex, index buffer 만들기
	Create(PMeshInstance.GetNumVertices(), PMeshInstance.GetNumIndices());

	// vertex index buffer 복사
	__VertexT1* pVertices = PMeshInstance.GetVertices();
	uint16_t* pIndices    = PMeshInstance.GetIndices();

	for (int i = 0; i < m_nVC; i++)
		m_pVertices[i] = pVertices[i];

	memcpy(m_psnIndices, pIndices, sizeof(uint16_t) * m_nIC);

	m_szName = pPMesh->m_szName; // 이름..
	return true;
}

#ifdef _N3TOOL
bool CN3Mesh::Import(CN3IMesh* pIMesh)
{
	if (nullptr == pIMesh)
		return false;
	__VertexT1* pvSrc = pIMesh->BuildVertexList();
	if (nullptr == pvSrc)
		return false;

	int nFC = pIMesh->FaceCount();
	this->Release();
	this->Create(nFC * 3, 0);

	memcpy(m_pVertices, pvSrc, sizeof(__VertexT1) * nFC * 3);

	__Vector3 v0, v1, v2, vN(0, 0, 0);
	for (int i = 0; i < nFC; i++) // Normal 값 다시 세팅..
	{
		v0 = m_pVertices[i * 3 + 0];
		v1 = m_pVertices[i * 3 + 1];
		v2 = m_pVertices[i * 3 + 2];

		vN.Cross(v1 - v0, v2 - v1); // Normal 값을 계산하고...
		vN.Normalize();

		m_pVertices[i * 3 + 0].n = vN;
		m_pVertices[i * 3 + 1].n = vN;
		m_pVertices[i * 3 + 2].n = vN;
	}

	m_szName = pIMesh->m_szName; // 이름..
	return true;
}

void CN3Mesh::ReGenerateSmoothNormal()
{
	if (m_nVC <= 0)
		return;

	int* pnNs = new int[m_nVC];
	memset(pnNs, 0, 4 * m_nVC);
	__Vector3* pvNs = new __Vector3[m_nVC];
	memset(pvNs, 0, sizeof(__Vector3) * m_nVC);

	int nFC = 0;
	if (m_nIC > 0)
		nFC = m_nIC / 3;
	else
		nFC = m_nVC / 3;

	__Vector3 v0, v1, v2, vN(0, 0, 0);
	for (int i = 0; i < m_nVC; i++)
	{
		for (int j = 0; j < nFC; j++)
		{
			if (m_nIC > 0)
			{
				v0 = m_pVertices[m_psnIndices[j * 3 + 0]];
				v1 = m_pVertices[m_psnIndices[j * 3 + 1]];
				v2 = m_pVertices[m_psnIndices[j * 3 + 2]];
			}
			else
			{
				v0 = m_pVertices[j * 3 + 0];
				v1 = m_pVertices[j * 3 + 1];
				v2 = m_pVertices[j * 3 + 2];
			}

			if (m_pVertices[i] == v0 || m_pVertices[i] == v1 || m_pVertices[i] == v2)
			{
				vN.Cross(v1 - v0, v2 - v1); // Normal 값을 계산하고...
				vN.Normalize();

				pnNs[i]++;
				pvNs[i] += vN;
			}
		}

		m_pVertices[i].n = pvNs[i] / (float) pnNs[i];
	}

	delete[] pnNs;
	delete[] pvNs;
}
#endif // end of _N3TOOL

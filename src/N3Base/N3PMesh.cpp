// N3PMesh.cpp: implementation of the CN3PMesh class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfxBase.h"
#include "N3PMesh.h"
#include "N3PMeshInstance.h"

CN3PMesh::CN3PMesh()
{
	m_dwType             |= OBJ_MESH_PROGRESSIVE;

	m_pVertices           = nullptr;
	m_pVertices2          = nullptr;
	m_pIndices            = nullptr;
	m_pCollapses          = nullptr;
	m_pAllIndexChanges    = nullptr;
	m_pLODCtrlValues      = nullptr;

	m_iNumCollapses       = 0;
	m_iTotalIndexChanges  = 0;
	m_iMaxNumVertices     = 0;
	m_iMaxNumIndices      = 0;
	m_iMinNumVertices     = 0;
	m_iMinNumIndices      = 0;
	m_iLODCtrlValueCount  = 0;

	m_vMin.Set(0, 0, 0);
	m_vMax.Set(0, 0, 0);
	m_fRadius = 0.0f;
}

CN3PMesh::~CN3PMesh()
{
	delete[] m_pVertices;
	m_pVertices = nullptr;
	delete[] m_pVertices2;
	m_pVertices2 = nullptr;
	delete[] m_pIndices;
	m_pIndices = nullptr;
	delete[] m_pCollapses;
	m_pCollapses = nullptr;
	delete[] m_pAllIndexChanges;
	m_pAllIndexChanges = nullptr;
	delete[] m_pLODCtrlValues;
	m_pLODCtrlValues = nullptr;
}

void CN3PMesh::Release()
{
	if (m_pVertices)
	{
		delete[] m_pVertices;
		m_pVertices = nullptr;
	}
	if (m_pVertices2)
	{
		delete[] m_pVertices2;
		m_pVertices2 = nullptr;
	}
	if (m_pIndices)
	{
		delete[] m_pIndices;
		m_pIndices = nullptr;
	}
	if (m_pCollapses)
	{
		delete[] m_pCollapses;
		m_pCollapses = nullptr;
	}
	if (m_pAllIndexChanges)
	{
		delete[] m_pAllIndexChanges;
		m_pAllIndexChanges = nullptr;
	}
	if (m_pLODCtrlValues)
	{
		delete[] m_pLODCtrlValues;
		m_pLODCtrlValues = nullptr;
	}

	m_iNumCollapses      = 0;
	m_iTotalIndexChanges = 0;
	m_iMaxNumVertices    = 0;
	m_iMaxNumIndices     = 0;
	m_iMinNumVertices    = 0;
	m_iMinNumIndices     = 0;
	m_iLODCtrlValueCount = 0;

	m_vMin.Set(0, 0, 0);
	m_vMax.Set(0, 0, 0);
	m_fRadius = 0.0f;

	CN3BaseFileAccess::Release();
}

bool CN3PMesh::Load(File& file)
{
	CN3BaseFileAccess::Load(file);

	file.Read(&m_iNumCollapses, sizeof(m_iNumCollapses));
	file.Read(&m_iTotalIndexChanges, sizeof(m_iTotalIndexChanges));

	file.Read(&m_iMaxNumVertices, sizeof(int));
	file.Read(&m_iMaxNumIndices, sizeof(int));
	file.Read(&m_iMinNumVertices, sizeof(int));
	file.Read(&m_iMinNumIndices, sizeof(int));

	HRESULT hr = Create(m_iMaxNumVertices, m_iMaxNumIndices);
	__ASSERT(SUCCEEDED(hr), "Failed to create progressive mesh");

	if (m_iMaxNumVertices > 0)
	{
		//		m_pVertices = new __VertexT1[m_iMaxNumVertices];
		file.Read(m_pVertices, m_iMaxNumVertices * sizeof(__VertexT1));
	}

	if (m_iMaxNumIndices > 0)
	{
		//		m_pIndices = new uint16_t[m_iMaxNumIndices];
		file.Read(m_pIndices, m_iMaxNumIndices * sizeof(uint16_t));
	}

	if (m_iNumCollapses > 0)
	{
		m_pCollapses = new __EdgeCollapse
			[m_iNumCollapses
				+ 1]; // +1을 한 이유 : PMeshInstance::SplitOne() 함수에서 부득이하게 포인터가 경계선을 가르키게 해야 하는 경우가 있어서.
		file.Read(m_pCollapses, m_iNumCollapses * sizeof(__EdgeCollapse));
		ZeroMemory(m_pCollapses + m_iNumCollapses,
			sizeof(
				__EdgeCollapse)); // 위의 +1을 한이유와 같음. 만약을 대비해 마지막 데이타를 초기화 해둠

		bool bFixed = false;
		for (int i = 0; i < m_iNumCollapses; i++)
		{
			if (m_pCollapses[i].iIndexChanges < 0)
			{
				m_pCollapses[i].iIndexChanges = 0;
				bFixed                        = true;
			}
		}
#ifdef _DEBUG
		if (bFixed)
			::MessageBox(s_hWndBase, "잘못된 Progressive Mesh 수정", m_szName.c_str(), MB_OK);
#endif
	}
	if (m_iTotalIndexChanges > 0)
	{
		m_pAllIndexChanges = new int[m_iTotalIndexChanges];
		file.Read(m_pAllIndexChanges, m_iTotalIndexChanges * sizeof(int));
	}

	__ASSERT(m_pLODCtrlValues == nullptr && m_iLODCtrlValueCount == 0,
		"Invalid Level of detail control value");
	file.Read(&m_iLODCtrlValueCount, sizeof(m_iLODCtrlValueCount));
	if (m_iLODCtrlValueCount > 0)
	{
		m_pLODCtrlValues = new __LODCtrlValue[m_iLODCtrlValueCount];
		file.Read(m_pLODCtrlValues, m_iLODCtrlValueCount * sizeof(__LODCtrlValue));
	}

	FindMinMax();

	return true;
}

#ifdef _N3TOOL
bool CN3PMesh::Save(File& file)
{
	CN3BaseFileAccess::Save(file);

	file.Write(&m_iNumCollapses, sizeof(m_iNumCollapses));
	file.Write(&m_iTotalIndexChanges, sizeof(m_iTotalIndexChanges));

	file.Write(&(m_iMaxNumVertices), sizeof(int));
	file.Write(&(m_iMaxNumIndices), sizeof(int));
	file.Write(&(m_iMinNumVertices), sizeof(int));
	file.Write(&(m_iMinNumIndices), sizeof(int));

	if (m_iMaxNumVertices > 0)
		file.Write(m_pVertices, m_iMaxNumVertices * sizeof(__VertexT1));
	if (m_iMaxNumIndices > 0)
		file.Write(m_pIndices, m_iMaxNumIndices * sizeof(uint16_t));

	if (m_iNumCollapses > 0)
	{
		for (int i = 0; i < m_iNumCollapses; i++)
			if (m_pCollapses[i].iIndexChanges < 0)
				m_pCollapses[i].iIndexChanges = 0; // 저장..
		file.Write(m_pCollapses, m_iNumCollapses * sizeof(__EdgeCollapse));
	}
	if (m_iTotalIndexChanges > 0)
		file.Write(m_pAllIndexChanges, m_iTotalIndexChanges * sizeof(m_pAllIndexChanges[0]));

	file.Write(&m_iLODCtrlValueCount, sizeof(m_iLODCtrlValueCount));
	if (m_iLODCtrlValueCount > 0)
		file.Write(m_pLODCtrlValues, m_iLODCtrlValueCount * sizeof(__LODCtrlValue));

	return true;
}
#endif // end of _N3TOOL

void CN3PMesh::FindMinMax()
{
	if (m_iMaxNumVertices <= 0)
	{
		m_vMin.Zero();
		m_vMax.Zero();
		m_fRadius = 0;
		return;
	}

	// 최소, 최대 점을 찾는다.
	m_vMin.Set(FLT_MAX, FLT_MAX, FLT_MAX);
	m_vMax.Set(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (int i = 0; i < m_iMaxNumVertices; i++)
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

#ifdef _N3TOOL
void CN3PMesh::CopyMesh(CN3PMesh* pSrcPMesh)
{
	Release();

	HRESULT hr;

	__ASSERT(pSrcPMesh, "Source progressive mesh pointer is NULL");

	m_iNumCollapses      = pSrcPMesh->m_iNumCollapses;
	m_iTotalIndexChanges = pSrcPMesh->m_iTotalIndexChanges;
	m_iMaxNumVertices    = pSrcPMesh->m_iMaxNumVertices;
	m_iMaxNumIndices     = pSrcPMesh->m_iMaxNumIndices;
	m_iMinNumVertices    = pSrcPMesh->m_iMinNumVertices;
	m_iMinNumIndices     = pSrcPMesh->m_iMinNumIndices;
	m_vMin               = pSrcPMesh->m_vMin;
	m_vMax               = pSrcPMesh->m_vMax;
	m_fRadius            = pSrcPMesh->m_fRadius;
	m_iLODCtrlValueCount = pSrcPMesh->m_iLODCtrlValueCount;

	if (m_iTotalIndexChanges > 0)
	{
		m_pAllIndexChanges = new int[m_iTotalIndexChanges];

		CopyMemory(
			m_pAllIndexChanges, pSrcPMesh->m_pAllIndexChanges, sizeof(int) * m_iTotalIndexChanges);
	}

	if (m_iNumCollapses > 0)
	{
		m_pCollapses = new __EdgeCollapse
			[m_iNumCollapses
				+ 1]; // +1을 한 이유 : PMeshInstance::SplitOne() 함수에서 부득이하게 포인터가 경계선을 가르키게 해야 하는 경우가 있어서.
		CopyMemory(m_pCollapses, pSrcPMesh->m_pCollapses, sizeof(__EdgeCollapse) * m_iNumCollapses);
		ZeroMemory(m_pCollapses + m_iNumCollapses,
			sizeof(
				__EdgeCollapse)); // 위의 +1을 한이유와 같음. 만약을 대비해 마지막 데이타를 초기화 해둠
	}

	hr = Create(m_iMaxNumVertices, m_iMaxNumIndices);
	__ASSERT(SUCCEEDED(hr), "Failed to create progressive mesh");

	if (m_iMaxNumVertices > 0)
	{
		//		m_pVertices = new __VertexT1[m_iMaxNumVertices];
		CopyMemory(m_pVertices, pSrcPMesh->m_pVertices, sizeof(__VertexT1) * m_iMaxNumVertices);
	}

	if (m_iMaxNumIndices > 0)
	{
		//		m_pIndices = new uint16_t[m_iMaxNumIndices];
		CopyMemory(m_pIndices, pSrcPMesh->m_pIndices, sizeof(uint16_t) * m_iMaxNumIndices);
	}

	if (m_iLODCtrlValueCount > 0)
	{
		m_pLODCtrlValues = new __LODCtrlValue[m_iLODCtrlValueCount];
		CopyMemory(m_pLODCtrlValues, pSrcPMesh->m_pLODCtrlValues,
			sizeof(__LODCtrlValue) * m_iLODCtrlValueCount);
	}

	m_szName = pSrcPMesh->m_szName;
}
#endif // end of _N3TOOL

HRESULT CN3PMesh::Create(int iNumVertices, int iNumIndices)
{
	m_iMaxNumVertices = iNumVertices;
	m_iMaxNumIndices  = iNumIndices;

	if (m_pVertices)
	{
		delete m_pVertices;
		m_pVertices = nullptr;
	}
	if (m_pIndices)
	{
		delete m_pIndices;
		m_pIndices = nullptr;
	}

	if (m_iMaxNumVertices > 0)
	{
		m_pVertices = new __VertexT1[m_iMaxNumVertices];
	}
	if (m_iMaxNumIndices > 0)
	{
		m_pIndices = new uint16_t[m_iMaxNumIndices];
	}

	return S_OK;
}

HRESULT CN3PMesh::GenerateSecondUV()
{
	if (m_pVertices2)
	{
		delete m_pVertices2;
		m_pVertices2 = nullptr;
	}

	if (m_iMaxNumVertices > 0)
	{
		m_pVertices2 = new __VertexT2[m_iMaxNumVertices];

		for (int i = 0; i < m_iMaxNumVertices; i++)
		{
			auto& dst       = m_pVertices2[i];
			const auto& src = m_pVertices[i];

			dst.x           = src.x;
			dst.y           = src.y;
			dst.z           = src.z;
			dst.n.x         = src.n.x;
			dst.n.y         = src.n.y;
			dst.n.z         = src.n.z;
			dst.tu          = src.tu;
			dst.tv          = src.tv;
			dst.tu2         = src.tu;
			dst.tv2         = src.tv;
		}
	}

	return S_OK;
}

#ifdef _N3TOOL
void CN3PMesh::LODCtrlSet(__LODCtrlValue* pLODCtrls, int nCount)
{
	m_iLODCtrlValueCount = 0;
	delete[] m_pLODCtrlValues;
	m_pLODCtrlValues = nullptr;
	if (nullptr == pLODCtrls || nCount <= 0)
		return;

	m_iLODCtrlValueCount = nCount;
	if (nCount > 0)
	{
		m_pLODCtrlValues = new __LODCtrlValue[nCount];
		memcpy(m_pLODCtrlValues, pLODCtrls, sizeof(__LODCtrlValue) * nCount);

		// 거리에 따라 정렬
		qsort(m_pLODCtrlValues, m_iLODCtrlValueCount, sizeof(__LODCtrlValue), SortByDistance);
	}
}
#endif // end of _N3TOOL

#ifdef _N3TOOL
int CN3PMesh::SortByDistance(const void* pArg1, const void* pArg2)
{
	__LODCtrlValue* pObj1 = (__LODCtrlValue*) pArg1;
	__LODCtrlValue* pObj2 = (__LODCtrlValue*) pArg2;

	if (pObj1->fDist < pObj2->fDist)
		return -1;
	else if (pObj1->fDist > pObj2->fDist)
		return 1;
	else
		return 0;
}
#endif // end of _N3TOOL

#ifdef _N3TOOL
void CN3PMesh::ReGenerateSmoothNormal()
{
	if (m_iMaxNumVertices <= 0)
		return;

	CN3PMeshInstance PMI(this);
	PMI.SetLODByNumVertices(m_iMaxNumVertices); // 최대 점으로 세팅하고..
	int nIC             = PMI.GetNumIndices();  // Index Count...
	uint16_t* pwIndices = PMI.GetIndices();     // Index ...

	int* pnNs           = new int[m_iMaxNumVertices];
	memset(pnNs, 0, 4 * m_iMaxNumVertices);
	__Vector3* pvNs = new __Vector3[m_iMaxNumVertices];
	memset(pvNs, 0, sizeof(__Vector3) * m_iMaxNumVertices);

	int nFC = nIC / 3;

	__Vector3 v0, v1, v2, vN(0, 0, 0);
	for (int i = 0; i < m_iMaxNumVertices; i++)
	{
		for (int j = 0; j < nFC; j++)
		{
			v0 = m_pVertices[pwIndices[j * 3 + 0]];
			v1 = m_pVertices[pwIndices[j * 3 + 1]];
			v2 = m_pVertices[pwIndices[j * 3 + 2]];

			if (m_pVertices[i] == v0 || m_pVertices[i] == v1 || m_pVertices[i] == v2)
			{
				vN.Cross(v1 - v0, v2 - v1); // Normal 값을 계산하고...
				vN.Normalize();

				pnNs[i]++;
				pvNs[i] += vN;
			}
		}

		if (pnNs[i] > 0)
			m_pVertices[i].n = pvNs[i] / (float) pnNs[i];
	}

	delete[] pnNs;
	delete[] pvNs;
}
#endif // end of _N3TOOL

#ifdef _N3TOOL
void CN3PMesh::ReGenerateSharpNormal()
{
	if (m_iMaxNumVertices <= 0)
		return;

	CN3PMeshInstance PMI(this);
	PMI.SetLODByNumVertices(m_iMaxNumVertices); // 최대 점으로 세팅하고..
	int nIC             = PMI.GetNumIndices();  // Index Count...
	uint16_t* pwIndices = PMI.GetIndices();     // Index ...

	int nFC             = nIC / 3;
	__Vector3 v0, v1, v2, vN(0, 0, 0);
	for (int j = 0; j < nFC; j++)
	{
		v0 = m_pVertices[pwIndices[j * 3 + 0]];
		v1 = m_pVertices[pwIndices[j * 3 + 1]];
		v2 = m_pVertices[pwIndices[j * 3 + 2]];

		vN.Cross(v1 - v0, v2 - v1); // Normal 값을 계산하고...
		vN.Normalize();

		m_pVertices[pwIndices[j * 3 + 0]].n = vN;
		m_pVertices[pwIndices[j * 3 + 1]].n = vN;
		m_pVertices[pwIndices[j * 3 + 2]].n = vN;
	}
}
#endif // end of _N3TOOL

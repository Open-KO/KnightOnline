// N3Skin.cpp: implementation of the CN3Skin class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfxBase.h"
#include "N3Skin.h"

CN3Skin::CN3Skin()
{
	m_dwType        |= OBJ_SKIN;

	m_pSkinVertices  = nullptr;
}

CN3Skin::~CN3Skin()
{
	delete[] m_pSkinVertices;
	m_pSkinVertices = nullptr;
}

void CN3Skin::Release()
{
	delete[] m_pSkinVertices;
	m_pSkinVertices = nullptr;

	CN3IMesh::Release();
}

bool CN3Skin::Load(File& file)
{
	CN3IMesh::Load(file);

	for (int i = 0; i < m_nVC; i++)
	{
		__VertexSkinned* pVtx = &m_pSkinVertices[i];
		file.Read(&pVtx->vOrigin, sizeof(__Vector3));
		file.Read(&pVtx->nAffect, sizeof(int));

		// Skip the useless explicitly-32-bit pointers pnJoints & pfWeights.
		file.Seek(8, SEEK_CUR);

		pVtx->pnJoints  = nullptr;
		pVtx->pfWeights = nullptr;

		int nAffect     = pVtx->nAffect;
		if (nAffect > 1)
		{
			pVtx->pnJoints  = new int[nAffect];
			pVtx->pfWeights = new float[nAffect];

			file.Read(pVtx->pnJoints, 4 * nAffect);
			file.Read(pVtx->pfWeights, 4 * nAffect);
		}
		else if (nAffect == 1)
		{
			pVtx->pnJoints = new int[1];
			file.Read(pVtx->pnJoints, 4);
		}
	}

	return true;
}

#ifdef _N3TOOL
bool CN3Skin::Save(File& file)
{
	CN3IMesh::Save(file);

	uint32_t unused = 0;
	for (int i = 0; i < m_nVC; i++)
	{
		__VertexSkinned* pVtx = &m_pSkinVertices[i];
		file.Write(&pVtx->vOrigin, sizeof(__Vector3));
		file.Write(&pVtx->nAffect, sizeof(int));

		// Skip the useless pointers pnJoints, pfWeights (assume they're 32-bit).
		file.Write(&unused, sizeof(uint32_t));
		file.Write(&unused, sizeof(uint32_t));

		int nAffect = pVtx->nAffect;
		if (nAffect > 1)
		{
			file.Write(pVtx->pnJoints, 4 * nAffect);
			file.Write(pVtx->pfWeights, 4 * nAffect);
		}
		else if (nAffect == 1)
		{
			file.Write(pVtx->pnJoints, 4);
		}
	}

	return true;
}
#endif // end of _N3TOOL

bool CN3Skin::Create(int nFC, int nVC, int nUVC)
{
	if (!CN3IMesh::Create(nFC, nVC, nUVC))
		return false;

	delete[] m_pSkinVertices;
	m_pSkinVertices = new __VertexSkinned[nVC] {};

	return true;
}

#ifdef _N3TOOL
int CN3Skin::SortWeightsProc(const void* pArg1, const void* pArg2)
{
	__Weight* pW1 = (__Weight*) pArg1;
	__Weight* pW2 = (__Weight*) pArg2;

	if (pW1->fWeight < pW2->fWeight)
		return 1;
	else
		return -1;
}

void CN3Skin::RecalcWeight()
{
	if (nullptr == m_pSkinVertices)
		return;
	int i, j;
	for (i = 0; i < m_nVC; ++i)
	{
		__VertexSkinned* pVtx = m_pSkinVertices + i;
		if (1 >= pVtx->nAffect)
			continue;
		float fSum = 0;
		for (j = 0; j < pVtx->nAffect; ++j)
			fSum += pVtx->pfWeights[j];
		for (j = 0; j < pVtx->nAffect; ++j)
			pVtx->pfWeights[j] /= fSum;
	}
}

#endif // end of _N3TOOL

bool CN3Skin::CheckCollisionPrecisely(
	const __Vector3& vPos, const __Vector3& vDir, __Vector3* pvPick)
{
	uint16_t* pwIs         = nullptr;
	__VertexXyzNormal* pVs = nullptr;
	int nFC = 0, nCI0 = 0, nCI1 = 0, nCI2 = 0;

	pVs  = Vertices();
	pwIs = VertexInices();

	if (pVs == nullptr || pwIs == nullptr)
		return false;

	nFC = FaceCount();

	// 각각의 Face 마다 충돌체크..
	for (int j = 0; j < nFC; j++)
	{
		nCI0 = pwIs[j * 3 + 0];
		nCI1 = pwIs[j * 3 + 1];
		nCI2 = pwIs[j * 3 + 2];

		if (!::_IntersectTriangle(vPos, vDir, pVs[nCI0], pVs[nCI1], pVs[nCI2]))
			continue;

		if (pvPick != nullptr)
		{
			float fT = 0.0f, fU = 0.0f, fV = 0.0f;
			::_IntersectTriangle(vPos, vDir, pVs[nCI0], pVs[nCI1], pVs[nCI2], fT, fU, fV, pvPick);
			//			(*pvPick) *= m_Mtx;
		}

		return true;
	}

	return false;
}

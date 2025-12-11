// N3Skin.cpp: implementation of the CN3Skin class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfxBase.h"
#include "N3Skin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CN3Skin::CN3Skin()
{
	m_dwType |= OBJ_SKIN;

	m_pSkinVertices = nullptr;
}

CN3Skin::~CN3Skin()
{
	delete [] m_pSkinVertices; m_pSkinVertices = nullptr;
}

void CN3Skin::Release()
{
	delete [] m_pSkinVertices; m_pSkinVertices = nullptr;

	CN3IMesh::Release();
}

bool CN3Skin::Load(HANDLE hFile)
{
	CN3IMesh::Load(hFile);

	DWORD dwRWC = 0;
	for (int i = 0; i < m_nVC; i++)
	{
		__VertexSkinned* pVtx = &m_pSkinVertices[i];
		ReadFile(hFile, &pVtx->vOrigin, sizeof(__Vector3), &dwRWC, nullptr);
		ReadFile(hFile, &pVtx->nAffect, sizeof(int), &dwRWC, nullptr);
		// Skip the useless pointers pnJoints, pfWeights (assume they're 32-bit).
		SetFilePointer(hFile, 8, nullptr, FILE_CURRENT);

		pVtx->pnJoints = nullptr;
		pVtx->pfWeights = nullptr;

		int nAffect = pVtx->nAffect;
		if (nAffect > 1)
		{
			pVtx->pnJoints = new int[nAffect];
			pVtx->pfWeights = new float[nAffect];

			ReadFile(hFile, pVtx->pnJoints, 4 * nAffect, &dwRWC, nullptr);
			ReadFile(hFile, pVtx->pfWeights, 4 * nAffect, &dwRWC, nullptr);
		}
		else if (nAffect == 1)
		{
			pVtx->pnJoints = new int[1];
			ReadFile(hFile, pVtx->pnJoints, 4, &dwRWC, nullptr);
		}
	}

	return true;
}

#ifdef _N3TOOL
bool CN3Skin::Save(HANDLE hFile)
{
	CN3IMesh::Save(hFile);

	DWORD dwRWC = 0, dwUnused = 0;
	for (int i = 0; i < m_nVC; i++)
	{
		__VertexSkinned* pVtx = &m_pSkinVertices[i];
		WriteFile(hFile, &pVtx->vOrigin, sizeof(__Vector3), &dwRWC, nullptr);
		WriteFile(hFile, &pVtx->nAffect, sizeof(int), &dwRWC, nullptr);

		// Skip the useless pointers pnJoints, pfWeights (assume they're 32-bit).
		WriteFile(hFile, &dwUnused, sizeof(DWORD), &dwRWC, nullptr);
		WriteFile(hFile, &dwUnused, sizeof(DWORD), &dwRWC, nullptr);

		int nAffect = pVtx->nAffect;
		if (nAffect > 1)
		{
			WriteFile(hFile, pVtx->pnJoints, 4 * nAffect, &dwRWC, nullptr);
			WriteFile(hFile, pVtx->pfWeights, 4 * nAffect, &dwRWC, nullptr);
		}
		else if (nAffect == 1)
		{
			WriteFile(hFile, pVtx->pnJoints, 4, &dwRWC, nullptr);
		}
	}

	return true;
}
#endif // end of _N3TOOL

bool CN3Skin::Create(int nFC, int nVC, int nUVC)
{
	if(false == CN3IMesh::Create(nFC, nVC, nUVC)) return false;

	delete [] m_pSkinVertices;
	m_pSkinVertices = new __VertexSkinned[nVC];
	memset(m_pSkinVertices, 0, sizeof(__VertexSkinned)*nVC);

	return true;
}

#ifdef _N3TOOL
int CN3Skin::SortWeightsProc(const void *pArg1, const void *pArg2)
{
	__Weight* pW1 = (__Weight*)pArg1;
	__Weight* pW2 = (__Weight*)pArg2;

	if(pW1->fWeight < pW2->fWeight) return 1;
	else return -1;
}

void CN3Skin::RecalcWeight()
{
	if (nullptr == m_pSkinVertices) return;
	int i, j;
	for (i=0; i<m_nVC; ++i)
	{
		__VertexSkinned* pVtx = m_pSkinVertices + i;
		if (1 >= pVtx->nAffect) continue;
		float fSum = 0;
		for (j=0; j<pVtx->nAffect; ++j) fSum += pVtx->pfWeights[j];
		for (j=0; j<pVtx->nAffect; ++j) pVtx->pfWeights[j] /= fSum;
	}
}

#endif // end of _N3TOOL

bool CN3Skin::CheckCollisionPrecisely(const __Vector3 &vPos, const __Vector3 &vDir, __Vector3 *pvPick)
{
	uint16_t* pwIs;
	__VertexXyzNormal* pVs;
	int nFC, nCI0, nCI1, nCI2;

	pVs	 = Vertices();
	pwIs = VertexInices();

	if(pVs == nullptr || pwIs == nullptr) return false;

	nFC = FaceCount();
	for(int j = 0; j < nFC; j++) // 각각의 Face 마다 충돌체크..
	{
		nCI0 = pwIs[j*3+0];
		nCI1 = pwIs[j*3+1];
		nCI2 = pwIs[j*3+2];

		if(false == ::_IntersectTriangle(vPos, vDir, pVs[nCI0], pVs[nCI1], pVs[nCI2])) continue;
		
		if(pvPick)
		{
			float fT, fU, fV;
			::_IntersectTriangle(vPos, vDir, pVs[nCI0], pVs[nCI1], pVs[nCI2], fT, fU, fV, pvPick);
//			(*pvPick) *= m_Mtx;
		}
		return true;
	}

	return false;

}

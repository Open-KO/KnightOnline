// N3MeshIndexed.h: interface for the CN3IMesh class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3IMesh_h__INCLUDED_)
#define AFX_N3IMesh_h__INCLUDED_

#pragma once

#include "N3BaseFileAccess.h"

const int MAX_IMESH_BUFFER = 10000;

class CN3IMesh : public CN3BaseFileAccess
{
protected:
	static __VertexT1 s_Vertices[MAX_IMESH_BUFFER];
	static __VertexT2 s_Vertices2[MAX_IMESH_BUFFER];

	int m_nFC;                // 삼각형 갯수

	int m_nVC;                // Vertex Count
	uint16_t* m_pwVtxIndices; // 점 인덱스 리스트.
	int m_nUVC;               // UV 좌표 Count
	uint16_t* m_pwUVsIndices; // 텍스처 좌표 인덱스 리스트.

	float* m_pfUVs;           // UV Data - m_nUVC * 2 만큼 할당한다.
							  //	LPDIRECT3DVERTEXBUFFER8 m_lpVB;

	__Vector3 m_vMin, m_vMax; // 최소, 최대점.. 변하긴 하지만 대략적으로...

public:
#ifdef _N3TOOL
	void ApplyOffset(__Vector3 vOffset);
	void ReGenerateSmoothNormal();
	void RenderSelected();
#endif // end of _N3TOOL
	__Vector3 Min()
	{
		return m_vMin;
	}
	__Vector3 Max()
	{
		return m_vMax;
	}
	void FindMinMax();
	void Render(bool bUseTwoUV = false);
	__VertexT1*
	BuildVertexList(); // Vertex Buffer 에다가 점을 만든다.. 동시에 두번 사용하지 않도록 주의가 필요
	__VertexT2*
	BuildVertexListTwoUV(); // Vertex Buffer 에다가 점을 만든다.. 동시에 두번 사용하지 않도록 주의가 필요

	int FaceCount()
	{
		return m_nFC;
	}
	int VertexCount()
	{
		return m_nVC;
	}

	__VertexXyzNormal* m_pVertices; // 좌표 Data

	__VertexXyzNormal* Vertices()
	{
		return m_pVertices;
	}
	uint16_t* VertexInices()
	{
		return m_pwVtxIndices;
	}
	int UVCount()
	{
		return m_nUVC;
	}
#ifdef _N3TOOL
	void VertexIndexSet(int index, int nVI)
	{
		if (index >= 0 && index < m_nFC * 3)
			m_pwVtxIndices[index] = nVI;
	}
	void UVSet(int index, float fU, float fV)
	{
		if (index >= 0 && index < m_nUVC)
		{
			m_pfUVs[index * 2 + 0] = fU;
			m_pfUVs[index * 2 + 1] = fV;
		}
	}
	void UVIndexSet(int index, int nUVI)
	{
		if (index >= 0 && index < m_nFC * 3)
			m_pwUVsIndices[index] = nUVI;
	}
#endif                                               // end of _N3TOOL

	virtual bool Create(int nFC, int nVC, int nUVC); // 차례대로  Face Count, VertexCount, UV Count
	bool Load(File& file) override;
#ifdef _N3TOOL
	bool Save(File& file) override;
#endif // end of _N3TOOL

	void Release() override;
	CN3IMesh();
	~CN3IMesh() override;

//	By : Ecli666 ( On 2002-07-23 오후 5:31:41 )
//
#ifdef _N3GAME
	void TickForShadow(bool bUseTwoUV = false);
	int GetVertexCount();
	__Vector3* GetVertexData();

private:
	__Vector3* m_pVertexT1;
#endif

	//	~(By Ecli666 On 2002-07-23 오후 5:31:41 )
};

#endif // !defined(AFX_N3IMesh_h__INCLUDED_)

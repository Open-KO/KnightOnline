// N3VMesh.h: interface for the C3DObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3VMesh_h__INCLUDED_)
#define AFX_N3VMesh_h__INCLUDED_

#pragma once

#include "N3BaseFileAccess.h"

class CN3IMesh;

class CN3VMesh : public CN3BaseFileAccess
{
protected:
	__Vector3* m_pVertices; // 점 버퍼
	int m_nVC;              // 점 갯수

	uint16_t* m_pwIndices;  // Index...
	int m_nIC;              // Index Count

	__Vector3 m_vMin;
	__Vector3 m_vMax;
	float m_fRadius; // 반지름

public:
	__Vector3 Min()
	{
		return m_vMin;
	}
	__Vector3 Max()
	{
		return m_vMax;
	}
	bool Pick(const __Matrix44& MtxWorld, const __Vector3& vPos, const __Vector3& vDir,
		__Vector3* pVCol = nullptr, __Vector3* pVNormal = nullptr);
	bool CheckCollision(const __Matrix44& MtxWorld, const __Vector3& v0, const __Vector3& v1,
		__Vector3* pVCol    = nullptr,
		__Vector3* pVNormal = nullptr); // Local 좌표를 통한 충돌 체크 판정..
	float Radius()
	{
		return m_fRadius;
	}
	void FindMinMax();
	void Render(D3DCOLOR crLine);
	void CreateVertices(int nVC);
	void CreateIndex(int nIC);
	void CreateCube(const __Vector3& vMin, const __Vector3& vMax);

	int VertexCount()
	{
		return m_nVC;
	}
	__Vector3* Vertices()
	{
		return m_pVertices;
	}

	int IndexCount()
	{
		return m_nIC;
	}
	uint16_t* Indices()
	{
		return m_pwIndices;
	}

	bool Load(File& file) override;
#ifdef _N3TOOL
	bool Import(CN3IMesh* pIMesh);
	bool Save(File& file) override;
#endif // end of _N3TOOL

	void Release() override;

	CN3VMesh();
	~CN3VMesh() override;

	//	By : Ecli666 ( On 2002-08-09 오후 2:47:59 )
	//
	int GetColIndexByiOrder(int iOrder);
	__Vector3 GetColVertexByIndex(int iIndex);
	void PartialColRender(int iCount, int* piIndices);
	void PartialGetCollision(int iIndex, __Vector3& vec);
	//	~(By Ecli666 On 2002-08-09 오후 2:47:59 )
};

#endif // !defined(AFX_N3VMesh_h__INCLUDED_)

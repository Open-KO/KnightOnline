// N3MeshNormal.h: interface for the C3DObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3Mesh_h__INCLUDED_)
#define AFX_N3Mesh_h__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "N3BaseFileAccess.h"

class CN3IMesh;
class CN3PMesh;

class CN3Mesh : public CN3BaseFileAccess
{
protected:
	__VertexT1*	m_pVertices; // 점 버퍼
	uint16_t*		m_psnIndices; // 인덱스 버퍼

	int		m_nVC; // 점 갯수
	int		m_nIC; // 점 갯수
	__Vector3 m_vMin, m_vMax;

public:
	__Vector3 Min() { return m_vMin; }
	__Vector3 Max() { return m_vMax; }
	void	FindMinMax();
	
	int			VertexCount() { return m_nVC; }
	__VertexT1*	Vertices() { return m_pVertices; }
	int			IndexCount() { return m_nIC; }
	uint16_t*		Indices() { return m_psnIndices; } // 인덱스 버퍼

	bool	Load(HANDLE hFile);

	void	Create(int nVC, int nIC);
	void	Create_Cube(const __Vector3& vMin, const __Vector3& vMax);

	bool	Import(CN3PMesh* pPMesh);

#ifdef _N3TOOL
	bool	Import(CN3IMesh* pIMesh);	
	bool	Save(HANDLE hFile);
	void	Create_Axis(float fLength);
	void	MakeIndexed();
	void	ReGenerateSmoothNormal();
#endif // end of _N3TOOL
	
	void	Release();
	void	ReleaseVertices();
	void	ReleaseIndices();

	CN3Mesh();
	virtual ~CN3Mesh();
};

#endif // !defined(AFX_N3Mesh_h__INCLUDED_)

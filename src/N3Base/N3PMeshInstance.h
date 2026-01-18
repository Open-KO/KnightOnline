// N3PMeshInstance.h: interface for the CN3PMeshInstance class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3PMeshInstance_h__INCLUDED_)
#define AFX_N3PMeshInstance_h__INCLUDED_

#pragma once

#include "N3Base.h"
#include "N3PMesh.h"

#include <string>

// Every instance of the mesh in the game needs one of these structures. They
// encode the level of detail being used to render at the moment.
class CN3PMeshInstance : public CN3Base
{
public:
	CN3PMeshInstance();
	CN3PMeshInstance(CN3PMesh* pN3PMesh);
	CN3PMeshInstance(const std::string& szFN);
	~CN3PMeshInstance() override;

protected:
	uint16_t* m_pIndices;

	int m_iNumIndices, m_iNumVertices; // number of triangles/vertices to use at this moment.

	CN3PMesh::__EdgeCollapse* m_pCollapseUpTo; // 참조 포인터

	// A pointer to the original mesh. This is where you'll find the vertex data (which
	// is invariant between PMRInstance's)
	CN3PMesh* m_pPMesh; // 참조 포인터

public:
	bool IsLOD() const
	{
		return (m_pCollapseUpTo != nullptr);
	}

	bool Create(CN3PMesh* pN3Mesh);
	bool Create(const std::string& szFN);
	void Release() override;
	void Render();
	void RenderTwoUV();

	// The value you pass into this could be based on distance from camera, frame rate, etc
	void SetLOD(float value); // value = Camera Distance / Camera FOV;

	// Or you can choose to collapse/split one step at a time. These return false if there was
	// no more collapsing/splitting to do.
	bool CollapseOne();
	bool SplitOne();

	void SetLODByNumVertices(int iNumVertices);

	int GetNumVertices() const
	{
		return m_iNumVertices;
	}

	int GetNumIndices() const
	{
		return m_iNumIndices;
	}

	CN3PMesh* GetMesh()
	{
		return m_pPMesh;
	}

	const CN3PMesh* GetMesh() const
	{
		return m_pPMesh;
	}

	__VertexT1* GetVertices() const;

	uint16_t* GetIndices() const
	{
		return m_pIndices;
	}

	//	By : Ecli666 ( On 2002-08-06 오후 4:33:04 )
	//
	void PartialRender(int iCount, uint16_t* pIndices);
	int GetIndexByiOrder(int iOrder);
	__Vector3 GetVertexByIndex(int iIndex);
	//	~(By Ecli666 On 2002-08-06 오후 4:33:04 )
};

#endif // !defined(AFX_N3PMeshInstance_h__INCLUDED_)

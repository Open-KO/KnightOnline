// N3PMeshCreate.h: interface for the CN3PMeshCreate class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3PMeshCreate_h__INCLUDED_)
#define AFX_N3PMeshCreate_h__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "N3Base.h"

#define _SAME_VERTEXPOS

class CN3Mesh;
class CN3PMesh;
class CN3PMeshCreate
{
public:
	CN3PMeshCreate();
	virtual ~CN3PMeshCreate();

public:
	struct __PMCreateOption
	{
		bool	bUseSumOfLoss;		// Loss의 합을 사용할 것인가 아니면 ..
		bool	bTriangleWeight;	// 사라질 삼각형 가중치 계산 할 것인가?
		bool	bArea; // false = sum of side 가중치를 넓이로 할 것인가? 세 변의 합으로 할 것인가?
		float	fWeight; 	// 가중치
	};

protected:
	// An edge collapse. No-one but this module needs to see this.
	struct __PMCEdgeCollapse
	{
		float Value;
		//PMCMaterial *Material;
		int NumTrianglesToLose, NumIndicesToChange, NumVerticesToLose;
		int *IndexChanges;
		int CollapseTo;
	};
// Helper function
	void swap_triangle(uint16_t *t1, uint16_t *t2);
	float GetTriCollapsesLoss(uint16_t* pTriIndex, bool bArea);
	void combine_modified(float &sofar, uint16_t *tri, int which, int what_to, bool bSumOfLoss);

public:
	int ReGenerate(CN3PMesh* pPMesh); // 다시 생성한다..
	// Do the calculation
	void CreateCollapseList();

	// Then output a mesh for the renderer (this could be saved to disk)
	CN3PMesh* CreateRendererMesh();

	__PMCreateOption m_PMCOption;

	// N3Mesh -> CN3PMeshCreate 로 컨버팅..
	bool ConvertFromN3Mesh(CN3Mesh* pN3Mesh);
	bool ConvertFromN3PMesh(CN3PMesh* pN3PMesh);

	void Release();

protected:
	// For internal use:

	// There is a list of all the index changes in all collapses
	int m_iTotalIndexChanges;
	int *m_pAllIndexChanges;

	// These are all the collapses, in reverse order (highest resolution mesh collapse first)
	__PMCEdgeCollapse *m_pCollapses, *m_pCollapseUpTo;

	bool FindACollapse(float &val_so_far);
	void TryEdge(int pt_a, int pt_b,
				 float &be_val,
				 uint16_t &be_a,
				 uint16_t &be_b,
				 bool &IsOver);
	void Collapse(uint16_t& pt_to, uint16_t& pt_from, float edge_val);

// mesh 정보
protected:

	// A material is an array of vertices and an array of triangles (which is an array of 3x as many indices)
	// Fill these in with your raw mesh data.
	__VertexT1* m_pVertices;
	uint16_t* m_pIndices;
	int m_iNumVertices, m_iNumIndices;


	// After collapse, NumVertices will be the lowest detail level and OriginalNumVertices the original highest
	int m_iOriginalNumVertices, m_iOriginalNumIndices;

	// Internal to collapse code.
	void SwapToEnd(uint16_t swapper, __PMCEdgeCollapse *collapses, __PMCEdgeCollapse *collapses_end, uint16_t &pt_to, uint16_t &pt_from);

#ifdef _SAME_VERTEXPOS
	float GetLossOfSamePosVertex(uint16_t pt_to, uint16_t pt_from);
#endif // _SAME_VERTEXPOS

};

#endif // !defined(AFX_N3PMeshCreate_h__INCLUDED_)

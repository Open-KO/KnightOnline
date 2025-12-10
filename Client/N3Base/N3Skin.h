// N3Skin.h: interface for the CN3Skin class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3Skin_h__INCLUDED_)
#define AFX_N3Skin_h__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "N3IMesh.h"

struct __VertexSkinned
{
	__Vector3	vOrigin; // 원래 점 위치..
	int			nAffect; // 관절 몇개가 영향을 주는지..
	int*		pnJoints; // 관절 인덱스
	float*		pfWeights; // 가중치

	__VertexSkinned()
	{
		vOrigin = {};
		nAffect = 0;
		pnJoints = nullptr;
		pfWeights = nullptr;
	}

	~__VertexSkinned()
	{
		delete[] pnJoints;
		delete[] pfWeights;
	}
};

class CN3Skin : public CN3IMesh
{
protected:
#ifdef _N3TOOL
	struct __Weight
	{
		int nIndex;
		float fWeight;
	};
	static int SortWeightsProc(const void* pArg1, const void* pArg2);
#endif // end of _N3TOOL
	
	__VertexSkinned*	m_pSkinVertices;

public:
	__VertexSkinned*	SkinVertices() { return m_pSkinVertices; }
	bool				Create(int nFC, int nVC, int nUVC);
	bool				Load(HANDLE hFile);

	bool				CheckCollisionPrecisely(const __Vector3 &vPos, const __Vector3 &vDir, __Vector3* pvPick);

#ifdef _N3TOOL
	bool				Save(HANDLE hFile);
	void				RecalcWeight();		// 웨이트값 다시 계산하기
#endif // end of _N3TOOL
	
	void Release();
	CN3Skin();
	virtual ~CN3Skin();

};

#endif // !defined(AFX_N3Skin_h__INCLUDED_)

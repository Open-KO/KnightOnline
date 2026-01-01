// TransDummy.h: interface for the CTransDummy class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TRANSDUMMY_H__EBD00CDE_FCE8_411E_A7A3_020D137904E2__INCLUDED_)
#define AFX_TRANSDUMMY_H__EBD00CDE_FCE8_411E_A7A3_020D137904E2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3Transform.h>
#include <afxtempl.h>

enum {DUMMY_CENTER=0, DUMMY_X=1, DUMMY_Y=2, DUMMY_Z=3, NUM_DUMMY=4,DUMMY_NO=5};
const int NUM_CUBEVERTEX = 36;

struct __DUMMYCUBE
{
	int iType;		// Center , X, Y, Z 종류 구분.
	__VertexXyzNormalColor	Vertices[NUM_CUBEVERTEX];	// 큐브의 구성 점들
	__Vector3				vCenterPos;	// 큐브의 가운데 점
	float					fDistance;	// 카메라와의 거리
};

class CLyTerrain;

class CTransDummy : public CN3Transform
{
public:
	CLyTerrain* m_pTerrainRef;

	CTransDummy();
	virtual ~CTransDummy();

// Attributes
public:
protected:
	__DUMMYCUBE			m_DummyCubes[NUM_DUMMY];	// 4개의 큐브(center, x, y, z);
	__VertexXyzColor	m_LineVertices[6];			// 4개의 큐브를 이어주는 선을 구성하는 점
	CTypedPtrArray<CPtrArray, CN3Transform*> m_SelObjArray;	// MapMng에서 선택된 객체들
	__DUMMYCUBE*		m_pSortedCubes[NUM_DUMMY];	// Cube 거리에 따라 정렬된 포인터
	__DUMMYCUBE*		m_pSelectedCube;			// 선택된 큐브
	__Vector3			m_vPrevPos;	// 이번 마우스 드래그 이전에 위치 회전 확대/축소값
	__Quaternion		m_qPrevRot;	// 이번 마우스 드래그 이전에 위치 회전 확대/축소값
	__Vector3*			m_vPrevScaleArray;	// 이번 마우스 드래그 이전에 확대/축소값들 (선택된 객체들 모두의 값을 가지고 있어야 하기 때문에 배열)

// Operations
public:
	virtual void Tick();
	virtual void Render();
	virtual void Release();
	virtual void SetSelObj(CN3Transform* pObj);		// 선택된 객체 바꾸기
	virtual void AddSelObj(CN3Transform* pObj);	// 선택된 객체 추가
	virtual BOOL MouseMsgFilter(LPMSG pMsg);	// 마우스 메세지 처리
	void GetPickRay(POINT point, __Vector3& vDir, __Vector3& vOrig);	// 마우스 포인터가 가리키는 쪽으로 뻗어나가는 선 얻는 함수
protected:
	__DUMMYCUBE* Pick(int x, int y);	// 큐브 picking
	void InitDummyCube(int iType, __DUMMYCUBE* pDummyCube, const __Vector3& vOffset, D3DCOLOR color);	// 큐브 초기화
	static int SortCube(const void* pArg1, const void* pArg2);	// 큐브 카메라 거리에 대한 정렬함수
	virtual void TransDiff(__Vector3* pvDiffPos, __Quaternion* pvDiffRot, __Vector3* pvDiffScale);		// 차이만큼 선택된 오므젝트들을 변형시킨다.
};

#endif // !defined(AFX_TRANSDUMMY_H__EBD00CDE_FCE8_411E_A7A3_020D137904E2__INCLUDED_)

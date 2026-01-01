// Pick.h: interface for the CPick class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PICK_H__4FEE8165_3BBD_4CD4_9D42_6F13B8A30603__INCLUDED_)
#define AFX_PICK_H__4FEE8165_3BBD_4CD4_9D42_6F13B8A30603__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "N3Mesh.h"

class CPick
{
public:
	CPick();
	virtual ~CPick();

// Attributes
public:
	CN3Mesh m_MeshBox; // Picking Box

protected:
    __Vector3		m_vPickRayDir;			// 충돌 체크할 직선의 방향
    __Vector3		m_vPickRayOrig;			// 충돌 체크할 직선의 시작점

// Operations
public:
	BOOL PickByBox(__Vector3 &vMin, __Vector3 &vMax, __Vector3& vIntersect);
	void			SetPickXY(long x, long y); // 클라이언트 좌표를 넣어준다..

	// [in] v0, v1, v2 세점의 좌표
	// [out] t가 클수록 직선과 평면과 만나는 점이 멀다.	t*dir + orig 를 구하면 만나는 점을 구할 수 있다.
	// [out] uv 좌표  v0 (0,0), v1(1,0), v2(0,1) <괄호안은 (U, V)좌표> 이런식으로 어느 점에 가깝나 나타낸 것
	// v0 + (v1-v0)*u + (v2-v0)*v 를 구하면 만나는 점을 구할 수 있다.
	// [out] vPos 마우스로 찍은 곳의 3D 좌표
	// 리턴 값이 FALSE이면 세점이 이루는 삼각형 안에서 만나지 않는다는 뜻, t u v vPos 모두 무효한 값이므로 쓰면 안된다.
	bool IntersectTriangle(__Vector3& v0, __Vector3& v1, __Vector3& v2, float& t, float& u, float& v, __Vector3* pVPos)
	{
		return ::_IntersectTriangle( m_vPickRayOrig, m_vPickRayDir, v0, v1, v2, t, u, v, pVPos );
	}
};

#endif // !defined(AFX_PICK_H__4FEE8165_3BBD_4CD4_9D42_6F13B8A30603__INCLUDED_)

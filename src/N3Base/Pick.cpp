// Pick.cpp: implementation of the CPick class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfxBase.h"
#include "Pick.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPick::CPick()
{
}

CPick::~CPick()
{
}

void CPick::SetPickXY(long x, long y)
{
	LPDIRECT3DDEVICE9 lpD3DDev = CN3Base::s_lpD3DDev;

	// Get the pick ray from the mouse position
	__Matrix44 matProj;
	lpD3DDev->GetTransform(D3DTS_PROJECTION, matProj.toD3D());

	// Compute the vector of the pick ray in screen space
	__Vector3 v;
	v.x = (((2.0f * x) / (CN3Base::s_CameraData.vp.Width)) - 1) / matProj.m[0][0];
	v.y = -(((2.0f * y) / (CN3Base::s_CameraData.vp.Height)) - 1) / matProj.m[1][1];
	v.z = 1.0f;

	// Get the inverse view matrix
	__Matrix44 matView, m;
	lpD3DDev->GetTransform(D3DTS_VIEW, matView.toD3D());
	m                = matView.Inverse();

	// Transform the screen space pick ray into 3D space
	m_vPickRayDir.x  = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0];
	m_vPickRayDir.y  = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1];
	m_vPickRayDir.z  = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2];
	m_vPickRayOrig.x = m.m[3][0];
	m_vPickRayOrig.y = m.m[3][1];
	m_vPickRayOrig.z = m.m[3][2];
}

BOOL CPick::PickByBox(__Vector3& vMin, __Vector3& vMax, __Vector3& vIntersect)
{
	m_MeshBox.Create_Cube(vMin, vMax);

	__VertexT1* pVs = m_MeshBox.Vertices();
	if (pVs == nullptr)
		return FALSE;

	float fT, fU, fV;
	__Vector3 vTri[3];
	for (int j = 0; j < 12; j++)
	{
		vTri[0].Set(pVs[j * 3 + 0].x, pVs[j * 3 + 0].y, pVs[j * 3 + 0].z);
		vTri[1].Set(pVs[j * 3 + 1].x, pVs[j * 3 + 1].y, pVs[j * 3 + 1].z);
		vTri[2].Set(pVs[j * 3 + 2].x, pVs[j * 3 + 2].y, pVs[j * 3 + 2].z);

		if (this->IntersectTriangle(vTri[0], vTri[1], vTri[2], fT, fU, fV, &vIntersect) == TRUE)
			return TRUE;
	}

	return FALSE;
}

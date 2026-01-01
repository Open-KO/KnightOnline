#include "MathUtils.h"

#include "Matrix44.inl"
#include "Vector2.inl"
#include "Vector3.inl"
#include "Vector4.inl"
#include "Quaternion.inl"

#include <cmath>

bool _CheckCollisionByBox(const __Vector3& vOrig, const __Vector3& vDir, const __Vector3& vMin, const __Vector3& vMax)
{
	__Vector3 Vertices[36];
	int nFace = 0;

	// z 축 음의 면
	nFace = 0;
	Vertices[nFace + 0].Set(vMin.x, vMax.y, vMin.z); Vertices[nFace + 1].Set(vMax.x, vMax.y, vMin.z); Vertices[nFace + 2].Set(vMax.x, vMin.y, vMin.z);
	Vertices[nFace + 3] = Vertices[nFace + 0]; Vertices[nFace + 4] = Vertices[nFace + 2]; Vertices[nFace + 5].Set(vMin.x, vMin.y, vMin.z);

	// x 축 양의 면
	nFace = 6;
	Vertices[nFace + 0].Set(vMax.x, vMax.y, vMin.z); Vertices[nFace + 1].Set(vMax.x, vMax.y, vMax.z); Vertices[nFace + 2].Set(vMax.x, vMin.y, vMax.z);
	Vertices[nFace + 3] = Vertices[nFace + 0]; Vertices[nFace + 4] = Vertices[nFace + 2]; Vertices[nFace + 5].Set(vMax.x, vMin.y, vMin.z);

	// z 축 양의 면
	nFace = 12;
	Vertices[nFace + 0].Set(vMax.x, vMax.y, vMax.z); Vertices[nFace + 1].Set(vMin.x, vMax.y, vMax.z); Vertices[nFace + 2].Set(vMin.x, vMin.y, vMax.z);
	Vertices[nFace + 3] = Vertices[nFace + 0]; Vertices[nFace + 4] = Vertices[nFace + 2]; Vertices[nFace + 5].Set(vMax.x, vMin.y, vMax.z);

	// x 축 음의 면
	nFace = 18;
	Vertices[nFace + 0].Set(vMin.x, vMax.y, vMax.z); Vertices[nFace + 1].Set(vMin.x, vMax.y, vMin.z); Vertices[nFace + 2].Set(vMin.x, vMin.y, vMin.z);
	Vertices[nFace + 3] = Vertices[nFace + 0]; Vertices[nFace + 4] = Vertices[nFace + 2]; Vertices[nFace + 5].Set(vMin.x, vMin.y, vMax.z);

	// y 축 양의 면
	nFace = 24;
	Vertices[nFace + 0].Set(vMin.x, vMax.y, vMax.z); Vertices[nFace + 1].Set(vMax.x, vMax.y, vMax.z); Vertices[nFace + 2].Set(vMax.x, vMax.y, vMin.z);
	Vertices[nFace + 3] = Vertices[nFace + 0]; Vertices[nFace + 4] = Vertices[nFace + 2]; Vertices[nFace + 5].Set(vMin.x, vMax.y, vMin.z);

	// y 축 음의 면
	nFace = 30;
	Vertices[nFace + 0].Set(vMin.x, vMin.y, vMin.z); Vertices[nFace + 1].Set(vMax.x, vMin.y, vMin.z); Vertices[nFace + 2].Set(vMax.x, vMin.y, vMax.z);
	Vertices[nFace + 3] = Vertices[nFace + 0]; Vertices[nFace + 4] = Vertices[nFace + 2]; Vertices[nFace + 5].Set(vMin.x, vMin.y, vMax.z);

	// 각 면에 대해서 충돌 검사..
	for (int i = 0; i < 12; i++)
	{
		if (::_IntersectTriangle(vOrig, vDir, Vertices[i * 3 + 0], Vertices[i * 3 + 1], Vertices[i * 3 + 2]))
			return true;
	}

	return false;
}

bool _IntersectTriangle(
	const __Vector3& vOrig, const __Vector3& vDir,
	const __Vector3& v0, const __Vector3& v1, const __Vector3& v2,
	float& fT, float& fU, float& fV, __Vector3* pVCol)
{
	// Find vectors for two edges sharing vert0
	__Vector3 vEdge1, vEdge2;

	vEdge1 = v1 - v0;
	vEdge2 = v2 - v0;

	// Begin calculating determinant - also used to calculate U parameter
	__Vector3 pVec;
	float fDet;

//	By : Ecli666 ( On 2001-09-12 오전 10:39:01 )

	pVec.Cross(vEdge1, vEdge2);
	fDet = pVec.Dot(vDir);
	if (fDet > -0.0001f)
		return false;

//	~(By Ecli666 On 2001-09-12 오전 10:39:01 )

	pVec.Cross(vDir, vEdge2);

	// If determinant is near zero, ray lies in plane of triangle
	fDet = vEdge1.Dot(pVec);
	if (fDet < 0.0001f)		// 거의 0에 가까우면 삼각형 평면과 지나가는 선이 평행하다.
		return false;

	// Calculate distance from vert0 to ray origin
	__Vector3 tVec = vOrig - v0;

	// Calculate U parameter and test bounds
	fU = tVec.Dot(pVec);
	if (fU < 0.0f || fU > fDet)
		return false;

	// Prepare to test V parameter
	__Vector3 qVec;
	qVec.Cross(tVec, vEdge1);

	// Calculate V parameter and test bounds
	fV = vDir.Dot(qVec);
	if (fV < 0.0f || fU + fV > fDet)
		return false;

	// Calculate t, scale parameters, ray intersects triangle
	fT = vEdge2.Dot(qVec);

	float fInvDet = 1.0f / fDet;
	fT *= fInvDet;
	fU *= fInvDet;
	fV *= fInvDet;

	// t가 클수록 멀리 직선과 평면과 만나는 점이 멀다.
	// t*dir + orig 를 구하면 만나는 점을 구할 수 있다.
	// u와 v의 의미는 무엇일까?
	// 추측 : v0 (0,0), v1(1,0), v2(0,1) <괄호안은 (U, V)좌표> 이런식으로 어느 점에 가깝나 나타낸 것 같음
	//

	if (pVCol != nullptr)
		(*pVCol) = vOrig + (vDir * fT);	// 접점을 계산..

	// *t < 0 이면 뒤쪽...
	if (fT < 0.0f)
		return false;

	return true;
}

bool _IntersectTriangle(
	const __Vector3& vOrig, const __Vector3& vDir,
	const __Vector3& v0, const __Vector3& v1, const __Vector3& v2)
{
	// Find vectors for two edges sharing vert0
	// Begin calculating determinant - also used to calculate U parameter
	float fDet, fT, fU, fV;
	__Vector3 vEdge1, vEdge2, tVec, pVec, qVec;

	vEdge1 = v1 - v0;
	vEdge2 = v2 - v0;

//	By : Ecli666 ( On 2001-09-12 오전 10:39:01 )

	pVec.Cross(vEdge1, vEdge2);
	fDet = pVec.Dot(vDir);
	if (fDet > -0.0001f)
		return false;

//	~(By Ecli666 On 2001-09-12 오전 10:39:01 )

	pVec.Cross(vDir, vEdge2);

	// If determinant is near zero, ray lies in plane of triangle
	fDet = vEdge1.Dot(pVec);
	if (fDet < 0.0001f)		// 거의 0에 가까우면 삼각형 평면과 지나가는 선이 평행하다.
		return false;

	// Calculate distance from vert0 to ray origin
	tVec = vOrig - v0;

	// Calculate U parameter and test bounds
	fU = tVec.Dot(pVec);
	if (fU < 0.0f || fU > fDet)
		return false;

	// Prepare to test V parameter
	qVec.Cross(tVec, vEdge1);

	// Calculate V parameter and test bounds
	fV = vDir.Dot(qVec);
	if (fV < 0.0f || fU + fV > fDet)
		return false;

	// Calculate t, scale parameters, ray intersects triangle
	fT = vEdge2.Dot(qVec) / fDet;

	// *t < 0 이면 뒤쪽...
	if (fT < 0.0f)
		return false;

	return true;
}

_POINT _Convert3D_To_2DCoordinate(
	const __Vector3& vPos,
	const __Matrix44& mtxView, const __Matrix44& mtxProjection,
	int iViewportWidth, int iViewportHeight)
{
	__Matrix44 matVP = mtxView * mtxProjection;
	__Vector4 v;
	v.Transform(vPos, matVP);

	_POINT pt;
	float fScreenZ = (v.z / v.w);
	if (fScreenZ > 1.0 || fScreenZ < 0.0)
	{
		pt.x = -1;
		pt.y = -1;
		return pt;
	}

	pt.x = int(((v.x / v.w) + 1.0f) * iViewportWidth / 2.0f);
	pt.y = int((1.0f - (v.y / v.w)) * iViewportHeight / 2.0f);

	return pt;
}

void _Convert2D_To_3DCoordinate(
	int ixScreen, int iyScreen,
	const __Matrix44& mtxView, const __Matrix44& mtxPrj,
	int iViewportWidth, int iViewportHeight,
	__Vector3& vPosResult, __Vector3& vDirResult)
{
	// Compute the vector of the pick ray in screen space
	__Vector3 vTmp;

	vTmp.x = (((2.0f * ixScreen) / iViewportWidth) - 1) / mtxPrj.m[0][0];
	vTmp.y = -(((2.0f * iyScreen) / iViewportHeight) - 1) / mtxPrj.m[1][1];
	vTmp.z = 1.0f;

	// Transform the screen space pick ray into 3D space
	__Matrix44 mtxVI = mtxView.Inverse();
	vDirResult.x = vTmp.x * mtxVI.m[0][0] + vTmp.y * mtxVI.m[1][0] + vTmp.z * mtxVI.m[2][0];
	vDirResult.y = vTmp.x * mtxVI.m[0][1] + vTmp.y * mtxVI.m[1][1] + vTmp.z * mtxVI.m[2][1];
	vDirResult.z = vTmp.x * mtxVI.m[0][2] + vTmp.y * mtxVI.m[1][2] + vTmp.z * mtxVI.m[2][2];
	vPosResult = mtxVI.Pos();
}

float _Yaw2D(float fDirX, float fDirZ)
{
	////////////////////////////////
	// 방향을 구하고.. -> 회전할 값을 구하는 루틴이다..
	if (fDirX >= 0.0f)						// ^^
	{
		if (fDirZ >= 0.0f)
			return (float) (asin(fDirX));

		return (DegreesToRadians(90.0f) + (float) (acos(fDirX)));
	}
	else
	{
		if (fDirZ >= 0.0f)
			return (DegreesToRadians(270.0f) + (float) (acos(-fDirX)));

		return (DegreesToRadians(180.0f) + (float) (asin(-fDirX)));
	}
	// 방향을 구하고..
	////////////////////////////////
}

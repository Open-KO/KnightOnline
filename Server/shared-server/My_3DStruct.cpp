#include "pch.h"
#include "My_3DStruct.h"

#include "Vector3.inl"
#include "Matrix44.inl"

#ifdef _DEBUG
void ASSERT_IMPL(const char* expressionString, bool expressionResult, const char* file, int line, const char* expressionMessage)
{
	if (!expressionResult)
		spdlog::error("Assertion failed: {}({}) - {} ({})", file, line, expressionMessage, expressionString);
}
#endif

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
	fV = qVec.Dot(vDir);
	if (fV < 0.0f || fU + fV > fDet)
		return false;

	// Calculate t, scale parameters, ray intersects triangle
	fT = qVec.Dot(vEdge2);

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
	fV = qVec.Dot(vDir);
	if (fV < 0.0f || fU + fV > fDet)
		return false;

	// Calculate t, scale parameters, ray intersects triangle
	fT = qVec.Dot(vEdge2) / fDet;

	// *t < 0 이면 뒤쪽...
	if (fT < 0.0f)
		return false;

	return true;
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

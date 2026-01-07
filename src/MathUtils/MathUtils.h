#ifndef MATHUTILS_MATHUTILS_H
#define MATHUTILS_MATHUTILS_H

#pragma once

inline constexpr float __PI  = 3.141592654f;
inline constexpr float __PI2 = 6.283185308f;

constexpr float DegreesToRadians(auto degrees)
{
	return static_cast<float>(degrees) * (__PI / 180.0f);
}

constexpr float RadiansToDegrees(auto radians)
{
	return static_cast<float>(radians) * (180.0f / __PI);
}

#include "GeometricStructs.h"
#include "Matrix44.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Quaternion.h"

bool _IntersectTriangle(const __Vector3& vOrig, const __Vector3& vDir, const __Vector3& v0,
	const __Vector3& v1, const __Vector3& v2, float& fT, float& fU, float& fV,
	__Vector3* pVCol = nullptr);
bool _IntersectTriangle(const __Vector3& vOrig, const __Vector3& vDir, const __Vector3& v0,
	const __Vector3& v1, const __Vector3& v2);
bool _CheckCollisionByBox(
	const __Vector3& vOrig, const __Vector3& vDir, const __Vector3& vMin, const __Vector3& vMax);
_POINT _Convert3D_To_2DCoordinate(const __Vector3& vPos, const __Matrix44& mtxView,
	const __Matrix44& mtxProjection, int iViewportWidth, int iViewportHeight);
void _Convert2D_To_3DCoordinate(int ixScreen, int iyScreen, const __Matrix44& mtxView,
	const __Matrix44& mtxPrj, int iViewportWidth, int iViewportHeight, __Vector3& vPosResult,
	__Vector3& vDirResult);
float _Yaw2D(float fDirX, float fDirZ);

#endif // MATHUTILS_MATHUTILS_H

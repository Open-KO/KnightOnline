#ifndef MATHUTILS_QUATERNION_H
#define MATHUTILS_QUATERNION_H

#pragma once

struct __Matrix44;
struct __Vector3;

struct __Quaternion
{
public:
	__Quaternion() = default;
	__Quaternion(const __Matrix44& mtx);
	__Quaternion(const __Quaternion& qt);
	__Quaternion(float fX, float fY, float fZ, float fW);

	void Identity();
	void Set(float fX, float fY, float fZ, float fW);

	void RotationAxis(const __Vector3& v, float fRadian);
	void RotationAxis(float fX, float fY, float fZ, float fRadian);
	__Quaternion& operator=(const __Matrix44& mtx);

	void AxisAngle(__Vector3& vAxisResult, float& fRadianResult) const;
	void Slerp(const __Quaternion& qt1, const __Quaternion& qt2, float fDelta);
	void RotationYawPitchRoll(float Yaw, float Pitch, float Roll);

	__Quaternion operator*(const __Quaternion& q) const;
	void operator*=(const __Quaternion& q);

public:
	float x, y, z, w;
};

#endif // MATHUTILS_QUATERNION_H

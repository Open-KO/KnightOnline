#ifndef MATHUTILS_VECTOR3_INL
#define MATHUTILS_VECTOR3_INL

#pragma once

#include "MathUtils.h"
#include <cmath> // sqrtf()

__Vector3::__Vector3(float fx, float fy, float fz)
{
	x = fx;
	y = fy;
	z = fz;
}

__Vector3::__Vector3(const __Vector3& vec)
{
	x = vec.x;
	y = vec.y;
	z = vec.z;
}

void __Vector3::Normalize()
{
	float fn = sqrtf(x * x + y * y + z * z);
	if (fn == 0)
		return;

	x /= fn;
	y /= fn;
	z /= fn;
}

void __Vector3::Normalize(const __Vector3& vec)
{
	float fn = sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
	if (fn == 0)
	{
		x = y = z = 0;
		return;
	}

	x = vec.x / fn;
	y = vec.y / fn;
	z = vec.z / fn;
}

float __Vector3::Magnitude() const
{
	return sqrtf(x * x + y * y + z * z);
}

float __Vector3::Dot(const __Vector3& vec) const
{
	return x * vec.x + y * vec.y + z * vec.z;
}

void __Vector3::Cross(const __Vector3& v1, const __Vector3& v2)
{
	x = v1.y * v2.z - v1.z * v2.y;
	y = v1.z * v2.x - v1.x * v2.z;
	z = v1.x * v2.y - v1.y * v2.x;
}

void __Vector3::Absolute()
{
	if (x < 0)
		x *= -1.0f;

	if (y < 0)
		y *= -1.0f;

	if (z < 0)
		z *= -1.0f;
}

void __Vector3::Zero()
{
	x = y = z = 0;
}

void __Vector3::Set(float fx, float fy, float fz)
{
	x = fx;
	y = fy;
	z = fz;
}

const __Vector3& __Vector3::operator=(const __Vector3& vec)
{
	x = vec.x;
	y = vec.y;
	z = vec.z;
	return *this;
}

const __Vector3 __Vector3::operator*(const __Matrix44& mtx) const
{
	__Vector3 vTmp;

	vTmp.x = x * mtx.m[0][0] + y * mtx.m[1][0] + z * mtx.m[2][0] + mtx.m[3][0];
	vTmp.y = x * mtx.m[0][1] + y * mtx.m[1][1] + z * mtx.m[2][1] + mtx.m[3][1];
	vTmp.z = x * mtx.m[0][2] + y * mtx.m[1][2] + z * mtx.m[2][2] + mtx.m[3][2];

	return vTmp;
}

void __Vector3::operator*=(float fDelta)
{
	x *= fDelta;
	y *= fDelta;
	z *= fDelta;
}

void __Vector3::operator*=(const __Matrix44& mtx)
{
	__Vector3 vTmp = { x, y, z };
	x = vTmp.x * mtx.m[0][0] + vTmp.y * mtx.m[1][0] + vTmp.z * mtx.m[2][0] + mtx.m[3][0];
	y = vTmp.x * mtx.m[0][1] + vTmp.y * mtx.m[1][1] + vTmp.z * mtx.m[2][1] + mtx.m[3][1];
	z = vTmp.x * mtx.m[0][2] + vTmp.y * mtx.m[1][2] + vTmp.z * mtx.m[2][2] + mtx.m[3][2];
}

__Vector3 __Vector3::operator+(const __Vector3& vec) const
{
	return { x + vec.x, y + vec.y, z + vec.z };
}

__Vector3 __Vector3::operator-(const __Vector3& vec) const
{
	return { x - vec.x, y - vec.y, z - vec.z };
}

__Vector3 __Vector3::operator*(const __Vector3& vec) const
{
	return { x * vec.x, y * vec.y, z * vec.z };
}

__Vector3 __Vector3::operator/(const __Vector3& vec) const
{
	return { x / vec.x, y / vec.y, z / vec.z };
}

void __Vector3::operator+=(const __Vector3& vec)
{
	x += vec.x;
	y += vec.y;
	z += vec.z;
}

void __Vector3::operator-=(const __Vector3& vec)
{
	x -= vec.x;
	y -= vec.y;
	z -= vec.z;
}

void __Vector3::operator*=(const __Vector3& vec)
{
	x *= vec.x;
	y *= vec.y;
	z *= vec.z;
}

void __Vector3::operator/=(const __Vector3& vec)
{
	x /= vec.x;
	y /= vec.y;
	z /= vec.z;
}

__Vector3 __Vector3::operator+(float fDelta) const
{
	return { x + fDelta, y + fDelta, z + fDelta };
}

__Vector3 __Vector3::operator-(float fDelta) const
{
	return { x - fDelta, y - fDelta, z - fDelta };
}

__Vector3 __Vector3::operator*(float fDelta) const
{
	return { x * fDelta, y * fDelta, z * fDelta };
}

__Vector3 __Vector3::operator/(float fDelta) const
{
	return { x / fDelta, y / fDelta, z / fDelta };
}

bool __Vector3::operator==(const __Vector3& rhs) const
{
	return x == rhs.x && y == rhs.y && z == rhs.z;
}

bool __Vector3::operator!=(const __Vector3& rhs) const
{
	return !(*this == rhs);
}

#endif // MATHUTILS_VECTOR3_INL

#ifndef MATHUTILS_VECTOR4_INL
#define MATHUTILS_VECTOR4_INL

#pragma once

#include "MathUtils.h"

__Vector4::__Vector4(float fx, float fy, float fz, float fw) : x(fx), y(fy), z(fz), w(fw)
{
}

void __Vector4::Zero()
{
	x = y = z = w = 0;
}

void __Vector4::Set(float fx, float fy, float fz, float fw)
{
	x = fx;
	y = fy;
	z = fz;
	w = fw;
}

void __Vector4::Transform(const __Vector3& v, const __Matrix44& m)
{
	x = m.m[0][0] * v.x + m.m[1][0] * v.y + m.m[2][0] * v.z + m.m[3][0];
	y = m.m[0][1] * v.x + m.m[1][1] * v.y + m.m[2][1] * v.z + m.m[3][1];
	z = m.m[0][2] * v.x + m.m[1][2] * v.y + m.m[2][2] * v.z + m.m[3][2];
	w = m.m[0][3] * v.x + m.m[1][3] * v.y + m.m[2][3] * v.z + m.m[3][3];
}

__Vector4& __Vector4::operator+=(const __Vector4& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
	return *this;
}

__Vector4& __Vector4::operator-=(const __Vector4& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	w -= v.w;
	return *this;
}

__Vector4& __Vector4::operator*=(float f)
{
	x *= f;
	y *= f;
	z *= f;
	w *= f;
	return *this;
}

__Vector4& __Vector4::operator/=(float f)
{
	float fInv  = 1.0f / f;
	x          *= fInv;
	y          *= fInv;
	z          *= fInv;
	w          *= fInv;
	return *this;
}

__Vector4 __Vector4::operator+(const __Vector4& v) const
{
	return { x + v.x, y + v.y, z + v.z, w + v.w };
}

__Vector4 __Vector4::operator-(const __Vector4& v) const
{
	return { x - v.x, y - v.y, z - v.z, w - v.w };
}

__Vector4 __Vector4::operator*(float f) const
{
	return { x * f, y * f, z * f, w * f };
}

__Vector4 __Vector4::operator/(float f) const
{
	float fInv = 1.0f / f;
	return { x * fInv, y * fInv, z * fInv, w * fInv };
}

#endif // MATHUTILS_VECTOR4_INL

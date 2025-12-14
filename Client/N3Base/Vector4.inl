#ifndef CLIENT_N3BASE_VECTOR4_INL
#define CLIENT_N3BASE_VECTOR4_INL

#pragma once

#include "My_3DStruct.h"

__Vector4::__Vector4(float fx, float fy, float fz, float fw)
{
	x = fx;
	y = fy;
	z = fz;
	w = fw;
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

void __Vector4::Transform(const D3DXVECTOR3& v, const D3DXMATRIX& m)
{
	D3DXVec3Transform(this, &v, &m);
}

__Vector4& __Vector4::operator += (const D3DXVECTOR4& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
	return *this;
}

__Vector4& __Vector4::operator -= (const D3DXVECTOR4& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	w -= v.w;
	return *this;
}

__Vector4& __Vector4::operator *= (float f)
{
	x *= f;
	y *= f;
	z *= f;
	w *= f;
	return *this;
}

__Vector4& __Vector4::operator /= (float f)
{
	float fInv = 1.0f / f;
	x *= fInv;
	y *= fInv;
	z *= fInv;
	w *= fInv;
	return *this;
}

__Vector4 __Vector4::operator + (const D3DXVECTOR4& v) const
{
	return __Vector4(x + v.x, y + v.y, z + v.z, w + v.w);
}

__Vector4 __Vector4::operator - (const D3DXVECTOR4& v) const
{
	return __Vector4(x - v.x, y - v.y, z - v.z, w - v.w);
}

__Vector4 __Vector4::operator * (float f) const
{
	return __Vector4(x * f, y * f, z * f, w * f);
}

__Vector4 __Vector4::operator / (float f) const
{
	float fInv = 1.0f / f;
	return __Vector4(x * fInv, y * fInv, z * fInv, w * fInv);
}

#endif // CLIENT_N3BASE_VECTOR4_INL

#ifndef CLIENT_N3BASE_VECTOR3_INL
#define CLIENT_N3BASE_VECTOR3_INL

#pragma once

#include "My_3DStruct.h"

__Vector3::__Vector3(float fx, float fy, float fz)
{
	x = fx;
	y = fy;
	z = fz;
}

__Vector3::__Vector3(const D3DXVECTOR3& vec)
{
	x = vec.x;
	y = vec.y;
	z = vec.z;
}

__Vector3::__Vector3(const _D3DVECTOR& vec)
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

void __Vector3::Normalize(const D3DXVECTOR3& vec)
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

float __Vector3::Dot(const D3DXVECTOR3& vec) const
{
	return x * vec.x + y * vec.y + z * vec.z;
}

void __Vector3::Cross(const D3DXVECTOR3& v1, const D3DXVECTOR3& v2)
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

const __Vector3& __Vector3::operator = (const __Vector3& vec)
{
	x = vec.x;
	y = vec.y;
	z = vec.z;
	return *this;
}

const __Vector3 __Vector3::operator * (const D3DXMATRIX& mtx) const
{
	__Vector3 vTmp;

	vTmp.x = x * mtx._11 + y * mtx._21 + z * mtx._31 + mtx._41;
	vTmp.y = x * mtx._12 + y * mtx._22 + z * mtx._32 + mtx._42;
	vTmp.z = x * mtx._13 + y * mtx._23 + z * mtx._33 + mtx._43;

	return vTmp;
}

void __Vector3::operator *= (float fDelta)
{
	x *= fDelta;
	y *= fDelta;
	z *= fDelta;
}

void __Vector3::operator *= (const D3DXMATRIX& mtx)
{
	__Vector3 vTmp;

	vTmp.Set(x, y, z);
	x = vTmp.x * mtx._11 + vTmp.y * mtx._21 + vTmp.z * mtx._31 + mtx._41;
	y = vTmp.x * mtx._12 + vTmp.y * mtx._22 + vTmp.z * mtx._32 + mtx._42;
	z = vTmp.x * mtx._13 + vTmp.y * mtx._23 + vTmp.z * mtx._33 + mtx._43;
}

__Vector3 __Vector3::operator + (const D3DXVECTOR3& vec) const
{
	__Vector3 vTmp;
	vTmp.x = x + vec.x;
	vTmp.y = y + vec.y;
	vTmp.z = z + vec.z;
	return vTmp;
}

__Vector3 __Vector3::operator - (const D3DXVECTOR3& vec) const
{
	__Vector3 vTmp;
	vTmp.x = x - vec.x;
	vTmp.y = y - vec.y;
	vTmp.z = z - vec.z;
	return vTmp;
}

__Vector3 __Vector3::operator * (const D3DXVECTOR3& vec) const
{
	__Vector3 vTmp;
	vTmp.x = x * vec.x;
	vTmp.y = y * vec.y;
	vTmp.z = z * vec.z;
	return vTmp;
}

__Vector3 __Vector3::operator / (const D3DXVECTOR3& vec) const
{
	__Vector3 vTmp;
	vTmp.x = x / vec.x;
	vTmp.y = y / vec.y;
	vTmp.z = z / vec.z;
	return vTmp;
}

void __Vector3::operator += (const D3DXVECTOR3& vec)
{
	x += vec.x;
	y += vec.y;
	z += vec.z;
}

void __Vector3::operator -= (const D3DXVECTOR3& vec)
{
	x -= vec.x;
	y -= vec.y;
	z -= vec.z;
}

void __Vector3::operator *= (const D3DXVECTOR3& vec)
{
	x *= vec.x;
	y *= vec.y;
	z *= vec.z;
}

void __Vector3::operator /= (const D3DXVECTOR3& vec)
{
	x /= vec.x;
	y /= vec.y;
	z /= vec.z;
}

__Vector3 __Vector3::operator + (float fDelta) const
{
	__Vector3 vTmp;
	vTmp.x = x + fDelta;
	vTmp.y = y + fDelta;
	vTmp.z = z + fDelta;
	return vTmp;
}

__Vector3 __Vector3::operator - (float fDelta) const
{
	__Vector3 vTmp;
	vTmp.x = x - fDelta;
	vTmp.y = y - fDelta;
	vTmp.z = z - fDelta;
	return vTmp;
}

__Vector3 __Vector3::operator * (float fDelta) const
{
	__Vector3 vTmp;
	vTmp.x = x * fDelta;
	vTmp.y = y * fDelta;
	vTmp.z = z * fDelta;
	return vTmp;
}

__Vector3 __Vector3::operator / (float fDelta) const
{
	__Vector3 vTmp;
	vTmp.x = x / fDelta;
	vTmp.y = y / fDelta;
	vTmp.z = z / fDelta;
	return vTmp;
}

#endif // CLIENT_N3BASE_VECTOR3_INL

#ifndef SERVER_SHAREDSERVER_MATRIX44_INL
#define SERVER_SHAREDSERVER_MATRIX44_INL

#pragma once

#include "My_3DStruct.h"
#include <cstring> // std::memcpy(), std::memset()

__Matrix44::__Matrix44(const __Matrix44& mtx)
{
	std::memcpy(&m, &mtx.m, sizeof(__Matrix44));
}

void __Matrix44::Zero()
{
	std::memset(&m, 0, sizeof(__Matrix44));
}

void __Matrix44::Identity()
{
	_12 = _13 = _14 = _21 = _23 = _24 = _31 = _32 = _34 = _41 = _42 = _43 = 0;
	_11 = _22 = _33 = _44 = 1.0f;
}

const __Vector3 __Matrix44::Pos() const
{
	__Vector3 vTmp;

	vTmp.Set(_41, _42, _43);
	return vTmp;
}

void __Matrix44::PosSet(float x, float y, float z)
{
	_41 = x;
	_42 = y;
	_43 = z;
}

void __Matrix44::PosSet(const __Vector3& v)
{
	_41 = v.x;
	_42 = v.y;
	_43 = v.z;
}

void __Matrix44::RotationX(float fDelta)
{
	Identity();
	_22 = cosf(fDelta);
	_23 = sinf(fDelta);
	_32 = -_23;
	_33 = _22;
}

void __Matrix44::RotationY(float fDelta)
{
	Identity();
	_11 = cosf(fDelta);
	_13 = -sinf(fDelta);
	_31 = -_13;
	_33 = _11;
}

void __Matrix44::RotationZ(float fDelta)
{
	Identity();
	_11 = cosf(fDelta);
	_12 = sinf(fDelta);
	_21 = -_12;
	_22 = _11;
}

void __Matrix44::Rotation(float fX, float fY, float fZ)
{
	float SX = sinf(fX), CX = cosf(fX);
	float SY = sinf(fY), CY = cosf(fY);
	float SZ = sinf(fZ), CZ = cosf(fZ);
	_11 = CY * CZ;
	_12 = CY * SZ;
	_13 = -SY;
	_14 = 0;

	_21 = SX * SY * CZ - CX * SZ;
	_22 = SX * SY * SZ + CX * CZ;
	_23 = SX * CY;
	_24 = 0;

	_31 = CX * SY * CZ + SX * SZ;
	_32 = CX * SY * SZ - SX * CZ;
	_33 = CX * CY;
	_34 = 0;

	_41 = _42 = _43 = 0; _44 = 1;
}

void __Matrix44::Rotation(const __Vector3& v)
{
	float SX = sinf(v.x), CX = cosf(v.x);
	float SY = sinf(v.y), CY = cosf(v.y);
	float SZ = sinf(v.z), CZ = cosf(v.z);
	_11 = CY * CZ;
	_12 = CY * SZ;
	_13 = -SY;
	_14 = 0;

	_21 = SX * SY * CZ - CX * SZ;
	_22 = SX * SY * SZ + CX * CZ;
	_23 = SX * CY;
	_24 = 0;

	_31 = CX * SY * CZ + SX * SZ;
	_32 = CX * SY * SZ - SX * CZ;
	_33 = CX * CY;
	_34 = 0;

	_41 = _42 = _43 = 0; _44 = 1;
}

void __Matrix44::Scale(float sx, float sy, float sz)
{
	Identity();
	_11 = sx;
	_22 = sy;
	_33 = sz;
}

void __Matrix44::Scale(const __Vector3& v)
{
	Identity();
	_11 = v.x;
	_22 = v.y;
	_33 = v.z;
}

__Matrix44 __Matrix44::operator * (const __Matrix44& mtx)
{
	__Matrix44 mtxTmp;

	mtxTmp._11 = _11 * mtx._11 + _12 * mtx._21 + _13 * mtx._31 + _14 * mtx._41;
	mtxTmp._12 = _11 * mtx._12 + _12 * mtx._22 + _13 * mtx._32 + _14 * mtx._42;
	mtxTmp._13 = _11 * mtx._13 + _12 * mtx._23 + _13 * mtx._33 + _14 * mtx._43;
	mtxTmp._14 = _11 * mtx._14 + _12 * mtx._24 + _13 * mtx._34 + _14 * mtx._44;

	mtxTmp._21 = _21 * mtx._11 + _22 * mtx._21 + _23 * mtx._31 + _24 * mtx._41;
	mtxTmp._22 = _21 * mtx._12 + _22 * mtx._22 + _23 * mtx._32 + _24 * mtx._42;
	mtxTmp._23 = _21 * mtx._13 + _22 * mtx._23 + _23 * mtx._33 + _24 * mtx._43;
	mtxTmp._24 = _21 * mtx._14 + _22 * mtx._24 + _23 * mtx._34 + _24 * mtx._44;

	mtxTmp._31 = _31 * mtx._11 + _32 * mtx._21 + _33 * mtx._31 + _34 * mtx._41;
	mtxTmp._32 = _31 * mtx._12 + _32 * mtx._22 + _33 * mtx._32 + _34 * mtx._42;
	mtxTmp._33 = _31 * mtx._13 + _32 * mtx._23 + _33 * mtx._33 + _34 * mtx._43;
	mtxTmp._34 = _31 * mtx._14 + _32 * mtx._24 + _33 * mtx._34 + _34 * mtx._44;

	mtxTmp._41 = _41 * mtx._11 + _42 * mtx._21 + _43 * mtx._31 + _44 * mtx._41;
	mtxTmp._42 = _41 * mtx._12 + _42 * mtx._22 + _43 * mtx._32 + _44 * mtx._42;
	mtxTmp._43 = _41 * mtx._13 + _42 * mtx._23 + _43 * mtx._33 + _44 * mtx._43;
	mtxTmp._44 = _41 * mtx._14 + _42 * mtx._24 + _43 * mtx._34 + _44 * mtx._44;

	return mtxTmp;
}

void __Matrix44::operator *= (const __Matrix44& mtx)
{
	__Matrix44 mtxTmp;

	std::memcpy(&mtxTmp.m, &m, sizeof(__Matrix44));

	_11 = mtxTmp._11 * mtx._11 + mtxTmp._12 * mtx._21 + mtxTmp._13 * mtx._31 + mtxTmp._14 * mtx._41;
	_12 = mtxTmp._11 * mtx._12 + mtxTmp._12 * mtx._22 + mtxTmp._13 * mtx._32 + mtxTmp._14 * mtx._42;
	_13 = mtxTmp._11 * mtx._13 + mtxTmp._12 * mtx._23 + mtxTmp._13 * mtx._33 + mtxTmp._14 * mtx._43;
	_14 = mtxTmp._11 * mtx._14 + mtxTmp._12 * mtx._24 + mtxTmp._13 * mtx._34 + mtxTmp._14 * mtx._44;

	_21 = mtxTmp._21 * mtx._11 + mtxTmp._22 * mtx._21 + mtxTmp._23 * mtx._31 + mtxTmp._24 * mtx._41;
	_22 = mtxTmp._21 * mtx._12 + mtxTmp._22 * mtx._22 + mtxTmp._23 * mtx._32 + mtxTmp._24 * mtx._42;
	_23 = mtxTmp._21 * mtx._13 + mtxTmp._22 * mtx._23 + mtxTmp._23 * mtx._33 + mtxTmp._24 * mtx._43;
	_24 = mtxTmp._21 * mtx._14 + mtxTmp._22 * mtx._24 + mtxTmp._23 * mtx._34 + mtxTmp._24 * mtx._44;

	_31 = mtxTmp._31 * mtx._11 + mtxTmp._32 * mtx._21 + mtxTmp._33 * mtx._31 + mtxTmp._34 * mtx._41;
	_32 = mtxTmp._31 * mtx._12 + mtxTmp._32 * mtx._22 + mtxTmp._33 * mtx._32 + mtxTmp._34 * mtx._42;
	_33 = mtxTmp._31 * mtx._13 + mtxTmp._32 * mtx._23 + mtxTmp._33 * mtx._33 + mtxTmp._34 * mtx._43;
	_34 = mtxTmp._31 * mtx._14 + mtxTmp._32 * mtx._24 + mtxTmp._33 * mtx._34 + mtxTmp._34 * mtx._44;

	_41 = mtxTmp._41 * mtx._11 + mtxTmp._42 * mtx._21 + mtxTmp._43 * mtx._31 + mtxTmp._44 * mtx._41;
	_42 = mtxTmp._41 * mtx._12 + mtxTmp._42 * mtx._22 + mtxTmp._43 * mtx._32 + mtxTmp._44 * mtx._42;
	_43 = mtxTmp._41 * mtx._13 + mtxTmp._42 * mtx._23 + mtxTmp._43 * mtx._33 + mtxTmp._44 * mtx._43;
	_44 = mtxTmp._41 * mtx._14 + mtxTmp._42 * mtx._24 + mtxTmp._43 * mtx._34 + mtxTmp._44 * mtx._44;
}

void __Matrix44::operator += (const __Vector3& v)
{
	_41 += v.x;
	_42 += v.y;
	_43 += v.z;
}

void __Matrix44::operator -= (const __Vector3& v)
{
	_41 -= v.x;
	_42 -= v.y;
	_43 -= v.z;
}

#endif // SERVER_SHAREDSERVER_MATRIX44_INL

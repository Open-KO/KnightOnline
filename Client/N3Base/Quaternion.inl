#ifndef CLIENT_N3BASE_QUATERNION_INL
#define CLIENT_N3BASE_QUATERNION_INL

#pragma once

#include "My_3DStruct.h"

__Quaternion::__Quaternion(const D3DXMATRIX& mtx)
{
	D3DXQuaternionRotationMatrix(this, &mtx);
}

__Quaternion::__Quaternion(const D3DXQUATERNION& qt)
{
	x = qt.x;
	y = qt.y;
	z = qt.z;
	w = qt.w;
}

void __Quaternion::Identity()
{
	x = y = z = 0;
	w = 1.0f;
}

void __Quaternion::Set(float fX, float fY, float fZ, float fW)
{
	x = fX;
	y = fY;
	z = fZ;
	w = fW;
}

void __Quaternion::RotationAxis(const __Vector3& v, float fRadian)
{
	D3DXQuaternionRotationAxis(this, &v, fRadian);
}

void __Quaternion::RotationAxis(float fX, float fY, float fZ, float fRadian)
{
	__Vector3 v(fX, fY, fZ);
	D3DXQuaternionRotationAxis(this, &v, fRadian);
}

void __Quaternion::operator = (const D3DXMATRIX& mtx)
{
	D3DXQuaternionRotationMatrix(this, &mtx);
}

void __Quaternion::AxisAngle(__Vector3& vAxisResult, float& fRadianResult) const
{
	D3DXQuaternionToAxisAngle(this, &vAxisResult, &fRadianResult);
}

void __Quaternion::Slerp(const D3DXQUATERNION& qt1, const D3DXQUATERNION& qt2, float fDelta)
{
	D3DXQuaternionSlerp(this, &qt1, &qt2, fDelta);
}

void __Quaternion::RotationYawPitchRoll(float Yaw, float Pitch, float Roll)
{
	D3DXQuaternionRotationYawPitchRoll(this, Yaw, Pitch, Roll);
}

#endif // CLIENT_N3BASE_QUATERNION_INL

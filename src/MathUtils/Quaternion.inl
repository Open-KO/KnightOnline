#ifndef MATHUTILS_QUATERNION_INL
#define MATHUTILS_QUATERNION_INL

#pragma once

#include "MathUtils.h"
#include <cmath> // acosf(), cosf(), sinf(), sqrtf()

__Quaternion::__Quaternion(const __Matrix44& mtx) : x(0.0f), y(0.0f), z(0.0f), w(0.0f)
{
	this->operator=(mtx);
}

__Quaternion::__Quaternion(const __Quaternion& qt) : x(qt.x), y(qt.y), z(qt.z), w(qt.w)
{
}

__Quaternion::__Quaternion(float fX, float fY, float fZ, float fW) : x(fX), y(fY), z(fZ), w(fW)
{
}

void __Quaternion::Identity()
{
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
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
	__Vector3 temp = v;
	temp.Normalize();

	x = sinf(fRadian / 2.0f) * temp.x;
	y = sinf(fRadian / 2.0f) * temp.y;
	z = sinf(fRadian / 2.0f) * temp.z;
	w = cosf(fRadian / 2.0f);
}

void __Quaternion::RotationAxis(float fX, float fY, float fZ, float fRadian)
{
	__Vector3 temp = { fX, fY, fZ };
	temp.Normalize();

	x = sinf(fRadian / 2.0f) * temp.x;
	y = sinf(fRadian / 2.0f) * temp.y;
	z = sinf(fRadian / 2.0f) * temp.z;
	w = cosf(fRadian / 2.0f);
}

__Quaternion& __Quaternion::operator=(const __Matrix44& mtx)
{
	float trace = mtx.m[0][0] + mtx.m[1][1] + mtx.m[2][2] + 1.0f;
	if (trace > 1.0f)
	{
		float s = 2.0f * sqrtf(trace);
		x       = (mtx.m[1][2] - mtx.m[2][1]) / s;
		y       = (mtx.m[2][0] - mtx.m[0][2]) / s;
		z       = (mtx.m[0][1] - mtx.m[1][0]) / s;
		w       = 0.25f * s;
	}
	else
	{
		float s  = 0.0f;
		int maxi = 0;
		for (int i = 1; i < 3; i++)
		{
			if (mtx.m[i][i] > mtx.m[maxi][maxi])
				maxi = i;
		}

		switch (maxi)
		{
			case 0:
				s = 2.0f * sqrtf(1.0f + mtx.m[0][0] - mtx.m[1][1] - mtx.m[2][2]);
				x = 0.25f * s;
				y = (mtx.m[0][1] + mtx.m[1][0]) / s;
				z = (mtx.m[0][2] + mtx.m[2][0]) / s;
				w = (mtx.m[1][2] - mtx.m[2][1]) / s;
				break;

			case 1:
				s = 2.0f * sqrtf(1.0f + mtx.m[1][1] - mtx.m[0][0] - mtx.m[2][2]);
				x = (mtx.m[0][1] + mtx.m[1][0]) / s;
				y = 0.25f * s;
				z = (mtx.m[1][2] + mtx.m[2][1]) / s;
				w = (mtx.m[2][0] - mtx.m[0][2]) / s;
				break;

			case 2:
				s = 2.0f * sqrtf(1.0f + mtx.m[2][2] - mtx.m[0][0] - mtx.m[1][1]);
				x = (mtx.m[0][2] + mtx.m[2][0]) / s;
				y = (mtx.m[1][2] + mtx.m[2][1]) / s;
				z = 0.25f * s;
				w = (mtx.m[0][1] - mtx.m[1][0]) / s;
				break;

			default:
				break;
		}
	}

	return *this;
}

void __Quaternion::AxisAngle(__Vector3& vAxisResult, float& fRadianResult) const
{
	vAxisResult.x = x;
	vAxisResult.y = y;
	vAxisResult.z = z;

	fRadianResult = 2.0f * acosf(w);
}

void __Quaternion::Slerp(const __Quaternion& qt1, const __Quaternion& qt2, float fDelta)
{
	float temp = 1.0f - fDelta;
	float dot  = (qt1.x * qt2.x + qt1.y * qt2.y + qt1.z * qt2.z + qt1.w * qt2.w);

	if (dot < 0.0f)
	{
		fDelta = -fDelta;
		dot    = -dot;
	}

	if (1.0f - dot > 0.001f)
	{
		float theta = acosf(dot);

		temp        = sinf(theta * temp) / sinf(theta);
		fDelta      = sinf(theta * fDelta) / sinf(theta);
	}

	x = temp * qt1.x + fDelta * qt2.x;
	y = temp * qt1.y + fDelta * qt2.y;
	z = temp * qt1.z + fDelta * qt2.z;
	w = temp * qt1.w + fDelta * qt2.w;
}

void __Quaternion::RotationYawPitchRoll(float Yaw, float Pitch, float Roll)
{
	float syaw = 0.0f, cyaw = 0.0f, spitch = 0.0f, cpitch = 0.0f, sroll = 0.0f, croll = 0.0f;

	syaw   = sinf(Yaw / 2.0f);
	cyaw   = cosf(Yaw / 2.0f);
	spitch = sinf(Pitch / 2.0f);
	cpitch = cosf(Pitch / 2.0f);
	sroll  = sinf(Roll / 2.0f);
	croll  = cosf(Roll / 2.0f);

	x      = syaw * cpitch * sroll + cyaw * spitch * croll;
	y      = syaw * cpitch * croll - cyaw * spitch * sroll;
	z      = cyaw * cpitch * sroll - syaw * spitch * croll;
	w      = cyaw * cpitch * croll + syaw * spitch * sroll;
}

__Quaternion __Quaternion::operator*(const __Quaternion& q) const
{
	__Quaternion out {};
	out.x = q.w * x + q.x * w + q.y * z - q.z * y;
	out.y = q.w * y - q.x * z + q.y * w + q.z * x;
	out.z = q.w * z + q.x * y - q.y * x + q.z * w;
	out.w = q.w * w - q.x * x - q.y * y - q.z * z;
	return out;
}

void __Quaternion::operator*=(const __Quaternion& q)
{
	__Quaternion tmp = this->operator*(q);
	Set(tmp.x, tmp.y, tmp.z, tmp.w);
}
#endif // MATHUTILS_QUATERNION_INL

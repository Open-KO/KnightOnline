#ifndef MATHUTILS_MATRIX44_INL
#define MATHUTILS_MATRIX44_INL

#pragma once

#include "MathUtils.h"

#include <cmath>	// cosf(), sinf()
#include <cstring>	// std::memcpy(), std::memset()

__Matrix44::__Matrix44(const float mtx[4][4])
{
	std::memcpy(&m, mtx, sizeof(m));
}

__Matrix44::__Matrix44(const __Matrix44& mtx)
{
	std::memcpy(&m, &mtx.m, sizeof(m));
}

__Matrix44::__Matrix44(const __Quaternion& qt)
{
	m[0][0] = 1.0f - 2.0f * (qt.y * qt.y + qt.z * qt.z);
	m[0][1] = 2.0f * (qt.x * qt.y + qt.z * qt.w);
	m[0][2] = 2.0f * (qt.x * qt.z - qt.y * qt.w);
	m[0][3] = 0.0f;
	m[1][0] = 2.0f * (qt.x * qt.y - qt.z * qt.w);
	m[1][1] = 1.0f - 2.0f * (qt.x * qt.x + qt.z * qt.z);
	m[1][2] = 2.0f * (qt.y * qt.z + qt.x * qt.w);
	m[1][3] = 0.0f;
	m[2][0] = 2.0f * (qt.x * qt.z + qt.y * qt.w);
	m[2][1] = 2.0f * (qt.y * qt.z - qt.x * qt.w);
	m[2][2] = 1.0f - 2.0f * (qt.x * qt.x + qt.y * qt.y);
	m[2][3] = 0.0f;
	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 0.0f;
	m[3][3] = 1.0f;
}

void __Matrix44::Zero()
{
	std::memset(&m, 0, sizeof(m));
}

void __Matrix44::Identity()
{
	m[0][1] = m[0][2] = m[0][3] = 0.0f;
	m[1][0] = m[1][2] = m[1][3] = 0.0f;
	m[2][0] = m[2][1] = m[2][3] = 0.0f;
	m[3][0] = m[3][1] = m[3][2] = 0.0f;
	m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
}

__Matrix44 __Matrix44::Inverse() const
{
	__Matrix44 mtxOut;
	BuildInverse(mtxOut);
	return mtxOut;
}

void __Matrix44::BuildInverse(__Matrix44& mtxOut) const
{
	float t[3], v[16];

	t[0] = m[2][2] * m[3][3] - m[2][3] * m[3][2];
	t[1] = m[1][2] * m[3][3] - m[1][3] * m[3][2];
	t[2] = m[1][2] * m[2][3] - m[1][3] * m[2][2];
	v[0] = m[1][1] * t[0] - m[2][1] * t[1] + m[3][1] * t[2];
	v[4] = -m[1][0] * t[0] + m[2][0] * t[1] - m[3][0] * t[2];

	t[0] = m[1][0] * m[2][1] - m[2][0] * m[1][1];
	t[1] = m[1][0] * m[3][1] - m[3][0] * m[1][1];
	t[2] = m[2][0] * m[3][1] - m[3][0] * m[2][1];
	v[8] = m[3][3] * t[0] - m[2][3] * t[1] + m[1][3] * t[2];
	v[12] = -m[3][2] * t[0] + m[2][2] * t[1] - m[1][2] * t[2];

	float det = m[0][0] * v[0] + m[0][1] * v[4] + m[0][2] * v[8] + m[0][3] * v[12];
	if (det == 0.0f)
	{
		mtxOut.Identity();
		return;
	}

	t[0] = m[2][2] * m[3][3] - m[2][3] * m[3][2];
	t[1] = m[0][2] * m[3][3] - m[0][3] * m[3][2];
	t[2] = m[0][2] * m[2][3] - m[0][3] * m[2][2];
	v[1] = -m[0][1] * t[0] + m[2][1] * t[1] - m[3][1] * t[2];
	v[5] = m[0][0] * t[0] - m[2][0] * t[1] + m[3][0] * t[2];

	t[0] = m[0][0] * m[2][1] - m[2][0] * m[0][1];
	t[1] = m[3][0] * m[0][1] - m[0][0] * m[3][1];
	t[2] = m[2][0] * m[3][1] - m[3][0] * m[2][1];
	v[9] = -m[3][3] * t[0] - m[2][3] * t[1] - m[0][3] * t[2];
	v[13] = m[3][2] * t[0] + m[2][2] * t[1] + m[0][2] * t[2];

	t[0] = m[1][2] * m[3][3] - m[1][3] * m[3][2];
	t[1] = m[0][2] * m[3][3] - m[0][3] * m[3][2];
	t[2] = m[0][2] * m[1][3] - m[0][3] * m[1][2];
	v[2] = m[0][1] * t[0] - m[1][1] * t[1] + m[3][1] * t[2];
	v[6] = -m[0][0] * t[0] + m[1][0] * t[1] - m[3][0] * t[2];

	t[0] = m[0][0] * m[1][1] - m[1][0] * m[0][1];
	t[1] = m[3][0] * m[0][1] - m[0][0] * m[3][1];
	t[2] = m[1][0] * m[3][1] - m[3][0] * m[1][1];
	v[10] = m[3][3] * t[0] + m[1][3] * t[1] + m[0][3] * t[2];
	v[14] = -m[3][2] * t[0] - m[1][2] * t[1] - m[0][2] * t[2];

	t[0] = m[1][2] * m[2][3] - m[1][3] * m[2][2];
	t[1] = m[0][2] * m[2][3] - m[0][3] * m[2][2];
	t[2] = m[0][2] * m[1][3] - m[0][3] * m[1][2];
	v[3] = -m[0][1] * t[0] + m[1][1] * t[1] - m[2][1] * t[2];
	v[7] = m[0][0] * t[0] - m[1][0] * t[1] + m[2][0] * t[2];

	v[11]
		= -m[0][0] * (m[1][1] * m[2][3] - m[1][3] * m[2][1])
		+ m[1][0] * (m[0][1] * m[2][3] - m[0][3] * m[2][1])
		- m[2][0] * (m[0][1] * m[1][3] - m[0][3] * m[1][1]);

	v[15]
		= m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
		- m[1][0] * (m[0][1] * m[2][2] - m[0][2] * m[2][1])
		+ m[2][0] * (m[0][1] * m[1][2] - m[0][2] * m[1][1]);

	det = 1.0f / det;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			mtxOut.m[i][j] = v[4 * i + j] * det;
	}
}

const __Vector3 __Matrix44::Pos() const
{
	return { m[3][0], m[3][1], m[3][2] };
}

void __Matrix44::PosSet(float x, float y, float z)
{
	m[3][0] = x;
	m[3][1] = y;
	m[3][2] = z;
}

void __Matrix44::PosSet(const __Vector3& v)
{
	m[3][0] = v.x;
	m[3][1] = v.y;
	m[3][2] = v.z;
}

void __Matrix44::RotationX(float fDelta)
{
	Identity();

	m[1][1] = cosf(fDelta);
	m[1][2] = sinf(fDelta);
	m[2][1] = -m[1][2];
	m[2][2] = m[1][1];
}

void __Matrix44::RotationY(float fDelta)
{
	Identity();

	m[0][0] = cosf(fDelta);
	m[0][2] = -sinf(fDelta);
	m[2][0] = -m[0][2];
	m[2][2] = m[0][0];
}

void __Matrix44::RotationZ(float fDelta)
{
	Identity();

	m[0][0] = cosf(fDelta);
	m[0][1] = sinf(fDelta);
	m[1][0] = -m[0][1];
	m[1][1] = m[0][0];
}

void __Matrix44::Rotation(float fX, float fY, float fZ)
{
	float SX = sinf(fX), CX = cosf(fX);
	float SY = sinf(fY), CY = cosf(fY);
	float SZ = sinf(fZ), CZ = cosf(fZ);

	m[0][0] = CY * CZ;
	m[0][1] = CY * SZ;
	m[0][2] = -SY;
	m[0][3] = 0;

	m[1][0] = SX * SY * CZ - CX * SZ;
	m[1][1] = SX * SY * SZ + CX * CZ;
	m[1][2] = SX * CY;
	m[1][3] = 0;

	m[2][0] = CX * SY * CZ + SX * SZ;
	m[2][1] = CX * SY * SZ - SX * CZ;
	m[2][2] = CX * CY;
	m[2][3] = 0;

	m[3][0] = m[3][1] = m[3][2] = 0;
	m[3][3] = 1;
}

void __Matrix44::Rotation(const __Vector3& v)
{
	float SX = sinf(v.x), CX = cosf(v.x);
	float SY = sinf(v.y), CY = cosf(v.y);
	float SZ = sinf(v.z), CZ = cosf(v.z);

	m[0][0] = CY * CZ;
	m[0][1] = CY * SZ;
	m[0][2] = -SY;
	m[0][3] = 0;

	m[1][0] = SX * SY * CZ - CX * SZ;
	m[1][1] = SX * SY * SZ + CX * CZ;
	m[1][2] = SX * CY;
	m[1][3] = 0;

	m[2][0] = CX * SY * CZ + SX * SZ;
	m[2][1] = CX * SY * SZ - SX * CZ;
	m[2][2] = CX * CY;
	m[2][3] = 0;

	m[3][0] = m[3][1] = m[3][2] = 0;
	m[3][3] = 1;
}

void __Matrix44::Scale(float sx, float sy, float sz)
{
	Identity();

	m[0][0] = sx;
	m[1][1] = sy;
	m[2][2] = sz;
}

void __Matrix44::Scale(const __Vector3& v)
{
	Identity();

	m[0][0] = v.x;
	m[1][1] = v.y;
	m[2][2] = v.z;
}

void __Matrix44::Direction(const __Vector3& vDir)
{
	Identity();

	__Vector3 vDir2, vRight, vUp;
	vUp.Set(0, 1, 0);
	vDir2 = vDir;
	vDir2.Normalize();
	vRight.Cross(vUp, vDir2); // right = CrossProduct(world_up, view_dir);
	vUp.Cross(vDir2, vRight); // up = CrossProduct(view_dir, right);
	vRight.Normalize(); // right = Normalize(right);
	vUp.Normalize(); // up = Normalize(up);

	m[0][0] = vRight.x; // view(0, 0) = right.x;
	m[1][0] = vRight.y; // view(1, 0) = right.y;
	m[2][0] = vRight.z; // view(2, 0) = right.z;
	m[0][1] = vUp.x; // view(0, 1) = up.x;
	m[1][1] = vUp.y; // view(1, 1) = up.y;
	m[2][1] = vUp.z; // view(2, 1) = up.z;
	m[0][2] = vDir2.x; // view(0, 2) = view_dir.x;
	m[1][2] = vDir2.y; // view(1, 2) = view_dir.y;
	m[2][2] = vDir2.z; // view(2, 2) = view_dir.z;

	BuildInverse(*this);

//  view(3, 0) = -DotProduct(right, from);
//  view(3, 1) = -DotProduct(up, from);
//  view(3, 2) = -DotProduct(view_dir, from);

	// Set roll
//	if (roll != 0.0f) {
//		view = MatrixMult(RotateZMatrix(-roll), view);
//	}

//  return view;
//} // end ViewMatrix
}

void __Matrix44::LookAtLH(const __Vector3& vEye, const __Vector3& vAt, const __Vector3& vUp)
{
	__Vector3 right, upn, vec;

	vec = vAt - vEye;
	vec.Normalize();

	right.Cross(vUp, vec);
	upn.Cross(vec, right);

	right.Normalize();
	upn.Normalize();

	m[0][0] = right.x;
	m[1][0] = right.y;
	m[2][0] = right.z;
	m[3][0] = -right.Dot(vEye);
	m[0][1] = upn.x;
	m[1][1] = upn.y;
	m[2][1] = upn.z;
	m[3][1] = -upn.Dot(vEye);
	m[0][2] = vec.x;
	m[1][2] = vec.y;
	m[2][2] = vec.z;
	m[3][2] = -vec.Dot(vEye);
	m[0][3] = 0.0f;
	m[1][3] = 0.0f;
	m[2][3] = 0.0f;
	m[3][3] = 1.0f;
}

void __Matrix44::OrthoLH(float w, float h, float zn, float zf)
{
	Identity();

	m[0][0] = 2.0f / w;
	m[1][1] = 2.0f / h;
	m[2][2] = 1.0f / (zf - zn);
	m[3][2] = zn / (zn - zf);
}

void __Matrix44::PerspectiveFovLH(float fovy, float Aspect, float zn, float zf)
{
	Identity();

	m[0][0] = 1.0f / (Aspect * tanf(fovy / 2.0f));
	m[1][1] = 1.0f / tanf(fovy / 2.0f);
	m[2][2] = zf / (zf - zn);
	m[2][3] = 1.0f;
	m[3][2] = (zf * zn) / (zn - zf);
	m[3][3] = 0.0f;
}

__Matrix44 __Matrix44::operator * (const __Matrix44& mtx) const
{
	__Matrix44 mtxOut;

	mtxOut.m[0][0] = m[0][0] * mtx.m[0][0] + m[0][1] * mtx.m[1][0] + m[0][2] * mtx.m[2][0] + m[0][3] * mtx.m[3][0];
	mtxOut.m[0][1] = m[0][0] * mtx.m[0][1] + m[0][1] * mtx.m[1][1] + m[0][2] * mtx.m[2][1] + m[0][3] * mtx.m[3][1];
	mtxOut.m[0][2] = m[0][0] * mtx.m[0][2] + m[0][1] * mtx.m[1][2] + m[0][2] * mtx.m[2][2] + m[0][3] * mtx.m[3][2];
	mtxOut.m[0][3] = m[0][0] * mtx.m[0][3] + m[0][1] * mtx.m[1][3] + m[0][2] * mtx.m[2][3] + m[0][3] * mtx.m[3][3];

	mtxOut.m[1][0] = m[1][0] * mtx.m[0][0] + m[1][1] * mtx.m[1][0] + m[1][2] * mtx.m[2][0] + m[1][3] * mtx.m[3][0];
	mtxOut.m[1][1] = m[1][0] * mtx.m[0][1] + m[1][1] * mtx.m[1][1] + m[1][2] * mtx.m[2][1] + m[1][3] * mtx.m[3][1];
	mtxOut.m[1][2] = m[1][0] * mtx.m[0][2] + m[1][1] * mtx.m[1][2] + m[1][2] * mtx.m[2][2] + m[1][3] * mtx.m[3][2];
	mtxOut.m[1][3] = m[1][0] * mtx.m[0][3] + m[1][1] * mtx.m[1][3] + m[1][2] * mtx.m[2][3] + m[1][3] * mtx.m[3][3];

	mtxOut.m[2][0] = m[2][0] * mtx.m[0][0] + m[2][1] * mtx.m[1][0] + m[2][2] * mtx.m[2][0] + m[2][3] * mtx.m[3][0];
	mtxOut.m[2][1] = m[2][0] * mtx.m[0][1] + m[2][1] * mtx.m[1][1] + m[2][2] * mtx.m[2][1] + m[2][3] * mtx.m[3][1];
	mtxOut.m[2][2] = m[2][0] * mtx.m[0][2] + m[2][1] * mtx.m[1][2] + m[2][2] * mtx.m[2][2] + m[2][3] * mtx.m[3][2];
	mtxOut.m[2][3] = m[2][0] * mtx.m[0][3] + m[2][1] * mtx.m[1][3] + m[2][2] * mtx.m[2][3] + m[2][3] * mtx.m[3][3];

	mtxOut.m[3][0] = m[3][0] * mtx.m[0][0] + m[3][1] * mtx.m[1][0] + m[3][2] * mtx.m[2][0] + m[3][3] * mtx.m[3][0];
	mtxOut.m[3][1] = m[3][0] * mtx.m[0][1] + m[3][1] * mtx.m[1][1] + m[3][2] * mtx.m[2][1] + m[3][3] * mtx.m[3][1];
	mtxOut.m[3][2] = m[3][0] * mtx.m[0][2] + m[3][1] * mtx.m[1][2] + m[3][2] * mtx.m[2][2] + m[3][3] * mtx.m[3][2];
	mtxOut.m[3][3] = m[3][0] * mtx.m[0][3] + m[3][1] * mtx.m[1][3] + m[3][2] * mtx.m[2][3] + m[3][3] * mtx.m[3][3];

	return mtxOut;
}

void __Matrix44::operator *= (const __Matrix44& mtx)
{
	__Matrix44 tmp(*this);

	m[0][0] = tmp.m[0][0] * mtx.m[0][0] + tmp.m[0][1] * mtx.m[1][0] + tmp.m[0][2] * mtx.m[2][0] + tmp.m[0][3] * mtx.m[3][0];
	m[0][1] = tmp.m[0][0] * mtx.m[0][1] + tmp.m[0][1] * mtx.m[1][1] + tmp.m[0][2] * mtx.m[2][1] + tmp.m[0][3] * mtx.m[3][1];
	m[0][2] = tmp.m[0][0] * mtx.m[0][2] + tmp.m[0][1] * mtx.m[1][2] + tmp.m[0][2] * mtx.m[2][2] + tmp.m[0][3] * mtx.m[3][2];
	m[0][3] = tmp.m[0][0] * mtx.m[0][3] + tmp.m[0][1] * mtx.m[1][3] + tmp.m[0][2] * mtx.m[2][3] + tmp.m[0][3] * mtx.m[3][3];

	m[1][0] = tmp.m[1][0] * mtx.m[0][0] + tmp.m[1][1] * mtx.m[1][0] + tmp.m[1][2] * mtx.m[2][0] + tmp.m[1][3] * mtx.m[3][0];
	m[1][1] = tmp.m[1][0] * mtx.m[0][1] + tmp.m[1][1] * mtx.m[1][1] + tmp.m[1][2] * mtx.m[2][1] + tmp.m[1][3] * mtx.m[3][1];
	m[1][2] = tmp.m[1][0] * mtx.m[0][2] + tmp.m[1][1] * mtx.m[1][2] + tmp.m[1][2] * mtx.m[2][2] + tmp.m[1][3] * mtx.m[3][2];
	m[1][3] = tmp.m[1][0] * mtx.m[0][3] + tmp.m[1][1] * mtx.m[1][3] + tmp.m[1][2] * mtx.m[2][3] + tmp.m[1][3] * mtx.m[3][3];

	m[2][0] = tmp.m[2][0] * mtx.m[0][0] + tmp.m[2][1] * mtx.m[1][0] + tmp.m[2][2] * mtx.m[2][0] + tmp.m[2][3] * mtx.m[3][0];
	m[2][1] = tmp.m[2][0] * mtx.m[0][1] + tmp.m[2][1] * mtx.m[1][1] + tmp.m[2][2] * mtx.m[2][1] + tmp.m[2][3] * mtx.m[3][1];
	m[2][2] = tmp.m[2][0] * mtx.m[0][2] + tmp.m[2][1] * mtx.m[1][2] + tmp.m[2][2] * mtx.m[2][2] + tmp.m[2][3] * mtx.m[3][2];
	m[2][3] = tmp.m[2][0] * mtx.m[0][3] + tmp.m[2][1] * mtx.m[1][3] + tmp.m[2][2] * mtx.m[2][3] + tmp.m[2][3] * mtx.m[3][3];

	m[3][0] = tmp.m[3][0] * mtx.m[0][0] + tmp.m[3][1] * mtx.m[1][0] + tmp.m[3][2] * mtx.m[2][0] + tmp.m[3][3] * mtx.m[3][0];
	m[3][1] = tmp.m[3][0] * mtx.m[0][1] + tmp.m[3][1] * mtx.m[1][1] + tmp.m[3][2] * mtx.m[2][1] + tmp.m[3][3] * mtx.m[3][1];
	m[3][2] = tmp.m[3][0] * mtx.m[0][2] + tmp.m[3][1] * mtx.m[1][2] + tmp.m[3][2] * mtx.m[2][2] + tmp.m[3][3] * mtx.m[3][2];
	m[3][3] = tmp.m[3][0] * mtx.m[0][3] + tmp.m[3][1] * mtx.m[1][3] + tmp.m[3][2] * mtx.m[2][3] + tmp.m[3][3] * mtx.m[3][3];
}

void __Matrix44::operator += (const __Vector3& v)
{
	m[3][0] += v.x;
	m[3][1] += v.y;
	m[3][2] += v.z;
}

void __Matrix44::operator -= (const __Vector3& v)
{
	m[3][0] -= v.x;
	m[3][1] -= v.y;
	m[3][2] -= v.z;
}

__Matrix44 __Matrix44::operator * (const __Quaternion& qRot) const
{
	__Matrix44 mtx;
	mtx.operator = (qRot);
	return *this * mtx;
}

void __Matrix44::operator *= (const __Quaternion& qRot)
{
	__Matrix44 mtx(qRot);
	*this *= mtx;
}

void __Matrix44::operator = (const __Quaternion& qt)
{
	m[0][0] = 1.0f - 2.0f * (qt.y * qt.y + qt.z * qt.z);
	m[0][1] = 2.0f * (qt.x * qt.y + qt.z * qt.w);
	m[0][2] = 2.0f * (qt.x * qt.z - qt.y * qt.w);
	m[0][3] = 0.0f;
	m[1][0] = 2.0f * (qt.x * qt.y - qt.z * qt.w);
	m[1][1] = 1.0f - 2.0f * (qt.x * qt.x + qt.z * qt.z);
	m[1][2] = 2.0f * (qt.y * qt.z + qt.x * qt.w);
	m[1][3] = 0.0f;
	m[2][0] = 2.0f * (qt.x * qt.z + qt.y * qt.w);
	m[2][1] = 2.0f * (qt.y * qt.z - qt.x * qt.w);
	m[2][2] = 1.0f - 2.0f * (qt.x * qt.x + qt.y * qt.y);
	m[2][3] = 0.0f;
	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 0.0f;
	m[3][3] = 1.0f;
}

#endif // MATHUTILS_MATRIX44_INL

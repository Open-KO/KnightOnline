#ifndef MATHUTILS_MATRIX44_H
#define MATHUTILS_MATRIX44_H

#pragma once

struct _D3DMATRIX;
struct __Quaternion;
struct __Vector3;

// 4x4 matrix
struct __Matrix44
{
public:
	__Matrix44() = default;
	__Matrix44(const float mtx[4][4]);
	__Matrix44(const __Matrix44& mtx);
	__Matrix44(const __Quaternion& qt);

	_D3DMATRIX* toD3D()
	{
		return reinterpret_cast<_D3DMATRIX*>(this);
	}

	const _D3DMATRIX* toD3D() const
	{
		return reinterpret_cast<const _D3DMATRIX*>(this);
	}

	void Zero();
	void Identity();
	static __Matrix44 GetIdentity();
	__Matrix44 Inverse() const;
	void BuildInverse(__Matrix44& mtxOut) const;
	const __Vector3 Pos() const;
	void PosSet(float x, float y, float z);
	void PosSet(const __Vector3& v);
	void RotationX(float fDelta);
	void RotationY(float fDelta);
	void RotationZ(float fDelta);
	void Rotation(float fX, float fY, float fZ);
	void Rotation(const __Vector3& v);
	void Scale(float sx, float sy, float sz);
	void Scale(const __Vector3& v);
	void Direction(const __Vector3& vDir);
	void LookAtLH(const __Vector3& vEye, const __Vector3& vAt, const __Vector3& vUp);
	void OrthoLH(float w, float h, float zn, float zf);
	void PerspectiveFovLH(float fovy, float Aspect, float zn, float zf);

	__Matrix44 operator*(const __Matrix44& mtx) const;
	void operator*=(const __Matrix44& mtx);
	void operator+=(const __Vector3& v);
	void operator-=(const __Vector3& v);

	__Matrix44 operator*(const __Quaternion& qRot) const;
	void operator*=(const __Quaternion& qRot);

	__Matrix44& operator=(const __Quaternion& qt);

public:
	float m[4][4];
};

#endif // MATHUTILS_MATRIX44_H

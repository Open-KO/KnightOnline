#ifndef SERVER_SHAREDSERVER_MY_3DSTRUCT_H
#define SERVER_SHAREDSERVER_MY_3DSTRUCT_H

#pragma once

#include <inttypes.h>
#include <stdint.h>
#include <string>
#include <math.h>

constexpr float __PI = 3.141592654f;
constexpr float __PI2 = 6.283185308f;

constexpr float DegreesToRadians(float degrees)
{
	return degrees * (__PI / 180.0f);
}

constexpr float RadiansToDegrees(float radians)
{
	return radians * (180.0f / __PI);
}

struct __Vector2;
struct __Vector3;
struct __Matrix44;
struct __Quaternion;

// 2D Vertex
struct __Vector2
{
public:
	float x, y;

	__Vector2() = default;
	__Vector2(float fx, float fy);
	void Zero();
	void Set(float fx, float fy);
};

// 3D Vertex
struct __Vector3
{
public:
	float x, y, z;

	__Vector3() = default;
	__Vector3(float fx, float fy, float fz);
	__Vector3(const __Vector3& vec);

	void	Normalize();
	float	Magnitude() const;
	float	Dot(const __Vector3& vec) const;
	void	Cross(const __Vector3& v1, const __Vector3& v2);
	void	Absolute();

	void Zero();
	void Set(float fx, float fy, float fz);

	const __Vector3& operator = (const __Vector3& vec);

	const __Vector3 operator * (const __Matrix44& mtx) const;
	void operator *= (float fDelta);
	void operator *= (const __Matrix44& mtx);
	__Vector3 operator + (const __Vector3& vec) const;
	__Vector3 operator - (const __Vector3& vec) const;
	__Vector3 operator * (const __Vector3& vec) const;
	__Vector3 operator / (const __Vector3& vec) const;

	void operator += (const __Vector3& vec);
	void operator -= (const __Vector3& vec);
	void operator *= (const __Vector3& vec);
	void operator /= (const __Vector3& vec);

	__Vector3 operator + (float fDelta) const;
	__Vector3 operator - (float fDelta) const;
	__Vector3 operator * (float fDelta) const;
	__Vector3 operator / (float fDelta) const;
};

// 4x4 Matrix
struct __Matrix44
{
public:
	union
	{
		struct
		{
			float        _11, _12, _13, _14;
			float        _21, _22, _23, _24;
			float        _31, _32, _33, _34;
			float        _41, _42, _43, _44;
		};

		float m[4][4];
	};

	__Matrix44() = default;
	__Matrix44(const __Matrix44& mtx);
	__Matrix44(const __Quaternion& qt);
	void Zero();
	void Identity();
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

	bool Inverse(float* pdeterminant = nullptr);
	bool BuildInverse(__Matrix44* mtxOut, float* pdeterminant) const;

	__Matrix44 operator * (const __Matrix44& mtx);
	void operator *= (const __Matrix44& mtx);
	void operator += (const __Vector3& v);
	void operator -= (const __Vector3& v);

	__Matrix44 operator * (const __Quaternion& qRot);
	void operator *= (const __Quaternion& qRot);
	void operator = (const __Quaternion& qt);
};

struct __Quaternion
{
public:
	float x, y, z, w;

	__Quaternion() = default;
	__Quaternion(const __Matrix44& mtx);
	__Quaternion(const __Quaternion& qt);

	void Identity();
	void Set(const __Matrix44& mtx);
	void Set(float fX, float fY, float fZ, float fW);

	void RotationAxis(const __Vector3& v, float fRadian);
	void RotationAxis(float fX, float fY, float fZ, float fRadian);
	void operator = (const __Matrix44& mtx);

	void AxisAngle(__Vector3& vAxisResult, float& fRadianResult) const;
	void Slerp(const __Quaternion& qt1, const __Quaternion& qt2, float fDelta);
	float Dot(const __Quaternion& qt) const;
};

inline __Vector2::__Vector2(float fx, float fy)
{
	x = fx;
	y = fy;
}

inline void __Vector2::Zero()
{
	x = y = 0;
}

inline void __Vector2::Set(float fx, float fy)
{
	x = fx; 
	y = fy;
}

inline __Vector3::__Vector3(float fx, float fy, float fz)
{
	x = fx; y = fy, z = fz;
}

inline __Vector3::__Vector3(const __Vector3& vec)
{
	x = vec.x; y = vec.y; z = vec.z;
}

inline void	__Vector3::Normalize()
{
	float fn = sqrtf(x*x + y*y + z*z);
	if(fn == 0) return;
	x /= fn; y /= fn; z /= fn;
}

inline float __Vector3::Magnitude() const 
{
	return sqrtf(x*x + y*y + z*z);
}

inline float __Vector3::Dot(const __Vector3& vec) const 
{
	return x*vec.x + y*vec.y + z*vec.z;
}

inline void __Vector3::Cross(const __Vector3& v1, const __Vector3& v2)
{
	x = v1.y * v2.z - v1.z * v2.y;
	y = v1.z * v2.x - v1.x * v2.z;
	z = v1.x * v2.y - v1.y * v2.x;
}

inline void __Vector3::Absolute()
{
	if(x < 0) x *= -1.0f;
	if(y < 0) y *= -1.0f;
	if(z < 0) z *= -1.0f;
}

inline void __Vector3::Zero()
{
	x = y = z = 0;
}

inline void __Vector3::Set(float fx, float fy, float fz)
{
	x = fx; y = fy, z = fz;
}

inline const __Vector3& __Vector3::operator = (const __Vector3& vec)
{
	x = vec.x; y = vec.y; z = vec.z;
	return *this;
}

inline const __Vector3 __Vector3::operator * (const __Matrix44& mtx) const 
{
	__Vector3 vTmp;

	vTmp.x = x*mtx._11 + y*mtx._21 + z*mtx._31 + mtx._41;
	vTmp.y = x*mtx._12 + y*mtx._22 + z*mtx._32 + mtx._42;
	vTmp.z = x*mtx._13 + y*mtx._23 + z*mtx._33 + mtx._43;

	return vTmp;
}

inline void __Vector3::operator *= (float fDelta)
{
	x *= fDelta;
	y *= fDelta;
	z *= fDelta;
}

inline void __Vector3::operator *= (const __Matrix44& mtx)
{
	__Vector3 vTmp;

	vTmp.Set(x,y,z);
	x = vTmp.x*mtx._11 + vTmp.y*mtx._21 + vTmp.z*mtx._31 + mtx._41;
	y = vTmp.x*mtx._12 + vTmp.y*mtx._22 + vTmp.z*mtx._32 + mtx._42;
	z = vTmp.x*mtx._13 + vTmp.y*mtx._23 + vTmp.z*mtx._33 + mtx._43;
}

inline __Vector3 __Vector3::operator + (const __Vector3& vec) const
{
	__Vector3 vTmp;

	vTmp.x = x + vec.x;
	vTmp.y = y + vec.y;
	vTmp.z = z + vec.z;
	return vTmp;
}

inline __Vector3 __Vector3::operator - (const __Vector3& vec) const
{
	__Vector3 vTmp;

	vTmp.x = x - vec.x;
	vTmp.y = y - vec.y;
	vTmp.z = z - vec.z;
	return vTmp;
}

inline __Vector3 __Vector3::operator * (const __Vector3& vec) const
{
	__Vector3 vTmp;

	vTmp.x = x * vec.x;
	vTmp.y = y * vec.y;
	vTmp.z = z * vec.z;
	return vTmp;
}

inline __Vector3 __Vector3::operator / (const __Vector3& vec) const
{
	__Vector3 vTmp;

	vTmp.x = x / vec.x;
	vTmp.y = y / vec.y;
	vTmp.z = z / vec.z;
	return vTmp;
}

inline void __Vector3::operator += (const __Vector3& vec)
{
	x += vec.x;
	y += vec.y;
	z += vec.z;
}

inline void __Vector3::operator -= (const __Vector3& vec)
{
	x -= vec.x;
	y -= vec.y;
	z -= vec.z;
}

inline void __Vector3::operator *= (const __Vector3& vec)
{
	x *= vec.x;
	y *= vec.y;
	z *= vec.z;
}

inline void __Vector3::operator /= (const __Vector3& vec)
{
	x /= vec.x;
	y /= vec.y;
	z /= vec.z;
}

inline __Vector3 __Vector3::operator + (float fDelta) const 
{ 
	__Vector3 vTmp;

	vTmp.x = x + fDelta;
	vTmp.y = y + fDelta;
	vTmp.z = z + fDelta;
	return vTmp;
}

inline __Vector3 __Vector3::operator - (float fDelta) const 
{
	__Vector3 vTmp;

	vTmp.x = x - fDelta;
	vTmp.y = y - fDelta;
	vTmp.z = z - fDelta;
	return vTmp;
}

inline __Vector3 __Vector3::operator * (float fDelta) const 
{
	__Vector3 vTmp;

	vTmp.x = x * fDelta;
	vTmp.y = y * fDelta;
	vTmp.z = z * fDelta;
	return vTmp;
}

inline __Vector3 __Vector3::operator / (float fDelta) const 
{
	__Vector3 vTmp;

	vTmp.x = x / fDelta;
	vTmp.y = y / fDelta;
	vTmp.z = z / fDelta;
	return vTmp;
}

inline __Matrix44::__Matrix44(const __Matrix44& mtx)
{
	memcpy(&m, &mtx.m, sizeof(__Matrix44));
}

inline __Matrix44::__Matrix44(const __Quaternion& qt)
{
	Identity();

	m[0][0] = 1.0f - 2.0f * (qt.y * qt.y + qt.z * qt.z);
	m[0][1] = 2.0f * (qt.x * qt.y + qt.z * qt.w);
	m[0][2] = 2.0f * (qt.x * qt.z - qt.y * qt.w);
	m[1][0] = 2.0f * (qt.x * qt.y - qt.z * qt.w);
	m[1][1] = 1.0f - 2.0f * (qt.x * qt.x + qt.z * qt.z);
	m[1][2] = 2.0f * (qt.y * qt.z + qt.x * qt.w);
	m[2][0] = 2.0f * (qt.x * qt.z + qt.y * qt.w);
	m[2][1] = 2.0f * (qt.y * qt.z - qt.x * qt.w);
	m[2][2] = 1.0f - 2.0f * (qt.x * qt.x + qt.y * qt.y);
}

inline void __Matrix44::Zero() 
{
	memset(&m, 0, sizeof(__Matrix44));
}

inline void __Matrix44::Identity()
{
	_12 = _13 = _14 = _21 = _23 = _24 = _31 = _32 = _34 = _41 = _42 = _43 = 0;
	_11 = _22 = _33 = _44 = 1.0f;
}

inline const __Vector3 __Matrix44::Pos() const 
{
	__Vector3 vTmp;

	vTmp.Set(_41, _42, _43);
	return vTmp;
}

inline void __Matrix44::PosSet(float x, float y, float z)
{
	_41 = x; _42 = y; _43 = z;
}

inline void __Matrix44::PosSet(const __Vector3& v) 
{
	_41 = v.x;
	_42 = v.y;
	_43 = v.z;
}

inline void __Matrix44::RotationX(float fDelta)
{
	Identity();
	_22 = cosf(fDelta); _23 = sinf(fDelta); _32 = -_23; _33 = _22;
}

inline void __Matrix44::RotationY(float fDelta)
{
	Identity();
	_11 = cosf(fDelta); _13 = -sinf(fDelta); _31 = -_13; _33 = _11;
}

inline void __Matrix44::RotationZ(float fDelta)
{
	Identity();
	_11 = cosf(fDelta); _12 = sinf(fDelta); _21 = -_12; _22 = _11;
}

inline void __Matrix44::Rotation(float fX, float fY, float fZ)
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

inline void __Matrix44::Rotation(const __Vector3& v)
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

inline void __Matrix44::Scale(float sx, float sy, float sz) 
{
	Identity();
	_11 = sx; _22 = sy; _33 = sz;
}

inline void __Matrix44::Scale(const __Vector3& v)
{
	Identity();
	_11 = v.x; _22 = v.y; _33 = v.z;
}

inline __Matrix44 __Matrix44::operator * (const __Matrix44& mtx)
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

	// 최적화 된 코드..	
	// dino 막음.. 아래 코드는 4번째 행들의 계산을 생략하여서 부정확한 계산을 한다.
	// 보통 4번째 행이 (0, 0, 0, 1)인 matrix를 쓰지만 projection matrix의 경우
	// (0, 0, 1, 0)인 matrix를 쓰므로 이상한 결과를 초래한다.
//	mtxTmp._11 = _11 * mtx._11 + _12 * mtx._21 + _13 * mtx._31;
//	mtxTmp._12 = _11 * mtx._12 + _12 * mtx._22 + _13 * mtx._32;
//	mtxTmp._13 = _11 * mtx._13 + _12 * mtx._23 + _13 * mtx._33;
//	mtxTmp._14 = 0;

//	mtxTmp._21 = _21 * mtx._11 + _22 * mtx._21 + _23 * mtx._31;
//	mtxTmp._22 = _21 * mtx._12 + _22 * mtx._22 + _23 * mtx._32;
//	mtxTmp._23 = _21 * mtx._13 + _22 * mtx._23 + _23 * mtx._33;
//	mtxTmp._24 = 0;

//	mtxTmp._31 = _31 * mtx._11 + _32 * mtx._21 + _33 * mtx._31;
//	mtxTmp._32 = _31 * mtx._12 + _32 * mtx._22 + _33 * mtx._32;
//	mtxTmp._33 = _31 * mtx._13 + _32 * mtx._23 + _33 * mtx._33;
//	mtxTmp._34 = 0;

//	mtxTmp._41 = _41 * mtx._11 + _42 * mtx._21 + _43 * mtx._31 + mtx._41;
//	mtxTmp._42 = _41 * mtx._12 + _42 * mtx._22 + _43 * mtx._32 + mtx._42;
//	mtxTmp._43 = _41 * mtx._13 + _42 * mtx._23 + _43 * mtx._33 + mtx._43;
//	mtxTmp._44 = 1.0f;

	return mtxTmp;
}

inline void __Matrix44::operator *= (const __Matrix44& mtx)
{
	__Matrix44 mtxTmp;

	memcpy(&mtxTmp.m, &m, sizeof(__Matrix44));

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

	// dino 막음.. 아래 코드는 4번째 행들의 계산을 생략하여서 부정확한 계산을 한다.
	// 보통 4번째 행이 (0, 0, 0, 1)인 matrix를 쓰지만 projection matrix의 경우
	// (0, 0, 1, 0)인 matrix를 쓰므로 이상한 결과를 초래한다.
//	_11 = mtxTmp._11 * mtx._11 + mtxTmp._12 * mtx._21 + mtxTmp._13 * mtx._31;
//	_12 = mtxTmp._11 * mtx._12 + mtxTmp._12 * mtx._22 + mtxTmp._13 * mtx._32;
//	_13 = mtxTmp._11 * mtx._13 + mtxTmp._12 * mtx._23 + mtxTmp._13 * mtx._33;
//	_14 = 0;

//	_21 = mtxTmp._21 * mtx._11 + mtxTmp._22 * mtx._21 + mtxTmp._23 * mtx._31 + mtxTmp._24 * mtx._41;
//	_22 = mtxTmp._21 * mtx._12 + mtxTmp._22 * mtx._22 + mtxTmp._23 * mtx._32 + mtxTmp._24 * mtx._42;
//	_23 = mtxTmp._21 * mtx._13 + mtxTmp._22 * mtx._23 + mtxTmp._23 * mtx._33 + mtxTmp._24 * mtx._43;
//	_24 = 0;

//	_31 = mtxTmp._31 * mtx._11 + mtxTmp._32 * mtx._21 + mtxTmp._33 * mtx._31 + mtxTmp._34 * mtx._41;
//	_32 = mtxTmp._31 * mtx._12 + mtxTmp._32 * mtx._22 + mtxTmp._33 * mtx._32 + mtxTmp._34 * mtx._42;
//	_33 = mtxTmp._31 * mtx._13 + mtxTmp._32 * mtx._23 + mtxTmp._33 * mtx._33 + mtxTmp._34 * mtx._43;
//	_34 = 0;

//	_41 = mtxTmp._41 * mtx._11 + mtxTmp._42 * mtx._21 + mtxTmp._43 * mtx._31 + mtx._41;
//	_42 = mtxTmp._41 * mtx._12 + mtxTmp._42 * mtx._22 + mtxTmp._43 * mtx._32 + mtx._42;
//	_43 = mtxTmp._41 * mtx._13 + mtxTmp._42 * mtx._23 + mtxTmp._43 * mtx._33 + mtx._43;
//	_44 = 1;
}

inline void __Matrix44::operator += (const __Vector3& v)
{
	_41 += v.x;
	_42 += v.y;
	_43 += v.z;
}

inline void __Matrix44::operator -= (const __Vector3& v)
{
	_41 -= v.x;
	_42 -= v.y;
	_43 -= v.z;
}

inline __Matrix44 __Matrix44::operator * (const __Quaternion& qRot)
{
	__Matrix44 mtx;
	mtx.operator = (qRot);

	return this->operator * (mtx);
}

inline void __Matrix44::operator *= (const __Quaternion& qRot)
{
	__Matrix44 mtx;
	mtx.operator = (qRot);

	this->operator *= (mtx);
}

inline void __Matrix44::operator = (const __Quaternion& qt)
{
	Identity();

	m[0][0] = 1.0f - 2.0f * (qt.y * qt.y + qt.z * qt.z);
	m[0][1] = 2.0f * (qt.x * qt.y + qt.z * qt.w);
	m[0][2] = 2.0f * (qt.x * qt.z - qt.y * qt.w);
	m[1][0] = 2.0f * (qt.x * qt.y - qt.z * qt.w);
	m[1][1] = 1.0f - 2.0f * (qt.x * qt.x + qt.z * qt.z);
	m[1][2] = 2.0f * (qt.y * qt.z + qt.x * qt.w);
	m[2][0] = 2.0f * (qt.x * qt.z + qt.y * qt.w);
	m[2][1] = 2.0f * (qt.y * qt.z - qt.x * qt.w);
	m[2][2] = 1.0f - 2.0f * (qt.x * qt.x + qt.y * qt.y);
}

inline void __Matrix44::Direction(const __Vector3& vDir)
{
	Identity();

	__Vector3 vDir2, vRight, vUp;
	vUp.Set(0,1,0);
	vDir2 = vDir;
	vDir2.Normalize();
	vRight.Cross(vUp, vDir2); // right = CrossProduct(world_up, view_dir);
	vUp.Cross(vDir2, vRight); // up = CrossProduct(view_dir, right);
	vRight.Normalize(); // right = Normalize(right);
	vUp.Normalize(); // up = Normalize(up);

	_11 = vRight.x; // view(0, 0) = right.x;
	_21 = vRight.y; // view(1, 0) = right.y;
	_31 = vRight.z; // view(2, 0) = right.z;
	_12 = vUp.x; // view(0, 1) = up.x;
	_22 = vUp.y; // view(1, 1) = up.y;
	_32 = vUp.z; // view(2, 1) = up.z;
	_13 = vDir2.x; // view(0, 2) = view_dir.x;
	_23 = vDir2.y; // view(1, 2) = view_dir.y;
	_33 = vDir2.z; // view(2, 2) = view_dir.z;

	Inverse();
	
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

inline bool __Matrix44::Inverse(float* pdeterminant /*= nullptr*/)
{
	__Matrix44 tmpMatrix(*this);
	if (!BuildInverse(&tmpMatrix, pdeterminant))
		return false;

	memcpy(&m, &tmpMatrix.m, sizeof(m));
	return true;
}

inline bool __Matrix44::BuildInverse(__Matrix44* mtxOut, float* pdeterminant) const
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
		return false;

	if (pdeterminant != nullptr)
		*pdeterminant = det;

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

	v[11] = -m[0][0] * (m[1][1] * m[2][3] - m[1][3] * m[2][1]) +
		m[1][0] * (m[0][1] * m[2][3] - m[0][3] * m[2][1]) -
		m[2][0] * (m[0][1] * m[1][3] - m[0][3] * m[1][1]);

	v[15] = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
		m[1][0] * (m[0][1] * m[2][2] - m[0][2] * m[2][1]) +
		m[2][0] * (m[0][1] * m[1][2] - m[0][2] * m[1][1]);

	det = 1.0f / det;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			mtxOut->m[i][j] = v[4 * i + j] * det;
	}

	return true;
}

inline __Quaternion::__Quaternion(const __Matrix44& mtx)
{
	Set(mtx);
}

inline __Quaternion::__Quaternion(const __Quaternion& qt)
{
	x = qt.x; y = qt.y; z = qt.z; w = qt.w;
}

inline void __Quaternion::Identity()
{
	x = y = z = 0; w = 1.0f;
}

inline void __Quaternion::Set(const __Matrix44& mtx)
{
	float s, trace = mtx.m[0][0] + mtx.m[1][1] + mtx.m[2][2] + 1.0f;
	if (trace > 1.0f)
	{
		s = 2.0f * sqrtf(trace);
		x = (mtx.m[1][2] - mtx.m[2][1]) / s;
		y = (mtx.m[2][0] - mtx.m[0][2]) / s;
		z = (mtx.m[0][1] - mtx.m[1][0]) / s;
		w = 0.25f * s;
	}
	else
	{
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
		}
	}
}

inline void __Quaternion::Set(float fX, float fY, float fZ, float fW)
{
	x = fX; y = fY; z = fZ; w = fW;
}

inline void __Quaternion::RotationAxis(const __Vector3& v, float fRadian)
{
	RotationAxis(v.x, v.y, v.z, fRadian);
}

inline void __Quaternion::RotationAxis(float fX, float fY, float fZ, float fRadian)
{
	__Vector3 v(fX, fY, fZ);
	v.Normalize();

	x = sinf(fRadian / 2.0f) * v.x;
	y = sinf(fRadian / 2.0f) * v.y;
	z = sinf(fRadian / 2.0f) * v.z;
	w = cosf(fRadian / 2.0f);
}

inline void __Quaternion::operator = (const __Matrix44& mtx)
{
	Set(mtx);
}

inline void __Quaternion::AxisAngle(__Vector3& vAxisResult, float& fRadianResult) const
{
	vAxisResult.x = x;
	vAxisResult.y = y;
	vAxisResult.z = z;

	fRadianResult = 2.0f * acosf(w);
}

inline void __Quaternion::Slerp(const __Quaternion& qt1, const __Quaternion& qt2, float fDelta)
{
	float temp = 1.0f - fDelta;
	float dot = qt1.Dot(qt2);
	if (dot < 0.0f)
	{
		fDelta = -fDelta;
		dot = -dot;
	}

	if ((1.0f - dot) > 0.00001f)
	{
		float theta = acosf(dot);
		temp = sinf(theta * temp) / sinf(theta);
		fDelta = sinf(theta * fDelta) / sinf(theta);
	}

	x = temp * qt1.x + fDelta * qt2.x;
	y = temp * qt1.y + fDelta * qt2.y;
	z = temp * qt1.z + fDelta * qt2.z;
	w = temp * qt1.w + fDelta * qt2.w;
}

inline float __Quaternion::Dot(const __Quaternion& qt) const
{
	return (x*qt.x) + (y*qt.y) + (z*qt.z) + (w*qt.w);
}

#include <cassert>
#include <spdlog/spdlog.h>

#ifndef _DEBUG
#define __ASSERT(expression, expressionMessage) (void)0
#else
#define __ASSERT(expression, expressionMessage) ASSERT_IMPL(#expression, expression, __FILE__, __LINE__, expressionMessage)

inline static void ASSERT_IMPL(const char* expressionString, bool expressionResult, const char* file, int line, const char* expressionMessage)
{
	if (!expressionResult)
		spdlog::error("Assertion failed: {}({}) - {} ({})", file, line, expressionMessage, expressionString);
}
#endif

bool			_IntersectTriangle(const __Vector3& vOrig, const __Vector3& vDir , const __Vector3& v0, const __Vector3& v1, const __Vector3& v2, float& fT, float& fU, float& fV, __Vector3* pVCol = nullptr);
bool			_IntersectTriangle(const __Vector3& vOrig, const __Vector3& vDir, const __Vector3& v0, const __Vector3& v1, const __Vector3& v2);
float			_Yaw2D(float fDirX, float fDirZ);

inline bool _IntersectTriangle(const __Vector3& vOrig, const __Vector3& vDir,
							  const __Vector3& v0, const __Vector3& v1, const __Vector3& v2,
							  float& fT, float& fU, float& fV, __Vector3* pVCol)
{
	// Find vectors for two edges sharing vert0
	__Vector3 vEdge1, vEdge2;

	vEdge1 = v1 - v0;
	vEdge2 = v2 - v0;

	// Begin calculating determinant - also used to calculate U parameter
	__Vector3 pVec;	float fDet;

//	By : Ecli666 ( On 2001-09-12 오전 10:39:01 )

	pVec.Cross(vEdge1, vEdge2);
	fDet = pVec.Dot(vDir);
	if (fDet > -0.0001f)
		return false;

//	~(By Ecli666 On 2001-09-12 오전 10:39:01 )

	pVec.Cross(vDir, vEdge2);

	// If determinant is near zero, ray lies in plane of triangle
	fDet = vEdge1.Dot(pVec);
	if (fDet < 0.0001f)		// 거의 0에 가까우면 삼각형 평면과 지나가는 선이 평행하다.
		return false;

	// Calculate distance from vert0 to ray origin
	__Vector3 tVec = vOrig - v0;

	// Calculate U parameter and test bounds
	fU = tVec.Dot(pVec);
	if (fU < 0.0f || fU > fDet)
		return false;

	// Prepare to test V parameter
	__Vector3 qVec;
	qVec.Cross(tVec, vEdge1);

	// Calculate V parameter and test bounds
	fV = qVec.Dot(vDir);
	if (fV < 0.0f || fU + fV > fDet)
		return false;

	// Calculate t, scale parameters, ray intersects triangle
	fT = qVec.Dot(vEdge2);

	float fInvDet = 1.0f / fDet;
	fT *= fInvDet;
	fU *= fInvDet;
	fV *= fInvDet;

	// t가 클수록 멀리 직선과 평면과 만나는 점이 멀다.
	// t*dir + orig 를 구하면 만나는 점을 구할 수 있다.
	// u와 v의 의미는 무엇일까?
	// 추측 : v0 (0,0), v1(1,0), v2(0,1) <괄호안은 (U, V)좌표> 이런식으로 어느 점에 가깝나 나타낸 것 같음
	//

	if (pVCol != nullptr)
		(*pVCol) = vOrig + (vDir * fT);	// 접점을 계산..

	// *t < 0 이면 뒤쪽...
	if (fT < 0.0f)
		return false;

	return true;
}

inline bool _IntersectTriangle(const __Vector3& vOrig, const __Vector3& vDir, const __Vector3& v0, const __Vector3& v1, const __Vector3& v2)
{
	// Find vectors for two edges sharing vert0
	// Begin calculating determinant - also used to calculate U parameter
	float fDet, fT, fU, fV;
	__Vector3 vEdge1, vEdge2, tVec, pVec, qVec;

	vEdge1 = v1 - v0;
	vEdge2 = v2 - v0;

//	By : Ecli666 ( On 2001-09-12 오전 10:39:01 )

	pVec.Cross(vEdge1, vEdge2);
	fDet = pVec.Dot(vDir);
	if (fDet > -0.0001f)
		return false;

//	~(By Ecli666 On 2001-09-12 오전 10:39:01 )

	pVec.Cross(vDir, vEdge2);

	// If determinant is near zero, ray lies in plane of triangle
	fDet = vEdge1.Dot(pVec);
	if (fDet < 0.0001f)		// 거의 0에 가까우면 삼각형 평면과 지나가는 선이 평행하다.
		return false;

	// Calculate distance from vert0 to ray origin
	tVec = vOrig - v0;

	// Calculate U parameter and test bounds
	fU = tVec.Dot(pVec);
	if (fU < 0.0f || fU > fDet)
		return false;

	// Prepare to test V parameter
	qVec.Cross(tVec, vEdge1);

	// Calculate V parameter and test bounds
	fV = qVec.Dot(vDir);
	if (fV < 0.0f || fU + fV > fDet)
		return false;

	// Calculate t, scale parameters, ray intersects triangle
	fT = qVec.Dot(vEdge2) / fDet;

	// *t < 0 이면 뒤쪽...
	if (fT < 0.0f)
		return false;

	return true;
}

inline float _Yaw2D(float fDirX, float fDirZ)
{
	////////////////////////////////
	// 방향을 구하고.. -> 회전할 값을 구하는 루틴이다..
	if (fDirX >= 0.0f)						// ^^
	{
		if (fDirZ >= 0.0f)
			return (float) (asin(fDirX));
		
		return (DegreesToRadians(90.0f) + (float) (acos(fDirX)));
	}
	else
	{
		if (fDirZ >= 0.0f)
			return (DegreesToRadians(270.0f) + (float) (acos(-fDirX)));

		return (DegreesToRadians(180.0f) + (float) (asin(-fDirX)));
	}
	// 방향을 구하고..
	////////////////////////////////
}

#endif // SERVER_SHAREDSERVER_MY_3DSTRUCT_H

#ifndef SERVER_SHAREDSERVER_MY_3DSTRUCT_H
#define SERVER_SHAREDSERVER_MY_3DSTRUCT_H

#pragma once

#include <inttypes.h>
#include <stdint.h>
#include <string>
#include <math.h>

constexpr float __PI = 3.141592654f;
constexpr float __PI2 = 6.283185308f;

constexpr float DegreesToRadians(auto degrees)
{
	return static_cast<float>(degrees) * (__PI / 180.0f);
}

constexpr float RadiansToDegrees(auto radians)
{
	return static_cast<float>(radians) * (180.0f / __PI);
}

struct __Vector3;
struct __Matrix44;

// 3D Vertex
struct __Vector3
{
public:
	float x, y, z;

	__Vector3() = default;
	__Vector3(float fx, float fy, float fz);
	__Vector3(const __Vector3& vec);

	void Normalize();
	float Magnitude() const;
	float Dot(const __Vector3& vec) const;
	void Cross(const __Vector3& v1, const __Vector3& v2);
	void Absolute();

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

	__Matrix44 operator * (const __Matrix44& mtx);
	void operator *= (const __Matrix44& mtx);
	void operator += (const __Vector3& v);
	void operator -= (const __Vector3& v);
};

#include <cassert>
#include <spdlog/spdlog.h>

#ifndef _DEBUG
#define __ASSERT(expression, expressionMessage) (void)0
#else
#define __ASSERT(expression, expressionMessage) ASSERT_IMPL(#expression, expression, __FILE__, __LINE__, expressionMessage)

void ASSERT_IMPL(const char* expressionString, bool expressionResult, const char* file, int line, const char* expressionMessage);
#endif

bool _IntersectTriangle(const __Vector3& vOrig, const __Vector3& vDir , const __Vector3& v0, const __Vector3& v1, const __Vector3& v2, float& fT, float& fU, float& fV, __Vector3* pVCol = nullptr);
bool _IntersectTriangle(const __Vector3& vOrig, const __Vector3& vDir, const __Vector3& v0, const __Vector3& v1, const __Vector3& v2);
float _Yaw2D(float fDirX, float fDirZ);

#endif // SERVER_SHAREDSERVER_MY_3DSTRUCT_H

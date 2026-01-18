#ifndef MATHUTILS_VECTOR3_H
#define MATHUTILS_VECTOR3_H

#pragma once

struct __Matrix44;

// 3D vertex
struct __Vector3
{
public:
	__Vector3() = default;
	__Vector3(const __Vector3& vec);
	__Vector3(float fx, float fy, float fz);

	void Normalize();
	void Normalize(const __Vector3& vec);
	float Magnitude() const;
	float Dot(const __Vector3& vec) const;
	void Cross(const __Vector3& v1, const __Vector3& v2);
	void Absolute();

	void Zero();
	void Set(float fx, float fy, float fz);

	__Vector3& operator=(const __Vector3& vec) = default;

	const __Vector3 operator*(const __Matrix44& mtx) const;
	void operator*=(float fDelta);
	void operator*=(const __Matrix44& mtx);
	__Vector3 operator+(const __Vector3& vec) const;
	__Vector3 operator-(const __Vector3& vec) const;
	__Vector3 operator*(const __Vector3& vec) const;
	__Vector3 operator/(const __Vector3& vec) const;

	void operator+=(const __Vector3& vec);
	void operator-=(const __Vector3& vec);
	void operator*=(const __Vector3& vec);
	void operator/=(const __Vector3& vec);

	__Vector3 operator+(float fDelta) const;
	__Vector3 operator-(float fDelta) const;
	__Vector3 operator*(float fDelta) const;
	__Vector3 operator/(float fDelta) const;

	bool operator==(const __Vector3& rhs) const;
	bool operator!=(const __Vector3& rhs) const;

public:
	float x, y, z;
};

#endif // MATHUTILS_VECTOR3_H

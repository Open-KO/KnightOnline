#ifndef MATHUTILS_VECTOR4_H
#define MATHUTILS_VECTOR4_H

#pragma once

struct __Vector3;

// 4D vertex
struct __Vector4
{
public:
	__Vector4() = default;
	__Vector4(float fx, float fy, float fz, float fw);
	void Zero();
	void Set(float fx, float fy, float fz, float fw);
	void Transform(const __Vector3& v, const __Matrix44& m);

	__Vector4& operator+=(const __Vector4&);
	__Vector4& operator-=(const __Vector4&);
	__Vector4& operator*=(float);
	__Vector4& operator/=(float);

	__Vector4 operator+(const __Vector4&) const;
	__Vector4 operator-(const __Vector4&) const;
	__Vector4 operator*(float) const;
	__Vector4 operator/(float) const;

public:
	float x, y, z, w;
};

#endif // MATHUTILS_VECTOR4_H

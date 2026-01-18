#ifndef MATHUTILS_VECTOR2_H
#define MATHUTILS_VECTOR2_H

#pragma once

// 2D vertex
struct __Vector2
{
public:
	__Vector2() = default;
	__Vector2(float fx, float fy);
	void Zero();
	void Set(float fx, float fy);

	__Vector2& operator+=(const __Vector2&);
	__Vector2& operator-=(const __Vector2&);
	__Vector2& operator*=(float);
	__Vector2& operator/=(float);

	__Vector2 operator+(const __Vector2&) const;
	__Vector2 operator-(const __Vector2&) const;
	__Vector2 operator*(float) const;
	__Vector2 operator/(float) const;

public:
	float x, y;
};

#endif // MATHUTILS_VECTOR2_H

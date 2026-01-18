#ifndef MATHUTILS_VECTOR2_INL
#define MATHUTILS_VECTOR2_INL

#pragma once

#include "MathUtils.h"

__Vector2::__Vector2(float fx, float fy) : x(fx), y(fy)
{
}

void __Vector2::Zero()
{
	x = y = 0;
}

void __Vector2::Set(float fx, float fy)
{
	x = fx;
	y = fy;
}

__Vector2& __Vector2::operator+=(const __Vector2& v)
{
	x += v.x;
	y += v.y;
	return *this;
}

__Vector2& __Vector2::operator-=(const __Vector2& v)
{
	x -= v.x;
	y -= v.y;
	return *this;
}

__Vector2& __Vector2::operator*=(float f)
{
	x *= f;
	y *= f;
	return *this;
}

__Vector2& __Vector2::operator/=(float f)
{
	float fInv  = 1.0f / f;
	x          *= fInv;
	y          *= fInv;
	return *this;
}

__Vector2 __Vector2::operator+(const __Vector2& v) const
{
	return { x + v.x, y + v.y };
}

__Vector2 __Vector2::operator-(const __Vector2& v) const
{
	return { x - v.x, y - v.y };
}

__Vector2 __Vector2::operator*(float f) const
{
	return { x * f, y * f };
}

__Vector2 __Vector2::operator/(float f) const
{
	float fInv = 1.0f / f;
	return { x * fInv, y * fInv };
}

#endif // MATHUTILS_VECTOR2_INL

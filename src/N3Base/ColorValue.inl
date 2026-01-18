#ifndef CLIENT_N3BASE_COLORVALUE_INL
#define CLIENT_N3BASE_COLORVALUE_INL

#pragma once

#include "My_3DStruct.h"

__ColorValue::__ColorValue(D3DCOLOR cr)
{
	*this = cr;
}

__ColorValue::__ColorValue(float r2, float g2, float b2, float a2)
{
	Set(r2, g2, b2, a2);
}

__ColorValue& __ColorValue::operator=(const D3DCOLORVALUE& cv)
{
	if (this == &cv)
		return *this;

	r = cv.r;
	g = cv.g;
	b = cv.b;
	a = cv.a;
	return *this;
}

__ColorValue& __ColorValue::operator=(D3DCOLOR cr)
{
	r = ((cr & 0x00ff0000) >> 16) / 255.0f;
	g = ((cr & 0x0000ff00) >> 8) / 255.0f;
	b = (cr & 0x000000ff) / 255.0f;
	a = ((cr & 0xff000000) >> 24) / 255.0f;
	return *this;
}

void __ColorValue::Set(float r2, float g2, float b2, float a2)
{
	r = r2;
	g = g2;
	b = b2;
	a = a2;
}

D3DCOLOR __ColorValue::ToD3DCOLOR() const
{
	return (((uint32_t) (a * 255.0f)) << 24) | (((uint32_t) (r * 255.0f)) << 16)
		   | (((uint32_t) (g * 255.0f)) << 8) | (((uint32_t) (b * 255.0f)));
}

void __ColorValue::operator+=(float fDelta)
{
	r += fDelta;
	g += fDelta;
	b += fDelta;
	a += fDelta;
}

void __ColorValue::operator-=(float fDelta)
{
	r -= fDelta;
	g -= fDelta;
	b -= fDelta;
	a -= fDelta;
}

void __ColorValue::operator*=(float fDelta)
{
	r *= fDelta;
	g *= fDelta;
	b *= fDelta;
	a *= fDelta;
}

void __ColorValue::operator/=(float fDelta)
{
	if (0 == fDelta)
		return;

	r /= fDelta;
	g /= fDelta;
	b /= fDelta;
	a /= fDelta;
}

D3DCOLORVALUE __ColorValue::operator+(float fDelta) const
{
	__ColorValue cv  = *this;
	cv.r            += fDelta;
	cv.g            += fDelta;
	cv.b            += fDelta;
	cv.a            += fDelta;
	return cv;
}

D3DCOLORVALUE __ColorValue::operator-(float fDelta) const
{
	__ColorValue cv  = *this;
	cv.r            -= fDelta;
	cv.g            -= fDelta;
	cv.b            -= fDelta;
	cv.a            -= fDelta;
	return cv;
}

D3DCOLORVALUE __ColorValue::operator*(float fDelta) const
{
	__ColorValue cv  = *this;
	cv.r            *= fDelta;
	cv.g            *= fDelta;
	cv.b            *= fDelta;
	cv.a            *= fDelta;
	return cv;
}

D3DCOLORVALUE __ColorValue::operator/(float fDelta) const
{
	__ColorValue cv  = *this;
	cv.r            /= fDelta;
	cv.g            /= fDelta;
	cv.b            /= fDelta;
	cv.a            /= fDelta;
	return cv;
}

void __ColorValue::operator+=(const D3DCOLORVALUE& cv)
{
	r += cv.r;
	g += cv.g;
	b += cv.b;
	a += cv.a;
}

void __ColorValue::operator-=(const D3DCOLORVALUE& cv)
{
	r -= cv.r;
	g -= cv.g;
	b -= cv.b;
	a -= cv.a;
}

void __ColorValue::operator*=(const D3DCOLORVALUE& cv)
{
	r *= cv.r;
	g *= cv.g;
	b *= cv.b;
	a *= cv.a;
}

void __ColorValue::operator/=(const D3DCOLORVALUE& cv)
{
	r /= cv.r;
	g /= cv.g;
	b /= cv.b;
	a /= cv.a;
}

D3DCOLORVALUE __ColorValue::operator+(const D3DCOLORVALUE& cv) const
{
	__ColorValue cv2(cv.r, cv.g, cv.b, cv.a);
	cv2 += cv;
	return cv2;
}

D3DCOLORVALUE __ColorValue::operator-(const D3DCOLORVALUE& cv) const
{
	__ColorValue cv2(cv.r, cv.g, cv.b, cv.a);
	cv2 -= cv;
	return cv2;
}

D3DCOLORVALUE __ColorValue::operator*(const D3DCOLORVALUE& cv) const
{
	__ColorValue cv2(cv.r, cv.g, cv.b, cv.a);
	cv2 *= cv;
	return cv2;
}

D3DCOLORVALUE __ColorValue::operator/(const D3DCOLORVALUE& cv) const
{
	__ColorValue cv2(cv.r, cv.g, cv.b, cv.a);
	cv2 /= cv;
	return cv2;
}

#endif // CLIENT_N3BASE_COLORVALUE_INL

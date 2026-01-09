#ifndef N3BASE_VERTEXTYPES_INL
#define N3BASE_VERTEXTYPES_INL

#pragma once

#include "My_3DStruct.h"

__VertexColor::__VertexColor(const __Vector3& p, D3DCOLOR sColor) : __Vector3(p), color(sColor)
{
}

__VertexColor::__VertexColor(float sx, float sy, float sz, D3DCOLOR sColor) :
	__Vector3(sx, sy, sz), color(sColor)
{
}

void __VertexColor::Set(const __Vector3& p, D3DCOLOR sColor)
{
	x     = p.x;
	y     = p.y;
	z     = p.z;
	color = sColor;
}

void __VertexColor::Set(float sx, float sy, float sz, D3DCOLOR sColor)
{
	x     = sx;
	y     = sy;
	z     = sz;
	color = sColor;
}

__VertexColor& __VertexColor::operator=(const __Vector3& vec)
{
	x = vec.x;
	y = vec.y;
	z = vec.z;
	return *this;
}

__VertexTransformedColor::__VertexTransformedColor(float sx, float sy, float sz, float srhw,
	D3DCOLOR sColor) : __Vector3(sx, sy, sz), rhw(srhw), color(sColor)
{
}

void __VertexTransformedColor::Set(float sx, float sy, float sz, float srhw, D3DCOLOR sColor)
{
	x     = sx;
	y     = sy;
	z     = sz;
	rhw   = srhw;
	color = sColor;
}

__VertexT1::__VertexT1(const __Vector3& p, const __Vector3& sn, float u, float v) :
	__Vector3(p), n(sn), tu(u), tv(v)
{
}

__VertexT1::__VertexT1(float sx, float sy, float sz, float snx, float sny, float snz, float stu,
	float stv) : __Vector3(sx, sy, sz), n(snx, sny, snz), tu(stu), tv(stv)
{
}

void __VertexT1::Set(const __Vector3& p, const __Vector3& sn, float u, float v)
{
	x  = p.x;
	y  = p.y;
	z  = p.z;
	n  = sn;
	tu = u;
	tv = v;
}

void __VertexT1::Set(
	float sx, float sy, float sz, float snx, float sny, float snz, float stu, float stv)
{
	x   = sx;
	y   = sy;
	z   = sz;
	n.x = snx;
	n.y = sny;
	n.z = snz;
	tu  = stu;
	tv  = stv;
}

__VertexT2::__VertexT2(const __Vector3& p, const __Vector3& sn, float u, float v, float u2,
	float v2) : __VertexT1(p, sn, u, v), tu2(u2), tv2(v2)
{
}

__VertexT2::__VertexT2(float sx, float sy, float sz, float snx, float sny, float snz, float stu,
	float stv, float stu2, float stv2) :
	__VertexT1(sx, sy, sz, snx, sny, snz, stu, stv), tu2(stu2), tv2(stv2)
{
}

void __VertexT2::Set(const __Vector3& p, const __Vector3& sn, float u, float v, float u2, float v2)
{
	x   = p.x;
	y   = p.y;
	z   = p.z;
	n   = sn;
	tu  = u;
	tv  = v;
	tu2 = u2;
	tv2 = v2;
}

void __VertexT2::Set(float sx, float sy, float sz, float snx, float sny, float snz, float stu,
	float stv, float stu2, float stv2)
{
	x   = sx;
	y   = sy;
	z   = sz;
	n.x = snx;
	n.y = sny;
	n.z = snz;
	tu  = stu;
	tv  = stv;
	tu2 = stu2;
	tv2 = stv2;
}

__VertexTransformed::__VertexTransformed(float sx, float sy, float sz, float srhw, D3DCOLOR sColor,
	float stu, float stv) : __Vector3(sx, sy, sz), rhw(srhw), color(sColor), tu(stu), tv(stv)
{
}

void __VertexTransformed::Set(
	float sx, float sy, float sz, float srhw, D3DCOLOR sColor, float stu, float stv)
{
	x     = sx;
	y     = sy;
	z     = sz;
	rhw   = srhw;
	color = sColor;
	tu    = stu;
	tv    = stv;
}

__VertexTransformedT2::__VertexTransformedT2(float sx, float sy, float sz, float srhw,
	D3DCOLOR sColor, float stu, float stv, float stu2, float stv2) :
	__VertexTransformed(sx, sy, sz, srhw, sColor, stu, stv), tu2(stu2), tv2(stv2)
{
}

void __VertexTransformedT2::Set(float sx, float sy, float sz, float srhw, D3DCOLOR sColor,
	float stu, float stv, float stu2, float stv2)
{
	x     = sx;
	y     = sy;
	z     = sz;
	rhw   = srhw;
	color = sColor;
	tu    = stu;
	tv    = stv;
	tu2   = stu2;
	tv2   = stv2;
}

__VertexXyzT1::__VertexXyzT1(const __Vector3& p, float u, float v) : __Vector3(p), tu(u), tv(v)
{
}

__VertexXyzT1::__VertexXyzT1(float sx, float sy, float sz, float u, float v) :
	__Vector3(sx, sy, sz), tu(u), tv(v)
{
}

void __VertexXyzT1::Set(const __Vector3& p, float u, float v)
{
	x  = p.x;
	y  = p.y;
	z  = p.z;
	tu = u;
	tv = v;
}

void __VertexXyzT1::Set(float sx, float sy, float sz, float u, float v)
{
	x  = sx;
	y  = sy;
	z  = sz;
	tu = u;
	tv = v;
}

__VertexXyzT1& __VertexXyzT1::operator=(const __Vector3& vec)
{
	x = vec.x;
	y = vec.y;
	z = vec.z;
	return *this;
}

__VertexXyzT2::__VertexXyzT2(const __Vector3& p, float u, float v, float u2, float v2) :
	__VertexXyzT1(p, u, v), tu2(u2), tv2(v2)
{
}

__VertexXyzT2::__VertexXyzT2(float sx, float sy, float sz, float u, float v, float u2, float v2) :
	__VertexXyzT1(sx, sy, sz, u, v), tu2(u2), tv2(v2)
{
}

void __VertexXyzT2::Set(const __Vector3& p, float u, float v, float u2, float v2)
{
	x   = p.x;
	y   = p.y;
	z   = p.z;
	tu  = u;
	tv  = v;
	tu2 = u2;
	tv2 = v2;
}

void __VertexXyzT2::Set(float sx, float sy, float sz, float u, float v, float u2, float v2)
{
	x   = sx;
	y   = sy;
	z   = sz;
	tu  = u;
	tv  = v;
	tu2 = u2;
	tv2 = v2;
}

__VertexXyzT2& __VertexXyzT2::operator=(const __Vector3& vec)
{
	x = vec.x;
	y = vec.y;
	z = vec.z;
	return *this;
}

__VertexXyzNormal::__VertexXyzNormal(const __Vector3& p, const __Vector3& sn) : __Vector3(p), n(sn)
{
}

__VertexXyzNormal::__VertexXyzNormal(float sx, float sy, float sz, float snx, float sny,
	float snz) : __Vector3(sx, sy, sz), n(snx, sny, snz)
{
}

void __VertexXyzNormal::Set(const __Vector3& p, const __Vector3& sn)
{
	x = p.x;
	y = p.y;
	z = p.z;
	n = sn;
}

void __VertexXyzNormal::Set(float sx, float sy, float sz, float snx, float sny, float snz)
{
	x   = sx;
	y   = sy;
	z   = sz;
	n.x = snx;
	n.y = sny;
	n.z = snz;
}

__VertexXyzNormal& __VertexXyzNormal::operator=(const __Vector3& vec)
{
	x = vec.x;
	y = vec.y;
	z = vec.z;
	return *this;
}

__VertexXyzColorT1::__VertexXyzColorT1(const __Vector3& p, D3DCOLOR sColor, float u, float v) :
	__Vector3(p)
{
	color = sColor;
	tu    = u;
	tv    = v;
}

__VertexXyzColorT1::__VertexXyzColorT1(float sx, float sy, float sz, D3DCOLOR sColor, float u,
	float v) : __Vector3(sx, sy, sz), color(sColor), tu(u), tv(v)
{
}

void __VertexXyzColorT1::Set(const __Vector3& p, D3DCOLOR sColor, float u, float v)
{
	x     = p.x;
	y     = p.y;
	z     = p.z;
	color = sColor;
	tu    = u;
	tv    = v;
}

void __VertexXyzColorT1::Set(float sx, float sy, float sz, D3DCOLOR sColor, float u, float v)
{
	x     = sx;
	y     = sy;
	z     = sz;
	color = sColor;
	tu    = u;
	tv    = v;
}

__VertexXyzColorT1& __VertexXyzColorT1::operator=(const __Vector3& vec)
{
	x = vec.x;
	y = vec.y;
	z = vec.z;
	return *this;
}

__VertexXyzColorT2::__VertexXyzColorT2(const __Vector3& p, D3DCOLOR sColor, float u, float v,
	float u2, float v2) : __VertexXyzColorT1(p, sColor, u, v), tu2(u2), tv2(v2)
{
}

__VertexXyzColorT2::__VertexXyzColorT2(float sx, float sy, float sz, D3DCOLOR sColor, float u,
	float v, float u2, float v2) : __VertexXyzColorT1(sx, sy, sz, sColor, u, v), tu2(u2), tv2(v2)
{
	x     = sx;
	y     = sy;
	z     = sz;
	color = sColor;
	tu    = u;
	tv    = v;
	tu2   = u2;
	tv2   = v2;
}

void __VertexXyzColorT2::Set(
	const __Vector3& p, D3DCOLOR sColor, float u, float v, float u2, float v2)
{
	x     = p.x;
	y     = p.y;
	z     = p.z;
	color = sColor;
	tu    = u;
	tv    = v;
	tu2   = u2;
	tv2   = v2;
}

void __VertexXyzColorT2::Set(
	float sx, float sy, float sz, D3DCOLOR sColor, float u, float v, float u2, float v2)
{
	x     = sx;
	y     = sy;
	z     = sz;
	color = sColor;
	tu    = u;
	tv    = v;
	tu2   = u2;
	tv2   = v2;
}

__VertexXyzColorT2& __VertexXyzColorT2::operator=(const __Vector3& vec)
{
	x = vec.x;
	y = vec.y;
	z = vec.z;
	return *this;
}

__VertexXyzColor::__VertexXyzColor(const __Vector3& p, D3DCOLOR sColor) :
	__Vector3(p), color(sColor)
{
}

__VertexXyzColor::__VertexXyzColor(float sx, float sy, float sz, D3DCOLOR sColor) :
	__Vector3(sx, sy, sz), color(sColor)
{
}

void __VertexXyzColor::Set(const __Vector3& p, D3DCOLOR sColor)
{
	x     = p.x;
	y     = p.y;
	z     = p.z;
	color = sColor;
}

void __VertexXyzColor::Set(float sx, float sy, float sz, D3DCOLOR sColor)
{
	x     = sx;
	y     = sy;
	z     = sz;
	color = sColor;
}

__VertexXyzColor& __VertexXyzColor::operator=(const __Vector3& vec)
{
	x = vec.x;
	y = vec.y;
	z = vec.z;
	return *this;
}

__VertexXyzNormalColor::__VertexXyzNormalColor(
	const __Vector3& p, const __Vector3& sn, D3DCOLOR sColor) : __Vector3(p), n(sn), color(sColor)
{
}

__VertexXyzNormalColor::__VertexXyzNormalColor(float sx, float sy, float sz, float snx, float sny,
	float snz, D3DCOLOR sColor) : __Vector3(sx, sy, sz), n(snx, sny, snz), color(sColor)
{
}

void __VertexXyzNormalColor::Set(const __Vector3& p, const __Vector3& sn, D3DCOLOR sColor)
{
	x     = p.x;
	y     = p.y;
	z     = p.z;
	n     = sn;
	color = sColor;
}

void __VertexXyzNormalColor::Set(
	float sx, float sy, float sz, float snx, float sny, float snz, D3DCOLOR sColor)
{
	x     = sx;
	y     = sy;
	z     = sz;
	n.x   = snx;
	n.y   = sny;
	n.z   = snz;
	color = sColor;
}

__VertexXyzNormalColor& __VertexXyzNormalColor::operator=(const __Vector3& vec)
{
	x = vec.x;
	y = vec.y;
	z = vec.z;
	return *this;
}

#endif // N3BASE_VERTEXTYPES_INL

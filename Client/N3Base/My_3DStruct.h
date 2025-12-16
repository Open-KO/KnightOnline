#ifndef __MY_3DSTRUCT_H_
#define __MY_3DSTRUCT_H_

#pragma once

#include <d3dx9.h>
#include <d3dx9math.h>
#include <string>
#include <stdint.h>
#include <inttypes.h>

#if defined(_N3TOOL)
#include <afx.h>
#else
#include "DebugUtils.h"
#endif

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

const float FRAME_SELFPLAY = FLT_MIN;

struct __Matrix44;
struct __Quaternion;
struct __Vector2;
struct __Vector3;

// 2D vertex
struct __Vector2
{
public:
	__Vector2() = default;
	__Vector2(float fx, float fy);
	void Zero();
	void Set(float fx, float fy);

	__Vector2& operator += (const __Vector2&);
	__Vector2& operator -= (const __Vector2&);
	__Vector2& operator *= (float);
	__Vector2& operator /= (float);

	__Vector2 operator + (const __Vector2&) const;
	__Vector2 operator - (const __Vector2&) const;
	__Vector2 operator * (float) const;
	__Vector2 operator / (float) const;

public:
	float x, y;
};

// 3D vertex
struct __Vector3
{
public:
	__Vector3() = default;
	__Vector3(float fx, float fy, float fz);
	__Vector3(const __Vector3& vec);

	void	Normalize();
	void	Normalize(const __Vector3& vec);
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

	bool operator==(const __Vector3& rhs) const;
	bool operator!=(const __Vector3& rhs) const;

public:
	float x, y, z;
};

// 4D vertex
struct __Vector4
{
public:
	__Vector4() = default;
	__Vector4(float fx, float fy, float fz, float fw);
	void Zero();
	void Set(float fx, float fy, float fz, float fw);
	void Transform(const __Vector3& v, const __Matrix44& m);

	__Vector4& operator += (const __Vector4&);
	__Vector4& operator -= (const __Vector4&);
	__Vector4& operator *= (float);
	__Vector4& operator /= (float);

	__Vector4 operator + (const __Vector4&) const;
	__Vector4 operator - (const __Vector4&) const;
	__Vector4 operator * (float) const;
	__Vector4 operator / (float) const;

public:
	float x, y, z, w;
};

struct _D3DMATRIX;

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

	__Matrix44 operator * (const __Matrix44& mtx) const;
	void operator *= (const __Matrix44& mtx);
	void operator += (const __Vector3& v);
	void operator -= (const __Vector3& v);

	__Matrix44 operator * (const __Quaternion& qRot) const;
	void operator *= (const __Quaternion& qRot);

	void operator = (const __Quaternion& qt);

public:
	float m[4][4];
};

struct __Quaternion
{
public:
	__Quaternion() = default;
	__Quaternion(const __Matrix44& mtx);
	__Quaternion(const __Quaternion& qt);
	__Quaternion(float fX, float fY, float fZ, float fW);

	void Identity();
	void Set(float fX, float fY, float fZ, float fW);

	void RotationAxis(const __Vector3& v, float fRadian);
	void RotationAxis(float fX, float fY, float fZ, float fRadian);
	void operator = (const __Matrix44& mtx);

	void AxisAngle(__Vector3& vAxisResult, float& fRadianResult) const;
	void Slerp(const __Quaternion& qt1, const __Quaternion& qt2, float fDelta);
	void RotationYawPitchRoll(float Yaw, float Pitch, float Roll);

	__Quaternion operator * (const __Quaternion& q) const;
	void operator *= (const __Quaternion& q);

public:
	float x, y, z, w;
};

struct __ColorValue : public _D3DCOLORVALUE
{
public:
	__ColorValue() = default;
	__ColorValue(D3DCOLOR cr);
	__ColorValue(float r2, float g2, float b2, float a2);

	void operator = (const D3DCOLORVALUE& cv);
	void operator = (D3DCOLOR cr);
	void Set(float r2, float g2, float b2, float a2);

	D3DCOLOR ToD3DCOLOR() const;

	void operator += (float fDelta);
	void operator -= (float fDelta);
	void operator *= (float fDelta);
	void operator /= (float fDelta);
	
	D3DCOLORVALUE operator + (float fDelta) const;
	D3DCOLORVALUE operator - (float fDelta) const;
	D3DCOLORVALUE operator * (float fDelta) const;
	D3DCOLORVALUE operator / (float fDelta) const;

	void operator += (const D3DCOLORVALUE& cv);
	void operator -= (const D3DCOLORVALUE& cv);
	void operator *= (const D3DCOLORVALUE& cv);
	void operator /= (const D3DCOLORVALUE& cv);
	
	D3DCOLORVALUE operator + (const D3DCOLORVALUE& cv) const;
	D3DCOLORVALUE operator - (const D3DCOLORVALUE& cv) const;
	D3DCOLORVALUE operator * (const D3DCOLORVALUE& cv) const;
	D3DCOLORVALUE operator / (const D3DCOLORVALUE& cv) const;
};

constexpr uint32_t FVF_VNT1 = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
constexpr uint32_t FVF_VNT2 = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2;
constexpr uint32_t FVF_CV = D3DFVF_XYZ | D3DFVF_DIFFUSE;
constexpr uint32_t FVF_CSV = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR;
constexpr uint32_t FVF_TRANSFORMED = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;
constexpr uint32_t FVF_TRANSFORMEDT2 = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX2;
constexpr uint32_t FVF_TRANSFORMEDCOLOR = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;
constexpr uint32_t FVF_PARTICLE = D3DFVF_XYZ | D3DFVF_PSIZE | D3DFVF_DIFFUSE;

//..
constexpr uint32_t FVF_XYZT1				= D3DFVF_XYZ | D3DFVF_TEX1;
constexpr uint32_t FVF_XYZT2				= D3DFVF_XYZ | D3DFVF_TEX2;
constexpr uint32_t FVF_XYZNORMAL			= D3DFVF_XYZ | D3DFVF_NORMAL;
constexpr uint32_t FVF_XYZCOLORT1			= D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
constexpr uint32_t FVF_XYZCOLORT2			= D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX2;
constexpr uint32_t FVF_XYZCOLORSPECULART1	= D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1;
constexpr uint32_t FVF_XYZCOLOR				= D3DFVF_XYZ | D3DFVF_DIFFUSE;
constexpr uint32_t FVF_XYZNORMALCOLOR		= D3DFVF_XYZ | D3DFVF_NORMAL  | D3DFVF_DIFFUSE;
constexpr uint32_t FVF_XYZNORMALCOLORT1		= D3DFVF_XYZ | D3DFVF_NORMAL  | D3DFVF_DIFFUSE | D3DFVF_TEX1;

constexpr uint32_t RF_NOTHING			= 0x0;
constexpr uint32_t RF_ALPHABLENDING		= 0x1;		// Alpha blending
constexpr uint32_t RF_NOTUSEFOG			= 0x2;		// 안개 무시
constexpr uint32_t RF_DOUBLESIDED		= 0x4;		// 양면 - D3DCULL_NONE
constexpr uint32_t RF_BOARD_Y			= 0x8;		// Y 축으로 해서.. 카메라를 본다.
constexpr uint32_t RF_POINTSAMPLING		= 0x10;		// MipMap 에서.. PointSampling 으로 한다..
constexpr uint32_t RF_WINDY				= 0x20;		// 바람에 날린다.. 바람의 값은 CN3Base::s_vWindFactor 를 참조 한다..
constexpr uint32_t RF_NOTUSELIGHT		= 0x40;		// Light Off
constexpr uint32_t RF_DIFFUSEALPHA		= 0x80;		// Diffuse 값을 갖고 투명하게 Alpha blending
constexpr uint32_t RF_NOTZWRITE			= 0x100;	// ZBuffer 에 안쓴다.
constexpr uint32_t RF_UV_CLAMP			= 0x200;	// texture UV적용을 Clamp로 한다..default는 wrap이다..
constexpr uint32_t RF_NOTZBUFFER		= 0x400;	// ZBuffer 무시.

struct __Material : public _D3DMATERIAL9
{
public:
	uint32_t	dwColorOp, dwColorArg1, dwColorArg2;
	uint32_t	nRenderFlags; // 1-AlphaBlending | 2-안개랑 관계없음 | 4-Double Side | 8- ??
	uint32_t	dwSrcBlend; // 소스 블렌딩 방법
	uint32_t	dwDestBlend; // 데스트 블렌딩 방법

public:
	void Init(const _D3DCOLORVALUE& diffuseColor);
	void Init(); // 기본 흰색으로 만든다..
	void ColorSet(const _D3DCOLORVALUE& crDiffuse);
};

// This must match the layout of _D3DLIGHT9.
struct __D3DLight9
{
	_D3DLIGHT9* toD3D()
	{
		return reinterpret_cast<_D3DLIGHT9*>(this);
	}

	const _D3DLIGHT9* toD3D() const
	{
		return reinterpret_cast<const _D3DLIGHT9*>(this);
	}

	D3DLIGHTTYPE	Type;            /* Type of light source */
	__ColorValue	Diffuse;         /* Diffuse color of light */
	__ColorValue	Specular;        /* Specular color of light */
	__ColorValue	Ambient;         /* Ambient color of light */
	__Vector3		Position;         /* Position in world space */
	__Vector3		Direction;        /* Direction in world space */
	float			Range;            /* Cutoff range */
	float			Falloff;          /* Falloff */
	float			Attenuation0;     /* Constant attenuation */
	float			Attenuation1;     /* Linear attenuation */
	float			Attenuation2;     /* Quadratic attenuation */
	float			Theta;            /* Inner angle of spotlight cone */
	float			Phi;              /* Outer angle of spotlight cone */
};

struct __VertexColor : public __Vector3
{
public:
	D3DCOLOR color;

public:
	void Set(const __Vector3& p, D3DCOLOR sColor)
	{
		x = p.x;
		y = p.y;
		z = p.z;
		color = sColor;
	}

	void Set(float sx, float sy, float sz, D3DCOLOR sColor)
	{
		x = sx;
		y = sy;
		z = sz;
		color = sColor;
	}

	const __VertexColor& operator = (const __Vector3& vec)
	{
		x = vec.x;
		y = vec.y;
		z = vec.z;
		return *this;
	}

	__VertexColor() = default;

	__VertexColor(const __Vector3& p, D3DCOLOR sColor)
	{
		Set(p, sColor);
	}

	__VertexColor(float sx, float sy, float sz, D3DCOLOR sColor)
	{
		Set(sx, sy, sz, sColor);
	}
};

struct __VertexParticle : public __Vector3
{
public:
	float PointSize;
	D3DCOLOR color;

public:
	void Set(const __Vector3& p, float fPointSize, D3DCOLOR sColor)
	{
		x = p.x;
		y = p.y;
		z = p.z;
		color = sColor;
		PointSize = fPointSize;
	}

	void Set(float sx, float sy, float sz, float fPointSize, D3DCOLOR sColor)
	{
		x = sx;
		y = sy;
		z = sz;
		color = sColor;
		PointSize = fPointSize;
	}

	__VertexParticle()
	{
		PointSize = 1.0f;
		color = 0xffffffff;
	}

	__VertexParticle(const __Vector3& p, float fPointSize, D3DCOLOR sColor)
	{
		Set(p, fPointSize, sColor);
	}

	__VertexParticle(float sx, float sy, float sz, float fPointSize, D3DCOLOR sColor)
	{
		Set(sx, sy, sz, fPointSize, sColor);
	}
};

struct __VertexTransformedColor : public __Vector3
{
public:
	float rhw;
	D3DCOLOR color;

public:
	void Set(float sx, float sy, float sz, float srhw, D3DCOLOR sColor)
	{
		x = sx;
		y = sy;
		z = sz;
		rhw = srhw;
		color = sColor;
	}

	__VertexTransformedColor() = default;

	__VertexTransformedColor(float sx, float sy, float sz, float srhw, D3DCOLOR sColor)
	{
		Set(sx, sy, sz, srhw, sColor);
	}
};

struct __VertexT1 : public __Vector3
{
public:
	__Vector3 n;
	float tu, tv;

public:
	void Set(const __Vector3& p, const __Vector3& sn, float u, float v)
	{
		x = p.x;
		y = p.y;
		z = p.z;
		n = sn;
		tu = u;
		tv = v;
	}

	void Set(float sx, float sy, float sz, float snx, float sny, float snz, float stu, float stv)
	{
		x = sx;
		y = sy;
		z = sz;
		n.x = snx;
		n.y = sny;
		n.z = snz;
		tu = stu;
		tv = stv;
	}

	__VertexT1() = default;

	__VertexT1(const __Vector3& p, const __Vector3& n, float u, float v)
	{
		Set(p, n, u, v);
	}

	__VertexT1(float sx, float sy, float sz, float snx, float sny, float snz, float stu, float stv)
	{
		Set(sx, sy, sz, snx, sny, snz, stu, stv);
	}
};

struct __VertexT2 : public __VertexT1
{
public:
	float tu2, tv2;

public:
	void Set(const __Vector3& p, const __Vector3& sn, float u, float v, float u2, float v2)
	{
		x = p.x;
		y = p.y;
		z = p.z;
		n = sn;
		tu = u;
		tv = v;
		tu2 = u2;
		tv2 = v2;
	}

	void Set(float sx, float sy, float sz, float snx, float sny, float snz, float stu, float stv, float stu2, float stv2)
	{
		x = sx;
		y = sy;
		z = sz;
		n.x = snx;
		n.y = sny;
		n.z = snz;
		tu = stu;
		tv = stv;
		tu2 = stu2;
		tv2 = stv2;
	}

	__VertexT2() = default;

	__VertexT2(const __Vector3& p, const __Vector3& n, float u, float v, float u2, float v2)
	{
		Set(p, n, u, v, u2, v2);
	}

	__VertexT2(float sx, float sy, float sz, float snx, float sny, float snz, float stu, float stv, float stu2, float stv2)
	{
		Set(sx, sy, sz, snx, sny, snz, stu, stv, stu2, stv2);
	}
};

struct __VertexTransformed : public __Vector3
{
public:
	float rhw;
	D3DCOLOR color; // 필요 없다..
	float tu, tv;

public:
	void Set(float sx, float sy, float sz, float srhw, D3DCOLOR sColor, float stu, float stv)
	{
		x = sx;
		y = sy;
		z = sz;
		rhw = srhw;
		color = sColor;
		tu = stu;
		tv = stv;
	}

	__VertexTransformed() = default;

	__VertexTransformed(float sx, float sy, float sz, float srhw, D3DCOLOR sColor, float stu, float stv)
	{
		Set(sx, sy, sz, srhw, sColor, stu, stv);
	}
};

struct __VertexTransformedT2 : public __VertexTransformed
{
public:
	float tu2, tv2;

public:
	void Set(float sx, float sy, float sz, float srhw, D3DCOLOR sColor, float stu, float stv, float stu2, float stv2)
	{
		x = sx;
		y = sy;
		z = sz;
		rhw = srhw;
		color = sColor;
		tu = stu;
		tv = stv;
		tu2 = stu2;
		tv2 = stv2;
	}

	__VertexTransformedT2() = default;

	__VertexTransformedT2(float sx, float sy, float sz, float srhw, D3DCOLOR sColor, float stu, float stv, float stu2, float stv2)
	{
		Set(sx, sy, sz, srhw, sColor, stu, stv, stu2, stv2);
	}
};

struct __VertexXyzT1 : public __Vector3
{
public:
	float tu, tv;	

public:
	void Set(const __Vector3& p, float u, float v)
	{
		x = p.x;
		y = p.y;
		z = p.z;
		tu = u; tv = v;
	}

	void Set(float sx, float sy, float sz, float u, float v)
	{
		x = sx;
		y = sy;
		z = sz;
		tu = u;
		tv = v;
	}

	const __VertexXyzT1& operator = (const __Vector3& vec)
	{
		x = vec.x;
		y = vec.y;
		z = vec.z;
		return *this;
	}

	__VertexXyzT1() = default;

	__VertexXyzT1(const __Vector3& p, float u, float v)
	{
		Set(p, u, v);
	}

	__VertexXyzT1(float sx, float sy, float sz, float u, float v)
	{
		Set(sx, sy, sz, u, v);
	}
};

struct __VertexXyzT2 : public __VertexXyzT1
{
public:
	float tu2, tv2;

public:
	void Set(const __Vector3& p, float u, float v, float u2, float v2)
	{
		x = p.x;
		y = p.y;
		z = p.z;
		tu = u;
		tv = v;
		tu2 = u2;
		tv2 = v2;
	}

	void Set(float sx, float sy, float sz, float u, float v, float u2, float v2)
	{
		x = sx;
		y = sy;
		z = sz;
		tu = u;
		tv = v;
		tu2 = u2;
		tv2 = v2;
	}

	const __VertexXyzT2& operator = (const __Vector3& vec)
	{
		x = vec.x;
		y = vec.y;
		z = vec.z;
		return *this;
	}

	__VertexXyzT2() = default;

	__VertexXyzT2(const __Vector3& p, float u, float v, float u2, float v2)
	{
		Set(p, u, v, u2, v2);
	}

	__VertexXyzT2(float sx, float sy, float sz, float u, float v, float u2, float v2)
	{
		Set(sx, sy, sz, u, v, u2, v2);
	}
};

struct __VertexXyzNormal : public __Vector3
{
public:
	__Vector3 n;

public:
	void Set(const __Vector3& p, const __Vector3& sn)
	{
		x = p.x;
		y = p.y;
		z = p.z;
		n = sn;
	}

	void Set(float xx, float yy, float zz, float nxx, float nyy, float nzz)
	{
		x = xx;
		y = yy;
		z = zz;
		n.x = nxx;
		n.y = nyy;
		n.z = nzz;
	}

	const __VertexXyzNormal& operator = (const __Vector3& vec)
	{
		x = vec.x;
		y = vec.y;
		z = vec.z;
		return *this;
	}

	__VertexXyzNormal() = default;

	__VertexXyzNormal(const __Vector3& p, const __Vector3& n)
	{
		Set(p, n);
	}

	__VertexXyzNormal(float sx, float sy, float sz, float xx, float yy, float zz)
	{
		Set(sx, sy, sz, xx, yy, zz);
	}
};

struct __VertexXyzColorSpecularT1 : public __Vector3
{
public:
	D3DCOLOR color;
	D3DCOLOR specular;
	float tu, tv;

public:
	void Set(const __Vector3& p, D3DCOLOR sColor, D3DCOLOR sSpecular, float u, float v)
	{
		x = p.x;
		y = p.y;
		z = p.z;
		color = sColor;
		specular = sSpecular;
		tu = u;
		tv = v;
	}

	void Set(float sx, float sy, float sz, D3DCOLOR sColor, D3DCOLOR sSpecular, float u, float v)
	{
		x = sx;
		y = sy;
		z = sz;
		color = sColor;
		specular = sSpecular;
		tu = u;
		tv = v;
	}

	__VertexXyzColorSpecularT1() = default;

	__VertexXyzColorSpecularT1(const __Vector3& p, D3DCOLOR sColor, D3DCOLOR sSpecular, float u, float v)
	{
		Set(p, sColor, sSpecular, u, v);
	}

	__VertexXyzColorSpecularT1(float sx, float sy, float sz, D3DCOLOR sColor, D3DCOLOR sSpecular, float u, float v)
	{
		Set(sx, sy, sz, sColor, sSpecular, u, v);
	}
};

struct __VertexXyzColorT1 : public __Vector3
{
public:
	D3DCOLOR color;
	float tu, tv;

public:
	void Set(const __Vector3& p, D3DCOLOR sColor, float u, float v)
	{
		x = p.x;
		y = p.y;
		z = p.z;
		color = sColor;
		tu = u;
		tv = v;
	}

	void Set(float sx, float sy, float sz, D3DCOLOR sColor, float u, float v)
	{
		x = sx;
		y = sy;
		z = sz;
		color = sColor;
		tu = u;
		tv = v;
	}

	const __VertexXyzColorT1& operator = (const __Vector3& vec)
	{
		x = vec.x;
		y = vec.y;
		z = vec.z;
		return *this;
	}

	__VertexXyzColorT1() = default;

	__VertexXyzColorT1(const __Vector3& p, D3DCOLOR sColor, float u, float v)
	{
		Set(p, sColor, u, v);
	}

	__VertexXyzColorT1(float sx, float sy, float sz, D3DCOLOR sColor, float u, float v)
	{
		Set(sx, sy, sz, sColor, u, v);
	}
};

struct __VertexXyzColorT2 : public __VertexXyzColorT1
{
public:
	float tu2, tv2;
public:
	void Set(const __Vector3& p, D3DCOLOR sColor, float u, float v, float u2, float v2)
	{
		x = p.x;
		y = p.y;
		z = p.z;
		color = sColor;
		tu = u;
		tv = v;
		tu2 = u2;
		tv2 = v2;
	}

	void Set(float sx, float sy, float sz, D3DCOLOR sColor, float u, float v, float u2, float v2)
	{
		x = sx;
		y = sy;
		z = sz;
		color = sColor;
		tu = u;
		tv = v;
		tu2 = u2;
		tv2 = v2;
	}

	const __VertexXyzColorT2& operator = (const __Vector3& vec)
	{
		x = vec.x;
		y = vec.y;
		z = vec.z;
		return *this;
	}

	__VertexXyzColorT2() = default;

	__VertexXyzColorT2(const __Vector3& p, D3DCOLOR sColor, float u, float v, float u2, float v2)
	{
		Set(p, sColor, u, v, u2, v2);
	}

	__VertexXyzColorT2(float sx, float sy, float sz, D3DCOLOR sColor, float u, float v, float u2, float v2)
	{
		Set(sx, sy, sz, sColor, u, v, u2, v2);
	}
};

struct __VertexXyzColor : public __Vector3
{
public:
	D3DCOLOR color;

public:
	void Set(const __Vector3& p, D3DCOLOR sColor)
	{
		x = p.x;
		y = p.y;
		z = p.z;
		color = sColor;
	}

	void Set(float sx, float sy, float sz, D3DCOLOR sColor)
	{
		x = sx;
		y = sy;
		z = sz;
		color = sColor;
	}

	const __VertexXyzColor& operator = (const __Vector3& vec)
	{
		x = vec.x;
		y = vec.y;
		z = vec.z;
		return *this;
	}

	__VertexXyzColor() = default;

	__VertexXyzColor(const __Vector3& p, D3DCOLOR sColor)
	{
		Set(p, sColor);
	}
	__VertexXyzColor(float sx, float sy, float sz, D3DCOLOR sColor)
	{
		Set(sx, sy, sz, sColor);
	}
};

struct __VertexXyzNormalColor : public __Vector3
{
public:
	__Vector3 n;
	D3DCOLOR color;

public:
	void Set(const __Vector3& p, const __Vector3& sn, D3DCOLOR sColor)
	{
		x = p.x;
		y = p.y;
		z = p.z;
		n = sn;
		color = sColor;
	}

	void Set(float sx, float sy, float sz, float nxx, float nyy, float nzz, D3DCOLOR sColor)
	{
		x = sx;
		y = sy;
		z = sz;
		n.x = nxx;
		n.y = nyy;
		n.z = nzz;
		color = sColor;
	}

	const __VertexXyzNormalColor& operator = (const __Vector3& vec)
	{
		x = vec.x;
		y = vec.y;
		z = vec.z;
		return *this;
	}

	__VertexXyzNormalColor() = default;

	__VertexXyzNormalColor(const __Vector3& p, const __Vector3& n, D3DCOLOR sColor)
	{
		Set(p, n, sColor);
	}

	__VertexXyzNormalColor(float sx, float sy, float sz, float xx, float yy, float zz, D3DCOLOR sColor)
	{
		Set(sx, sy, sz, xx, yy, zz, sColor);
	}
};

constexpr int MAX_MIPMAP_COUNT = 10; // 1024 * 1024 단계까지 생성

constexpr uint32_t OBJ_UNKNOWN					= 0;
constexpr uint32_t OBJ_BASE						= 0x1;
constexpr uint32_t OBJ_BASE_FILEACCESS			= 0x2;
constexpr uint32_t OBJ_TEXTURE					= 0x4;
constexpr uint32_t OBJ_TRANSFORM 				= 0x8;
constexpr uint32_t OBJ_TRANSFORM_COLLISION		= 0x10;
constexpr uint32_t OBJ_SCENE					= 0x20;

constexpr uint32_t OBJ_CAMERA					= 0x100;
constexpr uint32_t OBJ_LIGHT					= 0x200;
constexpr uint32_t OBJ_SHAPE					= 0x400;
constexpr uint32_t OBJ_SHAPE_PART				= 0x800;
constexpr uint32_t OBJ_SHAPE_EXTRA				= 0x1000;
constexpr uint32_t OBJ_CHARACTER				= 0x2000;
constexpr uint32_t OBJ_CHARACTER_PART			= 0x4000;
constexpr uint32_t OBJ_CHARACTER_PLUG			= 0x8000;
constexpr uint32_t OBJ_BOARD					= 0x1000;
constexpr uint32_t OBJ_FX_PLUG					= 0x20000;
constexpr uint32_t OBJ_FX_PLUG_PART				= 0x40000;

constexpr uint32_t OBJ_MESH						= 0x100000;
constexpr uint32_t OBJ_MESH_PROGRESSIVE			= 0x200000;
constexpr uint32_t OBJ_MESH_INDEXED				= 0x400000;
constexpr uint32_t OBJ_MESH_VECTOR3				= 0x800000;
constexpr uint32_t OBJ_JOINT					= 0x1000000;
constexpr uint32_t OBJ_SKIN						= 0x2000000;
constexpr uint32_t OBJ_CHARACTER_PART_SKINS		= 0x4000000;

constexpr uint32_t OBJ_DUMMY					= 0x10000000;
constexpr uint32_t OBJ_EFFECT					= 0x20000000;
constexpr uint32_t OBJ_ANIM_CONTROL				= 0x40000000;

#include "CrtDbg.h"

#ifndef _DEBUG
#define __ASSERT(expr, expMessage)
#else
#define __ASSERT(expr, expMessage) \
if (!(expr)) \
{ \
	_CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, "N3 Custom Assert Function", expMessage); \
	char __szErr[512] = {}; \
	snprintf(__szErr, sizeof(__szErr), "%s(%d): %s\n", __FILE__, __LINE__, expMessage); \
	OutputDebugStringA(__szErr); \
	_CrtDbgBreak(); \
}
#endif

D3DCOLOR		_RGB_To_D3DCOLOR(COLORREF cr, uint32_t dwAlpha);
COLORREF		_D3DCOLOR_To_RGB(D3DCOLOR cr);
COLORREF		_D3DCOLORVALUE_To_RGB(const D3DCOLORVALUE& cr);
D3DCOLOR		_D3DCOLORVALUE_To_D3DCOLOR(const D3DCOLORVALUE& cr);
D3DCOLORVALUE	_RGB_To_D3DCOLORVALUE(COLORREF cr, float fAlpha);
D3DCOLORVALUE	_D3DCOLOR_To_D3DCOLORVALUE(D3DCOLOR cr);
bool			_IntersectTriangle(const __Vector3& vOrig, const __Vector3& vDir , const __Vector3& v0, const __Vector3& v1, const __Vector3& v2, float& fT, float& fU, float& fV, __Vector3* pVCol = nullptr);
bool			_IntersectTriangle(const __Vector3& vOrig, const __Vector3& vDir, const __Vector3& v0, const __Vector3& v1, const __Vector3& v2);
bool			_CheckCollisionByBox(const __Vector3& vOrig, const __Vector3& vDir, const __Vector3& vMin, const __Vector3& vMax);
POINT			_Convert3D_To_2DCoordinate(const __Vector3 &vPos, const __Matrix44& mtxView, const __Matrix44& mtxProjection, uint32_t nViewportWidth, uint32_t nViewportHeight);
void			_Convert2D_To_3DCoordinate(int ixScreen, int iyScreen, const __Matrix44& mtxView, const __Matrix44& mtxPrj, uint32_t nViewportWidth, uint32_t nViewportHeight, __Vector3& vPosResult, __Vector3& vDirResult);
float			_Yaw2D(float fDirX, float fDirZ);
int16_t			_IsKeyDown(int iVirtualKey);
int16_t			_IsKeyDowned(int iVirtualKey);

#endif // __MY_3DSTRUCT_H_

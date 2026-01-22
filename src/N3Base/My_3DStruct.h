#ifndef __MY_3DSTRUCT_H_
#define __MY_3DSTRUCT_H_

#pragma once

#include <d3dx9.h>

#include <cstdint>
#include <cmath>
#include <filesystem>
#include <memory>
#include <string>

#include <spdlog/fmt/bundled/format.h>

#if defined(_N3TOOL)
#include <afx.h>
#endif

#include "DebugUtils.h"

#include <MathUtils/MathUtils.h>

const float FRAME_SELFPLAY = FLT_MIN;

struct __ColorValue : public _D3DCOLORVALUE
{
public:
	__ColorValue() = default;
	__ColorValue(D3DCOLOR cr);
	__ColorValue(float r2, float g2, float b2, float a2);

	__ColorValue& operator=(const D3DCOLORVALUE& cv);
	__ColorValue& operator=(D3DCOLOR cr);
	void Set(float r2, float g2, float b2, float a2);

	D3DCOLOR ToD3DCOLOR() const;

	void operator+=(float fDelta);
	void operator-=(float fDelta);
	void operator*=(float fDelta);
	void operator/=(float fDelta);

	D3DCOLORVALUE operator+(float fDelta) const;
	D3DCOLORVALUE operator-(float fDelta) const;
	D3DCOLORVALUE operator*(float fDelta) const;
	D3DCOLORVALUE operator/(float fDelta) const;

	void operator+=(const D3DCOLORVALUE& cv);
	void operator-=(const D3DCOLORVALUE& cv);
	void operator*=(const D3DCOLORVALUE& cv);
	void operator/=(const D3DCOLORVALUE& cv);

	D3DCOLORVALUE operator+(const D3DCOLORVALUE& cv) const;
	D3DCOLORVALUE operator-(const D3DCOLORVALUE& cv) const;
	D3DCOLORVALUE operator*(const D3DCOLORVALUE& cv) const;
	D3DCOLORVALUE operator/(const D3DCOLORVALUE& cv) const;
};

inline constexpr uint32_t FVF_VNT1             = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
inline constexpr uint32_t FVF_VNT2             = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2;
inline constexpr uint32_t FVF_CV               = D3DFVF_XYZ | D3DFVF_DIFFUSE;
inline constexpr uint32_t FVF_CSV              = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR;
inline constexpr uint32_t FVF_TRANSFORMED      = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;
inline constexpr uint32_t FVF_TRANSFORMEDT2    = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX2;
inline constexpr uint32_t FVF_TRANSFORMEDCOLOR = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;
inline constexpr uint32_t FVF_PARTICLE         = D3DFVF_XYZ | D3DFVF_PSIZE | D3DFVF_DIFFUSE;

//..
inline constexpr uint32_t FVF_XYZT1            = D3DFVF_XYZ | D3DFVF_TEX1;
inline constexpr uint32_t FVF_XYZT2            = D3DFVF_XYZ | D3DFVF_TEX2;
inline constexpr uint32_t FVF_XYZNORMAL        = D3DFVF_XYZ | D3DFVF_NORMAL;
inline constexpr uint32_t FVF_XYZCOLORT1       = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
inline constexpr uint32_t FVF_XYZCOLORT2       = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX2;
inline constexpr uint32_t FVF_XYZCOLOR         = D3DFVF_XYZ | D3DFVF_DIFFUSE;
inline constexpr uint32_t FVF_XYZNORMALCOLOR   = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE;

inline constexpr uint32_t RF_NOTHING           = 0x0;
inline constexpr uint32_t RF_ALPHABLENDING     = 0x1;  // Alpha blending
inline constexpr uint32_t RF_NOTUSEFOG         = 0x2;  // 안개 무시
inline constexpr uint32_t RF_DOUBLESIDED       = 0x4;  // 양면 - D3DCULL_NONE
inline constexpr uint32_t RF_BOARD_Y           = 0x8;  // Y 축으로 해서.. 카메라를 본다.
inline constexpr uint32_t RF_POINTSAMPLING     = 0x10; // MipMap 에서.. PointSampling 으로 한다..

// 바람에 날린다.. 바람의 값은 CN3Base::s_vWindFactor 를 참조 한다..
inline constexpr uint32_t RF_WINDY             = 0x20;
inline constexpr uint32_t RF_NOTUSELIGHT       = 0x40;  // Light Off
inline constexpr uint32_t RF_DIFFUSEALPHA      = 0x80;  // Diffuse 값을 갖고 투명하게 Alpha blending
inline constexpr uint32_t RF_NOTZWRITE         = 0x100; // ZBuffer 에 안쓴다.

// texture UV적용을 Clamp로 한다..default는 wrap이다..
inline constexpr uint32_t RF_UV_CLAMP          = 0x200;
inline constexpr uint32_t RF_NOTZBUFFER        = 0x400; // ZBuffer 무시.

struct __Material : public _D3DMATERIAL9
{
public:
	uint32_t dwColorOp, dwColorArg1, dwColorArg2;
	uint32_t nRenderFlags; // 1-AlphaBlending | 2-안개랑 관계없음 | 4-Double Side | 8- ??
	uint32_t dwSrcBlend;   // 소스 블렌딩 방법
	uint32_t dwDestBlend;  // 데스트 블렌딩 방법

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

	D3DLIGHTTYPE Type     = D3DLIGHT_POINT; /* Type of light source */
	__ColorValue Diffuse  = {};             /* Diffuse color of light */
	__ColorValue Specular = {};             /* Specular color of light */
	__ColorValue Ambient  = {};             /* Ambient color of light */
	__Vector3 Position    = {};             /* Position in world space */
	__Vector3 Direction   = {};             /* Direction in world space */
	float Range           = 0.0f;           /* Cutoff range */
	float Falloff         = 0.0f;           /* Falloff */
	float Attenuation0    = 0.0f;           /* Constant attenuation */
	float Attenuation1    = 0.0f;           /* Linear attenuation */
	float Attenuation2    = 0.0f;           /* Quadratic attenuation */
	float Theta           = 0.0f;           /* Inner angle of spotlight cone */
	float Phi             = 0.0f;           /* Outer angle of spotlight cone */
};

struct __VertexColor : public __Vector3
{
public:
	D3DCOLOR color;

public:
	__VertexColor() = default;
	__VertexColor(const __Vector3& p, D3DCOLOR sColor);
	__VertexColor(float sx, float sy, float sz, D3DCOLOR sColor);

	void Set(const __Vector3& p, D3DCOLOR sColor);
	void Set(float sx, float sy, float sz, D3DCOLOR sColor);
	__VertexColor& operator=(const __Vector3& vec);
};

struct __VertexTransformedColor : public __Vector3
{
public:
	float rhw;
	D3DCOLOR color;

public:
	__VertexTransformedColor() = default;
	__VertexTransformedColor(float sx, float sy, float sz, float srhw, D3DCOLOR sColor);
	void Set(float sx, float sy, float sz, float srhw, D3DCOLOR sColor);
};

struct __VertexT1 : public __Vector3
{
public:
	__Vector3 n;
	float tu, tv;

public:
	__VertexT1() = default;
	__VertexT1(const __Vector3& p, const __Vector3& sn, float u, float v);
	__VertexT1(float sx, float sy, float sz, float snx, float sny, float snz, float stu, float stv);
	void Set(const __Vector3& p, const __Vector3& sn, float u, float v);
	void Set(float sx, float sy, float sz, float snx, float sny, float snz, float stu, float stv);
};

struct __VertexT2 : public __VertexT1
{
public:
	float tu2, tv2;

public:
	__VertexT2() = default;
	__VertexT2(const __Vector3& p, const __Vector3& sn, float u, float v, float u2, float v2);
	__VertexT2(float sx, float sy, float sz, float snx, float sny, float snz, float stu, float stv,
		float stu2, float stv2);
	void Set(const __Vector3& p, const __Vector3& sn, float u, float v, float u2, float v2);
	void Set(float sx, float sy, float sz, float snx, float sny, float snz, float stu, float stv,
		float stu2, float stv2);
};

struct __VertexTransformed : public __Vector3
{
public:
	float rhw;
	D3DCOLOR color; // 필요 없다..
	float tu, tv;

public:
	__VertexTransformed() = default;
	__VertexTransformed(
		float sx, float sy, float sz, float srhw, D3DCOLOR sColor, float stu, float stv);
	void Set(float sx, float sy, float sz, float srhw, D3DCOLOR sColor, float stu, float stv);
};

struct __VertexTransformedT2 : public __VertexTransformed
{
public:
	float tu2, tv2;

public:
	__VertexTransformedT2() = default;
	__VertexTransformedT2(float sx, float sy, float sz, float srhw, D3DCOLOR sColor, float stu,
		float stv, float stu2, float stv2);
	void Set(float sx, float sy, float sz, float srhw, D3DCOLOR sColor, float stu, float stv,
		float stu2, float stv2);
};

struct __VertexXyzT1 : public __Vector3
{
public:
	float tu, tv;

public:
	__VertexXyzT1() = default;
	__VertexXyzT1(const __Vector3& p, float u, float v);
	__VertexXyzT1(float sx, float sy, float sz, float u, float v);
	void Set(const __Vector3& p, float u, float v);
	void Set(float sx, float sy, float sz, float u, float v);
	__VertexXyzT1& operator=(const __Vector3& vec);
};

struct __VertexXyzT2 : public __VertexXyzT1
{
public:
	float tu2, tv2;

public:
	__VertexXyzT2() = default;
	__VertexXyzT2(const __Vector3& p, float u, float v, float u2, float v2);
	__VertexXyzT2(float sx, float sy, float sz, float u, float v, float u2, float v2);
	void Set(const __Vector3& p, float u, float v, float u2, float v2);
	void Set(float sx, float sy, float sz, float u, float v, float u2, float v2);
	__VertexXyzT2& operator=(const __Vector3& vec);
};

struct __VertexXyzNormal : public __Vector3
{
public:
	__Vector3 n;

public:
	__VertexXyzNormal() = default;
	__VertexXyzNormal(const __Vector3& p, const __Vector3& sn);
	__VertexXyzNormal(float sx, float sy, float sz, float snx, float sny, float snz);
	void Set(const __Vector3& p, const __Vector3& sn);
	void Set(float sx, float sy, float sz, float snx, float sny, float snz);
	__VertexXyzNormal& operator=(const __Vector3& vec);
};

struct __VertexXyzColorT1 : public __Vector3
{
public:
	D3DCOLOR color;
	float tu, tv;

public:
	__VertexXyzColorT1() = default;
	__VertexXyzColorT1(const __Vector3& p, D3DCOLOR sColor, float u, float v);
	__VertexXyzColorT1(float sx, float sy, float sz, D3DCOLOR sColor, float u, float v);
	void Set(const __Vector3& p, D3DCOLOR sColor, float u, float v);
	void Set(float sx, float sy, float sz, D3DCOLOR sColor, float u, float v);
	__VertexXyzColorT1& operator=(const __Vector3& vec);
};

struct __VertexXyzColorT2 : public __VertexXyzColorT1
{
public:
	float tu2, tv2;

public:
	__VertexXyzColorT2() = default;
	__VertexXyzColorT2(const __Vector3& p, D3DCOLOR sColor, float u, float v, float u2, float v2);
	__VertexXyzColorT2(
		float sx, float sy, float sz, D3DCOLOR sColor, float u, float v, float u2, float v2);
	void Set(const __Vector3& p, D3DCOLOR sColor, float u, float v, float u2, float v2);
	void Set(float sx, float sy, float sz, D3DCOLOR sColor, float u, float v, float u2, float v2);
	__VertexXyzColorT2& operator=(const __Vector3& vec);
};

struct __VertexXyzColor : public __Vector3
{
public:
	D3DCOLOR color;

public:
	__VertexXyzColor() = default;
	__VertexXyzColor(const __Vector3& p, D3DCOLOR sColor);
	__VertexXyzColor(float sx, float sy, float sz, D3DCOLOR sColor);
	void Set(const __Vector3& p, D3DCOLOR sColor);
	void Set(float sx, float sy, float sz, D3DCOLOR sColor);
	__VertexXyzColor& operator=(const __Vector3& vec);
};

struct __VertexXyzNormalColor : public __Vector3
{
public:
	__Vector3 n;
	D3DCOLOR color;

public:
	__VertexXyzNormalColor() = default;
	__VertexXyzNormalColor(const __Vector3& p, const __Vector3& sn, D3DCOLOR sColor);
	__VertexXyzNormalColor(
		float sx, float sy, float sz, float snx, float sny, float snz, D3DCOLOR sColor);
	void Set(const __Vector3& p, const __Vector3& sn, D3DCOLOR sColor);
	void Set(float sx, float sy, float sz, float snx, float sny, float snz, D3DCOLOR sColor);
	__VertexXyzNormalColor& operator=(const __Vector3& vec);
};

inline constexpr int MAX_MIPMAP_COUNT              = 10; // 1024 * 1024 단계까지 생성

inline constexpr uint32_t OBJ_UNKNOWN              = 0;
inline constexpr uint32_t OBJ_BASE                 = 0x1;
inline constexpr uint32_t OBJ_BASE_FILEACCESS      = 0x2;
inline constexpr uint32_t OBJ_TEXTURE              = 0x4;
inline constexpr uint32_t OBJ_TRANSFORM            = 0x8;
inline constexpr uint32_t OBJ_TRANSFORM_COLLISION  = 0x10;
inline constexpr uint32_t OBJ_SCENE                = 0x20;

inline constexpr uint32_t OBJ_CAMERA               = 0x100;
inline constexpr uint32_t OBJ_LIGHT                = 0x200;
inline constexpr uint32_t OBJ_SHAPE                = 0x400;
inline constexpr uint32_t OBJ_SHAPE_PART           = 0x800;
inline constexpr uint32_t OBJ_SHAPE_EXTRA          = 0x1000;
inline constexpr uint32_t OBJ_CHARACTER            = 0x2000;
inline constexpr uint32_t OBJ_CHARACTER_PART       = 0x4000;
inline constexpr uint32_t OBJ_CHARACTER_PLUG       = 0x8000;
inline constexpr uint32_t OBJ_BOARD                = 0x1000;
inline constexpr uint32_t OBJ_FX_PLUG              = 0x20000;
inline constexpr uint32_t OBJ_FX_PLUG_PART         = 0x40000;

inline constexpr uint32_t OBJ_MESH                 = 0x100000;
inline constexpr uint32_t OBJ_MESH_PROGRESSIVE     = 0x200000;
inline constexpr uint32_t OBJ_MESH_INDEXED         = 0x400000;
inline constexpr uint32_t OBJ_MESH_VECTOR3         = 0x800000;
inline constexpr uint32_t OBJ_JOINT                = 0x1000000;
inline constexpr uint32_t OBJ_SKIN                 = 0x2000000;
inline constexpr uint32_t OBJ_CHARACTER_PART_SKINS = 0x4000000;

inline constexpr uint32_t OBJ_DUMMY                = 0x10000000;
inline constexpr uint32_t OBJ_EFFECT               = 0x20000000;
inline constexpr uint32_t OBJ_ANIM_CONTROL         = 0x40000000;

#ifndef _DEBUG
#define __ASSERT(expr, expMessage)
#else
#include "CrtDbg.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg,cppcoreguidelines-macro-usage)
#define __ASSERT(expr, expMessage)                                                               \
	if (!(expr))                                                                                 \
	{                                                                                            \
		_CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, "N3 Custom Assert Function", expMessage); \
		FormattedDebugString("{}({}): {}\n", __FILE__, __LINE__, expMessage);                    \
		_CrtDbgBreak();                                                                          \
	}
// NOLINTEND(cppcoreguidelines-pro-type-vararg,cppcoreguidelines-macro-usage)

#endif

D3DCOLOR _RGB_To_D3DCOLOR(COLORREF cr, uint32_t dwAlpha);
COLORREF _D3DCOLOR_To_RGB(D3DCOLOR cr);
COLORREF _D3DCOLORVALUE_To_RGB(const D3DCOLORVALUE& cr);
D3DCOLOR _D3DCOLORVALUE_To_D3DCOLOR(const D3DCOLORVALUE& cr);
D3DCOLORVALUE _RGB_To_D3DCOLORVALUE(COLORREF cr, float fAlpha);
D3DCOLORVALUE _D3DCOLOR_To_D3DCOLORVALUE(D3DCOLOR cr);
int16_t _IsKeyDown(int iVirtualKey);
int16_t _IsKeyDowned(int iVirtualKey);

#endif // __MY_3DSTRUCT_H_

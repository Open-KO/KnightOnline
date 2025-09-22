#ifndef __MY_3DSTRUCT_H_
#define __MY_3DSTRUCT_H_

#include <d3dx9.h>
#include <d3dx9math.h>
#include <string>

#include <shared/types.h>
#if defined(_N3TOOL) || defined(_3DSERVER)
#include <afx.h>
#else
#include <shared/DebugUtils.h>
#endif

const float __PI = 3.141592654f;
const float __PI2 = 6.283185308f;
const float FRAME_SELFPLAY = FLT_MIN;

struct __Vector2 : public D3DXVECTOR2 // 2D Vertex
{
public:
	void Zero();
	void Set(float fx, float fy);
	__Vector2() {};
	__Vector2(float fx, float fy);
};

struct __Matrix44;
struct __Quaternion;

struct __Vector3 : public D3DXVECTOR3 // 3D Vertex
{
public:
	void	Normalize();
	float	Magnitude() const;
	float	Dot(const D3DXVECTOR3& vec) const;
	void	Cross(const D3DXVECTOR3& v1, const D3DXVECTOR3& v2);
	void	Absolute();

	void Zero();
	void Set(float fx, float fy, float fz);

	__Vector3() {};
	__Vector3(float fx, float fy, float fz);
	__Vector3(const _D3DVECTOR& vec);
	__Vector3(const D3DXVECTOR3& vec);

	const __Vector3& operator = (const __Vector3& vec);

	const __Vector3 operator * (const D3DXMATRIX& mtx) const;
	void operator *= (float fDelta);
	void operator *= (const D3DXMATRIX& mtx);
	__Vector3 operator + (const D3DXVECTOR3& vec) const;
	__Vector3 operator - (const D3DXVECTOR3& vec) const;
	__Vector3 operator * (const D3DXVECTOR3& vec) const;
	__Vector3 operator / (const D3DXVECTOR3& vec) const;

	void operator += (const D3DXVECTOR3& vec);
	void operator -= (const D3DXVECTOR3& vec);
	void operator *= (const D3DXVECTOR3& vec);
	void operator /= (const D3DXVECTOR3& vec);

	__Vector3 operator + (float fDelta) const;
	__Vector3 operator - (float fDelta) const;
	__Vector3 operator * (float fDelta) const;
	__Vector3 operator / (float fDelta) const;
};

struct __Matrix44 : public D3DXMATRIX // 4x4 Matrix
{
public:
	void Zero();
	void Identity();
	const __Vector3 Pos() const;
	void PosSet(float x, float y, float z);
	void PosSet(const D3DXVECTOR3& v);
	void RotationX(float fDelta);
	void RotationY(float fDelta);
	void RotationZ(float fDelta);
	void Rotation(float fX, float fY, float fZ);
	void Rotation(const D3DXVECTOR3& v);
	void Scale(float sx, float sy, float sz);
	void Scale(const D3DXVECTOR3& v);

	void Direction(const D3DXVECTOR3& vDir);

	__Matrix44 operator * (const D3DXMATRIX& mtx);
	void operator *= (const D3DXMATRIX& mtx);
	void operator += (const D3DXVECTOR3& v);
	void operator -= (const D3DXVECTOR3& v);

	__Matrix44 operator * (const __Quaternion& qRot);
	void operator *= (const __Quaternion& qRot);

	void operator = (const D3DXQUATERNION& qt);

	__Matrix44();
	__Matrix44(const _D3DMATRIX& mtx);
	__Matrix44(const D3DXMATRIX& mtx);
	__Matrix44(const D3DXQUATERNION& qt);
};

struct __Quaternion : public D3DXQUATERNION
{
public:
	void Identity();
	void Set(float fX, float fY, float fZ, float fW);

	void RotationAxis(const __Vector3& v, float fRadian);
	void RotationAxis(float fX, float fY, float fZ, float fRadian);
	void operator = (const D3DXMATRIX& mtx);

	void AxisAngle(__Vector3& vAxisResult, float& fRadianResult) const;
	void Slerp(const D3DXQUATERNION& qt1, const D3DXQUATERNION& qt2, float fDelta);

	__Quaternion();
	__Quaternion(const D3DXMATRIX& mtx);
	__Quaternion(const D3DXQUATERNION& qt);
};

struct __ColorValue : public _D3DCOLORVALUE
{
public:
	void operator = (const D3DCOLORVALUE& cv) { r = cv.r; g = cv.g; b = cv.b; a = cv.a; }
	void operator = (D3DCOLOR cr) { r = ((cr & 0x00ff0000)>>16)/255.0f; g = ((cr & 0x0000ff00)>>8)/255.0f; b = (cr & 0x000000ff)/255.0f; a = ((cr & 0xff000000) >> 24)/255.0f; }
	void Set(float r2, float g2, float b2, float a2) { r = r2; g = g2; b = b2; a = a2; }

	D3DCOLOR ToD3DCOLOR() { return	(((uint32_t)(a*255.0f))<<24) | (((uint32_t)(r*255.0f))<<16) | (((uint32_t)(g*255.0f))<<8) | (((uint32_t)(b*255.0f))); }

	__ColorValue() {}
	__ColorValue(D3DCOLOR cr) { *this = cr; }
	__ColorValue(float r2, float g2, float b2, float a2) { this->Set(r2, g2, b2, a2); }

	void operator += (float fDelta) { r += fDelta; g += fDelta; b += fDelta; a += fDelta; }
	void operator -= (float fDelta) { r -= fDelta; g -= fDelta; b -= fDelta; a -= fDelta; }
	void operator *= (float fDelta) { r *= fDelta; g *= fDelta; b *= fDelta; a *= fDelta; }
	void operator /= (float fDelta) { if(0 == fDelta) return; r /= fDelta; g /= fDelta; b /= fDelta; a /= fDelta; }
	
	D3DCOLORVALUE operator + (float fDelta) { __ColorValue cv = *this; cv.r += fDelta; cv.g += fDelta; cv.b += fDelta; cv.a += fDelta; return cv; }
	D3DCOLORVALUE operator - (float fDelta) { __ColorValue cv = *this; cv.r -= fDelta; cv.g -= fDelta; cv.b -= fDelta; cv.a -= fDelta; return cv; }
	D3DCOLORVALUE operator * (float fDelta) { __ColorValue cv = *this; cv.r *= fDelta; cv.g *= fDelta; cv.b *= fDelta; cv.a *= fDelta; return cv; }
	D3DCOLORVALUE operator / (float fDelta) { __ColorValue cv = *this; cv.r /= fDelta; cv.g /= fDelta; cv.b /= fDelta; cv.a /= fDelta; return cv; }

	void operator += (const D3DCOLORVALUE& cv) { r += cv.r; g += cv.g; b += cv.b; a += cv.a; }
	void operator -= (const D3DCOLORVALUE& cv) { r -= cv.r; g -= cv.g; b -= cv.b; a -= cv.a; }
	void operator *= (const D3DCOLORVALUE& cv) { r *= cv.r; g *= cv.g; b *= cv.b; a *= cv.a; }
	void operator /= (const D3DCOLORVALUE& cv) { r /= cv.r; g /= cv.g; b /= cv.b; a /= cv.a; }
	
	D3DCOLORVALUE operator + (const D3DCOLORVALUE& cv) { __ColorValue cv2(cv.r, cv.g, cv.b, cv.a); cv2 += cv; return cv2; }
	D3DCOLORVALUE operator - (const D3DCOLORVALUE& cv) { __ColorValue cv2(cv.r, cv.g, cv.b, cv.a); cv2 -= cv; return cv2; }
	D3DCOLORVALUE operator * (const D3DCOLORVALUE& cv) { __ColorValue cv2(cv.r, cv.g, cv.b, cv.a); cv2 *= cv; return cv2; }
	D3DCOLORVALUE operator / (const D3DCOLORVALUE& cv) { __ColorValue cv2(cv.r, cv.g, cv.b, cv.a); cv2 /= cv; return cv2; }
};

inline void __Vector2::Zero()
{
	x = y = 0;
}

inline void __Vector2::Set(float fx, float fy)
{
	x = fx; 
	y = fy;
}

inline __Vector2::__Vector2(float fx, float fy)
{
	x = fx;
	y = fy;
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

inline float __Vector3::Dot(const D3DXVECTOR3& vec) const 
{
	return x*vec.x + y*vec.y + z*vec.z;
}

inline void __Vector3::Cross(const D3DXVECTOR3& v1, const D3DXVECTOR3& v2)
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

//inline __Vector3::__Vector3()
//{
//}

inline __Vector3::__Vector3(float fx, float fy, float fz)
{
	x = fx; y = fy, z = fz;
}

inline __Vector3::__Vector3(const D3DXVECTOR3& vec)
{
	x = vec.x; y = vec.y; z = vec.z;
}

inline __Vector3::__Vector3(const _D3DVECTOR& vec)
{
	x = vec.x; y = vec.y; z = vec.z;
}

inline const __Vector3& __Vector3::operator = (const __Vector3& vec)
{
	x = vec.x; y = vec.y; z = vec.z;
	return *this;
}

inline const __Vector3 __Vector3::operator * (const D3DXMATRIX& mtx) const 
{
	static __Vector3 vTmp;

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

inline void __Vector3::operator *= (const D3DXMATRIX& mtx)
{
	static __Vector3 vTmp;

	vTmp.Set(x,y,z);
	x = vTmp.x*mtx._11 + vTmp.y*mtx._21 + vTmp.z*mtx._31 + mtx._41;
	y = vTmp.x*mtx._12 + vTmp.y*mtx._22 + vTmp.z*mtx._32 + mtx._42;
	z = vTmp.x*mtx._13 + vTmp.y*mtx._23 + vTmp.z*mtx._33 + mtx._43;
}

inline __Vector3 __Vector3::operator + (const D3DXVECTOR3& vec) const
{
	static __Vector3 vTmp;

	vTmp.x = x + vec.x;
	vTmp.y = y + vec.y;
	vTmp.z = z + vec.z;
	return vTmp;
}

inline __Vector3 __Vector3::operator - (const D3DXVECTOR3& vec) const 
{
	static __Vector3 vTmp;

	vTmp.x = x - vec.x;
	vTmp.y = y - vec.y;
	vTmp.z = z - vec.z;
	return vTmp;
}

inline __Vector3 __Vector3::operator * (const D3DXVECTOR3& vec) const 
{
	static __Vector3 vTmp;

	vTmp.x = x * vec.x;
	vTmp.y = y * vec.y;
	vTmp.z = z * vec.z;
	return vTmp;
}

inline __Vector3 __Vector3::operator / (const D3DXVECTOR3& vec) const
{
	static __Vector3 vTmp;

	vTmp.x = x / vec.x;
	vTmp.y = y / vec.y;
	vTmp.z = z / vec.z;
	return vTmp;
}

inline void __Vector3::operator += (const D3DXVECTOR3& vec) 
{
	x += vec.x;
	y += vec.y;
	z += vec.z;
}

inline void __Vector3::operator -= (const D3DXVECTOR3& vec) 
{
	x -= vec.x;
	y -= vec.y;
	z -= vec.z;
}

inline void __Vector3::operator *= (const D3DXVECTOR3& vec) 
{
	x *= vec.x;
	y *= vec.y;
	z *= vec.z;
}

inline void __Vector3::operator /= (const D3DXVECTOR3& vec) 
{
	x /= vec.x;
	y /= vec.y;
	z /= vec.z;
}

inline __Vector3 __Vector3::operator + (float fDelta) const 
{ 
	static __Vector3 vTmp;

	vTmp.x = x + fDelta;
	vTmp.y = y + fDelta;
	vTmp.z = z + fDelta;
	return vTmp;
}

inline __Vector3 __Vector3::operator - (float fDelta) const 
{
	static __Vector3 vTmp;

	vTmp.x = x - fDelta;
	vTmp.y = y - fDelta;
	vTmp.z = z - fDelta;
	return vTmp;
}

inline __Vector3 __Vector3::operator * (float fDelta) const 
{
	static __Vector3 vTmp;

	vTmp.x = x * fDelta;
	vTmp.y = y * fDelta;
	vTmp.z = z * fDelta;
	return vTmp;
}

inline __Vector3 __Vector3::operator / (float fDelta) const 
{
	static __Vector3 vTmp;

	vTmp.x = x / fDelta;
	vTmp.y = y / fDelta;
	vTmp.z = z / fDelta;
	return vTmp;
}


inline void __Matrix44::Zero() 
{
	memset(this, 0, sizeof(_D3DMATRIX)); 
}

inline void __Matrix44::Identity()
{
	_12 = _13 = _14 = _21 = _23 = _24 = _31 = _32 = _34 = _41 = _42 = _43 = 0;
	_11 = _22 = _33 = _44 = 1.0f;
}

inline const __Vector3 __Matrix44::Pos() const 
{
	static __Vector3 vTmp;

	vTmp.Set(_41, _42, _43);
	return vTmp;
}

inline void __Matrix44::PosSet(float x, float y, float z)
{
	_41 = x; _42 = y; _43 = z;
}

inline void __Matrix44::PosSet(const D3DXVECTOR3& v) 
{
	_41 = v.x;
	_42 = v.y;
	_43 = v.z;
}

inline void __Matrix44::RotationX(float fDelta)
{
	this->Identity();
	_22 = cosf(fDelta); _23 = sinf(fDelta); _32 = -_23; _33 = _22;
}

inline void __Matrix44::RotationY(float fDelta)
{
	this->Identity();
	_11 = cosf(fDelta); _13 = -sinf(fDelta); _31 = -_13; _33 = _11;
}

inline void __Matrix44::RotationZ(float fDelta)
{
	this->Identity();
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

inline void __Matrix44::Rotation(const D3DXVECTOR3& v)
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
	this->Identity();
	_11 = sx; _22 = sy; _33 = sz;
}

inline void __Matrix44::Scale(const D3DXVECTOR3& v) 
{
	this->Identity();
	_11 = v.x; _22 = v.y; _33 = v.z;
}

inline __Matrix44::__Matrix44()
{
};

inline __Matrix44::__Matrix44(const _D3DMATRIX& mtx)
{
	memcpy(this, &mtx, sizeof(_D3DMATRIX));
}

inline __Matrix44::__Matrix44(const D3DXMATRIX& mtx)
{
	memcpy(this, &mtx, sizeof(D3DXMATRIX));
}

inline __Matrix44::__Matrix44(const D3DXQUATERNION& qt)
{
	D3DXMatrixRotationQuaternion(this, &qt);
}

inline __Matrix44 __Matrix44::operator * (const D3DXMATRIX& mtx)
{
	static __Matrix44 mtxTmp;

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

inline void __Matrix44::operator *= (const D3DXMATRIX& mtx)
{
	static __Matrix44 mtxTmp;

	memcpy(&mtxTmp, this, sizeof(__Matrix44));

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

inline void __Matrix44::operator += (const D3DXVECTOR3& v)
{
	_41 += v.x;
	_42 += v.y;
	_43 += v.z;
}

inline void __Matrix44::operator -= (const D3DXVECTOR3& v)
{
	_41 -= v.x;
	_42 -= v.y;
	_43 -= v.z;
}

inline __Matrix44 __Matrix44::operator * (const __Quaternion& qRot)
{
	static __Matrix44 mtx;
	mtx.operator = (qRot);

	return this->operator * (mtx);
}

inline void __Matrix44::operator *= (const __Quaternion& qRot)
{
	static __Matrix44 mtx;
	mtx.operator = (qRot);

	this->operator *= (mtx);
}


inline void __Matrix44::operator = (const D3DXQUATERNION& qt)
{
	D3DXMatrixRotationQuaternion(this, &qt);
}

inline void __Matrix44::Direction(const D3DXVECTOR3& vDir)
{
	this->Identity();

	static __Vector3 vDir2, vRight, vUp;
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

	D3DXMatrixInverse(this, nullptr, this);
	
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

inline void __Quaternion::Identity()
{
	x = y = z = 0; w = 1.0f;
}


inline void __Quaternion::Set(float fX, float fY, float fZ, float fW)
{
	x = fX; y = fY; z = fZ; w = fW;
}

inline void __Quaternion::RotationAxis(const __Vector3& v, float fRadian)
{
	D3DXQuaternionRotationAxis(this, &v, fRadian);
}

inline void __Quaternion::RotationAxis(float fX, float fY, float fZ, float fRadian)
{
	__Vector3 v(fX, fY, fZ);
	D3DXQuaternionRotationAxis(this, &v, fRadian);
}

inline void __Quaternion::operator = (const D3DXMATRIX& mtx)
{
	D3DXQuaternionRotationMatrix(this, &mtx);
}

inline void __Quaternion::AxisAngle(__Vector3& vAxisResult, float& fRadianResult) const
{
	D3DXQuaternionToAxisAngle(this, &vAxisResult, &fRadianResult);
}

inline void __Quaternion::Slerp(const D3DXQUATERNION& qt1, const D3DXQUATERNION& qt2, float fDelta)
{
	D3DXQuaternionSlerp(this, &qt1, &qt2, fDelta);
}

inline __Quaternion::__Quaternion()
{
}

inline __Quaternion::__Quaternion(const D3DXMATRIX& mtx)
{
	D3DXQuaternionRotationMatrix(this, &mtx);
}

inline __Quaternion::__Quaternion(const D3DXQUATERNION& qt)
{
	x = qt.x; y = qt.y; z = qt.z; w = qt.w;
}

const uint32_t FVF_VNT1 = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
const uint32_t FVF_VNT2 = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2;
const uint32_t FVF_CV = D3DFVF_XYZ | D3DFVF_DIFFUSE;
const uint32_t FVF_CSV = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR;
const uint32_t FVF_TRANSFORMED = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;
const uint32_t FVF_TRANSFORMEDT2 = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX2;
const uint32_t FVF_TRANSFORMEDCOLOR = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;
const uint32_t FVF_PARTICLE = D3DFVF_XYZ | D3DFVF_PSIZE | D3DFVF_DIFFUSE;

//..
const uint32_t FVF_XYZT1				= D3DFVF_XYZ | D3DFVF_TEX1;
const uint32_t FVF_XYZT2				= D3DFVF_XYZ | D3DFVF_TEX2;
const uint32_t FVF_XYZNORMAL			= D3DFVF_XYZ | D3DFVF_NORMAL;
const uint32_t FVF_XYZCOLORT1			= D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
const uint32_t FVF_XYZCOLORT2			= D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX2;
const uint32_t FVF_XYZCOLORSPECULART1	= D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1;
const uint32_t FVF_XYZCOLOR			= D3DFVF_XYZ | D3DFVF_DIFFUSE;
const uint32_t FVF_XYZNORMALCOLOR		= D3DFVF_XYZ | D3DFVF_NORMAL  | D3DFVF_DIFFUSE;
const uint32_t FVF_XYZNORMALCOLORT1	= D3DFVF_XYZ | D3DFVF_NORMAL  | D3DFVF_DIFFUSE | D3DFVF_TEX1;

const uint32_t RF_NOTHING			= 0x0;
const uint32_t RF_ALPHABLENDING	= 0x1;		// Alpha blending
const uint32_t RF_NOTUSEFOG		= 0x2;		// 안개 무시
const uint32_t RF_DOUBLESIDED		= 0x4;		// 양면 - D3DCULL_NONE
const uint32_t RF_BOARD_Y			= 0x8;		// Y 축으로 해서.. 카메라를 본다.
const uint32_t RF_POINTSAMPLING	= 0x10;		// MipMap 에서.. PointSampling 으로 한다..
const uint32_t RF_WINDY			= 0x20;		// 바람에 날린다.. 바람의 값은 CN3Base::s_vWindFactor 를 참조 한다..
const uint32_t RF_NOTUSELIGHT		= 0x40;		// Light Off
const uint32_t RF_DIFFUSEALPHA		= 0x80;		// Diffuse 값을 갖고 투명하게 Alpha blending
const uint32_t RF_NOTZWRITE		= 0x100;	// ZBuffer 에 안쓴다.
const uint32_t RF_UV_CLAMP			= 0x200;	// texture UV적용을 Clamp로 한다..default는 wrap이다..
const uint32_t RF_NOTZBUFFER		= 0x400;	// ZBuffer 무시.

struct __Material : public _D3DMATERIAL9
{
public:
	uint32_t	dwColorOp, dwColorArg1, dwColorArg2;
	uint32_t	nRenderFlags; // 1-AlphaBlending | 2-안개랑 관계없음 | 4-Double Side | 8- ??
	uint32_t	dwSrcBlend; // 소스 블렌딩 방법
	uint32_t	dwDestBlend; // 데스트 블렌딩 방법

public:
	void Init(const _D3DCOLORVALUE& diffuseColor)
	{
		memset(this, 0, sizeof(__Material));

		Diffuse = diffuseColor;
		Ambient.a = Diffuse.a;
		Ambient.r = Diffuse.r * 0.5f;
		Ambient.g = Diffuse.g * 0.5f;
		Ambient.b = Diffuse.b * 0.5f;

		dwColorOp = D3DTOP_MODULATE;
		dwColorArg1 = D3DTA_DIFFUSE;
		dwColorArg2 = D3DTA_TEXTURE;
		nRenderFlags = RF_NOTHING;
		dwSrcBlend = D3DBLEND_SRCALPHA;
		dwDestBlend = D3DBLEND_INVSRCALPHA;
	}

	void Init() // 기본 흰색으로 만든다..
	{
		D3DCOLORVALUE crDiffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
		this->Init(crDiffuse);
	}

	void ColorSet(const _D3DCOLORVALUE& crDiffuse)
	{
		Diffuse = crDiffuse; 
		Ambient.a = Diffuse.a;
		Ambient.r = Diffuse.r * 0.5f;
		Ambient.g = Diffuse.g * 0.5f;
		Ambient.b = Diffuse.b * 0.5f;
	}
};

struct __VertexColor : public __Vector3
{
public:
	D3DCOLOR color;

public:
	void Set(const _D3DVECTOR& p, D3DCOLOR sColor) { x = p.x; y = p.y; z = p.z; color = sColor; }
	void Set(float sx, float sy, float sz, D3DCOLOR sColor) { x = sx; y = sy; z = sz; color = sColor; }
	const __VertexColor& operator = (const __Vector3& vec) { x = vec.x; y = vec.y; z = vec.z; return *this; }

	__VertexColor() {}
	__VertexColor(const _D3DVECTOR& p, D3DCOLOR sColor) { this->Set(p, sColor); }
	__VertexColor(float sx, float sy, float sz, D3DCOLOR sColor) { this->Set(sx, sy, sz, sColor); }
};

struct __VertexParticle : public __Vector3
{
public:
	float PointSize;
	D3DCOLOR color;

public:
	void Set(const _D3DVECTOR& p, float fPointSize, D3DCOLOR sColor) { x = p.x; y = p.y; z = p.z; color = sColor; PointSize = fPointSize; }
	void Set(float sx, float sy, float sz, float fPointSize, D3DCOLOR sColor) { x = sx; y = sy; z = sz; color = sColor; PointSize = fPointSize; }
	
	__VertexParticle() { PointSize = 1.0f; color=0xffffffff; }
	__VertexParticle(const _D3DVECTOR& p, float fPointSize, D3DCOLOR sColor) { this->Set(p, fPointSize, sColor); }
	__VertexParticle(float sx, float sy, float sz, float fPointSize, D3DCOLOR sColor) { this->Set(sx, sy, sz, fPointSize, sColor); }
};

struct __VertexTransformedColor : public __Vector3
{
public:
	float rhw;
	D3DCOLOR color;

public:
	void Set(float sx, float sy, float sz, float srhw, D3DCOLOR sColor) { x = sx; y = sy; z = sz; rhw = srhw; color = sColor; }
	__VertexTransformedColor() {}
	__VertexTransformedColor(float sx, float sy, float sz, float srhw, D3DCOLOR sColor) { this->Set(sx, sy, sz, srhw, sColor); }
};

struct __VertexT1 : public __Vector3
{
public:
	__Vector3 n;
	float tu, tv;

public:
	void Set(const _D3DVECTOR& p, const _D3DVECTOR& sn, float u, float v)
	{
		x = p.x; y = p.y; z = p.z;
		n = sn;
		tu = u; tv = v;
	}
	void Set(float sx, float sy, float sz, float snx, float sny, float snz, float stu, float stv)
	{
		x = sx; y = sy; z = sz;
		n.x = snx; n.y = sny; n.z = snz;
		tu = stu; tv = stv;
	}

	__VertexT1() {}
	__VertexT1(const _D3DVECTOR& p, const _D3DVECTOR& n, float u, float v) { this->Set(p, n, u, v); }
	__VertexT1(float sx, float sy, float sz, float snx, float sny, float snz, float stu, float stv) 
		{ this->Set(sx, sy, sz, snx, sny, snz, stu, stv); } 
};

struct __VertexT2 : public __VertexT1
{
public:
	float tu2, tv2;
public:
	void Set(const _D3DVECTOR& p, const _D3DVECTOR& sn, float u, float v, float u2, float v2)
	{
		x = p.x; y = p.y; z = p.z;
		n = sn;
		tu = u; tv = v; tu2 = u2; tv2 = v2;
	}
	void Set(float sx, float sy, float sz, float snx, float sny, float snz, float stu, float stv, float stu2, float stv2)
	{
		x = sx; y = sy; z = sz;
		n.x = snx; n.y = sny; n.z = snz;
		tu = stu; tv = stv;
		tu2 = stu2; tv2 = stv2;
	}
	__VertexT2() {}
	__VertexT2(const _D3DVECTOR& p, const _D3DVECTOR& n, float u, float v, float u2, float v2) { this->Set(p, n, u, v, u2, v2); }
	__VertexT2(float sx, float sy, float sz, float snx, float sny, float snz, float stu, float stv, float stu2, float stv2)
		{ this->Set(sx, sy, sz, snx, sny, snz, stu, stv, stu2, stv2); } 
};

struct __VertexTransformed : public __Vector3
{
public:
	float rhw;
	D3DCOLOR color; // 필요 없다..
	float tu, tv;

public:
	void Set(float sx, float sy, float sz, float srhw, D3DCOLOR sColor, float stu, float stv)
		{ x = sx; y = sy; z = sz; rhw = srhw; color = sColor; tu = stu; tv = stv; }

	__VertexTransformed() {}
	__VertexTransformed(float sx, float sy, float sz, float srhw, D3DCOLOR sColor, float stu, float stv)
		{ this->Set(sx, sy, sz, srhw, sColor, stu, stv); }
};

struct __VertexTransformedT2 : public __VertexTransformed
{
public:
	float tu2, tv2;

public:
	void Set(float sx, float sy, float sz, float srhw, D3DCOLOR sColor, float stu, float stv, float stu2, float stv2)
	{ x = sx; y = sy; z = sz; rhw = srhw; color = sColor; tu = stu; tv = stv; tu2 = stu2; tv2 = stv2; }

	__VertexTransformedT2() {}
	__VertexTransformedT2(float sx, float sy, float sz, float srhw, D3DCOLOR sColor, float stu, float stv, float stu2, float stv2)
	{
		this->Set(sx, sy, sz, srhw, sColor, stu, stv, stu2, stv2);
	}
};


//..
struct __VertexXyzT1 : public __Vector3
{
public:
	float tu, tv;	

public:
	void Set(const _D3DVECTOR& p, float u, float v) { x = p.x; y = p.y; z = p.z; tu = u; tv = v; }
	void Set(float sx, float sy, float sz, float u, float v) { x = sx; y = sy; z = sz; tu = u; tv = v; }

	const __VertexXyzT1& operator = (const __Vector3& vec) { x = vec.x; y = vec.y; z = vec.z; return *this; }
	
	__VertexXyzT1() {}
	__VertexXyzT1(const _D3DVECTOR& p, float u, float v) { this->Set(p, u, v); }
	__VertexXyzT1(float sx, float sy, float sz, float u, float v) { this->Set(sx, sy, sz, u, v); }
};


struct __VertexXyzT2 : public __VertexXyzT1
{
public:
	float tu2, tv2;	

public:
	void Set(const _D3DVECTOR& p, float u, float v, float u2, float v2) { x = p.x; y = p.y; z = p.z; tu = u; tv = v; tu2 = u2; tv2 = v2;}
	void Set(float sx, float sy, float sz, float u, float v, float u2, float v2) { x = sx; y = sy; z = sz; tu = u; tv = v; tu2 = u2; tv2 = v2;}

	const __VertexXyzT2& operator = (const __Vector3& vec) { x = vec.x; y = vec.y; z = vec.z; return *this; }

	__VertexXyzT2() {}
	__VertexXyzT2(const _D3DVECTOR& p, float u, float v, float u2, float v2) { this->Set(p, u, v, u2, v2); }
	__VertexXyzT2(float sx, float sy, float sz, float u, float v, float u2, float v2) { this->Set(sx, sy, sz, u, v, u2, v2); }
};



struct __VertexXyzNormal : public __Vector3
{
public:
	__Vector3 n;

public:
	void Set(const _D3DVECTOR& p, const _D3DVECTOR& sn) { x = p.x; y = p.y; z = p.z; n = sn; }
	void Set(float xx, float yy, float zz, float nxx, float nyy, float nzz) { x = xx; y = yy; z = zz; n.x = nxx; n.y = nyy; n.z = nzz; }

	const __VertexXyzNormal& operator = (const __Vector3& vec) { x = vec.x; y = vec.y; z = vec.z; return *this; }

	__VertexXyzNormal() {}
	__VertexXyzNormal(const _D3DVECTOR& p, const _D3DVECTOR& n) { this->Set(p, n); }
	__VertexXyzNormal(float sx, float sy, float sz, float xx, float yy, float zz) { this->Set(sx, sy, sz, xx, yy, zz); }
};


struct __VertexXyzColorSpecularT1 : public __Vector3
{
public:
	D3DCOLOR color;
	D3DCOLOR specular;
	float tu, tv;

public:
	void Set(const _D3DVECTOR& p, D3DCOLOR sColor, D3DCOLOR sSpecular, float u, float v) { x = p.x; y = p.y; z = p.z; color = sColor; specular = sSpecular, tu = u; tv = v; }
	void Set(float sx, float sy, float sz, D3DCOLOR sColor, D3DCOLOR sSpecular, float u, float v) { x = sx; y = sy; z = sz; color = sColor; specular = sSpecular, tu = u; tv = v;	}
	__VertexXyzColorSpecularT1() {}
	__VertexXyzColorSpecularT1(const _D3DVECTOR& p, D3DCOLOR sColor, D3DCOLOR sSpecular, float u, float v) { this->Set(p, sColor, sSpecular, u, v); }
	__VertexXyzColorSpecularT1(float sx, float sy, float sz, D3DCOLOR sColor, D3DCOLOR sSpecular, float u, float v) { this->Set(sx, sy, sz, sColor, sSpecular, u, v); }
};


struct __VertexXyzColorT1 : public __Vector3
{
public:
	D3DCOLOR color;
	float tu, tv;

public:
	void Set(const _D3DVECTOR& p, D3DCOLOR sColor, float u, float v) { x = p.x; y = p.y; z = p.z; color = sColor; tu = u; tv = v; }
	void Set(float sx, float sy, float sz, D3DCOLOR sColor, float u, float v) { x = sx; y = sy; z = sz; color = sColor; tu = u; tv = v;	}
	
	const __VertexXyzColorT1& operator = (const __Vector3& vec) { x = vec.x; y = vec.y; z = vec.z; return *this; }

	__VertexXyzColorT1() {}
	__VertexXyzColorT1(const _D3DVECTOR& p, D3DCOLOR sColor, float u, float v) { this->Set(p, sColor, u, v); }
	__VertexXyzColorT1(float sx, float sy, float sz, D3DCOLOR sColor, float u, float v) { this->Set(sx, sy, sz, sColor, u, v); }
};

struct __VertexXyzColorT2 : public __VertexXyzColorT1
{
public:
	float tu2, tv2;
public:
	void Set(const _D3DVECTOR& p, D3DCOLOR sColor, float u, float v, float u2, float v2) { x = p.x; y = p.y; z = p.z; color = sColor; tu = u; tv = v; tu2 = u2; tv2 = v2;}
	void Set(float sx, float sy, float sz, D3DCOLOR sColor, float u, float v, float u2, float v2) { x = sx; y = sy; z = sz; color = sColor; tu = u; tv = v; tu2 = u2; tv2 = v2;	}
	
	const __VertexXyzColorT2& operator = (const __Vector3& vec) { x = vec.x; y = vec.y; z = vec.z; return *this; }

	__VertexXyzColorT2() {}
	__VertexXyzColorT2(const _D3DVECTOR& p, D3DCOLOR sColor, float u, float v, float u2, float v2) { this->Set(p, sColor, u, v, u2, v2); }
	__VertexXyzColorT2(float sx, float sy, float sz, D3DCOLOR sColor, float u, float v, float u2, float v2) { this->Set(sx, sy, sz, sColor, u, v, u2, v2); }
};



struct __VertexXyzColor : public __Vector3
{
public:
	D3DCOLOR color;

public:
	void Set(const _D3DVECTOR& p, D3DCOLOR sColor) { x = p.x; y = p.y; z = p.z; color = sColor; }
	void Set(float sx, float sy, float sz, D3DCOLOR sColor) { x = sx; y = sy; z = sz; color = sColor; }

	const __VertexXyzColor& operator = (const __Vector3& vec) { x = vec.x; y = vec.y; z = vec.z; return *this; }

	__VertexXyzColor() {}
	__VertexXyzColor(const _D3DVECTOR& p, D3DCOLOR sColor) { this->Set(p, sColor); }
	__VertexXyzColor(float sx, float sy, float sz, D3DCOLOR sColor) { this->Set(sx, sy, sz, sColor); }
};



struct __VertexXyzNormalColor : public __Vector3
{
public:
	__Vector3 n;
	D3DCOLOR color;

public:
	void Set(const _D3DVECTOR& p, const _D3DVECTOR& sn, D3DCOLOR sColor) { x = p.x; y = p.y; z = p.z; n = sn; color = sColor; }
	void Set(float sx, float sy, float sz, float nxx, float nyy, float nzz, D3DCOLOR sColor) { x = sx; y = sy; z = sz; n.x = nxx; n.y = nyy; n.z = nzz; color = sColor; }

	const __VertexXyzNormalColor& operator = (const __Vector3& vec) { x = vec.x; y = vec.y; z = vec.z; return *this; }

	__VertexXyzNormalColor() {}
	__VertexXyzNormalColor(const _D3DVECTOR& p, const _D3DVECTOR& n, D3DCOLOR sColor) { this->Set(p, n, sColor); }
	__VertexXyzNormalColor(float sx, float sy, float sz, float xx, float yy, float zz, D3DCOLOR sColor) { this->Set(sx, sy, sz, xx, yy, zz, sColor); }
};



const int MAX_MIPMAP_COUNT = 10; // 1024 * 1024 단계까지 생성

const uint32_t OBJ_UNKNOWN					= 0;
const uint32_t OBJ_BASE					= 0x1;
const uint32_t OBJ_BASE_FILEACCESS			= 0x2;
const uint32_t OBJ_TEXTURE					= 0x4;
const uint32_t OBJ_TRANSFORM 				= 0x8;
const uint32_t OBJ_TRANSFORM_COLLISION		= 0x10;
const uint32_t OBJ_SCENE					= 0x20;

const uint32_t OBJ_CAMERA					= 0x100;
const uint32_t OBJ_LIGHT					= 0x200;
const uint32_t OBJ_SHAPE					= 0x400;
const uint32_t OBJ_SHAPE_PART				= 0x800;
const uint32_t OBJ_SHAPE_EXTRA				= 0x1000;
const uint32_t OBJ_CHARACTER				= 0x2000;
const uint32_t OBJ_CHARACTER_PART			= 0x4000;
const uint32_t OBJ_CHARACTER_PLUG			= 0x8000;
const uint32_t OBJ_BOARD					= 0x1000;
const uint32_t OBJ_FX_PLUG					= 0x20000;
const uint32_t OBJ_FX_PLUG_PART			= 0x40000;

const uint32_t OBJ_MESH					= 0x100000;
const uint32_t OBJ_MESH_PROGRESSIVE		= 0x200000;
const uint32_t OBJ_MESH_INDEXED			= 0x400000;
const uint32_t OBJ_MESH_VECTOR3			= 0x800000;
const uint32_t OBJ_JOINT					= 0x1000000;
const uint32_t OBJ_SKIN					= 0x2000000;
const uint32_t OBJ_CHARACTER_PART_SKINS	= 0x4000000;

const uint32_t OBJ_DUMMY					= 0x10000000;
const uint32_t OBJ_EFFECT					= 0x20000000;
const uint32_t OBJ_ANIM_CONTROL			= 0x40000000;












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


D3DCOLOR		_RGB_To_D3DCOLOR(COLORREF cr, float fAlpha);
COLORREF		_D3DCOLOR_To_RGB(D3DCOLOR cr);
COLORREF		_D3DCOLORVALUE_To_RGB(const D3DCOLORVALUE& cr);
D3DCOLOR		_D3DCOLORVALUE_To_D3DCOLOR(const D3DCOLORVALUE& cr);
D3DCOLORVALUE	_RGB_To_D3DCOLORVALUE(COLORREF cr, float fAlpha);
bool			_IntersectTriangle(const __Vector3& vOrig, const __Vector3& vDir , const __Vector3& v0, const __Vector3& v1, const __Vector3& v2, float& fT, float& fU, float& fV, __Vector3* pVCol = nullptr);
bool			_IntersectTriangle(const __Vector3& vOrig, const __Vector3& vDir, const __Vector3& v0, const __Vector3& v1, const __Vector3& v2);
bool			_CheckCollisionByBox(const __Vector3& vOrig, const __Vector3& vDir, const __Vector3& vMin, const __Vector3& vMax);
POINT			_Convert3D_To_2DCoordinate(const __Vector3 &vPos, const __Matrix44& mtxView, const __Matrix44& mtxProjection, int nVPW, int nVPH);
void			_Convert2D_To_3DCoordinate(	int ixScreen, int iyScreen, const __Matrix44& mtxView, const __Matrix44& mtxPrj, const D3DVIEWPORT9& vp, __Vector3& vPosResult, __Vector3& vDirResult);
float			_Yaw2D(float fDirX, float fDirZ);

inline D3DCOLOR _RGB_To_D3DCOLOR(COLORREF cr, uint32_t dwAlpha)
{
	D3DCOLOR cr2 =	(dwAlpha << 24) | 
					((cr & 0x000000ff)<<16) | // R
					(cr & 0x0000ff00) | // G
					((cr & 0x00ff0000)>>16); // B
	return cr2;
};

inline COLORREF _D3DCOLOR_To_RGB(D3DCOLOR cr)
{
	COLORREF cr2 =	((cr & 0x00ff0000)>>16) | // R
					(cr & 0x0000ff00) | // G
					((cr & 0x000000ff)<<16); // B
	return cr2;
};

inline COLORREF _D3DCOLORVALUE_To_RGB(const D3DCOLORVALUE& cr)
{
	COLORREF cr2 =	(((uint32_t)(cr.r*255.0f))) | // R
					(((uint32_t)(cr.g*255.0f))<<8) | // G
					(((uint32_t)(cr.b*255.0f))<<16); // B
	return cr2;
};

inline D3DCOLOR _D3DCOLORVALUE_To_D3DCOLOR(const D3DCOLORVALUE& cr)
{
	COLORREF cr2 =	(((uint32_t)(cr.a*255.0f))<<24) | // A
					(((uint32_t)(cr.r*255.0f))<<16) | // R
					(((uint32_t)(cr.g*255.0f))<<8) | // G
					(((uint32_t)(cr.b*255.0f))); // B
	return cr2;
};

inline D3DCOLORVALUE _RGB_To_D3DCOLORVALUE(COLORREF cr, float fAlpha)
{
	D3DCOLORVALUE cr2;
	cr2.a =	fAlpha; // Alpha
	cr2.r = (cr & 0x000000ff)/255.0f;
	cr2.g = ((cr & 0x0000ff00)>>8)/255.0f;
	cr2.b = ((cr & 0x00ff0000)>>16)/255.0f;
	return cr2;
};

inline D3DCOLORVALUE _D3DCOLOR_To_D3DCOLORVALUE(D3DCOLOR cr)
{
	D3DCOLORVALUE cr2;
	cr2.a =	((cr & 0xff000000)>>24)/255.0f;
	cr2.r = ((cr & 0x00ff0000)>>16)/255.0f;
	cr2.g = ((cr & 0x0000ff00)>>8)/255.0f;
	cr2.b = (cr & 0x000000ff)/255.0f; // Alpha
	return cr2;
};

inline bool _CheckCollisionByBox(const __Vector3& vOrig, const __Vector3& vDir, const __Vector3& vMin, const __Vector3& vMax)
{
	static __Vector3 Vertices[36];
	int nFace = 0;

	// z 축 음의 면
	nFace = 0;
	Vertices[nFace+0].Set(vMin.x, vMax.y, vMin.z); Vertices[nFace+1].Set(vMax.x, vMax.y, vMin.z); Vertices[nFace+2].Set(vMax.x, vMin.y, vMin.z);
	Vertices[nFace+3] = Vertices[nFace+0]; Vertices[nFace+4] = Vertices[nFace+2]; Vertices[nFace+5].Set(vMin.x, vMin.y, vMin.z);

	// x 축 양의 면
	nFace = 6;
	Vertices[nFace+0].Set(vMax.x, vMax.y, vMin.z); Vertices[nFace+1].Set(vMax.x, vMax.y, vMax.z); Vertices[nFace+2].Set(vMax.x, vMin.y, vMax.z);
	Vertices[nFace+3] = Vertices[nFace+0]; Vertices[nFace+4] = Vertices[nFace+2]; Vertices[nFace+5].Set(vMax.x, vMin.y, vMin.z);

	// z 축 양의 면
	nFace = 12;
	Vertices[nFace+0].Set(vMax.x, vMax.y, vMax.z); Vertices[nFace+1].Set(vMin.x, vMax.y, vMax.z); Vertices[nFace+2].Set(vMin.x, vMin.y, vMax.z);
	Vertices[nFace+3] = Vertices[nFace+0]; Vertices[nFace+4] = Vertices[nFace+2]; Vertices[nFace+5].Set(vMax.x, vMin.y, vMax.z);

	// x 축 음의 면
	nFace = 18;
	Vertices[nFace+0].Set(vMin.x, vMax.y, vMax.z); Vertices[nFace+1].Set(vMin.x, vMax.y, vMin.z); Vertices[nFace+2].Set(vMin.x, vMin.y, vMin.z);
	Vertices[nFace+3] = Vertices[nFace+0]; Vertices[nFace+4] = Vertices[nFace+2]; Vertices[nFace+5].Set(vMin.x, vMin.y, vMax.z);

	// y 축 양의 면
	nFace = 24;
	Vertices[nFace+0].Set(vMin.x, vMax.y, vMax.z); Vertices[nFace+1].Set(vMax.x, vMax.y, vMax.z); Vertices[nFace+2].Set(vMax.x, vMax.y, vMin.z);
	Vertices[nFace+3] = Vertices[nFace+0]; Vertices[nFace+4] = Vertices[nFace+2]; Vertices[nFace+5].Set(vMin.x, vMax.y, vMin.z);

	// y 축 음의 면
	nFace = 30;
	Vertices[nFace+0].Set(vMin.x, vMin.y, vMin.z); Vertices[nFace+1].Set(vMax.x, vMin.y, vMin.z); Vertices[nFace+2].Set(vMax.x, vMin.y, vMax.z);
	Vertices[nFace+3] = Vertices[nFace+0]; Vertices[nFace+4] = Vertices[nFace+2]; Vertices[nFace+5].Set(vMin.x, vMin.y, vMax.z);

	// 각 면에 대해서 충돌 검사..
	for(int i = 0; i < 12; i++)
	{
		if(true == ::_IntersectTriangle(vOrig, vDir, Vertices[i*3+0], Vertices[i*3+1], Vertices[i*3+2]))
			return true;
	}

	return false;
}

inline bool _IntersectTriangle(const __Vector3& vOrig, const __Vector3& vDir,
							  const __Vector3& v0, const __Vector3& v1, const __Vector3& v2,
							  float& fT, float& fU, float& fV, __Vector3* pVCol)
{
	// Find vectors for two edges sharing vert0
	static __Vector3 vEdge1, vEdge2;

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
	fV = D3DXVec3Dot(&vDir, &qVec);
	if (fV < 0.0f || fU + fV > fDet)
		return false;

	// Calculate t, scale parameters, ray intersects triangle
	fT = D3DXVec3Dot(&vEdge2, &qVec);

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
	static float fDet, fT, fU, fV;
	static __Vector3 vEdge1, vEdge2, tVec, pVec, qVec;

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
	fV = D3DXVec3Dot(&vDir, &qVec);
	if (fV < 0.0f || fU + fV > fDet)
		return false;

	// Calculate t, scale parameters, ray intersects triangle
	fT = D3DXVec3Dot(&vEdge2, &qVec) / fDet;

	// *t < 0 이면 뒤쪽...
	if (fT < 0.0f)
		return false;

	return true;
}

inline POINT _Convert3D_To_2DCoordinate(const __Vector3& vPos, const __Matrix44& mtxView, const __Matrix44& mtxProjection, int nVPW, int nVPH)
{
	__Matrix44 matVP;
	D3DXMatrixMultiply(&matVP, &mtxView, &mtxProjection);
	D3DXVECTOR4 v;
	D3DXVec3Transform(&v, (D3DXVECTOR3*) (&vPos), &matVP);

	POINT pt;
	float fScreenZ = (v.z / v.w);
	if (fScreenZ > 1.0 || fScreenZ < 0.0)
	{
		pt.x = -1;
		pt.y = -1;
		return pt;
	}

	pt.x = int(((v.x / v.w) + 1.0f) * (nVPW) / 2.0f);
	pt.y = int((1.0f - (v.y / v.w)) * (nVPH) / 2.0f);

	return pt;
}

inline void _Convert2D_To_3DCoordinate(	int ixScreen, int iyScreen,
										const __Matrix44& mtxView, const __Matrix44& mtxPrj, const D3DVIEWPORT9& vp,
										__Vector3& vPosResult, __Vector3& vDirResult)
{
	// Compute the vector of the pick ray in screen space
	static __Vector3 vTmp;

	vTmp.x = (((2.0f * ixScreen) / (vp.Width)) - 1) / mtxPrj._11;
	vTmp.y = -(((2.0f * iyScreen) / (vp.Height)) - 1) / mtxPrj._22;
	vTmp.z = 1.0f;

	// Transform the screen space pick ray into 3D space
	__Matrix44 mtxVI;
	::D3DXMatrixInverse(&mtxVI, nullptr, &mtxView);
	vDirResult.x = vTmp.x * mtxVI._11 + vTmp.y * mtxVI._21 + vTmp.z * mtxVI._31;
	vDirResult.y = vTmp.x * mtxVI._12 + vTmp.y * mtxVI._22 + vTmp.z * mtxVI._32;
	vDirResult.z = vTmp.x * mtxVI._13 + vTmp.y * mtxVI._23 + vTmp.z * mtxVI._33;
	vPosResult = mtxVI.Pos();
}

inline float _Yaw2D(float fDirX, float fDirZ)
{
	////////////////////////////////
	// 방향을 구하고.. -> 회전할 값을 구하는 루틴이다..
	if (fDirX >= 0.0f)						// ^^
	{
		if (fDirZ >= 0.0f) return (float) (asin(fDirX));
		else return (D3DXToRadian(90.0f) + (float) (acos(fDirX)));
	}
	else
	{
		if (fDirZ >= 0.0f) return (D3DXToRadian(270.0f) + (float) (acos(-fDirX)));
		else return(D3DXToRadian(180.0f) + (float) (asin(-fDirX)));
	}
	// 방향을 구하고..
	////////////////////////////////
}

inline int16_t _IsKeyDown(int iVirtualKey)
{
	return (GetAsyncKeyState(iVirtualKey) & 0xff00);
}

inline int16_t _IsKeyDowned(int iVirtualKey)
{
	return (GetAsyncKeyState(iVirtualKey) & 0x00ff);
}

#endif // __MY_3DSTRUCT_H_


// N3Sun.h: interface for the CN3Sun class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3SUN_H__8CBCEEB3_465E_4884_9549_C86D2CCC8936__INCLUDED_)
#define AFX_N3SUN_H__8CBCEEB3_465E_4884_9549_C86D2CCC8936__INCLUDED_

#pragma once

#include "N3Base.h"
#include "N3ColorChange.h"

enum eSUNPART : uint8_t
{
	SUNPART_SUN = 0,
	SUNPART_GLOW,
	SUNPART_FLARE,
	NUM_SUNPART
};

class CN3Texture;
class CN3Sun : public CN3Base
{
	friend class CN3SkyMng;

public:
	CN3Sun();
	~CN3Sun() override;

protected:
	struct __SunPart
	{
		CN3Texture* pTex                 = nullptr; // texture
		__VertexTransformed pVertices[4] = {};      // vertex

		CN3ColorChange Color             = {};      // 색을 담당하는 클래스
		CN3DeltaChange Delta             = {};
	};

	__SunPart m_Parts[NUM_SUNPART];
	float m_fCurRadian; // 현재 해의 회전위치
						// Operations
public:
	// 현재 각도설정
	void SetCurAngle(float fAngle)
	{
		m_fCurRadian = DegreesToRadians(fAngle);
	}

	void Init(const std::string* pszFNs);

	float GetCurAngle() const
	{
		return RadiansToDegrees(m_fCurRadian);
	}

	void Release() override;
	void Render(__Matrix44& matView, __Matrix44& matProj);
	void Tick();

protected:
};

#endif // !defined(AFX_N3SUN_H__8CBCEEB3_465E_4884_9549_C86D2CCC8936__INCLUDED_)
